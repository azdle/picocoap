#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#include <modality/probe.h>
#include "../../generated_component_definitions.h"

#include "../../src/coap.h"
 
void hex_dump(uint8_t* bytes, size_t len);
void coap_pretty_print(coap_pdu*);
void modality_tick(modality_probe *probe);

modality_probe *g_producer_probe = MODALITY_PROBE_NULL_INITIALIZER;
uint8_t g_producer_probe_buffer[1024];
int g_report_socket;
struct addrinfo *g_collector_addr;

int main(void)
{
  char host[] = "coap.me";
  char port[] = "5683";

  srand(time(NULL));

  assert(MODALITY_PROBE_INIT(
    &g_producer_probe_buffer[0],
    sizeof(g_producer_probe_buffer),
    CLIENT_MAIN_PROCESS_PROBE,
    MODALITY_PROBE_TIME_RESOLUTION_UNSPECIFIED,
    MODALITY_PROBE_WALL_CLOCK_ID_LOCAL_ONLY,
    NULL,
    NULL,
    &g_producer_probe,
    MODALITY_TAGS("coap", "cli"),
    "picocoap posix example probe"
  ) == 0);

  int rv;
  struct addrinfo modalityhints, exohints, *servinfo, *q;

  memset(&modalityhints, 0, sizeof modalityhints);
  modalityhints.ai_family = AF_UNSPEC;
  modalityhints.ai_socktype = SOCK_DGRAM;
  modalityhints.ai_flags = AI_PASSIVE;

  // Modality Collector Address
  if ((rv = getaddrinfo("127.0.0.1", "34333", &modalityhints, &g_collector_addr)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // Socket to Modality Collector

  if ((rv = getaddrinfo("127.0.0.1", "34555", &modalityhints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // loop through all the results and make a socket
  for(q = servinfo; q != NULL; q = q->ai_next) {
    if ((g_report_socket = socket(q->ai_family, q->ai_socktype, q->ai_protocol)) == -1) {
      perror("bad socket");
      continue;
    }

    break;
  }

  bind(g_report_socket, q->ai_addr, q->ai_addrlen);

  if (q == NULL) {
      fprintf(stderr, "Failed to Bind Socket\n");
      return 2;
  }


  // CoAP Message Setup
  #define MSG_BUF_LEN 64
  uint8_t msg_send_buf[MSG_BUF_LEN];
  coap_pdu msg_send = {msg_send_buf, 0, 64};
  uint8_t msg_recv_buf[MSG_BUF_LEN];
  coap_pdu msg_recv = {msg_recv_buf, 0, 64};

  uint16_t message_id_counter = rand();

  // Socket to Exosite
  int remotesock;
  size_t bytes_sent;
  ssize_t bytes_recv;

  memset(&exohints, 0, sizeof exohints);
  exohints.ai_family = AF_UNSPEC;
  exohints.ai_socktype = SOCK_DGRAM;
  exohints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo(host, port, &exohints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }


  // loop through all the results and make a socket
  for(q = servinfo; q != NULL; q = q->ai_next) {
    if ((remotesock = socket(q->ai_family, q->ai_socktype, q->ai_protocol)) == -1) {
      perror("bad socket");
      continue;
    }

    break;
  }

  if (q == NULL) {
      fprintf(stderr, "Failed to Bind Socket\n");
      return 2;
  }
 
  for (uint8_t sent_count = 1; ; sent_count++) 
  {
    printf("--------------------------------------------------------------------------------\n");

    MODALITY_PROBE_RECORD(g_producer_probe, SEND_STARTED);

    // Build Message
    coap_init_pdu(&msg_send);
    coap_set_version(&msg_send, COAP_V1);
    coap_set_type(&msg_send, CT_CON);
    coap_set_code(&msg_send, CC_GET);
    coap_set_mid(&msg_send, message_id_counter++);
    coap_set_token(&msg_send, rand(), 2);
    coap_add_option(&msg_send, CON_URI_PATH, (uint8_t*)"hello", 5);

    MODALITY_PROBE_RECORD(g_producer_probe, MESSAGE_BUILT);

    // Send Message
    if ((bytes_sent = sendto(remotesock, msg_send.buf, msg_send.len, 0, q->ai_addr, q->ai_addrlen)) == -1){
      fprintf(stderr, "Failed to Send Message\n");
      return 2;
    }


    modality_tick(g_producer_probe);
    MODALITY_PROBE_RECORD(g_producer_probe, PACKET_SENT);

    printf("Sent.\n");
    coap_pretty_print(&msg_send);

    struct timeval tv;
    fd_set readfds;

    tv.tv_sec = 4;
    tv.tv_usec = 0;

    MODALITY_PROBE_RECORD(g_producer_probe, START_RECEIVE_WAIT);

    FD_ZERO(&readfds);
    FD_SET(remotesock, &readfds);
    assert(FD_ISSET(remotesock, &readfds));

    // don't care about writefds and exceptfds:
    select(remotesock+1, &readfds, NULL, NULL, &tv);

    assert(FD_ISSET(remotesock, &readfds));

    if (!FD_ISSET(remotesock, &readfds)) {
	printf("Timeout\n");
        MODALITY_PROBE_RECORD(g_producer_probe, RECEIVE_TIMEOUT);

	    if (sent_count > 3) {
		printf("Failed\n");
	    	return 1;
	    } else {
		printf("Retrying\n");
		continue;
	    }
    }

    
    MODALITY_PROBE_RECORD(g_producer_probe, SOCKET_READY);

    // Wait for Response
    bytes_recv = recvfrom(remotesock, (void *)msg_recv.buf, msg_recv.max, 0, q->ai_addr, &q->ai_addrlen);
    if (bytes_recv < 0) {
      fprintf(stderr, "%s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }


    //MODALITY_PROBE_RECORD(g_producer_probe, PACKET_RECEVIED, msg_recv.buf);

    msg_recv.len = bytes_recv;

    if(coap_validate_pkt(&msg_recv) == CE_NONE)
    {
      MODALITY_PROBE_RECORD(g_producer_probe, VALID_PACKET);
      printf("Got Valid CoAP Packet\n");
      if(coap_get_mid(&msg_recv) == coap_get_mid(&msg_send) &&
         coap_get_token(&msg_recv) == coap_get_token(&msg_send))
      {
      MODALITY_PROBE_RECORD(g_producer_probe, VALID_RESPONSE);
        printf("Is Response to Last Message\n");
        coap_pretty_print(&msg_recv);
	return 0;
      }
    }else{
      MODALITY_PROBE_RECORD(g_producer_probe, INVALID_PACKET);
      printf("Received %zi Bytes, Not Valid CoAP\n", msg_recv.len);
      hex_dump(msg_recv.buf, msg_recv.len);
    }

    sleep(1); // One Second
  }
}

void hex_dump(uint8_t* bytes, size_t len)
{
  size_t i, j;
  for (i = 0; i < len; i+=16){
    printf("  0x%.3zx    ", i);
    for (j = 0; j < 16; j++){
      if (i+j < len)
        printf("%02hhx ", bytes[i+j]);
      else
        printf("%s ", "--");
    }
    printf("   %.*s\n", (int)(16 > len-i ? len-i : 16), bytes+i);
  }
}

void coap_pretty_print(coap_pdu *pdu)
{
  coap_option opt;

  opt.num = 0;

  if (coap_validate_pkt(pdu) == CE_NONE){
      printf(" ------ Valid CoAP Packet (%zi) ------ \n", pdu->len);
      printf("Type:  %i\n",coap_get_type(pdu));
      printf("Code:  %i.%02i\n", coap_get_code_class(pdu), coap_get_code_detail(pdu));
      printf("MID:   0x%X\n", coap_get_mid(pdu));
      printf("Token: 0x%lX\n", coap_get_token(pdu));

      while ((opt = coap_get_option(pdu, &opt)).num != 0){
        if (opt.num == 0)
          break;

        printf("Option: %i\n", opt.num);
        if (opt.val != NULL && opt.len != 0)
          printf(" Value: %.*s (%zi)\n", (int)opt.len, opt.val, opt.len);
      }
      // Note: get_option returns payload pointer when it finds the payload marker
      if (opt.val != NULL && opt.len != 0)
        printf("Payload: %.*s (%zi)\n", (int)opt.len, opt.val, opt.len);
    } else {
      printf(" ------ Non-CoAP Message (%zi) ------ \n", pdu->len);
      hex_dump(pdu->buf, pdu->len);
    }
}

void send_modality_report(modality_probe *probe) {
	size_t report_size;
	uint8_t report_buffer[1500];
	const size_t err = modality_probe_report(
			probe,
			report_buffer,
			sizeof(report_buffer),
			&report_size);

	printf("Modality Err: %li\n", err);

	assert(err == MODALITY_PROBE_ERROR_OK);

	if (report_size != 0) {

  	struct addrinfo *q;
		  // loop through all the results and make a socket
		  for(q = g_collector_addr; q != NULL; q = q->ai_next) {

			const ssize_t status = sendto(
					g_report_socket,
					report_buffer, // isn't this a circular buffer?
					report_size,
					0,
					g_collector_addr->ai_addr,
					g_collector_addr->ai_addrlen);
			if (status == -1)
      		  		fprintf(stderr, "%s\n", strerror(errno));
			else
				break;
			//assert(status != -1);
		  }

		  if(q == NULL)
			  assert(false);
	}
}

void recieve_modailty_control(modality_probe *probe)
{
  uint8_t bytes [1500];

  /* Check for new control messages */ // just one? or all?
  struct timeval tv;
  fd_set readfds;

  tv.tv_sec = 0;
  tv.tv_usec = 0;

  FD_ZERO(&readfds);
  FD_SET(g_report_socket, &readfds);
  assert(FD_ISSET(g_report_socket, &readfds));

  // don't care about writefds and exceptfds:
  select(g_report_socket+1, &readfds, NULL, NULL, &tv);

  if (FD_ISSET(g_report_socket, &readfds)) {
    size_t length = recvfrom(g_report_socket, (void *)bytes, sizeof(bytes), 0, NULL, NULL);
    if(length != 0)
    {
      size_t should_forward;
      size_t err = modality_probe_process_control_message(
        probe,
        bytes,
        length,
        &should_forward);
      assert(err == MODALITY_PROBE_ERROR_OK);
    }
  }
}

void modality_tick(modality_probe *probe) {
  send_modality_report(probe);
  recieve_modailty_control(probe);
}

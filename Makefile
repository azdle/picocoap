CC=gcc
IDIR=src

test: tests/coap_test.c src/coap.h
	$(CC) -g tests/coap_test.c src/coap.c -o test -Wall
	./test
	rm test

buildtest: tests/coap_test.c src/coap.h
	$(CC) -g tests/coap_test.c src/coap.c -o test -Wall
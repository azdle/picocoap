cmake_minimum_required(VERSION 3.5)
project(picocoap)

set(CMAKE_BUILD_TYPE Debug)

# 1. Load ModalityProbe config package
find_package(ModalityProbe REQUIRED)

add_library(picocoap src/coap.c)

add_executable(picocoap-client examples/posix/client.c)
target_link_libraries(picocoap-client PUBLIC picocoap)

# 2. Link Modality's libraries
target_link_libraries(
    picocoap-client
    PRIVATE
    # Required
    ModalityProbe::LibModalityProbe
    # Optional: needed if using control message convenience utilities from the Modality probe support library
    ModalityProbe::LibModalityProbeSupport)

# 3. Call manifest and header generation functions
modality_probe_generate_manifest(
    TARGET picocoap-client
    COMPONENT_NAME "my-component"
    OUTPUT_PATH "my-component"
    EXCLUDES "build"
    FILE_EXTENSIONS "c"
    SOURCE_PATH ".")

modality_probe_generate_header(
    TARGET picocoap-client
    OUTPUT_FILE "generated_component_definitions.h"
    COMPONENT_PATH "my-component")


cmake_minimum_required(VERSION 3.30)
project(webserver C)

set(CMAKE_C_STANDARD 23)

# Define the target output directory (where the executable is built)
set(OUTPUT_DIR ${CMAKE_BINARY_DIR})

# Define where your static files are
set(STATIC_FILES_DIR ${CMAKE_SOURCE_DIR}/public)

# Copy the public directory (including index.html) to the output directory
file(COPY ${STATIC_FILES_DIR} DESTINATION ${OUTPUT_DIR})

add_executable(webserver main.c
        request.c
        request.h
        response.c
        response.h
        logger.c
        logger.h
        file_handler.c
        file_handler.h
        server.c
        server.h
)
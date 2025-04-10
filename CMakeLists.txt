cmake_minimum_required(VERSION 3.30)
project(webserver C)

set(CMAKE_C_STANDARD 23)

# Define the target output directory (where the executable is built)
set(OUTPUT_DIR ${CMAKE_BINARY_DIR})

# Copy the public directory (including index.html) to the output directory
set(OUTPUT_STATIC_DIR ${CMAKE_BINARY_DIR}/public)

set(STATIC_FILES_DIR ${CMAKE_SOURCE_DIR}/public)

# Create a custom target that will run during each build
add_custom_target(copy_static_files ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${STATIC_FILES_DIR} ${OUTPUT_STATIC_DIR}
        COMMENT "Copying static files to build directory"
)

add_executable(webserver main.c
        server.h
        server.c
        request.c
        request.h
        response.c
        response.h
        logger.c
        logger.h
        file_handler.c
        file_handler.h
        server_threaded.c
        server_async.c
        server_async.h
        server_threaded.h
)

# Force the build to copy our HTML files every time
add_dependencies(webserver copy_static_files)

# Add threading library
find_package(Threads REQUIRED)

if(WIN32)
    target_link_libraries(webserver PRIVATE ws2_32 Threads::Threads)
else()
    target_link_libraries(webserver PRIVATE Threads::Threads)
endif()

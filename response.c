#include "response.h"
#include "file_handler.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * You need this function to decide what kind of content type to return
 * Without it, the web browser won't know what to do with the contents and
 * it will display it as plain text rather than HTML
 * */
const char *get_content_type(const char *path) {
    //  If there's a period in the path we'll assume a file extension:
    const char *ext = strrchr(path, '.');
    //  If we found an extension, check it:
    if (ext) {
        if (strcmp(ext, ".html") == 0) return "text/html";
        if (strcmp(ext, ".css") == 0) return "text/css";
        if (strcmp(ext, ".js") == 0) return "application/javascript";
        if (strcmp(ext, ".png") == 0) return "image/png";
        if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
        if (strcmp(ext, ".txt") == 0) return "text/plain";
    }
    // No extension or unrecognized extension, return binary data:
    return "application/octet-stream";
}

/*
 * This function handles the request and returns a response
 * */
Response handle_request(const Request *req) {
    Response res = {0};

    // For right now, we're only going to handle GET requests
    if (strcmp(req->method, "GET") != 0) {
        res.body = strdup("HTTP/1.1 405 Method Not Allowed\r\n\r\n");
        return res;
    }

    // TODO - build the file path and
    //  You'll want to use snprintf to build the file path
    //   snprintf is like printf / sprintf but takes an extra parameter to specify how many bytes
    //   https://en.cppreference.com/w/c/io/fprintf


    //  TODO - now read the actual file and get the contents into the Response struct somehow
    //    Be sure to handle any errors such as the file doesn't exist


    //  TODO - Now get the content type into a string


    //  TODO - at this point, you have the path, the file contents, and the content type so put all of that into
    //    the response body.  You'll need to allocate memory for it.
    //    there are a lot of ways to do this, feel free to be creative and do it however you want.
    //    Just make sure that when you return the Response struct, the body is filled out with _everything_ that is
    //    needed to be sent back to the client.

    return res;
}



/*
 * This function frees the response body.  Since we're allocating memory for the body, whatever function
 * uses it needs a way to free it when it's done with it.
 * */
void free_response(Response *res) {
    // TODO - implement this function to free the response body
}

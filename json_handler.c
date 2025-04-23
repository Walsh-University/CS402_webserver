#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "response.h"
#include "file_handler.h"
#include "logger.h"
#include "json_handler.h"


// Function to find a string within a JSON object
char* json_get_string(JsonValue* obj, const char* key) {
    if (!obj || obj->type != JSON_OBJECT || !key)
        return NULL;

    for (size_t i = 0; i < obj->value.object.count; i++) {
        JsonField field = obj->value.object.fields[i];
        if (strcmp(field.key, key) == 0 && field.value->type == JSON_STRING) {
            return field.value->value.string;
        }
    }
    return NULL;
}

// Function to free JSON value
void json_free(JsonValue* value) {
    if (!value) return;

    switch (value->type) {
        case JSON_STRING:
            free(value->value.string);
            break;
        case JSON_ARRAY:
            for (size_t i = 0; i < value->value.array.count; i++) {
                json_free(value->value.array.items[i]);
            }
            free(value->value.array.items);
            break;
        case JSON_OBJECT:
            for (size_t i = 0; i < value->value.object.count; i++) {
                free(value->value.object.fields[i].key);
                json_free(value->value.object.fields[i].value);
            }
            free(value->value.object.fields);
            break;
        default:
            break;
    }
    free(value);
}

// Helper function for parsing JSON
void skip_whitespace(const char** json) {
    while (**json && isspace(**json)) (*json)++;
}

// Parse a JSON string
char* parse_json_string(const char** json) {
    if (**json != '"') return NULL;
    (*json)++; // Skip opening quote

    const char* start = *json;
    while (**json && **json != '"') {
        // Handle escape sequences
        if (**json == '\\' && *(*json + 1)) {
            (*json)++;
        }
        (*json)++;
    }

    if (**json != '"') return NULL; // No closing quote

    size_t len = *json - start;
    char* result = malloc(len + 1);
    if (!result) return NULL;

    // Copy and handle escape sequences (simplified)
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        if (start[i] == '\\' && i + 1 < len) {
            i++;
            switch (start[i]) {
                case 'n': result[j++] = '\n'; break;
                case 'r': result[j++] = '\r'; break;
                case 't': result[j++] = '\t'; break;
                default: result[j++] = start[i]; break;
            }
        } else {
            result[j++] = start[i];
        }
    }
    result[j] = '\0';

    (*json)++; // Skip closing quote
    return result;
}

// Forward declaration for recursive parsing
JsonValue* parse_json_value(const char** json);

// Parse a JSON object
JsonValue* parse_json_object(const char** json) {
    if (**json != '{') return NULL;
    (*json)++; // Skip opening brace
    skip_whitespace(json);

    JsonValue* obj = malloc(sizeof(JsonValue));
    if (!obj) return NULL;

    obj->type = JSON_OBJECT;
    obj->value.object.fields = NULL;
    obj->value.object.count = 0;

    // Empty object
    if (**json == '}') {
        (*json)++;
        return obj;
    }

    // Allocate initial capacity
    size_t capacity = 8;
    obj->value.object.fields = malloc(capacity * sizeof(JsonField));
    if (!obj->value.object.fields) {
        free(obj);
        return NULL;
    }

    while (1) {
        skip_whitespace(json);

        // Parse key
        if (**json != '"') {
            json_free(obj);
            return nullptr;
        }

        char* key = parse_json_string(json);
        if (!key) {
            json_free(obj);
            return nullptr;
        }

        skip_whitespace(json);
        if (**json != ':') {
            free(key);
            json_free(obj);
            return nullptr;
        }
        (*json)++; // Skip colon

        // Parse value
        JsonValue* value = parse_json_value(json);
        if (!value) {
            free(key);
            json_free(obj);
            return nullptr;
        }

        // Resize array if needed
        if (obj->value.object.count >= capacity) {
            capacity *= 2;
            JsonField* new_fields = realloc(obj->value.object.fields, capacity * sizeof(JsonField));
            if (!new_fields) {
                free(key);
                json_free(value);
                json_free(obj);
                return NULL;
            }
            obj->value.object.fields = new_fields;
        }

        // Add key-value pair
        obj->value.object.fields[obj->value.object.count].key = key;
        obj->value.object.fields[obj->value.object.count].value = value;
        obj->value.object.count++;

        skip_whitespace(json);
        if (**json == '}') {
            (*json)++;
            return obj;
        }

        if (**json != ',') {
            json_free(obj);
            return nullptr;
        }
        (*json)++; // Skip comma
    }
}

// Parse any JSON value
JsonValue* parse_json_value(const char** json) {
    skip_whitespace(json);

    JsonValue* value = malloc(sizeof(JsonValue));
    if (!value) return NULL;

    // Determine type based on first character
    switch (**json) {
        case '{': // Object
            json_free(value);
            return parse_json_object(json);

        case '"': // String
            value->type = JSON_STRING;
            value->value.string = parse_json_string(json);
            if (!value->value.string) {
                free(value);
                return nullptr;
            }
            return value;

            // Add support for other types as needed (arrays, numbers, booleans, null)
            // This is a simplified parser that focuses on objects and strings

        default:
            free(value);
            return NULL;
    }
}

// Main JSON parsing function
JsonValue* parse_json(const char* json_str) {
    if (!json_str) return NULL;
    return parse_json_value(&json_str);
}

// Handle JSON POST request
Response handle_json_request(const Request *req) {
    Response res = {0};

    // Parse the JSON body
    JsonValue* json = parse_json(req->body);
    if (!json) {
        res.body = strdup("HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nInvalid JSON");
        return res;
    }

    // Example: Handle login request with JSON
    if (strcmp(req->path, "/api/login") == 0) {
        char* username = json_get_string(json, "username");
        char* password = json_get_string(json, "password");

        if (username && password) {
            log_info("JSON login attempt for user: %s", username);

            // Here you would implement your authentication logic
            // Example: if (check_credentials(username, password)) ...

            // Create response JSON (simple example)
            char response_text[256];
            snprintf(response_text, sizeof(response_text),
                     "{\"success\":true,\"message\":\"Welcome, %s\"}",
                     username);

            size_t response_len = strlen(response_text);
            res.body = malloc(response_len + 256);
            if (res.body) {
                snprintf(res.body, response_len + 256,
                         "HTTP/1.1 200 OK\r\n"
                         "Content-Type: application/json\r\n"
                         "Content-Length: %zu\r\n"
                         "\r\n"
                         "%s",
                         response_len, response_text);
            }
        } else {
            // Missing required fields
            res.body = strdup("HTTP/1.1 400 Bad Request\r\n"
                              "Content-Type: application/json\r\n"
                              "\r\n"
                              "{\"success\":false,\"message\":\"Missing username or password\"}");
        }
    } else {
        // Unknown endpoint
        res.body = strdup("HTTP/1.1 404 Not Found\r\n"
                          "Content-Type: application/json\r\n"
                          "\r\n"
                          "{\"success\":false,\"message\":\"Endpoint not found\"}");
    }

    // Clean up
    json_free(json);

    return res;
}


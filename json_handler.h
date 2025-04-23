//
// Created by David Good on 4/10/25.
//

#ifndef WEBSERVER_JSON_HANDLER_H
#define WEBSERVER_JSON_HANDLER_H


// Simple JSON parsing functionality
typedef enum {
    JSON_NULL,
    JSON_BOOLEAN,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} JsonType;

typedef struct JsonValue JsonValue;

// Forward declaration for JSON object fields
typedef struct {
    char* key;
    JsonValue* value;
} JsonField;

struct JsonValue {
    JsonType type;
    union {
        int boolean;
        double number;
        char* string;
        struct {
            JsonValue** items;
            size_t count;
        } array;
        struct {
            JsonField* fields;
            size_t count;
        } object;
    } value;
};

JsonValue* parse_json(const char* json_str);
Response handle_json_request(const Request *req);

#endif //WEBSERVER_JSON_HANDLER_H

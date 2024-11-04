#include "json_parser.h"

#include <stdio.h>
#include <string.h>
#include <json-c/json.h>

static const char current[] = "current_condition";

static const char description[] = "lang_ru";
static const char value[] = "value";
static const char wind_direction[] = "winddir16Point";
static const char wind_speed[] = "windspeedKmph";
static const char temperature[] = "temp_C";

const char* get_string_value(struct json_object* source, const char* key) {
    struct json_object* val;
    int result = json_object_object_get_ex(source, key, &val);
    if (result != 1) {
        printf("Failed to get %s\n", key);
        return NULL;
    }
    return json_object_get_string(val);
}

struct json_object* get_current_condition(const char* serialized_weather){
    enum json_tokener_error error;
    struct json_object* object = json_tokener_parse_verbose(serialized_weather, &error);
    if (error != json_tokener_success) {
        printf("Failed to parse responce\n");
        return NULL;
    }
    struct json_object* conditions;
    int result = json_object_object_get_ex(object, current, &conditions);
    if (result != 1) {
        printf("Failed to get current condition\n");
        return NULL;
    }
    return json_object_array_get_idx(conditions, 0);
}

const char* get_description(struct json_object* condition){
    struct json_object* desc;
    int result = json_object_object_get_ex(condition, description, &desc);
    if (result != 1) {
        printf("Failed to get description\n");
        return NULL;
    }
    return get_string_value(json_object_array_get_idx(desc, 0), value);
}

const char* get_wind_direction(struct json_object* condition){
    return get_string_value(condition, wind_direction);
}

const char* get_wind_speed(struct json_object* condition) {
    return get_string_value(condition, wind_speed);
}

const char* get_temperature(struct json_object* condition) {
    return get_string_value(condition, temperature);
}


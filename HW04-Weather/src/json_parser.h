#ifndef JSON_PARSER_H
#define JSON_PARSER_H

struct json_object* get_current_condition(const char* serialized_weather);

const char* get_description(struct json_object* condition);

const char* get_wind_direction(struct json_object* condition);

const char* get_wind_speed(struct json_object* condition);

const char* get_temperature(struct json_object* condition);

#endif // JSON_PARSER_H

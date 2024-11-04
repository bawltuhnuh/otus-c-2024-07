#include "weather.h"
#include "json_parser.h"

#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>

static const char url[] = "https://wttr.in/";
static const char options[] = "?format=j1&lang=ru";

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) {
        printf("Not enough memory: realloc returned NULL\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

char* get_request(const char* location){
    char* request = malloc(strlen(url) + strlen(location) + strlen(options) + 1);
    if (!request) {
        printf("Not enough memory: malloc returned NULL\n");
        return NULL;
    }
    strcpy(request, url);
    strcat(request, location);
    strcat(request, options);
    return request;
}

char* get_serialized_weather(const char* location) {
    char* request = get_request(location);
    if (!request) {
        return NULL;
    }

    CURL *curl_handle;
    CURLcode res;

    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    curl_easy_setopt(curl_handle, CURLOPT_URL, request);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

    res = curl_easy_perform(curl_handle);

    if(res != CURLE_OK) {
        fprintf(stderr, "Failed to perform request %s: %s\n", request, curl_easy_strerror(res));
        free(request);
        return NULL;
    }

    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();

    free(request);

    return chunk.memory;
}

void print_weather(const char* location) {
    char* serialized_weather = get_serialized_weather(location);
    if (!serialized_weather) {
        return;
    }
    struct json_object* condition = get_current_condition(serialized_weather);
    printf("Погода в %s\n", location);
    printf("%s\n", get_description(condition));
    printf("Направление ветра: %s\n", get_wind_direction(condition));
    printf("Скорость ветра: %s\n", get_wind_speed(condition));
    printf("Температура: %s\n", get_temperature(condition));

    free(serialized_weather);
}

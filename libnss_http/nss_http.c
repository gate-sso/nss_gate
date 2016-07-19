#include <yaml.h>
#include "nss_http.h"

struct response {
    char *data;
    size_t pos;
};


// Newer versions of Jansson have this but the version
// on Ubuntu 12.04 don't, so make a wrapper.
extern size_t
j_strlen(json_t *str) {
    return strlen(json_string_value(str));
}


static size_t write_response(void *ptr, size_t size, size_t nmemb, void *stream) {
    struct response *result = (struct response *) stream;
    size_t required_len = result->pos + size * nmemb;

    if (required_len >= NSS_HTTP_INITIAL_BUFFER_SIZE - 1) {
        if (required_len < NSS_HTTP_MAX_BUFFER_SIZE) {
            result->data = realloc(result->data, required_len);
            if (!result->data) {
                // Failed to initialize a large enough buffer for the data.
                return 0;
            }
        } else {
            // Request data is too large.
            return 0;
        }
    }

    memcpy(result->data + result->pos, ptr, size * nmemb);
    result->pos += size * nmemb;

    return size * nmemb;
}


char *
nss_http_request(const char *url) {
    CURL *curl = NULL;
    CURLcode status;
    struct curl_slist *headers = NULL;
    char *data = NULL;
    long code;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (!curl) goto error;

    data = malloc(NSS_HTTP_INITIAL_BUFFER_SIZE);
    if (!data) goto error;

    struct response write_result = {.data = data, .pos = 0};

    curl_easy_setopt(curl, CURLOPT_URL, url);

    headers = curl_slist_append(headers, "User-Agent: NSS-HTTP");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_result);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    status = curl_easy_perform(curl);
    if (status != 0) {
        fprintf(stderr, "Error executing curl - returned with error code %d\n", (int)status);
        goto error;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    if (code != 200) {
        fprintf(stderr, "Got Wrong status code %d", (int)code);
        goto error;
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    curl_global_cleanup();

    data[write_result.pos] = '\0';

    return data;

    error:
    if (data)
        free(data);
    if (curl)
        curl_easy_cleanup(curl);
    if (headers)
        curl_slist_free_all(headers);
    curl_global_cleanup();

    return NULL;
}

void get_config_host(char *out_hostname, char *out_token) {
    //FILE *fh = fopen("/etc/nss_http/nss_http.yml", "r");
    FILE *fh = fopen("/etc/gate/nss.yml", "r");
    yaml_parser_t parser;
    yaml_token_t token;   /* new variable */

    /* Initialize parser */
    if (!yaml_parser_initialize(&parser))
        fputs("Failed to initialize parser!\n", stderr);
    if (fh == NULL)
        fputs("Failed to open file!\n", stderr);

    /* Set input file */
    yaml_parser_set_input_file(&parser, fh);

    int found_key_token = 0;
    int found_root_token = 0;
    int found_host_token = 0;
    int found_host_value_token = 0;

    int found_api_key_value_token = 0;
    int found_api_key_token = 0;

    int debug = 0;

    /* BEGIN new code */
    do {
        yaml_parser_scan(&parser, &token);

        if (token.type == YAML_KEY_TOKEN) {

            if (debug)
                printf("found key token\n");
            found_key_token = 1;

        }

        if (token.type == YAML_SCALAR_TOKEN && found_key_token == 1) {
            if (!strncmp((const char *) token.data.scalar.value, "nss_http", strlen((const char *) "nss_http"))) {

                if (debug)
                    printf("found root token %s\n", token.data.scalar.value);
                found_root_token = 1;
            }

            if (found_root_token == 1 &&
                !strncmp((const char *) token.data.scalar.value, "host_url", strlen("host_url"))) {
                if (debug)
                    printf("found host token\n");
                found_host_token = 1;
            }


            if (found_root_token == 1 &&
                !strncmp((const char *) token.data.scalar.value, "api_key", strlen("api_key"))) {
                if (debug)
                    printf("found host token\n");
                found_api_key_token = 1;

            }


            if (found_host_value_token == 1) {
                if (debug)
                    printf("found host value\n");
                strcpy(out_hostname, (const char *) token.data.scalar.value);
                found_host_value_token = 0;
                found_host_token = 0;
            }

            if (found_api_key_value_token == 1) {
                if (debug)
                    printf("found api key token\n");

                strcpy(out_token, (const char *) token.data.scalar.value);
                found_api_key_value_token = 0;
                found_api_key_token = 0;


            }

        }

        if (token.type == YAML_VALUE_TOKEN && found_root_token == 1 && found_host_token == 1) {
            if (debug)
                printf("found host value\n");
            found_host_value_token = 1;

        }

        if (token.type == YAML_VALUE_TOKEN && found_root_token == 1 && found_api_key_token == 1) {
            if (debug)
                printf("found host value\n");
            found_api_key_value_token = 1;

        }


        if (token.type != YAML_STREAM_END_TOKEN)
            yaml_token_delete(&token);
    } while (token.type != YAML_STREAM_END_TOKEN);
    yaml_token_delete(&token);
    /* END new code */

    /* Cleanup */
    yaml_parser_delete(&parser);
    fclose(fh);
}

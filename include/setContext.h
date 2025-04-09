#ifndef SET_CONTEXT_H_
#define SET_CONTEXT_H_

#include <string.h>

#include "mandelbrot.h"

const size_t MAX_BUFFER_SIZE = 256;

#define GET_STRING(json, structName, field) do { \
    cJSON *item = cJSON_GetObjectItemCaseSensitive(json, #field); \
    if (item && cJSON_IsString(item) && item->valuestring) { \
        strncpy(structName->field, item->valuestring, 32); \
        structName->field[31] = '\0'; \
    } \
} while (0)

#define GET_INT(json, structName, field) do { \
    cJSON *item = cJSON_GetObjectItemCaseSensitive(json, #field); \
    if (item && cJSON_IsNumber(item)) { \
        structName->field = item->valueint; \
    } \
} while (0)

#define GET_DOUBLE(json, structName, field) do { \
    cJSON *item = cJSON_GetObjectItemCaseSensitive(json, #field); \
    if (item && cJSON_IsNumber(item)) { \
        structName->field = item->valuedouble; \
    } \
} while (0)

#define GET_BOOL(json, structName, field) do { \
    cJSON *item = cJSON_GetObjectItemCaseSensitive(json, #field); \
    if (item) { \
        structName->field = cJSON_IsTrue(item); \
    } \
} while (0)

#define FREE_(ptr) do { \
    free(ptr);          \
    ptr = NULL;         \
} while (0)

renderContext *createRenderContext();
renderError    setRenderSettings  (renderContext *context);
cJSON         *parseJson          (const char    *jsonFile);

#endif // SET_CONTEXT_H_
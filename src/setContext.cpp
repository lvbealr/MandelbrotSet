#include <cjson/cJSON.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "customWarning.h"

#include "setContext.h"
#include "render.h"

renderContext *createRenderContext() {
    renderContext *context = (renderContext *)calloc(1, sizeof(renderContext));
    customWarning(context != NULL, NULL);

    context->window = (renderWindow *)calloc(1, sizeof(renderWindow));
    customWarning(context->window != NULL, NULL);

    setRenderSettings(context);

    return context;
}

renderError setRenderSettings(renderContext *context) {
    customAssert(context != NULL, INVALID_POINTER);
    customAssert(context->window != NULL, INVALID_POINTER);

    renderWindow *window = context->window;

    cJSON *json = parseJson("settings.json");
    customAssert(json != NULL, JSON_PARSE_ERROR);

    GET_STRING(json, window,  windowName);
    GET_INT   (json, window,  width);
    GET_INT   (json, window,  height);
    GET_DOUBLE(json, context, xStartShift);
    GET_DOUBLE(json, context, yStartShift);
    GET_DOUBLE(json, context, linearShift);
    GET_DOUBLE(json, context, scaleShift);
    GET_DOUBLE(json, context, maxRadius);
    GET_INT   (json, context, maxIter);
    GET_BOOL  (json, context, showFPS);

    cJSON_Delete(json);

    return NO_ERRORS;
}

cJSON *parseJson(const char *jsonFile) {
    int fileD = open(jsonFile, O_RDONLY);
    customAssert(fileD != -1, FILE_NOT_FOUND);

    char *buffer = (char *)calloc(MAX_BUFFER_SIZE, sizeof(char));
    customAssert(buffer != NULL, ALLOCATION_ERROR);

    ssize_t readBytes = read(fileD, buffer, MAX_BUFFER_SIZE);
    customAssert(readBytes != -1, FILE_READ_ERROR);

    cJSON *json = cJSON_Parse(buffer);
    customAssert(json != NULL, JSON_PARSE_ERROR);

    FREE_(buffer);
    close(fileD);

    return json;
}
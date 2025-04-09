#ifndef RENDER_H_
#define RENDER_H_

#include <inttypes.h>
enum renderError {
    NO_ERRORS        = 0,
    INVALID_POINTER  = 1,
    FILE_NOT_FOUND   = 2,
    FILE_READ_ERROR  = 3,
    JSON_PARSE_ERROR = 4,
    ALLOCATION_ERROR = 5
};

const size_t MAX_WINDOW_NAME_SIZE = 32;

struct renderWindow {
    char windowName[MAX_WINDOW_NAME_SIZE] = {};

    unsigned width   = {};
    unsigned height  = {};
};

struct renderContext {
    renderWindow *window = {};

    float xStartShift = {};
    float yStartShift = {};

    float linearShift = {};
    float scaleShift  = {};

    float maxRadius   = {};

    const float dx = maxRadius / (float) window->width;
    const float dy = maxRadius / (float) window->height;

    unsigned maxIter    = {};
    bool     showFPS    = {};
};

typedef renderError (*colorTheme)(renderContext *, uint8_t *, const float, const float, const float);

void hsvToRgb(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b);

////////////////////////////////////////////// BLACK && WHITE WAVES ////////////////////////////////////////////////
renderError theme_0(renderContext *context, uint8_t *points, const float x, const float y, const float iter);

//////////////////////////////////////////// BLUE && PURPLE GRADIENT ///////////////////////////////////////////////
renderError theme_1(renderContext *context, uint8_t *points, const float x, const float y, const float iter);

//////////////////////////////////////////////// GOLD && BROWN WAVES ///////////////////////////////////////////////
renderError theme_2(renderContext *context, uint8_t *points, const float x, const float y, const float iter);

#endif // RENDER_H_
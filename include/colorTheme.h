#ifndef COLOR_THEME_H_
#define COLOR_THEME_H_

const size_t WIDTH    = 1920;
const size_t HEIGHT   = 1080;

const size_t ITER_MAX = 1024;

enum renderError {
    NO_ERRORS = 0,
    NULL_PTR  = 1
};

void hsvToRgb(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b);

/////////////////////////////// BLACK && WHITE WAVES /////////////////////////////////
renderError theme_0(uint8_t *points, const float x, const float y, const float iter);

////////////////////////////// BLUE && PURPLE GRADIENT ///////////////////////////////
renderError theme_1(uint8_t *points, const float x, const float y, const float iter);

/////////////////////////////// GOLD && BROWN WAVES //////////////////////////////////
renderError theme_2(uint8_t *points, const float x, const float y, const float iter);

#endif // COLOR_THEME_H_
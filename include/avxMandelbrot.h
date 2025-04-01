#ifndef AVX_MANDELBROT_H_
#define AVX_MANDELBROT_H_

#include <SFML/Graphics.hpp>
#include <stdint.h>

const size_t WIDTH   = 1920;
const size_t HEIGHT  = 1080;

const size_t ITER_MAX = 8192;

const size_t RGB_MAX  = 256;

const float  X_START_SHIFT = -4.0f;
const float  Y_START_SHIFT = -2.0f;

const float  LIN_SHIFT   = 0.1;
const float  SCALE_SHIFT = 0.1;

const float MAX_RADIUS = 4.0f;

const float dx = MAX_RADIUS / WIDTH;
const float dy = MAX_RADIUS / HEIGHT;

enum renderError {
    NO_ERRORS = 0,
    NULL_PTR  = 1
};

renderError generateColor      (uint8_t *points, const float x, const float y, const float iter);
renderError calculateMandelbrot(sf::RenderWindow *window, uint8_t *points, const float xShift, const float yShift, const float scale);
renderError handleKeyboard     (const std::optional<sf::Event> event, float *xShift, float *yShift, float *scale);
renderError drawMandelbrotSet  (sf::RenderWindow *window, uint8_t *points);

#endif // AVX_MANDELBROT_H_
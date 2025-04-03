#ifndef MANDELBROT_H_
#define MANDELBROT_H_

#include <SFML/Graphics.hpp>
#include <stdint.h>

#include "colorTheme.h"

const size_t RGB_MAX  = 256;

const float  X_START_SHIFT = -4.0f;
const float  Y_START_SHIFT = -2.0f;

const float  LIN_SHIFT   = 0.1;
const float  SCALE_SHIFT = 0.1;

const float MAX_RADIUS = 4.0f;

const float dx = MAX_RADIUS / WIDTH;
const float dy = MAX_RADIUS / HEIGHT;

const size_t TEST = 300;

typedef renderError (*colorTheme)(uint8_t *, const float, const float, const float);

renderError calculateMandelbrot(sf::RenderWindow *window, uint8_t *points, const float xShift, const float yShift, const float scale);
renderError handleKeyboard     (const std::optional<sf::Event> event, float *xShift, float *yShift, float *scale);
renderError drawMandelbrotSet  (sf::RenderWindow *window, uint8_t *points);

#endif // MANDELBROT_H_
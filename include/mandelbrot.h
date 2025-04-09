#ifndef MANDELBROT_H_
#define MANDELBROT_H_

#include <SFML/Graphics.hpp>
#include <stdint.h>

#include "render.h"

renderError    setRenderSettings  (renderContext *context);
renderContext *createRenderContext();

renderError calculateMandelbrot(renderContext *context, sf::RenderWindow *window, uint8_t *points, const float  xShift, const float  yShift, const float  scale);
renderError drawMandelbrotSet  (renderContext *context, sf::RenderWindow *window, uint8_t *points, sf::Texture *texture);

renderError handleKeyboard     (renderContext *context, const std::optional<sf::Event> event,            float *xShift,       float *yShift,       float *scale);

#endif // MANDELBROT_H_
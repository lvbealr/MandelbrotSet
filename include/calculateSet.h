#ifndef CALCULATE_SET_H_
#define CALCULATE_SET_H_

#include "render.h"
#include "mandelbrot.h"

renderError naiveCalculation         (renderContext *context, uint8_t *points, const float xShift, const float yShift, const float scale, size_t startY, size_t endY);
renderError arrayOptimizedCalculation(renderContext *context, uint8_t *points, const float xShift, const float yShift, const float scale, size_t startY, size_t endY);
renderError avxOptimizedCalculation  (renderContext *context, uint8_t *points, const float xShift, const float yShift, const float scale, size_t startY, size_t endY);

typedef renderError (*calculateSectionFunc) \
                                     (renderContext *, uint8_t *, const float, const float, const float, size_t, size_t);

#endif // CALCULATE_SET_H_
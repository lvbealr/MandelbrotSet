#ifndef CALCULATE_SET_H_
#define CALCULATE_SET_H_

#include "colorTheme.h"

renderError naiveCalculation         (uint8_t *points, const float xShift, const float yShift, const float scale, size_t startY, size_t endY);
renderError arrayOptimizedCalculation(uint8_t *points, const float xShift, const float yShift, const float scale, size_t startY, size_t endY);
renderError avxOptimizedCalculation  (uint8_t *points, const float xShift, const float yShift, const float scale, size_t startY, size_t endY);

typedef renderError (*calculateSectionFunc)(uint8_t *, const float, const float, const float, size_t, size_t);

#endif // CALCULATE_SET_H_
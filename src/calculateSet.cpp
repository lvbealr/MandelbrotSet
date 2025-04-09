#include <immintrin.h>
#include <math.h>

#include "customWarning.h"

#include "calculateSet.h"
#include "intrinsics.h"
#include "mandelbrot.h"
#include "render.h"

//////////////////////////////////// COLORS ////////////////////////////////////////////

extern const colorTheme   COLOR_THEME[];
extern const size_t       MAX_COLOR_THEME;

extern size_t     COLOR_THEME_INDEX;
extern colorTheme setTheme;

////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////// CALCULATE FUNCTIONS /////////////////////////////////////

extern const calculateSectionFunc CALCULATE_FUNCS[] = {naiveCalculation,            \
                                                       arrayOptimizedCalculation,   \
                                                       avxOptimizedCalculation};

extern const size_t MAX_CALCULATE_FUNC_INDEX        = sizeof(CALCULATE_FUNCS) /     \
                                                      sizeof(CALCULATE_FUNCS[0]);

size_t               CALCULATE_FUNC_INDEX = 0;
calculateSectionFunc calculateSection     = CALCULATE_FUNCS[CALCULATE_FUNC_INDEX];

////////////////////////////// CALCULATE FUNCTIONS /////////////////////////////////////

renderError naiveCalculation(renderContext *context, uint8_t *points, const float xShift, const float yShift, const float scale, size_t startY, size_t endY) {
    customAssert(points != NULL, INVALID_POINTER);

    float width  = (float) context->window->width;
    float height = (float) context->window->height;

    float maxR   = (float) context->maxRadius;

    size_t maxIter = context->maxIter;

    for (size_t y = startY; y < endY; y++) {
        float     y0 = (y / height - 0.5f) * maxR * scale + yShift;

        for (size_t x = 0; x < width; x++) {
            float x0 = (x / width -  0.5f) * maxR * scale + xShift;

            float X = x0;
            float Y = y0;

            size_t iter = 0;

            for (; iter < maxIter; iter++) {
                float X2 = X * X;
                float Y2 = Y * Y;
                float XY = X * Y;

                float R2 = X2 + Y2;

                if (R2 > maxR) {
                    break;
                }

                X = X2 - Y2 + x0;
                Y = XY + XY + y0;
            }

            setTheme(context, points, x, y, iter);
        }
    }

    return NO_ERRORS;
}

renderError arrayOptimizedCalculation(renderContext *context, uint8_t *points, const float xShift, const float yShift, const float scale, size_t startY, size_t endY) {
    float width  = (float) context->window->width;
    float height = (float) context->window->height;

    float maxR   = (float) context->maxRadius;

    size_t maxIter = context->maxIter;

    const float dx = maxR / width;

    float R2max[ARRAY_SIZE];
    mm256_set1_ps(R2max, maxR);

    float POINTS[ARRAY_SIZE];
    mm256_set_ps(POINTS, 0.f, 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f);

    for (size_t y = startY; y < endY; y++) {
        float y0 = (y / height - 0.5f) * maxR * scale + yShift;

        for (size_t x = 0; x < width; x += 8) {
            float x0 = (x / width - 0.5f) * maxR * scale + xShift;

            float dxScale   [ARRAY_SIZE];
            mm256_set1_ps(dxScale, dx * scale);

            float POINTS_MUL[ARRAY_SIZE];
            mm256_mul_ps(POINTS_MUL, POINTS, dxScale);

            float X0[ARRAY_SIZE];
            mm256_set1_ps(X0, x0); mm256_add_ps(X0, X0, POINTS_MUL);

            float Y0[ARRAY_SIZE];
            mm256_set1_ps(Y0, y0);

            float X[ARRAY_SIZE];
            FOR_LOOP(X, X[idx] = X0[idx];);

            float Y[ARRAY_SIZE];
            FOR_LOOP(Y, Y[idx] = Y0[idx];);

            int iterations[ARRAY_SIZE];
            mm256_setzero_si256(iterations);

            for (size_t iteration = 0; iteration < maxIter; iteration++) {
                float X2[ARRAY_SIZE];
                mm256_mul_ps(X2, X, X);

                float Y2[ARRAY_SIZE];
                mm256_mul_ps(Y2, Y, Y);

                float XY[ARRAY_SIZE];
                mm256_mul_ps(XY, X, Y);

                float R2 [ARRAY_SIZE];
                mm256_add_ps(R2, X2, Y2);

                float cmp[ARRAY_SIZE];
                mm256_cmp_ps(cmp, R2, R2max, CMP_LE_OQ_);

                int mask = mm256_movemask_ps(cmp);

                if (!mask) {
                    break;
                }

                int ones[ARRAY_SIZE];
                mm256_set1_epi32(ones, 1);

                int cmpi[ARRAY_SIZE];
                mm256_castps_si256(cmpi, cmp);
                mm256_and_si256(cmpi, ones);

                mm256_add_epi32(iterations, iterations, cmpi);

                float X2subY2[ARRAY_SIZE];
                mm256_sub_ps(X2subY2, X2, Y2);
                mm256_add_ps(X, X2subY2, X0);

                float XYaddXY[ARRAY_SIZE];
                mm256_add_ps(XYaddXY, XY, XY);
                mm256_add_ps(Y, XYaddXY, Y0);
            }

            int iter[ARRAY_SIZE];
            mm256_storeu_si256(iter, iterations);

            for (size_t i = 0; i < 8 && x + i < width; i++) {
                setTheme(context, points, x + i, y, (float) iter[i]);
            }
        }
    }

    return NO_ERRORS;
}

renderError avxOptimizedCalculation(renderContext *context, uint8_t *points, const float xShift, const float yShift, const float scale, size_t startY, size_t endY) {
    float width  = (float) context->window->width;
    float height = (float) context->window->height;

    float maxR   = (float) context->maxRadius;

    size_t maxIter = context->maxIter;

    const float dx = maxR / width;

    const __m256 R2max  = _mm256_set1_ps(maxR);
    const __m256 POINTS = _mm256_set_ps(7.f, 6.f, 5.f, 4.f, 3.f, 2.f, 1.f, 0.f);
    
    for (size_t y = startY; y < endY; y++) {
        float y0 = (y / height - 0.5f) * maxR * scale + yShift;

        for (size_t x = 0; x < width; x += 8) {
            float x0 = (x / width - 0.5f) * maxR * scale + xShift;

            __m256 X0 = _mm256_add_ps(_mm256_set1_ps(x0), _mm256_mul_ps(POINTS, _mm256_set1_ps(dx * scale)));
            __m256 Y0 = _mm256_set1_ps(y0);

            __m256 X = X0;
            __m256 Y = Y0;

            __m256i iterations = _mm256_setzero_si256();

            for (size_t iteration = 0; iteration < maxIter; iteration++) {
                __m256 X2 = _mm256_mul_ps(X, X);
                __m256 Y2 = _mm256_mul_ps(Y, Y);
                __m256 XY = _mm256_mul_ps(X, Y);

                __m256 R2  = _mm256_add_ps(X2, Y2);
                __m256 cmp = _mm256_cmp_ps(R2, R2max, _CMP_LE_OQ);

                int mask = _mm256_movemask_ps(cmp);

                if (!mask) {
                    break;
                }

                __m256i ones = _mm256_set1_epi32(1);
                
                iterations = _mm256_add_epi32(iterations, _mm256_and_si256(_mm256_castps_si256(cmp), ones));

                X = _mm256_add_ps(_mm256_sub_ps(X2, Y2), X0);
                Y = _mm256_add_ps(_mm256_add_ps(XY, XY), Y0);

                _mm_prefetch((const char*)&points[(y * (size_t) width + x + 8) * 4], _MM_HINT_T0);
            }

            int iter[8];
            _mm256_storeu_si256((__m256i*)iter, iterations);

            for (size_t i = 0; i < 8 && x + i < (size_t) width; i++) {
                setTheme(context, points, x + i, y, (float) iter[i]);
            }
        }
    }

    return NO_ERRORS;
}
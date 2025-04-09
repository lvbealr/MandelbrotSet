#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include "colorTheme.h"
#include "customWarning.h"

//////////////////////////////////// COLORS ////////////////////////////////////////////

extern const colorTheme COLOR_THEME[]   = {theme_0, \
                                           theme_1, \
                                           theme_2};

extern const size_t     MAX_COLOR_THEME = sizeof(COLOR_THEME) /   \
                                          sizeof(COLOR_THEME[0]);

size_t COLOR_THEME_INDEX = 0;
colorTheme setTheme      = COLOR_THEME[COLOR_THEME_INDEX];

////////////////////////////////////////////////////////////////////////////////////////

void hsvToRgb(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b) {
    h = fmod(h, 360.0f);

    if (h < 0.0f) {
        h += 360.0f;
    }

    float c = v * s;
    float x = c * (1.0f - fabs(fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;

    float r1, g1, b1;

    if (h < 60) {
        
        r1 = c;
        g1 = x;
        b1 = 0;

    } else if (h < 120) {

        r1 = x;
        g1 = c;
        b1 = 0;
        
    } else if (h < 180) {

        r1 = 0;
        g1 = c;
        b1 = x;

    } else if (h < 240) {

        r1 = 0;
        g1 = x;
        b1 = c;

    } else if (h < 300) {

        r1 = x;
        g1 = 0;
        b1 = c;

    } else {

        r1 = c;
        g1 = 0;
        b1 = x;

    }

    *r = (uint8_t)((r1 + m) * 255.0f);
    *g = (uint8_t)((g1 + m) * 255.0f);
    *b = (uint8_t)((b1 + m) * 255.0f);
}

renderError theme_0(uint8_t *points, const float x, const float y, const float iter) {
    customAssert(points != NULL, NULL_PTR);
 
    size_t pointPos   = (size_t)(y * WIDTH + x) * 4;
    float  t          = iter / (float)(ITER_MAX - 1);
 
    uint8_t intensity = (uint8_t)(127.5f * (1 + cosf(255.0f * t)));
 
    points[pointPos]     = intensity;
    points[pointPos + 1] = intensity;
    points[pointPos + 2] = intensity;
    points[pointPos + 3] = 255;
 
    return NO_ERRORS;
 }
 
 renderError theme_1(uint8_t *points, const float x, const float y, const float iter) {
     customAssert(points != NULL, NULL_PTR);
 
     size_t pointPos = (size_t)(y * WIDTH + x) * 4;
     float threshold = 0.05f * ITER_MAX;
 
     if (iter == ITER_MAX || iter < threshold) {

        points[pointPos]     = 20;
        points[pointPos + 1] = 20;
        points[pointPos + 2] = 20;

     } else {

        float t = iter / (float)(ITER_MAX - 1);
 
        float baseHue      = 128.0f;
        float hueVariation = 5.0f;
        float saturation   = 0.8f;
 
        float value = 0.8f + 0.2f * t;
        float hue   = baseHue + hueVariation * cosf(256.0f * t);
 
        uint8_t r_t, g_t, b_t = {};
        hsvToRgb(hue, saturation, value, &r_t, &g_t, &b_t);
 
        float glow = 0.1f + 0.7f * sinf(40.0f * t);
        uint8_t r  = (uint8_t)(r_t * (1.0f - glow) + 128 * glow);
        uint8_t g  = (uint8_t)(g_t * (1.0f - glow) + 128 * glow);
        uint8_t b  = (uint8_t)(b_t * (1.0f - glow) + 128 * glow);
 
        float lightDirX = -0.7f;
        float lightDirY = -0.7f;

        float lightMag = sqrtf(lightDirX * lightDirX + lightDirY * lightDirY);

        lightDirX /= lightMag;
        lightDirY /= lightMag;
 
        float centerX = WIDTH / 2.0f;
        float centerY = HEIGHT / 2.0f;

        float normX = (x - centerX) / centerX;
        float normY = (y - centerY) / centerY;
 
        float angle   = atan2f(normY, normX);
        float normalX = cosf(angle);
        float normalY = sinf(angle);
 
        float shading = normalX * lightDirX + normalY * lightDirY;
        shading       = 0.1f + 0.5f * shading;
 
        r = (uint8_t)fminf(255.0f, r * shading);
        g = (uint8_t)fminf(255.0f, g * shading);
        b = (uint8_t)fminf(255.0f, b * shading);
 
        float distX    = fmin(x, WIDTH - x);
        float distY    = fmin(y, HEIGHT - y);

        float edgeDist = fmin(distX, distY);
 
        float borderThreshold = 65536.0f;
        float edgeGlowFactor  = 1.0f - fmin(edgeDist / borderThreshold, 1.0f);
 
        float gradientFactor = x / (float) WIDTH;
 
        uint8_t tintR = (uint8_t)(0   * (1.0f - gradientFactor) + 255 * gradientFactor);
        uint8_t tintG = (uint8_t)(150 * (1.0f - gradientFactor) + 15  * gradientFactor);
        uint8_t tintB = (uint8_t)(255 * (1.0f - gradientFactor) + 192 * gradientFactor);
 
        r = (uint8_t)(r * (1.0f - edgeGlowFactor) + tintR * edgeGlowFactor);
        g = (uint8_t)(g * (1.0f - edgeGlowFactor) + tintG * edgeGlowFactor);
        b = (uint8_t)(b * (1.0f - edgeGlowFactor) + tintB * edgeGlowFactor);
 
        points[pointPos]     = r;
        points[pointPos + 1] = g;
        points[pointPos + 2] = b;
     }
 
    points[pointPos + 3] = 255;
    
    return NO_ERRORS;
 }
 
 renderError theme_2(uint8_t *points, const float x, const float y, const float iter) {
    customAssert(points != NULL, NULL_PTR);
 
    size_t pointPos = (size_t)(y * WIDTH + x) * 4;
 
    if (iter == ITER_MAX) {

        points[pointPos]     = 0;
        points[pointPos + 1] = 0;
        points[pointPos + 2] = 0;

    } else {

        float t = iter / (float) (ITER_MAX - 1);
 
        float goldenHue        = 40.0f;
        float goldenSaturation = 0.8f;
 
        float hueVariation = 15.0f * sinf(255.0f * t);
        float hue          = goldenHue + hueVariation;
 
        float value = 0.5f + 0.3f * t;
 
        uint8_t r, g, b = {};
        hsvToRgb(hue, goldenSaturation, value, &r, &g, &b);
 
        r = (uint8_t)(r * 1.2f);
        g = (uint8_t)(g * 0.9f);
        b = (uint8_t)(b * 0.5f);
 
        points[pointPos]     = std::min(255, (int) r);
        points[pointPos + 1] = std::min(255, (int) g);
        points[pointPos + 2] = std::min(255, (int) b);

    }
 
    points[pointPos + 3] = 255.f * cosf(sinf(255 * iter));
 
    return NO_ERRORS;
 }
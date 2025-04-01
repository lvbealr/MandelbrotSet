#include <SFML/Graphics.hpp>
#include <immintrin.h>

#include <math.h>

#include "avxMandelbrot.h"
#include "customWarning.h"

void hsvToRgb(float h, float s, float v, uint8_t& r, uint8_t& g, uint8_t& b) {
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

    r = (uint8_t)((r1 + m) * 255.0f);
    g = (uint8_t)((g1 + m) * 255.0f);
    b = (uint8_t)((b1 + m) * 255.0f);
}

//renderError generateColor(uint8_t *points, const float x, const float y, const float iter) {
//    if (!points) return NULL_PTR;
//
//    size_t pointPos = (size_t)(y * WIDTH + x) * 4;
//    float t = iter / (float)(ITER_MAX - 1);
//
//    // black&white waves
//    uint8_t intensity = (uint8_t)(127.5f * (1 + cosf(255.0f * t)));
//
//    points[pointPos]     = intensity;
//    points[pointPos + 1] = intensity;
//    points[pointPos + 2] = intensity;
//    points[pointPos + 3] = 255;
//
//    return NO_ERRORS;
//}

renderError generateColor(uint8_t *points, const float x, const float y, const float iter) {
    customAssert(points != NULL, NULL_PTR);

    size_t pointPos = (size_t)(y * WIDTH + x) * 4;
    float threshold = 0.05f * ITER_MAX;

    if (iter == ITER_MAX || iter < threshold) {
        points[pointPos]     = 20;
        points[pointPos + 1] = 20;
        points[pointPos + 2] = 20;
    } else {
        float t = iter / (float)(ITER_MAX - 1);

        // base color
        float baseHue = 128.0f;
        float hueVariation = 5.0f;
        float saturation = 0.8f;

        // more value more brightness
        float value = 0.8f + 0.2f * t;
        float hue = baseHue + hueVariation * cosf(256.0f * t);

        uint8_t r_t, g_t, b_t;
        hsvToRgb(hue, saturation, value, r_t, g_t, b_t);


        float glow = 0.1f + 0.7f * sinf(40.0f * t);
        uint8_t r = (uint8_t)(r_t * (1.0f - glow) + 128 * glow);
        uint8_t g = (uint8_t)(g_t * (1.0f - glow) + 128 * glow);
        uint8_t b = (uint8_t)(b_t * (1.0f - glow) + 128 * glow);

        // 3d light
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
        shading = 0.1f + 0.5f * shading;

        r = (uint8_t)fminf(255.0f, r * shading);
        g = (uint8_t)fminf(255.0f, g * shading);
        b = (uint8_t)fminf(255.0f, b * shading);

        float distX = fmin(x, WIDTH - x);
        float distY = fmin(y, HEIGHT - y);
        float edgeDist = fmin(distX, distY);

        float borderThreshold = 65536.0f;
        float edgeGlowFactor = 1.0f - fmin(edgeDist / borderThreshold, 1.0f);

        // glow gradient
        // blue (from left)  RGB = (0, 150, 255)
        // pink (from right) RGB = (255, 15, 192)

        float gradientFactor = x / (float) WIDTH;

        uint8_t tintR = (uint8_t)(0 * (1.0f - gradientFactor) + 255  * gradientFactor);
        uint8_t tintG = (uint8_t)(150  * (1.0f - gradientFactor) + 15   * gradientFactor);
        uint8_t tintB = (uint8_t)(255 * (1.0f - gradientFactor) + 192  * gradientFactor);

        // interpolate
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

renderError calculateMandelbrot(sf::RenderWindow *window, uint8_t *points,
                                const float xShift, const float yShift, const float scale) {
    customAssert(window != NULL, NULL_PTR);
    customAssert(points != NULL, NULL_PTR);

    const float dx = MAX_RADIUS / WIDTH;

    static const __m256 R2max  = _mm256_set1_ps(MAX_RADIUS);
    // [MAX_RADIUS MAX_RADIUS MAX_RADIUS MAX_RADIUS MAX_RADIUS MAX_RADIUS MAX_RADIUS MAX_RADIUS ]
    static const __m256 POINTS = _mm256_set_ps (7.f, 6.f, 5.f, 4.f, 3.f, 2.f, 1.f, 0.f);
    // [7.f 6.f 5.f 4.f 3.f 2.f 1.f 0.f]

    for (size_t y = 0; y < HEIGHT; y++) {
        float y0 = (y / (float) HEIGHT - 0.5f) * MAX_RADIUS * scale + yShift;

        for (size_t x = 0; x < WIDTH; x += 8) {
            float x0 = (x / (float) WIDTH - 0.5f) * MAX_RADIUS * scale + xShift;

            __m256 X0 = _mm256_add_ps(_mm256_set1_ps(x0), _mm256_mul_ps(POINTS, _mm256_set1_ps(dx * scale)));
            // [x0 + 0*dx x0 + 1*dx x0 + 2*dx ... x0 + 7*dx]
            __m256 Y0 = _mm256_set1_ps(y0);
            // [y0 y0 y0 y0 y0 y0 y0 y0]

            __m256 X = X0;
            __m256 Y = Y0;
            __m256i iterations = _mm256_setzero_si256();

            for (size_t iteration = 0; iteration < ITER_MAX; iteration++) {
                __m256 X2  = _mm256_mul_ps(X, X);
                // [(x0+0*dx)^2 (x0+1*dx)^2 (x0+2*dx)^2 (x0+3*dx)^2 (x0+4*dx)^2 (x0+5*dx)^2 (x0+6*dx)^2 (x0+7*dx)^2]
                __m256 Y2  = _mm256_mul_ps(Y, Y);
                // [y0^2 y0^2 y0^2 y0^2 y0^2 y0^2 y0^2 y0^2]
                __m256 XY  = _mm256_mul_ps(X, Y);
                // [(x0+0*dx)y0 (x0+1*dx)y0 (x0+2*dx)y0 (x0+3*dx)y0 (x0+4*dx)y0 (x0+5*dx)y0 (x0+6*dx)y0 (x0+7*dx)y0 ]

                __m256 R2  = _mm256_add_ps(X2, Y2);
                // []
                __m256 cmp = _mm256_cmp_ps(R2, R2max, _CMP_LE_OQ);
                // R2 <= R2max ? 0xffffffff : 0 (by element)

                int mask   = _mm256_movemask_ps(cmp);
                // bit mask
                // ex.: cmp = [0xffffffff 0 0xffffffff 0 ... ]
                // mask = 0b10101010

                if (!mask) {
                    break;
                }

                __m256i ones = _mm256_set1_epi32(1);
                // [1 1 1 1 1 1 1 1]
                iterations   = _mm256_add_epi32(iterations, _mm256_and_si256(_mm256_castps_si256(cmp), ones));
                // int32_t + int32_t
                // _mm256_castps_si256 : __m256 (float) to __m256i (int)
                // _mm256_and_si256    : bitwise and between cmp and ones

                X = _mm256_add_ps(_mm256_sub_ps(X2, Y2), X0);
                // X = X^2 - Y^2 + X0
                Y = _mm256_add_ps(_mm256_add_ps(XY, XY), Y0);
                // Y = 2XY + Y0
            }

            int iter[8] = {};
            _mm256_storeu_si256((__m256i*)iter, iterations);
            // save 256 bit (8 * int32_t) from avx into memory (to iter)
            // storeu <=> unaligned (no alignment)

            for (size_t i = 0; i < 8 && x + i < WIDTH; i++) {
                generateColor(points, x + i, y, (float)iter[i]);
            }
        }
    }

    return NO_ERRORS;
}

renderError handleKeyboard(const std::optional<sf::Event> event, float *xShift, float *yShift, float *scale) {
    customAssert(event.has_value(), NULL_PTR);
    customAssert(xShift != NULL,    NULL_PTR);
    customAssert(yShift != NULL,    NULL_PTR);
    customAssert(scale  != NULL,    NULL_PTR);

    if (event->is<sf::Event::KeyPressed>()) {
        switch (event->getIf<sf::Event::KeyPressed>()->code) {
            case sf::Keyboard::Key::A:
                *xShift -= LIN_SHIFT * (*scale);
                break;

            case sf::Keyboard::Key::D:
                *xShift += LIN_SHIFT * (*scale);
                break;

            case sf::Keyboard::Key::W:
                *yShift -= LIN_SHIFT * (*scale);
                break;

            case sf::Keyboard::Key::S:
                *yShift += LIN_SHIFT * (*scale);
                break;

            case sf::Keyboard::Key::Up:
                *scale -= SCALE_SHIFT * (*scale);
                break;

            case sf::Keyboard::Key::Down:
                *scale += SCALE_SHIFT * (*scale);
                break;

            default:
                break;
        }
    }

    return NO_ERRORS;
}

renderError drawMandelbrotSet(sf::RenderWindow *window, uint8_t *points) {
    customAssert(window!= NULL, NULL_PTR);
    customAssert(points!= NULL, NULL_PTR);

    sf::Texture texture(sf::Vector2u(WIDTH, HEIGHT));

    texture.setSmooth(false);
    texture.update(points);

    sf::Sprite sprite(texture);
    window->draw(sprite);

    window->display();

    free(points);

    return NO_ERRORS;
}

int main() {
    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Mandelbrot Set");

    float xShift  = -0.8;
    float yShift  = 0.0;
    float scale   = 0.5;

    // uint64_t time = 0;

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            handleKeyboard(event, &xShift, &yShift, &scale);
        }

        window.clear();

        // uint64_t start = __rdtsc();

        uint8_t *points = (uint8_t *)calloc(WIDTH * HEIGHT * 4, sizeof(uint8_t));
        calculateMandelbrot(&window, points, xShift, yShift, scale);

        // uint64_t end = __rdtsc();

        drawMandelbrotSet(&window, points);
    }

    return 0;
}


#include <SFML/Graphics.hpp>
#include <immintrin.h>
#include <math.h>

#include <thread>
#include <vector>

#include "avxMandelbrot.h"
#include "customWarning.h"

//////////////////////////////////// COLORS ////////////////////////////////////////////

static colorTheme   COLOR_THEME[]     = {theme_0, theme_1, theme_2};
static const size_t MAX_COLOR_THEME   = sizeof(COLOR_THEME) / sizeof(COLOR_THEME[0]);

static size_t       COLOR_THEME_INDEX = 0;
static colorTheme   setTheme          = COLOR_THEME[COLOR_THEME_INDEX];

////////////////////////////////////////////////////////////////////////////////////////

void calculateMandelbrotSection(uint8_t *points, const float xShift, const float yShift, const float scale, size_t startY, size_t endY) {
    const float dx = MAX_RADIUS / WIDTH;
    static const __m256 R2max  = _mm256_set1_ps(MAX_RADIUS);
    // [MAX_RADIUS MAX_RADIUS MAX_RADIUS MAX_RADIUS MAX_RADIUS MAX_RADIUS MAX_RADIUS MAX_RADIUS]
    static const __m256 POINTS = _mm256_set_ps(7.f, 6.f, 5.f, 4.f, 3.f, 2.f, 1.f, 0.f);
    // [7.f 6.f 5.f 4.f 3.f 2.f 1.f 0.f]

    // x, y - pixels, x / WIDTH, y / HEIGHT -> [0, 1]
    // mandelbrot center - 0.0, -> [-0.5, 0.5] for symmetry
    // [-0.5, 0.5] -> [-0.5 * MAX_RADIUS, 0.5 * MAX_RADIUS] then scale
    // shift image center

    for (size_t y = startY; y < endY; y++) {
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
                __m256 X2 = _mm256_mul_ps(X, X);
                // [(x0+0*dx)^2 (x0+1*dx)^2 (x0+2*dx)^2 (x0+3*dx)^2 (x0+4*dx)^2 (x0+5*dx)^2 (x0+6*dx)^2 (x0+7*dx)^2]
                __m256 Y2 = _mm256_mul_ps(Y, Y);
                // [y0^2 y0^2 y0^2 y0^2 y0^2 y0^2 y0^2 y0^2]
                __m256 XY = _mm256_mul_ps(X, Y);
                // [(x0+0*dx)y0 (x0+1*dx)y0 (x0+2*dx)y0 (x0+3*dx)y0 (x0+4*dx)y0 (x0+5*dx)y0 (x0+6*dx)y0 (x0+7*dx)y0]

                __m256 R2  = _mm256_add_ps(X2, Y2);
                __m256 cmp = _mm256_cmp_ps(R2, R2max, _CMP_LE_OQ);
                // R2 <= R2max ? 0xffffffff : 0 (by element)

                int mask = _mm256_movemask_ps(cmp);
                // bit mask
                // ex. cmp = [0xffffffff 0 0xffffffff 0 ...]
                // mask = 0b10101010

                if (!mask) {
                    break;
                }

                __m256i ones = _mm256_set1_epi32(1);
                // [1 1 1 1 1 1 1 1]
                iterations = _mm256_add_epi32(iterations, _mm256_and_si256(_mm256_castps_si256(cmp), ones));
                // int32_t + int32_t
                // _mm256_castps_si256 : __m256 (float) to __m256i (int)
                // _mm256_and_si256    : bitwise and between cmp and ones

                X = _mm256_add_ps(_mm256_sub_ps(X2, Y2), X0);
                // X = X^2 - Y^2 + X0
                Y = _mm256_add_ps(_mm256_add_ps(XY, XY), Y0);
                // Y = 2XY + Y0

                // prefetch next data
                _mm_prefetch((const char*)&points[(y * WIDTH + x + 8) * 4], _MM_HINT_T0);
            }

            int iter[8] = {};
            _mm256_storeu_si256((__m256i*)iter, iterations);
            // save 256 bit (8 * int32_t) from avx into memory (to iter)
            // storeu <=> unaligned (no alignment)

            for (size_t i = 0; i < 8 && x + i < WIDTH; i++) {
                setTheme(points, x + i, y, (float) iter[i]);
            }
        }
    }
}

renderError calculateMandelbrot(sf::RenderWindow *window, uint8_t *points, const float xShift, const float yShift, const float scale) {
    customAssert(window != NULL, NULL_PTR);
    customAssert(points != NULL, NULL_PTR);

    const size_t numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    size_t rowsPerThread = HEIGHT / numThreads;

    // line-by-line processing
    for (size_t i = 0; i < numThreads; i++) {
        size_t startY = i * rowsPerThread;
        size_t endY   = (i == numThreads - 1) ? HEIGHT : startY + rowsPerThread;

        threads.emplace_back(calculateMandelbrotSection, points, xShift, yShift, scale, startY, endY);
    }

    for (auto &thread : threads) {
        thread.join();
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

            case sf::Keyboard::Key::T:
                COLOR_THEME_INDEX++;

                setTheme = COLOR_THEME[COLOR_THEME_INDEX % MAX_COLOR_THEME];
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

    texture.setSmooth(true);
    texture.update   (points);

    sf::Sprite sprite(texture);
    window->draw     (sprite);

    window->display();

    free(points);

    return NO_ERRORS;
}

int main() {
    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Mandelbrot Set");

    float xShift  = -0.8;
    float yShift  = 0.0;
    float scale   = 0.5;

    uint64_t time = 0;
    size_t run    = 0;

    sf::Font font;

    if (!font.openFromFile("common/font.ttf")) {
        return -1;
    }

    sf::Text fpsText(font);

    fpsText.setCharacterSize(20);
    fpsText.setFillColor(sf::Color::Red);

    sf::Clock clock;

    float FPS = 0.0f;

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            handleKeyboard(event, &xShift, &yShift, &scale);
        }

        float deltaTime = clock.restart().asSeconds();
        FPS             = 1.0f / deltaTime;

        fpsText.setString("FPS: " + std::to_string(static_cast<int>(FPS)));

        window.clear();

        uint64_t start  = __rdtsc();

        uint8_t *points = (uint8_t *)calloc(WIDTH * HEIGHT * 4, sizeof(uint8_t));
        calculateMandelbrot(&window, points, xShift, yShift, scale);

        uint64_t end    = __rdtsc();

        time += end - start;
        run++;

        #ifdef _test
            if (run >= TEST) {
                window.close();
            }

        printf("%ld/%ld\n", run, TEST);

        #else
            drawMandelbrotSet(&window, points);
        #endif

        window.draw(fpsText);
        window.display();
    }

    #if _test
        printf("RUNS: %ld, TIME: %ld (ticks) , TIME PER RUN: %ld (ticks per run) \n", run, time, time / run);
    #endif

    return 0;
}


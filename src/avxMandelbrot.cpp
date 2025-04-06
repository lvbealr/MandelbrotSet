#include <SFML/Graphics.hpp>

#include <immintrin.h>
#include <math.h>
#include <stdbool.h>
#include <thread>
#include <vector>

#include "avxMandelbrot.h"
#include "customWarning.h"

//////////////////////////////////// COLORS ////////////////////////////////////////////

extern colorTheme   COLOR_THEME[];
extern colorTheme   setTheme;
extern const size_t MAX_COLOR_THEME;
extern       size_t COLOR_THEME_INDEX;

////////////////////////////////////////////////////////////////////////////////////////

static bool SHOW_FPS = true;

void calculateMandelbrotSection(uint8_t *points, const float xShift, const float yShift, const float scale, size_t startY, size_t endY) {
    const float dx = MAX_RADIUS / WIDTH;
    static const __m256 R2max  = _mm256_set1_ps(MAX_RADIUS);
    
    static const __m256 POINTS = _mm256_set_ps(7.f, 6.f, 5.f, 4.f, 3.f, 2.f, 1.f, 0.f);
    
    for (size_t y = startY; y < endY; y++) {
        float y0 = (y / (float) HEIGHT - 0.5f) * MAX_RADIUS * scale + yShift;

        for (size_t x = 0; x < WIDTH; x += 8) {
            float x0 = (x / (float) WIDTH - 0.5f) * MAX_RADIUS * scale + xShift;

            __m256 X0 = _mm256_add_ps(_mm256_set1_ps(x0), _mm256_mul_ps(POINTS, _mm256_set1_ps(dx * scale)));
            __m256 Y0 = _mm256_set1_ps(y0);

            __m256 X = X0;
            __m256 Y = Y0;

            __m256i iterations = _mm256_setzero_si256();

            for (size_t iteration = 0; iteration < ITER_MAX; iteration++) {
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

                _mm_prefetch((const char*)&points[(y * WIDTH + x + 8) * 4], _MM_HINT_T0);
            }

            int iter[8] = {};
            _mm256_storeu_si256((__m256i*)iter, iterations);

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

    for (size_t i = 0; i < numThreads; i++) {
        size_t startY = i * rowsPerThread;
        size_t endY   = (i == numThreads - 1) ? HEIGHT : startY + rowsPerThread;

        #ifdef ON_MULTITHREADING_
            threads.emplace_back(calculateMandelbrotSection, points, xShift, yShift, scale, startY, endY);
        #else
            calculateMandelbrotSection(points, xShift, yShift, scale, startY, endY);
        #endif
    }

    #ifdef ON_MULTITHREADING_
        for (auto &thread : threads) {
            thread.join();
        }
    #endif

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

            case sf::Keyboard::Key::F11:
                SHOW_FPS = !SHOW_FPS;

            default:
                break;
        }
    }

    return NO_ERRORS;
}

renderError drawMandelbrotSet(sf::RenderWindow *window, uint8_t *points, sf::Texture *texture) {
    customAssert(window!= NULL, NULL_PTR);
    customAssert(points!= NULL, NULL_PTR);

    texture->update   (points);

    sf::Sprite sprite(*texture);
    window->draw     (sprite);

    free(points);

    return NO_ERRORS;
}

int main() {
    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Mandelbrot Set");
    
    float xShift  = -0.8;
    float yShift  = 0.0;
    float scale   = 1.0;

    sf::Clock clock;

    uint64_t time = 0;
    size_t   run  = 0;

    sf::Font font;
    if (!font.openFromFile("common/font.ttf")) {
        return -1;
    }

    sf::Text fpsText(font);
    fpsText.setCharacterSize(24);
    fpsText.setFillColor(sf::Color::Green);

    float FPS = 0.0f;

    sf::Texture texture(sf::Vector2u(WIDTH, HEIGHT));
    texture.setSmooth(true);

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            handleKeyboard(event, &xShift, &yShift, &scale);
        }

        float deltaTime = clock.restart().asSeconds();
        FPS = 1.0f / deltaTime;
        fpsText.setString("FPS: " + std::to_string(static_cast<int>(FPS)));

        uint64_t start = __rdtsc();

        uint8_t *points = (uint8_t *)calloc(WIDTH * HEIGHT * 4, sizeof(uint8_t));
        calculateMandelbrot(&window, points, xShift, yShift, scale);

        uint64_t end = __rdtsc();

        time += end - start;
        run++;

        #ifdef ON_TEST_
            if (run >= TEST) {
                window.close();
            }
            printf("%ld/%ld\n", run, TEST);
        #else
            window.clear();

            drawMandelbrotSet(&window, points, &texture);

            if (SHOW_FPS) {
                window.draw(fpsText);
            }

            window.display();
        #endif
    }

    #if ON_TEST_
        printf("RUNS: %ld, TIME: %ld (ticks) , TIME PER RUN: %ld (ticks per run) \n", run, time, time / run);
    #endif

    return 0;
}


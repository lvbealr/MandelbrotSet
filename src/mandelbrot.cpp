#include <SFML/Graphics.hpp>

#include <immintrin.h>
#include <math.h>
#include <stdbool.h>
#include <thread>
#include <vector>

#include "customWarning.h"
#include "mandelbrot.h"
#include "colorTheme.h"
#include "calculateSet.h"

//////////////////////////////////// COLORS ////////////////////////////////////////////

extern const colorTheme   COLOR_THEME[];
extern const size_t       MAX_COLOR_THEME;

extern size_t     COLOR_THEME_INDEX;
extern colorTheme setTheme;

////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////// FUNCS ////////////////////////////////////////////

extern calculateSectionFunc CALCULATE_FUNCS[];
extern const size_t         MAX_CALCULATE_FUNC_INDEX;

extern size_t               CALCULATE_FUNC_INDEX;
extern calculateSectionFunc calculateSection;

////////////////////////////////////////////////////////////////////////////////////////

static bool SHOW_FPS = true;

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
            threads.emplace_back(calculateSection, points, xShift, yShift, scale, startY, endY);
        #else
            calculateSection(points, xShift, yShift, scale, startY, endY);
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

            case sf::Keyboard::Key::F:
                CALCULATE_FUNC_INDEX++;
                calculateSection = CALCULATE_FUNCS[CALCULATE_FUNC_INDEX % MAX_CALCULATE_FUNC_INDEX];
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
            printf("%ld/%ld %lu\n", run, TEST, end - start);

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
        // printf("RUNS: %ld, TIME: %ld (ticks) , TIME PER RUN: %ld (ticks per run) \n", run, time, time / run);
    #endif

    return 0;
}

#include <SFML/Graphics.hpp>

#include <immintrin.h>
#include <math.h>
#include <stdbool.h>
#include <thread>
#include <vector>

#include <cjson/cJSON.h>
#include <string.h>

#include "customWarning.h"

#include "calculateSet.h"
#include "mandelbrot.h"
#include "render.h"
#include "setContext.h"

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

renderError calculateMandelbrot(renderContext *context, sf::RenderWindow *window, uint8_t *points, const float xShift, const float yShift, const float scale) {
    customAssert(window != NULL, INVALID_POINTER);
    customAssert(points != NULL, INVALID_POINTER);

    unsigned height = context->window->height;

    const size_t numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    size_t rowsPerThread = height / numThreads;

    for (size_t i = 0; i < numThreads; i++) {
        size_t startY = i * rowsPerThread;
        size_t endY   = (i == numThreads - 1) ? height : startY + rowsPerThread;

        #ifdef ON_MULTITHREADING_
            threads.emplace_back(calculateSection, context, points, xShift, yShift, scale, startY, endY);
        #else
            calculateSection(context, points, xShift, yShift, scale, startY, endY);
        #endif
    }

    #ifdef ON_MULTITHREADING_
        for (auto &thread : threads) {
            thread.join();
        }
    #endif

    return NO_ERRORS;
}

renderError handleKeyboard(renderContext *context, const std::optional<sf::Event> event, float *xShift, float *yShift, float *scale) {
    customAssert(event.has_value(), INVALID_POINTER);
    customAssert(xShift != NULL,    INVALID_POINTER);
    customAssert(yShift != NULL,    INVALID_POINTER);
    customAssert(scale  != NULL,    INVALID_POINTER);

    float linearShift = context->linearShift;
    float scaleShift  = context->scaleShift;

    bool *showFPS     = &(context->showFPS);

    if (event->is<sf::Event::KeyPressed>()) {
        switch (event->getIf<sf::Event::KeyPressed>()->code) {
            case sf::Keyboard::Key::A:
                *xShift -= linearShift * (*scale);
                break;

            case sf::Keyboard::Key::D:
                *xShift += linearShift * (*scale);
                break;

            case sf::Keyboard::Key::W:
                *yShift -= linearShift * (*scale);
                break;

            case sf::Keyboard::Key::S:
                *yShift += linearShift * (*scale);
                break;

            case sf::Keyboard::Key::Up:
                *scale -= scaleShift * (*scale);
                break;

            case sf::Keyboard::Key::Down:
                *scale += scaleShift * (*scale);
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
                (*showFPS) = !(*showFPS);

            default:
                break;
        }
    }

    return NO_ERRORS;
}

renderError drawMandelbrotSet(renderContext *context, sf::RenderWindow *window, uint8_t *points, sf::Texture *texture) {
    customAssert(window!= NULL, INVALID_POINTER);
    customAssert(points!= NULL, INVALID_POINTER);

    texture->update   (points);

    sf::Sprite sprite(*texture);
    window->draw     (sprite);

    FREE_(points);

    return NO_ERRORS;
}

int main() {
    renderContext *context        = createRenderContext();
    renderWindow  *contextWindow  = context->window;

    sf::RenderWindow window(sf::VideoMode({contextWindow->width, contextWindow->height}), contextWindow->windowName);
    
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

    sf::Texture texture(sf::Vector2u(contextWindow->width, contextWindow->height));
    texture.setSmooth(true);

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                FREE_(context);
                window.close();
            }

            handleKeyboard(context, event, &xShift, &yShift, &scale);
        }

        float deltaTime = clock.restart().asSeconds();
        FPS = 1.0f / deltaTime;
        fpsText.setString("FPS: " + std::to_string(static_cast<int>(FPS)));

        uint64_t start = __rdtsc();

        uint8_t *points = (uint8_t *)calloc(contextWindow->width * contextWindow->height * 4, sizeof(uint8_t));
        calculateMandelbrot(context, &window, points, xShift, yShift, scale);

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

            drawMandelbrotSet(context, &window, points, &texture);

            if (context->showFPS) {
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

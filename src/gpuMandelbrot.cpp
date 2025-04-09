#include <SFML/Graphics.hpp>

#include "customWarning.h"
#include "mandelbrot.h"

static size_t COLOR_THEME_INDEX = 0;

renderError handleKeyboard(renderContext *context, const std::optional<sf::Event> event, float *xShift, float *yShift, float *scale) {
    customAssert(event.has_value(), INVALID_POINTER);
    customAssert(xShift != NULL,    INVALID_POINTER);
    customAssert(yShift != NULL,    INVALID_POINTER);
    customAssert(scale  != NULL,    INVALID_POINTER);

    float linearShift = context->linearShift;
    float scaleShift  = context->scaleShift;

    bool *showFPS     = &context->showFPS;

    if (event->is<sf::Event::KeyPressed>()) {
        switch (event->getIf<sf::Event::KeyPressed>()->code) {
            case sf::Keyboard::Key::A:
                *xShift -= linearShift * (*scale);
                break;

            case sf::Keyboard::Key::D:
                *xShift += linearShift * (*scale);
                break;

            case sf::Keyboard::Key::W:
                *yShift += linearShift * (*scale);
                break;

            case sf::Keyboard::Key::S:
                *yShift -= linearShift * (*scale);
                break;

            case sf::Keyboard::Key::Up:
                *scale -= scaleShift * (*scale);
                break;

            case sf::Keyboard::Key::Down:
                *scale += scaleShift * (*scale);
                break;

            case sf::Keyboard::Key::T:
                COLOR_THEME_INDEX++;
                break;

            case sf::Keyboard::Key::F11:
                (*showFPS) = !(*showFPS);

            default:
                break;
        }
    }

    return NO_ERRORS;
}

int main() {
    renderContext *context = createRenderContext();
    renderWindow  *contextWindow = context->window;

    sf::RenderWindow window(sf::VideoMode({contextWindow->width,
                                                    contextWindow->height}),
                                                       contextWindow->windowName);
    
    float xShift = 0.0;
    float yShift = 0.0;
    float scale  = 1.0;

    sf::Clock clock;
    sf::Clock fpsClock;

    sf::Shader shader;
    if (!shader.loadFromFile("shaders/mandelbrot.frag", sf::Shader::Type::Fragment)) {
        return 1;
    }

    sf::RectangleShape fullscreenRect(sf::Vector2f({(float) contextWindow->width, (float) contextWindow->height}));
    fullscreenRect.setPosition({0, 0});

    sf::Font font;
    if (!font.openFromFile("common/font.ttf")) {
        return 1;
    }

    sf::Text fpsText(font);
    fpsText.setCharacterSize(24);
    fpsText.setFillColor(sf::Color::Green);

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            handleKeyboard(context, event, &xShift, &yShift, &scale);
        }

        float deltaTime = fpsClock.restart().asSeconds();
        float FPS = (deltaTime > 0) ? 1.0f / deltaTime : 0.0f;

        shader.setUniform("time",   clock.getElapsedTime().asSeconds());
        shader.setUniform("width",  (int) contextWindow->width);
        shader.setUniform("height", (int) contextWindow->height);
        shader.setUniform("offset", sf::Vector2f({xShift, yShift}));
        shader.setUniform("scale",  scale);
        shader.setUniform("theme",  (int) COLOR_THEME_INDEX);

        window.clear();
        window.draw(fullscreenRect, &shader);

        if (context->showFPS) {
            fpsText.setString("FPS: " + std::to_string(static_cast<int>(FPS)));
            window.draw(fpsText);
        }

        window.display();
    }

    return 0;
}
#include <SFML/Graphics.hpp>

#include "customWarning.h"
#include "mandelbrot.h"

static size_t COLOR_THEME_INDEX = 0;

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
                *yShift += LIN_SHIFT * (*scale);
                break;

            case sf::Keyboard::Key::S:
                *yShift -= LIN_SHIFT * (*scale);
                break;

            case sf::Keyboard::Key::Up:
                *scale -= SCALE_SHIFT * (*scale);
                break;

            case sf::Keyboard::Key::Down:
                *scale += SCALE_SHIFT * (*scale);
                break;

            case sf::Keyboard::Key::T:
                COLOR_THEME_INDEX++;
                break;

            default:
                break;
        }
    }

    return NO_ERRORS;
}

int main() {
    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Mandelbrot Set");
    
    float xShift  = 0.0;
    float yShift  = 0.0;
    float scale   = 1.0;

    sf::Clock clock;

    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);

    sf::Shader shader;
    if (!shader.loadFromFile("shaders/mandelbrot.frag", sf::Shader::Type::Fragment)) {
        return 1;
    }

    sf::RectangleShape fullscreenRect(sf::Vector2f({WIDTH, HEIGHT}));
    fullscreenRect.setPosition({0, 0});

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            handleKeyboard(event, &xShift, &yShift, &scale);
        }

        shader.setUniform("time",   clock.getElapsedTime().asSeconds());
        shader.setUniform("width",  (int) WIDTH);
        shader.setUniform("height", (int) HEIGHT);
        shader.setUniform("offset", sf::Vector2f({xShift, yShift}));
        shader.setUniform("scale",  scale);
        shader.setUniform("theme",  (int) COLOR_THEME_INDEX);

        window.clear();

        window.draw(fullscreenRect, &shader);
        window.display();
    }

    return 0;
}
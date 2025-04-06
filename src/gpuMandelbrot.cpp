#include <SFML/Graphics.hpp>

#include "customWarning.h"
#include "mandelbrot.h"

static size_t COLOR_THEME_INDEX = 0;

static bool SHOW_FPS = true;

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

            case sf::Keyboard::Key::F11:
                SHOW_FPS = !SHOW_FPS;

            default:
                break;
        }
    }

    return NO_ERRORS;
}

int main() {
    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Mandelbrot Set");
    
    float xShift = 0.0;
    float yShift = 0.0;
    float scale  = 1.0;

    sf::Clock clock;
    sf::Clock fpsClock;

    sf::Shader shader;
    if (!shader.loadFromFile("shaders/mandelbrot.frag", sf::Shader::Type::Fragment)) {
        return 1;
    }

    sf::RectangleShape fullscreenRect(sf::Vector2f({WIDTH, HEIGHT}));
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
            handleKeyboard(event, &xShift, &yShift, &scale);
        }

        float deltaTime = fpsClock.restart().asSeconds();
        float FPS = (deltaTime > 0) ? 1.0f / deltaTime : 0.0f;

        shader.setUniform("time",   clock.getElapsedTime().asSeconds());
        shader.setUniform("width",  (int) WIDTH);
        shader.setUniform("height", (int) HEIGHT);
        shader.setUniform("offset", sf::Vector2f({xShift, yShift}));
        shader.setUniform("scale",  scale);
        shader.setUniform("theme",  (int) COLOR_THEME_INDEX);

        window.clear();
        window.draw(fullscreenRect, &shader);

        if (SHOW_FPS) {
            fpsText.setString("FPS: " + std::to_string(static_cast<int>(FPS)));
            window.draw(fpsText);
        }

        window.display();
    }

    return 0;
}
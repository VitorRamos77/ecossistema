#ifndef COLORUTILS_HPP
#define COLORUTILS_HPP

#include <SFML/Graphics/Color.hpp>

// Interpola cor de vermelho (energia baixa) a verde (energia alta)
inline sf::Color energiaParaCor(float energiaAtual, float energiaMaxima) {
    float t = energiaAtual / energiaMaxima;
    if (t < 0.f) t = 0.f;
    if (t > 1.f) t = 1.f;
    sf::Uint8 r = static_cast<sf::Uint8>((1.f - t) * 220);
    sf::Uint8 g = static_cast<sf::Uint8>(t * 200);
    sf::Uint8 b = 30;
    return sf::Color(r, g, b);
}

#endif // COLORUTILS_HPP

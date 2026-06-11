#include "Planta.hpp"
#include "../core/WorldManager.hpp"

#ifdef USE_SFML
#include <SFML/Graphics.hpp>
#else
#include <iostream>
#endif

Planta::Planta(float x, float y) : Organismo(x, y, ENERGIA_INICIAL, 0), contadorEspalhamento(0) {}

void Planta::agir(WorldManager& /*world*/) {
    contadorEspalhamento++;
    if (idade >= IDADE_MAXIMA)
        consumirEnergia(energia); // morte natural por envelhecimento
}

void Planta::mover(WorldManager& /*world*/) {
    // Não se move
}

#ifdef USE_SFML
void Planta::desenhar(sf::RenderWindow& window, float cellSize) const {
    sf::RectangleShape shape(sf::Vector2f(cellSize - 1.f, cellSize - 1.f));
    shape.setPosition(x * cellSize, y * cellSize);
    shape.setFillColor(sf::Color(34, 139, 34));
    window.draw(shape);
}
#else
void Planta::desenhar() const {
    std::cout << 'P';
}
#endif

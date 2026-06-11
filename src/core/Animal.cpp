#include "Animal.hpp"
#include "WorldManager.hpp"

Animal::Animal(float x, float y, float energiaInicial, int cooldownReproducao,
               float velocidade, float raioVisao)
    : Organismo(x, y, energiaInicial, cooldownReproducao),
      velocidade(velocidade), raioVisao(raioVisao), vx(0.f), vy(0.f) {}

std::vector<Organismo*> Animal::detectarVizinhos(WorldManager& world) const {
    return world.obterVizinhos(x, y, raioVisao, this);
}

void Animal::aplicarForca(Vec2 forca, float maxSpeed,
                           float larguraMundo, float alturaMundo) {
    vx += forca.x;
    vy += forca.y;

    float speed = Vec2{vx, vy}.length();
    if (speed > maxSpeed) {
        vx = (vx / speed) * maxSpeed;
        vy = (vy / speed) * maxSpeed;
    }

    x += vx;
    y += vy;

    if (x < 0.f)              x = 0.f;
    if (x > larguraMundo - 1.f) x = larguraMundo - 1.f;
    if (y < 0.f)              y = 0.f;
    if (y > alturaMundo  - 1.f) y = alturaMundo  - 1.f;
}

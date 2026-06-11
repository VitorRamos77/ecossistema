#ifndef ANIMAL_HPP
#define ANIMAL_HPP

#include "Organismo.hpp"
#include "SteeringBehaviors.hpp"
#include <vector>

enum class EstadoAgente {
    WANDERING,  // sem alvo visível
    HUNTING,    // predador perseguindo presa   (só Predador usa)
    FORAGING,   // presa buscando planta        (só Presa usa)
    FLEEING     // presa fugindo de predador    (só Presa usa)
};

class Animal : public Organismo {
protected:
    float velocidade;
    float raioVisao;
    float vx, vy;
    EstadoAgente estado    = EstadoAgente::WANDERING;
    float        wanderAngle = 0.f;

public:
    Animal(float x, float y, float energiaInicial, int cooldownReproducao,
           float velocidade, float raioVisao);

    virtual std::vector<Organismo*> detectarVizinhos(WorldManager& world) const;

    void aplicarForca(Vec2 forca, float maxSpeed,
                      float larguraMundo, float alturaMundo);

    float getVx() const { return vx; }
    float getVy() const { return vy; }
};

#endif // ANIMAL_HPP

#include "Predador.hpp"
#include "../core/WorldManager.hpp"
#include "../core/SteeringBehaviors.hpp"
#include <cmath>
#include <limits>

#ifdef USE_SFML
#include "../core/ColorUtils.hpp"
#include <SFML/Graphics.hpp>
#else
#include <iostream>
#endif

Predador::Predador(float x, float y)
    : Animal(x, y, ENERGIA_INICIAL, COOLDOWN_REPRODUCAO, VELOCIDADE, RAIO_VISAO) {}

void Predador::agir(WorldManager& world) {
    // Validação cross-tick: alvoAtual pode ser ponteiro dangling após erase do tick
    // anterior; o endereço também pode ter sido reutilizado por outro tipo de organismo
    // (o static_cast<Animal*> em mover() exige que continue sendo uma Presa)
    if (alvoAtual && (!world.organismoExisteEVivo(alvoAtual) ||
                      alvoAtual->getTipo() != TipoOrganismo::Presa)) {
        alvoAtual = nullptr;
        estado    = EstadoAgente::WANDERING;
    }

    float raioEfetivo  = raioVisao * world.params.raioVisaoPredador;
    float raioEfetivo2 = raioEfetivo * raioEfetivo;

    // Abandona alvo se saiu do raio de visão (sem sqrt)
    if (alvoAtual) {
        float dx = alvoAtual->getX() - x;
        float dy = alvoAtual->getY() - y;
        if (dx*dx + dy*dy > raioEfetivo2) {
            alvoAtual = nullptr;
            estado    = EstadoAgente::WANDERING;
        }
    }

    auto presas = world.obterVizinhosDeTipo(x, y, raioEfetivo, TipoOrganismo::Presa, this);

    switch (estado) {
        case EstadoAgente::WANDERING:
            if (!presas.empty()) {
                estado = EstadoAgente::HUNTING;
                float melhor = -1.f;
                alvoAtual    = nullptr;
                bool faminto = energia < ENERGIA_INICIAL * 0.4f;
                for (auto* p : presas) {
                    float pdx  = p->getX() - x, pdy = p->getY() - y;
                    float dist2 = pdx*pdx + pdy*pdy;
                    float prio;
                    if (faminto) {
                        prio = 1.f / (dist2 + 0.001f); // atacar a mais próxima
                    } else {
                        float dist  = std::sqrt(dist2);
                        Vec2 dirA   = Vec2{vx, vy}.normalized();
                        float dot   = (dirA.x * pdx + dirA.y * pdy) / (dist + 0.001f);
                        prio = (1.f / (dist + 0.001f)) * p->getEnergia() * (0.5f + 0.5f * dot);
                    }
                    if (prio > melhor) { melhor = prio; alvoAtual = p; }
                }
            }
            break;
        case EstadoAgente::HUNTING:
            if (presas.empty() || alvoAtual == nullptr) {
                alvoAtual = nullptr;
                estado    = EstadoAgente::WANDERING;
            }
            break;
        default: break;
    }

    switch (estado) {
        case EstadoAgente::WANDERING: consumirEnergia(CUSTO_PARADO);                          break;
        case EstadoAgente::HUNTING:   consumirEnergia(CUSTO_MOVIMENTO + CUSTO_EXTRA_HUNTING); break;
        default: break;
    }

    if (idade > IDADE_MAXIMA * 3 / 4)
        consumirEnergia(0.5f);
}

void Predador::mover(WorldManager& world) {
    if (!estaVivo()) return;

    Vec2 pos{x, y};
    Vec2 vel{vx, vy};
    Vec2 forca{0.f, 0.f};

    switch (estado) {
        case EstadoAgente::WANDERING:
            forca = Steering::wander(pos, vel, wanderAngle,
                WANDER_RADIUS, WANDER_DISTANCE, WANDER_JITTER, world.getRng());
            break;
        case EstadoAgente::HUNTING:
            // alvoAtual já foi validado em agir() neste mesmo tick (erase não rodou ainda)
            if (alvoAtual && alvoAtual->estaVivo()) {
                auto* a = static_cast<Animal*>(alvoAtual);
                forca   = Steering::pursue(pos,
                    {alvoAtual->getX(), alvoAtual->getY()},
                    {a->getVx(), a->getVy()},
                    VELOCIDADE);
            } else {
                forca = Steering::wander(pos, vel, wanderAngle,
                    WANDER_RADIUS, WANDER_DISTANCE, WANDER_JITTER, world.getRng());
            }
            break;
        default: break;
    }

    float maxSpeedEfetivo = VELOCIDADE * world.params.velocidadePredador;
    aplicarForca(forca, maxSpeedEfetivo,
        static_cast<float>(world.getLargura()),
        static_cast<float>(world.getAltura()));
}

#ifdef USE_SFML
void Predador::desenhar(sf::RenderWindow& window, float cellSize) const {
    sf::ConvexShape shape(3);
    float s = cellSize * 0.45f;
    shape.setPoint(0, sf::Vector2f( s,        0.f));
    shape.setPoint(1, sf::Vector2f(-s * 0.6f,  s * 0.7f));
    shape.setPoint(2, sf::Vector2f(-s * 0.6f, -s * 0.7f));
    float angulo = std::atan2(vy, vx) * 180.f / 3.14159265f;
    shape.setRotation(angulo);
    shape.setPosition(x * cellSize + cellSize * 0.5f,
                      y * cellSize + cellSize * 0.5f);
    shape.setFillColor(energiaParaCor(energia, ENERGIA_INICIAL * 2.f));
    window.draw(shape);
}
#else
void Predador::desenhar() const {
    std::cout << 'L';
}
#endif

#include "Presa.hpp"
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

Presa::Presa(float x, float y)
    : Animal(x, y, ENERGIA_INICIAL, COOLDOWN_REPRODUCAO, VELOCIDADE, RAIO_VISAO) {}

void Presa::agir(WorldManager& world) {
    // Validação cross-tick: alvoPlanta pode ser ponteiro dangling após erase do tick
    // anterior; o endereço também pode ter sido reutilizado por outro tipo de organismo
    if (alvoPlanta && (!world.organismoExisteEVivo(alvoPlanta) ||
                       alvoPlanta->getTipo() != TipoOrganismo::Planta))
        alvoPlanta = nullptr;

    float raioEfetivo = raioVisao * world.params.raioVisaoPresa;

    // Busca combinada: predadores + plantas em uma única varredura da grade
    vizPredadores.clear();
    vizPlantas.clear();
    world.obterVizinhos2Tipos(x, y, raioEfetivo,
                               TipoOrganismo::Predador, TipoOrganismo::Planta,
                               this, vizPredadores, vizPlantas);

    bool predadorProximo = !vizPredadores.empty();
    bool plantaProxima   = !vizPlantas.empty();

    switch (estado) {
        case EstadoAgente::WANDERING:
            if (predadorProximo) { estado = EstadoAgente::FLEEING; ticksSemPredador = 0; }
            else if (plantaProxima) { estado = EstadoAgente::FORAGING; alvoPlanta = nullptr; }
            break;
        case EstadoAgente::FORAGING:
            if (predadorProximo) { estado = EstadoAgente::FLEEING; ticksSemPredador = 0; }
            // só desiste se não há planta no raio E não tem alvo ativo — evita abandonar a perseguição
            else if (!plantaProxima && (alvoPlanta == nullptr || !alvoPlanta->estaVivo())) {
                estado = EstadoAgente::WANDERING;
                alvoPlanta = nullptr;
            }
            break;
        case EstadoAgente::FLEEING:
            if (!predadorProximo) {
                if (++ticksSemPredador >= 3) estado = EstadoAgente::WANDERING;
            } else {
                ticksSemPredador = 0;
            }
            break;
        default: break;
    }

    switch (estado) {
        case EstadoAgente::WANDERING:
        case EstadoAgente::FORAGING:  consumirEnergia(CUSTO_MOVIMENTO);          break;
        case EstadoAgente::FLEEING:   consumirEnergia(CUSTO_MOVIMENTO * 1.5f);   break;
        default: break;
    }

    // Senescência: animais velhos consomem mais energia
    if (idade > IDADE_MAXIMA * 3 / 4)
        consumirEnergia(0.5f);
}

void Presa::mover(WorldManager& world) {
    if (!estaVivo()) return;

    Vec2 pos{x, y};
    Vec2 vel{vx, vy};
    Vec2 forca{0.f, 0.f};
    float maxSpeed = VELOCIDADE;

    switch (estado) {
        case EstadoAgente::WANDERING: {
            Vec2 wanderForce = Steering::wander(pos, vel, wanderAngle,
                WANDER_RADIUS, WANDER_DISTANCE, WANDER_JITTER, world.getRng());
            auto vizinhos = world.obterVizinhosDeTipo(x, y, raioVisao, TipoOrganismo::Presa, this);
            if (!vizinhos.empty()) {
                std::vector<Vec2> posViz, velViz;
                posViz.reserve(vizinhos.size());
                velViz.reserve(vizinhos.size());
                for (auto* v : vizinhos) {
                    posViz.push_back({v->getX(), v->getY()});
                    velViz.push_back({static_cast<Animal*>(v)->getVx(),
                                      static_cast<Animal*>(v)->getVy()});
                }
                auto flock = Steering::calcularFlocking(pos, vel, posViz, velViz, RAIO_SEPARACAO);
                forca = wanderForce
                      + flock.separacao   * PESO_SEPARACAO
                      + flock.alinhamento * PESO_ALINHAMENTO
                      + flock.coesao      * PESO_COESAO;
            } else {
                forca = wanderForce;
            }
            break;
        }
        case EstadoAgente::FORAGING: {
            // alvoPlanta já foi validado em agir() neste mesmo tick (erase não rodou ainda);
            // vizPlantas foi preenchido em agir() — reutiliza em vez de varrer a grade de novo
            if (!alvoPlanta || !alvoPlanta->estaVivo()) {
                alvoPlanta = nullptr;
                float minDist2 = std::numeric_limits<float>::max();
                for (auto* p : vizPlantas) {
                    if (!p->estaVivo()) continue;
                    float dx = p->getX() - x, dy = p->getY() - y;
                    float d2 = dx*dx + dy*dy;
                    if (d2 < minDist2) { minDist2 = d2; alvoPlanta = p; }
                }
            }
            if (alvoPlanta) {
                forca = Steering::seek(pos, {alvoPlanta->getX(), alvoPlanta->getY()}, VELOCIDADE);
            } else {
                forca = Steering::wander(pos, vel, wanderAngle,
                    WANDER_RADIUS, WANDER_DISTANCE, WANDER_JITTER, world.getRng());
            }
            // Separação leve para presas não se amontoarem na mesma planta
            auto flockViz = world.obterVizinhosDeTipo(x, y, RAIO_SEPARACAO * 2.f, TipoOrganismo::Presa, this);
            if (!flockViz.empty()) {
                std::vector<Vec2> posViz, velViz;
                posViz.reserve(flockViz.size());
                velViz.reserve(flockViz.size());
                for (auto* v : flockViz) {
                    posViz.push_back({v->getX(), v->getY()});
                    velViz.push_back({static_cast<Animal*>(v)->getVx(),
                                      static_cast<Animal*>(v)->getVy()});
                }
                auto flock = Steering::calcularFlocking(pos, vel, posViz, velViz, RAIO_SEPARACAO);
                forca = forca + flock.separacao * PESO_SEPARACAO * 0.2f;
            }
            break;
        }
        case EstadoAgente::FLEEING: {
            // vizPredadores foi preenchido em agir() neste mesmo tick
            float minDist2 = std::numeric_limits<float>::max();
            Organismo* closest = nullptr;
            for (auto* p : vizPredadores) {
                if (!p->estaVivo()) continue;
                float dx = p->getX() - x, dy = p->getY() - y;
                float d2 = dx*dx + dy*dy;
                if (d2 < minDist2) { minDist2 = d2; closest = p; }
            }
            if (closest) {
                auto* anim = static_cast<Animal*>(closest);
                forca    = Steering::evade(pos,
                    {closest->getX(), closest->getY()},
                    {anim->getVx(),   anim->getVy()},
                    VELOCIDADE);
                maxSpeed = VELOCIDADE * 1.5f;
            }
            // Fuga em grupo: alinhar com companheiros próximos (efeito manada)
            auto flockViz = world.obterVizinhosDeTipo(x, y, RAIO_SEPARACAO * 2.f, TipoOrganismo::Presa, this);
            if (!flockViz.empty()) {
                std::vector<Vec2> posViz, velViz;
                posViz.reserve(flockViz.size());
                velViz.reserve(flockViz.size());
                for (auto* v : flockViz) {
                    posViz.push_back({v->getX(), v->getY()});
                    velViz.push_back({static_cast<Animal*>(v)->getVx(),
                                      static_cast<Animal*>(v)->getVy()});
                }
                auto flock = Steering::calcularFlocking(pos, vel, posViz, velViz, RAIO_SEPARACAO);
                forca = forca + flock.alinhamento * 0.8f + flock.separacao * PESO_SEPARACAO * 0.3f;
            }
            break;
        }
        default: break;
    }

    float maxSpeedEfetivo = maxSpeed * world.params.velocidadePresa;
    aplicarForca(forca, maxSpeedEfetivo,
        static_cast<float>(world.getLargura()),
        static_cast<float>(world.getAltura()));
}

#ifdef USE_SFML
void Presa::desenhar(sf::RenderWindow& window, float cellSize) const {
    sf::CircleShape shape(cellSize * 0.4f);
    shape.setPosition(x * cellSize + cellSize * 0.1f,
                      y * cellSize + cellSize * 0.1f);
    shape.setFillColor(energiaParaCor(energia, ENERGIA_INICIAL * 2.f));
    window.draw(shape);
}
#else
void Presa::desenhar() const {
    std::cout << 'C';
}
#endif

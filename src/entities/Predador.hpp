#ifndef PREDADOR_HPP
#define PREDADOR_HPP

#include "../core/Animal.hpp"

class Predador : public Animal {
public:
    // Ciclo de vida
    static constexpr int   IDADE_MAXIMA            = 1800; // triplo das presas (600)

    // Energia
    static constexpr float ENERGIA_INICIAL         = 50.0f;
    static constexpr float CUSTO_PARADO            = 1.0f;
    static constexpr float CUSTO_MOVIMENTO         = 2.0f;
    static constexpr float CUSTO_EXTRA_HUNTING     = 1.0f;
    static constexpr float ENERGIA_PARA_REPRODUCAO = 55.0f;
    static constexpr float CUSTO_REPRODUCAO        = 20.0f;
    static constexpr int   COOLDOWN_REPRODUCAO     = 30;

    // Steering
    static constexpr float WANDER_RADIUS   = 2.0f;
    static constexpr float WANDER_DISTANCE = 4.0f;
    static constexpr float WANDER_JITTER   = 0.2f;
    static constexpr float VELOCIDADE      = 0.8f;
    static constexpr float RAIO_VISAO      = 8.0f;
    static constexpr float RAIO_REPRODUCAO = 20.0f;

private:
    Organismo* alvoAtual = nullptr;

public:
    Predador(float x, float y);
    void agir(WorldManager& world) override;
    void mover(WorldManager& world) override;

#ifdef USE_SFML
    void desenhar(sf::RenderWindow& window, float cellSize) const override;
#else
    void desenhar() const override;
#endif

    TipoOrganismo getTipo()                const override { return TipoOrganismo::Predador; }
    char          getSimboloDesenho()      const override { return 'L'; }
    float getEnergiaParaReproducao()       const override { return ENERGIA_PARA_REPRODUCAO; }
    float getCustoReproducao()             const override { return CUSTO_REPRODUCAO; }
    int   getCooldownReproducao()          const override { return COOLDOWN_REPRODUCAO; }
    float getEnergiaFornecidaAoSerComido() const override { return energia; }
    float getRaioReproducao()              const override { return 0.f; } // reprodução assexuada
};

#endif // PREDADOR_HPP

#ifndef PRESA_HPP
#define PRESA_HPP

#include "../core/Animal.hpp"

class Presa : public Animal {
public:
    // Ciclo de vida
    static constexpr int   IDADE_MAXIMA            = 600;

    // Energia
    static constexpr float ENERGIA_INICIAL         = 30.0f;
    static constexpr float CUSTO_PARADO            = 0.5f;
    static constexpr float CUSTO_MOVIMENTO         = 1.0f;
    static constexpr float ENERGIA_AO_COMER_PLANTA = 10.0f;
    static constexpr float ENERGIA_PARA_REPRODUCAO = 40.0f;
    static constexpr float CUSTO_REPRODUCAO        = 20.0f;
    static constexpr int   COOLDOWN_REPRODUCAO     = 20;

    // Steering
    static constexpr float WANDER_RADIUS    = 1.5f;
    static constexpr float WANDER_DISTANCE  = 3.0f;
    static constexpr float WANDER_JITTER    = 0.3f;
    static constexpr float VELOCIDADE       = 0.6f;
    static constexpr float RAIO_VISAO       = 6.0f;
    static constexpr float RAIO_FUGA        = 5.0f;
    static constexpr float RAIO_REPRODUCAO  = 3.0f;

    // Flocking
    static constexpr float PESO_SEPARACAO   = 1.5f;
    static constexpr float PESO_ALINHAMENTO = 1.0f;
    static constexpr float PESO_COESAO      = 1.0f;
    static constexpr float RAIO_SEPARACAO   = 2.0f;

private:
    int        ticksSemPredador = 0;
    Organismo* alvoPlanta       = nullptr;
    // Cache preenchido em agir() e reutilizado em mover() no mesmo tick —
    // evita varrer a grade espacial duas vezes por presa
    std::vector<Organismo*> vizPredadores;
    std::vector<Organismo*> vizPlantas;

public:
    Presa(float x, float y);
    void agir(WorldManager& world) override;
    void mover(WorldManager& world) override;

#ifdef USE_SFML
    void desenhar(sf::RenderWindow& window, float cellSize) const override;
#else
    void desenhar() const override;
#endif

    TipoOrganismo getTipo()                const override { return TipoOrganismo::Presa; }
    char          getSimboloDesenho()      const override { return 'C'; }
    float getEnergiaParaReproducao()       const override { return ENERGIA_PARA_REPRODUCAO; }
    float getCustoReproducao()             const override { return CUSTO_REPRODUCAO; }
    int   getCooldownReproducao()          const override { return COOLDOWN_REPRODUCAO; }
    float getEnergiaFornecidaAoSerComido() const override { return energia; }
    float getRaioReproducao()              const override { return 0.f; } // reprodução assexuada
};

#endif // PRESA_HPP

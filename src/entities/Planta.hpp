#ifndef PLANTA_HPP
#define PLANTA_HPP

#include "../core/Organismo.hpp"

class Planta : public Organismo {
public:
    static constexpr float ENERGIA_INICIAL         = 20.0f;
    static constexpr int   INTERVALO_ESPALHAMENTO  = 50;
    static constexpr float ENERGIA_FORNECIDA       = 10.0f;
    static constexpr int   IDADE_MAXIMA            = 300;

private:
    int contadorEspalhamento;

public:
    Planta(float x, float y);
    void agir(WorldManager& world) override;
    void mover(WorldManager& world) override;

    bool deveEspalhar()       const { return contadorEspalhamento >= INTERVALO_ESPALHAMENTO; }
    void resetarEspalhamento()      { contadorEspalhamento = 0; }

#ifdef USE_SFML
    void desenhar(sf::RenderWindow& window, float cellSize) const override;
#else
    void desenhar() const override;
#endif

    TipoOrganismo getTipo() const override { return TipoOrganismo::Planta; }
    char getSimboloDesenho() const override { return 'P'; }
    float getEnergiaParaReproducao() const override { return ENERGIA_INICIAL + 1.0f; }
    float getCustoReproducao() const override { return 0.0f; }
    int getCooldownReproducao() const override { return 0; }
    float getEnergiaFornecidaAoSerComido() const override { return ENERGIA_FORNECIDA; }
};

#endif // PLANTA_HPP

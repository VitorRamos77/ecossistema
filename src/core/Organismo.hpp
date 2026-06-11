#ifndef ORGANISMO_HPP
#define ORGANISMO_HPP

#include <memory>

class WorldManager;

#ifdef USE_SFML
// Forward declaration: evita contaminar toda a hierarquia com <SFML/Graphics.hpp>
namespace sf { class RenderWindow; }
#endif

enum class TipoOrganismo { Planta, Presa, Predador };

class Organismo {
protected:
    float x, y;
    float energia;
    int idade;
    int tempoParaReproducao;
    bool vivo;

public:
    Organismo(float x, float y, float energiaInicial, int cooldownReproducao);
    virtual ~Organismo() = default;

    virtual void agir(WorldManager& world) = 0;
    virtual void mover(WorldManager& world) = 0;

#ifdef USE_SFML
    virtual void desenhar(sf::RenderWindow& window, float cellSize) const = 0;
#else
    virtual void desenhar() const = 0;
#endif

    // Identificação e renderização sem dynamic_cast
    virtual TipoOrganismo getTipo() const = 0;
    virtual char getSimboloDesenho() const = 0;

    // Parâmetros de reprodução por espécie
    virtual float getEnergiaParaReproducao() const = 0;
    virtual float getCustoReproducao() const = 0;
    virtual int getCooldownReproducao() const = 0;

    // Energia fornecida ao ser comido
    virtual float getEnergiaFornecidaAoSerComido() const = 0;

    // Raio de proximidade exigido para reprodução (0 = sem exigência)
    virtual float getRaioReproducao() const { return 0.f; }

    bool estaVivo() const;
    float getX() const;
    float getY() const;
    float getEnergia() const;
    int getTempoParaReproducao() const;
    void envelhecer();
    void consumirEnergia(float quantidade);
    void resetCooldown(int value);
};

#endif // ORGANISMO_HPP

#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <array>
#include "WorldManager.hpp"
#include "UI.hpp"

class Renderer {
public:
    Renderer(int gridLargura, int gridAltura,
             float cellSize, float painelW,
             const std::string& titulo);

    // Mostra tela de configuração antes de iniciar; retorna false se janela fechada.
    bool mostrarTelaInicio(int& nPlantas, int& nPresas, int& nPredadores);

    // Processa eventos; retorna false se a janela foi fechada.
    // Atualiza world.params diretamente a partir dos sliders.
    bool processarEventos(float deltaTime, WorldManager& world);

    // encerrado=true: exibe gráfico em tela cheia (presas+predadores == 0).
    void renderizar(const WorldManager& world,
                    int nPlantas, int nPresas, int nPredadores,
                    bool pausado, bool encerrado = false);

    bool estaAberto() const { return window.isOpen(); }

    // Flags de eventos consumidos — lidos e resetados por main.cpp
    bool foiPausado()     { bool v = eventosPausa;      eventosPausa      = false; return v; }
    bool foiAcelerado()   { bool v = eventosAcelerar;   eventosAcelerar   = false; return v; }
    bool foiReiniciado()  { bool v = eventosReiniciado; eventosReiniciado = false; return v; }
    bool foiEncerrado()   { bool v = eventosEncerrar;   eventosEncerrar   = false; return v; }

    void atualizarLabelVelocidade(int v);
    void resetarUI();     // reseta sliders → 1.0 e botão pausa → inativo
    void resetarPausa();  // apenas botão pausa → inativo (sliders intactos)

private:
    sf::RenderWindow window;
    sf::View         cameraView;
    sf::Font         font;
    bool             fontCarregada = false;

    float cellSize;
    int   gridLargura;
    int   gridAltura;
    float gridPixelW;
    float gridPixelH;
    float painelX;
    float painelW;

    float zoomAtual       = 1.f;
    float cameraMoveSpeed = 200.f;

    bool eventosPausa      = false;
    bool eventosAcelerar   = false;
    bool eventosReiniciado = false;
    bool eventosEncerrar   = false;

    // Widgets
    Botao btnPausar;
    Botao btnAcelerar;
    Botao btnReiniciar;
    Botao btnEncerrar;
    std::array<Slider, 7> sliders;

    void inicializarUI();
    void aplicarParamsSliders(WorldManager& world);

    void processarMovimentoCamera(float deltaTime);
    void resetarCameraView();

    void renderizarFundoPainel();
    void renderizarEstatisticas(const WorldManager& world,
                                int nPlantas, int nPresas, int nPredadores);
    void renderizarGrafico(const WorldManager& world);
    void renderizarGraficoEncerrado(const WorldManager& world);
    void renderizarGrid();
    void renderizarOverlayPausa();
};

#endif // RENDERER_HPP

#include "core/WorldManager.hpp"
#include <iostream>
#include <string>

#ifdef USE_SFML
#include "core/Renderer.hpp"
#endif

int main(int argc, char* argv[]) {
    const int   GRID_W    = 60;
    const int   GRID_H    = 60;

    WorldManager world;

#ifdef USE_SFML
    const int   PAINEL_W  = 280;
    // ------------------------------------------------------------------
    // Modo terminal forçado com --terminal
    // ------------------------------------------------------------------
    bool modoTerminal = (argc > 1 && std::string(argv[1]) == "--terminal");
    if (!modoTerminal) {
        const float CELL_SIZE = 12.f; // grid 60x60 → 720x720px, janela total 1000x720
        Renderer renderer(GRID_W, GRID_H, CELL_SIZE, static_cast<float>(PAINEL_W),
                          "Simulador de Ecossistema");

        // Tela de configuração inicial — o usuário escolhe as populações
        int nPlantas = 150, nPresas = 40, nPredadores = 10;
        if (!renderer.mostrarTelaInicio(nPlantas, nPresas, nPredadores))
            return 0;

        world.inicializar(GRID_W, GRID_H, nPlantas, nPresas, nPredadores);

        sf::Clock clock;
        float acumulador   = 0.f;
        float tickInterval = 1.f / 10.f;
        int   velocidade   = 1;
        bool  pausado      = false;
        bool  encerrado    = false;  // true quando presas+predadores == 0

        while (renderer.estaAberto()) {
            float deltaTime = clock.restart().asSeconds();
            if (deltaTime > 0.05f) deltaTime = 0.05f;

            if (!renderer.processarEventos(deltaTime, world)) break;

            if (renderer.foiPausado()) {
                pausado = !pausado;
            }
            if (renderer.foiAcelerado()) {
                velocidade = (velocidade < 8) ? velocidade * 2 : 1;
                renderer.atualizarLabelVelocidade(velocidade);
            }
            if (renderer.foiEncerrado()) {
                encerrado = true;
            }
            if (renderer.foiReiniciado()) {
                // Reinicia com os mesmos valores escolhidos na tela inicial;
                // params dos sliders são preservados (não chamamos resetarUI).
                world.reiniciar(nPlantas, nPresas, nPredadores);
                acumulador = 0.f;
                velocidade = 1;
                pausado    = false;
                encerrado  = false;
                renderer.resetarPausa();
                renderer.atualizarLabelVelocidade(1);
            }

            // Avança simulação apenas enquanto não encerrada e não pausada
            if (!pausado && !encerrado) {
                acumulador += deltaTime * static_cast<float>(velocidade);
                while (acumulador >= tickInterval) {
                    world.tick();
                    acumulador -= tickInterval;
                }
            }

            int nP = 0, nC = 0, nL = 0;
            world.getCounts(nP, nC, nL);

            // Detecta fim da simulação (após ao menos 1 tick)
            if (world.getTickAtual() > 0 && nC == 0 && nL == 0)
                encerrado = true;

            renderer.renderizar(world, nP, nC, nL, pausado, encerrado);
        }
        return 0;
    }
#else
    // Sem USE_SFML: ignora argumento --terminal e vai direto ao modo texto
    (void)argc; (void)argv;
#endif

    // ------------------------------------------------------------------
    // Modo terminal
    // ------------------------------------------------------------------
    world.inicializar(GRID_W, GRID_H, /*plantas=*/150, /*presas=*/40, /*predadores=*/10);
    const int TICKS = 500;
    for (int t = 0; t < TICKS; ++t) {
        int nP = 0, nC = 0, nL = 0;
        for (const auto& org : world.getOrganismos()) {
            if (!org->estaVivo()) continue;
            switch (org->getTipo()) {
                case TipoOrganismo::Planta:   nP++; break;
                case TipoOrganismo::Presa:    nC++; break;
                case TipoOrganismo::Predador: nL++; break;
            }
        }
        std::cout << "Tick: " << t
                  << " | Plantas: " << nP
                  << " | Presas: "  << nC
                  << " | Predadores: " << nL << '\n';
        world.renderizar();
        std::cout << '\n';
        world.tick();
        if (nC == 0 && nL == 0) {
            std::cout << "Simulacao terminada: todas as presas e predadores morreram.\n";
            break;
        }
    }

    return 0;
}

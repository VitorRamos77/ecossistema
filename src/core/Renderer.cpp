#include "Renderer.hpp"
#include <string>
#include <algorithm>
#include <cmath>

Renderer::Renderer(int gridLargura, int gridAltura,
                   float cellSize, float painelW,
                   const std::string& titulo)
    : cellSize(cellSize),
      gridLargura(gridLargura), gridAltura(gridAltura),
      gridPixelW(gridLargura * cellSize),
      gridPixelH(gridAltura  * cellSize),
      painelX(gridLargura * cellSize),
      painelW(painelW)
{
    unsigned int winW = static_cast<unsigned int>(gridPixelW + painelW);
    unsigned int winH = static_cast<unsigned int>(gridPixelH);

    // Janela fixa: redimensionar distorceria as views (não há tratamento de Resized)
    window.create(sf::VideoMode(winW, winH), titulo,
                  sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    // cameraView cobre apenas a área do grid (viewport fracionado)
    float vpW = gridPixelW / (gridPixelW + painelW);
    cameraView.setSize(gridPixelW, gridPixelH);
    cameraView.setCenter(gridPixelW / 2.f, gridPixelH / 2.f);
    cameraView.setViewport(sf::FloatRect(0.f, 0.f, vpW, 1.f));

    if (font.loadFromFile("assets/font.ttf"))
        fontCarregada = true;

    inicializarUI();
}

void Renderer::inicializarUI() {
    if (!fontCarregada) return;

    float bx  = painelX + 10.f;
    float bh  = 30.f;
    float bw  = (painelW - 30.f) / 3.f;
    float by  = 330.f;

    btnPausar.init   (font, "Pausar",     bx,                    by, bw, bh);
    btnAcelerar.init (font, "1x",         bx + bw + 5.f,         by, bw, bh);
    btnReiniciar.init(font, "Reiniciar",  bx + (bw + 5.f) * 2.f, by, bw, bh);

    float encY = static_cast<float>(window.getSize().y) - 46.f;
    btnEncerrar.init(font, "Encerrar Simulacao", bx, encY, painelW - 20.f, 30.f);
    btnEncerrar.corNormal = sf::Color(80, 25, 25);
    btnEncerrar.corHover  = sf::Color(150, 40, 40);

    struct SliderDef { const char* label; float vmin, vmax, val; };
    SliderDef defs[7] = {
        {"Reprod. Presa",    0.1f, 3.0f, 1.0f},
        {"Reprod. Predador", 0.1f, 3.0f, 1.0f},
        {"Veloc. Presa",     0.1f, 3.0f, 1.0f},
        {"Veloc. Predador",  0.1f, 3.0f, 1.0f},
        {"Spawn Plantas",    0.1f, 5.0f, 1.0f},
        {"Visao Presa",      0.25f,4.0f, 1.0f},
        {"Visao Predador",   0.25f,4.0f, 1.0f}
    };

    float sx   = painelX + 10.f;
    float sW   = painelW - 70.f;
    float sy   = 395.f;
    float sEsp = 44.f;

    for (int i = 0; i < 7; ++i) {
        sliders[i].init(font, defs[i].label,
                        sx, sy + i * sEsp,
                        sW, defs[i].vmin, defs[i].vmax, defs[i].val);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Tela de configuração inicial
// ─────────────────────────────────────────────────────────────────────────────
bool Renderer::mostrarTelaInicio(int& nPlantas, int& nPresas, int& nPredadores) {
    if (!fontCarregada) return true;  // sem fonte: usa valores padrão

    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);
    float cx   = winW / 2.f;

    // Sliders de configuração (variáveis locais — independentes dos de simulação)
    Slider sldPlantas, sldPresas, sldPredadores;
    Botao  btnIniciar;

    float sx = cx - 140.f;
    float sW = 260.f;
    sldPlantas.init   (font, "Plantas",     sx, 250.f, sW,  10.f, 500.f, static_cast<float>(nPlantas));
    sldPresas.init    (font, "Presas",      sx, 310.f, sW,   5.f, 150.f, static_cast<float>(nPresas));
    sldPredadores.init(font, "Predadores",  sx, 370.f, sW,   1.f,  50.f, static_cast<float>(nPredadores));

    // Exibe valores como inteiros
    auto mostrarInt = [](Slider& s) {
        s.valorTxt.setString(std::to_string(static_cast<int>(s.valor)));
    };
    mostrarInt(sldPlantas);
    mostrarInt(sldPresas);
    mostrarInt(sldPredadores);

    btnIniciar.init(font, "Iniciar Simulacao", cx - 100.f, 440.f, 200.f, 36.f);

    while (window.isOpen()) {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) { window.close(); return false; }
            if (ev.type == sf::Event::KeyPressed) {
                if (ev.key.code == sf::Keyboard::Escape) { window.close(); return false; }
                if (ev.key.code == sf::Keyboard::Return) {
                    nPlantas    = static_cast<int>(sldPlantas.valor);
                    nPresas     = static_cast<int>(sldPresas.valor);
                    nPredadores = static_cast<int>(sldPredadores.valor);
                    return true;
                }
            }
            // Propaga mouse para sliders; atualiza display inteiro após cada evento
            sldPlantas.processarMouse(ev);    mostrarInt(sldPlantas);
            sldPresas.processarMouse(ev);     mostrarInt(sldPresas);
            sldPredadores.processarMouse(ev); mostrarInt(sldPredadores);

            if (ev.type == sf::Event::MouseButtonPressed &&
                ev.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f pos(static_cast<float>(ev.mouseButton.x),
                                 static_cast<float>(ev.mouseButton.y));
                if (btnIniciar.contemPonto(pos)) {
                    nPlantas    = static_cast<int>(sldPlantas.valor);
                    nPresas     = static_cast<int>(sldPresas.valor);
                    nPredadores = static_cast<int>(sldPredadores.valor);
                    return true;
                }
            }
        }

        window.clear(sf::Color(15, 15, 20));

        // Título
        sf::Text titulo("ECOSSISTEMA", font, 42);
        titulo.setFillColor(sf::Color(180, 210, 255));
        auto tb = titulo.getLocalBounds();
        titulo.setPosition((winW - tb.width) / 2.f - tb.left, 90.f);
        window.draw(titulo);

        sf::Text sub("Configurar populacoes iniciais", font, 14);
        sub.setFillColor(sf::Color(90, 90, 130));
        auto sb = sub.getLocalBounds();
        sub.setPosition((winW - sb.width) / 2.f - sb.left, 148.f);
        window.draw(sub);

        // Separador decorativo
        sf::RectangleShape sep(sf::Vector2f(420.f, 1.f));
        sep.setPosition(cx - 210.f, 195.f);
        sep.setFillColor(sf::Color(45, 45, 65));
        window.draw(sep);

        sldPlantas.desenhar(window);
        sldPresas.desenhar(window);
        sldPredadores.desenhar(window);

        sf::Vector2f mousePos(
            static_cast<float>(sf::Mouse::getPosition(window).x),
            static_cast<float>(sf::Mouse::getPosition(window).y));
        btnIniciar.desenhar(window, mousePos);

        sf::Text hint("Enter para iniciar   |   Esc para sair", font, 10);
        hint.setFillColor(sf::Color(55, 55, 75));
        auto hb = hint.getLocalBounds();
        hint.setPosition((winW - hb.width) / 2.f - hb.left, winH - 30.f);
        window.draw(hint);

        window.display();
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Eventos
// ─────────────────────────────────────────────────────────────────────────────
bool Renderer::processarEventos(float deltaTime, WorldManager& world) {
    sf::Event event;
    while (window.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                window.close();
                return false;

            case sf::Event::KeyPressed:
                switch (event.key.code) {
                    case sf::Keyboard::Escape:
                        window.close();
                        return false;
                    case sf::Keyboard::Space:
                        eventosPausa = true;
                        btnPausar.ativo = !btnPausar.ativo;
                        break;
                    case sf::Keyboard::F:
                        eventosAcelerar = true;
                        break;
                    case sf::Keyboard::R:
                        // R reseta apenas câmera
                        resetarCameraView();
                        break;
                    default:
                        break;
                }
                break;

            case sf::Event::MouseWheelScrolled:
                // Zoom apenas sobre a área do grid
                if (event.mouseWheelScroll.x < painelX) {
                    float fator = (event.mouseWheelScroll.delta > 0) ? 0.9f : 1.1f;
                    zoomAtual  *= fator;
                    if (zoomAtual < 0.2f) zoomAtual = 0.2f;
                    if (zoomAtual > 4.0f) zoomAtual = 4.0f;
                    cameraView.setSize(sf::Vector2f(gridPixelW, gridPixelH) * zoomAtual);
                }
                break;

            case sf::Event::MouseButtonPressed:
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f pos(static_cast<float>(event.mouseButton.x),
                                     static_cast<float>(event.mouseButton.y));
                    if (btnPausar.contemPonto(pos)) {
                        eventosPausa    = true;
                        btnPausar.ativo = !btnPausar.ativo;
                    }
                    if (btnAcelerar.contemPonto(pos))
                        eventosAcelerar = true;
                    if (btnReiniciar.contemPonto(pos))
                        eventosReiniciado = true;
                    if (btnEncerrar.contemPonto(pos))
                        eventosEncerrar = true;
                }
                // Passa para sliders (clique no trilho já pode alterar o valor)
                {
                    bool mudou = false;
                    for (auto& s : sliders)
                        if (s.processarMouse(event)) mudou = true;
                    if (mudou) aplicarParamsSliders(world);
                }
                break;

            case sf::Event::MouseButtonReleased:
                for (auto& s : sliders) s.processarMouse(event);
                break;

            case sf::Event::MouseMoved: {
                // Propaga para sliders; se algum mudou, atualiza params
                bool mudou = false;
                for (auto& s : sliders)
                    if (s.processarMouse(event)) mudou = true;
                if (mudou) aplicarParamsSliders(world);
                break;
            }

            default:
                break;
        }
    }

    // Movimento de câmera com teclado — apenas quando mouse sobre o grid
    processarMovimentoCamera(deltaTime);
    return true;
}

void Renderer::aplicarParamsSliders(WorldManager& world) {
    world.params.taxaReproducaoPresa    = sliders[0].valor;
    world.params.taxaReproducaoPredador = sliders[1].valor;
    world.params.velocidadePresa        = sliders[2].valor;
    world.params.velocidadePredador     = sliders[3].valor;
    world.params.taxaSpawnPlanta        = sliders[4].valor;
    world.params.raioVisaoPresa         = sliders[5].valor;
    world.params.raioVisaoPredador      = sliders[6].valor;
}

void Renderer::resetarCameraView() {
    cameraView.setSize(gridPixelW, gridPixelH);
    cameraView.setCenter(gridPixelW / 2.f, gridPixelH / 2.f);
    float vpW = gridPixelW / (gridPixelW + painelW);
    cameraView.setViewport(sf::FloatRect(0.f, 0.f, vpW, 1.f));
    zoomAtual = 1.f;
}

void Renderer::processarMovimentoCamera(float deltaTime) {
    float speed = cameraMoveSpeed * zoomAtual * deltaTime;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        cameraView.move(0.f, -speed);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        cameraView.move(0.f,  speed);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        cameraView.move(-speed, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        cameraView.move( speed, 0.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// UI helpers
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::atualizarLabelVelocidade(int v) {
    btnAcelerar.setLabel(std::to_string(v) + "x");
}

void Renderer::resetarUI() {
    for (auto& s : sliders) s.resetar();
    btnPausar.ativo = false;
    atualizarLabelVelocidade(1);
}

void Renderer::resetarPausa() {
    btnPausar.ativo = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Renderização
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::renderizarGrid() {
    if (zoomAtual > 0.6f) {
        // Todas as linhas num único VertexArray — 1 draw call em vez de ~122
        sf::Color cor(30, 30, 38);
        sf::VertexArray linhas(sf::Lines);
        for (int col = 0; col <= gridLargura; ++col) {
            linhas.append({sf::Vector2f(col * cellSize, 0.f),        cor});
            linhas.append({sf::Vector2f(col * cellSize, gridPixelH), cor});
        }
        for (int row = 0; row <= gridAltura; ++row) {
            linhas.append({sf::Vector2f(0.f,        row * cellSize), cor});
            linhas.append({sf::Vector2f(gridPixelW, row * cellSize), cor});
        }
        window.draw(linhas);
    }
}

void Renderer::renderizarFundoPainel() {
    // Fundo
    sf::RectangleShape bg(sf::Vector2f(painelW, static_cast<float>(window.getSize().y)));
    bg.setPosition(painelX, 0.f);
    bg.setFillColor(sf::Color(18, 18, 25));
    window.draw(bg);

    // Borda esquerda
    sf::RectangleShape borda(sf::Vector2f(1.f, static_cast<float>(window.getSize().y)));
    borda.setPosition(painelX, 0.f);
    borda.setFillColor(sf::Color(60, 60, 80));
    window.draw(borda);
}

void Renderer::renderizarEstatisticas(const WorldManager& world,
                                       int nPlantas, int nPresas, int nPredadores) {
    if (!fontCarregada) return;

    auto linha = [&](const std::string& txt, float y, sf::Color cor) {
        // fromUtf8: strings literais são UTF-8, mas sf::Text trata std::string
        // como Latin-1 — sem isso o "Δ" vira "Î”" na tela
        sf::Text t(sf::String::fromUtf8(txt.begin(), txt.end()), font, 13);
        t.setFillColor(cor);
        t.setPosition(painelX + 10.f, y);
        window.draw(t);
    };

    linha("Tick: " + std::to_string(world.getTickAtual()),  8.f,  sf::Color(220, 220, 220));
    linha("Plantas:    " + std::to_string(nPlantas),        26.f, sf::Color(34,  200,  34));
    linha("Presas:     " + std::to_string(nPresas),         44.f, sf::Color(100, 180, 255));
    linha("Predadores: " + std::to_string(nPredadores),     62.f, sf::Color(255,  80,  80));

    // Delta desde último snapshot
    const auto& hist = world.getHistorico();
    if (hist.size() >= 2) {
        const auto& prev = hist[hist.size() - 2];
        const auto& curr = hist.back();
        auto delta = [](int a, int b) -> std::string {
            int d = b - a;
            return (d >= 0 ? "+" : "") + std::to_string(d);
        };
        linha("\xce\x94 Presas: " + delta(prev.nPresas, curr.nPresas)
            + "  \xce\x94 Pred: " + delta(prev.nPredadores, curr.nPredadores),
            80.f, sf::Color(150, 150, 170));
    }

    // Separador
    sf::RectangleShape sep(sf::Vector2f(painelW - 20.f, 1.f));
    sep.setPosition(painelX + 10.f, 100.f);
    sep.setFillColor(sf::Color(60, 60, 80));
    window.draw(sep);
}

void Renderer::renderizarGrafico(const WorldManager& world) {
    float gX = painelX + 10.f;
    float gY = 108.f;
    float gW = painelW - 20.f;
    float gH = 175.f;

    // Fundo e borda
    sf::RectangleShape fundo(sf::Vector2f(gW, gH));
    fundo.setPosition(gX, gY);
    fundo.setFillColor(sf::Color(20, 20, 28));
    fundo.setOutlineColor(sf::Color(60, 60, 75));
    fundo.setOutlineThickness(1.f);
    window.draw(fundo);

    // Linhas de referência horizontais
    for (int i = 1; i <= 4; ++i) {
        float y = gY + gH * (i / 5.f);
        sf::Vertex ref[2] = {
            {sf::Vector2f(gX,      y), sf::Color(50, 50, 65)},
            {sf::Vector2f(gX + gW, y), sf::Color(50, 50, 65)}
        };
        window.draw(ref, 2, sf::Lines);
    }

    const auto& hist = world.getHistorico();
    if (hist.size() < 2) return;

    // Plota uma série de dados
    auto plotarSerie = [&](int maxGlobal, sf::Color cor,
                           int SnapshotPopulacao::* campo) {
        int maxVal = maxGlobal > 0 ? maxGlobal : 1;
        sf::VertexArray va(sf::LineStrip, hist.size());
        for (std::size_t i = 0; i < hist.size(); ++i) {
            float px = gX + (static_cast<float>(i) / (hist.size() - 1)) * gW;
            float py = gY + gH - (static_cast<float>(hist[i].*campo) / maxVal) * gH;
            va[i].position = sf::Vector2f(px, py);
            va[i].color    = cor;
        }
        window.draw(va);
    };

    // Escala baseada em presas e predadores apenas
    int maxVal = 1;
    for (const auto& s : hist) {
        if (s.nPresas     > maxVal) maxVal = s.nPresas;
        if (s.nPredadores > maxVal) maxVal = s.nPredadores;
    }

    plotarSerie(maxVal, sf::Color(100, 180, 255), &SnapshotPopulacao::nPresas);
    plotarSerie(maxVal, sf::Color(255,  80,  80), &SnapshotPopulacao::nPredadores);

    // Legenda
    if (!fontCarregada) return;
    struct Item { sf::Color cor; const char* lbl; };
    Item legenda[2] = {
        {sf::Color(100, 180, 255), "Presas"},
        {sf::Color(255,  80,  80), "Pred."}
    };
    for (int i = 0; i < 2; ++i) {
        float lx = gX + i * (gW / 3.f);
        float ly = gY + gH + 6.f;
        sf::RectangleShape q(sf::Vector2f(10.f, 10.f));
        q.setPosition(lx, ly);
        q.setFillColor(legenda[i].cor);
        window.draw(q);
        sf::Text txt(legenda[i].lbl, font, 10);
        txt.setFillColor(sf::Color(200, 200, 200));
        txt.setPosition(lx + 13.f, ly - 1.f);
        window.draw(txt);
    }

    // Título do gráfico
    sf::Text titulo("Populacoes", font, 10);
    titulo.setFillColor(sf::Color(120, 120, 140));
    titulo.setPosition(gX + 2.f, gY + 2.f);
    window.draw(titulo);

    // Separador abaixo da legenda
    sf::RectangleShape sep(sf::Vector2f(painelW - 20.f, 1.f));
    sep.setPosition(painelX + 10.f, gY + gH + 24.f);
    sep.setFillColor(sf::Color(60, 60, 80));
    window.draw(sep);
}

void Renderer::renderizarGraficoEncerrado(const WorldManager& world) {
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    // Fundo escuro cobrindo a janela inteira
    sf::RectangleShape bg(sf::Vector2f(winW, winH));
    bg.setFillColor(sf::Color(12, 12, 18));
    window.draw(bg);

    if (fontCarregada) {
        sf::Text titulo("SIMULACAO ENCERRADA", font, 26);
        titulo.setFillColor(sf::Color(255, 200, 80));
        auto tb = titulo.getLocalBounds();
        titulo.setPosition((winW - tb.width) / 2.f - tb.left, 10.f);
        window.draw(titulo);

        std::string infoStr = "Ticks: " + std::to_string(world.getTickAtual())
                            + "    |    Todas as presas e predadores morreram";
        sf::Text info(infoStr, font, 12);
        info.setFillColor(sf::Color(110, 110, 145));
        auto ib = info.getLocalBounds();
        info.setPosition((winW - ib.width) / 2.f - ib.left, 48.f);
        window.draw(info);
    }

    float gX = 30.f;
    float gY = 75.f;
    float gW = winW - 60.f;
    float gH = winH - 190.f;

    sf::RectangleShape fundo(sf::Vector2f(gW, gH));
    fundo.setPosition(gX, gY);
    fundo.setFillColor(sf::Color(20, 20, 28));
    fundo.setOutlineColor(sf::Color(60, 60, 75));
    fundo.setOutlineThickness(1.f);
    window.draw(fundo);

    for (int i = 1; i <= 4; ++i) {
        float y = gY + gH * (i / 5.f);
        sf::Vertex ref[2] = {
            {sf::Vector2f(gX,      y), sf::Color(50, 50, 65)},
            {sf::Vector2f(gX + gW, y), sf::Color(50, 50, 65)}
        };
        window.draw(ref, 2, sf::Lines);
    }

    const auto& hist = world.getHistorico();
    if (hist.size() >= 2) {
        auto plotar = [&](int maxGlobal, sf::Color cor, int SnapshotPopulacao::* campo) {
            int maxVal = maxGlobal > 0 ? maxGlobal : 1;
            sf::VertexArray va(sf::LineStrip, hist.size());
            for (std::size_t i = 0; i < hist.size(); ++i) {
                float px = gX + (static_cast<float>(i) / (hist.size() - 1)) * gW;
                float py = gY + gH - (static_cast<float>(hist[i].*campo) / maxVal) * gH;
                va[i].position = sf::Vector2f(px, py);
                va[i].color    = cor;
            }
            window.draw(va);
        };

        int maxVal = 1;
        for (const auto& s : hist) {
            if (s.nPresas     > maxVal) maxVal = s.nPresas;
            if (s.nPredadores > maxVal) maxVal = s.nPredadores;
        }
        plotar(maxVal, sf::Color(100, 180, 255), &SnapshotPopulacao::nPresas);
        plotar(maxVal, sf::Color(255,  80,  80), &SnapshotPopulacao::nPredadores);
    }

    if (fontCarregada) {
        struct Item { sf::Color cor; const char* lbl; };
        Item legenda[2] = {
            {sf::Color(100, 180, 255), "Presas"},
            {sf::Color(255,  80,  80), "Predadores"}
        };
        float ly = gY + gH + 12.f;
        for (int i = 0; i < 2; ++i) {
            float lx = gX + i * (gW / 3.f);
            sf::RectangleShape q(sf::Vector2f(12.f, 12.f));
            q.setPosition(lx, ly);
            q.setFillColor(legenda[i].cor);
            window.draw(q);
            sf::Text txt(legenda[i].lbl, font, 12);
            txt.setFillColor(sf::Color(200, 200, 200));
            txt.setPosition(lx + 16.f, ly - 1.f);
            window.draw(txt);
        }
    }
}

void Renderer::renderizarOverlayPausa() {
    // Retângulo semi-transparente sobre o grid
    sf::RectangleShape overlay(sf::Vector2f(gridPixelW, 40.f));
    overlay.setPosition(0.f, gridPixelH / 2.f - 20.f);
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    window.draw(overlay);

    if (fontCarregada) {
        sf::Text txt("PAUSADO", font, 24);
        txt.setFillColor(sf::Color(255, 220, 100));
        auto b = txt.getLocalBounds();
        txt.setPosition(
            (gridPixelW - b.width)  / 2.f - b.left,
            gridPixelH  / 2.f - b.height / 2.f - b.top
        );
        window.draw(txt);
    }
}

void Renderer::renderizar(const WorldManager& world,
                          int nPlantas, int nPresas, int nPredadores,
                          bool pausado, bool encerrado) {
    window.clear(sf::Color(15, 15, 20));

    sf::Vector2f mousePos(static_cast<float>(sf::Mouse::getPosition(window).x),
                          static_cast<float>(sf::Mouse::getPosition(window).y));

    // ── Modo encerrado: gráfico em tela cheia ────────────────────────────────
    if (encerrado) {
        window.setView(window.getDefaultView());
        renderizarGraficoEncerrado(world);

        // Botões centralizados na parte inferior
        float cx = static_cast<float>(window.getSize().x) / 2.f;
        float cy = static_cast<float>(window.getSize().y) - 55.f;
        btnReiniciar.shape.setSize(sf::Vector2f(200.f, 36.f));
        btnReiniciar.setPosition(cx - 210.f, cy);
        btnReiniciar.desenhar(window, mousePos);

        btnEncerrar.shape.setSize(sf::Vector2f(200.f, 36.f));
        btnEncerrar.setPosition(cx + 10.f, cy);
        btnEncerrar.desenhar(window, mousePos);

        window.display();
        return;
    }

    // ── Modo normal: restaura botão Reiniciar ao painel ─────────────────────
    {
        float bw = (painelW - 30.f) / 3.f;
        btnReiniciar.shape.setSize(sf::Vector2f(bw, 30.f));
        btnReiniciar.setPosition(painelX + 10.f + (bw + 5.f) * 2.f, 330.f);
    }

    // 1. Mundo (view do grid)
    window.setView(cameraView);
    renderizarGrid();

    // Plantas em batch: podem chegar a 2000 — um único draw call em vez de um por planta.
    // Cor/tamanho replicam Planta::desenhar.
    sf::VertexArray plantasVA(sf::Quads);
    const sf::Color corPlanta(34, 139, 34);
    for (const auto& org : world.getOrganismos()) {
        if (!org->estaVivo() || org->getTipo() != TipoOrganismo::Planta) continue;
        float px = org->getX() * cellSize;
        float py = org->getY() * cellSize;
        float s  = cellSize - 1.f;
        plantasVA.append({{px,     py    }, corPlanta});
        plantasVA.append({{px + s, py    }, corPlanta});
        plantasVA.append({{px + s, py + s}, corPlanta});
        plantasVA.append({{px,     py + s}, corPlanta});
    }
    window.draw(plantasVA);

    // Animais por cima das plantas
    for (const auto& org : world.getOrganismos()) {
        if (org->estaVivo() && org->getTipo() != TipoOrganismo::Planta)
            org->desenhar(window, cellSize);
    }

    // 2. Painel e widgets (view padrão — fixo na tela)
    window.setView(window.getDefaultView());
    renderizarFundoPainel();
    renderizarEstatisticas(world, nPlantas, nPresas, nPredadores);
    renderizarGrafico(world);

    // 3. Botões e sliders
    btnPausar.desenhar   (window, mousePos);
    btnAcelerar.desenhar (window, mousePos);
    btnReiniciar.desenhar(window, mousePos);
    btnEncerrar.desenhar (window, mousePos);
    for (auto& s : sliders) s.desenhar(window);

    if (fontCarregada) {
        sf::Text tSliders("Parametros", font, 11);
        tSliders.setFillColor(sf::Color(120, 120, 140));
        tSliders.setPosition(painelX + 10.f, 378.f);
        window.draw(tSliders);

        sf::Text hint("WASD:cam  Scroll:zoom  R:reset  Esc:sair", font, 9);
        hint.setFillColor(sf::Color(80, 80, 100));
        hint.setPosition(painelX + 5.f,
                         static_cast<float>(window.getSize().y) - 14.f);
        window.draw(hint);
    }

    // 4. Overlay de pausa (por cima de tudo)
    if (pausado) renderizarOverlayPausa();

    window.display();
}

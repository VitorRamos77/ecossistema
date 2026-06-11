# Prompt — Fase 4: Interface e Controles (C++17)

## Contexto obrigatório — leia antes de qualquer coisa

As Fases 1, 2 e 3 estão implementadas e funcionando. Existe uma janela SFML
renderizando o grid 100×100 com câmera, zoom e HUD básico. Esta fase adiciona
um painel lateral com estatísticas em tempo real, gráfico de populações,
botões de controle e sliders para ajuste de parâmetros.

**Regras de ouro para esta fase:**
1. **Lógica de simulação é intocável.** `agir()`, `mover()`, `tick()` — zero alterações.
2. **`Renderer` recebe um painel lateral, não vira um framework de UI.**
   Toda a UI é construída com primitivas SFML — sem bibliotecas externas de UI.
3. **Os sliders alteram parâmetros via `WorldManager`**, não hackeando as entidades diretamente.
4. Antes de escrever qualquer código, leia todos os `.hpp` existentes.

---

## Passo 0 — Leitura obrigatória do projeto atual

1. Leia `src/core/Renderer.hpp` — estrutura atual, o que já existe
2. Leia `src/core/WorldManager.hpp` — métodos públicos disponíveis
3. Leia `src/core/Organismo.hpp` e `Animal.hpp` — constantes e atributos existentes
4. Leia `src/main.cpp` — loop atual, como pausa e velocidade estão implementados
5. Confirme o tamanho da janela atual (`GRID_W * CELL_SIZE` × `GRID_H * CELL_SIZE`)

Só prossiga após essa leitura.

---

## Passo 1 — Redimensionar a janela para acomodar o painel

O painel lateral tem largura fixa de **280px** e fica à direita do grid.

### Em `main.cpp`, ajuste a criação da janela:

```cpp
const int   GRID_W      = 100;
const int   GRID_H      = 100;
const float CELL_SIZE   = 8.f;
const int   PAINEL_W    = 280;   // largura do painel lateral em pixels

// Janela total: grid + painel
const unsigned int WIN_W = static_cast<unsigned int>(GRID_W * CELL_SIZE) + PAINEL_W;
const unsigned int WIN_H = static_cast<unsigned int>(GRID_H * CELL_SIZE);
```

Passe `PAINEL_W` ao construtor do `Renderer` para que ele saiba onde começa o painel.

### Em `Renderer`, ajuste a `cameraView` para ocupar apenas a área do grid:

```cpp
// A cameraView cobre apenas a região do grid (lado esquerdo)
float gridPixelW = gridLargura * cellSize;
float gridPixelH = gridAltura  * cellSize;

cameraView.setSize(gridPixelW, gridGridPixelH);
cameraView.setCenter(gridPixelW / 2.f, gridPixelH / 2.f);
cameraView.setViewport(sf::FloatRect(
    0.f,                                    // left
    0.f,                                    // top
    gridPixelW / (gridPixelW + painelW),    // width  (fração da janela)
    1.f                                     // height (100% da altura)
));
```

Isso garante que a câmera nunca renderize sobre o painel.

---

## Passo 2 — Sistema de parâmetros ajustáveis

Antes de criar a UI, crie a estrutura que os sliders vão modificar.

### Crie `src/core/SimParams.hpp`:

```cpp
#pragma once

// Parâmetros ajustáveis em tempo de execução via sliders.
// WorldManager mantém uma referência a esta struct.
struct SimParams {
    // Reprodução
    float taxaReproducaoPresa     = 1.0f;  // multiplicador (0.1 – 3.0)
    float taxaReproducaoPredador  = 1.0f;  // multiplicador (0.1 – 3.0)

    // Velocidade das entidades
    float velocidadePresa         = 1.0f;  // multiplicador (0.1 – 3.0)
    float velocidadePredador      = 1.0f;  // multiplicador (0.1 – 3.0)

    // Spawn de plantas
    float taxaSpawnPlanta         = 1.0f;  // multiplicador (0.1 – 5.0)

    // Raio de visão
    float raioVisaoPresa          = 1.0f;  // multiplicador (0.25 – 4.0)
    float raioVisaoPredador       = 1.0f;  // multiplicador (0.25 – 4.0)
};
```

### Em `WorldManager`, adicione:

```cpp
// Em WorldManager.hpp:
#include "SimParams.hpp"

class WorldManager {
public:
    SimParams params;   // público para acesso direto pelo painel
    // ...
};
```

### Como os multiplicadores são aplicados

Os multiplicadores **não alteram as constantes das classes** — elas são `static constexpr`
e imutáveis. Em vez disso, aplique o multiplicador no ponto de uso dentro de `WorldManager`
ou nas próprias entidades via getter:

```cpp
// Exemplo em WorldManager::tick() na lógica de reprodução de Presa:
float cooldownEfetivo = Presa::COOLDOWN_REPRODUCAO / params.taxaReproducaoPresa;

// Exemplo em Presa::mover() — passe o multiplicador via WorldManager:
float velocidadeEfetiva = VELOCIDADE * world.params.velocidadePresa;
```

Documente no código onde cada parâmetro é aplicado.

---

## Passo 3 — Estrutura do painel lateral

O painel é dividido em quatro seções verticais, de cima para baixo:

```
┌─────────────────────────────┐
│  SEÇÃO 1 — Estatísticas     │  ~120px
│  Tick, populações, taxa     │
├─────────────────────────────┤
│  SEÇÃO 2 — Gráfico          │  ~200px
│  Linhas de população        │
├─────────────────────────────┤
│  SEÇÃO 3 — Controles        │  ~100px
│  Botões: pausar/acelerar/   │
│  reiniciar                  │
├─────────────────────────────┤
│  SEÇÃO 4 — Sliders          │  ~360px
│  7 sliders de parâmetros    │
└─────────────────────────────┘
```

### Coordenadas de referência

```cpp
// Em Renderer.hpp, adicione como membros:
float painelX;       // = gridLargura * cellSize  (borda esquerda do painel)
float painelW;       // = 280.f
// Seções (Y absoluto na janela):
// Estatísticas: painelX, y=0,   altura=120
// Gráfico:      painelX, y=120, altura=200
// Controles:    painelX, y=320, altura=100
// Sliders:      painelX, y=420, altura=360
```

---

## Passo 4 — Histórico de populações para o gráfico

O gráfico precisa de histórico. Adicione em `WorldManager`:

```cpp
// Em WorldManager.hpp:
struct SnapshotPopulacao {
    int tick;
    int nPlantas;
    int nPresas;
    int nPredadores;
};

// Histórico dos últimos N snapshots
static constexpr int MAX_HISTORICO = 500;
std::deque<SnapshotPopulacao> historicoPopulacao;

// Chamado no final de tick():
void registrarSnapshot();
```

### Implementação de `registrarSnapshot()`:

```cpp
void WorldManager::registrarSnapshot() {
    SnapshotPopulacao snap{tickAtual, 0, 0, 0};
    for (const auto& org : organismos) {
        if (!org->estaVivo()) continue;
        switch (org->getTipo()) {
            case TipoOrganismo::Planta:    ++snap.nPlantas;    break;
            case TipoOrganismo::Presa:     ++snap.nPresas;     break;
            case TipoOrganismo::Predador:  ++snap.nPredadores; break;
        }
    }
    historicoPopulacao.push_back(snap);
    if (historicoPopulacao.size() > MAX_HISTORICO)
        historicoPopulacao.pop_front();
}
```

Exponha via getter read-only:

```cpp
const std::deque<SnapshotPopulacao>& getHistorico() const {
    return historicoPopulacao;
}
int getTickAtual() const { return tickAtual; }
```

---

## Passo 5 — Gráfico de linhas

Implemente em `Renderer` como método privado `renderizarGrafico()`.

### Layout do gráfico:

```cpp
// Área do gráfico dentro do painel:
float gX = painelX + 10.f;   // margem esquerda
float gY = 130.f;             // topo (abaixo das estatísticas)
float gW = painelW - 20.f;   // largura
float gH = 180.f;             // altura
```

### Fundo e borda:

```cpp
sf::RectangleShape fundo(sf::Vector2f(gW, gH));
fundo.setPosition(gX, gY);
fundo.setFillColor(sf::Color(20, 20, 28));
fundo.setOutlineColor(sf::Color(60, 60, 75));
fundo.setOutlineThickness(1.f);
window.draw(fundo);
```

### Linhas de referência horizontais (4 linhas, opacidade baixa):

```cpp
for (int i = 1; i <= 4; ++i) {
    float y = gY + gH * (i / 5.f);
    sf::Vertex linha[2] = {
        {{gX,      y}, sf::Color(50, 50, 65)},
        {{gX + gW, y}, sf::Color(50, 50, 65)}
    };
    window.draw(linha, 2, sf::Lines);
}
```

### Plotar as três séries de dados:

```cpp
// Para cada série (Plantas, Presas, Predadores):
// 1. Encontrar o valor máximo para normalizar
// 2. Mapear cada ponto: x = índice normalizado na largura, y = valor normalizado na altura
// 3. Desenhar com sf::VertexArray e sf::LineStrip

auto plotarSerie = [&](auto valorFn, sf::Color cor) {
    const auto& hist = world.getHistorico();
    if (hist.size() < 2) return;

    int maxVal = 1;
    for (const auto& s : hist)
        maxVal = std::max(maxVal, valorFn(s));

    sf::VertexArray linhas(sf::LineStrip, hist.size());
    for (std::size_t i = 0; i < hist.size(); ++i) {
        float px = gX + (float(i) / (hist.size() - 1)) * gW;
        float py = gY + gH - (float(valorFn(hist[i])) / maxVal) * gH;
        linhas[i].position = {px, py};
        linhas[i].color    = cor;
    }
    window.draw(linhas);
};

plotarSerie([](const auto& s){ return s.nPlantas;    }, sf::Color(34,  180, 34));
plotarSerie([](const auto& s){ return s.nPresas;     }, sf::Color(100, 180, 255));
plotarSerie([](const auto& s){ return s.nPredadores; }, sf::Color(255, 80,  80));
```

### Legenda do gráfico (3 linhas coloridas + texto):

```cpp
// Abaixo do gráfico, y = gY + gH + 8
struct LegendaItem { sf::Color cor; std::string label; };
std::array<LegendaItem, 3> legenda = {{
    {sf::Color(34,  180, 34),  "Plantas"},
    {sf::Color(100, 180, 255), "Presas"},
    {sf::Color(255, 80,  80),  "Predadores"}
}};

for (int i = 0; i < 3; ++i) {
    float lx = gX + i * (gW / 3.f);
    float ly = gY + gH + 10.f;
    // Quadrado colorido
    sf::RectangleShape quad(sf::Vector2f(10.f, 10.f));
    quad.setPosition(lx, ly);
    quad.setFillColor(legenda[i].cor);
    window.draw(quad);
    // Texto
    if (fontCarregada) {
        sf::Text txt(legenda[i].label, font, 11);
        txt.setFillColor(sf::Color(200, 200, 200));
        txt.setPosition(lx + 14.f, ly - 1.f);
        window.draw(txt);
    }
}
```

---

## Passo 6 — Botões de controle

Crie uma struct `Botao` simples (não uma classe) para evitar overengineering:

```cpp
// Em src/core/UI.hpp (novo arquivo, header-only):
#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <functional>

struct Botao {
    sf::RectangleShape shape;
    sf::Text           label;
    sf::Color          corNormal   = sf::Color(50,  50,  65);
    sf::Color          corHover    = sf::Color(70,  70,  90);
    sf::Color          corAtivo    = sf::Color(80,  120, 180);
    bool               ativo       = false;   // estado toggle (ex: pausado)

    void setPosition(float x, float y) {
        shape.setPosition(x, y);
        // centralizar texto no botão
        auto bounds = label.getLocalBounds();
        label.setPosition(
            x + (shape.getSize().x - bounds.width)  / 2.f - bounds.left,
            y + (shape.getSize().y - bounds.height) / 2.f - bounds.top
        );
    }

    bool contemPonto(sf::Vector2f p) const {
        return shape.getGlobalBounds().contains(p);
    }

    void desenhar(sf::RenderWindow& w, sf::Vector2f mousePos) {
        if (ativo)
            shape.setFillColor(corAtivo);
        else if (contemPonto(mousePos))
            shape.setFillColor(corHover);
        else
            shape.setFillColor(corNormal);
        w.draw(shape);
        w.draw(label);
    }
};
```

### Três botões na Seção 3 (y ≈ 330):

```
[  ⏸ Pausar  ]  [  ⏩ 2×  ]  [  ↺ Reiniciar  ]
```

```cpp
// Em Renderer, crie e inicialize em inicializarUI():
Botao btnPausar, btnAcelerar, btnReiniciar;

void Renderer::inicializarUI(const sf::Font& f) {
    float by = 330.f;
    float bh = 32.f;
    float bw = (painelW - 30.f) / 3.f;  // ~83px cada

    auto initBtn = [&](Botao& b, const std::string& txt, float bx) {
        b.shape.setSize({bw, bh});
        b.shape.setOutlineThickness(1.f);
        b.shape.setOutlineColor(sf::Color(80, 80, 100));
        b.label.setFont(f);
        b.label.setString(txt);
        b.label.setCharacterSize(12);
        b.label.setFillColor(sf::Color(220, 220, 220));
        b.setPosition(bx, by);
    };

    initBtn(btnPausar,    "Pausar",     painelX + 10.f);
    initBtn(btnAcelerar,  "2x",         painelX + 10.f + bw + 5.f);
    initBtn(btnReiniciar, "Reiniciar",  painelX + 10.f + (bw + 5.f) * 2.f);
}
```

### Processar cliques nos botões em `processarEventos()`:

```cpp
case sf::Event::MouseButtonPressed:
    if (event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f pos(event.mouseButton.x, event.mouseButton.y);

        if (btnPausar.contemPonto(pos)) {
            eventosPausa    = true;
            btnPausar.ativo = !btnPausar.ativo;
        }
        if (btnAcelerar.contemPonto(pos)) {
            eventosAcelerar = true;
        }
        if (btnReiniciar.contemPonto(pos)) {
            eventosReiniciar = true;
        }
    }
    break;
```

### Label do botão Acelerar reflete velocidade atual:

```cpp
// Chamado em main.cpp após processar eventos:
renderer.atualizarLabelVelocidade(velocidade);
// Implementação em Renderer:
void atualizarLabelVelocidade(int v) {
    btnAcelerar.label.setString(std::to_string(v) + "x");
    // re-centralizar o label
    auto b = btnAcelerar.label.getLocalBounds();
    auto p = btnAcelerar.shape.getPosition();
    auto s = btnAcelerar.shape.getSize();
    btnAcelerar.label.setPosition(
        p.x + (s.x - b.width)  / 2.f - b.left,
        p.y + (s.y - b.height) / 2.f - b.top
    );
}
```

---

## Passo 7 — Sliders

Crie uma struct `Slider` em `src/core/UI.hpp`:

```cpp
struct Slider {
    sf::RectangleShape trilho;
    sf::CircleShape    handle;
    sf::Text           labelTxt;
    sf::Text           valorTxt;

    float minVal, maxVal, valor;
    bool  arrastando = false;

    void init(const sf::Font& f, const std::string& lbl,
              float x, float y, float largura,
              float vmin, float vmax, float vinicial) {
        minVal = vmin; maxVal = vmax; valor = vinicial;

        trilho.setSize({largura, 4.f});
        trilho.setPosition(x, y + 14.f);
        trilho.setFillColor(sf::Color(60, 60, 80));

        handle.setRadius(7.f);
        handle.setOrigin(7.f, 7.f);
        handle.setFillColor(sf::Color(100, 150, 220));
        handle.setOutlineThickness(1.f);
        handle.setOutlineColor(sf::Color(140, 180, 255));
        atualizarHandlePos(x, largura);

        labelTxt.setFont(f); labelTxt.setCharacterSize(11);
        labelTxt.setFillColor(sf::Color(180, 180, 200));
        labelTxt.setString(lbl);
        labelTxt.setPosition(x, y);

        valorTxt.setFont(f); valorTxt.setCharacterSize(11);
        valorTxt.setFillColor(sf::Color(220, 220, 255));
        atualizarValorTxt(x, largura, y);
    }

    // Atualiza posição do handle dado o valor atual
    void atualizarHandlePos(float trilhoX, float trilhoW) {
        float t = (valor - minVal) / (maxVal - minVal);
        handle.setPosition(trilhoX + t * trilhoW,
                           trilho.getPosition().y + 2.f);
    }

    void atualizarValorTxt(float trilhoX, float trilhoW, float y) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << valor;
        valorTxt.setString(oss.str());
        valorTxt.setPosition(trilhoX + trilhoW + 6.f, y);
    }

    // Retorna true se o valor mudou
    bool processarMouse(sf::Event& ev, float trilhoX, float trilhoW) {
        if (ev.type == sf::Event::MouseButtonPressed &&
            ev.mouseButton.button == sf::Mouse::Left) {
            if (handle.getGlobalBounds().contains(
                    ev.mouseButton.x, ev.mouseButton.y))
                arrastando = true;
        }
        if (ev.type == sf::Event::MouseButtonReleased)
            arrastando = false;

        if (arrastando && ev.type == sf::Event::MouseMoved) {
            float t = (ev.mouseMove.x - trilhoX) / trilhoW;
            t = std::clamp(t, 0.f, 1.f);
            float novoValor = minVal + t * (maxVal - minVal);
            if (std::abs(novoValor - valor) > 0.001f) {
                valor = novoValor;
                atualizarHandlePos(trilhoX, trilhoW);
                atualizarValorTxt(trilhoX, trilhoW,
                                  labelTxt.getPosition().y);
                return true;
            }
        }
        return false;
    }

    void desenhar(sf::RenderWindow& w) {
        w.draw(trilho);
        w.draw(handle);
        w.draw(labelTxt);
        w.draw(valorTxt);
    }
};
```

### Os 7 sliders e suas posições (Seção 4, y ≈ 430):

```cpp
// Em Renderer, declare array de sliders:
std::array<Slider, 7> sliders;

// Em inicializarUI():
struct SliderDef {
    std::string label;
    float minVal, maxVal, inicial;
};
std::array<SliderDef, 7> defs = {{
    {"Reprod. Presa",     0.1f, 3.0f, 1.0f},
    {"Reprod. Predador",  0.1f, 3.0f, 1.0f},
    {"Veloc. Presa",      0.1f, 3.0f, 1.0f},
    {"Veloc. Predador",   0.1f, 3.0f, 1.0f},
    {"Spawn Plantas",     0.1f, 5.0f, 1.0f},
    {"Visão Presa",       0.25f,4.0f, 1.0f},
    {"Visão Predador",    0.25f,4.0f, 1.0f}
}};

float sx     = painelX + 10.f;
float sLarg  = painelW - 70.f;  // deixa espaço para o valor à direita
float sy     = 430.f;
float sEsp   = 46.f;            // espaçamento vertical entre sliders

for (int i = 0; i < 7; ++i) {
    sliders[i].init(font, defs[i].label, sx, sy + i * sEsp,
                    sLarg, defs[i].minVal, defs[i].maxVal, defs[i].inicial);
}
```

### Propagar valores dos sliders para `SimParams`:

Em `processarEventos()`, após detectar que um slider mudou:

```cpp
// Mapeamento direto: índice do slider → campo de SimParams
world.params.taxaReproducaoPresa    = sliders[0].valor;
world.params.taxaReproducaoPredador = sliders[1].valor;
world.params.velocidadePresa        = sliders[2].valor;
world.params.velocidadePredador     = sliders[3].valor;
world.params.taxaSpawnPlanta        = sliders[4].valor;
world.params.raioVisaoPresa         = sliders[5].valor;
world.params.raioVisaoPredador      = sliders[6].valor;
```

---

## Passo 8 — Seção de estatísticas

Renderize no topo do painel (y ≈ 8 a 115):

```cpp
void Renderer::renderizarEstatisticas(const WorldManager& world,
                                       int nPlantas, int nPresas,
                                       int nPredadores) {
    if (!fontCarregada) return;

    auto linha = [&](const std::string& txt, float y, sf::Color cor) {
        sf::Text t(txt, font, 13);
        t.setFillColor(cor);
        t.setPosition(painelX + 10.f, y);
        window.draw(t);
    };

    linha("Tick: " + std::to_string(world.getTickAtual()),
          8.f, sf::Color(220, 220, 220));
    linha("Plantas:     " + std::to_string(nPlantas),
          26.f, sf::Color(34,  200, 34));
    linha("Presas:      " + std::to_string(nPresas),
          44.f, sf::Color(100, 180, 255));
    linha("Predadores:  " + std::to_string(nPredadores),
          62.f, sf::Color(255, 80,  80));

    // Taxa de variação (delta desde último snapshot)
    const auto& hist = world.getHistorico();
    if (hist.size() >= 2) {
        auto& prev = hist[hist.size() - 2];
        auto& curr = hist.back();
        auto delta = [](int a, int b) -> std::string {
            int d = b - a;
            return (d >= 0 ? "+" : "") + std::to_string(d);
        };
        linha("Δ Presas: " + delta(prev.nPresas, curr.nPresas)
            + "  Δ Pred: " + delta(prev.nPredadores, curr.nPredadores),
            80.f, sf::Color(150, 150, 170));
    }

    // Separador
    sf::RectangleShape sep(sf::Vector2f(painelW - 20.f, 1.f));
    sep.setPosition(painelX + 10.f, 112.f);
    sep.setFillColor(sf::Color(60, 60, 80));
    window.draw(sep);
}
```

---

## Passo 9 — Fundo e separadores do painel

Antes de renderizar qualquer elemento do painel, pinte o fundo:

```cpp
void Renderer::renderizarFundoPainel() {
    // Fundo do painel
    sf::RectangleShape bg(sf::Vector2f(painelW, window.getSize().y));
    bg.setPosition(painelX, 0.f);
    bg.setFillColor(sf::Color(18, 18, 25));
    window.draw(bg);

    // Borda esquerda do painel (separa do grid)
    sf::RectangleShape borda(sf::Vector2f(1.f, window.getSize().y));
    borda.setPosition(painelX, 0.f);
    borda.setFillColor(sf::Color(60, 60, 80));
    window.draw(borda);
}
```

---

## Passo 10 — Reiniciar simulação

Quando o botão Reiniciar for pressionado, o `WorldManager` precisa resetar
tudo sem ser destruído (o `Renderer` mantém referência a ele).

### Adicione em `WorldManager`:

```cpp
void reiniciar(int nPlantas, int nPresas, int nPredadores) {
    organismos.clear();
    historicoPopulacao.clear();
    tickAtual = 0;
    params    = SimParams{};   // resetar parâmetros para padrão
    inicializar(largura, altura, nPlantas, nPresas, nPredadores);
}
```

### Em `main.cpp`:

```cpp
if (renderer.foiReiniciado()) {
    world.reiniciar(150, 40, 10);
    pausado    = false;
    velocidade = 1;
    renderer.resetarSliders();       // volta sliders para posição central
    renderer.atualizarLabelVelocidade(1);
    btnPausar.ativo = false;
}
```

### `resetarSliders()` em `Renderer`:

```cpp
void Renderer::resetarSliders() {
    for (auto& s : sliders) {
        s.valor = 1.0f;   // todos os multiplicadores voltam a 1×
        s.atualizarHandlePos(s.trilho.getPosition().x,
                              s.trilho.getSize().x);
        s.atualizarValorTxt(s.trilho.getPosition().x,
                             s.trilho.getSize().x,
                             s.labelTxt.getPosition().y);
    }
}
```

---

## Passo 11 — `renderizar()` completo e ordenado

A ordem de renderização importa (painter's algorithm):

```cpp
void Renderer::renderizar(const WorldManager& world,
                           int tick, int nPlantas,
                           int nPresas, int nPredadores) {
    window.clear(sf::Color(15, 15, 20));

    // 1. Mundo (com câmera)
    window.setView(cameraView);
    renderizarGrid();
    for (const auto& org : world.getOrganismos())
        if (org->estaVivo())
            org->desenhar(window, cellSize);

    // 2. Painel e HUD (view padrão, fixo na tela)
    window.setView(window.getDefaultView());
    renderizarFundoPainel();
    renderizarEstatisticas(world, nPlantas, nPresas, nPredadores);
    renderizarGrafico(world);

    // 3. Botões e sliders
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    btnPausar.desenhar(window, mousePos);
    btnAcelerar.desenhar(window, mousePos);
    btnReiniciar.desenhar(window, mousePos);
    for (auto& s : sliders) s.desenhar(window);

    // 4. Overlay de pausa (por cima de tudo)
    if (pausado) renderizarOverlayPausa();

    window.display();
}
```

### Overlay de pausa:

```cpp
void Renderer::renderizarOverlayPausa() {
    // Retângulo semi-transparente sobre o grid apenas
    sf::RectangleShape overlay(sf::Vector2f(gridLargura * cellSize, 40.f));
    overlay.setPosition(0.f, gridAltura * cellSize / 2.f - 20.f);
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    window.draw(overlay);

    if (fontCarregada) {
        sf::Text txt("PAUSADO", font, 24);
        txt.setFillColor(sf::Color(255, 220, 100));
        auto b = txt.getLocalBounds();
        txt.setPosition(
            (gridLargura * cellSize - b.width)  / 2.f,
            gridAltura  * cellSize / 2.f - b.height / 2.f - b.top
        );
        window.draw(txt);
    }
}
```

---

## Estrutura de arquivos a criar/modificar

| Arquivo | Ação |
|---|---|
| `src/core/SimParams.hpp` | **Criar** |
| `src/core/UI.hpp` | **Criar** (`Botao` e `Slider`) |
| `src/core/WorldManager.hpp / .cpp` | **Modificar**: `SimParams params`, `historicoPopulacao`, `registrarSnapshot()`, `reiniciar()`, getters |
| `src/core/Renderer.hpp / .cpp` | **Modificar**: painel, gráfico, botões, sliders, overlay |
| `src/main.cpp` | **Modificar**: tamanho da janela, reiniciar, propagar eventos |

---

## Ordem de implementação recomendada

1. `SimParams.hpp` → adicionar `params` em `WorldManager` → confirmar que compila
2. `registrarSnapshot()` + `getHistorico()` → testar imprimindo no terminal
3. Redimensionar janela e ajustar `cameraView` viewport → confirmar que grid não invade painel
4. `renderizarFundoPainel()` → painel aparece sólido, sem conteúdo ainda
5. `renderizarEstatisticas()` → texto de contagem no painel
6. `renderizarGrafico()` mínimo → só o fundo e as linhas de referência
7. Plotar as 3 séries do gráfico + legenda
8. Criar `Botao` em `UI.hpp` → adicionar os 3 botões → testar clique
9. Criar `Slider` em `UI.hpp` → adicionar os 7 sliders → testar arraste
10. Propagar sliders para `SimParams` → testar se velocidade muda na simulação
11. Implementar `reiniciar()` + botão Reiniciar
12. Overlay de pausa
13. Ajuste visual final (cores, espaçamento, alinhamentos)

---

## Critério de aceite desta fase

- Janela exibe grid + painel lateral de 280px sem sobreposição
- Gráfico de linhas mostra evolução de Plantas (verde), Presas (azul) e Predadores (vermelho)
- Gráfico atualiza a cada tick em tempo real
- Botão Pausar para/retoma simulação e muda de aparência quando ativo
- Botão Acelerar cicla entre 1×, 2×, 4× e 8× (label atualiza)
- Botão Reiniciar reseta simulação e sliders sem fechar a janela
- 7 sliders respondem ao arraste e alteram o comportamento visivelmente
- Zoom/câmera continuam funcionando igual à Fase 3
- `--terminal` continua funcionando
- Zero warnings com `-Wall -Wextra -Wpedantic`
- 60fps estável com grid 100×100, 300+ entidades e painel ativo

---

## Armadilhas comuns — evite-as

**1. Eventos de slider interceptando câmera**
Cliques e drags no painel não devem mover a câmera. Verifique se o `mouseX`
está dentro da área do painel (`>= painelX`) antes de processar câmera.

**2. `sf::VertexArray` com índices fora do histórico**
Se `historicoPopulacao` estiver vazia ou tiver um único elemento, `hist.size() - 1`
dá underflow (unsigned). Sempre verifique `hist.size() >= 2` antes de plotar.

**3. Multiplicadores em `SimParams` sendo ignorados silenciosamente**
Cada multiplicador precisa ser aplicado em *exatamente um lugar* no código.
Se esquecer de aplicar `velocidadePresa` em `Presa::mover()`, o slider funciona
mas a simulação não muda — difícil de detectar. Deixe um comentário
`// aplicando params.velocidadePresa` no ponto de uso.

**4. `reiniciar()` sem limpar `historicoPopulacao`**
O gráfico mostrará dados antigos misturados com os novos. Sempre chame
`historicoPopulacao.clear()` no início de `reiniciar()`.

**5. View não restaurada antes do painel**
Se a `cameraView` estiver ativa quando o painel for desenhado, os elementos
do painel aparecem em posições erradas ou fora da tela. A linha
`window.setView(window.getDefaultView())` antes de renderizar o painel é obrigatória.

**6. Slider com `trilhoW` incluindo margem do valor**
O valor numérico (ex.: "1.50") é renderizado à direita do trilho. Se o trilho
for muito largo, o texto fica fora do painel. Use `painelW - 70.f` para o
comprimento do trilho e os 60px restantes acomodam o texto.

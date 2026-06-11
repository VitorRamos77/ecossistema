# Prompt — Fase 3: Visualização com SFML (C++17)

## Contexto obrigatório — leia antes de qualquer coisa

As Fases 1 e 2 estão implementadas e funcionando. A simulação roda no terminal com
steering behaviors, FSM e flocking operacionais. Esta fase adiciona renderização 2D
em tempo real via SFML 2.6, sem alterar nenhuma lógica de simulação.

**Regras de ouro para esta fase:**
1. **Lógica de simulação é intocável.** `agir()`, `mover()`, `WorldManager::tick()` — zero alterações.
2. **`desenhar()` muda de assinatura.** Este é o único método das entidades que será modificado.
3. **Separação estrita**: simulação não conhece SFML; SFML não conhece lógica de simulação.
4. Antes de escrever qualquer código, leia todos os `.hpp` existentes.

---

## Passo 0 — Leitura obrigatória do projeto atual

1. Leia `src/core/Organismo.hpp` — assinatura atual de `desenhar()`
2. Leia `src/core/WorldManager.hpp` — métodos públicos disponíveis
3. Leia `src/entities/Presa.hpp`, `Predador.hpp`, `Planta.hpp`
4. Leia `CMakeLists.txt` — verifique se SFML já está comentado/previsto
5. Confirme as dimensões do grid (ex.: 100×100) e onde estão definidas

Só prossiga após essa leitura.

---

## Passo 1 — Configuração do CMake

Modifique `CMakeLists.txt` para incluir SFML 2.6.

### Estratégia de busca (use nesta ordem de prioridade):

```cmake
# Tentativa 1: pacote do sistema (Linux/macOS com brew ou apt)
find_package(SFML 2.6 COMPONENTS graphics window system)

# Se não encontrar, tente FetchContent como fallback:
if(NOT SFML_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG        2.6.1
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(SFML)
endif()
```

### Link libraries:

```cmake
target_link_libraries(ecosystem_sim PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)
```

### Flags de compilação para SFML no Windows (adicionar condicionalmente):

```cmake
if(WIN32)
    target_compile_definitions(ecosystem_sim PRIVATE SFML_STATIC)
endif()
```

**Teste imediato:** após editar o CMake, rode `cmake -B build && cmake --build build`
e confirme que compila antes de continuar. Não escreva nenhum código SFML ainda.

---

## Passo 2 — Nova assinatura de `desenhar()`

A assinatura atual de `desenhar()` imprime no terminal. Ela precisa mudar para receber
o `sf::RenderWindow` e o tamanho de célula.

### Em `Organismo.hpp`, altere:

```cpp
// ANTES (Fase 1):
virtual void desenhar() const = 0;

// DEPOIS (Fase 3):
#include <SFML/Graphics.hpp>
virtual void desenhar(sf::RenderWindow& window, float cellSize) const = 0;
```

**Atenção:** isso quebra a compilação intencionalmente até que todas as subclasses
(`Planta`, `Presa`, `Predador`) também atualizem suas implementações. Atualize as
três antes de tentar compilar novamente.

### Remova o `#include <SFML/Graphics.hpp>` de `Organismo.hpp`

Em vez disso, use forward declaration para não contaminar toda a hierarquia com
o cabeçalho pesado da SFML:

```cpp
// Em Organismo.hpp — use forward declaration:
namespace sf { class RenderWindow; }

// O include completo vai apenas em Organismo.cpp e nas implementações concretas
```

---

## Passo 3 — Implementação de `desenhar()` nas entidades

### Lógica de cor por energia (comum a Presa e Predador)

A cor interpola linearmente entre vermelho (energia baixa) e verde (energia alta):

```cpp
// Função auxiliar — coloque em um header utilitário src/core/ColorUtils.hpp
sf::Color energiaParaCor(float energiaAtual, float energiaMaxima) {
    float t = std::clamp(energiaAtual / energiaMaxima, 0.f, 1.f);
    // t=0 → vermelho puro, t=1 → verde puro
    sf::Uint8 r = static_cast<sf::Uint8>((1.f - t) * 220);
    sf::Uint8 g = static_cast<sf::Uint8>(t * 200);
    sf::Uint8 b = 30;
    return sf::Color(r, g, b);
}
```

### `Planta::desenhar()`

```cpp
void Planta::desenhar(sf::RenderWindow& window, float cellSize) const {
    sf::RectangleShape shape(sf::Vector2f(cellSize - 1.f, cellSize - 1.f));
    shape.setPosition(x * cellSize, y * cellSize);
    shape.setFillColor(sf::Color(34, 139, 34));   // verde floresta, fixo
    window.draw(shape);
}
```

### `Presa::desenhar()`

```cpp
void Presa::desenhar(sf::RenderWindow& window, float cellSize) const {
    sf::CircleShape shape(cellSize * 0.4f);
    shape.setPosition(x * cellSize + cellSize * 0.1f,
                      y * cellSize + cellSize * 0.1f);
    shape.setFillColor(energiaParaCor(energia, ENERGIA_INICIAL * 2.f));
    window.draw(shape);
}
```

### `Predador::desenhar()`

```cpp
// Triângulo apontando na direção de movimento
void Predador::desenhar(sf::RenderWindow& window, float cellSize) const {
    sf::ConvexShape shape(3);
    float s = cellSize * 0.45f;
    // Triângulo centrado na célula
    shape.setPoint(0, sf::Vector2f( s,    0.f));   // ponta (frente)
    shape.setPoint(1, sf::Vector2f(-s*0.6f,  s*0.7f));
    shape.setPoint(2, sf::Vector2f(-s*0.6f, -s*0.7f));

    // Rotacionar na direção atual de vx, vy
    float angulo = std::atan2(vy, vx) * 180.f / 3.14159f;
    shape.setRotation(angulo);
    shape.setPosition(x * cellSize + cellSize * 0.5f,
                      y * cellSize + cellSize * 0.5f);
    shape.setFillColor(energiaParaCor(energia, ENERGIA_INICIAL * 2.f));
    window.draw(shape);
}
```

---

## Passo 4 — Renderer e loop principal

Crie `src/core/Renderer.hpp` e `src/core/Renderer.cpp`.

O `Renderer` é responsável exclusivamente por desenhar — não chama `tick()`,
não altera estado de nenhum organismo.

### `Renderer.hpp`

```cpp
#pragma once
#include <SFML/Graphics.hpp>
#include "WorldManager.hpp"

class Renderer {
public:
    Renderer(int gridLargura, int gridAltura,
             float cellSize, const std::string& titulo);

    // Retorna false se a janela foi fechada
    bool processarEventos();

    void renderizar(const WorldManager& world, int tick,
                    int nPlantas, int nPresas, int nPredadores);

    bool estaAberto() const { return window.isOpen(); }

    sf::RenderWindow& getWindow() { return window; }

private:
    sf::RenderWindow window;
    sf::View         cameraView;
    sf::Font         font;
    bool             fontCarregada = false;
    float            cellSize;
    int              gridLargura, gridAltura;

    // Câmera
    float zoomAtual    = 1.f;
    float cameraMoveSpeed = 200.f;  // pixels/segundo

    void renderizarGrid();
    void renderizarHUD(int tick, int nPlantas,
                       int nPresas, int nPredadores);
    void processarMovimentoCamera(float deltaTime);
};
```

### `Renderer.cpp` — construtor

```cpp
Renderer::Renderer(int gridLargura, int gridAltura,
                   float cellSize, const std::string& titulo)
    : cellSize(cellSize), gridLargura(gridLargura), gridAltura(gridAltura)
{
    unsigned int winW = static_cast<unsigned int>(gridLargura * cellSize);
    unsigned int winH = static_cast<unsigned int>(gridAltura  * cellSize);

    window.create(sf::VideoMode(winW, winH), titulo,
                  sf::Style::Default);
    window.setFramerateLimit(60);

    cameraView = window.getDefaultView();

    // Tente carregar fonte — fallback gracioso se não encontrar
    if (!font.loadFromFile("assets/font.ttf")) {
        fontCarregada = false;
        // Sem fonte, o HUD não será desenhado — não é erro fatal
    } else {
        fontCarregada = true;
    }
}
```

**Importante:** crie a pasta `assets/` na raiz do projeto e adicione uma fonte TTF
(ex.: `DejaVuSans.ttf`, disponível em dejavu-fonts.github.io, licença livre).
Atualize o CMakeLists para copiar `assets/` para o diretório de build:

```cmake
add_custom_command(TARGET ecosystem_sim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:ecosystem_sim>/assets"
)
```

---

## Passo 5 — Sistema de câmera

A câmera usa `sf::View` com zoom e translação via teclado e scroll do mouse.

### Controles:

| Entrada | Ação |
|---|---|
| `WASD` ou setas | Mover câmera |
| Scroll do mouse para cima | Zoom in (mínimo 0.2×) |
| Scroll do mouse para baixo | Zoom out (máximo 4.0×) |
| `R` | Resetar câmera para posição inicial |
| `Espaço` | Pausar/retomar simulação |
| `F` | Acelerar simulação (dobra velocidade, máx. 8×) |
| `Escape` | Fechar janela |

### Implementação em `Renderer`:

```cpp
void Renderer::processarMovimentoCamera(float deltaTime) {
    float speed = cameraMoveSpeed * zoomAtual * deltaTime;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        cameraView.move(0.f, -speed);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        cameraView.move(0.f, speed);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        cameraView.move(-speed, 0.f);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        cameraView.move(speed, 0.f);
}

// No loop de eventos (processarEventos):
case sf::Event::MouseWheelScrolled:
    zoomAtual *= (event.mouseWheelScroll.delta > 0) ? 0.9f : 1.1f;
    zoomAtual  = std::clamp(zoomAtual, 0.2f, 4.0f);
    cameraView.setSize(window.getDefaultView().getSize() * zoomAtual);
    break;

case sf::Event::KeyPressed:
    if (event.key.code == sf::Keyboard::R) {
        cameraView = window.getDefaultView();
        zoomAtual  = 1.f;
    }
    break;
```

### Aplicar view antes de renderizar organismos, restaurar para HUD:

```cpp
void Renderer::renderizar(...) {
    window.clear(sf::Color(15, 15, 20));   // fundo quase preto

    // Renderizar mundo com câmera
    window.setView(cameraView);
    renderizarGrid();
    for (const auto& org : world.getOrganismos()) {
        if (org->estaVivo())
            org->desenhar(window, cellSize);
    }

    // HUD sempre na view padrão (fixo na tela)
    window.setView(window.getDefaultView());
    renderizarHUD(tick, nPlantas, nPresas, nPredadores);

    window.display();
}
```

**Exposição necessária em `WorldManager`:** adicione getter read-only:

```cpp
const std::vector<std::unique_ptr<Organismo>>& getOrganismos() const {
    return organismos;
}
```

---

## Passo 6 — Grade de fundo

Renderize linhas de grid apenas quando o zoom estiver suficientemente ampliado
(evita ruído visual em zoom out):

```cpp
void Renderer::renderizarGrid() {
    if (zoomAtual > 0.6f) {  // só desenha grid se zoom > 60%
        sf::Color corGrade(30, 30, 38);  // cinza muito escuro
        sf::Vertex linha[2];
        linha[0].color = linha[1].color = corGrade;

        for (int col = 0; col <= gridLargura; ++col) {
            linha[0].position = {col * cellSize, 0.f};
            linha[1].position = {col * cellSize, gridAltura * cellSize};
            window.draw(linha, 2, sf::Lines);
        }
        for (int row = 0; row <= gridAltura; ++row) {
            linha[0].position = {0.f,              row * cellSize};
            linha[1].position = {gridLargura * cellSize, row * cellSize};
            window.draw(linha, 2, sf::Lines);
        }
    }
}
```

---

## Passo 7 — HUD (informações na tela)

O HUD é desenhado por cima, na view padrão, no canto superior esquerdo.
Se a fonte não carregou, pule silenciosamente.

```cpp
void Renderer::renderizarHUD(int tick, int nPlantas,
                              int nPresas, int nPredadores) {
    if (!fontCarregada) return;

    auto texto = [&](const std::string& str, float y, sf::Color cor) {
        sf::Text t(str, font, 14);
        t.setFillColor(cor);
        t.setPosition(8.f, y);
        window.draw(t);
    };

    texto("Tick: "      + std::to_string(tick),        8.f,  sf::Color::White);
    texto("Plantas: "   + std::to_string(nPlantas),   26.f,  sf::Color(34, 200, 34));
    texto("Presas: "    + std::to_string(nPresas),    44.f,  sf::Color(100, 180, 255));
    texto("Predadores: "+ std::to_string(nPredadores),62.f,  sf::Color(255, 80, 80));
    texto("Zoom: "      + std::to_string(int(1.f/zoomAtual * 100)) + "%",
                                                       80.f,  sf::Color(180, 180, 180));

    // Controles (canto inferior esquerdo)
    unsigned int winH = window.getSize().y;
    texto("WASD: câmera | Scroll: zoom | R: reset | Espaço: pausar | F: acelerar",
          winH - 22.f, sf::Color(120, 120, 120));
}
```

---

## Passo 8 — Loop principal em `main.cpp`

Substitua o loop de terminal pelo loop SFML. Mantenha a capacidade de rodar
em modo terminal (flag `--terminal`) para debug.

### Estrutura do loop:

```cpp
#include "core/Renderer.hpp"
#include "core/WorldManager.hpp"

int main(int argc, char* argv[]) {
    // Modo terminal para debug
    bool modoTerminal = (argc > 1 && std::string(argv[1]) == "--terminal");

    const int   GRID_W    = 100;
    const int   GRID_H    = 100;
    const float CELL_SIZE = 8.f;    // pixels por célula

    WorldManager world;
    world.inicializar(GRID_W, GRID_H,
                      /*plantas=*/150, /*presas=*/40, /*predadores=*/10);

    if (modoTerminal) {
        // Loop terminal da Fase 1/2 (mantido intacto)
        for (int t = 0; t < 500; ++t) {
            world.tick();
            world.renderizar();   // imprime no terminal
        }
        return 0;
    }

    // Loop SFML
    Renderer renderer(GRID_W, GRID_H, CELL_SIZE, "Simulador de Ecossistema");

    sf::Clock clock;
    float     acumulador    = 0.f;
    float     tickInterval  = 1.f / 10.f;   // 10 ticks/segundo base
    int       velocidade    = 1;             // multiplicador (1, 2, 4, 8)
    bool      pausado       = false;
    int       tickAtual     = 0;

    while (renderer.estaAberto()) {
        float deltaTime = clock.restart().asSeconds();
        deltaTime = std::min(deltaTime, 0.05f);  // cap para evitar spiral of death

        // Processar eventos (pausa, velocidade, câmera, fechar)
        // processarEventos() retorna false se a janela fechou
        if (!renderer.processarEventos()) break;

        // Atualizar câmera
        // (processarMovimentoCamera é chamado internamente por processarEventos)

        // Avançar simulação
        if (!pausado) {
            acumulador += deltaTime * velocidade;
            while (acumulador >= tickInterval) {
                world.tick();
                ++tickAtual;
                acumulador -= tickInterval;
            }
        }

        // Contar populações
        int nPlantas = 0, nPresas = 0, nPredadores = 0;
        for (const auto& org : world.getOrganismos()) {
            if (!org->estaVivo()) continue;
            // use TipoOrganismo ou dynamic_cast para contar
            // sugestão: adicionar virtual TipoOrganismo getTipo() const em Organismo
        }

        renderer.renderizar(world, tickAtual, nPlantas, nPresas, nPredadores);
    }

    return 0;
}
```

### Para contar populações sem `dynamic_cast` espalhado, adicione em `Organismo`:

```cpp
virtual TipoOrganismo getTipo() const = 0;
```

Implemente em cada subclasse retornando o enum correspondente.
Isso também será útil na Fase 4 para o gráfico de populações.

---

## Passo 9 — Integrar pausa e velocidade nos eventos

Em `Renderer::processarEventos()`, capture os eventos de pausa e velocidade
e os exponha via getters:

```cpp
// Em Renderer.hpp, adicione:
bool foiPausado()     { bool v = eventosPausa;     eventosPausa = false;     return v; }
bool foiAcelerado()   { bool v = eventosAcelerar;  eventosAcelerar = false;  return v; }

private:
    bool eventosPausa    = false;
    bool eventosAcelerar = false;
```

Em `main.cpp`:
```cpp
if (renderer.foiPausado())   pausado   = !pausado;
if (renderer.foiAcelerado()) velocidade = std::min(velocidade * 2, 8);
```

---

## Passo 10 — cellSize e resolução recomendadas

| Grid | cellSize | Janela resultante | Quando usar |
|---|---|---|---|
| 100×100 | 8px | 800×800 | Padrão — boa visibilidade |
| 100×100 | 6px | 600×600 | Monitores menores |
| 100×100 | 10px | 1000×1000 | Monitores grandes |
| 40×20  | 16px | 640×320 | Debug/terminal |

Deixe `CELL_SIZE` como constante em `main.cpp` para fácil ajuste.

---

## Estrutura de arquivos a criar/modificar

| Arquivo | Ação |
|---|---|
| `CMakeLists.txt` | **Modificar**: adicionar SFML, copiar assets |
| `assets/font.ttf` | **Criar**: baixar DejaVuSans.ttf (licença livre) |
| `src/core/ColorUtils.hpp` | **Criar**: função `energiaParaCor()` |
| `src/core/Renderer.hpp / .cpp` | **Criar** |
| `src/core/Organismo.hpp` | **Modificar**: assinatura de `desenhar()` e `getTipo()` |
| `src/core/WorldManager.hpp / .cpp` | **Modificar**: adicionar `getOrganismos()` |
| `src/entities/Planta.hpp / .cpp` | **Modificar**: implementar nova `desenhar()` |
| `src/entities/Presa.hpp / .cpp` | **Modificar**: implementar nova `desenhar()` |
| `src/entities/Predador.hpp / .cpp` | **Modificar**: implementar nova `desenhar()` |
| `src/main.cpp` | **Modificar**: loop SFML + flag `--terminal` |

---

## Ordem de implementação recomendada

1. Editar CMake → confirmar que compila com SFML linkado (sem código SFML ainda)
2. Criar `ColorUtils.hpp` (sem dependência de nada)
3. Alterar assinatura de `desenhar()` + `getTipo()` em `Organismo` → o projeto **não compila** até o passo seguinte
4. Implementar `desenhar()` e `getTipo()` nas três subclasses → projeto compila novamente
5. Criar `Renderer` mínimo: abre janela, limpa com cor sólida, fecha ao pressionar Escape
6. Adicionar `getOrganismos()` em `WorldManager`
7. Integrar `org->desenhar()` no `Renderer::renderizar()`
8. Adicionar câmera (zoom + movimento)
9. Adicionar HUD com texto
10. Adicionar grade de fundo
11. Integrar pausa e velocidade
12. Testar com `--terminal` para garantir que a lógica continua intacta

---

## Critério de aceite desta fase

- Janela SFML abre sem crash
- Grid 100×100 renderizado com plantas (verde), presas (círculo verde→vermelho) e predadores (triângulo verde→vermelho)
- Câmera move com WASD e zoom com scroll
- HUD exibe tick e contagem de populações
- `Espaço` pausa e retoma
- `F` acelera (1× → 2× → 4× → 8×)
- `R` reseta câmera
- `--terminal` continua funcionando igual à Fase 2
- Zero warnings com `-Wall -Wextra -Wpedantic`
- Framerate estável a 60fps com grid 100×100 e 200+ entidades

---

## Armadilhas comuns — evite-as

**1. Incluir `<SFML/Graphics.hpp>` em `Organismo.hpp`**
Contamina todo o projeto com o cabeçalho SFML. Use forward declaration no `.hpp`
e o include completo apenas no `.cpp`.

**2. Chamar `world.tick()` dentro do `Renderer`**
O `Renderer` apenas desenha. Atualização de simulação é responsabilidade de `main.cpp`.

**3. Não usar `deltaTime` no loop**
Sem delta time, a velocidade da simulação varia com o framerate. Use `sf::Clock`
e o acumulador mostrado no Passo 8.

**4. Esquecer `window.setView(window.getDefaultView())` antes do HUD**
O HUD será desenhado com a transformação da câmera aplicada e aparecerá
em posição errada ou invisível.

**5. Desenhar a grade com `sf::RectangleShape` por célula**
Para um grid 100×100, isso são 10.000 draw calls por frame. Use `sf::VertexArray`
ou `sf::Vertex` com `sf::Lines` como mostrado no Passo 6.

**6. Não limitar `deltaTime`**
Se a janela ficar minimizada e depois restaurada, `deltaTime` pode ser vários
segundos, causando centenas de ticks num único frame. O `std::min(deltaTime, 0.05f)`
do Passo 8 previne isso.

# Prompt — Fase 1: Fundação do Simulador de Ecossistema (C++17)

## Contexto do projeto

Estou desenvolvendo um simulador de ecossistema 2D em C++17 com orientação a objetos.
O projeto terá 5 fases. Esta é a **Fase 1**, cujo objetivo é estabelecer toda a estrutura
base sem interface gráfica — apenas lógica de classes, grid e loop de simulação no terminal.

As fases futuras adicionarão: IA e steering behaviors (Fase 2), renderização SFML (Fase 3),
interface interativa (Fase 4) e extras como biomas e mutações (Fase 5). Por isso, a
arquitetura desta fase deve ser **extensível por design** — evitar acoplamento desnecessário.

---

## Stack e ferramentas

- **Linguagem**: C++17
- **Build**: CMake (mínimo 3.16)
- **Dependências desta fase**: apenas STL (`vector`, `map`, `memory`, `random`, `iostream`)
- **Dependências futuras** (não implementar agora, mas o CMake já deve prever): SFML 2.6, nlohmann/json

---

## O que deve ser implementado nesta fase

### 1. Estrutura de diretórios esperada

```
ecosystem/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── Organismo.hpp / .cpp
│   │   ├── Animal.hpp / .cpp
│   │   ├── WorldManager.hpp / .cpp
│   │   └── OrganismoFactory.hpp / .cpp
│   └── entities/
│       ├── Planta.hpp / .cpp
│       ├── Presa.hpp / .cpp
│       └── Predador.hpp / .cpp
```

---

### 2. Hierarquia de classes

#### `Organismo` (classe abstrata base)
Atributos protegidos:
- `float x, y` — posição no grid (coordenadas inteiras mapeadas como float para Fase 2)
- `float energia`
- `int idade` — incrementada a cada tick
- `int tempoParaReproducao` — contador regressivo de cooldown
- `bool vivo`

Métodos virtuais puros:
- `virtual void agir(WorldManager& world) = 0`
- `virtual void mover(WorldManager& world) = 0`
- `virtual void desenhar() const = 0` — na Fase 1 imprime um caractere no terminal; na Fase 3 será sobrescrito com SFML

Métodos concretos em `Organismo`:
- `bool estaVivo() const`
- `float getX() const`, `float getY() const`
- `float getEnergia() const`
- `void envelhecer()` — incrementa `idade`, decrementa `tempoParaReproducao`
- `void consumirEnergia(float quantidade)` — reduz energia; se chegar a zero, seta `vivo = false`

#### `Animal` (abstrata, herda de `Organismo`)
Atributos adicionais:
- `float velocidade`
- `float raioVisao`
- `float vx, vy` — vetor de direção/velocidade atual (preparação para steering na Fase 2)

Métodos adicionais:
- `virtual std::vector<Organismo*> detectarVizinhos(WorldManager& world) const`
  — retorna organismos dentro de `raioVisao` (preparação para Fase 2; na Fase 1 pode retornar vazio)

#### `Planta` (concreta, herda de `Organismo`)
- Não se move: `mover()` é no-op
- `agir()`: verifica superlotação e marca como morta se necessário; incrementa contador de espalhamento
- `desenhar()`: imprime `'P'` na posição

Constantes sugeridas (como `static constexpr`):
- `ENERGIA_INICIAL = 20.0f`
- `ENERGIA_POR_TICK = -0.0f` (plantas não consomem energia passivamente, mas morrem por superlotação)
- `INTERVALO_ESPALHAMENTO = 10` (ticks)

#### `Presa` (concreta, herda de `Animal`) — representa um Coelho
- `agir()`: consome energia por tick (-0.5f parado, -1.0f em movimento); morre se energia = 0
- `mover()`: Fase 1 = movimento aleatório simples (direção sorteada a cada tick ou a cada N ticks)
- `desenhar()`: imprime `'C'`

Constantes:
- `ENERGIA_INICIAL = 30.0f`
- `CUSTO_PARADO = 0.5f`
- `CUSTO_MOVIMENTO = 1.0f`
- `ENERGIA_AO_COMER_PLANTA = 10.0f`
- `ENERGIA_PARA_REPRODUCAO = 40.0f`
- `COOLDOWN_REPRODUCAO = 20` (ticks)
- `CUSTO_REPRODUCAO = 20.0f`

#### `Predador` (concreta, herda de `Animal`) — representa um Lobo
- `agir()`: consome energia por tick (-1.0f parado, -2.0f em movimento); morre se energia = 0
- `mover()`: Fase 1 = movimento aleatório simples
- `desenhar()`: imprime `'L'`

Constantes:
- `ENERGIA_INICIAL = 50.0f`
- `CUSTO_PARADO = 1.0f`
- `CUSTO_MOVIMENTO = 2.0f`
- `CUSTO_EXTRA_HUNTING = 1.0f` (por tick caçando — reservado para Fase 2)
- `ENERGIA_PARA_REPRODUCAO = 60.0f`
- `COOLDOWN_REPRODUCAO = 30` (ticks)
- `CUSTO_REPRODUCAO = 30.0f`

---

### 3. `WorldManager`

Responsável por gerenciar o grid e coordenar todas as interações.

Atributos:
- `int largura, altura` — dimensões do grid (padrão: 40×20 para terminal; 100×100 para Fase 3)
- `std::vector<std::unique_ptr<Organismo>> organismos` — lista flat de todos os organismos vivos
- `std::mt19937 rng` — gerador de números aleatórios centralizado (seed configurável)

Métodos:
- `void inicializar(int largura, int altura, int nPlantas, int nPresas, int nPredadores)`
  — usa `OrganismoFactory` para popular o mundo
- `void tick()` — executa um passo da simulação:
  1. Chama `agir()` e `mover()` em cada organismo vivo
  2. Resolve interações (Presa come Planta se na mesma célula; Predador captura Presa se na mesma célula)
  3. Remove organismos mortos (`vivo == false`)
  4. Tenta reprodução para organismos elegíveis
  5. Faz spawn de novas Plantas em intervalo fixo
- `void renderizar()` — imprime o grid no terminal usando os caracteres de `desenhar()`; células vazias = `'.'`
- `void adicionarOrganismo(std::unique_ptr<Organismo> org)`
- `Organismo* obterOrganismoEm(int x, int y) const`
- `bool celulaTaOcupada(int x, int y) const`
- `std::mt19937& getRng()` — acesso centralizado ao RNG

**Regras de colisão/interação a implementar na Fase 1:**
- Cada célula suporta **no máximo um organismo**
- Se Presa e Planta estão na mesma célula após o movimento: Presa come Planta (remove Planta, Presa ganha `ENERGIA_AO_COMER_PLANTA`)
- Se Predador e Presa estão na mesma célula após o movimento: captura ocorre (remove Presa, Predador ganha energia equivalente à `energiaAtual` da Presa)
- Movimentos inválidos (fora dos limites) são descartados — o organismo fica no lugar

**Spawn de Plantas:**
- A cada `INTERVALO_SPAWN_PLANTA = 5` ticks, uma nova Planta surge em posição aleatória desocupada

**Reprodução (simplificada para Fase 1):**
- Um organismo pode se reproduzir se: `energia >= ENERGIA_PARA_REPRODUCAO` e `tempoParaReproducao <= 0`
- Na Fase 1, não exige vizinho (simplificação; Fase 2 exigirá proximidade)
- O filhote surge em célula adjacente desocupada com `ENERGIA_INICIAL` da espécie
- O pai perde `CUSTO_REPRODUCAO` de energia e reseta `tempoParaReproducao`

---

### 4. `OrganismoFactory`

Implementar o padrão Factory com método estático:

```cpp
class OrganismoFactory {
public:
    static std::unique_ptr<Organismo> criar(TipoOrganismo tipo, float x, float y);
    static std::unique_ptr<Organismo> criarAleatorio(TipoOrganismo tipo,
                                                      int largura, int altura,
                                                      WorldManager& world);
};

enum class TipoOrganismo { Planta, Presa, Predador };
```

---

### 5. `main.cpp` — loop de simulação no terminal

```
Parâmetros de início sugeridos:
- Grid: 40 × 20
- Plantas iniciais: 30
- Presas iniciais: 15
- Predadores iniciais: 5
- Ticks totais: 200
- Delay entre ticks: 100ms (usar std::this_thread::sleep_for)
```

A cada tick, imprimir:
```
Tick: 42 | Plantas: 28 | Presas: 12 | Predadores: 4
[grid renderizado aqui]
```

---

## Restrições de design importantes

1. **Sem SFML nesta fase** — zero dependência gráfica. `desenhar()` imprime no terminal.
2. **`WorldManager` não deve depender de tipos concretos** (`Planta`, `Presa`, `Predador`).
   Ele opera apenas via ponteiros `Organismo*`. Apenas `OrganismoFactory` e `main.cpp` conhecem os tipos concretos.
3. **Ownership via `unique_ptr`** — `WorldManager` é dono de todos os organismos.
   Ponteiros crus (`Organismo*`) são usados apenas para leitura/acesso temporário dentro de um tick.
4. **RNG centralizado** — todo uso de aleatoriedade passa por `WorldManager::getRng()`.
   Não instanciar geradores locais nas classes.
5. **Constantes como `static constexpr`** dentro de cada classe — não usar `#define` nem constantes globais soltas.
6. **Sem herança múltipla** — a hierarquia é estritamente linear: `Organismo → Animal → Presa/Predador`.
7. **Preparar para Fase 2**: os métodos `mover()` e `agir()` devem receber `WorldManager&` como parâmetro
   (já está na assinatura virtual) para que a Fase 2 possa acessar vizinhos sem refatoração.

---

## CMakeLists.txt esperado

- Target: `ecosystem_sim`
- C++17 obrigatório (`set(CMAKE_CXX_STANDARD 17)`)
- Warnings habilitados: `-Wall -Wextra -Wpedantic`
- Comentário reservando espaço para SFML e nlohmann/json nas fases futuras
- Build types: Debug (padrão) e Release

---

## Critério de aceite desta fase

O projeto compila sem warnings com `-Wall -Wextra` e, ao rodar, exibe:
- Grid no terminal com `P`, `C`, `L` e `.` nas posições corretas
- Contador de tick e população por espécie atualizando a cada passo
- Presas morrendo de fome se não houver plantas suficientes
- Predadores morrendo se não houver presas
- Reprodução ocorrendo quando energia é suficiente
- Simulação terminando após N ticks (ou antes, se todas as espécies morrerem)

Não há interface gráfica, sliders, arquivos de configuração nem IA avançada nesta fase.

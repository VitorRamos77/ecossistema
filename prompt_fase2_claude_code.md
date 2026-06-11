# Prompt — Fase 2: Comportamento e IA simples (C++17)

## Contexto obrigatório — leia antes de qualquer coisa

A Fase 1 já está implementada e funcionando. A estrutura existente é:

```
ecosystem/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── Organismo.hpp / .cpp       ← classe abstrata base
│   │   ├── Animal.hpp / .cpp          ← abstrata, herda de Organismo
│   │   ├── WorldManager.hpp / .cpp    ← gerencia grid e tick
│   │   └── OrganismoFactory.hpp / .cpp
│   └── entities/
│       ├── Planta.hpp / .cpp
│       ├── Presa.hpp / .cpp           ← movimento aleatório simples (Fase 1)
│       └── Predador.hpp / .cpp        ← movimento aleatório simples (Fase 1)
```

**Regras de ouro para esta fase:**
1. **Não quebrar a Fase 1.** O projeto deve compilar e rodar após cada alteração.
2. **Não refatorar o que não precisa mudar.** Altere apenas o necessário para adicionar comportamento.
3. **Não adicionar SFML.** Zero dependência gráfica ainda — `desenhar()` continua imprimindo no terminal.
4. Antes de implementar qualquer coisa, leia os arquivos `.hpp` existentes para entender as assinaturas atuais.

---

## O que esta fase adiciona

- Raio de visão com detecção real de vizinhos
- Steering behaviors: Wander, Seek, Flee, Pursue, Evade
- Flocking (Boids) para presas em grupo
- FSM com 4 estados para cada agente
- Reprodução com exigência de proximidade
- Seleção de alvo por heurística de prioridade (predador)
- Balanço energético completo e correto

---

## Passo 0 — Antes de implementar

Faça isso primeiro, sem escrever código:

1. Leia todos os arquivos `.hpp` em `src/core/` e `src/entities/`
2. Confirme que `mover(WorldManager&)` e `agir(WorldManager&)` já recebem `WorldManager&`
3. Confirme que `Animal` já tem `float vx, vy` e `float raioVisao`
4. Confirme que `WorldManager` já expõe `getRng()`, `obterOrganismoEm()` e `celulaTaOcupada()`
5. Se algum desses itens estiver faltando, adicione-o antes de prosseguir

---

## Passo 1 — Sistema de detecção de vizinhos

### Em `WorldManager`, adicione:

```cpp
// Retorna ponteiros crus (não-owning) para organismos dentro do raio
std::vector<Organismo*> obterVizinhos(float cx, float cy, float raio) const;

// Variantes filtradas por tipo (evitam dynamic_cast espalhado)
std::vector<Organismo*> obterVizinhosDeTipo(float cx, float cy,
                                             float raio,
                                             TipoOrganismo tipo) const;
```

**Implementação de `obterVizinhos`:**
- Itere apenas nas células do grid dentro do bounding box `[cx-raio, cx+raio] x [cy-raio, cy+raio]`
- Complexidade: O(r²), não O(n) — não itere sobre `organismos` inteiro
- Filtre pela distância euclidiana real: `sqrt((ox-cx)²+(oy-cy)²) <= raio`
- Retorne apenas organismos com `estaVivo() == true`
- Nunca retorne o próprio organismo (compare ponteiro)

### Em `Animal`, implemente `detectarVizinhos`:

```cpp
std::vector<Organismo*> detectarVizinhos(WorldManager& world) const override {
    return world.obterVizinhos(x, y, raioVisao);
}
```

---

## Passo 2 — Steering Behaviors

Crie o arquivo `src/core/SteeringBehaviors.hpp` (header-only, funções livres ou namespace).
Não crie uma classe com estado — são funções puras que recebem dados e retornam vetores de força.

### Tipos auxiliares

```cpp
struct Vec2 {
    float x = 0.f, y = 0.f;

    Vec2 operator+(const Vec2& o) const { return {x+o.x, y+o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x-o.x, y-o.y}; }
    Vec2 operator*(float s)       const { return {x*s,   y*s};   }

    float length()  const { return std::sqrt(x*x + y*y); }
    Vec2  normalized() const {
        float l = length();
        return l > 0.0001f ? Vec2{x/l, y/l} : Vec2{0.f, 0.f};
    }
};
```

### Funções de steering

```cpp
namespace Steering {

// Wander: projeta círculo à frente e desloca ponto-alvo suavemente
// wanderAngle é estado persistente do agente (float membro de Animal)
Vec2 wander(Vec2 pos, Vec2 vel, float& wanderAngle,
            float wanderRadius, float wanderDistance,
            float wanderJitter, std::mt19937& rng);

// Seek: vai em direção ao alvo na velocidade máxima
Vec2 seek(Vec2 pos, Vec2 target, float maxSpeed);

// Flee: foge do ponto de perigo (inverso do seek)
Vec2 flee(Vec2 pos, Vec2 danger, float maxSpeed);

// Pursue: estima posição futura da presa e faz seek nela
Vec2 pursue(Vec2 pos, Vec2 preyPos, Vec2 preyVel, float maxSpeed);

// Evade: estima posição futura do predador e faz flee dela
Vec2 evade(Vec2 pos, Vec2 predPos, Vec2 predVel, float maxSpeed);

} // namespace Steering
```

### Implementação de `wander`

```cpp
Vec2 wander(Vec2 pos, Vec2 vel, float& wanderAngle,
            float wanderRadius, float wanderDistance,
            float wanderJitter, std::mt19937& rng) {

    std::uniform_real_distribution<float> jitterDist(-wanderJitter, wanderJitter);
    wanderAngle += jitterDist(rng);

    // Centro do círculo projetado à frente
    Vec2 circleCenter = vel.normalized() * wanderDistance;
    // Ponto no círculo
    Vec2 displacement{
        wanderRadius * std::cos(wanderAngle),
        wanderRadius * std::sin(wanderAngle)
    };
    return (circleCenter + displacement).normalized();
}
```

### Implementação de `pursue` e `evade`

```cpp
Vec2 pursue(Vec2 pos, Vec2 preyPos, Vec2 preyVel, float maxSpeed) {
    float dist = (preyPos - pos).length();
    float fatorTempo = dist / maxSpeed;           // tempo estimado até alcançar
    Vec2 futurePos = preyPos + preyVel * fatorTempo;
    return seek(pos, futurePos, maxSpeed);
}

Vec2 evade(Vec2 pos, Vec2 predPos, Vec2 predVel, float maxSpeed) {
    float dist = (predPos - pos).length();
    float fatorTempo = dist / maxSpeed;
    Vec2 futurePos = predPos + predVel * fatorTempo;
    return flee(pos, futurePos, maxSpeed);
}
```

---

## Passo 3 — Flocking (Boids) para Presas

Adicione em `SteeringBehaviors.hpp` dentro do namespace `Steering`:

```cpp
struct FlockingResult {
    Vec2 separacao;
    Vec2 alinhamento;
    Vec2 coesao;
};

// vizinhos: apenas outras Presas dentro do raio de visão
FlockingResult calcularFlocking(Vec2 pos, Vec2 vel,
                                 const std::vector<Vec2>& posVizinhos,
                                 const std::vector<Vec2>& velVizinhos,
                                 float raioSeparacao);
```

**Regras de Reynolds:**

| Regra | Cálculo |
|---|---|
| Separação | Para cada vizinho dentro de `raioSeparacao`: acumule `normalize(pos - vizinho.pos)`. Normalize o resultado. |
| Alinhamento | Média das velocidades dos vizinhos, normalizada. |
| Coesão | `seek(pos, centroMassa, maxSpeed)` onde `centroMassa` é a média das posições dos vizinhos. |

**Pesos padrão (como `static constexpr float` em `Presa`):**

```cpp
static constexpr float PESO_SEPARACAO  = 1.5f;
static constexpr float PESO_ALINHAMENTO = 1.0f;
static constexpr float PESO_COESAO     = 1.0f;
static constexpr float RAIO_SEPARACAO  = 2.0f; // células
```

**Força final de flocking:**

```cpp
Vec2 forcaFlocking = resultado.separacao  * PESO_SEPARACAO
                   + resultado.alinhamento * PESO_ALINHAMENTO
                   + resultado.coesao      * PESO_COESAO;
```

---

## Passo 4 — FSM (Máquina de Estados Finitos)

### Enum de estados

Adicione em `Animal.hpp` (ou em `core/EstadoAgente.hpp` se preferir separar):

```cpp
enum class EstadoAgente {
    WANDERING,   // sem alvo visível
    HUNTING,     // predador perseguindo presa      (só Predador usa)
    FORAGING,    // presa buscando planta           (só Presa usa)
    FLEEING      // presa fugindo de predador       (só Presa usa)
};
```

Adicione em `Animal`:
```cpp
protected:
    EstadoAgente estado = EstadoAgente::WANDERING;
    float        wanderAngle = 0.f;   // estado persistente para Wander
```

### Tabela de transições — avaliada a cada tick no início de `agir()`

**Para `Presa`:**

```
estado atual  → condição de transição             → novo estado
WANDERING     → predador dentro do raioVisao      → FLEEING   (prioridade máxima)
WANDERING     → planta dentro do raioVisao        → FORAGING
FORAGING      → predador dentro do raioVisao      → FLEEING   (prioridade máxima)
FORAGING      → planta desapareceu do raio        → WANDERING
FLEEING       → nenhum predador no raio por 3t    → WANDERING
```

Para o timer de "3 ticks sem predador", adicione `int ticksSemPredador = 0` em `Presa`.

**Para `Predador`:**

```
estado atual  → condição de transição             → novo estado
WANDERING     → presa dentro do raioVisao         → HUNTING
HUNTING       → presa alvo morreu ou sumiu do raio → WANDERING
```

### Tabela de ações por estado

| Estado | Entidade | Ação em `mover()` |
|---|---|---|
| WANDERING | Presa | `Steering::wander` + `Steering::calcularFlocking` (com outras presas visíveis) |
| FORAGING | Presa | `Steering::seek` até a planta mais próxima |
| FLEEING | Presa | `Steering::evade` do predador mais próximo, velocidade máxima |
| WANDERING | Predador | `Steering::wander` |
| HUNTING | Predador | `Steering::pursue` da presa selecionada |

---

## Passo 5 — Seleção de alvo do Predador

Quando entrar em estado `HUNTING`, o predador avalia todas as presas visíveis e escolhe a
de **maior pontuação de prioridade**:

```cpp
float prioridade = (1.f / (distancia + 0.001f))
                 * presa->getEnergia()
                 * fatorDirecao;

// fatorDirecao: bônus se a presa está na direção atual de movimento
Vec2 dirAtual = Vec2{vx, vy}.normalized();
Vec2 dirAlvo  = (Vec2{alvo->getX(), alvo->getY()} - Vec2{x, y}).normalized();
float dot = dirAtual.x * dirAlvo.x + dirAtual.y * dirAtual.y;
float fatorDirecao = 0.5f + 0.5f * dot;  // normaliza para [0, 1]
```

Armazene o ponteiro da presa selecionada em `Predador` como `Organismo* alvoAtual = nullptr`.
Invalide `alvoAtual` se `alvoAtual->estaVivo() == false` ou se saiu do raio de visão.

---

## Passo 6 — Integração do movimento

Em `Animal`, adicione o método:

```cpp
void aplicarForca(Vec2 forca, float maxSpeed, float larguraMundo, float alturaMundo);
```

Implementação:
```cpp
void Animal::aplicarForca(Vec2 forca, float maxSpeed,
                           float larguraMundo, float alturaMundo) {
    vx += forca.x;
    vy += forca.y;

    // Limitar à velocidade máxima
    float speed = Vec2{vx, vy}.length();
    if (speed > maxSpeed) {
        vx = (vx / speed) * maxSpeed;
        vy = (vy / speed) * maxSpeed;
    }

    // Aplicar deslocamento
    x += vx;
    y += vy;

    // Clampar dentro dos limites (wrap-around alternativo: x = fmod(x + largura, largura))
    x = std::clamp(x, 0.f, larguraMundo - 1.f);
    y = std::clamp(y, 0.f, alturaMundo  - 1.f);
}
```

**Importante:** `WorldManager` deve passar `getLargura()` e `getAltura()` para `aplicarForca`.
Adicione esses getters se ainda não existirem.

---

## Passo 7 — Balanço energético por estado

Substitua o consumo fixo da Fase 1 pelo consumo dependente de estado, em `agir()`:

**Presa:**
```cpp
switch (estado) {
    case EstadoAgente::WANDERING:
    case EstadoAgente::FORAGING:  consumirEnergia(CUSTO_MOVIMENTO); break;
    case EstadoAgente::FLEEING:   consumirEnergia(CUSTO_MOVIMENTO * 1.5f); break;
    default: break;
}
```

**Predador:**
```cpp
switch (estado) {
    case EstadoAgente::WANDERING: consumirEnergia(CUSTO_PARADO);   break;
    case EstadoAgente::HUNTING:   consumirEnergia(CUSTO_MOVIMENTO
                                               + CUSTO_EXTRA_HUNTING); break;
    default: break;
}
```

**Tabela de referência completa (do documento original):**

| Evento | Predador | Presa |
|---|---|---|
| Tick parado (WANDERING) | -1.0 | -0.5 |
| Tick em movimento | -2.0 | -1.0 |
| Tick em FLEEING | — | -1.5 |
| Tick em HUNTING | -3.0 | — |
| Capturar presa | +energia da presa | — |
| Consumir planta | — | +10.0 |
| Reprodução | -30.0 | -20.0 |
| Morte por inanição | energia = 0 | energia = 0 |

---

## Passo 8 — Reprodução com proximidade

Substitua a reprodução simplificada da Fase 1. Agora exige vizinho da mesma espécie.

Em `WorldManager::tick()`, na etapa de reprodução:

```cpp
// Para cada organismo vivo elegível (energia >= limiar E cooldown <= 0):
//   Busca vizinho da mesma espécie dentro de raio RAIO_REPRODUCAO
//   Se encontrar, cria filhote em célula adjacente desocupada
//   Pai perde CUSTO_REPRODUCAO e reseta cooldown
//   Filhote nasce com ENERGIA_INICIAL da espécie e cooldown resetado
```

Constantes novas:
```cpp
// Em Presa:
static constexpr float RAIO_REPRODUCAO = 3.0f;

// Em Predador:
static constexpr float RAIO_REPRODUCAO = 4.0f;
```

**Cuidado com iterator invalidation:** ao adicionar filhotes durante a iteração sobre
`organismos`, acumule os novos em um `vector` temporário e faça `push_back` após o loop.

---

## Passo 9 — Parâmetros de steering por entidade

Adicione como `static constexpr` nas classes correspondentes:

**Presa:**
```cpp
static constexpr float WANDER_RADIUS   = 1.5f;
static constexpr float WANDER_DISTANCE = 3.0f;
static constexpr float WANDER_JITTER   = 0.3f;
static constexpr float VELOCIDADE      = 0.6f;
static constexpr float RAIO_VISAO      = 6.0f;
static constexpr float RAIO_FUGA       = 5.0f;  // ativa FLEEING se predador < este raio
```

**Predador:**
```cpp
static constexpr float WANDER_RADIUS   = 2.0f;
static constexpr float WANDER_DISTANCE = 4.0f;
static constexpr float WANDER_JITTER   = 0.2f;
static constexpr float VELOCIDADE      = 0.8f;  // mais rápido que a presa
static constexpr float RAIO_VISAO      = 8.0f;
```

---

## Passo 10 — Verificação e testes no terminal

Não adicione interface gráfica. O critério de aceite é observável no terminal:

**Teste 1 — Fuga funciona:**
Uma presa deve mudar para `FLEEING` quando um predador entra no seu raio. Adicione
temporariamente um log em `agir()` da Presa: `[PRESA] estado → FLEEING`.

**Teste 2 — Predador persegue:**
Um predador deve perseguir uma presa e capturá-la. Log: `[PREDADOR] capturou presa, energia: X`.

**Teste 3 — Boids visível:**
Com 10+ presas, elas devem se mover em grupos coesos — perceptível no terminal como
clusters de `C` que se deslocam juntos.

**Teste 4 — Ciclo de Lotka-Volterra:**
Com parâmetros balanceados, a simulação deve durar 300+ ticks sem extinção imediata.
Se o ecossistema colapsar antes de 100 ticks, ajuste os valores de energia (não a lógica).

**Remova os logs de debug antes de finalizar.**

---

## Estrutura de arquivos a criar/modificar

| Arquivo | Ação |
|---|---|
| `src/core/SteeringBehaviors.hpp` | **Criar** (header-only) |
| `src/core/Animal.hpp / .cpp` | **Modificar**: adicionar `estado`, `wanderAngle`, `aplicarForca()` |
| `src/core/WorldManager.hpp / .cpp` | **Modificar**: adicionar `obterVizinhos()`, `obterVizinhosDeTipo()`, `getLargura()`, `getAltura()` |
| `src/entities/Presa.hpp / .cpp` | **Modificar**: substituir `mover()` e `agir()` completamente |
| `src/entities/Predador.hpp / .cpp` | **Modificar**: substituir `mover()` e `agir()` completamente |
| `src/main.cpp` | **Modificar**: aumentar ticks para 500, ajustar populações iniciais |
| `CMakeLists.txt` | Provavelmente sem alteração |

---

## Ordem de implementação recomendada

Siga essa ordem para poder testar incrementalmente:

1. `obterVizinhos()` no `WorldManager` → teste chamando manualmente em `main.cpp`
2. `SteeringBehaviors.hpp` com `Vec2` + `seek` e `flee` → teste com valores fixos
3. `wander` e `pursue`/`evade` → integre em `Predador` primeiro (mais simples)
4. FSM do `Predador` (WANDERING ↔ HUNTING) → verifique perseguição no terminal
5. FSM da `Presa` (WANDERING ↔ FORAGING ↔ FLEEING) → verifique fuga
6. Flocking → adicione por último (é o mais complexo e menos crítico para o loop básico)
7. Reprodução com proximidade → substitua a da Fase 1
8. Balanço energético refinado → ajuste os custos para Lotka-Volterra estável

---

## Restrições que continuam valendo da Fase 1

- Sem SFML
- `WorldManager` não conhece tipos concretos (`Planta`, `Presa`, `Predador`) — use `TipoOrganismo` e `dynamic_cast` apenas onde absolutamente necessário
- Ownership exclusivo via `unique_ptr` em `WorldManager::organismos`
- RNG centralizado em `WorldManager::getRng()`
- Sem herança múltipla
- Zero warnings com `-Wall -Wextra -Wpedantic`

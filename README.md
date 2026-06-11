# Simulador de Ecossistema

Simulação de ecossistema predador–presa em C++17 com visualização gráfica em SFML.
Plantas crescem e se espalham em clusters, presas forrageiam em bando (flocking) e
fogem de predadores, que caçam usando steering behaviors (seek, pursue, evade, wander).

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue) ![SFML 2.6](https://img.shields.io/badge/SFML-2.6-green)

## Funcionalidades

- **Grade espacial** para buscas de vizinhança O(1) por célula (sem varredura global)
- **Máquina de estados** por agente: `WANDERING`, `FORAGING`, `FLEEING`, `HUNTING`
- **Steering behaviors** com flocking (separação, alinhamento, coesão) para as presas
- **Painel interativo**: gráfico de populações em tempo real, pausa, aceleração (1x–8x),
  reinício e 7 sliders de parâmetros ajustáveis durante a simulação
- **Câmera** com zoom (scroll) e pan (WASD/setas)
- **Tela inicial** para configurar as populações
- **Tela de encerramento** com o histórico completo quando presas e predadores se extinguem
- **Modo terminal** (sem SFML) para rodar a simulação em texto

## Compilando

### Com SFML (modo gráfico)

```bash
cmake -B build_sfml
cmake --build build_sfml
./build_sfml/ecosystem_sim
```

O CMake usa o SFML do sistema se disponível; caso contrário baixa via FetchContent.

### Sem SFML (modo terminal)

```bash
g++ -Wall -Wextra -std=c++17 -O2 src/main.cpp src/core/*.cpp src/entities/*.cpp -Isrc -o ecosystem_sim
./ecosystem_sim
```

O binário gráfico também aceita `--terminal` para forçar o modo texto.

## Controles

| Tecla / Ação      | Efeito                       |
|-------------------|------------------------------|
| `Enter`           | Inicia a simulação (tela inicial) |
| `Espaço`          | Pausa / retoma               |
| `F`               | Alterna velocidade 1x→2x→4x→8x |
| `WASD` / setas    | Move a câmera                |
| Scroll do mouse   | Zoom no grid                 |
| `R`               | Reseta a câmera              |
| `Esc`             | Sai                          |

## Estrutura

```
src/
├── main.cpp                  # loop principal (gráfico e terminal)
├── core/
│   ├── WorldManager.*        # tick da simulação, grade espacial, reprodução
│   ├── Organismo.*           # classe base
│   ├── Animal.*              # base de presa/predador (movimento por forças)
│   ├── SteeringBehaviors.hpp # seek, flee, pursue, evade, wander, flocking
│   ├── OrganismoFactory.*    # criação de organismos
│   ├── Renderer.*            # janela SFML, painel, gráficos
│   ├── UI.hpp                # botões e sliders
│   ├── SimParams.hpp         # parâmetros ajustáveis pelos sliders
│   └── ColorUtils.hpp        # cor por nível de energia
└── entities/
    ├── Planta.*              # cresce, espalha em clusters, morre de velhice
    ├── Presa.*               # forrageia, foge em bando
    └── Predador.*            # caça com perseguição preditiva
```

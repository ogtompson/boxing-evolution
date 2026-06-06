# BOXING EVOLUTION — Projeto PIF

Jogo educativo desenvolvido em C com Raylib para a disciplina de Programação Imperativa e Funcional.

---

## Objetivo

Ensinar conceitos básicos do boxe de forma integrada à jogabilidade, demonstrando simultaneamente os principais conceitos de programação em C.

---

## Como Compilar

### Pré-requisitos

- GCC instalado
- Raylib instalada (versão 4.5+)

### Linux/macOS

```bash
make
./boxing_evolution
```

### Instalar Raylib no Linux (Ubuntu/Debian)

```bash
sudo apt install libraylib-dev
# ou compilar do fonte: https://github.com/raysan5/raylib
```

### Instalar Raylib no macOS

```bash
brew install raylib
```

---

## Controles

### Geral
- `CIMA / BAIXO` — Navegar menus
- `ENTER` — Confirmar
- `ESC` — Voltar

### Durante a Luta
| Tecla | Ação |
|-------|------|
| `A` / `D` | Mover esquerda / direita |
| `ESPAÇO` | Defender (guarda) |
| `J` | Jab (nível 1) |
| `K` | Cross (nível 3) |
| `L` | Hook (nível 5) |
| `I` | Uppercut (nível 8) |

### Estatísticas (distribuir atributos)
| Tecla | Atributo |
|-------|----------|
| `1` | Vida Máxima |
| `2` | Técnica |
| `3` | Velocidade |
| `4` | Resistência |
| `5` | Potência |

---

## Estrutura do Projeto

```
boxing_evolution/
├── Makefile
├── README.md
└── src/
    ├── main.c          — Ponto de entrada, loop principal
    ├── game.h / game.c — Structs globais, Game States, Update/Draw
    ├── player.h / player.c — Lógica do jogador, atributos, golpes
    ├── enemy.h / enemy.c   — Adversários, IA simples
    ├── fight.h / fight.c   — Sistema de luta 2D
    ├── training.h / training.c — Três minigames de treino
    ├── knowledge.h / knowledge.c — Sistema educativo
    └── save.h / save.c     — Salvar/carregar em arquivo binário
```

---

## Conceitos de C Demonstrados

| Conceito | Onde aparece |
|----------|-------------|
| Structs | `Jogador`, `Adversario`, `EstadoLuta`, `Atributos`, `Golpe` |
| Enums | `GameState`, `TipoGolpe`, `TipoTreino` |
| Vetores | `golpes[MAX_GOLPES]`, `adversarios[MAX_ADVERSARIOS]`, `seq[8]` |
| Funções | Todos os módulos (player, enemy, fight, training, save...) |
| Ponteiros | Passagem de `Jogo*`, `Jogador*`, `Adversario*` |
| Condicionais | IA, cálculo de dano, desbloqueio de golpes |
| Laços | Atualização de cooldowns, iteração de adversários |
| Arquivos | `save.c` — `fwrite`/`fread` em arquivo binário |
| Aleatoriedade | `GetRandomValue()` — critico, IA, minigames |
| Modularização | 8 arquivos `.c` separados por responsabilidade |
| `#define` | Constantes globais em `game.h` |
| `typedef` | Todos os tipos com alias legíveis |

---

## Progressão do Jogo

| Fase | Adversário | Estilo | Nível |
|------|-----------|--------|-------|
| 1 | Carlos Novato | Iniciante | 1 |
| 2 | Bruno Furia | Agressivo | 3 |
| 3 | Diego Muralha | Defensivo | 5 |
| 4 | Ricardo Técnico | Técnico | 8 |
| 5 | El Campeón | Campeão Regional | 12 |

---

## Sistema de Conhecimento

O aprendizado está integrado à mecânica:

1. **Desbloqueio de golpe** → exibe descrição educativa
2. **Antes de cada luta** → dica sobre o adversário/golpe ideal
3. **Após cada treino** → curiosidade real do boxe
4. **Na derrota** → curiosidade aleatória (incentiva continuar)

---

## Golpes e Níveis de Desbloqueio

| Golpe | Tecla | Nível | Ensinamento |
|-------|-------|-------|-------------|
| Jab | J | 1 | Controle de distância |
| Cross | K | 3 | Rotação do corpo para potência |
| Hook | L | 5 | Eficiência em curta distância |
| Uppercut | I | 8 | Ataque à guarda aberta |

---

## Autores

Projeto acadêmico — Disciplina de Programação Imperativa e Funcional (PIF)

#ifndef FIGHT_H
#define FIGHT_H

#include "game.h"

/* Inicializa uma nova luta */
void FightInit(Jogo *j, int adversario_idx);

/* Atualiza a logica da luta (input do jogador + IA) */
void FightUpdate(Jogo *j);

/* Renderiza a tela de luta */
void FightDraw(Jogo *j);

/* Finaliza a luta e aplica recompensas */
void FightFinalizar(Jogo *j);

/* Renderiza o jogador na arena */
void FightDrawJogador(Jogador *p, float x, float y, int defendendo, int atacando);

/* Renderiza o fundo da arena */
void FightDrawArena(void);

#endif /* FIGHT_H */

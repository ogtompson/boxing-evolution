#ifndef ENEMY_H
#define ENEMY_H

#include "game.h"

void       EnemyInitTodos(Adversario adversarios[MAX_ADVERSARIOS]);
Adversario *EnemyGetPorFase(Adversario adversarios[], int fase);
void       EnemyUpdateIA(Jogo *j);
void       EnemyDraw(Adversario *adv, float x, float y, int atacando);
int        EnemyCalcularDano(Adversario *adv);

#endif

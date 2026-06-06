#ifndef SAVE_H
#define SAVE_H

#include "game.h"

/* Salva o jogo em arquivo binario */
int SaveJogo(const Jogador *jogador);

/* Carrega o jogo do arquivo; retorna 1 se ok, 0 se falhou */
int LoadJogo(Jogador *jogador);

/* Verifica se existe arquivo de save */
int SaveExiste(void);

/* Deleta o arquivo de save */
void SaveDeletar(void);

#endif /* SAVE_H */

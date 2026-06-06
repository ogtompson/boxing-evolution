#ifndef KNOWLEDGE_H
#define KNOWLEDGE_H

#include "game.h"

/* Mantido para compatibilidade  use AbrirConhecimento em game.c quando possivel */
void KnowledgeExibir(Jogo *j, const char *titulo, const char *texto);

const char *KnowledgeGetDicaLuta(int adversario_idx, int golpe_usado);
const char *KnowledgeGetCuriosidade(void);
void KnowledgeDraw(Jogo *j);

#endif /* KNOWLEDGE_H */

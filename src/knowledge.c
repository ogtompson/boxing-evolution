#include "knowledge.h"
#include <string.h>
#include <stdio.h>

static const char *CURIOSIDADES[] = {
    "O Jab e considerado o golpe mais importante do boxe.\nControla a distancia, cria oportunidades e desgasta o adversario.",
    "O Cross gera potencia atraves da rotacao do corpo.\nUm Cross bem executado envolve quadril, tronco e braco.",
    "O Hook e mais eficiente em curta distancia.\nAtaca de lado, dificil de ver e de defender.",
    "O Uppercut e usado para atacar adversarios\ncom guarda aberta. Sobe de baixo para cima.",
    "Lutadores profissionais podem ultrapassar 170 BPM\ndurante uma luta. O condicionamento fisico e essencial.",
    "O boxe olimpico e o profissional possuem regras\ndiferentes: rounds, pontuacao e equipamento.",
    "A guarda nao serve apenas para defender.\nUma boa guarda cria angulos para contra-ataques.",
    "Categorias de peso existem para garantir\njustica no boxe - do Minimosca ao Super-Pesado.",
    "Muhammad Ali era conhecido pela frase:\n'Flutua como borboleta, ferroe como abelha'.",
    "O tempo de reacao medio de um boxeador treinado\ne de aproximadamente 150 a 200 milissegundos.",
};
#define N_CURIOSIDADES 10

const char *KnowledgeGetCuriosidade(void) {
    return CURIOSIDADES[GetRandomValue(0, N_CURIOSIDADES - 1)];
}

const char *KnowledgeGetDicaLuta(int adversario_idx, int golpe_usado) {
    (void)adversario_idx;
    static const char *dicas[] = {
        "Jab: use para controlar a distancia e abrir a guarda!",
        "Cross: potente, mas deixa voce exposto. Use com cuidado.",
        "Hook: eficaz perto. Aproxime-se antes de usar.",
        "Uppercut: perfeito quando o adversario tem guarda baixa."
    };
    if (golpe_usado < 0 || golpe_usado >= MAX_GOLPES) return "";
    return dicas[golpe_usado];
}

/* KnowledgeExibir - mantido para compatibilidade com enemy.c/fight.c */
void KnowledgeExibir(Jogo *j, const char *titulo, const char *texto) {
    strncpy(j->conhecimento_titulo, titulo, sizeof(j->conhecimento_titulo) - 1);
    strncpy(j->conhecimento_texto,  texto,  sizeof(j->conhecimento_texto)  - 1);
    j->conhecimento_destino = CONHECIMENTO_VOLTA_ACADEMIA;
    JogoMudarEstado(j, STATE_CONHECIMENTO);
}

void KnowledgeDraw(Jogo *j) {
    ClearBackground((Color){10, 15, 30, 255});

    /* Moldura */
    DrawRectangle(50, 50, SCREEN_W - 100, SCREEN_H - 100, (Color){18, 28, 56, 255});
    DrawRectangleLines(50, 50, SCREEN_W - 100, SCREEN_H - 100, GOLD);

    /* Icone */
    DrawCircle(SCREEN_W/2, 100, 26, GOLD);
    DrawText("!", SCREEN_W/2 - 5, 84, 28, (Color){18, 28, 56, 255});

    /* Cabecalho */
    DrawText("[ CONHECIMENTO DESBLOQUEADO ]",
             SCREEN_W/2 - MeasureText("[ CONHECIMENTO DESBLOQUEADO ]", 18)/2,
             138, 18, GOLD);
    DrawLine(70, 166, SCREEN_W - 70, 166, (Color){200, 160, 50, 100});

    /* Titulo do conhecimento */
    DrawText(j->conhecimento_titulo,
             SCREEN_W/2 - MeasureText(j->conhecimento_titulo, 22)/2,
             182, 22, WHITE);

    /* Texto educativo */
    DrawText(j->conhecimento_texto, 90, 226, 15, LIGHTGRAY);

    /* Destino */
    const char *destino_txt = (j->conhecimento_destino == CONHECIMENTO_VOLTA_LUTA)
        ? "Proximo: LUTA"
        : "Proximo: ACADEMIA";
    DrawText(destino_txt,
             SCREEN_W/2 - MeasureText(destino_txt, 13)/2,
             SCREEN_H - 110, 13, (Color){100, 140, 200, 255});

    DrawText("[ ENTER ou ESPACO para continuar ]",
             SCREEN_W/2 - MeasureText("[ ENTER ou ESPACO para continuar ]", 16)/2,
             SCREEN_H - 84, 16, (Color){200, 200, 100, 255});
}

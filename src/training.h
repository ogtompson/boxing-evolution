#ifndef TRAINING_H
#define TRAINING_H

#include "game.h"

/* Tipos de treino */
typedef enum {
    TREINO_VELOCIDADE  = 0,
    TREINO_POTENCIA    = 1,
    TREINO_RESISTENCIA = 2
} TipoTreino;

/* Estado do minigame de treino */
typedef struct {
    TipoTreino tipo;
    int  rodadas;           /* rodadas completadas            */
    int  rodadas_max;       /* meta de rodadas                */
    int  acertos;           /* acertos na rodada              */
    int  ativo;             /* minigame em andamento          */
    /* Velocidade: tecla aleatoria */
    int  tecla_alvo;        /* KeyboardKey esperada           */
    float timer_tecla;      /* tempo para a tecla aparecer    */
    float janela_reacao;    /* janela para pressionar (segs)  */
    int  tecla_visivel;
    /* Potencia: barra deslizante */
    float barra_pos;        /* 0..1                           */
    float barra_dir;        /* velocidade de movimento         */
    float zona_inicio;      /* inicio da zona alvo            */
    float zona_fim;         /* fim da zona alvo               */
    /* Resistencia: sequencia de teclas */
    int  seq[8];            /* sequencia de KeyboardKeys       */
    int  seq_idx;           /* posicao atual na sequencia      */
    int  seq_len;
    float timer_seq;        /* tempo entre teclas da sequencia */
} EstadoTreino;

/* Funcoes do modulo */
void TrainoInit(EstadoTreino *t, TipoTreino tipo);
void TreinoUpdate(Jogo *j, EstadoTreino *t);
void TreinoDraw(Jogo *j, EstadoTreino *t);
void TreinoAplicarRecompensa(Jogo *j, EstadoTreino *t);

/* Retorna texto educativo do treino */
const char *TreinoGetConhecimento(TipoTreino tipo);

#endif /* TRAINING_H */

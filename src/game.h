#ifndef GAME_H
#define GAME_H

#include "raylib.h"

#define SCREEN_W        1024
#define SCREEN_H        600
#define TARGET_FPS      60
#define SAVE_FILE       "boxing_save.dat"

#define MAX_GOLPES      4
#define MAX_ADVERSARIOS 5
#define MAX_FASES       5

/* ============================================================
   ENUM: ESTADOS DO JOGO
   ============================================================ */
typedef enum {
    STATE_MENU = 0,
    STATE_ACADEMIA,
    STATE_TREINO_VELOCIDADE,
    STATE_TREINO_POTENCIA,
    STATE_TREINO_RESISTENCIA,
    STATE_LUTA,
    STATE_ESTATISTICAS,
    STATE_CONHECIMENTO,
    STATE_ACADEMIA_LUTAR,    /* card do proximo adversario */
    STATE_ACADEMIA_TREINAR,  /* submenu de treino          */
    STATE_GAMEOVER,
    STATE_VITORIA,
    STATE_CREDITOS,
    STATE_COUNT
} GameState;

/* Contexto de onde o conhecimento deve retornar apos ser lido */
typedef enum {
    CONHECIMENTO_VOLTA_ACADEMIA = 0,
    CONHECIMENTO_VOLTA_LUTA
} ConhecimentoDestino;

typedef enum {
    GOLPE_JAB      = 0,
    GOLPE_CROSS    = 1,
    GOLPE_HOOK     = 2,
    GOLPE_UPPERCUT = 3
} TipoGolpe;

typedef struct {
    int vida;
    int vida_max;
    int tecnica;
    int velocidade;
    int resistencia;
    int potencia;
} Atributos;

typedef struct {
    char   nome[32];
    char   descricao[128];
    char   tecla_str[8];
    int    dano_base;
    int    desbloqueado;
    int    nivel_necessario;
    float  cooldown;
    float  timer_cooldown;
} Golpe;

typedef struct {
    char      nome[32];
    Atributos atrib;
    int       xp;
    int       xp_proximo_nivel;
    int       nivel;
    int       pontos_atributo;
    Golpe     golpes[MAX_GOLPES];
    int       fase_atual;
    int       lutas_vencidas;
    int       lutas_perdidas;
} Jogador;

typedef struct {
    char      nome[32];
    char      estilo[32];
    char      dica_pre_luta[256];
    Atributos atrib;
    int       nivel;
    Color     cor;
    Color     pele_cor;
    Color     luva_cor;
    Color     cabelo_cor;
    /* comportamento de IA */
    int       avanca;           /* 1=persegue jogador, 0=recua */
    float     cooldown_golpe;   /* intervalo entre ataques      */
    float     timer_golpe;      /* timer atual                  */
    float     vel_movimento;    /* pixels por segundo           */
} Adversario;

typedef struct {
    float jogador_x;
    float adversario_x;
    int   jogador_defendendo;
    int   adversario_atacando;
    float timer_adversario;
    int   round_atual;
    float tempo_round;
    int   luta_encerrada;
    int   jogador_venceu;
    char  mensagem[128];
    float timer_mensagem;
} EstadoLuta;

typedef struct {
    GameState            estado_atual;
    Jogador              jogador;
    Adversario           adversarios[MAX_ADVERSARIOS];
    EstadoLuta           luta;
    int                  adversario_atual_idx;

    /* Navegacao do menu da academia (0=Treinar 1=Lutar 2=Stats) */
    int                  academia_opcao;
    /* Submenu de treino (0=Vel 1=Pot 2=Res) */
    int                  treino_opcao;
    /* 0=academia normal, 1=submenu treino, 2=card adversario */
    int                  academia_sub;

    /* Navegacao do menu principal */
    int                  menu_opcao;

    /* Tela de conhecimento */
    char                 conhecimento_titulo[64];
    char                 conhecimento_texto[512];
    ConhecimentoDestino  conhecimento_destino;

    /* Feedback de save */
    float                timer_save_feedback;

    float                delta;
} Jogo;

void JogoInit(Jogo *j);
void JogoUpdate(Jogo *j);
void JogoDraw(Jogo *j);
void JogoMudarEstado(Jogo *j, GameState novo);

#endif /* GAME_H */

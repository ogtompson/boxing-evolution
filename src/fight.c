#include "fight.h"
#include "enemy.h"
#include "player.h"
#include <string.h>
#include <stdio.h>

#define ARENA_ESQ       120.0f
#define ARENA_DIR       904.0f
#define ARENA_CHAO      510.0f
#define VELOCIDADE_MOV  180.0f
#define TEMPO_ROUND     99.0f
#define CARENCIA_INICIO 0.3f
#define PS              6        /* pixel size: cada "pixel" = 6x6 px reais */

static float g_timer_carencia   = 0.0f;
static int   g_atk_visual       = 0;   /* 0=idle 1=jab 2=cross 3=hook 4=upper */
static float g_atk_visual_timer = 0.0f;
static int   g_flash_hit        = 0;
static float g_flash_timer      = 0.0f;

/* ============================================================
   Pixel drawing helper
   ============================================================ */
static void PX(int x, int y, Color c) {
    DrawRectangle(x, y, PS, PS, c);
}

/* ============================================================
   Desenha sprite pixel art do JOGADOR 
   ============================================================ */
static void DrawSprite_Jogador(int ox, int oy, int atk, int defend) {
    /* Paleta */
    Color pele   = (Color){210, 160, 100, 255};
    Color luva   = (Color){210,  40,  40, 255};
    Color short_ = (Color){ 30,  80, 200, 255};
    Color short2 = (Color){ 20,  50, 140, 255};
    Color tenis  = (Color){ 30,  30,  30, 255};
    Color cabelo = (Color){ 30,  20,  10, 255};
    Color olho   = (Color){ 10,  10,  10, 255};
    Color contorno = (Color){10, 10, 10, 255};
    Color headgear = (Color){180, 20, 20, 255};
    Color headg2   = (Color){140, 10, 10, 255};
    (void)contorno;

    /* --- Tenis --- */
    PX(ox+1,oy+22,tenis); PX(ox+2,oy+22,tenis); PX(ox+3,oy+22,tenis);
    PX(ox+7,oy+22,tenis); PX(ox+8,oy+22,tenis); PX(ox+9,oy+22,tenis);
    PX(ox+1,oy+23,tenis); PX(ox+2,oy+23,tenis); PX(ox+3,oy+23,tenis); PX(ox+4,oy+23,tenis);
    PX(ox+7,oy+23,tenis); PX(ox+8,oy+23,tenis); PX(ox+9,oy+23,tenis); PX(ox+10,oy+23,tenis);

    /* --- Pernas --- */
    PX(ox+2,oy+18,short_); PX(ox+3,oy+18,short_);
    PX(ox+8,oy+18,short_); PX(ox+9,oy+18,short_);
    PX(ox+2,oy+19,pele);   PX(ox+3,oy+19,pele);
    PX(ox+8,oy+19,pele);   PX(ox+9,oy+19,pele);
    PX(ox+2,oy+20,pele);   PX(ox+3,oy+20,pele);
    PX(ox+8,oy+20,pele);   PX(ox+9,oy+20,pele);
    PX(ox+2,oy+21,tenis);  PX(ox+3,oy+21,tenis);
    PX(ox+8,oy+21,tenis);  PX(ox+9,oy+21,tenis);

    /* --- Short --- */
    for (int xx = 1; xx <= 10; xx++) {
        PX(ox+xx, oy+14, short_);
        PX(ox+xx, oy+15, short_);
        PX(ox+xx, oy+16, short_);
        PX(ox+xx, oy+17, short_);
    }
    /* Listra lateral do short */
    PX(ox+1,oy+14,short2); PX(ox+1,oy+15,short2);
    PX(ox+1,oy+16,short2); PX(ox+1,oy+17,short2);
    PX(ox+10,oy+14,short2); PX(ox+10,oy+15,short2);

    /* --- Torso --- */
    for (int yy = 8; yy <= 13; yy++)
        for (int xx = 2; xx <= 9; xx++)
            PX(ox+xx, oy+yy, pele);

    /* --- Bracos em repouso --- */
    if (!defend && atk == 0) {
        /* Braco esquerdo */
        PX(ox+1,oy+9,pele);  PX(ox+1,oy+10,pele); PX(ox+1,oy+11,pele);
        PX(ox+0,oy+12,luva); PX(ox+1,oy+12,luva); PX(ox+0,oy+13,luva); PX(ox+1,oy+13,luva);
        /* Braco direito */
        PX(ox+10,oy+9,pele);  PX(ox+10,oy+10,pele); PX(ox+11,oy+10,pele);
        PX(ox+10,oy+11,luva); PX(ox+11,oy+11,luva);
        PX(ox+10,oy+12,luva); PX(ox+11,oy+12,luva);
    } else if (defend) {
        /* Posicao de guarda - luvas na frente do rosto */
        PX(ox+2,oy+6,luva);  PX(ox+3,oy+6,luva);  PX(ox+4,oy+6,luva);
        PX(ox+7,oy+6,luva);  PX(ox+8,oy+6,luva);  PX(ox+9,oy+6,luva);
        PX(ox+2,oy+7,luva);  PX(ox+3,oy+7,luva);
        PX(ox+7,oy+7,luva);  PX(ox+8,oy+7,luva);
        PX(ox+1,oy+9,pele);  PX(ox+10,oy+9,pele);
        PX(ox+1,oy+10,pele); PX(ox+10,oy+10,pele);
    } else if (atk == 1) {
        /* JAB - braco direito esticado */
        PX(ox+10,oy+9,pele); PX(ox+11,oy+9,pele); PX(ox+12,oy+9,pele);
        PX(ox+13,oy+9,luva); PX(ox+14,oy+9,luva);
        PX(ox+13,oy+10,luva); PX(ox+14,oy+10,luva);
        /* Rastro de soco */
        PX(ox+12,oy+10,(Color){255,200,200,120});
        PX(ox+11,oy+10,(Color){255,200,200,80});
        /* Braco esquerdo na guarda */
        PX(ox+1,oy+9,pele); PX(ox+1,oy+10,pele);
        PX(ox+0,oy+11,luva); PX(ox+1,oy+11,luva);
    } else if (atk == 2) {
        /* CROSS - braco esquerdo com torcao */
        PX(ox+10,oy+8,pele); PX(ox+11,oy+8,pele); PX(ox+12,oy+8,pele); PX(ox+13,oy+8,pele);
        PX(ox+13,oy+8,luva); PX(ox+14,oy+8,luva); PX(ox+15,oy+8,luva);
        PX(ox+13,oy+9,luva); PX(ox+14,oy+9,luva); PX(ox+15,oy+9,luva);
        PX(ox+12,oy+9,(Color){255,200,200,150});
        PX(ox+11,oy+9,(Color){255,200,200,100});
        PX(ox+10,oy+9,(Color){255,200,200,60});
        PX(ox+1,oy+10,pele); PX(ox+0,oy+11,luva); PX(ox+1,oy+11,luva);
    } else if (atk == 3) {
        /* HOOK - braco em arco lateral */
        PX(ox+10,oy+7,pele); PX(ox+11,oy+7,pele);
        PX(ox+12,oy+7,luva); PX(ox+13,oy+7,luva);
        PX(ox+12,oy+8,luva); PX(ox+13,oy+8,luva);
        PX(ox+1,oy+9,pele); PX(ox+0,oy+10,luva); PX(ox+1,oy+10,luva);
    } else if (atk == 4) {
        /* UPPERCUT - soco de baixo para cima */
        PX(ox+10,oy+11,pele); PX(ox+11,oy+10,pele); PX(ox+12,oy+9,pele);
        PX(ox+12,oy+8,luva);  PX(ox+13,oy+8,luva);
        PX(ox+12,oy+7,luva);  PX(ox+13,oy+7,luva);
        PX(ox+12,oy+9,(Color){255,200,200,120});
        PX(ox+1,oy+9,pele); PX(ox+0,oy+10,luva); PX(ox+1,oy+10,luva);
    }

    /* --- Cabeca --- */
    /* Headgear (capacete) */
    PX(ox+3,oy+1,headgear); PX(ox+4,oy+1,headgear); PX(ox+5,oy+1,headgear);
    PX(ox+6,oy+1,headgear); PX(ox+7,oy+1,headgear); PX(ox+8,oy+1,headgear);
    PX(ox+2,oy+2,headgear); PX(ox+9,oy+2,headgear);
    PX(ox+2,oy+3,headg2);   PX(ox+9,oy+3,headg2);
    PX(ox+2,oy+4,headg2);   PX(ox+9,oy+4,headg2);

    /* Rosto */
    for (int xx = 3; xx <= 8; xx++) PX(ox+xx, oy+2, pele);
    for (int xx = 3; xx <= 8; xx++) PX(ox+xx, oy+3, pele);
    for (int xx = 3; xx <= 8; xx++) PX(ox+xx, oy+4, pele);
    for (int xx = 3; xx <= 8; xx++) PX(ox+xx, oy+5, pele);
    for (int xx = 2; xx <= 9; xx++) PX(ox+xx, oy+6, pele);
    for (int xx = 2; xx <= 9; xx++) PX(ox+xx, oy+7, pele);

    /* Cabelo */
    PX(ox+3,oy+2,cabelo); PX(ox+4,oy+2,cabelo); PX(ox+5,oy+2,cabelo);

    /* Olhos */
    PX(ox+4,oy+4,olho); PX(ox+7,oy+4,olho);

    /* Boca */
    PX(ox+5,oy+6,olho); PX(ox+6,oy+6,olho);
}

/* ============================================================
   Sprite pixel art do ADVERSARIO
   Recebe cor do short e tom de pele para diferenciar cada um
   ============================================================ */
static void DrawSprite_Adversario(int ox, int oy, int atacando,
                                   Color short_cor, Color pele_cor,
                                   Color luva_cor, Color cabelo_cor) {
    Color short2 = (Color){(unsigned char)(short_cor.r/2),
                            (unsigned char)(short_cor.g/2),
                            (unsigned char)(short_cor.b/2), 255};
    Color tenis  = (Color){40, 35, 30, 255};
    Color olho   = (Color){10, 10, 10, 255};

    /* Tenis */
    PX(ox+2,oy+22,tenis); PX(ox+3,oy+22,tenis); PX(ox+4,oy+22,tenis);
    PX(ox+8,oy+22,tenis); PX(ox+9,oy+22,tenis); PX(ox+10,oy+22,tenis);
    PX(ox+1,oy+23,tenis); PX(ox+2,oy+23,tenis); PX(ox+3,oy+23,tenis); PX(ox+4,oy+23,tenis);
    PX(ox+7,oy+23,tenis); PX(ox+8,oy+23,tenis); PX(ox+9,oy+23,tenis); PX(ox+10,oy+23,tenis);

    /* Pernas */
    PX(ox+2,oy+18,short_cor); PX(ox+3,oy+18,short_cor);
    PX(ox+8,oy+18,short_cor); PX(ox+9,oy+18,short_cor);
    PX(ox+2,oy+19,pele_cor); PX(ox+3,oy+19,pele_cor);
    PX(ox+8,oy+19,pele_cor); PX(ox+9,oy+19,pele_cor);
    PX(ox+2,oy+20,pele_cor); PX(ox+3,oy+20,pele_cor);
    PX(ox+8,oy+20,pele_cor); PX(ox+9,oy+20,pele_cor);
    PX(ox+2,oy+21,tenis); PX(ox+3,oy+21,tenis);
    PX(ox+8,oy+21,tenis); PX(ox+9,oy+21,tenis);

    /* Short */
    for (int xx = 1; xx <= 10; xx++)
        for (int yy = 14; yy <= 17; yy++)
            PX(ox+xx, oy+yy, short_cor);
    PX(ox+10,oy+14,short2); PX(ox+10,oy+15,short2);
    PX(ox+10,oy+16,short2); PX(ox+10,oy+17,short2);
    PX(ox+1,oy+14,short2);  PX(ox+1,oy+15,short2);

    /* Torso */
    for (int yy = 8; yy <= 13; yy++)
        for (int xx = 2; xx <= 9; xx++)
            PX(ox+xx, oy+yy, pele_cor);

    /* Bracos */
    if (!atacando) {
        /* Guarda */
        PX(ox+0,oy+9,pele_cor); PX(ox+1,oy+9,pele_cor);
        PX(ox+0,oy+10,luva_cor); PX(ox+1,oy+10,luva_cor); PX(ox+0,oy+11,luva_cor); PX(ox+1,oy+11,luva_cor);
        PX(ox+10,oy+9,pele_cor); PX(ox+11,oy+9,pele_cor);
        PX(ox+10,oy+10,luva_cor); PX(ox+11,oy+10,luva_cor); PX(ox+10,oy+11,luva_cor); PX(ox+11,oy+11,luva_cor);
    } else {
        /* Soco esticado para esquerda */
        PX(ox-1,oy+9,pele_cor); PX(ox+0,oy+9,pele_cor); PX(ox+1,oy+9,pele_cor);
        PX(ox-3,oy+9,luva_cor); PX(ox-2,oy+9,luva_cor);
        PX(ox-3,oy+10,luva_cor); PX(ox-2,oy+10,luva_cor);
        /* Rastro */
        PX(ox-1,oy+10,(Color){255,180,180,120});
        PX(ox+10,oy+9,pele_cor); PX(ox+11,oy+9,pele_cor);
        PX(ox+10,oy+10,luva_cor); PX(ox+11,oy+10,luva_cor);
    }

    /* Cabeca com headgear na cor do short */
    Color hg  = short_cor;
    Color hg2 = short2;

    /* Headgear */
    PX(ox+3,oy+1,hg); PX(ox+4,oy+1,hg); PX(ox+5,oy+1,hg);
    PX(ox+6,oy+1,hg); PX(ox+7,oy+1,hg); PX(ox+8,oy+1,hg);
    PX(ox+2,oy+2,hg); PX(ox+9,oy+2,hg);
    PX(ox+2,oy+3,hg2); PX(ox+9,oy+3,hg2);
    PX(ox+2,oy+4,hg2); PX(ox+9,oy+4,hg2);

    /* Rosto */
    for (int xx = 3; xx <= 8; xx++) PX(ox+xx, oy+2, pele_cor);
    for (int xx = 3; xx <= 8; xx++) PX(ox+xx, oy+3, pele_cor);
    for (int xx = 3; xx <= 8; xx++) PX(ox+xx, oy+4, pele_cor);
    for (int xx = 3; xx <= 8; xx++) PX(ox+xx, oy+5, pele_cor);
    for (int xx = 2; xx <= 9; xx++) PX(ox+xx, oy+6, pele_cor);
    for (int xx = 2; xx <= 9; xx++) PX(ox+xx, oy+7, pele_cor);

    /* Cabelo */
    PX(ox+4,oy+2,cabelo_cor); PX(ox+5,oy+2,cabelo_cor); PX(ox+6,oy+2,cabelo_cor);
    PX(ox+7,oy+2,cabelo_cor);

    /* Olhos (espelhado: olho direito primeiro) */
    PX(ox+4,oy+4,olho); PX(ox+7,oy+4,olho);

    /* Boca */
    PX(ox+5,oy+6,olho); PX(ox+6,oy+6,olho);
}

/* Escala do sprite: PS=4 --- sprite ocupa 16*4=64 x 24*4=96 pixels reais */
#define SPR_W  (16*PS)
#define SPR_H  (24*PS)

/* ============================================================
   FightInit
   ============================================================ */
void FightInit(Jogo *j, int adversario_idx) {
    j->adversario_atual_idx = adversario_idx;

    Adversario *adv  = &j->adversarios[adversario_idx];
    adv->atrib.vida  = adv->atrib.vida_max;
    adv->timer_golpe = adv->cooldown_golpe + 1.0f;

    /* Reset COMPLETO da vida do jogador */
    j->jogador.atrib.vida = j->jogador.atrib.vida_max;
    if (j->jogador.atrib.vida <= 0) j->jogador.atrib.vida = 100;

    j->luta.jogador_x           = ARENA_ESQ + 100.0f;
    j->luta.adversario_x        = ARENA_DIR  - 100.0f;
    j->luta.jogador_defendendo  = 0;
    j->luta.adversario_atacando = 0;
    j->luta.round_atual         = 1;
    j->luta.tempo_round         = TEMPO_ROUND;
    j->luta.luta_encerrada      = 0;
    j->luta.jogador_venceu      = 0;
    j->luta.timer_mensagem      = 2.5f;
    strcpy(j->luta.mensagem, "Luta iniciada! Boa sorte!");

    g_timer_carencia   = CARENCIA_INICIO;
    g_atk_visual       = 0;
    g_atk_visual_timer = 0.0f;
    g_flash_hit        = 0;
    g_flash_timer      = 0.0f;
}

/* ============================================================
   FightUpdate
   ============================================================ */
void FightUpdate(Jogo *j) {
    if (j->luta.luta_encerrada) return;

    int input_bloqueado = 0;
    if (g_timer_carencia > 0.0f) {
        g_timer_carencia -= j->delta;
        input_bloqueado = 1;
    }
    if (input_bloqueado) { EnemyUpdateIA(j); return; }

    Jogador    *p    = &j->jogador;
    Adversario *adv  = EnemyGetPorFase(j->adversarios, j->adversario_atual_idx);
    EstadoLuta *luta = &j->luta;

    if (g_atk_visual_timer > 0.0f) {
        g_atk_visual_timer -= j->delta;
        if (g_atk_visual_timer <= 0.0f) g_atk_visual = 0;
    }
    if (g_flash_timer > 0.0f) {
        g_flash_timer -= j->delta;
        if (g_flash_timer <= 0.0f) g_flash_hit = 0;
    }

    luta->tempo_round -= j->delta;
    if (luta->tempo_round <= 0.0f) {
        luta->luta_encerrada = 1;
        luta->jogador_venceu = (p->atrib.vida >= adv->atrib.vida) ? 1 : 0;
        return;
    }

    PlayerAtualizarCooldowns(p, j->delta);

    luta->jogador_defendendo = 0;
    if (IsKeyDown(KEY_A)) {
        luta->jogador_x -= VELOCIDADE_MOV * j->delta;
        if (luta->jogador_x < ARENA_ESQ + 10.0f) luta->jogador_x = ARENA_ESQ + 10.0f;
    }
    if (IsKeyDown(KEY_D)) {
        luta->jogador_x += VELOCIDADE_MOV * j->delta;
        if (luta->jogador_x > luta->adversario_x - 80.0f)
            luta->jogador_x = luta->adversario_x - 80.0f;
    }
    if (IsKeyDown(KEY_SPACE)) luta->jogador_defendendo = 1;

    float distancia = luta->adversario_x - luta->jogador_x;
    int em_alcance  = (distancia < 85.0f);

    int golpe_usado = -1;
    if (IsKeyPressed(KEY_J) && PlayerPodeUsarGolpe(p, GOLPE_JAB)) {
        golpe_usado = GOLPE_JAB; g_atk_visual = 1; g_atk_visual_timer = 0.25f;
        if (distancia < 105.0f) em_alcance = 1;
        if (em_alcance) { strcpy(luta->mensagem, "Jab! Controle a distancia!"); luta->timer_mensagem = 1.5f; }
    } else if (IsKeyPressed(KEY_K) && PlayerPodeUsarGolpe(p, GOLPE_CROSS)) {
        golpe_usado = GOLPE_CROSS; g_atk_visual = 2; g_atk_visual_timer = 0.3f;
        if (em_alcance) { strcpy(luta->mensagem, "Cross! Rotacao total!"); luta->timer_mensagem = 1.5f; }
    } else if (IsKeyPressed(KEY_L) && PlayerPodeUsarGolpe(p, GOLPE_HOOK)) {
        golpe_usado = GOLPE_HOOK; g_atk_visual = 3; g_atk_visual_timer = 0.3f;
        if (em_alcance) { strcpy(luta->mensagem, "Hook! Perfeito de perto!"); luta->timer_mensagem = 1.5f; }
    } else if (IsKeyPressed(KEY_I) && PlayerPodeUsarGolpe(p, GOLPE_UPPERCUT)) {
        golpe_usado = GOLPE_UPPERCUT; g_atk_visual = 4; g_atk_visual_timer = 0.35f;
        if (em_alcance) { strcpy(luta->mensagem, "Uppercut! Na guarda aberta!"); luta->timer_mensagem = 1.5f; }
    }

    if (golpe_usado >= 0) {
        PlayerUsarGolpe(p, (TipoGolpe)golpe_usado);
        if (em_alcance) {
            int dano = PlayerCalcularDano(p, (TipoGolpe)golpe_usado);
            int eh_ideal = 0;
            if (j->adversario_atual_idx == 0 && golpe_usado == GOLPE_JAB)      eh_ideal = 1;
            if (j->adversario_atual_idx == 1 && golpe_usado == GOLPE_CROSS)    eh_ideal = 1;
            if (j->adversario_atual_idx == 2 && golpe_usado == GOLPE_HOOK)     eh_ideal = 1;
            if (j->adversario_atual_idx == 3 && golpe_usado == GOLPE_UPPERCUT) eh_ideal = 1;
            if (j->adversario_atual_idx == 4 && golpe_usado != GOLPE_JAB)      eh_ideal = 1;
            if (eh_ideal) {
                dano = (int)((float)dano * 1.6f);
                strcpy(luta->mensagem, "GOLPE IDEAL! +60% dano!");
                luta->timer_mensagem = 2.0f;
            }
            if (golpe_usado == GOLPE_JAB && j->adversario_atual_idx >= 2) {
                dano = (int)((float)dano * 0.5f);
            }
            adv->atrib.vida -= dano;
            if (adv->atrib.vida < 0) adv->atrib.vida = 0;
            g_flash_hit = 1; g_flash_timer = 0.1f;
            if (adv->atrib.vida <= 0) { luta->luta_encerrada = 1; luta->jogador_venceu = 1; }
        } else {
            strcpy(luta->mensagem, "Fora de alcance! Avance com D.");
            luta->timer_mensagem = 1.2f;
        }
    }

    if (luta->timer_mensagem > 0.0f) luta->timer_mensagem -= j->delta;
    EnemyUpdateIA(j);
}

/* ============================================================
   FightDrawArena  -  ringue pixel art
   ============================================================ */
void FightDrawArena(void) {
    /* Ceu / fundo escuro */
    DrawRectangle(0, 0, 1024, 600, (Color){8, 6, 18, 255});

    /* Plateia pixel - blocos 8x8 simulando pessoas */
    Color plateia_cores[4] = {
        (Color){120,30,30,255}, (Color){30,30,120,255},
        (Color){30,100,30,255}, (Color){100,80,20,255}
    };
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 48; col++) {
            int px = col * 22;
            int py = 60 + row * 32;
            Color c = plateia_cores[(col + row) % 4];
            /* Corpo */
            DrawRectangle(px+4, py+8, 14, 16, c);
            /* Cabeca */
            DrawRectangle(px+6, py,   10, 10, (Color){200,160,110,255});
            /* Variacao de cor para dar vida */
            if ((col+row) % 3 == 0)
                DrawRectangle(px+6, py, 10, 10, (Color){160,110,70,255});
        }
    }

    /* Holofotes */
    DrawRectangle(150, 180, 20, 8,  (Color){180,170,100,255});
    DrawRectangle(854, 180, 20, 8,  (Color){180,170,100,255});
    /* Feixe de luz simulado */
    DrawRectangle(155, 188, 8, 120, (Color){255,255,200,15});
    DrawRectangle(856, 188, 8, 120, (Color){255,255,200,15});

    /* Lona - verde escuro com textura */
    DrawRectangle(180, 340, 664, 200, (Color){20, 80, 40, 255});
    /* Grade da lona */
    for (int gx = 180; gx < 844; gx += 40)
        DrawRectangle(gx, 340, 2, 200, (Color){15, 65, 32, 255});
    for (int gy = 340; gy < 540; gy += 40)
        DrawRectangle(180, gy, 664, 2, (Color){15, 65, 32, 255});

    /* Logo na lona */
    DrawRectangle(380, 420, 264, 40, (Color){16, 68, 34, 255});
    DrawText("BOXING EVOLUTION", 395, 428, 20, (Color){12, 55, 26, 255});

    /* Cordas */
    Color corda = (Color){220, 50, 50, 255};
    Color corda2 = (Color){200, 200, 50, 255};
    DrawRectangle(180, 300, 664, 6, corda);
    DrawRectangle(180, 320, 664, 6, corda2);
    DrawRectangle(180, 340, 664, 6, corda);

    /* Postes pixel */
    Color poste = (Color){200, 160, 40, 255};
    Color poste2 = (Color){160, 120, 20, 255};
    /* Esquerdo */
    DrawRectangle(168, 288, 16, 60, poste);
    DrawRectangle(172, 288, 4,  60, poste2);
    DrawRectangle(164, 284, 24, 8,  poste);
    /* Direito */
    DrawRectangle(840, 288, 16, 60, poste);
    DrawRectangle(840, 288, 4,  60, poste2);
    DrawRectangle(836, 284, 24, 8,  poste);

    /* Sombra na lona */
    DrawRectangle(180, 536, 664, 8, (Color){0, 0, 0, 80});
}

/* ============================================================
   FightDraw
   ============================================================ */
void FightDraw(Jogo *j) {
    Jogador    *p    = &j->jogador;
    Adversario *adv  = EnemyGetPorFase(j->adversarios, j->adversario_atual_idx);
    EstadoLuta *luta = &j->luta;

    ClearBackground((Color){8,6,18,255});
    FightDrawArena();

    if (g_flash_hit)
        DrawRectangle(0, 0, 1024, 600, (Color){255,255,100,20});

    /* Sombras dos lutadores na lona */
    DrawRectangle((int)luta->jogador_x - 30, 534, 60, 8, (Color){0,0,0,60});
    DrawRectangle((int)luta->adversario_x - 30, 534, 60, 8, (Color){0,0,0,60});

    /* Adversario - pixel art espelhado */
    int ax = (int)luta->adversario_x - SPR_W/2;
    int ay = (int)ARENA_CHAO - SPR_H;
    DrawSprite_Adversario(ax, ay, luta->adversario_atacando,
                          adv->cor,
                          adv->pele_cor,
                          adv->luva_cor,
                          adv->cabelo_cor);

    /* Barra de vida do adversario acima do sprite */
    int vmax = adv->atrib.vida_max > 0 ? adv->atrib.vida_max : 1;
    int vv   = adv->atrib.vida < 0 ? 0 : adv->atrib.vida;
    int bv   = (int)(80.0f * (float)vv / (float)vmax);
    DrawRectangle(ax, ay - 14, 80, 8, (Color){60,0,0,200});
    Color cv = vv > vmax*2/3 ? GREEN : vv > vmax/3 ? YELLOW : RED;
    DrawRectangle(ax, ay - 14, bv, 8, cv);
    DrawRectangleLines(ax, ay - 14, 80, 8, WHITE);
    DrawText(adv->nome, ax, ay - 26, 10, WHITE);

    /* Jogador - pixel art */
    int jx = (int)luta->jogador_x - SPR_W/2;
    int jy = (int)ARENA_CHAO - SPR_H;
    DrawSprite_Jogador(jx, jy, g_atk_visual, luta->jogador_defendendo);

    /* ========== HUD ========== */
    DrawRectangle(0, 0, 1024, 68, (Color){0,0,0,200});
    DrawRectangle(0, 67, 1024, 2, (Color){180,150,30,255});

    /* Barra de vida do jogador - estilo pixel */
    DrawText("HP", 12, 10, 14, WHITE);
    DrawRectangle(36, 10, 200, 14, (Color){40,0,0,255});
    int vm2 = p->atrib.vida_max > 0 ? p->atrib.vida_max : 1;
    int bv2 = (int)(200.0f * (float)p->atrib.vida / (float)vm2);
    Color cv2 = p->atrib.vida > vm2/2 ? GREEN : p->atrib.vida > vm2/4 ? YELLOW : RED;
    DrawRectangle(36, 10, bv2, 14, cv2);
    /* Pixelado: linhas verticais a cada 20px */
    for (int gx = 36; gx < 236; gx += 20)
        DrawRectangle(gx, 10, 2, 14, (Color){0,0,0,80});
    DrawRectangleLines(36, 10, 200, 14, WHITE);

    /* XP bar */
    DrawText("XP", 12, 28, 12, LIGHTGRAY);
    int xpmax = p->xp_proximo_nivel > 0 ? p->xp_proximo_nivel : 1;
    int bxp = (int)(200.0f * (float)p->xp / (float)xpmax);
    DrawRectangle(36, 28, 200, 8, (Color){0,0,40,255});
    DrawRectangle(36, 28, bxp,  8, BLUE);
    DrawRectangleLines(36, 28, 200, 8, (Color){60,60,120,255});

    char buf[128];
    sprintf(buf, "Nivel %d", p->nivel);
    DrawText(buf, 12, 42, 12, LIGHTGRAY);

    /* Timer central - estilo placar */
    DrawRectangle(462, 4, 100, 44, (Color){20,20,40,255});
    DrawRectangleLines(462, 4, 100, 44, (Color){180,150,30,255});
    int seg = (int)luta->tempo_round;
    Color ct = seg > 30 ? WHITE : seg > 10 ? YELLOW : RED;
    sprintf(buf, "%d:%02d", seg/60, seg%60);
    DrawText(buf, 512 - MeasureText(buf,26)/2, 8, 26, ct);
    sprintf(buf, "RND %d", luta->round_atual);
    DrawText(buf, 512 - MeasureText(buf,11)/2, 36, 11, LIGHTGRAY);

    /* Info adversario - direita */
    sprintf(buf, "%s", adv->nome);
    DrawText(buf, 1012 - MeasureText(buf,13), 8, 13, WHITE);
    sprintf(buf, "%s", adv->estilo);
    DrawText(buf, 1012 - MeasureText(buf,11), 24, 11, adv->cor);
    sprintf(buf, "HP: %d/%d", vv, vmax);
    DrawText(buf, 1012 - MeasureText(buf,11), 38, 11, LIGHTGRAY);
    /* Barra HP adversario */
    DrawRectangle(812, 54, 200, 10, (Color){40,0,0,255});
    DrawRectangle(812, 54, bv*200/80, 10, cv);
    DrawRectangleLines(812, 54, 200, 10, WHITE);

    /* Golpes HUD - icones pixel */
    int gidx[4]        = {GOLPE_JAB, GOLPE_CROSS, GOLPE_HOOK, GOLPE_UPPERCUT};
    const char *glabel[4] = {"J:Jab", "K:Cross", "L:Hook", "I:Upcut"};
    for (int i = 0; i < 4; i++) {
        if (!p->golpes[gidx[i]].desbloqueado) continue;
        float t  = p->golpes[gidx[i]].timer_cooldown;
        float cd = p->golpes[gidx[i]].cooldown;
        int bx = 12 + i*100, by = 548;
        int pronto = (t <= 0.0f);
        DrawRectangle(bx, by, 90, 44, pronto ? (Color){0,40,0,220} : (Color){30,30,30,180});
        DrawRectangleLines(bx, by, 90, 44, pronto ? GREEN : GRAY);
        DrawText(glabel[i], bx+5, by+6, 14, pronto ? WHITE : GRAY);
        if (!pronto && cd > 0.0f) {
            int cw = (int)(90.0f*(t/cd));
            DrawRectangle(bx, by+34, cw, 10, (Color){180,80,0,220});
            DrawRectangle(bx, by+34, 90, 10, (Color){0,0,0,0});
            DrawRectangleLines(bx, by+34, 90, 10, (Color){100,50,0,200});
        } else {
            DrawRectangle(bx, by+34, 90, 10, (Color){0,120,0,180});
        }
    }
    DrawText("[ESPACO] Defender  [A][D] Mover", 430, 556, 12, (Color){120,120,140,255});

    /* Mensagem educativa */
    if (luta->timer_mensagem > 0.0f) {
        int mw = MeasureText(luta->mensagem, 16);
        DrawRectangle(512-mw/2-12, 76, mw+24, 26, (Color){0,0,0,210});
        DrawRectangleLines(512-mw/2-12, 76, mw+24, 26, GOLD);
        DrawText(luta->mensagem, 512-mw/2, 82, 16, YELLOW);
    }

    /* Tela de resultado */
    if (luta->luta_encerrada) {
        DrawRectangle(0, 0, 1024, 600, (Color){0,0,0,180});
        /* Caixa de resultado pixel */
        DrawRectangle(312, 180, 400, 140, (Color){20,20,40,255});
        DrawRectangleLines(312, 180, 400, 140, GOLD);
        DrawRectangleLines(314, 182, 396, 136, (Color){180,150,30,100});
        if (luta->jogador_venceu) {
            DrawText("VITORIA!", 512-MeasureText("VITORIA!",48)/2, 200, 48, GOLD);
        } else {
            DrawText("NOCAUTE!", 512-MeasureText("NOCAUTE!",48)/2, 200, 48, RED);
        }
        DrawText("Pressione ENTER",
                 512-MeasureText("Pressione ENTER",18)/2, 270, 18, WHITE);
        DrawText("para continuar",
                 512-MeasureText("para continuar",16)/2, 292, 16, LIGHTGRAY);
    }
}

/* ============================================================
   FightFinalizar
   ============================================================ */
void FightFinalizar(Jogo *j) {
    Adversario *adv = EnemyGetPorFase(j->adversarios, j->adversario_atual_idx);
    if (j->luta.jogador_venceu) {
        j->jogador.lutas_vencidas++;
        int xp = adv->nivel * 35 + GetRandomValue(15, 40);
        PlayerGanharXP(&j->jogador, xp);
        if (j->jogador.fase_atual == j->adversario_atual_idx)
            j->jogador.fase_atual++;
    } else {
        j->jogador.lutas_perdidas++;
        PlayerGanharXP(&j->jogador, 15);
    }
    /* Restaura vida completa apos qualquer resultado */
    j->jogador.atrib.vida = j->jogador.atrib.vida_max;
}

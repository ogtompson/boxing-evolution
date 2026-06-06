#include "fight.h"
#include "enemy.h"
#include "player.h"
#include <string.h>
#include <stdio.h>

#define ARENA_ESQ       210.0f
#define ARENA_DIR       810.0f
#define ARENA_CHAO      430.0f
#define VELOCIDADE_MOV  180.0f
#define TEMPO_ROUND     99.0f
#define CARENCIA_INICIO 0.3f

static float g_timer_carencia   = 0.0f;
static int   g_atk_visual       = 0;
static float g_atk_visual_timer = 0.0f;
static int   g_flash_hit        = 0;
static float g_flash_timer      = 0.0f;

void FightInit(Jogo *j, int adversario_idx) {
    j->adversario_atual_idx = adversario_idx;

    Adversario *adv      = &j->adversarios[adversario_idx];
    adv->atrib.vida      = adv->atrib.vida_max;
    adv->timer_golpe     = adv->cooldown_golpe + 1.0f;

    PlayerRestaurarVida(&j->jogador);

    j->luta.jogador_x           = ARENA_ESQ + 80.0f;
    j->luta.adversario_x        = ARENA_DIR  - 80.0f;
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

void FightUpdate(Jogo *j) {
    if (j->luta.luta_encerrada) return;

    /* Carencia: bloqueia input do jogador mas IA ja roda */
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
        if (luta->jogador_x < ARENA_ESQ + 20.0f) luta->jogador_x = ARENA_ESQ + 20.0f;
    }
    if (IsKeyDown(KEY_D)) {
        luta->jogador_x += VELOCIDADE_MOV * j->delta;
        if (luta->jogador_x > luta->adversario_x - 55.0f)
            luta->jogador_x = luta->adversario_x - 55.0f;
    }
    if (IsKeyDown(KEY_SPACE)) luta->jogador_defendendo = 1;

    float distancia = luta->adversario_x - luta->jogador_x;
    int em_alcance  = (distancia < 120.0f);

    int golpe_usado = -1;

    if (IsKeyPressed(KEY_J) && PlayerPodeUsarGolpe(p, GOLPE_JAB)) {
        golpe_usado = GOLPE_JAB;
        g_atk_visual = 1; g_atk_visual_timer = 0.25f;
        if (distancia < 150.0f) em_alcance = 1;
        if (em_alcance) { strcpy(luta->mensagem, "Jab! Controle a distancia!"); luta->timer_mensagem = 1.5f; }
    } else if (IsKeyPressed(KEY_K) && PlayerPodeUsarGolpe(p, GOLPE_CROSS)) {
        golpe_usado = GOLPE_CROSS;
        g_atk_visual = 2; g_atk_visual_timer = 0.3f;
        if (em_alcance) { strcpy(luta->mensagem, "Cross! Potencia maxima!"); luta->timer_mensagem = 1.5f; }
    } else if (IsKeyPressed(KEY_L) && PlayerPodeUsarGolpe(p, GOLPE_HOOK)) {
        golpe_usado = GOLPE_HOOK;
        g_atk_visual = 3; g_atk_visual_timer = 0.3f;
        if (em_alcance) { strcpy(luta->mensagem, "Hook! Perfeito de perto!"); luta->timer_mensagem = 1.5f; }
    } else if (IsKeyPressed(KEY_I) && PlayerPodeUsarGolpe(p, GOLPE_UPPERCUT)) {
        golpe_usado = GOLPE_UPPERCUT;
        g_atk_visual = 4; g_atk_visual_timer = 0.35f;
        if (em_alcance) { strcpy(luta->mensagem, "Uppercut! Na guarda aberta!"); luta->timer_mensagem = 1.5f; }
    }

    if (golpe_usado >= 0) {
        PlayerUsarGolpe(p, (TipoGolpe)golpe_usado);
        if (em_alcance) {
            int dano = PlayerCalcularDano(p, (TipoGolpe)golpe_usado);
            /* Bonus 60% se usar golpe ideal */
            int eh_ideal = 0;
            if (j->adversario_atual_idx == 0 && golpe_usado == GOLPE_JAB)      eh_ideal = 1;
            if (j->adversario_atual_idx == 1 && golpe_usado == GOLPE_CROSS)    eh_ideal = 1;
            if (j->adversario_atual_idx == 2 && golpe_usado == GOLPE_HOOK)     eh_ideal = 1;
            if (j->adversario_atual_idx == 3 && golpe_usado == GOLPE_UPPERCUT) eh_ideal = 1;
            if (j->adversario_atual_idx == 4 && golpe_usado != GOLPE_JAB)      eh_ideal = 1;
            if (eh_ideal) {
                dano = (int)((float)dano * 1.6f);
                strcpy(luta->mensagem, "GOLPE IDEAL! Dano +60%!");
                luta->timer_mensagem = 2.0f;
            }
            /* Penalidade: Jab faz menos dano contra adversarios avancados */
            if (golpe_usado == GOLPE_JAB && j->adversario_atual_idx >= 2) {
                dano = (int)((float)dano * 0.5f);
                if (luta->timer_mensagem <= 0.0f) {
                    strcpy(luta->mensagem, "Jab fraco aqui! Tente outro golpe.");
                    luta->timer_mensagem = 1.8f;
                }
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

/* Arena */
void FightDrawArena(void) {
    DrawRectangle(0, 0, 1024, 280, (Color){15, 10, 25, 255});
    DrawRectangle(0, 280, 1024, 320, (Color){20, 15, 30, 255});

    /* Plateia */
    for (int i = 0; i < 40; i++) {
        int px = 30 + (i % 20) * 50;
        int py = 30 + (i / 20) * 40;
        Color pc = (i%3==0) ? (Color){180,50,50,180} :
                   (i%3==1) ? (Color){50,50,180,180} : (Color){180,180,50,180};
        DrawCircle(px, py, 8, pc);
        DrawCircle(px, py-12, 7, (Color){220,180,130,200});
    }

    /* Lona */
    DrawRectangle(185, 350, 654, 170, (Color){180,40,40,255});
    DrawLine(185,400,839,400,(Color){160,30,30,255});
    DrawLine(185,440,839,440,(Color){160,30,30,255});
    DrawCircle(512, 435, 40, (Color){160,30,30,255});

    /* Cordas */
    Color corda = (Color){220,200,60,255};
    DrawLine(185,310,839,310,corda);
    DrawLine(185,335,839,335,corda);
    DrawLine(185,360,839,360,corda);

    /* Postes */
    DrawRectangle(183,300,10,222,(Color){160,130,30,255});
    DrawRectangle(831,300,10,222,(Color){160,130,30,255});
    DrawRectangle(180,296,16,8,(Color){200,170,50,255});
    DrawRectangle(828,296,16,8,(Color){200,170,50,255});
}

/* Jogador */
void FightDrawJogador(Jogador *p, float x, float y, int defendendo, int atk_tipo) {
    (void)p;
    int ix = (int)x;
    int iy = (int)y;
    Color azul     = (Color){30,100,220,255};
    Color azul_esc = (Color){15,50,110,255};
    Color pele     = (Color){220,180,130,255};

    int leg_off = (atk_tipo == 2) ? 5 : 0;
    DrawRectangle(ix-13+leg_off, iy-30, 11, 30, azul_esc);
    DrawRectangle(ix+2 +leg_off, iy-30, 11, 30, azul_esc);
    DrawRectangle(ix-16+leg_off, iy-2,  13,  6, (Color){40,40,40,255});
    DrawRectangle(ix+3 +leg_off, iy-2,  13,  6, (Color){40,40,40,255});
    DrawRectangle(ix-18, iy-68, 36, 40, azul);
    DrawCircle(ix, iy-82, 16, pele);
    DrawRectangle(ix-17, iy-96, 34, 16, (Color){200,30,30,255});
    DrawCircle(ix, iy-82, 17, (Color){200,30,30,80});

    if (defendendo) {
        DrawCircle(ix-14, iy-78, 13, (Color){200,30,30,255});
        DrawCircle(ix+14, iy-78, 13, (Color){200,30,30,255});
    } else if (atk_tipo == 1) {
        DrawCircle(ix+42, iy-65, 13, (Color){220,50,50,255});
        DrawCircle(ix-18, iy-70, 11, (Color){200,30,30,255});
        DrawCircle(ix+30, iy-65,  7, (Color){255,150,150,100});
    } else if (atk_tipo == 2) {
        DrawCircle(ix+48, iy-62, 14, (Color){220,50,50,255});
        DrawCircle(ix-16, iy-68, 11, (Color){200,30,30,255});
        DrawCircle(ix+33, iy-62,  8, (Color){255,150,150,120});
        DrawCircle(ix+20, iy-62,  5, (Color){255,150,150,60});
    } else if (atk_tipo == 3) {
        DrawCircle(ix+35, iy-76, 13, (Color){220,50,50,255});
        DrawCircle(ix-22, iy-68, 11, (Color){200,30,30,255});
    } else if (atk_tipo == 4) {
        DrawCircle(ix+20, iy-88, 13, (Color){220,50,50,255});
        DrawCircle(ix-20, iy-65, 11, (Color){200,30,30,255});
        DrawCircle(ix+20, iy-76,  6, (Color){255,150,150,100});
    } else {
        DrawCircle(ix+22, iy-68, 12, (Color){200,30,30,255});
        DrawCircle(ix-22, iy-68, 12, (Color){200,30,30,255});
    }
}

void FightDraw(Jogo *j) {
    Jogador    *p    = &j->jogador;
    Adversario *adv  = EnemyGetPorFase(j->adversarios, j->adversario_atual_idx);
    EstadoLuta *luta = &j->luta;

    ClearBackground((Color){15,10,25,255});
    FightDrawArena();

    if (g_flash_hit) DrawRectangle(0,0,1024,600,(Color){255,255,100,25});

    EnemyDraw(adv, luta->adversario_x, ARENA_CHAO, luta->adversario_atacando);
    FightDrawJogador(p, luta->jogador_x, ARENA_CHAO, luta->jogador_defendendo, g_atk_visual);

    /* HUD */
    DrawRectangle(0, 0, 1024, 70, (Color){0,0,0,180});
    PlayerDrawStatus(p, 12, 8);

    char buf[128];
    int seg = (int)luta->tempo_round;
    Color ct = (seg>30) ? WHITE : (seg>10) ? YELLOW : RED;
    sprintf(buf, "%d:%02d", seg/60, seg%60);
    DrawText(buf, 512-MeasureText(buf,28)/2, 8, 28, ct);
    sprintf(buf, "Round %d", luta->round_atual);
    DrawText(buf, 512-MeasureText(buf,12)/2, 42, 12, LIGHTGRAY);

    sprintf(buf, "%s | %s", adv->nome, adv->estilo);
    DrawText(buf, 1012-MeasureText(buf,13), 8, 13, WHITE);
    int vmax = adv->atrib.vida_max > 0 ? adv->atrib.vida_max : 1;
    int vv   = adv->atrib.vida < 0 ? 0 : adv->atrib.vida;
    int bv   = (int)(200.0f*(float)vv/(float)vmax);
    DrawRectangle(812,26,200,12,DARKGRAY);
    Color cv = (vv>vmax*2/3)?GREEN:(vv>vmax/3)?YELLOW:RED;
    DrawRectangle(812,26,bv,12,cv);
    DrawRectangleLines(812,26,200,12,WHITE);
    sprintf(buf,"HP: %d/%d",vv,vmax);
    DrawText(buf,812,42,11,LIGHTGRAY);

    /* Golpes HUD com cooldown visual */
    int gidx[4] = {GOLPE_JAB,GOLPE_CROSS,GOLPE_HOOK,GOLPE_UPPERCUT};
    const char *glabel[4] = {"J:Jab","K:Cross","L:Hook","I:Upcut"};
    for (int i = 0; i < 4; i++) {
        if (!p->golpes[gidx[i]].desbloqueado) continue;
        float t  = p->golpes[gidx[i]].timer_cooldown;
        float cd = p->golpes[gidx[i]].cooldown;
        int bx = 12 + i*90, by = 555;
        Color bc = (t<=0.0f) ? (Color){0,160,0,200} : (Color){80,80,80,180};
        DrawRectangle(bx,by,80,36,bc);
        DrawRectangleLines(bx,by,80,36,WHITE);
        DrawText(glabel[i],bx+5,by+5,13,WHITE);
        if (t > 0.0f && cd > 0.0f) {
            int cw = (int)(80.0f*(t/cd));
            DrawRectangle(bx,by+28,cw,8,(Color){200,100,0,220});
        }
    }
    DrawText("[ESPACO] Defender   [A][D] Mover", 380, 560, 12, LIGHTGRAY);

    if (luta->timer_mensagem > 0.0f) {
        int mw = MeasureText(luta->mensagem,15);
        DrawRectangle(512-mw/2-10,80,mw+20,28,(Color){0,0,0,200});
        DrawText(luta->mensagem,512-mw/2,86,15,YELLOW);
    }

    if (luta->luta_encerrada) {
        DrawRectangle(0,0,1024,600,(Color){0,0,0,170});
        if (luta->jogador_venceu) {
            DrawText("VITORIA!", 512-MeasureText("VITORIA!",52)/2, 200, 52, GOLD);
        } else {
            DrawText("NOCAUTE!", 512-MeasureText("NOCAUTE!",52)/2, 200, 52, RED);
        }
        DrawText("Pressione ENTER para continuar",
                 512-MeasureText("Pressione ENTER para continuar",18)/2, 290, 18, WHITE);
    }
}

void FightFinalizar(Jogo *j) {
    Adversario *adv = EnemyGetPorFase(j->adversarios, j->adversario_atual_idx);
    if (j->luta.jogador_venceu) {
        j->jogador.lutas_vencidas++;
        int xp = adv->nivel * 35 + GetRandomValue(15,40);
        PlayerGanharXP(&j->jogador, xp);
        if (j->jogador.fase_atual == j->adversario_atual_idx)
            j->jogador.fase_atual++;
    } else {
        j->jogador.lutas_perdidas++;
        PlayerGanharXP(&j->jogador, 15);
    }
}

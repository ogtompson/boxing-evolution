#include "enemy.h"
#include "player.h"
#include <string.h>

void EnemyInitTodos(Adversario adversarios[MAX_ADVERSARIOS]) {

    /* 0: Iniciante - lento, fraco, serve para aprender o Jab */
    strcpy(adversarios[0].nome,          "Carlos Novato");
    strcpy(adversarios[0].estilo,         "Iniciante");
    strcpy(adversarios[0].dica_pre_luta,
        "Carlos nao sabe defender bem.\n"
        "Use o JAB (J) para controlar a distancia\n"
        "e mantenha-se em movimento!");
    adversarios[0].atrib.vida_max    = 100;
    adversarios[0].atrib.vida        = 100;
    adversarios[0].atrib.tecnica     = 3;
    adversarios[0].atrib.velocidade  = 3;
    adversarios[0].atrib.resistencia = 3;
    adversarios[0].atrib.potencia    = 5;
    adversarios[0].nivel             = 1;
    adversarios[0].cor               = SKYBLUE;
    adversarios[0].avanca            = 1;
    adversarios[0].cooldown_golpe    = 2.0f;
    adversarios[0].timer_golpe       = 2.0f;
    adversarios[0].vel_movimento     = 80.0f;

    /* 1: Agressivo - rapido e forte, precisa do Cross para parar */
    strcpy(adversarios[1].nome,          "Bruno Furia");
    strcpy(adversarios[1].estilo,         "Agressivo");
    strcpy(adversarios[1].dica_pre_luta,
        "Bruno avanca sem parar e bate forte.\n"
        "Mantenha distancia com JAB e use\n"
        "CROSS (K) quando ele abrir a guarda!");
    adversarios[1].atrib.vida_max    = 140;
    adversarios[1].atrib.vida        = 140;
    adversarios[1].atrib.tecnica     = 5;
    adversarios[1].atrib.velocidade  = 9;
    adversarios[1].atrib.resistencia = 8;
    adversarios[1].atrib.potencia    = 14;
    adversarios[1].nivel             = 3;
    adversarios[1].cor               = ORANGE;
    adversarios[1].avanca            = 1;
    adversarios[1].cooldown_golpe    = 1.1f;
    adversarios[1].timer_golpe       = 1.1f;
    adversarios[1].vel_movimento     = 130.0f;

    /* 2: Defensivo - recua e bloqueia, Hook lateral quebra a defesa */
    strcpy(adversarios[2].nome,          "Diego Muralha");
    strcpy(adversarios[2].estilo,         "Defensivo");
    strcpy(adversarios[2].dica_pre_luta,
        "Diego recua e bloqueia golpes frontais.\n"
        "Avance e use o HOOK (L) de perto,\n"
        "ele nao consegue bloquear pela lateral!");
    adversarios[2].atrib.vida_max    = 160;
    adversarios[2].atrib.vida        = 160;
    adversarios[2].atrib.tecnica     = 12;
    adversarios[2].atrib.velocidade  = 5;
    adversarios[2].atrib.resistencia = 14;
    adversarios[2].atrib.potencia    = 9;
    adversarios[2].nivel             = 5;
    adversarios[2].cor               = GREEN;
    adversarios[2].avanca            = 0;
    adversarios[2].cooldown_golpe    = 1.6f;
    adversarios[2].timer_golpe       = 1.6f;
    adversarios[2].vel_movimento     = 90.0f;

    /* 3: Tecnico - rapido e esperto, Uppercut quando ele contra-ataca */
    strcpy(adversarios[3].nome,          "Ricardo Tecnico");
    strcpy(adversarios[3].estilo,         "Tecnico");
    strcpy(adversarios[3].dica_pre_luta,
        "Ricardo contra-ataca rapidamente.\n"
        "Quando ele atacar use UPPERCUT (I),\n"
        "ele fica com a guarda aberta!");
    adversarios[3].atrib.vida_max    = 150;
    adversarios[3].atrib.vida        = 150;
    adversarios[3].atrib.tecnica     = 14;
    adversarios[3].atrib.velocidade  = 13;
    adversarios[3].atrib.resistencia = 10;
    adversarios[3].atrib.potencia    = 12;
    adversarios[3].nivel             = 8;
    adversarios[3].cor               = PURPLE;
    adversarios[3].avanca            = 1;
    adversarios[3].cooldown_golpe    = 0.9f;
    adversarios[3].timer_golpe       = 0.9f;
    adversarios[3].vel_movimento     = 140.0f;

    /* 4: Campeao - usa tudo, exige combinacoes */
    strcpy(adversarios[4].nome,          "El Campeon");
    strcpy(adversarios[4].estilo,         "Campeao");
    strcpy(adversarios[4].dica_pre_luta,
        "O campeao domina todos os golpes.\n"
        "Use COMBINACOES: JAB para abrir,\n"
        "CROSS ou HOOK para finalizar!");
    adversarios[4].atrib.vida_max    = 180;
    adversarios[4].atrib.vida        = 180;
    adversarios[4].atrib.tecnica     = 16;
    adversarios[4].atrib.velocidade  = 14;
    adversarios[4].atrib.resistencia = 15;
    adversarios[4].atrib.potencia    = 16;
    adversarios[4].nivel             = 12;
    adversarios[4].cor               = GOLD;
    adversarios[4].avanca            = 1;
    adversarios[4].cooldown_golpe    = 0.7f;
    adversarios[4].timer_golpe       = 0.7f;
    adversarios[4].vel_movimento     = 155.0f;
}

Adversario *EnemyGetPorFase(Adversario adversarios[], int fase) {
    if (fase < 0 || fase >= MAX_ADVERSARIOS) return &adversarios[0];
    return &adversarios[fase];
}

void EnemyUpdateIA(Jogo *j) {
    EstadoLuta *luta    = &j->luta;
    Adversario *adv     = EnemyGetPorFase(j->adversarios, j->adversario_atual_idx);
    Jogador    *jogador = &j->jogador;

    if (luta->luta_encerrada) return;

    float distancia = luta->adversario_x - luta->jogador_x;

    /* Movimento: avancador persegue, defensivo recua */
    if (adv->avanca) {
        if (distancia > 90.0f) {
            luta->adversario_x -= adv->vel_movimento * j->delta;
            if (luta->adversario_x < luta->jogador_x + 90.0f)
                luta->adversario_x = luta->jogador_x + 90.0f;
        } else if (distancia < 65.0f) {
            luta->adversario_x += 70.0f * j->delta;
        }
    } else {
        /* Defensivo: recua mas ataca quando o jogador chega perto */
        if (distancia < 140.0f) {
            luta->adversario_x += adv->vel_movimento * j->delta;
        }
        if (luta->adversario_x > 780.0f)
            luta->adversario_x = 780.0f;
    }

    /* Ataque */
    adv->timer_golpe -= j->delta;
    if (adv->timer_golpe <= 0.0f) {
        adv->timer_golpe = adv->cooldown_golpe;

        if (distancia < 130.0f) {
            luta->adversario_atacando = 1;
            int dano = EnemyCalcularDano(adv);
            if (luta->jogador_defendendo)
                dano = (int)((float)dano * 0.3f);
            PlayerReceberDano(jogador, dano, adv->atrib.potencia);
            if (jogador->atrib.vida <= 0) {
                luta->luta_encerrada = 1;
                luta->jogador_venceu = 0;
            }
        }
    } else {
        if (adv->timer_golpe < adv->cooldown_golpe - 0.25f)
            luta->adversario_atacando = 0;
    }
}

void EnemyDraw(Adversario *adv, float x, float y, int atacando) {
    int ix = (int)x;
    int iy = (int)y;
    Color cor       = adv->cor;
    Color cor_pele  = (Color){220,180,130,255};
    Color cor_short = (Color){(unsigned char)(cor.r/2),
                               (unsigned char)(cor.g/2),
                               (unsigned char)(cor.b/2),255};

    DrawRectangle(ix-14, iy-30, 12, 30, cor_short);
    DrawRectangle(ix+2,  iy-30, 12, 30, cor_short);
    DrawRectangle(ix-16, iy-2,  14,  6, DARKGRAY);
    DrawRectangle(ix+2,  iy-2,  14,  6, DARKGRAY);
    DrawRectangle(ix-18, iy-68, 36, 40, cor);
    DrawCircle(ix, iy-82, 16, cor_pele);
    DrawCircle(ix, iy-82, 17, (Color){cor.r,cor.g,cor.b,100});
    DrawRectangle(ix-17,iy-92,34,10,cor);

    if (atacando) {
        /* Soco esticado para a esquerda (em direcao ao jogador) */
        DrawCircle(ix-40, iy-62, 13, RED);
        DrawCircle(ix+16, iy-60, 11, RED);
        DrawCircle(ix-28, iy-62,  6, (Color){255,100,100,120});
    } else {
        DrawCircle(ix-20, iy-72, 12, RED);
        DrawCircle(ix+20, iy-72, 12, RED);
    }

    /* Barra de vida */
    int vmax = adv->atrib.vida_max > 0 ? adv->atrib.vida_max : 1;
    int vida = adv->atrib.vida < 0 ? 0 : adv->atrib.vida;
    int bv   = (int)(100.0f*(float)vida/(float)vmax);
    DrawRectangle(ix-50,iy-108,100,8,DARKGRAY);
    Color cv = (vida>vmax*2/3)?GREEN:(vida>vmax/3)?YELLOW:RED;
    DrawRectangle(ix-50,iy-108,bv,8,cv);
    DrawRectangleLines(ix-50,iy-108,100,8,WHITE);
    int nw = MeasureText(adv->nome,11);
    DrawText(adv->nome,ix-nw/2,iy-122,11,WHITE);
}

int EnemyCalcularDano(Adversario *adv) {
    return adv->atrib.potencia * 2 + GetRandomValue(0, adv->atrib.potencia);
}

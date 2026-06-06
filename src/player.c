#include "player.h"
#include <string.h>
#include <stdio.h>

static const char *GOLPE_NOMES[MAX_GOLPES]       = {"Jab","Cross","Hook","Uppercut"};
static const char *GOLPE_DESCRICOES[MAX_GOLPES]  = {
    "O Jab e o golpe mais usado no boxe. Rapido e direto,\nserve para controlar a distancia e abrir a guarda.",
    "O Cross utiliza a rotacao do quadril e do corpo\npara gerar mais forca. E o golpe mais potente.",
    "O Hook e eficiente em curta distancia.\nAtaca pela lateral, dificil de defender.",
    "O Uppercut e usado quando o adversario tem guarda\naberta. Sobe de baixo para cima, direto ao queixo."
};
static const char *GOLPE_TECLAS[MAX_GOLPES]      = {"J","K","L","I"};
static const int   GOLPE_DANO_BASE[MAX_GOLPES]   = {4, 15, 13, 21};
static const int   GOLPE_NIVEL[MAX_GOLPES]        = {1,  3,  5,  8};
static const float GOLPE_COOLDOWN[MAX_GOLPES]    = {1.0f, 0.85f, 0.75f, 1.1f};

void PlayerInit(Jogador *p) {
    memset(p, 0, sizeof(Jogador));
    strcpy(p->nome, "Lutador");
    p->atrib.vida        = 100;
    p->atrib.vida_max    = 100;   /* NUNCA pode ser 0 */
    p->atrib.tecnica     = 5;
    p->atrib.velocidade  = 5;
    p->atrib.resistencia = 5;
    p->atrib.potencia    = 5;
    p->xp                = 0;
    p->xp_proximo_nivel  = 100;
    p->nivel             = 1;
    p->pontos_atributo   = 0;
    p->fase_atual        = 0;
    p->lutas_vencidas    = 0;
    p->lutas_perdidas    = 0;

    for (int i = 0; i < MAX_GOLPES; i++) {
        strncpy(p->golpes[i].nome,      GOLPE_NOMES[i],      31);
        strncpy(p->golpes[i].descricao, GOLPE_DESCRICOES[i], 127);
        strncpy(p->golpes[i].tecla_str, GOLPE_TECLAS[i],     7);
        p->golpes[i].dano_base        = GOLPE_DANO_BASE[i];
        p->golpes[i].nivel_necessario = GOLPE_NIVEL[i];
        p->golpes[i].cooldown         = GOLPE_COOLDOWN[i];
        p->golpes[i].timer_cooldown   = 0.0f;
        p->golpes[i].desbloqueado     = (i == GOLPE_JAB) ? 1 : 0;
    }
}

int PlayerGanharXP(Jogador *p, int quantidade) {
    p->xp += quantidade;
    if (p->xp >= p->xp_proximo_nivel) {
        p->xp -= p->xp_proximo_nivel;
        p->nivel++;
        p->xp_proximo_nivel = p->nivel * 100 + 50;
        p->pontos_atributo += 3;
        PlayerAtualizarGolpes(p);
        return 1;
    }
    return 0;
}

void PlayerAtualizarGolpes(Jogador *p) {
    for (int i = 0; i < MAX_GOLPES; i++) {
        if (p->nivel >= p->golpes[i].nivel_necessario)
            p->golpes[i].desbloqueado = 1;
    }
}

void PlayerReceberDano(Jogador *p, int dano, int adversario_potencia) {
    float reducao = (float)p->atrib.tecnica / 25.0f;
    if (reducao > 0.4f) reducao = 0.4f;
    int dano_final = (int)((float)(dano + adversario_potencia / 2) * (1.0f - reducao));
    if (dano_final < 1) dano_final = 1;
    p->atrib.vida -= dano_final;
    if (p->atrib.vida < 0) p->atrib.vida = 0;
}

void PlayerDistribuirPonto(Jogador *p, int atributo) {
    if (p->pontos_atributo <= 0) return;
    switch (atributo) {
        case 0: p->atrib.vida_max += 10; p->atrib.vida += 10; break;
        case 1: p->atrib.tecnica++;      break;
        case 2: p->atrib.velocidade++;   break;
        case 3: p->atrib.resistencia++;  break;
        case 4: p->atrib.potencia++;     break;
        default: return;
    }
    p->pontos_atributo--;
}

void PlayerRestaurarVida(Jogador *p) {
    /* Garante que vida_max nunca e 0 antes de restaurar */
    if (p->atrib.vida_max <= 0) p->atrib.vida_max = 100;
    int recuperacao = 30 + p->atrib.resistencia * 3;
    p->atrib.vida += recuperacao;
    if (p->atrib.vida > p->atrib.vida_max)
        p->atrib.vida = p->atrib.vida_max;
}

int PlayerCalcularDano(Jogador *p, TipoGolpe golpe) {
    int base = p->golpes[golpe].dano_base;
    float mult = 1.0f + (float)p->atrib.potencia / 20.0f;
    int critico = (GetRandomValue(1, 100) <= (10 + p->atrib.velocidade)) ? 1 : 0;
    int dano = (int)((float)base * mult);
    if (critico) dano = (int)((float)dano * 1.5f);
    return dano;
}

void PlayerAtualizarCooldowns(Jogador *p, float delta) {
    for (int i = 0; i < MAX_GOLPES; i++) {
        if (p->golpes[i].timer_cooldown > 0.0f) {
            p->golpes[i].timer_cooldown -= delta;
            if (p->golpes[i].timer_cooldown < 0.0f)
                p->golpes[i].timer_cooldown = 0.0f;
        }
    }
}

int PlayerPodeUsarGolpe(Jogador *p, TipoGolpe golpe) {
    return (p->golpes[golpe].desbloqueado &&
            p->golpes[golpe].timer_cooldown <= 0.0f);
}

void PlayerUsarGolpe(Jogador *p, TipoGolpe golpe) {
    p->golpes[golpe].timer_cooldown = p->golpes[golpe].cooldown;
}

void PlayerDrawStatus(Jogador *p, int x, int y) {
    char buf[128]; /* aumentado para evitar overflow */

    snprintf(buf, sizeof(buf), "%s  Nivel %d", p->nome, p->nivel);
    DrawText(buf, x, y, 18, WHITE);

    /* Barra de vida - protege contra vida_max == 0 */
    int vmax = (p->atrib.vida_max > 0) ? p->atrib.vida_max : 1;
    int vida  = p->atrib.vida;
    if (vida < 0) vida = 0;
    if (vida > vmax) vida = vmax;

    DrawRectangle(x, y + 24, 200, 12, DARKGRAY);
    int barra_vida = (int)(200.0f * (float)vida / (float)vmax);
    Color cor_vida = (vida > vmax/2) ? GREEN :
                     (vida > vmax/4) ? YELLOW : RED;
    DrawRectangle(x, y + 24, barra_vida, 12, cor_vida);
    DrawRectangleLines(x, y + 24, 200, 12, WHITE);
    snprintf(buf, sizeof(buf), "HP: %d/%d", vida, vmax);
    DrawText(buf, x + 205, y + 24, 12, WHITE);

    /* Barra de XP - protege contra xp_proximo_nivel == 0 */
    int xpmax = (p->xp_proximo_nivel > 0) ? p->xp_proximo_nivel : 1;
    DrawRectangle(x, y + 40, 200, 8, DARKGRAY);
    int barra_xp = (int)(200.0f * (float)p->xp / (float)xpmax);
    if (barra_xp < 0) barra_xp = 0;
    if (barra_xp > 200) barra_xp = 200;
    DrawRectangle(x, y + 40, barra_xp, 8, BLUE);
    DrawRectangleLines(x, y + 40, 200, 8, WHITE);
    snprintf(buf, sizeof(buf), "XP: %d/%d", p->xp, xpmax);
    DrawText(buf, x + 205, y + 40, 12, SKYBLUE);
}

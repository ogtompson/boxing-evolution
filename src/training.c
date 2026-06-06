#include "training.h"
#include "player.h"
#include <string.h>
#include <stdio.h>

/* Teclas usadas nos minigames (A, S, D, F, J, K, L) */
static const int TECLAS_TREINO[] = {KEY_A, KEY_S, KEY_D, KEY_F, KEY_J, KEY_K, KEY_L};
static const char *NOME_TECLAS[] = {"A", "S", "D", "F", "J", "K", "L"};
#define N_TECLAS 7

/* ============================================================
   TreinoGetConhecimento
   Texto educativo integrado a cada tipo de treino
   ============================================================ */
const char *TreinoGetConhecimento(TipoTreino tipo) {
    switch (tipo) {
        case TREINO_VELOCIDADE:
            return "Tempo de reacao e um dos fatores mais importantes\n"
                   "para um boxeador. Lutadores de elite reagem a\n"
                   "estimulos visuais em menos de 200 milissegundos!";
        case TREINO_POTENCIA:
            return "O Cross utiliza a rotacao do quadril para gerar\n"
                   "mais forca. A potencia no boxe nao vem apenas\n"
                   "dos braccos, mas de todo o corpo.";
        case TREINO_RESISTENCIA:
            return "Lutadores profissionais podem manter frequencia\n"
                   "cardiaca acima de 170 BPM durante varios rounds.\n"
                   "Resistencia e essencial para o combate prolongado.";
        default: return "";
    }
}

/* ============================================================
   TreinoInit
   ============================================================ */
void TrainoInit(EstadoTreino *t, TipoTreino tipo) {
    memset(t, 0, sizeof(EstadoTreino));
    t->tipo       = tipo;
    t->rodadas    = 0;
    t->rodadas_max = 5;
    t->acertos    = 0;
    t->ativo      = 1;

    switch (tipo) {
        case TREINO_VELOCIDADE:
            t->janela_reacao = 0.8f;
            t->timer_tecla   = 1.5f;
            t->tecla_visivel = 0;
            break;

        case TREINO_POTENCIA:
            t->barra_pos   = 0.0f;
            t->barra_dir   = 0.6f;
            t->zona_inicio = 0.35f;
            t->zona_fim    = 0.65f;
            break;

        case TREINO_RESISTENCIA:
            t->seq_len   = 5;
            t->seq_idx   = 0;
            t->timer_seq = 1.2f;
            /* Gera sequencia aleatoria */
            for (int i = 0; i < t->seq_len; i++) {
                t->seq[i] = TECLAS_TREINO[GetRandomValue(0, N_TECLAS - 1)];
            }
            break;
    }
}

/* ============================================================
   TreinoUpdate  -  logica separada por tipo de treino
   ============================================================ */
void TreinoUpdate(Jogo *j, EstadoTreino *t) {
    if (!t->ativo) return;

    switch (t->tipo) {

    /* -- TREINO DE VELOCIDADE -- */
    case TREINO_VELOCIDADE: {
        t->timer_tecla -= j->delta;

        if (!t->tecla_visivel) {
            if (t->timer_tecla <= 0.0f) {
                /* Escolhe tecla aleatoria */
                t->tecla_alvo    = TECLAS_TREINO[GetRandomValue(0, N_TECLAS - 1)];
                t->tecla_visivel = 1;
                t->timer_tecla   = t->janela_reacao;
            }
        } else {
            /* Verifica se o jogador pressionou a tecla certa */
            if (IsKeyPressed(t->tecla_alvo)) {
                t->acertos++;
                t->tecla_visivel = 0;
                t->timer_tecla   = 1.0f + GetRandomValue(0, 100) / 100.0f;
                t->rodadas++;
                /* Treino fica mais dificil progressivamente */
                t->janela_reacao -= 0.03f;
                if (t->janela_reacao < 0.3f) t->janela_reacao = 0.3f;
            } else if (t->timer_tecla <= 0.0f) {
                /* Perdeu a janela - tecla some sem acertar */
                t->tecla_visivel = 0;
                t->timer_tecla   = 1.0f;
                t->rodadas++;
            } else {
                /* Pressionou tecla errada */
                for (int k = 0; k < N_TECLAS; k++) {
                    if (IsKeyPressed(TECLAS_TREINO[k]) && TECLAS_TREINO[k] != t->tecla_alvo) {
                        t->tecla_visivel = 0;
                        t->timer_tecla   = 1.2f;
                        t->rodadas++;
                        break;
                    }
                }
            }
        }
        if (t->rodadas >= t->rodadas_max) t->ativo = 0;
        break;
    }

    /* -- TREINO DE POTENCIA -- */
    case TREINO_POTENCIA: {
        /* Move a barra (vai e volta) */
        t->barra_pos += t->barra_dir * j->delta;
        if (t->barra_pos >= 1.0f) { t->barra_pos = 1.0f; t->barra_dir = -t->barra_dir; }
        if (t->barra_pos <= 0.0f) { t->barra_pos = 0.0f; t->barra_dir = -t->barra_dir; }

        /* Jogador pressiona ESPACO no momento certo */
        if (IsKeyPressed(KEY_SPACE)) {
            if (t->barra_pos >= t->zona_inicio && t->barra_pos <= t->zona_fim) {
                t->acertos++;
                /* Zona fica menor a cada acerto */
                float largura = t->zona_fim - t->zona_inicio;
                largura -= 0.04f;
                if (largura < 0.10f) largura = 0.10f;
                float centro = (t->zona_inicio + t->zona_fim) / 2.0f;
                t->zona_inicio = centro - largura / 2.0f;
                t->zona_fim    = centro + largura / 2.0f;
                /* Velocidade aumenta */
                t->barra_dir = (t->barra_dir > 0) ?
                    (t->barra_dir + 0.05f) : (t->barra_dir - 0.05f);
            }
            t->rodadas++;
            if (t->rodadas >= t->rodadas_max) t->ativo = 0;
        }
        break;
    }

    /* -- TREINO DE RESISTENCIA -- */
    case TREINO_RESISTENCIA: {
        if (t->seq_idx >= t->seq_len) {
            /* Completou a sequencia */
            t->acertos++;
            t->rodadas++;
            /* Reinicia sequencia maior */
            t->seq_idx = 0;
            t->seq_len++;
            if (t->seq_len > 8) t->seq_len = 8;
            for (int i = 0; i < t->seq_len; i++) {
                t->seq[i] = TECLAS_TREINO[GetRandomValue(0, N_TECLAS - 1)];
            }
            if (t->rodadas >= t->rodadas_max) { t->ativo = 0; break; }
        }

        /* Verifica tecla esperada na sequencia */
        int tecla_esperada = t->seq[t->seq_idx];
        for (int k = 0; k < N_TECLAS; k++) {
            if (IsKeyPressed(TECLAS_TREINO[k])) {
                if (TECLAS_TREINO[k] == tecla_esperada) {
                    t->seq_idx++;
                } else {
                    /* Errou - reinicia sequencia */
                    t->seq_idx = 0;
                }
                break;
            }
        }
        break;
    }
    }
}

/* ============================================================
   TreinoDraw  -  renderiza o minigame conforme o tipo
   ============================================================ */
void TreinoDraw(Jogo *j, EstadoTreino *t) {
    (void)j;
    char buf[128];
    ClearBackground((Color){15, 20, 40, 255});

    /* Titulo do treino */
    const char *titulos[] = {"TREINO DE VELOCIDADE", "TREINO DE POTENCIA", "TREINO DE RESISTENCIA"};
    DrawText(titulos[t->tipo], SCREEN_W / 2 - 150, 20, 24, GOLD);

    /* Texto educativo (sempre visivel) */
    const char *conhecimento = TreinoGetConhecimento(t->tipo);
    DrawText(conhecimento, 40, 60, 14, LIGHTGRAY);

    DrawLine(0, 130, SCREEN_W, 130, DARKGRAY);

    /* Progresso */
    sprintf(buf, "Rodada %d de %d  |  Acertos: %d", t->rodadas, t->rodadas_max, t->acertos);
    DrawText(buf, 40, 140, 14, WHITE);

    /* -- Desenho especifico por tipo -- */
    switch (t->tipo) {

    case TREINO_VELOCIDADE:
        DrawText("Pressione a tecla quando aparecer na tela!", 40, 170, 16, WHITE);
        if (t->tecla_visivel) {
            /* Acha o nome da tecla */
            const char *nome_tecla = "?";
            for (int k = 0; k < N_TECLAS; k++) {
                if (TECLAS_TREINO[k] == t->tecla_alvo) { nome_tecla = NOME_TECLAS[k]; break; }
            }
            /* Caixa grande com a tecla */
            DrawRectangle(SCREEN_W/2 - 60, 220, 120, 120, (Color){50, 100, 200, 255});
            DrawRectangleLines(SCREEN_W/2 - 60, 220, 120, 120, WHITE);
            DrawText(nome_tecla, SCREEN_W/2 - 20, 255, 60, WHITE);
            /* Barra de tempo (diminui) */
            int barra = (int)(300.0f * t->timer_tecla / t->janela_reacao);
            DrawRectangle(SCREEN_W/2 - 150, 360, 300, 16, DARKGRAY);
            DrawRectangle(SCREEN_W/2 - 150, 360, barra, 16,
                          t->timer_tecla > t->janela_reacao * 0.4f ? GREEN : RED);
            DrawRectangleLines(SCREEN_W/2 - 150, 360, 300, 16, WHITE);
        } else {
            DrawText("Prepare-se...", SCREEN_W/2 - 60, 280, 20, GRAY);
        }
        break;

    case TREINO_POTENCIA:
        DrawText("Pressione ESPACO quando o indicador estiver na zona verde!", 40, 170, 14, WHITE);
        /* Barra de forca */
        DrawRectangle(SCREEN_W/2 - 200, 260, 400, 40, DARKGRAY);
        /* Zona alvo (verde) */
        int zona_x = SCREEN_W/2 - 200 + (int)(t->zona_inicio * 400);
        int zona_w = (int)((t->zona_fim - t->zona_inicio) * 400);
        DrawRectangle(zona_x, 260, zona_w, 40, (Color){0, 180, 0, 180});
        /* Indicador */
        int ind_x = SCREEN_W/2 - 200 + (int)(t->barra_pos * 400);
        DrawRectangle(ind_x - 4, 250, 8, 60, YELLOW);
        DrawRectangleLines(SCREEN_W/2 - 200, 260, 400, 40, WHITE);
        DrawText("[ESPACO]", SCREEN_W/2 - 40, 330, 16, WHITE);
        break;

    case TREINO_RESISTENCIA:
        DrawText("Pressione as teclas na sequencia!", 40, 170, 16, WHITE);
        /* Mostra a sequencia */
        for (int i = 0; i < t->seq_len; i++) {
            const char *nome = "?";
            for (int k = 0; k < N_TECLAS; k++) {
                if (TECLAS_TREINO[k] == t->seq[i]) { nome = NOME_TECLAS[k]; break; }
            }
            Color cor = (i < t->seq_idx) ? GREEN :
                        (i == t->seq_idx) ? YELLOW : WHITE;
            int bx = SCREEN_W/2 - (t->seq_len * 50)/2 + i * 50;
            DrawRectangle(bx, 240, 40, 40, (i == t->seq_idx) ?
                          (Color){80, 80, 20, 255} : (Color){40, 40, 60, 255});
            DrawRectangleLines(bx, 240, 40, 40, cor);
            DrawText(nome, bx + 14, 252, 18, cor);
        }
        sprintf(buf, "Proxima tecla: %d de %d", t->seq_idx + 1, t->seq_len);
        DrawText(buf, SCREEN_W/2 - 80, 310, 14, LIGHTGRAY);
        break;
    }

    /* Resultado final */
    if (!t->ativo) {
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H, (Color){0, 0, 0, 180});
        DrawText("TREINO CONCLUIDO!", SCREEN_W/2 - 120, SCREEN_H/2 - 40, 32, GOLD);
        sprintf(buf, "Acertos: %d / %d", t->acertos, t->rodadas_max);
        DrawText(buf, SCREEN_W/2 - 80, SCREEN_H/2 + 10, 20, WHITE);
        DrawText("Pressione ENTER para continuar", SCREEN_W/2 - 140, SCREEN_H/2 + 50, 16, LIGHTGRAY);
    }
}

/* ============================================================
   TreinoAplicarRecompensa
   Aumenta atributo conforme acertos no treino
   ============================================================ */
void TreinoAplicarRecompensa(Jogo *j, EstadoTreino *t) {
    int bonus = t->acertos;
    if (bonus < 1) bonus = 1;

    switch (t->tipo) {
        case TREINO_VELOCIDADE:
            j->jogador.atrib.velocidade += bonus / 2 + 1;
            PlayerGanharXP(&j->jogador, bonus * 10);
            break;
        case TREINO_POTENCIA:
            j->jogador.atrib.potencia += bonus / 2 + 1;
            PlayerGanharXP(&j->jogador, bonus * 10);
            break;
        case TREINO_RESISTENCIA:
            j->jogador.atrib.resistencia += bonus / 2 + 1;
            j->jogador.atrib.vida_max    += bonus;
            PlayerGanharXP(&j->jogador, bonus * 10);
            break;
    }
}

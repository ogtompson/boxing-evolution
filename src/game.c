#include "game.h"
#include "player.h"
#include "enemy.h"
#include "fight.h"
#include "training.h"
#include "knowledge.h"
#include "save.h"
#include <string.h>
#include <stdio.h>

/* Estado global de treino - persiste entre frames */
static EstadoTreino g_treino;

/* Flag: recompensa do treino ja foi aplicada nesta sessao */
static int g_treino_recompensa_aplicada = 0;

/* ============================================================
   JogoMudarEstado
   ============================================================ */
void JogoMudarEstado(Jogo *j, GameState novo) {
    j->estado_atual = novo;
}

/* ============================================================
   JogoInit
   ============================================================ */
void JogoInit(Jogo *j) {
    memset(j, 0, sizeof(Jogo));
    j->estado_atual   = STATE_MENU;
    j->menu_opcao     = 0;
    j->academia_opcao = 0;
    PlayerInit(&j->jogador);
    EnemyInitTodos(j->adversarios);
}

/* ============================================================
   Helpers de navegacao: evita repeticao
   ============================================================ */
static void NavUp(int *opcao, int total) {
    *opcao = (*opcao - 1 + total) % total;
}
static void NavDown(int *opcao, int total) {
    *opcao = (*opcao + 1) % total;
}

/* ============================================================
   Abre tela de conhecimento com destino explicito
   ============================================================ */
static void AbrirConhecimento(Jogo *j,
                               const char *titulo,
                               const char *texto,
                               ConhecimentoDestino destino) {
    strncpy(j->conhecimento_titulo, titulo, sizeof(j->conhecimento_titulo) - 1);
    strncpy(j->conhecimento_texto,  texto,  sizeof(j->conhecimento_texto)  - 1);
    j->conhecimento_destino = destino;
    JogoMudarEstado(j, STATE_CONHECIMENTO);
}

/* ============================================================
   JogoUpdate - maquina de estados principal
   ============================================================ */
void JogoUpdate(Jogo *j) {
    j->delta = GetFrameTime();

    /* Decrementa feedback de save */
    if (j->timer_save_feedback > 0.0f)
        j->timer_save_feedback -= j->delta;

    switch (j->estado_atual) {

    /* ==================== MENU ==================== */
    case STATE_MENU: {
        int tem_save = SaveExiste();
        int n_opcoes = tem_save ? 4 : 3;

        if (IsKeyPressed(KEY_DOWN)) NavDown(&j->menu_opcao, n_opcoes);
        if (IsKeyPressed(KEY_UP))   NavUp  (&j->menu_opcao, n_opcoes);

        if (IsKeyPressed(KEY_ENTER)) {
            if (tem_save) {
                /* 0=Novo 1=Continuar 2=Creditos 3=Sair */
                switch (j->menu_opcao) {
                    case 0:
                        PlayerInit(&j->jogador);
                        EnemyInitTodos(j->adversarios);
                        j->academia_opcao = 0;
                        JogoMudarEstado(j, STATE_ACADEMIA);
                        break;
                    case 1:
                        if (LoadJogo(&j->jogador)) {
                            EnemyInitTodos(j->adversarios);
                            j->academia_opcao = 0;
                            JogoMudarEstado(j, STATE_ACADEMIA);
                        }
                        break;
                    case 2: JogoMudarEstado(j, STATE_CREDITOS); break;
                    case 3: CloseWindow(); break;
                }
            } else {
                /* 0=Novo 1=Creditos 2=Sair */
                switch (j->menu_opcao) {
                    case 0:
                        PlayerInit(&j->jogador);
                        EnemyInitTodos(j->adversarios);
                        j->academia_opcao = 0;
                        JogoMudarEstado(j, STATE_ACADEMIA);
                        break;
                    case 1: JogoMudarEstado(j, STATE_CREDITOS); break;
                    case 2: CloseWindow(); break;
                }
            }
        }
        break;
    }

    /* ==================== ACADEMIA ==================== */
    case STATE_ACADEMIA: {
        /* Opcoes fixas: 7 itens */
        if (IsKeyPressed(KEY_DOWN)) NavDown(&j->academia_opcao, 7);
        if (IsKeyPressed(KEY_UP))   NavUp  (&j->academia_opcao, 7);

        if (IsKeyPressed(KEY_ENTER)) {
            switch (j->academia_opcao) {
                case 0: /* Treinar Velocidade */
                    TrainoInit(&g_treino, TREINO_VELOCIDADE);
                    g_treino_recompensa_aplicada = 0;
                    JogoMudarEstado(j, STATE_TREINO_VELOCIDADE);
                    break;
                case 1: /* Treinar Potencia */
                    TrainoInit(&g_treino, TREINO_POTENCIA);
                    g_treino_recompensa_aplicada = 0;
                    JogoMudarEstado(j, STATE_TREINO_POTENCIA);
                    break;
                case 2: /* Treinar Resistencia */
                    TrainoInit(&g_treino, TREINO_RESISTENCIA);
                    g_treino_recompensa_aplicada = 0;
                    JogoMudarEstado(j, STATE_TREINO_RESISTENCIA);
                    break;
                case 3: /* Lutar */
                    if (j->jogador.fase_atual < MAX_ADVERSARIOS) {
                        FightInit(j, j->jogador.fase_atual);
                        AbrirConhecimento(j,
                            j->adversarios[j->jogador.fase_atual].nome,
                            j->adversarios[j->jogador.fase_atual].dica_pre_luta,
                            CONHECIMENTO_VOLTA_LUTA);
                    }
                    break;
                case 4: /* Estatisticas */
                    JogoMudarEstado(j, STATE_ESTATISTICAS);
                    break;
                case 5: /* Salvar */
                    SaveJogo(&j->jogador);
                    j->timer_save_feedback = 2.0f;
                    break;
                case 6: /* Menu */
                    j->menu_opcao = 0;
                    JogoMudarEstado(j, STATE_MENU);
                    break;
            }
        }
        break;
    }

    /* ==================== TREINOS ==================== */
    case STATE_TREINO_VELOCIDADE:
    case STATE_TREINO_POTENCIA:
    case STATE_TREINO_RESISTENCIA: {
        /* Enquanto o minigame esta ativo, roda normalmente */
        if (g_treino.ativo) {
            TreinoUpdate(j, &g_treino);
        } else {
            /* Minigame terminou - aplica recompensa UMA VEZ */
            if (!g_treino_recompensa_aplicada) {
                TreinoAplicarRecompensa(j, &g_treino);
                g_treino_recompensa_aplicada = 1;
            }
            /* Aguarda ENTER para ir para o conhecimento */
            if (IsKeyPressed(KEY_ENTER)) {
                AbrirConhecimento(j,
                    "Sabia que...",
                    TreinoGetConhecimento(g_treino.tipo),
                    CONHECIMENTO_VOLTA_ACADEMIA);
            }
        }
        break;
    }

    /* ==================== CONHECIMENTO ==================== */
    case STATE_CONHECIMENTO: {
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            if (j->conhecimento_destino == CONHECIMENTO_VOLTA_LUTA) {
                JogoMudarEstado(j, STATE_LUTA);
            } else {
                /* Volta para academia e reseta o cursor */
                j->academia_opcao = 0;
                JogoMudarEstado(j, STATE_ACADEMIA);
            }
        }
        break;
    }

    /* ==================== LUTA ==================== */
    case STATE_LUTA: {
        if (!j->luta.luta_encerrada) {
            FightUpdate(j);
        } else {
            if (IsKeyPressed(KEY_ENTER)) {
                FightFinalizar(j);
                if (j->luta.jogador_venceu) {
                    if (j->jogador.fase_atual >= MAX_ADVERSARIOS) {
                        JogoMudarEstado(j, STATE_VITORIA);
                    } else {
                        j->academia_opcao = 0;
                        JogoMudarEstado(j, STATE_ACADEMIA);
                    }
                } else {
                    JogoMudarEstado(j, STATE_GAMEOVER);
                }
            }
        }
        break;
    }

    /* ==================== ESTATISTICAS ==================== */
    case STATE_ESTATISTICAS: {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) {
            JogoMudarEstado(j, STATE_ACADEMIA);
        }
        if (j->jogador.pontos_atributo > 0) {
            if (IsKeyPressed(KEY_ONE))   PlayerDistribuirPonto(&j->jogador, 0);
            if (IsKeyPressed(KEY_TWO))   PlayerDistribuirPonto(&j->jogador, 1);
            if (IsKeyPressed(KEY_THREE)) PlayerDistribuirPonto(&j->jogador, 2);
            if (IsKeyPressed(KEY_FOUR))  PlayerDistribuirPonto(&j->jogador, 3);
            if (IsKeyPressed(KEY_FIVE))  PlayerDistribuirPonto(&j->jogador, 4);
        }
        break;
    }

    /* ==================== GAME OVER ==================== */
    case STATE_GAMEOVER: {
        if (IsKeyPressed(KEY_ENTER)) {
            j->academia_opcao = 0;
            JogoMudarEstado(j, STATE_ACADEMIA);
        }
        if (IsKeyPressed(KEY_M)) {
            j->menu_opcao = 0;
            JogoMudarEstado(j, STATE_MENU);
        }
        break;
    }

    /* ==================== VITORIA ==================== */
    case STATE_VITORIA: {
        if (IsKeyPressed(KEY_ENTER)) {
            j->menu_opcao = 0;
            JogoMudarEstado(j, STATE_MENU);
        }
        break;
    }

    /* ==================== CREDITOS ==================== */
    case STATE_CREDITOS: {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_BACKSPACE)) {
            JogoMudarEstado(j, STATE_MENU);
        }
        break;
    }

    default: break;
    }
}

/* ============================================================
   Helpers de desenho
   ============================================================ */
static void DrawTitulo(const char *texto, int y, Color cor) {
    int largura = MeasureText(texto, 32);
    DrawText(texto, SCREEN_W/2 - largura/2, y, 32, cor);
}

static void DrawItemMenu(const char *texto, int x, int y, int tamanho,
                         int selecionado) {
    if (selecionado) {
        DrawText(">", x - 24, y, tamanho, GOLD);
        DrawText(texto, x, y, tamanho, GOLD);
    } else {
        DrawText(texto, x, y, tamanho, WHITE);
    }
}

/* ============================================================
   JogoDraw
   ============================================================ */
void JogoDraw(Jogo *j) {
    BeginDrawing();

    switch (j->estado_atual) {

    /* ==================== MENU ==================== */
    case STATE_MENU: {
        ClearBackground((Color){10, 10, 20, 255});
        DrawTitulo("BOXING EVOLUTION", 70, GOLD);
        DrawText("Um jogo educativo sobre boxe",
                 SCREEN_W/2 - MeasureText("Um jogo educativo sobre boxe", 18)/2,
                 120, 18, LIGHTGRAY);
        DrawLine(SCREEN_W/2 - 200, 150, SCREEN_W/2 + 200, 150, DARKGRAY);
        int tem_save = SaveExiste();
        const char *opcoes_save[]    = {"Novo Jogo", "Continuar", "Creditos", "Sair"};
        const char *opcoes_nosave[]  = {"Novo Jogo", "Creditos",  "Sair"};
        int n = tem_save ? 4 : 3;
        const char **opcoes = tem_save ? opcoes_save : opcoes_nosave;
        int cx = SCREEN_W/2 - 60;
        for (int i = 0; i < n; i++) {
            DrawItemMenu(opcoes[i], cx, 190 + i * 48, 22, j->menu_opcao == i);
        }

        DrawText("CIMA/BAIXO para navegar | ENTER para selecionar",
                 SCREEN_W/2 - MeasureText("CIMA/BAIXO para navegar | ENTER para selecionar", 13)/2,
                 SCREEN_H - 36, 13, DARKGRAY);
        break;
    }

    /* ==================== ACADEMIA ==================== */
    case STATE_ACADEMIA: {
        /* Fundo com degrad-- simulado */
        ClearBackground((Color){10, 14, 28, 255});
        DrawRectangle(0, 0, SCREEN_W, 300, (Color){12, 18, 36, 255});

        /* Desenho decorativo: boxeador no fundo (silhueta) */
        DrawCircle(880, 320, 55, (Color){20, 30, 55, 255});   /* cabeca */
        DrawRectangle(840, 370, 80, 100, (Color){20, 30, 55, 255}); /* corpo */
        DrawCircle(820, 390, 18, (Color){18, 28, 50, 255});   /* luva */
        DrawCircle(900, 385, 18, (Color){18, 28, 50, 255});   /* luva */
        DrawRectangle(848, 468, 20, 50, (Color){20, 30, 55, 255}); /* perna */
        DrawRectangle(872, 468, 20, 50, (Color){20, 30, 55, 255}); /* perna */

        /* Titulo */
        DrawRectangle(0, 0, SCREEN_W, 72, (Color){0,0,0,160});
        DrawText("ACADEMIA DE BOXE", 20, 14, 30, GOLD);

        char buf[128];
        /* Barra de progressao da fase */
        DrawText("PROGRESSO:", SCREEN_W - 320, 10, 12, LIGHTGRAY);
        for (int i = 0; i < MAX_ADVERSARIOS; i++) {
            Color cp = (i < j->jogador.fase_atual) ? GOLD :
                       (i == j->jogador.fase_atual) ? (Color){200,200,0,255} :
                       (Color){50,50,80,255};
            DrawRectangle(SCREEN_W - 310 + i * 56, 26, 48, 16, cp);
            DrawRectangleLines(SCREEN_W - 310 + i * 56, 26, 48, 16, WHITE);
            const char *fnames[] = {"F1","F2","F3","F4","F5"};
            DrawText(fnames[i], SCREEN_W - 298 + i * 56, 30, 11,
                     i < j->jogador.fase_atual ? BLACK : WHITE);
        }

        sprintf(buf, "Nivel %d", j->jogador.nivel);
        DrawText(buf, SCREEN_W - 80, 10, 13, LIGHTGRAY);

        /* Status do jogador */
        PlayerDrawStatus(&j->jogador, 16, 80);

        if (j->jogador.pontos_atributo > 0) {
            DrawRectangle(16, 136, 380, 20, (Color){80, 60, 0, 200});
            DrawText("[ ! ] Pontos disponiveis - va em Estatisticas (opcao 5)",
                     20, 139, 12, YELLOW);
        }

        if (j->timer_save_feedback > 0.0f) {
            DrawText("Salvo!", SCREEN_W - 70, 80, 14, GREEN);
        }

        DrawLine(0, 158, SCREEN_W, 158, (Color){40, 55, 90, 255});

        /* Painel esquerdo: menu */
        const char *opcoes[] = {
            "Treinar Velocidade",
            "Treinar Potencia",
            "Treinar Resistencia",
            ">> LUTAR <<",
            "Estatisticas",
            "Salvar Jogo",
            "Voltar ao Menu"
        };
        const char *desc[] = {
            "+Velocidade  |  Tempo de reacao",
            "+Potencia    |  Barra de forca",
            "+Resistencia |  Sequencia de teclas",
            "Enfrentar adversario da fase atual",
            "Atributos, golpes e level-up",
            "Gravar progresso em arquivo",
            "Retornar ao menu principal"
        };
        /* Icones de texto para cada opcao */
        const char *icons[] = {"[V]","[P]","[R]","[!]","[=]","[S]","[<]"};
        Color icon_cols[] = {SKYBLUE, ORANGE, GREEN, RED, LIGHTGRAY, GOLD, GRAY};

        for (int i = 0; i < 7; i++) {
            int y = 164 + i * 40;
            int sel = (i == j->academia_opcao);
            /* Fundo do item */
            if (sel) {
                DrawRectangle(0, y, 620, 38, (Color){30, 50, 110, 255});
                DrawRectangle(0, y, 4, 38, GOLD);
            } else {
                DrawRectangle(0, y, 620, 38, (Color){15, 22, 44, 200});
                DrawRectangle(0, y, 2, 38, (Color){40,55,90,255});
            }
            /* Icone */
            DrawText(icons[i], 10, y + 11, 14, icon_cols[i]);
            /* Texto principal */
            Color ct = sel ? GOLD : (i == 3 ? (Color){255,150,100,255} : WHITE);
            DrawText(opcoes[i], 46, y + 11, 15, ct);
            /* Descricao */
            DrawText(desc[i], 310, y + 13, 12,
                     sel ? (Color){200,200,200,255} : (Color){70,85,110,255});
        }

        /* Painel direito: info do proximo adversario */
        DrawRectangle(630, 158, 394, 280, (Color){12, 18, 38, 230});
        DrawRectangleLines(630, 158, 394, 280, (Color){40,55,90,255});
        DrawText("PROXIMO ADVERSARIO", 640, 165, 13, GOLD);
        DrawLine(630, 182, 1024, 182, (Color){40,55,90,255});

        if (j->jogador.fase_atual < MAX_ADVERSARIOS) {
            Adversario *prox = &j->adversarios[j->jogador.fase_atual];
            /* Boneco do adversario em miniatura */
            int cx = 780, cy = 290;
            DrawCircle(cx, cy - 50, 22, prox->cor);
            DrawRectangle(cx - 18, cy - 28, 36, 40, prox->cor);
            DrawCircle(cx - 22, cy - 16, 12, (Color){180,50,50,255});
            DrawCircle(cx + 22, cy - 16, 12, (Color){180,50,50,255});

            int nw2 = MeasureText(prox->nome, 16);
            DrawText(prox->nome, cx - nw2/2, cy + 18, 16, WHITE);
            int sw = MeasureText(prox->estilo, 12);
            DrawText(prox->estilo, cx - sw/2, cy + 38, 12, LIGHTGRAY);

            /* Atributos do adversario como barras */
            const char *atr_names[] = {"POT","VEL","TEC","RES"};
            int atr_vals[] = {prox->atrib.potencia, prox->atrib.velocidade,
                              prox->atrib.tecnica,  prox->atrib.resistencia};
            for (int i = 0; i < 4; i++) {
                int ax = 640, ay = 330 + i * 22;
                DrawText(atr_names[i], ax, ay, 11, LIGHTGRAY);
                DrawRectangle(ax + 35, ay, 100, 12, DARKGRAY);
                int bw = (int)(100.0f * (float)atr_vals[i] / 20.0f);
                if (bw > 100) bw = 100;
                Color bc2 = (atr_vals[i] > 12) ? RED :
                            (atr_vals[i] > 8)  ? ORANGE : GREEN;
                DrawRectangle(ax + 35, ay, bw, 12, bc2);
                DrawRectangleLines(ax + 35, ay, 100, 12, GRAY);
                sprintf(buf, "%d", atr_vals[i]);
                DrawText(buf, ax + 140, ay, 11, WHITE);
            }
        } else {
            DrawText("Todos os adversarios", 660, 220, 14, GOLD);
            DrawText("foram derrotados!", 660, 245, 14, GOLD);
            DrawText("Veja Estatisticas", 660, 280, 13, LIGHTGRAY);
        }

        /* Rodape */
        DrawRectangle(0, SCREEN_H - 28, SCREEN_W, 28, (Color){0,0,0,160});
        DrawText("[CIMA/BAIXO] Navegar   [ENTER] Selecionar",
                 SCREEN_W/2 - MeasureText("[CIMA/BAIXO] Navegar   [ENTER] Selecionar", 12)/2,
                 SCREEN_H - 20, 12, (Color){80,90,110,255});
        break;
    }

    /* ==================== TREINOS ==================== */
    case STATE_TREINO_VELOCIDADE:
    case STATE_TREINO_POTENCIA:
    case STATE_TREINO_RESISTENCIA:
        TreinoDraw(j, &g_treino);
        break;

    /* ==================== LUTA ==================== */
    case STATE_LUTA:
        FightDraw(j);
        break;

    /* ==================== CONHECIMENTO ==================== */
    case STATE_CONHECIMENTO:
        KnowledgeDraw(j);
        break;

    /* ==================== ESTATISTICAS ==================== */
    case STATE_ESTATISTICAS: {
        ClearBackground((Color){10, 15, 30, 255});

        DrawRectangle(0, 0, SCREEN_W, 58, (Color){18, 26, 52, 255});
        DrawTitulo("ESTATISTICAS", 12, GOLD);
        DrawLine(0, 58, SCREEN_W, 58, DARKGRAY);

        Jogador *p = &j->jogador;
        char buf[128];

        /* -- Atributos (esquerda) -- */
        DrawText("ATRIBUTOS", 50, 74, 15, GOLD);
        const char *nomes[] = {"Vida Max","Tecnica","Velocidade","Resistencia","Potencia"};
        int vals[] = {p->atrib.vida_max, p->atrib.tecnica,
                      p->atrib.velocidade, p->atrib.resistencia, p->atrib.potencia};
        for (int i = 0; i < 5; i++) {
            sprintf(buf, "[%d] %s: %d", i+1, nomes[i], vals[i]);
            Color c = (p->pontos_atributo > 0) ? YELLOW : WHITE;
            DrawText(buf, 50, 98 + i * 28, 14, c);
        }

        if (p->pontos_atributo > 0) {
            sprintf(buf, "Pontos disponiveis: %d  (pressione 1-5 para distribuir)",
                    p->pontos_atributo);
            DrawText(buf, 50, 244, 13, GOLD);
        }

        /* -- Golpes (meio) -- */
        DrawText("GOLPES", 50, 284, 15, GOLD);
        for (int i = 0; i < MAX_GOLPES; i++) {
            Golpe *g = &p->golpes[i];
            int y = 308 + i * 44;
            if (g->desbloqueado) {
                DrawRectangle(50, y, SCREEN_W/2 - 70, 38, (Color){25,40,25,220});
                DrawRectangleLines(50, y, SCREEN_W/2 - 70, 38, (Color){0,140,0,200});
                sprintf(buf, "[%s] %s", g->tecla_str, g->nome);
                DrawText(buf, 60, y + 4, 14, GREEN);
                DrawText(g->descricao, 60, y + 22, 11, LIGHTGRAY);
            } else {
                DrawRectangle(50, y, SCREEN_W/2 - 70, 38, (Color){35,25,25,200});
                DrawRectangleLines(50, y, SCREEN_W/2 - 70, 38, (Color){80,40,40,200});
                sprintf(buf, "[%s] %s  - requer nivel %d",
                        g->tecla_str, g->nome, g->nivel_necessario);
                DrawText(buf, 60, y + 12, 13, (Color){120,80,80,255});
            }
        }

        /* -- Progresso (direita) -- */
        DrawText("PROGRESSO", SCREEN_W*2/3, 74, 15, GOLD);
        int rx = SCREEN_W*2/3;
        sprintf(buf, "Nivel:        %d", p->nivel);
        DrawText(buf, rx, 100, 14, WHITE);
        sprintf(buf, "XP:           %d / %d", p->xp, p->xp_proximo_nivel);
        DrawText(buf, rx, 124, 14, WHITE);

        /* Barra de XP */
        DrawRectangle(rx, 142, 200, 10, DARKGRAY);
        int xp_w = (int)(200.0f * (float)p->xp / (float)p->xp_proximo_nivel);
        DrawRectangle(rx, 142, xp_w, 10, BLUE);
        DrawRectangleLines(rx, 142, 200, 10, GRAY);

        sprintf(buf, "Fase:         %d / %d", p->fase_atual + 1, MAX_FASES);
        DrawText(buf, rx, 160, 14, WHITE);
        sprintf(buf, "Vitorias:     %d", p->lutas_vencidas);
        DrawText(buf, rx, 184, 14, GREEN);
        sprintf(buf, "Derrotas:     %d", p->lutas_perdidas);
        DrawText(buf, rx, 208, 14, RED);

        DrawText("[ESC] ou [BACKSPACE] para voltar",
                 SCREEN_W/2 - MeasureText("[ESC] ou [BACKSPACE] para voltar", 13)/2,
                 SCREEN_H - 30, 13, DARKGRAY);
        break;
    }

    /* ==================== GAME OVER ==================== */
    case STATE_GAMEOVER: {
        ClearBackground((Color){18, 5, 5, 255});

        DrawTitulo("NOCAUTE!", SCREEN_H/2 - 110, RED);
        DrawText("Voce foi derrubado. Treine mais e volte mais forte!",
                 SCREEN_W/2 - MeasureText("Voce foi derrubado. Treine mais e volte mais forte!", 16)/2,
                 SCREEN_H/2 - 56, 16, WHITE);

        /* Dica educativa na derrota */
        DrawRectangle(60, SCREEN_H/2 - 10, SCREEN_W - 120, 60, (Color){30,10,10,220});
        DrawRectangleLines(60, SCREEN_H/2 - 10, SCREEN_W - 120, 60, (Color){100,30,30,255});
        DrawText(KnowledgeGetCuriosidade(), 74, SCREEN_H/2 + 4, 13, LIGHTGRAY);

        DrawText("[ENTER] Voltar a Academia    [M] Menu Principal",
                 SCREEN_W/2 - MeasureText("[ENTER] Voltar a Academia    [M] Menu Principal", 15)/2,
                 SCREEN_H - 52, 15, DARKGRAY);
        break;
    }

    /* ==================== VITORIA ==================== */
    case STATE_VITORIA: {
        ClearBackground((Color){8, 24, 8, 255});

        DrawTitulo("VOCE E O CAMPEAO!", 60, GOLD);
        DrawText("Parabens! Voce conquistou o titulo e completou o Boxing Evolution!",
                 SCREEN_W/2 - MeasureText("Parabens! Voce conquistou o titulo e completou o Boxing Evolution!", 15)/2,
                 118, 15, WHITE);

        DrawLine(60, 144, SCREEN_W - 60, 144, DARKGRAY);
        DrawText("O que voce aprendeu:", 60, 158, 16, GOLD);

        const char *aprendizados[] = {
            "O Jab e o golpe mais importante: controla a distancia.",
            "O Cross usa a rotacao do quadril para gerar potencia.",
            "O Hook e eficaz em curta distancia e dificiL de defender.",
            "O Uppercut e ideal quando o adversario abre a guarda.",
            "Resistencia permite manter ritmo alto por mais tempo.",
            "Tecnica reduz o dano recebido e melhora o acerto."
        };
        for (int i = 0; i < 6; i++) {
            DrawText(aprendizados[i], 80, 186 + i * 28, 14, LIGHTGRAY);
        }

        char buf[64];
        Jogador *p = &j->jogador;
        sprintf(buf, "Estatisticas finais: Nivel %d | %d vitorias | %d derrotas",
                p->nivel, p->lutas_vencidas, p->lutas_perdidas);
        DrawText(buf,
                 SCREEN_W/2 - MeasureText(buf, 14)/2,
                 400, 14, LIGHTGRAY);

        DrawText("[ENTER] Voltar ao Menu",
                 SCREEN_W/2 - MeasureText("[ENTER] Voltar ao Menu", 18)/2,
                 SCREEN_H - 60, 18, GOLD);
        break;
    }

    /* ==================== CREDITOS ==================== */
    case STATE_CREDITOS: {
        ClearBackground((Color){10, 10, 20, 255});

        DrawTitulo("CREDITOS", 50, GOLD);
        DrawLine(60, 96, SCREEN_W - 60, 96, DARKGRAY);

        DrawText("Projeto PIF - Programacao Imperativa e Funcional", 60, 112, 16, WHITE);
        DrawText("Desenvolvido em linguagem C com a biblioteca Raylib.", 60, 136, 14, LIGHTGRAY);

        DrawText("Conceitos de C demonstrados neste projeto:", 60, 174, 15, GOLD);
        const char *conceitos[] = {
            "Structs (Jogador, Adversario, EstadoLuta, Golpe...)",
            "Enums (GameState, TipoGolpe, TipoTreino)",
            "Vetores de structs (golpes[], adversarios[])",
            "Funcoes e modularizacao em multiplos arquivos .c/.h",
            "Condicionais e estruturas de repeticao (for, while)",
            "Ponteiros - passagem de structs por referencia",
            "Leitura e escrita em arquivo binario (fwrite/fread)",
            "Geracao de numeros aleatorios (GetRandomValue)",
            "Maquina de estados (switch/case com GameState)"
        };
        for (int i = 0; i < 9; i++) {
            DrawText(conceitos[i], 80, 200 + i * 26, 13, LIGHTGRAY);
        }

        DrawText("[ENTER] ou [ESC] para voltar",
                 SCREEN_W/2 - MeasureText("[ENTER] ou [ESC] para voltar", 14)/2,
                 SCREEN_H - 36, 14, DARKGRAY);
        break;
    }

    default: break;
    }

    EndDrawing();
}

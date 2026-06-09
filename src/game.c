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
        /* academia_sub=0: tela das portas
           academia_sub=1: submenu treino
           academia_sub=2: card do adversario (confirmar luta) */

        if (j->academia_sub == 0) {
            /* Navegacao lateral pelas 3 portas: 0=Treinar 1=Lutar 2=Stats */
            if (IsKeyPressed(KEY_LEFT))  NavUp  (&j->academia_opcao, 3);
            if (IsKeyPressed(KEY_RIGHT)) NavDown(&j->academia_opcao, 3);
            if (IsKeyPressed(KEY_ESCAPE)) {
                j->menu_opcao = 0;
                JogoMudarEstado(j, STATE_MENU);
            }
            if (IsKeyPressed(KEY_S)) {
                SaveJogo(&j->jogador);
                j->timer_save_feedback = 2.0f;
            }
            if (IsKeyPressed(KEY_ENTER)) {
                if (j->academia_opcao == 0) {
                    /* Porta TREINAR: abre submenu */
                    j->academia_sub = 1;
                    j->treino_opcao = 0;
                } else if (j->academia_opcao == 1) {
                    /* Porta LUTAR: mostra card do adversario */
                    j->academia_sub = 2;
                } else if (j->academia_opcao == 2) {
                    /* Porta STATS */
                    JogoMudarEstado(j, STATE_ESTATISTICAS);
                }
            }
        } else if (j->academia_sub == 1) {
            /* Submenu de treino */
            if (IsKeyPressed(KEY_LEFT))  NavUp  (&j->treino_opcao, 3);
            if (IsKeyPressed(KEY_RIGHT)) NavDown(&j->treino_opcao, 3);
            if (IsKeyPressed(KEY_ESCAPE)) j->academia_sub = 0;
            if (IsKeyPressed(KEY_ENTER)) {
                int tipos[3] = {TREINO_VELOCIDADE, TREINO_POTENCIA, TREINO_RESISTENCIA};
                int estados[3] = {STATE_TREINO_VELOCIDADE, STATE_TREINO_POTENCIA, STATE_TREINO_RESISTENCIA};
                TrainoInit(&g_treino, tipos[j->treino_opcao]);
                g_treino_recompensa_aplicada = 0;
                j->academia_sub = 0;
                JogoMudarEstado(j, estados[j->treino_opcao]);
            }
        } else if (j->academia_sub == 2) {
            /* Card de confirmacao de luta */
            if (IsKeyPressed(KEY_ESCAPE)) j->academia_sub = 0;
            if (IsKeyPressed(KEY_ENTER)) {
                if (j->jogador.fase_atual < MAX_ADVERSARIOS) {
                    FightInit(j, j->jogador.fase_atual);
                    AbrirConhecimento(j,
                        j->adversarios[j->jogador.fase_atual].nome,
                        j->adversarios[j->jogador.fase_atual].dica_pre_luta,
                        CONHECIMENTO_VOLTA_LUTA);
                    j->academia_sub = 0;
                }
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
        /* ---- FUNDO: Academia de boxe pixel art ---- */
        /* Chao de madeira */
        for (int i=0;i<16;i++) {
            Color mc = (i%2==0)?(Color){100,65,30,255}:(Color){90,58,26,255};
            DrawRectangle(i*64,380,64,220,mc);
            DrawRectangle(i*64,380,1,220,(Color){70,45,18,255});
        }
        /* Parede de tijolo */
        for (int row=0;row<12;row++) {
            int off=(row%2)*48;
            for (int col=-1;col<18;col++) {
                int bx2=col*96+off, by2=row*32;
                Color tc=(row+col)%2==0?(Color){160,70,50,255}:(Color){140,60,42,255};
                DrawRectangle(bx2,by2,94,30,tc);
                DrawRectangle(bx2,by2,94,1,(Color){80,40,25,255});
                DrawRectangle(bx2,by2,1,30,(Color){80,40,25,255});
            }
        }
        /* Teto */
        DrawRectangle(0,370,1024,16,(Color){60,40,20,255});

        /* Janela esquerda */
        DrawRectangle(30,60,140,160,(Color){100,150,200,200});
        DrawRectangle(30,60,140,2,BLACK); DrawRectangle(30,60,2,160,BLACK);
        DrawRectangle(168,60,2,160,BLACK); DrawRectangle(30,218,140,2,BLACK);
        DrawRectangle(100,60,2,160,(Color){80,40,20,255});
        DrawRectangle(30,140,140,2,(Color){80,40,20,255});
        /* Grade janela */
        DrawRectangle(26,56,148,168,(Color){80,40,20,255});
        DrawRectangle(30,60,140,160,(Color){100,160,220,180});

        /* Janela direita */
        DrawRectangle(854,60,140,160,(Color){100,150,200,200});
        DrawRectangle(854,56,148,4,(Color){80,40,20,255});
        DrawRectangle(850,56,4,168,(Color){80,40,20,255});
        DrawRectangle(998,56,4,168,(Color){80,40,20,255});
        DrawRectangle(854,220,148,4,(Color){80,40,20,255});
        DrawRectangle(928,60,2,160,(Color){80,40,20,255});
        DrawRectangle(854,140,140,2,(Color){80,40,20,255});

        /* Saco de pancada esquerda */
        DrawRectangle(160,130,50,120,(Color){160,80,30,255});
        DrawRectangle(160,130,50,4,(Color){100,50,20,255});
        DrawRectangle(160,248,50,6,(Color){100,50,20,255});
        DrawRectangle(158,128,2,122,(Color){100,50,20,255});
        DrawRectangle(208,128,2,122,(Color){100,50,20,255});
        DrawRectangle(180,80,10,52,(Color){80,40,20,255});
        DrawRectangle(176,76,18,8,(Color){60,35,15,255});
        /* Costuras do saco */
        DrawLine(185,130,185,248,(Color){120,60,25,255});
        DrawLine(160,190,210,190,(Color){120,60,25,255});

        /* Trof--us na prateleira */
        DrawRectangle(400,50,220,10,(Color){100,65,30,255});
        DrawRectangle(400,48,220,4,(Color){120,80,35,255});
        /* Trofeu 1 */
        DrawRectangle(420,20,20,30,(Color){220,180,20,255});
        DrawRectangle(415,48,30,6,(Color){180,140,15,255});
        DrawRectangle(425,15,10,8,(Color){220,180,20,255});
        DrawCircle(430,14,7,(Color){220,180,20,255});
        DrawCircle(430,14,4,(Color){255,220,50,255});
        /* Trofeu 2 (maior) */
        DrawRectangle(480,10,24,38,(Color){220,180,20,255});
        DrawRectangle(474,46,36,8,(Color){180,140,15,255});
        DrawRectangle(486,4,12,10,(Color){220,180,20,255});
        DrawCircle(492,3,9,(Color){220,180,20,255});
        DrawCircle(492,3,5,(Color){255,220,50,255});
        /* Trofeu 3 */
        DrawRectangle(540,22,18,26,(Color){180,180,180,255});
        DrawRectangle(536,46,26,6,(Color){140,140,140,255});
        DrawRectangle(544,18,10,6,(Color){180,180,180,255});
        DrawCircle(549,17,6,(Color){180,180,180,255});
        /* Fotos na parede */
        DrawRectangle(660,40,80,60,(Color){240,230,200,255});
        DrawRectangle(660,40,80,2,BLACK); DrawRectangle(660,40,2,60,BLACK);
        DrawRectangle(738,40,2,60,BLACK); DrawRectangle(660,98,80,2,BLACK);
        DrawRectangle(666,46,68,48,(Color){100,80,60,200});
        DrawRectangle(750,30,80,80,(Color){240,230,200,255});
        DrawRectangle(750,30,80,2,BLACK); DrawRectangle(750,30,2,80,BLACK);
        DrawRectangle(828,30,2,80,BLACK); DrawRectangle(750,108,80,2,BLACK);
        DrawRectangle(756,36,68,68,(Color){60,80,100,200});

        /* Luz no teto */
        DrawCircle(512,50,20,(Color){255,255,200,255});
        DrawRectangle(502,0,20,52,(Color){200,180,60,255});
        DrawCircle(512,380,200,(Color){255,255,220,15});

        /* ---- HUD TOPO ---- */
        DrawRectangle(0,0,1024,36,(Color){0,0,0,180});
        char buf2[128];
        sprintf(buf2,"Nivel %d  |  Fase %d/%d  |  Vida: %d/%d",
                j->jogador.nivel, j->jogador.fase_atual+1, MAX_ADVERSARIOS,
                j->jogador.atrib.vida, j->jogador.atrib.vida_max);
        DrawText(buf2, 512-MeasureText(buf2,13)/2, 10, 13, WHITE);
        if (j->timer_save_feedback > 0.0f)
            DrawText("[S] Salvo!", 920, 10, 12, GREEN);
        else
            DrawText("[S] Salvar  [ESC] Menu", 820, 10, 11, DARKGRAY);

        /* Pontos dispon--veis */
        if (j->jogador.pontos_atributo > 0) {
            DrawRectangle(0,34,1024,20,(Color){80,60,0,220});
            sprintf(buf2,"! %d ponto(s) de atributo disponivel! Entre em Estatisticas para distribuir.",
                    j->jogador.pontos_atributo);
            DrawText(buf2, 512-MeasureText(buf2,12)/2, 38, 12, GOLD);
        }

        /* ---- PORTAS ---- */
        /* 3 portas: Treinar(0) Lutar(1) Stats(2) */
        const char *porta_nomes[3] = {"TREINAR","LUTAR","ESTATISTICAS"};
        const char *porta_subs[3]  = {"Velocidade/Potencia/Resistencia",
                                       "Proximo adversario","Atributos e progresso"};
        Color porta_cores[3] = {
            (Color){40,100,180,255},
            (Color){180,40,40,255},
            (Color){40,140,60,255}
        };

        int sub = j->academia_sub;

        /* Desenha fundo escurecido se em submenu */
        if (sub > 0) {
            DrawRectangle(0,36,1024,564,(Color){0,0,0,160});
        }

        for (int i=0;i<3;i++) {
            int px3 = 160 + i*260;
            int py3 = 160;
            int pw  = 200, ph = 200;
            int sel = (j->academia_opcao == i);

            /* Zoom na porta selecionada quando nao esta em submenu */
            if (sel && sub == 0) {
                px3 -= 12; py3 -= 12; pw += 24; ph += 24;
            }

            /* Sombra da porta */
            DrawRectangle(px3+6,py3+6,pw,ph,(Color){0,0,0,120});

            /* Moldura */
            DrawRectangle(px3-6,py3-6,pw+12,ph+12,(Color){80,50,20,255});

            /* Porta em si */
            Color cp = porta_cores[i];
            if (!sel || sub > 0) cp = (Color){(unsigned char)(cp.r*6/10),(unsigned char)(cp.g*6/10),(unsigned char)(cp.b*6/10),255};
            DrawRectangle(px3,py3,pw,ph,cp);

            /* Detalhes da porta */
            DrawRectangle(px3+10,py3+10,pw-20,ph/2-10,(Color){0,0,0,40});
            DrawRectangle(px3+10,py3+ph/2+5,pw-20,ph/2-20,(Color){0,0,0,40});
            DrawRectangle(px3+pw-20,py3+ph/2-6,10,12,(Color){200,160,30,255});

            /* Contorno brilhante se selecionada */
            if (sel && sub == 0) {
                DrawRectangleLines(px3,py3,pw,ph,(Color){255,220,50,255});
                DrawRectangleLines(px3+2,py3+2,pw-4,ph-4,(Color){255,220,50,120});
            } else {
                DrawRectangleLines(px3,py3,pw,ph,(Color){60,40,15,255});
            }

            /* Nome da porta */
            int tw = MeasureText(porta_nomes[i], sel&&sub==0?18:15);
            int ts = sel&&sub==0?18:15;
            DrawText(porta_nomes[i], px3+pw/2-tw/2, py3+ph+12, ts,
                     sel&&sub==0?GOLD:LIGHTGRAY);
            int tw2 = MeasureText(porta_subs[i], 11);
            DrawText(porta_subs[i], px3+pw/2-tw2/2, py3+ph+34, 11,
                     sel&&sub==0?(Color){200,200,100,255}:DARKGRAY);
        }

        /* Setas de navegacao */
        if (sub == 0) {
            DrawText("<  ESQUERDA / DIREITA  >",
                     512-MeasureText("<  ESQUERDA / DIREITA  >",14)/2,
                     400, 14, (Color){120,120,120,200});
            DrawText("ENTER para entrar",
                     512-MeasureText("ENTER para entrar",13)/2,
                     420, 13, (Color){100,100,80,200});
        }

        /* ---- SUBMENU TREINO ---- */
        if (sub == 1) {
            DrawRectangle(262,150,500,300,(Color){15,20,40,240});
            DrawRectangleLines(262,150,500,300,GOLD);
            DrawText("ESCOLHA O TREINO",
                     512-MeasureText("ESCOLHA O TREINO",20)/2, 168, 20, GOLD);
            DrawLine(272,196,752,196,(Color){60,60,30,255});

            const char *tnomes[3] = {"VELOCIDADE","POTENCIA","RESISTENCIA"};
            const char *tdesc[3]  = {"Tempo de reacao - ganhe +Velocidade",
                                      "Barra de forca - ganhe +Potencia",
                                      "Sequencia de teclas - ganhe +Resistencia"};
            for (int i=0;i<3;i++) {
                int ty = 210+i*70;
                int tsel = (j->treino_opcao==i);
                Color tc = tsel?(Color){30,50,110,255}:(Color){20,30,60,200};
                DrawRectangle(282,ty,460,58,tc);
                DrawRectangleLines(282,ty,460,58,tsel?GOLD:(Color){40,50,80,255});
                if (tsel) DrawRectangle(282,ty,4,58,GOLD);
                DrawText(tnomes[i],300,ty+8,16,tsel?GOLD:WHITE);
                DrawText(tdesc[i],300,ty+30,12,tsel?(Color){200,200,150,255}:DARKGRAY);
            }
            DrawText("ESC = Voltar  |  ENTER = Confirmar",
                     512-MeasureText("ESC = Voltar  |  ENTER = Confirmar",13)/2,
                     428,13,DARKGRAY);
        }

        /* ---- CARD DO ADVERSARIO ---- */
        if (sub == 2 && j->jogador.fase_atual < MAX_ADVERSARIOS) {
            Adversario *prox = &j->adversarios[j->jogador.fase_atual];
            DrawRectangle(262,100,500,400,(Color){15,10,25,245});
            DrawRectangleLines(262,100,500,400,(Color){180,40,40,255});
            DrawRectangleLines(264,102,496,396,(Color){100,20,20,150});

            DrawText("PROXIMO ADVERSARIO",
                     512-MeasureText("PROXIMO ADVERSARIO",16)/2,114,16,(Color){200,100,100,255});
            DrawLine(272,136,752,136,(Color){100,30,30,255});

            DrawText(prox->nome,
                     512-MeasureText(prox->nome,28)/2,148,28,WHITE);
            DrawText(prox->estilo,
                     512-MeasureText(prox->estilo,16)/2,184,16,prox->cor);

            /* Atributos do adversario */
            DrawLine(272,210,752,210,(Color){60,40,40,255});
            DrawText("ATRIBUTOS:",290,220,13,(Color){180,140,140,255});
            char buf3[64];
            const char *anomes[5]={"Tecnica","Velocidade","Resistencia","Potencia","Vida"};
            int avals[5]={prox->atrib.tecnica,prox->atrib.velocidade,
                          prox->atrib.resistencia,prox->atrib.potencia,prox->atrib.vida_max};
            for (int i=0;i<5;i++) {
                int ax=290+i*92, ay=240;
                DrawText(anomes[i],ax,ay,10,(Color){160,130,130,255});
                DrawRectangle(ax,ay+14,80,8,DARKGRAY);
                int bw=(int)(80.0f*(float)avals[i]/20.0f);
                if(bw>80)bw=80;
                Color bc2=(i==3)?(Color){200,80,80,255}:
                          (i==1)?(Color){80,180,200,255}:
                          (i==2)?(Color){80,200,80,255}:(Color){180,180,80,255};
                DrawRectangle(ax,ay+14,bw,8,bc2);
                DrawRectangleLines(ax,ay+14,80,8,(Color){60,50,50,255});
                sprintf(buf3,"%d",avals[i]);
                DrawText(buf3,ax+82,ay+13,10,WHITE);
            }

            /* Dica da luta */
            DrawLine(272,280,752,280,(Color){60,40,40,255});
            DrawText("ESTRATEGIA:",290,292,13,(Color){180,140,140,255});
            DrawText(prox->dica_pre_luta,290,312,13,LIGHTGRAY);

            /* Nivel */
            sprintf(buf3,"Nivel %d",prox->nivel);
            DrawText(buf3,290,430,14,(Color){200,160,160,255});

            DrawRectangle(362,454,280,34,(Color){150,30,30,220});
            DrawRectangleLines(362,454,280,34,(Color){220,60,60,255});
            DrawText("ENTER = LUTAR!",
                     512-MeasureText("ENTER = LUTAR!",16)/2,462,16,WHITE);
            DrawText("ESC = Voltar",
                     512-MeasureText("ESC = Voltar",12)/2,494,12,DARKGRAY);
        }
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

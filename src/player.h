#ifndef PLAYER_H
#define PLAYER_H

#include "game.h"

/* Inicializa o jogador com valores padrao */
void PlayerInit(Jogador *p);

/* Concede XP e verifica level-up; retorna 1 se subiu de nivel */
int  PlayerGanharXP(Jogador *p, int quantidade);

/* Desbloqueia golpes conforme o nivel */
void PlayerAtualizarGolpes(Jogador *p);

/* Aplica dano ao jogador */
void PlayerReceberDano(Jogador *p, int dano, int adversario_potencia);

/* Distribui ponto de atributo (menu de level-up) */
void PlayerDistribuirPonto(Jogador *p, int atributo);
/*   atributo: 0=vida_max 1=tecnica 2=velocidade 3=resistencia 4=potencia */

/* Restaura vida entre lutas (baseado em resistencia) */
void PlayerRestaurarVida(Jogador *p);

/* Calcula dano de um golpe do jogador */
int  PlayerCalcularDano(Jogador *p, TipoGolpe golpe);

/* Atualiza timers de cooldown (chamado a cada frame) */
void PlayerAtualizarCooldowns(Jogador *p, float delta);

/* Verifica se o golpe pode ser usado agora */
int  PlayerPodeUsarGolpe(Jogador *p, TipoGolpe golpe);

/* Seta uso do golpe (inicia cooldown) */
void PlayerUsarGolpe(Jogador *p, TipoGolpe golpe);

/* Renderiza barra de status do jogador */
void PlayerDrawStatus(Jogador *p, int x, int y);

#endif /* PLAYER_H */

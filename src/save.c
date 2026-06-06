#include "save.h"
#include <stdio.h>

/* ============================================================
   SaveJogo
   Demonstra escrita em arquivo binario em C
   ============================================================ */
int SaveJogo(const Jogador *jogador) {
    FILE *f = fopen(SAVE_FILE, "wb");
    if (!f) return 0;

    /* Escreve a struct inteira de uma vez (arquivo binario) */
    size_t escritos = fwrite(jogador, sizeof(Jogador), 1, f);
    fclose(f);

    return (escritos == 1) ? 1 : 0;
}

/* ============================================================
   LoadJogo
   Demonstra leitura de arquivo binario em C
   ============================================================ */
int LoadJogo(Jogador *jogador) {
    FILE *f = fopen(SAVE_FILE, "rb");
    if (!f) return 0;

    size_t lidos = fread(jogador, sizeof(Jogador), 1, f);
    fclose(f);

    if (lidos != 1) return 0;

    /* Sanidade: verifica campos basicos */
    if (jogador->nivel < 1 || jogador->nivel > 50) return 0;
    if (jogador->atrib.vida_max < 1)               return 0;

    return 1;
}

/* ============================================================
   SaveExiste
   ============================================================ */
int SaveExiste(void) {
    FILE *f = fopen(SAVE_FILE, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

/* ============================================================
   SaveDeletar
   ============================================================ */
void SaveDeletar(void) {
    remove(SAVE_FILE);
}

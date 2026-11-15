#include "tripulacao.h"
#include "constantes.h"

/* Dados iniciais da tripulação. Cada linha é um Tripulante, com os
   valores na ordem dos campos do struct:
   nome, arquivo, idade, oxigenio, vivo, { traços }.
   Os traços seguem a ordem do Tracos:
   egoismo, inteligencia, carisma, impulsividade, curiosidade, sorte.
   A ordem das linhas é a mesma dos sprites no tela_nave.c. */
static const Tripulante iniciais[TOTAL_TRIPULANTES] = {
    { "Bento", "assets/imagens/tripulante_castanho.png",       23, OXIGENIO_MAXIMO, true,
      { 45, 35, 35, 90, 55, 40 } },
    { "Ana",   "assets/imagens/tripulante_rabo_de_cavalo.png", 22, OXIGENIO_MAXIMO, true,
      { 20, 88, 45, 30, 60, 40 } },
    { "Elias", "assets/imagens/tripulante_cacheado.png",       21, OXIGENIO_MAXIMO, true,
      { 30, 65, 40, 40, 89, 50 } },
    { "Flora", "assets/imagens/tripulante_ruivo.png",          20, OXIGENIO_MAXIMO, true,
      { 25, 50, 90, 35, 40, 45 } },
    { "Dora",  "assets/imagens/tripulante_loiro.png",          22, OXIGENIO_MAXIMO, true,
      { 35, 40, 50, 45, 50, 91 } },
    { "Caio",  "assets/imagens/tripulante_preto.png",          23, OXIGENIO_MAXIMO, true,
      { 92, 60, 55, 50, 30, 35 } }
};

void iniciarTripulacao(Tripulante tripulacao[]){
    for(int i = 0; i < TOTAL_TRIPULANTES; i++){
        tripulacao[i] = iniciais[i];
    }
}
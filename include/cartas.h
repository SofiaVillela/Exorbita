#ifndef CARTAS_H
#define CARTAS_H

#include <stdbool.h>
#include <stdint.h>

#define CARTAS_NA_MAO 3
#define CARTAS_NO_BARALHO 9

typedef enum { CARTAS_ESCOLHENDO, CARTAS_RESULTADO, CARTAS_ENCERRADAS } FaseCartas;

typedef struct {
    int maoJogador[CARTAS_NA_MAO], maoAlien[CARTAS_NA_MAO];
    bool usadasJogador[CARTAS_NA_MAO], usadasAlien[CARTAS_NA_MAO];
    int rodada, pontosJogador, pontosAlien;
    int indiceAlien, anuncioAlien, cartaJogada;
    bool desconfiou, venceuRodada, vitoriaConsumida;
    uint32_t aleatorio;
    FaseCartas fase;
} PartidaCartas;

/* Baralho único de 1 a 9. A semente externa torna partidas reproduzíveis. */
bool iniciarCartas(PartidaCartas *partida, uint32_t semente);
/* Cada jogada consome uma carta de cada mão. Comparar usa os valores reais;
   desconfiar vence somente se o anúncio do visitante for falso. */
bool jogarCartas(PartidaCartas *partida, int indiceCarta, bool desconfiar);
bool avancarRodadaCartas(PartidaCartas *partida);
/* Entrega o resultado vencedor uma única vez para o progresso do jogo. */
bool consumirVitoriaCartas(PartidaCartas *partida);

#endif

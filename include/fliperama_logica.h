/* fliperama_logica.h: regras do jogo de naves do fliperama do bar
   (planeta extraterrestre).

   O jogador pilota uma nave na parte de baixo da tela e atira para
   cima. As naves inimigas ficam numa formação que balança de um lado
   para o outro; de tempos em tempos uma delas sai mergulhando em
   direção ao jogador, atirando. Limpar a formação traz a onda
   seguinte, mais rápida. Cada tiro inimigo ou batida custa uma vida.

   Este módulo não usa a raylib: dá para compilar e testar sozinho
   (ver tests/teste_fliperama.c). As medidas são em pixels da tela do
   fliperama, com (0, 0) no canto de cima à esquerda. */

#ifndef FLIPERAMA_LOGICA_H
#define FLIPERAMA_LOGICA_H

#include <stdbool.h>
#include <stdint.h>

#define FLIP_LARGURA        400    /* tamanho da tela do fliperama */
#define FLIP_ALTURA         296
#define FLIP_TOPO_JOGADOR   200    /* o jogador anda só daqui para baixo */
#define FLIP_MAX_INIMIGOS   24
#define FLIP_MAX_TIROS      6      /* tiros do jogador ao mesmo tempo */
#define FLIP_MAX_TIROS_INIMIGOS 12
#define FLIP_MAX_EXPLOSOES  12
#define FLIP_VIDAS          3
#define FLIP_VELOCIDADE     170.0f /* nave do jogador, pixels por segundo */
#define FLIP_VELOCIDADE_TIRO 340.0f
#define FLIP_RECARGA        0.24f  /* segundos entre dois tiros */
#define FLIP_TEMPO_INVENCIVEL 2.0f /* depois de perder uma vida */
#define FLIP_TEMPO_ENTRE_ONDAS 1.8f
#define FLIP_TEMPO_EXPLOSAO 0.4f
#define FLIP_RAIO_JOGADOR   10.0f  /* tamanho usado nas batidas */
#define FLIP_RAIO_INIMIGO   11.0f

typedef enum { FLIP_JOGANDO, FLIP_ENTRE_ONDAS, FLIP_FIM } FaseFliperama;
typedef enum { INIMIGO_EM_FORMACAO, INIMIGO_MERGULHANDO, INIMIGO_VOLTANDO } EstadoInimigo;

typedef struct {
    float x, y;
    bool ativo;
} TiroFliperama;

typedef struct {
    int tipo;             /* 0 batedor, 1 caça, 2 comandante (vale mais) */
    int vida;             /* tiros que ainda aguenta */
    bool vivo;
    EstadoInimigo estado;
    float x, y;           /* posição na tela */
    float lugarX, lugarY; /* lugar na formação (sem o balanço) */
    float tempoMergulho;  /* segundos desde que começou a mergulhar */
    float lado;           /* -1 ou 1: para que lado faz a curva */
    float origemX;        /* x onde o mergulho começou */
    float recargaTiro;
} InimigoFliperama;

typedef struct {
    float x, y, tempo;
    bool grande;          /* a do jogador é maior */
    bool ativa;
} ExplosaoFliperama;

typedef struct {
    FaseFliperama fase;
    float jogadorX, jogadorY;
    int vidas, pontos, onda;
    float recarga, invencivel, tempoFase, tempo;
    float balanco;        /* deslocamento da formação, de um lado para o outro */
    float proximoMergulho;
    InimigoFliperama inimigos[FLIP_MAX_INIMIGOS];
    int totalInimigos;
    TiroFliperama tiros[FLIP_MAX_TIROS];
    TiroFliperama tirosInimigos[FLIP_MAX_TIROS_INIMIGOS];
    ExplosaoFliperama explosoes[FLIP_MAX_EXPLOSOES];
    uint32_t aleatorio;
} Fliperama;

/* Partida nova: 3 vidas, 0 pontos, primeira onda já na tela.
   A semente torna as partidas reproduzíveis nos testes. */
void iniciarFliperama(Fliperama *f, uint32_t semente);

/* Avança dt segundos. eixoX e eixoY vão de -1 a 1 (WASD);
   atirar é a tecla de tiro apertada neste quadro ou segurada. */
void atualizarFliperama(Fliperama *f, float dt, float eixoX, float eixoY, bool atirar);

/* Quantas naves inimigas ainda estão vivas. */
int inimigosVivos(const Fliperama *f);

/* Pontos que um inimigo vale; mergulhando vale o dobro. */
int pontosInimigo(const InimigoFliperama *inimigo);

#endif

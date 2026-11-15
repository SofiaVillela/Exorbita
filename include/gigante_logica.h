/* gigante_logica.h: regras do planeta do gigante, sem raylib.

   O gigante observa o planalto. Pisar numa planta brilhante ou esbarrar
   em um habitante é uma infração, e cada infração soma pontos. Com
   pontos suficientes o gigante reage; no limite, ele pega o tripulante.
   Quem foi pego tenta se explicar: cada resposta depende de um traço.
   O sorteio fica com quem chama (a tela usa GetRandomValue), para que
   as regras possam ser testadas sem abrir o jogo. */

#ifndef GIGANTE_LOGICA_H
#define GIGANTE_LOGICA_H

#include <stdbool.h>
#include "exploracao.h"
#include "tripulacao.h"

typedef enum {
    GIGANTE_OBSERVANDO,   /* nenhuma infração */
    GIGANTE_ALERTA,       /* há infrações recentes */
    GIGANTE_REACAO,       /* muitas infrações: o chão treme */
    GIGANTE_CALMA         /* há infrações, mas ele está se acalmando */
} EstadoGigante;

typedef enum {
    INFRACAO_NENHUMA,
    INFRACAO_PLANTA,
    INFRACAO_HABITANTE
} InfracaoGigante;

typedef struct {
    int pontos;              /* 0 a GIGANTE_LIMITE_INFRACOES */
    int infracoes;           /* quantas infrações já aconteceram neste pouso */
    InfracaoGigante ultima;  /* a mais recente (o gigante lembra dela) */
    float semInfracao;       /* segundos desde a última infração */
    float perdao;            /* segundos acumulados para perdoar 1 ponto */
} VigilanciaGigante;

/* O que o tripulante pode dizer depois de pego. */
typedef enum {
    APELO_EXPLICAR,   /* depende do carisma */
    APELO_REGRAS,     /* depende da inteligência */
    APELO_CULPAR,     /* depende do egoísmo */
    TOTAL_APELOS
} TipoApelo;

typedef struct {
    const char *fala;      /* o que o tripulante diz */
    const char *detalhe;   /* de que traço depende */
} Apelo;

/* Habitante do planalto: um bichinho que anda aos pulos, de um lado
   para o outro, entre minX e maxX. Só sai do lugar enquanto está no ar. */
typedef struct {
    float x, y;          /* pés */
    float minX, maxX;
    float velocidade;    /* pixels por segundo; o sinal é o sentido */
    float relogio;       /* tempo dentro do ciclo pulo + pausa */
    float atordoado;     /* segundos parado depois de um esbarrão */
} HabitanteGigante;

void iniciarVigilanciaGigante(VigilanciaGigante *vigilancia);

/* Soma os pontos da infração. Devolve true quando chega ao limite
   (o gigante pega o tripulante). */
bool registrarInfracaoGigante(VigilanciaGigante *vigilancia, InfracaoGigante infracao);

/* Passa o tempo sem infração. Devolve true no instante em que o
   gigante perdoa 1 ponto. */
bool acalmarGigante(VigilanciaGigante *vigilancia, float dt);

EstadoGigante estadoGigante(const VigilanciaGigante *vigilancia);
int pontosInfracaoGigante(InfracaoGigante infracao);
const char *nomeEstadoGigante(EstadoGigante estado);
const char *nomeInfracaoGigante(InfracaoGigante infracao);

/* Apelo do vetor fixo, pelo tipo. */
const Apelo *apeloGigante(TipoApelo tipo);
/* Chance (0 a 100) de o apelo convencer o gigante. */
int chanceApeloGigante(TipoApelo tipo, const Tracos *tracos);
/* true se o sorteio (1 a 100) cair dentro da chance. */
bool apeloConvenceu(int chance, int sorteio);

void atualizarHabitanteGigante(HabitanteGigante *habitante, float dt);
/* Altura do pulo neste instante, de 0 (no chão) a 1 (ponto mais alto). */
float alturaPuloHabitante(const HabitanteGigante *habitante);
bool habitanteEncostou(const HabitanteGigante *habitante, PontoExploracao pes);
bool pisouNaPlanta(PontoExploracao planta, PontoExploracao pes);

#endif

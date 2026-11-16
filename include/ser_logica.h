/* ser_logica.h: regras do planeta do Ser dos desejos, sem raylib.

   O Ser oferece um desejo a quem desceu. Alguns desejos aparecem para
   todos; outros exigem um traço mínimo; dois são armadilhas, ligadas
   aos traços ruins (egoísmo e impulsividade).
   A lista de desejos é um vetor fixo; os que aparecem para o tripulante
   saem de um filtro que percorre esse vetor. O desejo "Mostre o caminho"
   percorre a lista circular de planetas.
   O sorteio fica com quem chama (a tela usa GetRandomValue), para que
   as regras possam ser testadas sem abrir o jogo. */

#ifndef SER_LOGICA_H
#define SER_LOGICA_H

#include <stdbool.h>
#include "partida.h"
#include "lista_planetas.h"

typedef enum {
    /* para todos */
    DESEJO_TANQUE,        /* combustível completo */
    DESEJO_INTELIGENCIA,  /* +15 de inteligência para quem desceu */
    DESEJO_SORTE,         /* +20 de sorte para quem desceu */
    /* liberados por traço */
    DESEJO_REJUVENESCER,  /* quem desceu passa a ter 18 anos (carisma 50+) */
    DESEJO_REVIVER,       /* o primeiro morto volta (sorte 55+) */
    DESEJO_CAMINHO,       /* próximo planeta não visitado conta como visitado (curiosidade 60+) */
    /* armadilhas */
    DESEJO_TERRA,         /* quem desceu morre (egoísmo 70+) */
    DESEJO_TUDO,          /* quem pediu envelhece 30 anos (impulsividade 70+) */
    TOTAL_DESEJOS
} TipoDesejo;

typedef struct {
    TipoDesejo tipo;
    const char *pedido;      /* o que o tripulante diz */
    const char *detalhe;     /* explicação curta, mostrada na lista */
} Desejo;

/* Desejo do vetor fixo, pelo tipo. */
const Desejo *desejoDoTipo(TipoDesejo tipo);

/* Chance (0 a 100) de o Ser aparecer.
   visitasAnteriores: quantos pousos já houve antes deste (0 = primeiro).
   O primeiro é garantido; depois, a chance base da visita mais
   (sorte - 50) / 2, limitada entre 0 e 100. */
int chanceSer(int visitasAnteriores, int sorte);

/* true se o sorteio (1 a 100) cair dentro da chance. */
bool serApareceu(int chance, int sorteio);

/* Índice do primeiro tripulante morto, ou -1 se todos estão vivos. */
int primeiroMorto(const Tripulante tripulacao[]);

/* Preenche disponiveis[] com os desejos que o Ser oferece a quem desceu,
   na ordem do vetor fixo, e devolve quantos são.
   planetas: início da lista circular (pode ser NULL; aí "Mostre o
   caminho" não aparece). disponiveis precisa ter espaço para TOTAL_DESEJOS. */
int desejosDisponiveis(const ProgressoPartida *jogo, int quemDesceu,
                       Planeta *planetas, TipoDesejo disponiveis[]);

/* Aplica o desejo na partida e devolve o texto do que aconteceu.
   O texto fica num buffer interno e vale até a próxima chamada. */
const char *aplicarDesejo(ProgressoPartida *jogo, int quemDesceu,
                          Planeta *planetas, TipoDesejo tipo);

#endif

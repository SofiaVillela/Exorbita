#ifndef PARTIDA_H
#define PARTIDA_H

#include <stdbool.h>
#include "tripulacao.h" 
#include "constantes.h"

/* Progresso em memória, preservado ao voltar para a nave; ainda sem save. */
typedef struct {
    bool temTradutor;
    bool roboConcluido;
    int vitoriasCartas;
    bool extraterrestreConcluido;   /* pedaço do mapa ganho do viajante, nas cartas */
    int capturasGigante;       /* vezes em que o gigante pegou alguém */
    bool giganteConcluido;     /* pedaço do mapa tirado do posto do planalto */
    int cristaisGigante;       /* cristais de luz já recolhidos: um bit para cada (fica entre visitas) */
    bool celulaGiganteUsada;   /* a célula de combustível dos destroços já foi levada */
    int etapaPortaoSemAr;      /* 0 sem energia, 1 pede a placa, 2 aberto (fica entre visitas) */
    bool semArConcluido;       /* pedaço do mapa trazido para a nave */
    int combustivel;   /* combustível atual da nave; o painel de planetas gasta */
    int visitasSer;    /* pousos no planeta do Ser; decide a chance de ele aparecer */
    bool tanqueSerUsado;   /* "Encher o tanque" já foi pedido: o Ser só dá uma vez por partida */
    Tripulante tripulacao[TOTAL_TRIPULANTES];
    int controlado;
} ProgressoPartida;

ProgressoPartida *progressoPartida(void);
/* Começa uma partida nova: combustível inicial, tripulação original,
   nenhum planeta concluído. Chamada ao escolher Jogar no menu. */
void iniciarPartida(void);
Tripulante *tripulanteControlado(void);

/* Pedaços do mapa estelar que existem no jogo: um no planeta sem ar,
   um com o viajante do planeta extraterrestre e um no posto do
   planeta do gigante. */
#define TOTAL_PEDACOS_MAPA 3
/* Quantos pedaços já estão na nave. */
int pedacosMapaObtidos(void);
/* true com todos os pedaços na nave: é o que abre a Via Láctea. */
bool mapaEstelarCompleto(void);

/* Existe uma partida que pode ser continuada pelo menu?
   Vira true ao começar uma partida nova e false quando ela acaba
   (chegada à Via Láctea ou ninguém vivo a bordo). */
bool partidaEmAndamento(void);
void marcarPartidaEmAndamento(bool emAndamento);

#endif

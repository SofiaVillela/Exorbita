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
    bool extraterrestreConcluido;
    int capturasGigante;       /* vezes em que o gigante pegou alguém */
    bool giganteConcluido;     /* marco do posto registrado */
    int etapaPortaoSemAr;      /* 0 sem energia, 1 pede a placa, 2 aberto (fica entre visitas) */
    bool semArConcluido;       /* pedaço do mapa trazido para a nave */
    int combustivel;   /* combustível atual da nave; o painel de planetas gasta */
    int visitasSer;    /* pousos no planeta do Ser; decide a chance de ele aparecer */
    Tripulante tripulacao[TOTAL_TRIPULANTES];
    int controlado;
} ProgressoPartida;

ProgressoPartida *progressoPartida(void);
/* Começa uma partida nova: combustível inicial, tripulação original,
   nenhum planeta concluído. Chamada ao escolher Jogar no menu. */
void iniciarPartida(void);
Tripulante *tripulanteControlado(void);

/* Existe uma partida que pode ser continuada pelo menu?
   Vira true ao começar uma partida nova e false quando ela acaba
   (chegada à Via Láctea ou ninguém vivo a bordo). */
bool partidaEmAndamento(void);
void marcarPartidaEmAndamento(bool emAndamento);

#endif

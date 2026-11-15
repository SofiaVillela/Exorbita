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
    unsigned int areasGiganteVisitadas;
    int capturasGigante;
    bool giganteConcluido;
    int combustivel;   /* combustível atual da nave; o painel de planetas gasta */
    int visitasSer;    /* pousos no planeta do Ser; decide a chance de ele aparecer */
    Tripulante tripulacao[TOTAL_TRIPULANTES];
    int controlado;
} ProgressoPartida;

ProgressoPartida *progressoPartida(void);
void iniciarPartida(void);
Tripulante *tripulanteControlado(void);

#endif

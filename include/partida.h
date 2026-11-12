#ifndef PARTIDA_H
#define PARTIDA_H

#include <stdbool.h>

/* Progresso em memória, preservado ao voltar para a nave; ainda sem save. */
typedef struct {
    bool temTradutor;
    bool roboConcluido;
    int inteligencia;
    int vitoriasCartas;
    bool extraterrestreConcluido;
    unsigned int areasGiganteVisitadas;
    int capturasGigante;
    bool giganteConcluido;
    int combustivel;   /* combustível atual da nave; o painel de planetas gasta */
} ProgressoPartida;

ProgressoPartida *progressoPartida(void);

#endif

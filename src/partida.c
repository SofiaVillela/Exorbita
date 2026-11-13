#include "partida.h"
#include "constantes.h"

static ProgressoPartida progresso = { .inteligencia = 65,
                                      .combustivel = COMBUSTIVEL_INICIAL };

ProgressoPartida *progressoPartida(void)
{
    return &progresso;
}

void iniciarPartida(void)
{
    iniciarTripulacao(progresso.tripulacao);   /* chamada: passa o vetor do estado */
    progresso.controlado = 0;
}
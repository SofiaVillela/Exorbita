#include "partida.h"
#include "constantes.h"

static ProgressoPartida progresso = { .inteligencia = 65,
                                      .combustivel = COMBUSTIVEL_INICIAL };

ProgressoPartida *progressoPartida(void)
{
    return &progresso;
}

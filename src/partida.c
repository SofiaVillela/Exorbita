#include "partida.h"

static ProgressoPartida progresso = { .inteligencia = 65 };

ProgressoPartida *progressoPartida(void)
{
    return &progresso;
}

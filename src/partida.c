#include "partida.h"
#include "constantes.h"

static ProgressoPartida progresso = { 
     .combustivel = COMBUSTIVEL_INICIAL,
     .visitasSer = 0 };

ProgressoPartida *progressoPartida(void)
{
    return &progresso;
}

void iniciarPartida(void)
{
    /* Nova partida: tudo volta ao começo. Os campos que não aparecem
       aqui (tradutor, missões concluídas, capturas...) ficam zerados. */
    progresso = (ProgressoPartida){
        .combustivel = COMBUSTIVEL_INICIAL,
        .visitasSer = 0
    };
    iniciarTripulacao(progresso.tripulacao);   /* chamada: passa o vetor do estado */
    progresso.controlado = 0;
}
    
Tripulante *tripulanteControlado(void)
{
    return &progresso.tripulacao[progresso.controlado];
}

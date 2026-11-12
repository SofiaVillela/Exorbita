#ifndef GIGANTE_LOGICA_H
#define GIGANTE_LOGICA_H

#include <stdbool.h>
#include "exploracao.h"

#define GIGANTE_TOTAL_AREAS 4
#define GIGANTE_TODAS_AREAS 15u

typedef enum {
    GIGANTE_CALMO, GIGANTE_SUSPEITA, GIGANTE_ALERTA, GIGANTE_CAPTURADO
} EstadoVigilanciaGigante;

typedef enum {
    INFRACAO_NENHUMA, INFRACAO_EXPOSICAO, INFRACAO_CORRIDA, INFRACAO_RESTRITA
} InfracaoGigante;

typedef struct {
    PontoExploracao origem;
    float angulo, meioAngulo, alcance;
    float tempoAberto, tempoFechado, defasagem;
} OlhoGigante;

typedef struct {
    int area;
    unsigned int areasVisitadas;
    float alerta, tempo;
    int infracoes, capturas;
    bool concluido;
    EstadoVigilanciaGigante estado;
    InfracaoGigante ultimaInfracao;
    bool vistoAntes, corridaAntes, restritoAntes;
} VigilanciaGigante;

void iniciarVigilanciaGigante(VigilanciaGigante *vigilancia, unsigned int visitadas,
                              int capturas, bool concluido);
bool olhoGiganteAberto(const OlhoGigante *olho, float tempo);
/* Retorna a distância até a primeira cobertura; usada também para desenhar
   o mesmo campo de visão que a lógica considera. Ângulos em radianos. */
float alcanceRaioGigante(PontoExploracao origem, float angulo, float alcance,
                         const ObstaculoExploracao *coberturas, int quantidade);
bool olhoGiganteVe(const OlhoGigante *olho, PontoExploracao pessoa, float tempo,
                   const ObstaculoExploracao *coberturas, int quantidade);
/* Infrações são registradas nas entradas de condições, não a cada frame.
   Retorna true somente no instante em que uma nova captura ocorre. */
bool atualizarVigilanciaGigante(VigilanciaGigante *vigilancia, bool visto,
                                bool correndo, bool restrito, float dt);
bool mudarAreaGigante(VigilanciaGigante *vigilancia, int direcao);
bool confirmarCapturaGigante(VigilanciaGigante *vigilancia);
bool concluirObjetivoGigante(VigilanciaGigante *vigilancia);
const char *nomeEstadoGigante(EstadoVigilanciaGigante estado);
const char *nomeInfracaoGigante(InfracaoGigante infracao);

#endif

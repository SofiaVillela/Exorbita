/* gigante_logica.c: regras do planeta do gigante (ver gigante_logica.h). */

#include <math.h>
#include <stddef.h>
#include "constantes.h"
#include "gigante_logica.h"

#define PI_GIGANTE 3.14159265f

/* Na ordem do TipoApelo. */
static const Apelo apelos[TOTAL_APELOS] = {
    { "Explicar com calma o que aconteceu.",     "Depende do carisma" },
    { "Dizer que as regras não estavam claras.", "Depende da inteligência" },
    { "Pôr a culpa em outro tripulante.",        "Depende do egoísmo" }
};

void iniciarVigilanciaGigante(VigilanciaGigante *v)
{
    if (v == NULL) {
        return;
    }
    v->pontos = 0;
    v->infracoes = 0;
    v->ultima = INFRACAO_NENHUMA;
    v->semInfracao = 0;
    v->perdao = 0;
}

int pontosInfracaoGigante(InfracaoGigante infracao)
{
    if (infracao == INFRACAO_PLANTA) {
        return GIGANTE_PONTOS_PLANTA;
    }
    if (infracao == INFRACAO_HABITANTE) {
        return GIGANTE_PONTOS_HABITANTE;
    }
    return 0;
}

bool registrarInfracaoGigante(VigilanciaGigante *v, InfracaoGigante infracao)
{
    int pontos = pontosInfracaoGigante(infracao);
    if (v == NULL || pontos == 0) {
        return false;
    }
    v->infracoes++;
    v->ultima = infracao;
    v->pontos += pontos;
    if (v->pontos > GIGANTE_LIMITE_INFRACOES) {
        v->pontos = GIGANTE_LIMITE_INFRACOES;
    }
    /* Qualquer infração interrompe a calma. */
    v->semInfracao = 0;
    v->perdao = 0;
    return v->pontos >= GIGANTE_LIMITE_INFRACOES;
}

bool acalmarGigante(VigilanciaGigante *v, float dt)
{
    if (v == NULL || !isfinite(dt) || dt <= 0) {
        return false;
    }
    v->semInfracao += dt;
    if (v->pontos <= 0 || v->semInfracao <= GIGANTE_TEMPO_CALMA) {
        return false;
    }
    v->perdao += dt;
    if (v->perdao < GIGANTE_TEMPO_PERDAO) {
        return false;
    }
    v->perdao = 0;
    v->pontos--;
    return true;
}

EstadoGigante estadoGigante(const VigilanciaGigante *v)
{
    if (v == NULL || v->pontos <= 0) {
        return GIGANTE_OBSERVANDO;
    }
    if (v->pontos >= GIGANTE_PONTOS_REACAO) {
        return GIGANTE_REACAO;
    }
    if (v->semInfracao > GIGANTE_TEMPO_CALMA) {
        return GIGANTE_CALMA;
    }
    return GIGANTE_ALERTA;
}

const char *nomeEstadoGigante(EstadoGigante estado)
{
    switch (estado) {
        case GIGANTE_ALERTA: return "ALERTA";
        case GIGANTE_REACAO: return "REAÇÃO";
        case GIGANTE_CALMA:  return "CALMA";
        default:             return "OBSERVANDO";
    }
}

const char *nomeInfracaoGigante(InfracaoGigante infracao)
{
    switch (infracao) {
        case INFRACAO_PLANTA:    return "pisou numa planta brilhante";
        case INFRACAO_HABITANTE: return "esbarrou em um habitante";
        default:                 return "";
    }
}

const Apelo *apeloGigante(TipoApelo tipo)
{
    if (tipo < 0 || tipo >= TOTAL_APELOS) {
        return NULL;
    }
    return &apelos[tipo];
}

int chanceApeloGigante(TipoApelo tipo, const Tracos *tracos)
{
    if (tracos == NULL) {
        return 0;
    }
    int chance = 0;
    if (tipo == APELO_EXPLICAR) {
        chance = tracos->carisma * GIGANTE_APELO_CARISMA / 100;
    } else if (tipo == APELO_REGRAS) {
        chance = tracos->inteligencia * GIGANTE_APELO_INTELIGENCIA / 100;
    } else if (tipo == APELO_CULPAR) {
        chance = tracos->egoismo * GIGANTE_APELO_EGOISMO / 100;
    }
    if (chance < 0) {
        chance = 0;
    }
    if (chance > 100) {
        chance = 100;
    }
    return chance;
}

bool apeloConvenceu(int chance, int sorteio)
{
    return sorteio <= chance;
}

void atualizarHabitanteGigante(HabitanteGigante *h, float dt)
{
    if (h == NULL || !isfinite(dt) || dt <= 0) {
        return;
    }
    if (h->atordoado > 0) {
        h->atordoado -= dt;
        h->relogio = GIGANTE_TEMPO_PULO;   /* fica no chão até se recuperar */
        return;
    }
    float ciclo = GIGANTE_TEMPO_PULO + GIGANTE_TEMPO_CHAO;
    bool noAr = h->relogio < GIGANTE_TEMPO_PULO;
    if (noAr) {
        h->x += h->velocidade * dt;
        /* Chegou à ponta do caminho: volta para dentro e inverte o sentido. */
        if (h->x < h->minX) {
            h->x = h->minX;
            h->velocidade = fabsf(h->velocidade);
        } else if (h->x > h->maxX) {
            h->x = h->maxX;
            h->velocidade = -fabsf(h->velocidade);
        }
    }
    h->relogio += dt;
    while (h->relogio >= ciclo) {
        h->relogio -= ciclo;
    }
}

float alturaPuloHabitante(const HabitanteGigante *h)
{
    if (h == NULL || h->atordoado > 0 || h->relogio < 0 ||
        h->relogio >= GIGANTE_TEMPO_PULO) {
        return 0;
    }
    return sinf(PI_GIGANTE * h->relogio / GIGANTE_TEMPO_PULO);
}

static bool dentroDoRaio(float ax, float ay, PontoExploracao b, float raio)
{
    float dx = ax - b.x;
    float dy = ay - b.y;
    return dx * dx + dy * dy < raio * raio;
}

bool habitanteEncostou(const HabitanteGigante *h, PontoExploracao pes)
{
    if (h == NULL || h->atordoado > 0) {
        return false;   /* atordoado não conta de novo */
    }
    return dentroDoRaio(h->x, h->y, pes, GIGANTE_RAIO_HABITANTE);
}

bool pisouNaPlanta(PontoExploracao planta, PontoExploracao pes)
{
    return dentroDoRaio(planta.x, planta.y, pes, GIGANTE_RAIO_PLANTA);
}

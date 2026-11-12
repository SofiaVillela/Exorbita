#include "gigante_logica.h"

#include <limits.h>
#include <math.h>
#include <stddef.h>

void iniciarVigilanciaGigante(VigilanciaGigante *v, unsigned int visitadas,
                              int capturas, bool concluido)
{
    if (!v) return;
    *v = (VigilanciaGigante){ 0 };
    v->areasVisitadas = (visitadas & GIGANTE_TODAS_AREAS) | 1u;
    v->capturas = capturas < 0 ? 0 : capturas;
    v->concluido = concluido;
}

bool olhoGiganteAberto(const OlhoGigante *olho, float tempo)
{
    if (!olho || !isfinite(tempo) || !isfinite(olho->tempoAberto) ||
        !isfinite(olho->tempoFechado) || !isfinite(olho->defasagem) ||
        olho->tempoAberto <= 0 || olho->tempoFechado < 0) return false;
    float periodo = olho->tempoAberto + olho->tempoFechado;
    if (!isfinite(periodo) || periodo <= 0) return false;
    float instante = fmodf(tempo + olho->defasagem, periodo);
    if (instante < 0) instante += periodo;
    return instante < olho->tempoAberto;
}

static bool atingirEixo(float origem, float direcao, float minimo, float maximo,
                         float *entrada, float *saida)
{
    if (fabsf(direcao) < 0.000001f) return origem >= minimo && origem <= maximo;
    float a = (minimo - origem) / direcao;
    float b = (maximo - origem) / direcao;
    if (a > b) { float troca = a; a = b; b = troca; }
    if (a > *entrada) *entrada = a;
    if (b < *saida) *saida = b;
    return *entrada <= *saida;
}

float alcanceRaioGigante(PontoExploracao origem, float angulo, float alcance,
                         const ObstaculoExploracao *coberturas, int quantidade)
{
    if (!isfinite(origem.x) || !isfinite(origem.y) || !isfinite(angulo) ||
        !isfinite(alcance) || alcance <= 0 || quantidade < 0 ||
        (quantidade > 0 && !coberturas)) return 0;
    float dx = cosf(angulo), dy = sinf(angulo);
    float menor = alcance;
    for (int i = 0; i < quantidade; i++) {
        ObstaculoExploracao r = coberturas[i];
        if (!isfinite(r.x) || !isfinite(r.y) || !isfinite(r.largura) ||
            !isfinite(r.altura)) return 0;
        if (r.largura <= 0 || r.altura <= 0) continue;
        float entrada = 0, saida = menor;
        if (atingirEixo(origem.x, dx, r.x, r.x + r.largura, &entrada, &saida) &&
            atingirEixo(origem.y, dy, r.y, r.y + r.altura, &entrada, &saida) &&
            entrada < menor && saida >= 0) menor = fmaxf(0, entrada);
    }
    return menor;
}

bool olhoGiganteVe(const OlhoGigante *olho, PontoExploracao pessoa, float tempo,
                   const ObstaculoExploracao *coberturas, int quantidade)
{
    if (!olhoGiganteAberto(olho, tempo) || !isfinite(pessoa.x) ||
        !isfinite(pessoa.y) || !isfinite(olho->origem.x) ||
        !isfinite(olho->origem.y) || !isfinite(olho->angulo) ||
        !isfinite(olho->meioAngulo) || !isfinite(olho->alcance) ||
        olho->meioAngulo <= 0 || olho->meioAngulo > 3.141593f || olho->alcance <= 0)
        return false;
    float dx = pessoa.x - olho->origem.x;
    float dy = pessoa.y - olho->origem.y;
    float distancia = hypotf(dx, dy);
    if (!isfinite(distancia) || distancia > olho->alcance) return false;
    if (distancia > 0.001f) {
        float produto = (dx * cosf(olho->angulo) + dy * sinf(olho->angulo)) / distancia;
        if (produto + 0.000001f < cosf(olho->meioAngulo)) return false;
    }
    float angulo = distancia > 0.001f ? atan2f(dy, dx) : olho->angulo;
    return alcanceRaioGigante(olho->origem, angulo, distancia + 0.002f,
                              coberturas, quantidade) >= distancia + 0.001f;
}

static void registrar(VigilanciaGigante *v, InfracaoGigante infracao, float aumento)
{
    if (v->infracoes < INT_MAX) v->infracoes++;
    v->ultimaInfracao = infracao;
    v->alerta += aumento;
}

bool atualizarVigilanciaGigante(VigilanciaGigante *v, bool visto,
                                bool correndo, bool restrito, float dt)
{
    if (!v || !isfinite(dt) || dt <= 0 || v->estado == GIGANTE_CAPTURADO) return false;
    if (!isfinite(v->alerta) || v->alerta < 0) v->alerta = 0;
    if (!isfinite(v->tempo) || v->tempo < 0) v->tempo = 0;
    v->tempo = fmodf(v->tempo + dt, 3600.0f);
    if (visto && !v->vistoAntes) registrar(v, INFRACAO_EXPOSICAO, 8);
    if (visto && correndo && !v->corridaAntes) registrar(v, INFRACAO_CORRIDA, 6);
    if (restrito && !v->restritoAntes) registrar(v, INFRACAO_RESTRITA, 10);
    v->vistoAntes = visto;
    v->corridaAntes = visto && correndo;
    v->restritoAntes = restrito;
    if (visto) v->alerta += (correndo ? 30.0f : 18.0f) * dt;
    if (restrito) v->alerta += 12.0f * dt;
    if (!visto && !restrito) v->alerta -= 12.0f * dt;
    if (v->alerta >= 100) {
        v->alerta = 100;
        v->estado = GIGANTE_CAPTURADO;
        if (v->capturas < INT_MAX) v->capturas++;
        return true;
    }
    if (v->alerta < 0) v->alerta = 0;
    v->estado = v->alerta >= 55 ? GIGANTE_ALERTA :
                v->alerta > 0 ? GIGANTE_SUSPEITA : GIGANTE_CALMO;
    return false;
}

bool mudarAreaGigante(VigilanciaGigante *v, int direcao)
{
    if (!v || v->estado == GIGANTE_CAPTURADO || v->area < 0 ||
        v->area >= GIGANTE_TOTAL_AREAS || (direcao != -1 && direcao != 1)) return false;
    int destino = v->area + direcao;
    if (destino < 0 || destino >= GIGANTE_TOTAL_AREAS) return false;
    v->area = destino;
    v->areasVisitadas |= 1u << destino;
    v->vistoAntes = v->corridaAntes = v->restritoAntes = false;
    return true;
}

bool confirmarCapturaGigante(VigilanciaGigante *v)
{
    if (!v || v->estado != GIGANTE_CAPTURADO) return false;
    v->area = 0;
    v->alerta = 0;
    v->estado = GIGANTE_CALMO;
    v->ultimaInfracao = INFRACAO_NENHUMA;
    v->vistoAntes = v->corridaAntes = v->restritoAntes = false;
    v->areasVisitadas |= 1u;
    return true;
}

bool concluirObjetivoGigante(VigilanciaGigante *v)
{
    if (!v || v->estado == GIGANTE_CAPTURADO || v->area != 3 ||
        (v->areasVisitadas & GIGANTE_TODAS_AREAS) != GIGANTE_TODAS_AREAS ||
        v->concluido) return false;
    v->concluido = true;
    return true;
}

const char *nomeEstadoGigante(EstadoVigilanciaGigante estado)
{
    switch (estado) {
        case GIGANTE_SUSPEITA: return "SUSPEITA";
        case GIGANTE_ALERTA: return "ALERTA";
        case GIGANTE_CAPTURADO: return "CAPTURADO";
        default: return "CALMO";
    }
}

const char *nomeInfracaoGigante(InfracaoGigante infracao)
{
    switch (infracao) {
        case INFRACAO_EXPOSICAO: return "Você entrou no campo de visão.";
        case INFRACAO_CORRIDA: return "Corrida diante de um olho.";
        case INFRACAO_RESTRITA: return "Entrada em área restrita.";
        default: return "";
    }
}

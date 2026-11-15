/* sem_ar_logica.c: regras do planeta sem ar (ver sem_ar_logica.h). */

#include <math.h>
#include <stddef.h>
#include "constantes.h"
#include "sem_ar_logica.h"

TipoItem itemPedidoPortao(EstadoPortao estado)
{
    if (estado == PORTAO_SEM_ENERGIA) {
        return ITEM_CELULA;
    }
    if (estado == PORTAO_PEDE_PLACA) {
        return ITEM_PLACA;
    }
    return TOTAL_ITENS;
}

ResultadoPortao usarTopoNoPortao(EstadoPortao *estado, PilhaMochila *mochila)
{
    if (estado == NULL || *estado == PORTAO_ABERTO) {
        return USO_JA_ABERTO;
    }
    TipoItem topo;
    if (!topoDaPilha(mochila, &topo)) {
        return USO_MOCHILA_VAZIA;
    }
    /* Só o topo vale: se o item pedido estiver embaixo, não adianta. */
    if (topo != itemPedidoPortao(*estado)) {
        return USO_ITEM_ERRADO;
    }
    desempilhar(mochila, NULL);
    if (*estado == PORTAO_SEM_ENERGIA) {
        *estado = PORTAO_PEDE_PLACA;
        return USO_LIGOU;
    }
    *estado = PORTAO_ABERTO;
    return USO_ABRIU;
}

ResultadoPegar pegarItem(PilhaMochila *mochila, TipoItem item, float *oxigenio)
{
    if (item == ITEM_CILINDRO) {
        if (oxigenio != NULL) {
            *oxigenio = recarregarOxigenio(*oxigenio);
        }
        return PEGAR_RESPIROU;
    }
    if (!empilhar(mochila, item)) {
        return PEGAR_CHEIA;
    }
    return PEGAR_GUARDOU;
}

float consumoOxigenio(bool correndo, bool noJato)
{
    float consumo = SEM_AR_CONSUMO_ANDANDO;
    if (correndo) {
        consumo = SEM_AR_CONSUMO_CORRENDO;
    }
    if (noJato) {
        consumo += SEM_AR_CONSUMO_JATO;
    }
    return consumo;
}

float gastarOxigenio(float oxigenio, float dt, bool correndo, bool noJato)
{
    if (!isfinite(dt) || dt <= 0) {
        return oxigenio;
    }
    oxigenio -= consumoOxigenio(correndo, noJato) * dt;
    if (oxigenio < 0) {
        oxigenio = 0;
    }
    return oxigenio;
}

float recarregarOxigenio(float oxigenio)
{
    oxigenio += SEM_AR_RECARGA_CILINDRO;
    if (oxigenio > OXIGENIO_MAXIMO) {
        oxigenio = OXIGENIO_MAXIMO;
    }
    return oxigenio;
}

FaseJato faseJato(float tempo, float defasagem)
{
    float ciclo = SEM_AR_JATO_QUIETO + SEM_AR_JATO_AVISO + SEM_AR_JATO_ATIVO;
    float instante = fmodf(tempo + defasagem, ciclo);
    if (instante < 0) {
        instante += ciclo;
    }
    if (instante < SEM_AR_JATO_QUIETO) {
        return JATO_QUIETO;
    }
    if (instante < SEM_AR_JATO_QUIETO + SEM_AR_JATO_AVISO) {
        return JATO_AVISO;
    }
    return JATO_ATIVO;
}

bool itemOcultoVisivel(int curiosidade)
{
    return curiosidade >= SEM_AR_CURIOSIDADE_OCULTO;
}

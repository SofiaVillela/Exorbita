/* pilha_mochila.c: mochila em pilha (ver pilha_mochila.h). */

#include <stddef.h>
#include "pilha_mochila.h"

void iniciarPilha(PilhaMochila *p)
{
    if (p == NULL) {
        return;
    }
    p->quantidade = 0;
}

bool pilhaVazia(const PilhaMochila *p)
{
    return p == NULL || p->quantidade <= 0;
}

bool pilhaCheia(const PilhaMochila *p)
{
    return p != NULL && p->quantidade >= MOCHILA_CAPACIDADE;
}

bool empilhar(PilhaMochila *p, TipoItem item)
{
    if (p == NULL || pilhaCheia(p)) {
        return false;
    }
    p->itens[p->quantidade] = item;   /* próxima casa livre */
    p->quantidade++;
    return true;
}

bool desempilhar(PilhaMochila *p, TipoItem *saida)
{
    if (pilhaVazia(p)) {
        return false;
    }
    p->quantidade--;                  /* o topo deixa de fazer parte da pilha */
    if (saida != NULL) {
        *saida = p->itens[p->quantidade];
    }
    return true;
}

bool topoDaPilha(const PilhaMochila *p, TipoItem *saida)
{
    if (pilhaVazia(p)) {
        return false;
    }
    if (saida != NULL) {
        *saida = p->itens[p->quantidade - 1];
    }
    return true;
}

TipoItem itemNaPosicao(const PilhaMochila *p, int i)
{
    /* 0 é o topo, então conta de trás para a frente no vetor. */
    return p->itens[p->quantidade - 1 - i];
}

bool pilhaContem(const PilhaMochila *p, TipoItem item)
{
    if (p == NULL) {
        return false;
    }
    for (int i = 0; i < p->quantidade; i++) {
        if (p->itens[i] == item) {
            return true;
        }
    }
    return false;
}

const char *nomeItem(TipoItem item)
{
    switch (item) {
        case ITEM_CILINDRO: return "Cilindro de oxigênio";
        case ITEM_PLACA:    return "Placa de acesso";
        case ITEM_CELULA:   return "Célula de energia";
        case ITEM_SUCATA:   return "Sucata";
        case ITEM_ROCHA:    return "Rocha brilhante";
        case ITEM_MAPA:     return "Pedaço do mapa";
        default:            return "";
    }
}

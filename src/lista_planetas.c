#include <stdlib.h>
#include "lista_planetas.h"

bool inserirPlaneta(Planeta **inicio, const char *nome, const char *arquivo,
                    int custo, Tela destino, bool bloqueado)
{
    Planeta *novo = malloc(sizeof(Planeta));
    if (novo == NULL) {
        return false;
    }
    novo->nome = nome;
    novo->arquivo = arquivo;
    novo->custo = custo;
    novo->destino = destino;
    novo->bloqueado = bloqueado;
    novo->visitado = false;
    if (*inicio == NULL) {
        (*inicio) = novo;
        novo->ant = novo;
        novo->prox = novo;
        return true;
    }
    Planeta *ultimo = (*inicio)->ant;
    novo->ant = ultimo;
    novo->prox = *inicio;
    ultimo->prox = novo;
    (*inicio)->ant = novo;
    return true;
}

void liberarPlanetas(Planeta **inicio)
{
    if (*inicio == NULL) {
        return;
    }

    /* Quebra o círculo: o último passa a apontar para NULL.
       Sem isso, o laço abaixo nunca acharia o fim e rodaria para sempre. */
    (*inicio)->ant->prox = NULL;

    Planeta *atual = *inicio;
    while (atual != NULL) {
        /* Guarda o próximo ANTES do free: depois do free,
           atual->prox já não pode ser lido. */
        Planeta *proximo = atual->prox;
        free(atual);
        atual = proximo;
    }

    /* Quem chamou não fica apontando para memória liberada. */
    *inicio = NULL;
}

bool todosLivresVisitados(const Planeta *inicio)
{
    if (inicio == NULL) {
        return false;
    }

    /* Percorrer uma lista circular: começa no início e para quando volta
       a ele. Precisa ser do-while, porque com while a condição
       (atual != inicio) já seria falsa antes da primeira volta. */
    const Planeta *atual = inicio;
    do {
        if (!atual->bloqueado && !atual->visitado) {
            return false;   /* achou um livre que ainda não foi visitado */
        }
        atual = atual->prox;
    } while (atual != inicio);

    return true;
}

void desbloquearPlanetas(Planeta *inicio)
{
    if (inicio == NULL) {
        return;
    }

    Planeta *atual = inicio;
    do {
        atual->bloqueado = false;
        atual = atual->prox;
    } while (atual != inicio);
}

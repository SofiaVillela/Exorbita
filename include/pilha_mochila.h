/* pilha_mochila.h: mochila do tripulante no planeta sem ar.

   Pilha = o último que entra é o primeiro que sai (LIFO). O que foi
   guardado por último fica no topo, e só o topo pode ser usado ou
   largado. Para alcançar um item de baixo, é preciso tirar os de cima.

   Feita com vetor de tamanho fixo: "quantidade" diz quantos itens há,
   e o topo é sempre itens[quantidade - 1]. Empilhar escreve na próxima
   casa livre; desempilhar só diminui "quantidade". Como a mochila tem
   capacidade pequena e conhecida, não precisa de malloc.

   Não usa a raylib: dá para testar sozinho (tests/teste_sem_ar.c). */

#ifndef PILHA_MOCHILA_H
#define PILHA_MOCHILA_H

#include <stdbool.h>
#include "constantes.h"

typedef enum {
    ITEM_CILINDRO,   /* oxigênio: é usado na hora, não vai para a mochila */
    ITEM_PLACA,      /* placa de acesso: destrava o portão */
    ITEM_CELULA,     /* célula de energia: liga o terminal do portão */
    ITEM_SUCATA,     /* sem uso: só ocupa espaço */
    ITEM_ROCHA,      /* rocha brilhante: sem uso */
    ITEM_MAPA,       /* pedaço do mapa: o objetivo */
    TOTAL_ITENS
} TipoItem;

typedef struct {
    TipoItem itens[MOCHILA_CAPACIDADE];
    int quantidade;   /* quantos itens há; o topo é itens[quantidade - 1] */
} PilhaMochila;

/* Deixa a pilha vazia. */
void iniciarPilha(PilhaMochila *p);

bool pilhaVazia(const PilhaMochila *p);
bool pilhaCheia(const PilhaMochila *p);

/* Coloca um item no topo. Devolve false se a pilha estava cheia. */
bool empilhar(PilhaMochila *p, TipoItem item);

/* Tira o item do topo e guarda em *saida. Devolve false se estava vazia. */
bool desempilhar(PilhaMochila *p, TipoItem *saida);

/* Olha o topo sem tirar. Devolve false se estava vazia. */
bool topoDaPilha(const PilhaMochila *p, TipoItem *saida);

/* Devolve o item na posição i a partir do topo, sem tirar (0 = topo).
   Serve para desenhar a mochila na tela. i precisa ser menor que quantidade. */
TipoItem itemNaPosicao(const PilhaMochila *p, int i);

/* true se o item está em qualquer lugar da pilha. */
bool pilhaContem(const PilhaMochila *p, TipoItem item);

const char *nomeItem(TipoItem item);

#endif

/* fila_comandos.h: fila dos comandos que o jogador monta para o robô.

   Fila = o primeiro que entra é o primeiro que sai (FIFO). O jogador vai
   colocando setas no fim, e o robô cumpre a partir do começo.

   Feita com vetor circular: os itens ficam num vetor de tamanho fixo, e
   "inicio" diz onde está o primeiro. Tirar o primeiro só anda "inicio"
   uma casa (dando a volta no fim do vetor), sem empurrar os outros.
   Como a fila tem tamanho máximo pequeno e conhecido, não precisa de
   malloc, então não há como vazar memória.

   Não usa a raylib: dá para testar sozinho (tests/teste_robo.c). */

#ifndef FILA_COMANDOS_H
#define FILA_COMANDOS_H

#include <stdbool.h>
#include "constantes.h"

typedef enum { CMD_CIMA, CMD_BAIXO, CMD_ESQUERDA, CMD_DIREITA } Comando;

typedef struct {
    Comando itens[ROBO_MAX_COMANDOS];
    int inicio;       /* posição do primeiro da fila dentro do vetor */
    int quantidade;   /* quantos comandos estão na fila */
} FilaComandos;

/* Deixa a fila vazia. Também serve para limpar. */
void iniciarFila(FilaComandos *f);

bool filaVazia(const FilaComandos *f);
bool filaCheia(const FilaComandos *f);

/* Coloca um comando no fim. Devolve false se a fila estava cheia. */
bool enfileirar(FilaComandos *f, Comando c);

/* Tira o primeiro comando e guarda em *saida. Devolve false se estava vazia. */
bool desenfileirar(FilaComandos *f, Comando *saida);

/* Tira o último comando colocado (o botão "remover último").
   Devolve false se estava vazia. */
bool removerUltimo(FilaComandos *f);

/* Devolve o comando na posição i sem tirar (0 = primeiro da fila).
   Serve para desenhar a fila na tela. i precisa ser menor que quantidade. */
Comando comandoNaPosicao(const FilaComandos *f, int i);

#endif

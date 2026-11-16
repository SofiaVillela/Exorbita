/* sem_ar_logica.h: regras do planeta sem ar, sem raylib.

   Não há ar: o oxigênio do traje cai o tempo todo, mais depressa para
   quem corre ou fica dentro de um jato de gás. Cilindros recarregam.
   Os itens vão para a mochila, que é uma pilha (pilha_mochila.h).
   O portão da estação abre em duas etapas, e cada uma pede um item no
   topo da mochila: primeiro a célula de energia liga o terminal, depois
   a placa de acesso destrava. Como só o topo pode ser usado, a ordem
   em que os itens são guardados importa. */

#ifndef SEM_AR_LOGICA_H
#define SEM_AR_LOGICA_H

#include <stdbool.h>
#include "pilha_mochila.h"

typedef enum {
    PORTAO_SEM_ENERGIA,   /* terminal apagado: pede a célula de energia */
    PORTAO_PEDE_PLACA,    /* terminal ligado: pede a placa de acesso */
    PORTAO_ABERTO
} EstadoPortao;

typedef enum {
    USO_MOCHILA_VAZIA,    /* não havia nada para usar */
    USO_ITEM_ERRADO,      /* o topo não é o item pedido */
    USO_LIGOU,            /* a célula ligou o terminal */
    USO_ABRIU,            /* a placa abriu o portão */
    USO_JA_ABERTO
} ResultadoPortao;

typedef enum {
    PEGAR_GUARDOU,        /* foi para o topo da mochila */
    PEGAR_RESPIROU,       /* era um cilindro: recarregou o oxigênio */
    PEGAR_CHEIA           /* não coube: o item fica no chão */
} ResultadoPegar;

/* Respiradouro: fica quieto, avisa (borbulha) e solta o jato, em ciclo. */
typedef enum {
    JATO_QUIETO,
    JATO_AVISO,
    JATO_ATIVO
} FaseJato;

/* Item que o portão pede neste estado. Aberto não pede nada, e devolve
   TOTAL_ITENS. */
TipoItem itemPedidoPortao(EstadoPortao estado);

/* Usa o topo da mochila no portão. Se for o item pedido, ele é gasto
   (sai da pilha) e o portão passa para o próximo estado. */
ResultadoPortao usarTopoNoPortao(EstadoPortao *estado, PilhaMochila *mochila);

/* Encostou em um item no chão: o cilindro é respirado na hora; o resto
   vai para o topo da mochila, se couber. */
ResultadoPegar pegarItem(PilhaMochila *mochila, TipoItem item, float *oxigenio);

/* Oxigênio gasto por segundo. */
float consumoOxigenio(bool correndo, bool noJato);
/* Oxigênio depois de dt segundos; nunca fica abaixo de 0. */
float gastarOxigenio(float oxigenio, float dt, bool correndo, bool noJato);
/* Oxigênio depois de um cilindro; nunca passa de OXIGENIO_MAXIMO. */
float recarregarOxigenio(float oxigenio);

/* Fase do respiradouro no instante tempo. A defasagem faz cada um
   soltar o jato num momento diferente. */
FaseJato faseJato(float tempo, float defasagem);

/* O cilindro escondido só aparece para tripulantes curiosos. */
bool itemOcultoVisivel(int curiosidade);

#endif

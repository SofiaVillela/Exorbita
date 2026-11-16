/* teste_sem_ar.c: testa a mochila em pilha e as regras do planeta sem
   ar, sem abrir o jogo.

   Compilar e rodar, a partir da pasta do projeto:
     gcc -std=c99 -Wall -Wextra -Iinclude src/pilha_mochila.c src/sem_ar_logica.c tests/teste_sem_ar.c -lm -o build/teste_sem_ar
     ./build/teste_sem_ar
*/

#include <stdio.h>
#include "constantes.h"
#include "pilha_mochila.h"
#include "sem_ar_logica.h"

static int falhas = 0;

static void conferir(bool condicao, const char *mensagem)
{
    if (condicao) {
        printf("ok: %s\n", mensagem);
    } else {
        printf("FALHOU: %s\n", mensagem);
        falhas++;
    }
}

static void testarPilha(void)
{
    PilhaMochila p;
    TipoItem item = TOTAL_ITENS;
    iniciarPilha(&p);
    conferir(pilhaVazia(&p) && !pilhaCheia(&p), "comeca vazia");
    conferir(!desempilhar(&p, &item) && !topoDaPilha(&p, &item), "vazia nao tem topo nem desempilha");

    conferir(empilhar(&p, ITEM_PLACA) && empilhar(&p, ITEM_SUCATA) && empilhar(&p, ITEM_CELULA),
             "empilha tres itens");
    conferir(topoDaPilha(&p, &item) && item == ITEM_CELULA, "o topo e o ultimo que entrou");
    conferir(p.quantidade == 3, "olhar o topo nao tira o item");
    conferir(itemNaPosicao(&p, 0) == ITEM_CELULA && itemNaPosicao(&p, 2) == ITEM_PLACA,
             "posicao 0 e o topo, a ultima e o fundo");
    conferir(pilhaContem(&p, ITEM_PLACA) && !pilhaContem(&p, ITEM_MAPA), "procura um item na pilha");

    conferir(desempilhar(&p, &item) && item == ITEM_CELULA, "sai primeiro o ultimo que entrou");
    conferir(desempilhar(&p, &item) && item == ITEM_SUCATA, "depois o do meio");
    conferir(desempilhar(&p, &item) && item == ITEM_PLACA, "por ultimo o primeiro que entrou");
    conferir(pilhaVazia(&p), "volta a ficar vazia");

    for (int i = 0; i < MOCHILA_CAPACIDADE; i++) {
        empilhar(&p, ITEM_SUCATA);
    }
    conferir(pilhaCheia(&p) && !empilhar(&p, ITEM_MAPA), "cheia nao aceita mais");
    conferir(p.quantidade == MOCHILA_CAPACIDADE, "a quantidade nao passa da capacidade");
    conferir(desempilhar(&p, NULL) && p.quantidade == MOCHILA_CAPACIDADE - 1,
             "desempilha sem pedir o item de volta");
}

static void testarPortao(void)
{
    PilhaMochila p;
    EstadoPortao portao = PORTAO_SEM_ENERGIA;
    iniciarPilha(&p);
    conferir(itemPedidoPortao(portao) == ITEM_CELULA, "sem energia, pede a celula");
    conferir(usarTopoNoPortao(&portao, &p) == USO_MOCHILA_VAZIA, "mochila vazia nao abre nada");

    /* Ordem errada: a placa por cima da celula. */
    empilhar(&p, ITEM_CELULA);
    empilhar(&p, ITEM_PLACA);
    conferir(usarTopoNoPortao(&portao, &p) == USO_ITEM_ERRADO, "placa no topo: o terminal quer a celula");
    conferir(portao == PORTAO_SEM_ENERGIA && p.quantidade == 2, "item errado nao gasta nada");

    /* Largar o topo resolve: agora a celula esta em cima. */
    desempilhar(&p, NULL);
    conferir(usarTopoNoPortao(&portao, &p) == USO_LIGOU, "celula no topo liga o terminal");
    conferir(portao == PORTAO_PEDE_PLACA && pilhaVazia(&p), "a celula foi gasta");
    conferir(itemPedidoPortao(portao) == ITEM_PLACA, "ligado, pede a placa");

    empilhar(&p, ITEM_PLACA);
    empilhar(&p, ITEM_SUCATA);
    conferir(usarTopoNoPortao(&portao, &p) == USO_ITEM_ERRADO, "sucata por cima da placa atrapalha");
    desempilhar(&p, NULL);
    conferir(usarTopoNoPortao(&portao, &p) == USO_ABRIU && portao == PORTAO_ABERTO, "placa abre o portao");
    conferir(usarTopoNoPortao(&portao, &p) == USO_JA_ABERTO, "aberto nao pede mais nada");
    conferir(itemPedidoPortao(portao) == TOTAL_ITENS, "aberto: nenhum item pedido");

    /* Ordem certa de primeira: placa embaixo, celula em cima. */
    portao = PORTAO_SEM_ENERGIA;
    iniciarPilha(&p);
    empilhar(&p, ITEM_PLACA);
    empilhar(&p, ITEM_CELULA);
    conferir(usarTopoNoPortao(&portao, &p) == USO_LIGOU && usarTopoNoPortao(&portao, &p) == USO_ABRIU,
             "placa primeiro e celula depois: abre sem largar nada");
}

static void testarOxigenio(void)
{
    conferir(consumoOxigenio(false, false) == SEM_AR_CONSUMO_ANDANDO, "andando gasta o consumo normal");
    conferir(consumoOxigenio(true, false) > consumoOxigenio(false, false), "correr gasta mais");
    conferir(consumoOxigenio(false, true) == SEM_AR_CONSUMO_ANDANDO + SEM_AR_CONSUMO_JATO,
             "o jato soma ao consumo");
    conferir(gastarOxigenio(100, 2, false, false) == 100 - 2 * SEM_AR_CONSUMO_ANDANDO, "gasta conforme o tempo");
    conferir(gastarOxigenio(1, 10, true, true) == 0, "nunca fica negativo");
    conferir(gastarOxigenio(50, 0, false, false) == 50, "sem tempo nada muda");
    conferir(recarregarOxigenio(40) == 40 + SEM_AR_RECARGA_CILINDRO, "cilindro recarrega");
    conferir(recarregarOxigenio(90) == OXIGENIO_MAXIMO, "a recarga nao passa do maximo");

    PilhaMochila p;
    float oxigenio = 30;
    iniciarPilha(&p);
    conferir(pegarItem(&p, ITEM_CILINDRO, &oxigenio) == PEGAR_RESPIROU && pilhaVazia(&p),
             "cilindro e usado na hora, sem ocupar a mochila");
    conferir(oxigenio == 30 + SEM_AR_RECARGA_CILINDRO, "e recarrega o oxigenio");
    conferir(pegarItem(&p, ITEM_PLACA, &oxigenio) == PEGAR_GUARDOU && p.quantidade == 1,
             "outro item vai para a mochila");
    for (int i = 1; i < MOCHILA_CAPACIDADE; i++) {
        pegarItem(&p, ITEM_SUCATA, &oxigenio);
    }
    conferir(pegarItem(&p, ITEM_MAPA, &oxigenio) == PEGAR_CHEIA, "mochila cheia recusa o item");
    conferir(pegarItem(&p, ITEM_CILINDRO, &oxigenio) == PEGAR_RESPIROU, "mas o cilindro ainda pode ser usado");
}

static void testarJato(void)
{
    float ciclo = SEM_AR_JATO_QUIETO + SEM_AR_JATO_AVISO + SEM_AR_JATO_ATIVO;
    conferir(faseJato(0, 0) == JATO_QUIETO, "comeca quieto");
    conferir(faseJato(SEM_AR_JATO_QUIETO + 0.1f, 0) == JATO_AVISO, "avisa antes do jato");
    conferir(faseJato(SEM_AR_JATO_QUIETO + SEM_AR_JATO_AVISO + 0.1f, 0) == JATO_ATIVO, "depois solta o jato");
    conferir(faseJato(ciclo + 0.1f, 0) == JATO_QUIETO, "e o ciclo recomeca");
    conferir(faseJato(0, SEM_AR_JATO_QUIETO + 0.1f) == JATO_AVISO, "a defasagem adianta o ciclo");
    conferir(itemOcultoVisivel(SEM_AR_CURIOSIDADE_OCULTO) && !itemOcultoVisivel(SEM_AR_CURIOSIDADE_OCULTO - 1),
             "item escondido so aparece para curiosos");
}

int main(void)
{
    testarPilha();
    testarPortao();
    testarOxigenio();
    testarJato();
    printf("\n%d falha(s)\n", falhas);
    return falhas != 0;
}

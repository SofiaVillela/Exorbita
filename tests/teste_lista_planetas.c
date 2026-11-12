/* teste_lista_planetas.c: testa a lista circular duplamente encadeada
   do painel de planetas sem abrir o jogo.

   Compilar e rodar, a partir da pasta do projeto:
     gcc -std=c99 -Wall -Wextra -Iinclude src/lista_planetas.c tests/teste_lista_planetas.c -o build/teste_lista_planetas
     ./build/teste_lista_planetas
*/

#include <stdio.h>
#include <string.h>
#include "lista_planetas.h"

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

static void testarListaVazia(void)
{
    Planeta *inicio = NULL;
    liberarPlanetas(&inicio);   /* não pode quebrar com a lista vazia */
    conferir(inicio == NULL, "liberar lista vazia nao quebra");
    conferir(!todosLivresVisitados(inicio), "lista vazia nao conta como tudo visitado");
}

static void testarUmPlaneta(void)
{
    Planeta *inicio = NULL;
    inserirPlaneta(&inicio, "Robo", "robo.png", 15, TELA_ROBO, false);
    conferir(inicio != NULL, "inserir na lista vazia muda o inicio");
    conferir(inicio->prox == inicio && inicio->ant == inicio,
             "um planeta sozinho aponta para ele mesmo nos dois sentidos");
    conferir(!inicio->visitado, "planeta novo comeca nao visitado");
    liberarPlanetas(&inicio);
    conferir(inicio == NULL, "liberar deixa o inicio em NULL");
}

static void testarTresPlanetas(void)
{
    Planeta *inicio = NULL;
    inserirPlaneta(&inicio, "A", "a.png", 10, TELA_ROBO, false);
    inserirPlaneta(&inicio, "B", "b.png", 20, TELA_GIGANTE, false);
    inserirPlaneta(&inicio, "C", "c.png", 30, TELA_NAVE, true);

    conferir(strcmp(inicio->nome, "A") == 0, "o inicio continua sendo o primeiro inserido");
    conferir(strcmp(inicio->ant->nome, "C") == 0, "o ant do inicio e o ultimo inserido");

    /* Para frente (prox): A B C A. */
    Planeta *p = inicio;
    conferir(strcmp(p->prox->nome, "B") == 0 &&
             strcmp(p->prox->prox->nome, "C") == 0 &&
             p->prox->prox->prox == inicio, "prox da a volta: A B C A");

    /* Para trás (ant): A C B A. */
    conferir(strcmp(p->ant->nome, "C") == 0 &&
             strcmp(p->ant->ant->nome, "B") == 0 &&
             p->ant->ant->ant == inicio, "ant da a volta: A C B A");

    /* Via Láctea: C está bloqueado e não conta. */
    conferir(!todosLivresVisitados(inicio), "no comeco nem todos foram visitados");
    inicio->visitado = true;
    conferir(!todosLivresVisitados(inicio), "falta B: ainda nao");
    inicio->prox->visitado = true;
    conferir(todosLivresVisitados(inicio), "A e B visitados: o bloqueado C nao conta");

    desbloquearPlanetas(inicio);
    conferir(!inicio->ant->bloqueado, "desbloquear libera C");
    conferir(!todosLivresVisitados(inicio), "depois de liberado, C conta e falta visitar");

    liberarPlanetas(&inicio);
    conferir(inicio == NULL, "liberar a lista de 3 deixa o inicio em NULL");
}

int main(void)
{
    testarListaVazia();
    testarUmPlaneta();
    testarTresPlanetas();

    printf("\n%d falha(s)\n", falhas);
    if (falhas > 0) {
        return 1;
    }
    return 0;
}

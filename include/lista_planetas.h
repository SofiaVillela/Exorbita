#ifndef LISTA_PLANETAS_H
#define LISTA_PLANETAS_H

#include <stdbool.h>
#include "telas.h"
 
typedef struct Planeta{
    const char *nome;
    const char *arquivo;
    bool bloqueado;
    bool visitado;
    Tela destino;
    int custo;
    /* Planeta que precisa ser visitado para este abrir. NULL num
       planeta bloqueado = destino final (a Via Láctea): não abre por
       visita, só quando quem chama atualizarDesbloqueios mandar. */
    struct Planeta *requisito;
    struct Planeta *prox;
    struct Planeta *ant;
    
}Planeta;

/* Insere um planeta no fim da lista circular. Custa O(1): o fim é o
   ant do início, então não precisa percorrer a lista.
   Recebe Planeta ** porque, com a lista vazia, precisa mudar o início
   de quem chamou. Devolve false se faltar memória. */
bool inserirPlaneta(Planeta **inicio, const char *nome, const char *arquivo,
                    int custo, Tela destino, bool bloqueado);

/* Libera todos os nós com free e deixa o início em NULL.
   Recebe Planeta ** para zerar o ponteiro de quem chamou: assim ninguém
   fica apontando para memória já liberada. Lista vazia não faz nada. */
void liberarPlanetas(Planeta **inicio);

/* Devolve true se todos os planetas livres (não bloqueados) já foram
   visitados. Os bloqueados não contam. Lista vazia devolve false.
   Usada para liberar a Via Láctea. */
bool todosLivresVisitados(const Planeta *inicio);

/* Tira o bloqueio de todos os planetas da lista. */
void desbloquearPlanetas(Planeta *inicio);

/* Abre os planetas bloqueados cujo requisito já foi visitado. Um
   bloqueado sem requisito (a Via Láctea) abre só se abrirFinal for
   true: no jogo, quando os pedaços do mapa estelar estão completos.
   Devolve quantos planetas abriram agora. */
int atualizarDesbloqueios(Planeta *inicio, bool abrirFinal);

/* Devolve o primeiro planeta livre (não bloqueado) que ainda não foi
   visitado, andando pela lista a partir do início. NULL se não houver.
   Usada pelo desejo "Mostre o caminho" do planeta do Ser. */
Planeta *primeiroNaoVisitado(Planeta *inicio);

#endif

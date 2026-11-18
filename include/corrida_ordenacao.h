#ifndef CORRIDA_ORDENACAO_H
#define CORRIDA_ORDENACAO_H

#include <stdbool.h>

#define ORDENACAO_MAX_VALORES 8

typedef enum {
    ORDENACAO_BOLHA,
    ORDENACAO_INSERCAO
} AlgoritmoOrdenacao;

typedef struct {
    int valores[ORDENACAO_MAX_VALORES];
    int quantidade;
    AlgoritmoOrdenacao algoritmo;
    int i;
    int j;
    int limite;
    int chave;
    bool temChave;
    bool houveTroca;
    bool terminou;
    bool moveuNestePasso;
    int comparaA;
    int comparaB;
    int comparacoes;
    int movimentos;
    int trocas;
} Ordenador;

/* Inicializa uma das pistas da corrida com a mesma sequência embaralhada. */
bool iniciarOrdenador(Ordenador *ordenador, AlgoritmoOrdenacao algoritmo,
                      const int valores[], int quantidade);

/* Executa uma operação visível do algoritmo: comparação e, quando precisa,
   uma troca/deslocamento. Devolve false se a ordenação já terminou. */
bool passoOrdenador(Ordenador *ordenador);

bool valoresOrdenados(const int valores[], int quantidade);

#endif

#ifndef TRIPULACAO_H 
#define TRIPULACAO_H

#include <stdbool.h>

typedef struct {
    int egoismo;
    int inteligencia;
    int carisma; 
    int impulsividade;
    int curiosidade;
    int sorte;
}Tracos;

typedef struct {
    const char *nome;
    const char *arquivo;
    int idade;
    int oxigenio;
    bool vivo;
    Tracos tracos;
}Tripulante;

void iniciarTripulacao(Tripulante tripulacao[]);

/* Frase curta sobre o traço mais forte do tripulante (empate: o
   primeiro na ordem do struct). Usada no memorial de quem não voltou. */
const char *destaqueTripulante(const Tracos *tracos);

/* Quantos tripulantes estão vivos. */
int tripulantesVivos(const Tripulante tripulacao[]);

#endif
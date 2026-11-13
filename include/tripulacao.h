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
    int mental;
    int oxigenio;
    bool vivo;
    Tracos tracos;
}Tripulante;

void iniciarTripulacao(Tripulante tripulacao[]);

#endif
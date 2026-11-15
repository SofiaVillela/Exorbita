#ifndef PERSONAGEM_H 
#define PERSONAGEM_H

#include "raylib.h"

typedef enum {
    DIRECAO_FRENTE,
    DIRECAO_TRAS,
    DIRECAO_ESQUERDA,
    DIRECAO_DIREITA,
}Direcao;

typedef struct 
{
   Texture2D folha;     /* a imagem com todos os quadros */
   bool andando;
   Vector2 posicaoPes;
   Direcao direcao;
   int quadro;
   float acumulado;     /* tempo desde a última troca de quadro */
}Personagem;

void iniciarPersonagem(Personagem *p, const char *caminho, Vector2 pes);
void desenharPersonagem(const Personagem *p);
void encerrarPersonagem(Personagem *p);


#endif
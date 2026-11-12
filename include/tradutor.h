/* tradutor.h: tradutor com análise de voz, montado pelo robô cientista.

   Sem tradutor, a fala do alien vira símbolos. Com tradutor, a fala aparece
   em português e vem junto uma leitura do tom de voz, que pode indicar blefe.
   A leitura erra às vezes: quanto mais inteligente quem desceu, mais ela acerta.

   Divisão de responsabilidades:
     - o jogo de cartas decide se o alien está blefando e preenche FalaAlien;
     - este módulo só converte a fala e sorteia se a leitura acerta.

   Este módulo não usa a raylib: dá para compilar e testar sozinho
   (ver tests/teste_tradutor.c). */

#ifndef TRADUTOR_H
#define TRADUTOR_H

#include <stdbool.h>

#define TRADUTOR_TAM_TEXTO 128   /* bytes, contando o '\0' do fim */

typedef struct {
    const char *fala;   /* o que o alien disse, em UTF-8 */
    bool blefando;      /* a verdade: só o jogo de cartas sabe */
} FalaAlien;

typedef enum {
    LEITURA_NENHUMA,    /* sem tradutor: não há leitura de tom */
    LEITURA_CALMA,      /* o tradutor acha que ele está sendo sincero */
    LEITURA_SUSPEITA    /* o tradutor acha que é um possível blefe */
} Leitura;

typedef struct {
    char texto[TRADUTOR_TAM_TEXTO];   /* UTF-8, sempre termina em '\0' */
    Leitura leitura;
} FalaTraduzida;

/* Converte uma fala do alien.
     temTradutor   o jogador já tem o tradutor?
     inteligencia  traço de quem desceu (0 a 100); define a precisão
     sorteio       número aleatório de 0 a 1, vindo de fora (ex.: GetRandomValue).
                   Vem de fora para a função dar sempre o mesmo resultado nos testes. */
void traduzir(const FalaAlien *f, bool temTradutor, int inteligencia,
              float sorteio, FalaTraduzida *saida);

/* Chance (0 a 1) de a leitura acertar, para essa inteligência. */
float precisaoTradutor(int inteligencia);

/* Frase curta para mostrar embaixo da fala. "" para LEITURA_NENHUMA.
   Só usa caracteres que a fonte do jogo tem (32-126 e 160-255). */
const char *textoLeitura(Leitura l);

#endif

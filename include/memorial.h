/* memorial.h: tela de despedida de um tripulante que não voltou.

   Fundo de espaço, o traje à deriva, o sinal do coração que para e as
   informações aparecendo letra por letra. Não depende de um planeta
   específico: quem chama passa os textos. */

#ifndef MEMORIAL_H
#define MEMORIAL_H

#include <stdbool.h>
#include "raylib.h"
#include "animacao.h"

#define MEMORIAL_MAX_LINHAS  4
#define MEMORIAL_TAM_LINHA   96

typedef struct {
    const char *nome;
    int idade;
    const char *causa;       /* uma frase: o que aconteceu */
    const char *destaque;    /* frase sobre o traço mais forte (pode ser NULL) */
    /* Linhas extras, na ordem em que aparecem (tempo, itens perdidos...). */
    char linhas[MEMORIAL_MAX_LINHAS][MEMORIAL_TAM_LINHA];
    int totalLinhas;
} DadosMemorial;

/* Começa a tela do zero. Os textos são copiados. */
void abrirMemorial(const DadosMemorial *dados);

/* Avança as animações. Devolve true quando o jogador confirma a saída.
   O primeiro Enter/E só termina de mostrar o texto. */
bool atualizarMemorial(float dt);

/* folhaTraje: folha do astronauta (o primeiro quadro é desenhado).
   planeta: planeta girando ao fundo (pode ser NULL). */
void desenharMemorial(Texture2D folhaTraje, const Animacao *planeta);

#endif

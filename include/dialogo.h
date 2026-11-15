/* dialogo.h: caixa de diálogo padrão do jogo.

   Toda fala aparece na mesma caixa, presa embaixo da tela: retrato do
   rosto à esquerda, nome em verde-água, texto e uma seta que pisca
   quando dá para avançar. A caixa cresce para cima conforme o texto. */

#ifndef DIALOGO_H
#define DIALOGO_H

#include <stdbool.h>
#include "raylib.h"

/* Quem fala: nome e a parte da folha que mostra o rosto. */
typedef struct {
    const char *nome;
    Texture2D folha;
    Rectangle rosto;    /* recorte do rosto dentro da folha */
} Falante;

/* Caixa com uma fala.
   rodape: dica de teclas no canto de baixo (pode ser NULL).
   seta: desenha a seta piscando (há mais para ler). */
void desenharFala(const Falante *falante, const char *texto,
                  const char *rodape, bool seta);

/* Caixa com uma lista de opções (uma por linha).
   detalhe: explicação da opção escolhida, mostrada abaixo da lista. */
void desenharEscolhaDialogo(const Falante *falante, const char *pergunta,
                            const char *opcoes[], int total, int escolhida,
                            const char *detalhe, const char *rodape);

#endif

#ifndef TELA_NAVE_H
#define TELA_NAVE_H

#include "telas.h"

void iniciarTelaNave(void);

/* Chamar ao voltar de um planeta: tira os mortos da sala e, se quem
   desceu não voltou, passa o controle para outro tripulante vivo. */
void entrarTelaNave(void);
Tela atualizarTelaNave(void);
void desenharTelaNave(void);
void encerrarTelaNave(void);

#endif
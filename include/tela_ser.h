/* tela_ser.h: planeta do Ser dos desejos. */

#ifndef TELA_SER_H
#define TELA_SER_H

#include "telas.h"

/* Carrega o cenário e os sprites. Chamar uma vez, depois do InitWindow. */
void iniciarTelaSer(void);

/* Chamar a cada pouso: conta a visita e sorteia se o Ser está lá. */
void entrarTelaSer(void);

Tela atualizarTelaSer(void);
void desenharTelaSer(void);

/* Libera o que foi carregado. Chamar antes do CloseWindow. */
void encerrarTelaSer(void);

#endif

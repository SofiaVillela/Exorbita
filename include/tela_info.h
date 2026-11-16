/* tela_info.h: telas de texto do menu (Regras e Créditos). */

#ifndef TELA_INFO_H
#define TELA_INFO_H

#include "telas.h"

/* As duas voltam ao menu com Esc ou Enter. */
Tela atualizarTelaRegras(void);
void desenharTelaRegras(void);

Tela atualizarTelaCreditos(void);
void desenharTelaCreditos(void);

#endif

/* fundo.h: fundo do espaço em três camadas que rolam para a esquerda. */

#ifndef FUNDO_H
#define FUNDO_H

/* Carrega as três imagens. Chamar uma vez, depois do InitWindow. */
void iniciarFundo(void);

/* Move as camadas. Chamar uma vez por quadro, na lógica. */
void atualizarFundo(void);

/* Desenha as camadas. Chamar antes do resto da tela, para ficarem no fundo. */
void desenharFundo(void);

/* Libera as imagens. Chamar antes do CloseWindow. */
void encerrarFundo(void);

#endif

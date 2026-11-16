/* musica.h: música de fundo do jogo e o ícone que mostra se ela toca.

   A música (assets/musica/tema.qoa) toca em laço o jogo inteiro. A
   tecla M silencia e volta a tocar, em qualquer tela. No canto de
   baixo à direita fica uma nota musical com barrinhas que sobem e
   descem conforme o volume do que está tocando naquele instante.
   A música é feita por ferramentas/gerar_musica.py. */

#ifndef MUSICA_H
#define MUSICA_H

/* Chamar depois do InitWindow. Se o arquivo ou o áudio faltar, o jogo
   segue sem música (o ícone mostra só a nota apagada). */
void iniciarMusica(void);

/* Uma vez por quadro: alimenta o áudio e trata a tecla M. */
void atualizarMusica(void);

/* Desenha o ícone e o aviso da tecla M. Chamar por último, por cima
   da tela do jogo. */
void desenharMusica(void);

/* Abaixa ou devolve o volume da música, aos poucos: 0 cala, 1 é o
   normal. Usado pelo fliperama, que tem a música dele. */
void definirVolumeMusica(float fator);

void encerrarMusica(void);

#endif

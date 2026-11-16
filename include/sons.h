/* sons.h: efeitos sonoros, sons de ambiente e passos.

   Três coisas tocam ao mesmo tempo que a música (musica.c):
     - efeitos curtos: tocarSom(SOM_...), em qualquer tela;
     - ambiente: um som contínuo para cada lugar, que troca aos poucos
       quando o lugar muda;
     - passos: um som para cada tipo de piso, tocado enquanto o
       tripulante anda.

   A tecla M silencia só a música; os sons continuam.
   Os arquivos ficam em assets/sons/ e são feitos por
   ferramentas/gerar_sons.py. Se algum faltar, o jogo segue sem ele. */

#ifndef SONS_H
#define SONS_H

#include <stdbool.h>
#include "telas.h"

typedef enum {
    /* interface */
    SOM_UI_MOVER, SOM_UI_CONFIRMAR, SOM_UI_VOLTAR, SOM_NEGADO, SOM_TERMINAL,
    SOM_ITEM, SOM_VITORIA, SOM_DERROTA, SOM_PERDA, SOM_FALA_ALIEN,
    /* nave, portas e viagens */
    SOM_DESCIDA, SOM_RETORNO, SOM_PORTA_BAR, SOM_PORTAO,
    /* passos (tocados por tocarPasso) */
    SOM_PASSO_METAL, SOM_PASSO_POEIRA, SOM_PASSO_GRAMA, SOM_PASSO_ASFALTO,
    SOM_PASSO_MADEIRA, SOM_PASSO_SANTUARIO,
    /* planeta do gigante */
    SOM_PLANTA, SOM_HABITANTE, SOM_GIGANTE_REAGE, SOM_GIGANTE_PEGA, SOM_GIGANTE_ACALMA,
    /* planeta sem ar */
    SOM_OXIGENIO, SOM_VAZAMENTO, SOM_ALARME, SOM_LARGAR,
    /* planeta do Ser */
    SOM_SER_APARECE, SOM_DESEJO,
    /* laboratório do robô */
    SOM_ROBO_COMANDO, SOM_ROBO_APAGA, SOM_ROBO_PASSO,
    /* cartas */
    SOM_CARTA_MEXE, SOM_CARTA_JOGA, SOM_RODADA_GANHA, SOM_RODADA_PERDE,
    /* fliperama */
    SOM_FLIP_TIRO, SOM_FLIP_EXPLOSAO, SOM_FLIP_ATINGIDO, SOM_FLIP_ONDA, SOM_FLIP_FIM, SOM_FLIP_INICIO,
    TOTAL_SONS
} Som;

typedef enum {
    AMBIENTE_NENHUM,      /* menu, introdução e final: só a música */
    AMBIENTE_NAVE, AMBIENTE_CIDADE, AMBIENTE_BAR, AMBIENTE_SEM_AR,
    AMBIENTE_GIGANTE, AMBIENTE_SER, AMBIENTE_ROBO,
    AMBIENTE_FLIPERAMA,   /* a música da máquina; a música do jogo se cala */
    TOTAL_AMBIENTES
} Ambiente;

typedef enum {
    PISO_METAL, PISO_POEIRA, PISO_GRAMA, PISO_ASFALTO, PISO_MADEIRA, PISO_SANTUARIO, TOTAL_PISOS
} Piso;

/* Chamar depois de iniciarMusica (que abre o dispositivo de áudio). */
void iniciarSons(void);
/* Uma vez por quadro: alimenta os ambientes e faz as trocas suaves. */
void atualizarSons(void);
void encerrarSons(void);

/* Toca um efeito. Se ele já estiver tocando, recomeça. */
void tocarSom(Som som);
/* Igual, mas com o tom um pouco diferente a cada vez (variacao de
   0 a 1; 0.1 = até 10% mais grave ou mais agudo). Para sons que se
   repetem muito, como tiros. */
void tocarSomVariado(Som som, float variacao);

/* O jogo mudou de tela: troca ambiente e piso, e toca a descida do
   módulo (nave -> planeta) ou o retorno (planeta -> nave). */
void sonsMudouDeTela(Tela anterior, Tela nova);

/* Para lugares dentro de uma mesma tela (rua e bar, por exemplo). */
void definirAmbiente(Ambiente ambiente);
void definirPiso(Piso piso);

/* Um passo no piso atual. correndo: um pouco mais forte. */
void tocarPasso(bool correndo);

/* Tensão do gigante, de 0 (calmo) a 1 (furioso): um zumbido com
   batidas que fica mais forte e mais rápido. Zera ao mudar de tela. */
void definirTensao(float nivel);

#endif

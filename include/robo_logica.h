/* robo_logica.h: regras do laboratório do robô cientista, sem desenho.

   O robô está preso numa grade. O jogador monta uma fila de comandos
   (setas) e manda executar. O robô cumpre um comando por vez, na ordem
   em que entraram, e precisa chegar à saída sem bater na parede nem
   cair na lava.

   Este arquivo só guarda as regras. Desenhar e ler o teclado ficam na
   tela (a fazer). Assim as regras podem ser testadas sem abrir o jogo
   (tests/teste_robo.c).

   Uso pela tela:
     iniciarLaboratorio(&lab, MAPA_LAB_PADRAO);
     adicionarComando(&lab, CMD_DIREITA);      a cada seta apertada
     executarPrograma(&lab);                   no Enter
     passoDoRobo(&lab);                        a cada ROBO_TEMPO_PASSO segundos
*/

#ifndef ROBO_LOGICA_H
#define ROBO_LOGICA_H

#include <stdbool.h>
#include "fila_comandos.h"

#define LAB_COLUNAS 7
#define LAB_LINHAS  5

typedef enum { CASA_LIVRE, CASA_PAREDE, CASA_LAVA, CASA_SAIDA } Casa;

typedef enum {
    ROBO_MONTANDO,     /* jogador montando a fila */
    ROBO_EXECUTANDO,   /* robô cumprindo os comandos */
    ROBO_FALHOU,       /* bateu, caiu ou ficou sem comandos; o robô fica parado
                          onde falhou para a tela mostrar */
    ROBO_VENCEU        /* chegou à saída */
} FaseRobo;

typedef enum {
    PASSO_NADA,           /* não estava executando */
    PASSO_ANDOU,          /* andou uma casa */
    PASSO_PAREDE,         /* bateu na parede ou na borda (não saiu do lugar) */
    PASSO_LAVA,           /* caiu na lava */
    PASSO_SEM_COMANDOS,   /* a fila acabou antes da saída */
    PASSO_SAIDA           /* chegou à saída */
} ResultadoPasso;

typedef struct {
    Casa mapa[LAB_LINHAS][LAB_COLUNAS];
    int inicioX, inicioY;   /* onde o robô começa */
    int x, y;               /* onde o robô está agora */
    FilaComandos fila;
    FaseRobo fase;
    int tentativas;         /* quantas vezes o jogador mandou executar */
} Laboratorio;

/* Mapa do protótipo. Cada texto é uma linha da grade:
     S = início   . = livre   # = parede   L = lava   E = saída */
extern const char *const MAPA_LAB_PADRAO[LAB_LINHAS];

/* Os circuitos do desafio, do mais simples ao mais difícil. O último é
   o MAPA_LAB_PADRAO. O tradutor só sai depois de vencer todos. */
#define LAB_TOTAL_CIRCUITOS 3
extern const char *const *const CIRCUITOS_LAB[LAB_TOTAL_CIRCUITOS];

/* Monta o laboratório a partir de um mapa em texto. */
void iniciarLaboratorio(Laboratorio *lab, const char *const linhas[LAB_LINHAS]);

/* Põe um comando no fim da fila. Devolve false se a fila está cheia
   ou se o robô está executando ou já venceu. */
bool adicionarComando(Laboratorio *lab, Comando c);

/* Tira o último comando colocado. Só vale enquanto monta. */
bool removerUltimoComando(Laboratorio *lab);

/* Esvazia a fila. Só vale enquanto monta. */
void limparComandos(Laboratorio *lab);

/* Começa a executar: o robô volta ao início e a tentativa é contada.
   Devolve false se a fila está vazia ou se não é hora de executar. */
bool executarPrograma(Laboratorio *lab);

/* Cumpre o próximo comando da fila. Chamar uma vez por passo, enquanto
   a fase for ROBO_EXECUTANDO. Quando falha, a fila é limpa. */
ResultadoPasso passoDoRobo(Laboratorio *lab);

/* Depois de uma falha: põe o robô de volta no início para montar de novo.
   (adicionarComando e executarPrograma já fazem isso sozinhos.) */
void voltarAoInicio(Laboratorio *lab);

/* Acha o caminho mais curto do início até a saída (busca em largura).
   Guarda até "max" comandos em saida e devolve o tamanho do caminho
   inteiro, ou -1 se não há caminho. Usos: dica para tripulante
   inteligente e conferir se um mapa novo tem solução que cabe na fila. */
int caminhoMaisCurto(const Laboratorio *lab, Comando *saida, int max);

#endif

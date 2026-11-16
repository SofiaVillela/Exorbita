/* teste_fliperama.c: testa as regras do jogo de naves do fliperama,
   sem abrir o jogo.

   Compilar e rodar, a partir da pasta do projeto:
     gcc -std=c99 -Wall -Wextra -Iinclude src/fliperama_logica.c tests/teste_fliperama.c -lm -o build/teste_fliperama
     ./build/teste_fliperama
*/

#include <math.h>
#include <stdio.h>
#include "fliperama_logica.h"

static int falhas = 0;

static void conferir(bool ok, const char *descricao)
{
    printf("%s %s\n", ok ? "ok   " : "FALHA", descricao);
    if (!ok) falhas++;
}

/* Deixa só o inimigo i vivo, parado na formação. */
static void soUmInimigo(Fliperama *f, int i)
{
    for (int k = 0; k < f->totalInimigos; k++) f->inimigos[k].vivo = k == i;
    f->proximoMergulho = 1000;
}

int main(void)
{
    Fliperama f;

    iniciarFliperama(&f, 42);
    conferir(f.vidas == FLIP_VIDAS && f.pontos == 0 && f.onda == 1, "partida começa com 3 vidas, 0 pontos, onda 1");
    conferir(f.totalInimigos == 18 && inimigosVivos(&f) == 18, "primeira onda tem 18 naves (4 + 6 + 8)");

    /* Para testar o movimento, ninguém atrapalha. */
    soUmInimigo(&f, 0);
    f.invencivel = 1000;

    /* Andar: limitado à parte de baixo da tela. */
    for (int i = 0; i < 200; i++) atualizarFliperama(&f, 0.05f, 0, -1, false);
    conferir(f.jogadorY >= FLIP_TOPO_JOGADOR, "o jogador não sobe além da faixa dele");
    for (int i = 0; i < 200; i++) atualizarFliperama(&f, 0.05f, -1, 0, false);
    conferir(f.jogadorX >= 14 && f.jogadorX < 20, "o jogador para na borda esquerda");

    /* Tiro: segurar a tecla respeita a recarga. */
    iniciarFliperama(&f, 7);
    soUmInimigo(&f, 0);
    atualizarFliperama(&f, 0.01f, 0, 0, true);
    atualizarFliperama(&f, 0.01f, 0, 0, true);
    int ativos = 0;
    for (int i = 0; i < FLIP_MAX_TIROS; i++) ativos += f.tiros[i].ativo;
    conferir(ativos == 1, "dois quadros seguidos com tiro saem só 1 tiro (recarga)");

    /* Acertar um batedor (1 de vida) dá 10 pontos. */
    iniciarFliperama(&f, 3);
    int batedor = f.totalInimigos - 1;            /* última fileira: batedores */
    soUmInimigo(&f, batedor);
    InimigoFliperama *e = &f.inimigos[batedor];
    f.tiros[0] = (TiroFliperama){ e->x, e->y + 4, true };
    f.balanco = 0;
    atualizarFliperama(&f, 0.001f, 0, 0, false);
    conferir(!e->vivo && f.pontos == 10, "tiro no batedor: ele cai e vale 10 pontos");
    conferir(f.fase == FLIP_ENTRE_ONDAS, "sem naves vivas, vem o intervalo entre ondas");
    for (int i = 0; i < 40; i++) atualizarFliperama(&f, 0.05f, 0, 0, false);
    conferir(f.onda == 2 && f.fase == FLIP_JOGANDO && inimigosVivos(&f) == 18, "depois do intervalo começa a onda 2");
    f.onda = 2;
    f.fase = FLIP_ENTRE_ONDAS;
    f.tempoFase = 0;
    for (int i = 0; i < 40; i++) atualizarFliperama(&f, 0.05f, 0, 0, false);
    conferir(f.onda == 3 && f.totalInimigos == 24, "da onda 3 em diante são 24 naves");

    /* Comandante aguenta 2 tiros. */
    iniciarFliperama(&f, 5);
    soUmInimigo(&f, 0);
    e = &f.inimigos[0];
    conferir(e->tipo == 2, "a primeira fileira é de comandantes");
    f.tiros[0] = (TiroFliperama){ e->x, e->y, true };
    atualizarFliperama(&f, 0.001f, 0, 0, false);
    conferir(e->vivo, "comandante sobrevive ao primeiro tiro");
    f.tiros[0] = (TiroFliperama){ e->x, e->y, true };
    atualizarFliperama(&f, 0.001f, 0, 0, false);
    conferir(!e->vivo && f.pontos == 50, "segundo tiro derruba o comandante: 50 pontos");

    /* Tiro inimigo tira uma vida e dá um tempo sem levar dano. */
    iniciarFliperama(&f, 9);
    soUmInimigo(&f, 0);
    f.tirosInimigos[0] = (TiroFliperama){ f.jogadorX, f.jogadorY, true };
    atualizarFliperama(&f, 0.001f, 0, 0, false);
    conferir(f.vidas == FLIP_VIDAS - 1 && f.invencivel > 0, "tiro inimigo tira 1 vida");
    f.tirosInimigos[0] = (TiroFliperama){ f.jogadorX, f.jogadorY, true };
    atualizarFliperama(&f, 0.001f, 0, 0, false);
    conferir(f.vidas == FLIP_VIDAS - 1, "logo depois, o jogador está protegido");

    /* Sem vidas, a partida acaba. */
    f.vidas = 1;
    f.invencivel = 0;
    f.tirosInimigos[0] = (TiroFliperama){ f.jogadorX, f.jogadorY, true };
    atualizarFliperama(&f, 0.001f, 0, 0, false);
    conferir(f.fase == FLIP_FIM && f.vidas == 0, "perdeu a última vida: fim de jogo");

    /* Simulação longa sem mexer: o jogo não trava e termina. */
    iniciarFliperama(&f, 123);
    for (int i = 0; i < 60 * 300 && f.fase != FLIP_FIM; i++) atualizarFliperama(&f, 1.0f / 60, 0, 0, false);
    conferir(f.fase == FLIP_FIM, "parado no lugar, o jogador acaba perdendo");

    /* Atirando sem parar e desviando, dá para passar da primeira onda. */
    iniciarFliperama(&f, 321);
    for (int i = 0; i < 60 * 300 && f.onda < 2 && f.fase != FLIP_FIM; i++) {
        float lado = sinf((float)i / 40.0f);
        atualizarFliperama(&f, 1.0f / 60, lado, 0, true);
    }
    conferir(f.onda >= 2 || f.pontos >= 100, "atirando e se mexendo, faz pontos de verdade");

    printf(falhas ? "\n%d FALHA(S)\n" : "\nTUDO CERTO (0 falhas)\n", falhas);
    return falhas != 0;
}

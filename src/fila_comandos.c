/* fila_comandos.c: ver fila_comandos.h. */

#include "fila_comandos.h"

void iniciarFila(FilaComandos *f)
{
    f->inicio = 0;
    f->quantidade = 0;
}

bool filaVazia(const FilaComandos *f)
{
    return f->quantidade == 0;
}

bool filaCheia(const FilaComandos *f)
{
    return f->quantidade == ROBO_MAX_COMANDOS;
}

bool enfileirar(FilaComandos *f, Comando c)
{
    if (filaCheia(f)) return false;

    /* O fim fica "quantidade" casas depois do início. O resto da divisão (%)
       faz a conta dar a volta: com 16 casas, depois da 15 vem a 0. */
    int fim = (f->inicio + f->quantidade) % ROBO_MAX_COMANDOS;
    f->itens[fim] = c;
    f->quantidade++;
    return true;
}

bool desenfileirar(FilaComandos *f, Comando *saida)
{
    if (filaVazia(f)) return false;

    *saida = f->itens[f->inicio];
    f->inicio = (f->inicio + 1) % ROBO_MAX_COMANDOS;
    f->quantidade--;
    return true;
}

bool removerUltimo(FilaComandos *f)
{
    if (filaVazia(f)) return false;

    f->quantidade--;   /* o último simplesmente deixa de contar */
    return true;
}

Comando comandoNaPosicao(const FilaComandos *f, int i)
{
    return f->itens[(f->inicio + i) % ROBO_MAX_COMANDOS];
}

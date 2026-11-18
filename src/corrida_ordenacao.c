#include <stddef.h>
#include "corrida_ordenacao.h"

bool iniciarOrdenador(Ordenador *o, AlgoritmoOrdenacao algoritmo,
                      const int valores[], int quantidade)
{
    if (o == NULL || valores == NULL || quantidade < 2 ||
        quantidade > ORDENACAO_MAX_VALORES ||
        (algoritmo != ORDENACAO_BOLHA && algoritmo != ORDENACAO_INSERCAO)) {
        return false;
    }
    for (int i = 0; i < quantidade; i++) {
        if (valores[i] <= 0) return false;
    }

    *o = (Ordenador){
        .quantidade = quantidade,
        .algoritmo = algoritmo,
        .i = algoritmo == ORDENACAO_BOLHA ? 0 : 1,
        .limite = quantidade - 1,
        .comparaA = -1,
        .comparaB = -1
    };
    for (int i = 0; i < quantidade; i++) o->valores[i] = valores[i];
    return true;
}

bool valoresOrdenados(const int valores[], int quantidade)
{
    if (valores == NULL || quantidade < 0) return false;
    for (int i = 1; i < quantidade; i++) {
        if (valores[i - 1] > valores[i]) return false;
    }
    return true;
}

static void passoBolha(Ordenador *o)
{
    o->comparaA = o->j;
    o->comparaB = o->j + 1;
    o->comparacoes++;
    o->moveuNestePasso = false;

    if (o->valores[o->j] > o->valores[o->j + 1]) {
        int temporario = o->valores[o->j];
        o->valores[o->j] = o->valores[o->j + 1];
        o->valores[o->j + 1] = temporario;
        o->trocas++;
        o->movimentos += 3; /* três atribuições para trocar dois valores */
        o->moveuNestePasso = true;
        o->houveTroca = true;
    }

    o->j++;
    if (o->j >= o->limite) {
        if (!o->houveTroca || o->limite <= 1) {
            o->terminou = true;
        } else {
            o->limite--;
            o->j = 0;
            o->houveTroca = false;
        }
    }
}

static void colocarChave(Ordenador *o, int posicao)
{
    o->valores[posicao] = o->chave;
    o->movimentos++;
    o->temChave = false;
    o->i++;
    if (o->i >= o->quantidade) o->terminou = true;
}

static void passoInsercao(Ordenador *o)
{
    o->moveuNestePasso = false;
    if (!o->temChave) {
        if (o->i >= o->quantidade) {
            o->terminou = true;
            return;
        }
        o->chave = o->valores[o->i];
        o->valores[o->i] = 0; /* o espaço vazio acompanha a chave na animação */
        o->movimentos++;
        o->temChave = true;
        o->j = o->i - 1;
    }

    if (o->j < 0) {
        o->comparaA = -1;
        o->comparaB = 0;
        colocarChave(o, 0);
        o->moveuNestePasso = true;
        return;
    }

    o->comparaA = o->j;
    o->comparaB = o->j + 1;
    o->comparacoes++;
    if (o->valores[o->j] > o->chave) {
        o->valores[o->j + 1] = o->valores[o->j];
        o->valores[o->j] = 0;
        o->movimentos++;
        o->moveuNestePasso = true;
        o->j--;
        if (o->j < 0) colocarChave(o, 0);
    } else {
        colocarChave(o, o->j + 1);
        o->moveuNestePasso = true;
    }
}

bool passoOrdenador(Ordenador *o)
{
    if (o == NULL || o->terminou) return false;
    o->comparaA = -1;
    o->comparaB = -1;
    o->moveuNestePasso = false;
    if (o->algoritmo == ORDENACAO_BOLHA) passoBolha(o);
    else passoInsercao(o);
    return true;
}

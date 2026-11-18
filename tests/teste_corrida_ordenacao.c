#include <stdio.h>
#include "corrida_ordenacao.h"

static int falhas;
static int permutacoesVerificadas;
#define VERIFICAR(condicao, nome) do { \
    if (!(condicao)) { printf("FALHOU: %s\n", nome); falhas++; } \
    else printf("ok: %s\n", nome); \
} while (0)

static void completar(Ordenador *o)
{
    int passos = 0;
    while (!o->terminou && passos < 1000) {
        passoOrdenador(o);
        passos++;
    }
}

static void testarSequencia(const int *entrada, int quantidade, const char *nome)
{
    Ordenador bolha, insercao;
    bool inicioOk = iniciarOrdenador(&bolha, ORDENACAO_BOLHA, entrada, quantidade) &&
                    iniciarOrdenador(&insercao, ORDENACAO_INSERCAO, entrada, quantidade);
    completar(&bolha);
    completar(&insercao);
    char mensagem[128];

    snprintf(mensagem, sizeof mensagem, "%s: ambos os algoritmos terminam", nome);
    VERIFICAR(inicioOk && bolha.terminou && insercao.terminou, mensagem);
    snprintf(mensagem, sizeof mensagem, "%s: Bubble Sort ordena os valores", nome);
    VERIFICAR(valoresOrdenados(bolha.valores, quantidade), mensagem);
    snprintf(mensagem, sizeof mensagem, "%s: Insertion Sort ordena os valores", nome);
    VERIFICAR(valoresOrdenados(insercao.valores, quantidade), mensagem);
    snprintf(mensagem, sizeof mensagem, "%s: pistas começam com a mesma sequência", nome);
    bool iguais = true;
    for (int i = 0; i < quantidade; i++) {
        if (bolha.valores[i] != insercao.valores[i]) iguais = false;
    }
    VERIFICAR(iguais, mensagem);
    snprintf(mensagem, sizeof mensagem, "%s: operações são contabilizadas", nome);
    VERIFICAR(bolha.comparacoes > 0 && insercao.comparacoes > 0 &&
              bolha.movimentos >= 0 && insercao.movimentos > 0, mensagem);
    VERIFICAR(!passoOrdenador(&bolha) && !passoOrdenador(&insercao),
              "ordenador concluído não aceita passos extras");
}

static void verificarPermutacoes(int valores[], int posicao, int quantidade)
{
    if (posicao == quantidade) {
        Ordenador bolha, insercao;
        iniciarOrdenador(&bolha, ORDENACAO_BOLHA, valores, quantidade);
        iniciarOrdenador(&insercao, ORDENACAO_INSERCAO, valores, quantidade);
        completar(&bolha);
        completar(&insercao);
        if (!valoresOrdenados(bolha.valores, quantidade) ||
            !valoresOrdenados(insercao.valores, quantidade)) {
            falhas++;
        }
        permutacoesVerificadas++;
        return;
    }
    for (int i = posicao; i < quantidade; i++) {
        int troca = valores[posicao];
        valores[posicao] = valores[i];
        valores[i] = troca;
        verificarPermutacoes(valores, posicao + 1, quantidade);
        troca = valores[posicao];
        valores[posicao] = valores[i];
        valores[i] = troca;
    }
}

int main(void)
{
    const int quaseOrdenada[] = { 1, 3, 2, 4, 5 };
    const int invertida[] = { 5, 4, 3, 2, 1 };
    const int misturada[] = { 6, 2, 5, 1, 4, 3 };
    Ordenador o;

    VERIFICAR(!iniciarOrdenador(NULL, ORDENACAO_BOLHA, quaseOrdenada, 5),
              "recusa estrutura nula");
    VERIFICAR(!iniciarOrdenador(&o, ORDENACAO_BOLHA, quaseOrdenada, 1),
              "recusa sequência curta demais");
    VERIFICAR(!iniciarOrdenador(&o, ORDENACAO_INSERCAO, quaseOrdenada,
                                ORDENACAO_MAX_VALORES + 1),
              "recusa sequência maior que o limite");
    VERIFICAR(!iniciarOrdenador(&o, ORDENACAO_INSERCAO, NULL, 5),
              "recusa sequência nula");

    testarSequencia(quaseOrdenada, 5, "quase ordenada");
    testarSequencia(invertida, 5, "invertida");
    testarSequencia(misturada, 6, "misturada");

    Ordenador bolha, insercao;
    iniciarOrdenador(&bolha, ORDENACAO_BOLHA, quaseOrdenada, 5);
    iniciarOrdenador(&insercao, ORDENACAO_INSERCAO, quaseOrdenada, 5);
    completar(&bolha);
    completar(&insercao);
    VERIFICAR(bolha.comparacoes + bolha.movimentos <
              insercao.comparacoes + insercao.movimentos,
              "custo da sequência quase ordenada escolhe Bubble Sort");

    iniciarOrdenador(&bolha, ORDENACAO_BOLHA, invertida, 5);
    iniciarOrdenador(&insercao, ORDENACAO_INSERCAO, invertida, 5);
    completar(&bolha);
    completar(&insercao);
    VERIFICAR(insercao.comparacoes + insercao.movimentos <
              bolha.comparacoes + bolha.movimentos,
              "custo da sequência invertida escolhe Insertion Sort");

    int todas[ORDENACAO_MAX_VALORES];
    for (int i = 0; i < ORDENACAO_MAX_VALORES; i++) todas[i] = i + 1;
    verificarPermutacoes(todas, 0, ORDENACAO_MAX_VALORES);
    VERIFICAR(permutacoesVerificadas == 40320 && falhas == 0,
              "ambos ordenam todas as 40320 permutações de oito valores");

    if (falhas != 0) {
        printf("%d falha(s)\n", falhas);
        return 1;
    }
    puts("Todos os testes da corrida passaram.");
    return 0;
}

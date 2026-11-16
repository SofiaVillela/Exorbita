/* teste_ser.c: testa as regras do planeta do Ser sem abrir o jogo.

   Compilar e rodar, a partir da pasta do projeto:
     gcc -std=c99 -Wall -Wextra -Iinclude src/ser_logica.c src/partida.c src/tripulacao.c src/lista_planetas.c tests/teste_ser.c -o build/teste_ser
     ./build/teste_ser
*/

#include <stdio.h>
#include "constantes.h"
#include "partida.h"
#include "lista_planetas.h"
#include "ser_logica.h"

static int falhas = 0;

static void conferir(bool condicao, const char *mensagem)
{
    if (condicao) {
        printf("ok: %s\n", mensagem);
    } else {
        printf("FALHOU: %s\n", mensagem);
        falhas++;
    }
}

static bool contem(const TipoDesejo lista[], int total, TipoDesejo tipo)
{
    for (int i = 0; i < total; i++) {
        if (lista[i] == tipo) {
            return true;
        }
    }
    return false;
}

/* Índices na ordem do tripulacao.c: Bento 0, Ana 1, Elias 2, Flora 3, Dora 4, Caio 5. */
enum { BENTO, ANA, ELIAS, FLORA, DORA, CAIO };

/* Lista pequena para os testes: dois planetas livres e a Via Láctea bloqueada. */
static Planeta *montarPlanetas(void)
{
    Planeta *inicio = NULL;
    inserirPlaneta(&inicio, "SER", "ser.png", 10, TELA_NAVE, false);
    inserirPlaneta(&inicio, "ROBÔ", "robo.png", 15, TELA_NAVE, false);
    inserirPlaneta(&inicio, "VIA LÁCTEA", "via.png", 20, TELA_NAVE, true);
    inicio->visitado = true;   /* o SER já foi visitado (estamos nele) */
    return inicio;
}

static void testarChance(void)
{
    conferir(chanceSer(0, 0) == 100, "primeira visita e garantida mesmo com sorte 0");
    conferir(chanceSer(1, 50) == 50, "segunda visita com sorte 50 = 50");
    conferir(chanceSer(2, 50) == 25, "terceira visita com sorte 50 = 25");
    conferir(chanceSer(3, 50) == 12, "quarta visita com sorte 50 = 12");
    conferir(chanceSer(9, 50) == 12, "da quarta em diante continua 12");
    conferir(chanceSer(1, 91) == 70, "sorte 91 soma 20 na segunda visita");
    conferir(chanceSer(3, 0) == 0, "chance nunca fica negativa");
    conferir(serApareceu(12, 12) && !serApareceu(12, 13), "sorteio dentro/fora da chance");
}

static void testarFiltro(void)
{
    ProgressoPartida *jogo = progressoPartida();
    iniciarPartida();
    jogo->temTradutor = false;
    Planeta *planetas = montarPlanetas();
    TipoDesejo lista[TOTAL_DESEJOS];

    for (int i = 0; i < TOTAL_TRIPULANTES; i++) {
        int n = desejosDisponiveis(jogo, i, planetas, lista);
        char mensagem[64];
        snprintf(mensagem, sizeof mensagem, "%s tem pelo menos 3 desejos (%d)",
                 jogo->tripulacao[i].nome, n);
        conferir(n >= 3, mensagem);
    }

    int n = desejosDisponiveis(jogo, BENTO, planetas, lista);
    conferir(n == 4 && contem(lista, n, DESEJO_TUDO), "Bento: 3 para todos + Quero tudo");

    n = desejosDisponiveis(jogo, ANA, planetas, lista);
    conferir(n == 4 && contem(lista, n, DESEJO_CAMINHO), "Ana: 3 + caminho");

    n = desejosDisponiveis(jogo, ELIAS, planetas, lista);
    conferir(n == 4 && contem(lista, n, DESEJO_CAMINHO), "Elias: 3 + caminho");

    n = desejosDisponiveis(jogo, FLORA, planetas, lista);
    conferir(n == 4 && contem(lista, n, DESEJO_REJUVENESCER), "Flora: 3 + rejuvenescer");

    n = desejosDisponiveis(jogo, DORA, planetas, lista);
    conferir(n == 4 && !contem(lista, n, DESEJO_REVIVER), "Dora: 3 + rejuvenescer, sem reviver");
    jogo->tripulacao[ANA].vivo = false;
    n = desejosDisponiveis(jogo, DORA, planetas, lista);
    conferir(contem(lista, n, DESEJO_REVIVER), "Dora ve reviver quando alguem morreu");

    n = desejosDisponiveis(jogo, CAIO, planetas, lista);
    conferir(n == 5 && contem(lista, n, DESEJO_TERRA), "Caio: 3 + rejuvenescer + Terra");

    n = desejosDisponiveis(jogo, ELIAS, NULL, lista);
    conferir(!contem(lista, n, DESEJO_CAMINHO), "sem lista de planetas, caminho some");

    liberarPlanetas(&planetas);
}

static void testarEfeitos(void)
{
    ProgressoPartida *jogo = progressoPartida();
    iniciarPartida();
    jogo->temTradutor = false;
    Planeta *planetas = montarPlanetas();

    jogo->combustivel = 30;
    aplicarDesejo(jogo, BENTO, planetas, DESEJO_TANQUE);
    conferir(jogo->combustivel == COMBUSTIVEL_MAXIMO, "tanque fica no maximo");
    TipoDesejo depois[TOTAL_DESEJOS];
    int restantes = desejosDisponiveis(jogo, BENTO, planetas, depois);
    conferir(!contem(depois, restantes, DESEJO_TANQUE), "tanque so uma vez por partida");

    aplicarDesejo(jogo, BENTO, planetas, DESEJO_INTELIGENCIA);
    conferir(jogo->tripulacao[BENTO].tracos.inteligencia == 50, "Bento: inteligencia 35 + 15 = 50");

    aplicarDesejo(jogo, DORA, planetas, DESEJO_SORTE);
    conferir(jogo->tripulacao[DORA].tracos.sorte == TRACO_MAXIMO, "Dora: sorte 91 + 20 para em 100");

    jogo->tripulacao[BENTO].idade = 53;
    aplicarDesejo(jogo, FLORA, planetas, DESEJO_REJUVENESCER);
    conferir(jogo->tripulacao[BENTO].idade == IDADE_REJUVENESCIDA && jogo->tripulacao[FLORA].idade == 20,
             "anos perdidos: o mais velho (Bento) fica com 18, Flora nao muda");

    jogo->tripulacao[ELIAS].vivo = false;
    aplicarDesejo(jogo, DORA, planetas, DESEJO_REVIVER);
    conferir(jogo->tripulacao[ELIAS].vivo, "Elias volta a viver");

    aplicarDesejo(jogo, ELIAS, planetas, DESEJO_CAMINHO);
    conferir(planetas->prox->visitado, "caminho marca o ROBO como visitado");
    conferir(planetas->prox->prox->bloqueado, "sem os pedacos do mapa, a Via Lactea continua fechada");

    jogo->combustivel = 10;
    aplicarDesejo(jogo, BENTO, planetas, DESEJO_TUDO);
    conferir(jogo->tripulacao[BENTO].idade == 18 + ENVELHECER_TUDO && jogo->tripulacao[BENTO].vivo &&
             jogo->combustivel == COMBUSTIVEL_MAXIMO,
             "Quero tudo: tanque cheio, Bento envelhece 30 anos e sobrevive");
    aplicarDesejo(jogo, BENTO, planetas, DESEJO_TUDO);
    conferir(!jogo->tripulacao[BENTO].vivo, "Quero tudo de novo: passa de 60 anos e morre");

    aplicarDesejo(jogo, CAIO, planetas, DESEJO_TERRA);
    conferir(!jogo->tripulacao[CAIO].vivo, "armadilha: Caio morre");

    liberarPlanetas(&planetas);
}

int main(void)
{
    testarChance();
    testarFiltro();
    testarEfeitos();
    printf("\n%d falha(s)\n", falhas);
    return falhas != 0;
}

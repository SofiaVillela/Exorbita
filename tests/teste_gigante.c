/* teste_gigante.c: testa as regras do planeta do gigante sem abrir o jogo.

   Compilar e rodar, a partir da pasta do projeto:
     gcc -std=c99 -Wall -Wextra -Iinclude src/gigante_logica.c tests/teste_gigante.c -lm -o build/teste_gigante
     ./build/teste_gigante
*/

#include <stdio.h>
#include "constantes.h"
#include "gigante_logica.h"

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

static void testarInfracoes(void)
{
    VigilanciaGigante v;
    iniciarVigilanciaGigante(&v);
    conferir(estadoGigante(&v) == GIGANTE_OBSERVANDO, "sem infracao o gigante so observa");

    conferir(!registrarInfracaoGigante(&v, INFRACAO_PLANTA), "uma planta nao leva a captura");
    conferir(v.pontos == 1 && v.infracoes == 1, "planta vale 1 ponto");
    conferir(estadoGigante(&v) == GIGANTE_ALERTA, "com 1 ponto o gigante fica alerta");

    conferir(!registrarInfracaoGigante(&v, INFRACAO_HABITANTE), "planta + habitante ainda nao captura");
    conferir(v.pontos == 3, "habitante vale 2 pontos");
    conferir(estadoGigante(&v) == GIGANTE_REACAO, "com 3 pontos o gigante reage");

    conferir(registrarInfracaoGigante(&v, INFRACAO_HABITANTE), "com 5 pontos o gigante pega");
    conferir(v.ultima == INFRACAO_HABITANTE && v.infracoes == 3, "lembra a ultima infracao e o total");

    iniciarVigilanciaGigante(&v);
    registrarInfracaoGigante(&v, INFRACAO_HABITANTE);
    registrarInfracaoGigante(&v, INFRACAO_HABITANTE);
    conferir(registrarInfracaoGigante(&v, INFRACAO_HABITANTE) && v.pontos == GIGANTE_LIMITE_INFRACOES,
             "os pontos nao passam do limite");
    conferir(!registrarInfracaoGigante(&v, INFRACAO_NENHUMA) && v.infracoes == 3,
             "infracao nenhuma nao conta");
}

static void testarCalma(void)
{
    VigilanciaGigante v;
    iniciarVigilanciaGigante(&v);
    registrarInfracaoGigante(&v, INFRACAO_PLANTA);
    registrarInfracaoGigante(&v, INFRACAO_PLANTA);

    conferir(!acalmarGigante(&v, GIGANTE_TEMPO_CALMA), "ate o tempo de calma nada muda");
    conferir(estadoGigante(&v) == GIGANTE_ALERTA, "ainda alerta no limite do tempo de calma");
    conferir(!acalmarGigante(&v, 1.0f), "comeca a se acalmar, mas ainda nao perdoa");
    conferir(estadoGigante(&v) == GIGANTE_CALMA, "depois do tempo de calma o estado e calma");
    conferir(acalmarGigante(&v, GIGANTE_TEMPO_PERDAO) && v.pontos == 1, "perdoa 1 ponto");

    registrarInfracaoGigante(&v, INFRACAO_PLANTA);
    conferir(estadoGigante(&v) == GIGANTE_ALERTA && v.semInfracao == 0,
             "nova infracao interrompe a calma");

    acalmarGigante(&v, GIGANTE_TEMPO_CALMA + 1.0f);
    acalmarGigante(&v, GIGANTE_TEMPO_PERDAO);
    conferir(acalmarGigante(&v, GIGANTE_TEMPO_PERDAO) && v.pontos == 0, "perdoa ate zerar");
    conferir(estadoGigante(&v) == GIGANTE_OBSERVANDO, "zerado, volta a so observar");
    conferir(!acalmarGigante(&v, 100.0f) && v.pontos == 0, "os pontos nao ficam negativos");
}

static void testarApelos(void)
{
    /* egoismo, inteligencia, carisma, impulsividade, curiosidade, sorte */
    Tracos ana = { 20, 88, 45, 30, 60, 40 };
    conferir(chanceApeloGigante(APELO_EXPLICAR, &ana) == 45, "explicar usa o carisma inteiro");
    conferir(chanceApeloGigante(APELO_REGRAS, &ana) == 79, "regras usa 90% da inteligencia");
    conferir(chanceApeloGigante(APELO_CULPAR, &ana) == 17, "culpar usa 85% do egoismo");
    conferir(chanceApeloGigante(APELO_EXPLICAR, NULL) == 0, "sem tracos a chance e zero");
    conferir(apeloConvenceu(45, 45) && !apeloConvenceu(45, 46), "sorteio dentro/fora da chance");
    conferir(!apeloConvenceu(0, 1), "chance zero nunca convence");
    conferir(apeloGigante(APELO_CULPAR) != NULL && apeloGigante(TOTAL_APELOS) == NULL,
             "apelo fora da lista devolve NULL");
}

static void testarHabitante(void)
{
    HabitanteGigante h = { 100, 200, 100, 120, 50, 0, 0 };
    conferir(alturaPuloHabitante(&h) == 0, "comeca no chao");
    atualizarHabitanteGigante(&h, GIGANTE_TEMPO_PULO / 2);
    conferir(h.x > 100 && alturaPuloHabitante(&h) > 0.9f, "no meio do pulo esta no alto e andou");

    float antes;
    atualizarHabitanteGigante(&h, GIGANTE_TEMPO_PULO / 2 + 0.01f);
    antes = h.x;
    atualizarHabitanteGigante(&h, GIGANTE_TEMPO_CHAO / 2);
    conferir(h.x == antes && alturaPuloHabitante(&h) == 0, "na pausa fica parado no chao");

    for (int i = 0; i < 600; i++) {
        atualizarHabitanteGigante(&h, 1.0f / 60);
    }
    conferir(h.x >= 100 && h.x <= 120, "nao sai do caminho");

    h.x = 119.9f;
    h.velocidade = 50;
    h.relogio = 0;
    atualizarHabitanteGigante(&h, 0.1f);
    conferir(h.x == 120 && h.velocidade < 0, "na ponta, inverte o sentido");

    PontoExploracao pes = { h.x + 5, h.y + 5 };
    conferir(habitanteEncostou(&h, pes), "pes perto: esbarrou");
    h.atordoado = GIGANTE_TEMPO_ATORDOADO;
    conferir(!habitanteEncostou(&h, pes), "atordoado nao conta de novo");
    antes = h.x;
    atualizarHabitanteGigante(&h, 0.5f);
    conferir(h.x == antes && alturaPuloHabitante(&h) == 0, "atordoado nao anda nem pula");
    pes.x = h.x + GIGANTE_RAIO_HABITANTE + 1;
    h.atordoado = 0;
    conferir(!habitanteEncostou(&h, pes), "pes longe: nao esbarrou");

    PontoExploracao planta = { 300, 200 };
    PontoExploracao perto = { 305, 203 };
    PontoExploracao longe = { 300, 200 + GIGANTE_RAIO_PLANTA };
    conferir(pisouNaPlanta(planta, perto), "pes sobre a planta: pisou");
    conferir(!pisouNaPlanta(planta, longe), "na borda do raio: nao pisou");
}

int main(void)
{
    testarInfracoes();
    testarCalma();
    testarApelos();
    testarHabitante();
    printf("\n%d falha(s)\n", falhas);
    return falhas != 0;
}

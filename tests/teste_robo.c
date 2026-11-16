/* teste_robo.c: testa a fila de comandos e as regras do laboratório do robô
   sem abrir o jogo.

   Compilar e rodar, a partir da pasta do projeto:
     gcc -std=c99 -Wall -Wextra -Iinclude src/fila_comandos.c src/robo_logica.c tests/teste_robo.c -o build/teste_robo
     ./build/teste_robo
*/

#include <stdio.h>
#include "robo_logica.h"

static int falhas = 0;
#define CONFERIR(cond, msg) do { if (!(cond)) { printf("FALHOU: %s\n", msg); falhas++; } else printf("ok: %s\n", msg); } while (0)

/* Atalhos para escrever programas nos testes: "DDB" = direita, direita, baixo. */
static Comando letra(char c)
{
    return c == 'C' ? CMD_CIMA : c == 'B' ? CMD_BAIXO : c == 'E' ? CMD_ESQUERDA : CMD_DIREITA;
}
static void programar(Laboratorio *lab, const char *prog)
{
    for (; *prog; prog++) adicionarComando(lab, letra(*prog));
}
/* Executa até parar e devolve o último resultado. */
static ResultadoPasso rodar(Laboratorio *lab)
{
    ResultadoPasso r = PASSO_NADA;
    if (!executarPrograma(lab)) return r;
    while (lab->fase == ROBO_EXECUTANDO) r = passoDoRobo(lab);
    return r;
}

static void testarFila(void)
{
    FilaComandos f;
    Comando c;
    iniciarFila(&f);

    CONFERIR(filaVazia(&f) && !desenfileirar(&f, &c), "fila nova esta vazia e nao deixa tirar");

    enfileirar(&f, CMD_CIMA); enfileirar(&f, CMD_BAIXO); enfileirar(&f, CMD_DIREITA);
    desenfileirar(&f, &c);
    CONFERIR(c == CMD_CIMA, "o primeiro que entra e o primeiro que sai");
    CONFERIR(comandoNaPosicao(&f, 0) == CMD_BAIXO && comandoNaPosicao(&f, 1) == CMD_DIREITA, "olhar sem tirar");

    removerUltimo(&f);
    CONFERIR(f.quantidade == 1 && comandoNaPosicao(&f, 0) == CMD_BAIXO, "remover ultimo tira do fim");

    /* Dar a volta no vetor: enche, tira alguns do começo, põe de novo. */
    iniciarFila(&f);
    for (int i = 0; i < ROBO_MAX_COMANDOS; i++) enfileirar(&f, CMD_CIMA);
    CONFERIR(filaCheia(&f) && !enfileirar(&f, CMD_BAIXO), "fila cheia recusa mais um");
    for (int i = 0; i < 5; i++) desenfileirar(&f, &c);
    for (int i = 0; i < 5; i++) enfileirar(&f, CMD_ESQUERDA);   /* estes caem no começo do vetor */
    int ordemCerta = 1;
    for (int i = 0; i < ROBO_MAX_COMANDOS; i++) {
        desenfileirar(&f, &c);
        if (c != (i < ROBO_MAX_COMANDOS - 5 ? CMD_CIMA : CMD_ESQUERDA)) ordemCerta = 0;
    }
    CONFERIR(ordemCerta && filaVazia(&f), "a ordem se mantem quando a fila da a volta no vetor");
}

static void testarLaboratorio(void)
{
    Laboratorio lab;
    iniciarLaboratorio(&lab, MAPA_LAB_PADRAO);
    CONFERIR(lab.x == 0 && lab.y == 0 && lab.fase == ROBO_MONTANDO, "robo comeca no S, montando");
    CONFERIR(!executarPrograma(&lab), "nao executa com a fila vazia");

    /* Solução do protótipo: 14 comandos. */
    programar(&lab, "DDBBDDCCDDBBBB");
    CONFERIR(rodar(&lab) == PASSO_SAIDA && lab.fase == ROBO_VENCEU, "a solucao do prototipo chega na saida");
    CONFERIR(lab.x == 6 && lab.y == 4 && lab.tentativas == 1, "robo termina na saida, 1 tentativa");
    CONFERIR(!adicionarComando(&lab, CMD_CIMA), "depois de vencer nao aceita mais comandos");

    /* Parede: o robô não sai do lugar. */
    iniciarLaboratorio(&lab, MAPA_LAB_PADRAO);
    programar(&lab, "DDDD");   /* a 3a seta bate na parede em (3,0); a 4a nem roda */
    CONFERIR(rodar(&lab) == PASSO_PAREDE && lab.x == 2 && lab.y == 0, "bate na parede e para antes dela");
    CONFERIR(lab.fase == ROBO_FALHOU && filaVazia(&lab.fila), "falha limpa a fila");

    /* Borda da grade conta como parede. */
    iniciarLaboratorio(&lab, MAPA_LAB_PADRAO);
    programar(&lab, "C");
    CONFERIR(rodar(&lab) == PASSO_PAREDE && lab.x == 0 && lab.y == 0, "sair da grade conta como parede");

    /* Lava: o robô fica na lava para a tela mostrar. */
    iniciarLaboratorio(&lab, MAPA_LAB_PADRAO);
    programar(&lab, "DDBBBE");   /* a 5a seta desce para (2,3), que é lava; a 6a nem roda */
    CONFERIR(rodar(&lab) == PASSO_LAVA && lab.x == 2 && lab.y == 3, "cai na lava e fica nela");

    /* Depois da falha, montar de novo devolve o robô ao início. */
    adicionarComando(&lab, CMD_DIREITA);
    CONFERIR(lab.fase == ROBO_MONTANDO && lab.x == 0 && lab.y == 0, "montar de novo volta o robo ao inicio");

    /* Comandos acabam antes da saída. */
    CONFERIR(rodar(&lab) == PASSO_SEM_COMANDOS && lab.fase == ROBO_FALHOU, "fila acaba antes da saida");
    CONFERIR(lab.tentativas == 2, "cada execucao conta uma tentativa");

    /* Durante a execução não dá para mexer na fila. */
    iniciarLaboratorio(&lab, MAPA_LAB_PADRAO);
    programar(&lab, "DD");
    executarPrograma(&lab);
    CONFERIR(!adicionarComando(&lab, CMD_BAIXO) && !removerUltimoComando(&lab), "fila travada durante a execucao");
    CONFERIR(passoDoRobo(&lab) == PASSO_ANDOU && lab.x == 1, "um passo por chamada");

    /* A fila não aceita mais que o máximo. */
    iniciarLaboratorio(&lab, MAPA_LAB_PADRAO);
    for (int i = 0; i < ROBO_MAX_COMANDOS; i++) adicionarComando(&lab, CMD_DIREITA);
    CONFERIR(!adicionarComando(&lab, CMD_DIREITA), "nao passa do maximo de comandos");
}

static void testarCaminho(void)
{
    Laboratorio lab;
    Comando caminho[ROBO_MAX_COMANDOS];
    iniciarLaboratorio(&lab, MAPA_LAB_PADRAO);

    int n = caminhoMaisCurto(&lab, caminho, ROBO_MAX_COMANDOS);
    printf("     caminho mais curto do mapa padrao: %d comandos (a fila aceita %d)\n", n, ROBO_MAX_COMANDOS);
    CONFERIR(n == 14, "caminho mais curto do mapa padrao tem 14 comandos");
    CONFERIR(n <= ROBO_MAX_COMANDOS, "a solucao cabe na fila");

    /* O caminho achado, se executado, tem que vencer. */
    for (int i = 0; i < n; i++) adicionarComando(&lab, caminho[i]);
    CONFERIR(rodar(&lab) == PASSO_SAIDA, "executar o caminho achado chega na saida");

    /* Pedir só os primeiros passos (a dica) não estoura o vetor. */
    Comando dica[ROBO_PASSOS_DICA];
    iniciarLaboratorio(&lab, MAPA_LAB_PADRAO);
    CONFERIR(caminhoMaisCurto(&lab, dica, ROBO_PASSOS_DICA) == 14 && dica[0] == CMD_DIREITA && dica[2] == CMD_BAIXO,
             "dica: so os primeiros passos");

    /* Mapa sem saída alcançável. */
    const char *const fechado[LAB_LINHAS] = { "S.#....", "..#....", "###....", ".......", "......E" };
    iniciarLaboratorio(&lab, fechado);
    CONFERIR(caminhoMaisCurto(&lab, caminho, ROBO_MAX_COMANDOS) == -1, "mapa sem caminho devolve -1");

    /* Mapa com linha curta e S fora do canto. */
    const char *const curto[LAB_LINHAS] = { "...", ".S.", "", "....E", "" };
    iniciarLaboratorio(&lab, curto);
    CONFERIR(lab.inicioX == 1 && lab.inicioY == 1 && caminhoMaisCurto(&lab, caminho, ROBO_MAX_COMANDOS) == 5,
             "linha curta vira casa livre e o S pode estar em qualquer lugar");
}

/* Todo circuito precisa ter saída alcançável com a fila (16 comandos),
   e seguir o caminho mais curto precisa mesmo levar à saída. */
static void testarCircuitos(void)
{
    int anterior = 0;
    for (int i = 0; i < LAB_TOTAL_CIRCUITOS; i++) {
        Laboratorio lab;
        Comando caminho[ROBO_MAX_COMANDOS];
        iniciarLaboratorio(&lab, CIRCUITOS_LAB[i]);
        int n = caminhoMaisCurto(&lab, caminho, ROBO_MAX_COMANDOS);
        printf("circuito %d: caminho mais curto com %d comandos\n", i + 1, n);
        CONFERIR(n > 0 && n <= ROBO_MAX_COMANDOS, "circuito tem solucao que cabe na fila");
        CONFERIR(n >= anterior, "circuito nao e mais facil que o anterior");
        anterior = n;
        for (int k = 0; k < n; k++) adicionarComando(&lab, caminho[k]);
        executarPrograma(&lab);
        ResultadoPasso ultimo = PASSO_NADA;
        while (lab.fase == ROBO_EXECUTANDO) ultimo = passoDoRobo(&lab);
        CONFERIR(lab.fase == ROBO_VENCEU && ultimo == PASSO_SAIDA, "seguir o caminho mais curto vence o circuito");
    }
    CONFERIR(CIRCUITOS_LAB[LAB_TOTAL_CIRCUITOS - 1] == MAPA_LAB_PADRAO, "o ultimo circuito e o mapa padrao");
}

int main(void)
{
    testarFila();
    testarLaboratorio();
    testarCaminho();
    testarCircuitos();
    printf("\n%s (%d falha%s)\n", falhas ? "HA FALHAS" : "TUDO CERTO", falhas, falhas == 1 ? "" : "s");
    return falhas ? 1 : 0;
}

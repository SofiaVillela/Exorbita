/* robo_logica.c: ver robo_logica.h. */

#include "robo_logica.h"

const char *const MAPA_LAB_PADRAO[LAB_LINHAS] = {
    "S..#...",
    "##.#.L.",
    ".....L.",
    ".LLL#..",
    "....#.E"
};

/* Circuito 1, de treino: um corredor com uma curva e pouca lava. */
static const char *const CIRCUITO_TREINO[LAB_LINHAS] = {
    "S....#.",
    ".##..#.",
    ".#L....",
    ".#..#L.",
    "...#..E"
};

/* Circuito 2: dois caminhos possíveis, os dois passando perto da lava. */
static const char *const CIRCUITO_DESVIO[LAB_LINHAS] = {
    "..#...E",
    ".L#.#L.",
    ".L..#..",
    ".##.L.#",
    "S......"
};

const char *const *const CIRCUITOS_LAB[LAB_TOTAL_CIRCUITOS] = {
    CIRCUITO_TREINO, CIRCUITO_DESVIO, MAPA_LAB_PADRAO
};

/* Quanto cada comando muda em x e em y. A ordem é a do enum Comando:
   cima, baixo, esquerda, direita. (y cresce para baixo, como na tela.) */
static const int MUDA_X[] = { 0, 0, -1, 1 };
static const int MUDA_Y[] = { -1, 1, 0, 0 };

static bool dentroDaGrade(int x, int y)
{
    return x >= 0 && x < LAB_COLUNAS && y >= 0 && y < LAB_LINHAS;
}

void iniciarLaboratorio(Laboratorio *lab, const char *const linhas[LAB_LINHAS])
{
    lab->inicioX = 0;
    lab->inicioY = 0;

    for (int y = 0; y < LAB_LINHAS; y++) {
        bool acabouLinha = false;   /* linha mais curta que a grade: o resto fica livre */
        for (int x = 0; x < LAB_COLUNAS; x++) {
            char c = '.';
            if (!acabouLinha) {
                c = linhas[y][x];
                if (c == '\0') { acabouLinha = true; c = '.'; }
            }

            if (c == '#')      lab->mapa[y][x] = CASA_PAREDE;
            else if (c == 'L') lab->mapa[y][x] = CASA_LAVA;
            else if (c == 'E') lab->mapa[y][x] = CASA_SAIDA;
            else               lab->mapa[y][x] = CASA_LIVRE;

            if (c == 'S') { lab->inicioX = x; lab->inicioY = y; }
        }
    }

    lab->x = lab->inicioX;
    lab->y = lab->inicioY;
    iniciarFila(&lab->fila);
    lab->fase = ROBO_MONTANDO;
    lab->tentativas = 0;
}

void voltarAoInicio(Laboratorio *lab)
{
    lab->x = lab->inicioX;
    lab->y = lab->inicioY;
    if (lab->fase == ROBO_FALHOU) {
        lab->fase = ROBO_MONTANDO;
    }
}

bool adicionarComando(Laboratorio *lab, Comando c)
{
    if (lab->fase == ROBO_FALHOU) voltarAoInicio(lab);
    if (lab->fase != ROBO_MONTANDO) return false;
    return enfileirar(&lab->fila, c);
}

bool removerUltimoComando(Laboratorio *lab)
{
    if (lab->fase != ROBO_MONTANDO) return false;
    return removerUltimo(&lab->fila);
}

void limparComandos(Laboratorio *lab)
{
    if (lab->fase == ROBO_MONTANDO) iniciarFila(&lab->fila);
}

bool executarPrograma(Laboratorio *lab)
{
    if (lab->fase == ROBO_FALHOU) voltarAoInicio(lab);
    if (lab->fase != ROBO_MONTANDO || filaVazia(&lab->fila)) return false;

    lab->x = lab->inicioX;
    lab->y = lab->inicioY;
    lab->fase = ROBO_EXECUTANDO;
    lab->tentativas++;
    return true;
}

/* Falhou: a fila é limpa e o robô fica parado onde está, para a tela mostrar. */
static void falhar(Laboratorio *lab)
{
    iniciarFila(&lab->fila);
    lab->fase = ROBO_FALHOU;
}

ResultadoPasso passoDoRobo(Laboratorio *lab)
{
    if (lab->fase != ROBO_EXECUTANDO) return PASSO_NADA;

    Comando c;
    if (!desenfileirar(&lab->fila, &c)) {
        falhar(lab);
        return PASSO_SEM_COMANDOS;
    }

    int novoX = lab->x + MUDA_X[c];
    int novoY = lab->y + MUDA_Y[c];

    if (!dentroDaGrade(novoX, novoY) || lab->mapa[novoY][novoX] == CASA_PAREDE) {
        falhar(lab);
        return PASSO_PAREDE;
    }

    lab->x = novoX;
    lab->y = novoY;

    if (lab->mapa[novoY][novoX] == CASA_LAVA) {
        falhar(lab);
        return PASSO_LAVA;
    }
    if (lab->mapa[novoY][novoX] == CASA_SAIDA) {
        iniciarFila(&lab->fila);
        lab->fase = ROBO_VENCEU;
        return PASSO_SAIDA;
    }
    return PASSO_ANDOU;
}

/* Busca em largura: visita primeiro as casas a 1 passo do início, depois
   as a 2 passos, e assim por diante. Por isso, a primeira vez que chega
   na saída é pelo caminho mais curto. Usa uma fila de casas a visitar
   (outra fila, só de números, que vive apenas dentro desta função). */
int caminhoMaisCurto(const Laboratorio *lab, Comando *saida, int max)
{
    enum { TOTAL = LAB_COLUNAS * LAB_LINHAS };

    int veioDe[TOTAL];        /* de qual casa cheguei aqui (-1 = ainda não visitei) */
    Comando comoVeio[TOTAL];  /* com qual comando cheguei aqui */
    int aVisitar[TOTAL];      /* cada casa entra no máximo uma vez, então TOTAL basta */
    int primeiro = 0, ultimo = 0;

    for (int i = 0; i < TOTAL; i++) veioDe[i] = -1;

    /* Cada casa vira um número só: linha * colunas + coluna. */
    int inicio = lab->inicioY * LAB_COLUNAS + lab->inicioX;
    veioDe[inicio] = inicio;
    aVisitar[ultimo++] = inicio;

    int achou = -1;
    while (primeiro < ultimo && achou < 0) {
        int atual = aVisitar[primeiro++];
        int ax = atual % LAB_COLUNAS;
        int ay = atual / LAB_COLUNAS;

        if (lab->mapa[ay][ax] == CASA_SAIDA) {
            achou = atual;
            break;
        }

        for (int c = 0; c < 4; c++) {
            int nx = ax + MUDA_X[c];
            int ny = ay + MUDA_Y[c];
            if (!dentroDaGrade(nx, ny)) continue;
            if (lab->mapa[ny][nx] == CASA_PAREDE || lab->mapa[ny][nx] == CASA_LAVA) continue;

            int vizinho = ny * LAB_COLUNAS + nx;
            if (veioDe[vizinho] != -1) continue;   /* já visitada */

            veioDe[vizinho] = atual;
            comoVeio[vizinho] = (Comando)c;
            aVisitar[ultimo++] = vizinho;
        }
    }

    if (achou < 0) return -1;

    /* Conta o tamanho voltando da saída até o início. */
    int tamanho = 0;
    for (int casa = achou; casa != inicio; casa = veioDe[casa]) tamanho++;

    /* Volta de novo, agora guardando: o comando número i vai para saida[i]. */
    int i = tamanho - 1;
    for (int casa = achou; casa != inicio; casa = veioDe[casa]) {
        if (i < max) saida[i] = comoVeio[casa];
        i--;
    }
    return tamanho;
}

/* mapa_estelar.c: painel de planetas, aberto pelo console central da nave.

   Os planetas ficam numa lista circular duplamente encadeada
   (lista_planetas.c). O painel mostra um planeta por vez, grande e no
   centro. A seta da esquerda vai para o ant e a da direita para o prox;
   como a lista é circular, do último se vai para o primeiro sem nenhum if.

   Regras:
   - toda viagem gasta o custo do planeta;
   - a tripulação está em outra galáxia: voltar para casa é voltar à
     Via Láctea. Depois de qualquer viagem, precisa sobrar o custo dela;
   - a Via Láctea começa bloqueada e abre quando todos os outros
     planetas forem visitados. */

#include <string.h>
#include "raylib.h"
#include "constantes.h"
#include "recursos.h"
#include "fundo.h"
#include "animacao.h"
#include "partida.h"
#include "lista_planetas.h"
#include "mapa_estelar.h"

#define TOTAL_PLANETAS_PAINEL 6
#define INDICE_VIA_LACTEA (TOTAL_PLANETAS_PAINEL - 1)   /* última de dados[] */

/* Dados para montar a lista. A ordem aqui é a ordem no painel.
   destino = TELA_NAVE quer dizer "ainda sem tela": o painel mostra o
   planeta, mas não deixa pousar. arquivoBloqueado só existe na Via
   Láctea: é a imagem desenhada enquanto ela está bloqueada. */
typedef struct {
    const char *nome;
    const char *arquivo;
    const char *arquivoBloqueado;
    int custo;
    Tela destino;
    bool bloqueado;
} DadosPlaneta;

static const DadosPlaneta dados[TOTAL_PLANETAS_PAINEL] = {
    { "SEM AR", "assets/imagens/planeta_sem_ar.png", NULL,
      CUSTO_SEM_AR, TELA_NAVE, false },
    { "GIGANTE", "assets/imagens/planeta_gigante.png", NULL,
      CUSTO_GIGANTE, TELA_GIGANTE, false },
    { "SER", "assets/imagens/planeta_ser.png", NULL,
      CUSTO_SER, TELA_SER, false },
    { "ROBÔ", "assets/imagens/planeta_robo.png", NULL,
      CUSTO_ROBO, TELA_ROBO, false },
    { "EXTRATERRESTRE", "assets/imagens/planeta_extraterrestre.png", NULL,
      CUSTO_EXTRATERRESTRE, TELA_EXTRATERRESTRE, false },
    { "VIA LÁCTEA", "assets/imagens/via_lactea.png",
      "assets/imagens/via_lactea_bloqueada.png",
      CUSTO_VIA_LACTEA, TELA_FINAL, true }
};

/* As imagens ficam fora da lista, porque a lista não usa raylib.
   animacoes[i] e bloqueadas[i] correspondem a dados[i]. */
static Animacao animacoes[TOTAL_PLANETAS_PAINEL];
static Animacao bloqueadas[TOTAL_PLANETAS_PAINEL];

static Planeta *lista = NULL;        /* início da lista circular */
static Planeta *selecionado = NULL;  /* nó que está no centro do painel */

static bool iniciado = false;
static bool aberto = false;
static Tela viagemSolicitada = TELA_NAVE;

static const char *aviso = NULL;     /* mensagem de alerta, se houver */
static float tempoAviso = 0;

/* ---------- Funções de apoio ---------- */

/* Acha a posição de um nó em dados[], comparando o caminho do arquivo.
   strcmp compara o texto; == compararia só os endereços. */
static int indiceDe(const Planeta *p)
{
    for (int i = 0; i < TOTAL_PLANETAS_PAINEL; i++) {
        if (strcmp(p->arquivo, dados[i].arquivo) == 0) {
            return i;
        }
    }
    return 0;
}

/* Animação certa para o planeta: a bloqueada, se ele estiver bloqueado
   e tiver essa imagem; senão, a normal. */
static const Animacao *animacaoDe(const Planeta *p)
{
    int i = indiceDe(p);
    if (p->bloqueado && dados[i].arquivoBloqueado != NULL) {
        return &bloqueadas[i];
    }
    return &animacoes[i];
}

/* Combustível que precisa sobrar depois de viajar para p: o custo da
   Via Láctea, para a volta. Indo para a própria Via Láctea, nada. */
static int reservaPara(const Planeta *p)
{
    if (indiceDe(p) == INDICE_VIA_LACTEA) {
        return 0;
    }
    return CUSTO_VIA_LACTEA;
}

/* Desenha o quadro atual do planeta com o centro em (x, y), ampliado
   pela escala. Igual ao desenharAnimacao, mas a partir do centro. */
static void desenharPlaneta(const Planeta *p, int x, int y, int escala)
{
    const Animacao *a = animacaoDe(p);
    if (a->folha.id == 0 || a->colunas == 0) {
        return;   /* imagem não carregou: não desenha (e não divide por zero) */
    }
    int coluna = a->quadroAtual % a->colunas;
    int linha = a->quadroAtual / a->colunas;
    Rectangle origem = { coluna * a->larguraQuadro, linha * a->alturaQuadro,
                         a->larguraQuadro, a->alturaQuadro };
    int tamanho = a->larguraQuadro * escala;
    Rectangle destino = { x - tamanho / 2, y - tamanho / 2, tamanho, tamanho };
    DrawTexturePro(a->folha, origem, destino, (Vector2){ 0, 0 }, 0, WHITE);
}

/* Escreve um texto com o centro em x. */
static void textoCentralizado(const char *texto, int x, int y, int tamanho, Color cor)
{
    Vector2 medida = MeasureTextEx(fontePadrao(), texto, tamanho, 0);
    DrawTextEx(fontePadrao(), texto, (Vector2){ x - (int)medida.x / 2, y },
               tamanho, 0, cor);
}

static void avisar(const char *mensagem)
{
    aviso = mensagem;
    tempoAviso = TEMPO_AVISO_PAINEL;
}

/* Tenta viajar para o planeta selecionado. Devolve true se a viagem
   aconteceu. Cada regra que impede a viagem mostra um aviso. */
static bool tentarViajar(void)
{
    ProgressoPartida *jogo = progressoPartida();

    if (selecionado->bloqueado) {
        avisar("Bloqueado: visite todos os outros planetas antes.");
        return false;
    }
    if (selecionado->destino == TELA_NAVE) {
        avisar("Pouso ainda indisponível neste planeta.");
        return false;
    }
    if (jogo->combustivel - selecionado->custo < reservaPara(selecionado)) {
        avisar("Combustível insuficiente: a reserva é para voltar à Via Láctea.");
        return false;
    }

    jogo->combustivel -= selecionado->custo;
    selecionado->visitado = true;

    /* Regra da Via Láctea: quando todos os livres foram visitados,
       os bloqueados abrem. */
    if (todosLivresVisitados(lista)) {
        desbloquearPlanetas(lista);
    }

    viagemSolicitada = selecionado->destino;
    return true;
}

/* ---------- Funções do mapa_estelar.h ---------- */

void iniciarMapaEstelar(void)
{
    if (iniciado) {
        return;
    }

    /* Monta a lista circular na ordem de dados[]. */
    for (int i = 0; i < TOTAL_PLANETAS_PAINEL; i++) {
        inserirPlaneta(&lista, dados[i].nome, dados[i].arquivo,
                       dados[i].custo, dados[i].destino, dados[i].bloqueado);

        carregarAnimacao(&animacoes[i], dados[i].arquivo,
                         TAMANHO_PLANETA, TAMANHO_PLANETA, QUADROS_PLANETA);
        if (dados[i].arquivoBloqueado != NULL) {
            carregarAnimacao(&bloqueadas[i], dados[i].arquivoBloqueado,
                             TAMANHO_PLANETA, TAMANHO_PLANETA, QUADROS_PLANETA);
        }
    }

    selecionado = lista;   /* começa no primeiro planeta */
    iniciado = true;
}

void abrirMapaEstelar(void)
{
    aberto = true;
    viagemSolicitada = TELA_NAVE;
    aviso = NULL;
    tempoAviso = 0;
    /* selecionado não muda: o painel abre onde o jogador parou. */
}

Tela destinoEscolhidoMapaEstelar(void)
{
    return viagemSolicitada;
}

bool atualizarMapaEstelar(void)
{
    if (!aberto || selecionado == NULL) {
        return false;
    }

    for (int i = 0; i < TOTAL_PLANETAS_PAINEL; i++) {
        atualizarAnimacao(&animacoes[i]);
        if (dados[i].arquivoBloqueado != NULL) {
            atualizarAnimacao(&bloqueadas[i]);
        }
    }

    if (tempoAviso > 0) {
        tempoAviso -= GetFrameTime();
    }

    /* Esc fecha sem viajar: a tela da nave continua. */
    if (IsKeyPressed(KEY_ESCAPE)) {
        aberto = false;
        viagemSolicitada = TELA_NAVE;
        return true;
    }

    /* Setas andam pela lista: ant para a esquerda, prox para a direita. */
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        selecionado = selecionado->ant;
        aviso = NULL;
    } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        selecionado = selecionado->prox;
        aviso = NULL;
    }

    if (IsKeyPressed(KEY_ENTER)) {
        if (tentarViajar()) {
            aberto = false;
            return true;
        }
    }
    return false;
}

void desenharMapaEstelar(void)
{
    if (!aberto || selecionado == NULL) {
        return;
    }
    ProgressoPartida *jogo = progressoPartida();

    /* Guia de estilo: painéis usam o fundo de espaço com estrelas. */
    desenharFundo();

    /* Topo: título à esquerda, combustível à direita. */
    DrawTextEx(fontePadrao(), "PAINEL DE PLANETAS", (Vector2){ MARGEM_PAINEL, MARGEM_PAINEL },
               16, 0, COR_TITULO);
    const char *textoCombustivel = TextFormat("COMBUSTÍVEL %d", jogo->combustivel);
    int largura = (int)MeasureTextEx(fontePadrao(), textoCombustivel, 8, 0).x;
    DrawTextEx(fontePadrao(), textoCombustivel,
               (Vector2){ LARGURA_VIRTUAL - MARGEM_PAINEL - largura, MARGEM_PAINEL }, 8, 0, COR_TITULO);
    const char *textoReserva = TextFormat("RESERVA PARA A VIA LÁCTEA %d", CUSTO_VIA_LACTEA);
    largura = (int)MeasureTextEx(fontePadrao(), textoReserva, 8, 0).x;
    DrawTextEx(fontePadrao(), textoReserva,
               (Vector2){ LARGURA_VIRTUAL - MARGEM_PAINEL - largura, MARGEM_PAINEL + 16 }, 8, 0, COR_MENU);

    /* Planeta selecionado, grande e no centro (pedido do playtest). */
    int metade = TAMANHO_PLANETA * ESCALA_PLANETA_PAINEL / 2;
    desenharPlaneta(selecionado, X_CENTRO_PAINEL, Y_CENTRO_PAINEL, ESCALA_PLANETA_PAINEL);

    /* Setas dos lados, só se existir mais de um planeta. */
    if (selecionado->ant != selecionado) {
        textoCentralizado("<", X_CENTRO_PAINEL - metade - ESPACO_SETA_PAINEL,
                          Y_CENTRO_PAINEL - 8, 16, COR_MARCADOR);
        textoCentralizado(">", X_CENTRO_PAINEL + metade + ESPACO_SETA_PAINEL,
                          Y_CENTRO_PAINEL - 8, 16, COR_MARCADOR);
    }

    int y = Y_CENTRO_PAINEL + metade + 12;
    textoCentralizado(selecionado->nome, X_CENTRO_PAINEL, y, 16, COR_TITULO);

    /* Situação do planeta: bloqueado, visitado ou não visitado. */
    const char *situacao;
    Color corSituacao = COR_MENU;
    if (selecionado->bloqueado) {
        situacao = "BLOQUEADO";
        corSituacao = COR_ALERTA;
    } else if (selecionado->visitado) {
        situacao = "VISITADO";
    } else {
        situacao = "NÃO VISITADO";
        corSituacao = COR_MARCADOR;
    }
    textoCentralizado(TextFormat("CUSTO %d   %s", selecionado->custo, situacao),
                      X_CENTRO_PAINEL, y + 24, 8, corSituacao);

    /* Quanto sobra depois da viagem. Em pêssego se ficar abaixo da reserva. */
    int depois = jogo->combustivel - selecionado->custo;
    Color corDepois = COR_MENU;
    if (depois < reservaPara(selecionado)) {
        corDepois = COR_ALERTA;
    }
    textoCentralizado(TextFormat("DEPOIS DA VIAGEM: %d", depois),
                      X_CENTRO_PAINEL, y + 38, 8, corDepois);

    if (aviso != NULL && tempoAviso > 0) {
        textoCentralizado(aviso, X_CENTRO_PAINEL, y + 54, 8, COR_ALERTA);
    }

    /* Teclas, no rodapé. */
    textoCentralizado("SETAS  Escolher     ENTER  Viajar     ESC  Voltar",
                      X_CENTRO_PAINEL, ALTURA_VIRTUAL - 16, 8, COR_MENU);
}

void encerrarMapaEstelar(void)
{
    if (!iniciado) {
        return;
    }
    for (int i = 0; i < TOTAL_PLANETAS_PAINEL; i++) {
        descarregarAnimacao(&animacoes[i]);
        if (dados[i].arquivoBloqueado != NULL) {
            descarregarAnimacao(&bloqueadas[i]);
        }
    }
    liberarPlanetas(&lista);
    selecionado = NULL;
    iniciado = false;
    aberto = false;
}

Planeta *listaPlanetasMapa(void)
{
    return lista;
}

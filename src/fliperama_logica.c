/* fliperama_logica.c: regras do jogo de naves do fliperama (ver fliperama_logica.h). */

#include <math.h>
#include <string.h>
#include "fliperama_logica.h"

/* ---------- apoio ---------- */

static uint32_t sortear(Fliperama *f)
{
    f->aleatorio = f->aleatorio * UINT32_C(1664525) + UINT32_C(1013904223);
    return f->aleatorio >> 8;
}

/* Número de 0 a 1. */
static float sortearFracao(Fliperama *f)
{
    return (float)(sortear(f) % 10000) / 10000.0f;
}

static float limitar(float v, float minimo, float maximo)
{
    if (v < minimo) return minimo;
    if (v > maximo) return maximo;
    return v;
}

static bool encostou(float ax, float ay, float bx, float by, float raio)
{
    float dx = ax - bx, dy = ay - by;
    return dx * dx + dy * dy <= raio * raio;
}

static void explodir(Fliperama *f, float x, float y, bool grande)
{
    for (int i = 0; i < FLIP_MAX_EXPLOSOES; i++) {
        if (!f->explosoes[i].ativa) {
            f->explosoes[i] = (ExplosaoFliperama){ x, y, 0, grande, true };
            return;
        }
    }
}

/* Velocidade do jogo na onda atual: cada onda é 12% mais rápida. */
static float ritmo(const Fliperama *f)
{
    return 1.0f + 0.12f * (float)(f->onda - 1);
}

/* ---------- ondas ---------- */

/* Formação: comandantes em cima, caças no meio, batedores embaixo.
   Mais fileiras a partir da terceira onda. */
static void montarOnda(Fliperama *f)
{
    static const int fileiras[][2] = {   /* tipo e quantidade de cada fileira */
        { 2, 4 }, { 1, 6 }, { 1, 6 }, { 0, 8 }
    };
    /* Ondas 1 e 2 pulam a segunda fileira de caças: 4 + 6 + 8 = 18 naves.
       Da terceira em diante vêm as quatro: 24 naves. */
    int ordem[4] = { 0, 1, 3, 3 }, quantas = 3;
    if (f->onda >= 3) {
        ordem[2] = 2;
        quantas = 4;
    }
    f->totalInimigos = 0;
    for (int fila = 0; fila < quantas; fila++) {
        int tipo = fileiras[ordem[fila]][0], n = fileiras[ordem[fila]][1];
        for (int i = 0; i < n && f->totalInimigos < FLIP_MAX_INIMIGOS; i++) {
            InimigoFliperama *e = &f->inimigos[f->totalInimigos++];
            memset(e, 0, sizeof(*e));
            e->tipo = tipo;
            e->vida = tipo == 2 ? 2 : 1;
            e->vivo = true;
            e->estado = INIMIGO_EM_FORMACAO;
            e->lugarX = FLIP_LARGURA / 2.0f + ((float)i - (float)(n - 1) / 2.0f) * 34.0f;
            e->lugarY = 46.0f + (float)fila * 30.0f;
            e->x = e->lugarX;
            e->y = e->lugarY;
        }
    }
    f->balanco = 0;
    f->proximoMergulho = 2.0f;
    f->fase = FLIP_JOGANDO;
}

void iniciarFliperama(Fliperama *f, uint32_t semente)
{
    memset(f, 0, sizeof(*f));
    f->aleatorio = semente ? semente : 1;
    f->vidas = FLIP_VIDAS;
    f->onda = 1;
    f->jogadorX = FLIP_LARGURA / 2.0f;
    f->jogadorY = FLIP_ALTURA - 28.0f;
    montarOnda(f);
}

int inimigosVivos(const Fliperama *f)
{
    int vivos = 0;
    for (int i = 0; i < f->totalInimigos; i++) {
        if (f->inimigos[i].vivo) vivos++;
    }
    return vivos;
}

int pontosInimigo(const InimigoFliperama *e)
{
    static const int valor[] = { 10, 20, 50 };
    int pontos = valor[e->tipo];
    if (e->estado != INIMIGO_EM_FORMACAO) {
        pontos *= 2;
    }
    return pontos;
}

/* ---------- partes do quadro ---------- */

static void moverJogador(Fliperama *f, float dt, float eixoX, float eixoY, bool atirar)
{
    float tamanho = sqrtf(eixoX * eixoX + eixoY * eixoY);
    if (tamanho > 1) {           /* diagonal não é mais rápida */
        eixoX /= tamanho;
        eixoY /= tamanho;
    }
    f->jogadorX = limitar(f->jogadorX + eixoX * FLIP_VELOCIDADE * dt, 14, FLIP_LARGURA - 14);
    f->jogadorY = limitar(f->jogadorY + eixoY * FLIP_VELOCIDADE * dt, FLIP_TOPO_JOGADOR, FLIP_ALTURA - 16);

    f->recarga -= dt;
    if (atirar && f->recarga <= 0) {
        for (int i = 0; i < FLIP_MAX_TIROS; i++) {
            if (!f->tiros[i].ativo) {
                f->tiros[i] = (TiroFliperama){ f->jogadorX, f->jogadorY - 14, true };
                f->recarga = FLIP_RECARGA;
                break;
            }
        }
    }
}

static void atirarInimigo(Fliperama *f, const InimigoFliperama *e)
{
    for (int i = 0; i < FLIP_MAX_TIROS_INIMIGOS; i++) {
        if (!f->tirosInimigos[i].ativo) {
            f->tirosInimigos[i] = (TiroFliperama){ e->x, e->y + 10, true };
            return;
        }
    }
}

/* Escolhe uma nave da formação para mergulhar. */
static void comecarMergulho(Fliperama *f)
{
    int candidatos[FLIP_MAX_INIMIGOS], total = 0;
    for (int i = 0; i < f->totalInimigos; i++) {
        if (f->inimigos[i].vivo && f->inimigos[i].estado == INIMIGO_EM_FORMACAO) {
            candidatos[total++] = i;
        }
    }
    if (total == 0) return;
    InimigoFliperama *e = &f->inimigos[candidatos[sortear(f) % (uint32_t)total]];
    e->estado = INIMIGO_MERGULHANDO;
    e->tempoMergulho = 0;
    e->origemX = e->x;
    e->lado = e->x < FLIP_LARGURA / 2.0f ? 1.0f : -1.0f;
    e->recargaTiro = 0.5f + sortearFracao(f) * 0.6f;
}

static void moverInimigos(Fliperama *f, float dt)
{
    float r = ritmo(f);
    f->balanco = sinf(f->tempo * 0.7f * r) * 40.0f;

    /* Quanto menos naves sobram, mais vezes elas mergulham. */
    f->proximoMergulho -= dt;
    if (f->proximoMergulho <= 0) {
        comecarMergulho(f);
        float pausa = 1.6f + sortearFracao(f) * 1.4f;
        if (inimigosVivos(f) <= 6) pausa *= 0.6f;
        f->proximoMergulho = pausa / r;
    }

    for (int i = 0; i < f->totalInimigos; i++) {
        InimigoFliperama *e = &f->inimigos[i];
        if (!e->vivo) continue;
        float alvoX = e->lugarX + f->balanco;
        float alvoY = e->lugarY + sinf(f->tempo * 2.0f + (float)i) * 2.0f;
        if (e->estado == INIMIGO_EM_FORMACAO) {
            e->x = alvoX;
            e->y = alvoY;
            /* De vez em quando, quem está na formação também atira. */
            if (sortear(f) % 100000 < (uint32_t)(dt * 900 * r)) {
                atirarInimigo(f, e);
            }
        } else if (e->estado == INIMIGO_MERGULHANDO) {
            /* Desce em curva, puxando um pouco para o lado do jogador. */
            e->tempoMergulho += dt;
            float t = e->tempoMergulho;
            e->y += (70.0f + 55.0f * t) * r * dt;
            float curva = sinf(t * 2.4f) * 70.0f * e->lado;
            float persegue = (f->jogadorX - e->origemX) * limitar(t * 0.5f, 0, 0.8f);
            e->x = limitar(e->origemX + curva + persegue, 12, FLIP_LARGURA - 12);
            e->recargaTiro -= dt;
            if (e->recargaTiro <= 0 && e->y < FLIP_ALTURA - 60) {
                atirarInimigo(f, e);
                e->recargaTiro = 0.9f / r;
            }
            if (e->y > FLIP_ALTURA + 20) {   /* passou: volta por cima */
                e->y = -20;
                e->estado = INIMIGO_VOLTANDO;
            }
        } else {
            /* Voltando para o lugar na formação. */
            float dx = alvoX - e->x, dy = alvoY - e->y;
            float d = sqrtf(dx * dx + dy * dy);
            float passo = 140.0f * r * dt;
            if (d <= passo) {
                e->estado = INIMIGO_EM_FORMACAO;
            } else {
                e->x += dx / d * passo;
                e->y += dy / d * passo;
            }
        }
    }
}

static void moverTiros(Fliperama *f, float dt)
{
    for (int i = 0; i < FLIP_MAX_TIROS; i++) {
        TiroFliperama *t = &f->tiros[i];
        if (!t->ativo) continue;
        t->y -= FLIP_VELOCIDADE_TIRO * dt;
        if (t->y < -10) t->ativo = false;
    }
    float velocidade = 150.0f * ritmo(f);
    for (int i = 0; i < FLIP_MAX_TIROS_INIMIGOS; i++) {
        TiroFliperama *t = &f->tirosInimigos[i];
        if (!t->ativo) continue;
        t->y += velocidade * dt;
        if (t->y > FLIP_ALTURA + 10) t->ativo = false;
    }
}

static void perderVida(Fliperama *f)
{
    explodir(f, f->jogadorX, f->jogadorY, true);
    f->vidas--;
    f->invencivel = FLIP_TEMPO_INVENCIVEL;
    /* Limpa os tiros inimigos: recomeçar no meio de uma chuva é injusto. */
    for (int i = 0; i < FLIP_MAX_TIROS_INIMIGOS; i++) f->tirosInimigos[i].ativo = false;
    if (f->vidas <= 0) {
        f->vidas = 0;
        f->fase = FLIP_FIM;
        f->tempoFase = 0;
    }
}

static void verificarBatidas(Fliperama *f)
{
    /* Tiros do jogador nas naves. */
    for (int i = 0; i < FLIP_MAX_TIROS; i++) {
        TiroFliperama *t = &f->tiros[i];
        if (!t->ativo) continue;
        for (int k = 0; k < f->totalInimigos; k++) {
            InimigoFliperama *e = &f->inimigos[k];
            if (!e->vivo || !encostou(t->x, t->y, e->x, e->y, FLIP_RAIO_INIMIGO)) continue;
            t->ativo = false;
            e->vida--;
            if (e->vida <= 0) {
                e->vivo = false;
                f->pontos += pontosInimigo(e);
                explodir(f, e->x, e->y, false);
            }
            break;
        }
    }
    if (f->invencivel > 0 || f->fase != FLIP_JOGANDO) return;

    /* Tiros inimigos e batidas no jogador. */
    for (int i = 0; i < FLIP_MAX_TIROS_INIMIGOS; i++) {
        TiroFliperama *t = &f->tirosInimigos[i];
        if (t->ativo && encostou(t->x, t->y, f->jogadorX, f->jogadorY, FLIP_RAIO_JOGADOR - 2)) {
            t->ativo = false;
            perderVida(f);
            return;
        }
    }
    for (int k = 0; k < f->totalInimigos; k++) {
        InimigoFliperama *e = &f->inimigos[k];
        if (e->vivo && encostou(e->x, e->y, f->jogadorX, f->jogadorY, FLIP_RAIO_JOGADOR + FLIP_RAIO_INIMIGO - 6)) {
            e->vivo = false;     /* a nave inimiga também se desfaz na batida */
            explodir(f, e->x, e->y, false);
            perderVida(f);
            return;
        }
    }
}

/* ---------- quadro ---------- */

void atualizarFliperama(Fliperama *f, float dt, float eixoX, float eixoY, bool atirar)
{
    if (!f || !(dt > 0)) return;
    if (dt > 0.1f) dt = 0.1f;
    f->tempo += dt;
    f->tempoFase += dt;

    for (int i = 0; i < FLIP_MAX_EXPLOSOES; i++) {
        ExplosaoFliperama *x = &f->explosoes[i];
        if (!x->ativa) continue;
        x->tempo += dt;
        if (x->tempo >= FLIP_TEMPO_EXPLOSAO * (x->grande ? 2 : 1)) x->ativa = false;
    }
    if (f->fase == FLIP_FIM) return;     /* fim de jogo: só as explosões terminam */

    if (f->invencivel > 0) f->invencivel -= dt;
    moverJogador(f, dt, eixoX, eixoY, atirar);
    moverTiros(f, dt);

    if (f->fase == FLIP_ENTRE_ONDAS) {
        if (f->tempoFase >= FLIP_TEMPO_ENTRE_ONDAS) {
            f->onda++;
            montarOnda(f);
        }
        return;
    }
    moverInimigos(f, dt);
    verificarBatidas(f);
    if (f->fase == FLIP_JOGANDO && inimigosVivos(f) == 0) {
        f->fase = FLIP_ENTRE_ONDAS;
        f->tempoFase = 0;
        for (int i = 0; i < FLIP_MAX_TIROS_INIMIGOS; i++) f->tirosInimigos[i].ativo = false;
    }
}

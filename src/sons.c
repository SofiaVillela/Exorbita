/* sons.c: efeitos sonoros, sons de ambiente e passos (ver sons.h). */

#include <math.h>
#include <stddef.h>
#include "raylib.h"
#include "musica.h"
#include "sons.h"

#define PASTA_SONS        "assets/sons/"
#define VOLUME_EFEITOS    0.8f
#define VOLUME_AMBIENTE   0.55f
#define VOLUME_TENSAO     0.9f
#define RAPIDEZ_TROCA     1.6f    /* ambientes: de 0 ao volume cheio em cerca de 0,6 s */

/* Nome do arquivo (sem pasta e sem .qoa) e volume de cada efeito,
   na mesma ordem do enum Som. */
static const struct { const char *arquivo; float volume; } tabela[TOTAL_SONS] = {
    [SOM_UI_MOVER]        = { "ui_mover", 0.5f },
    [SOM_UI_CONFIRMAR]    = { "ui_confirmar", 0.35f },
    [SOM_UI_VOLTAR]       = { "ui_voltar", 0.35f },
    [SOM_NEGADO]          = { "negado", 0.6f },
    [SOM_TERMINAL]        = { "terminal", 0.7f },
    [SOM_ITEM]            = { "item", 0.8f },
    [SOM_VITORIA]         = { "vitoria", 0.9f },
    [SOM_DERROTA]         = { "derrota", 0.9f },
    [SOM_PERDA]           = { "perda", 1.0f },
    [SOM_FALA_ALIEN]      = { "fala_alien", 0.6f },
    [SOM_DESCIDA]         = { "descida", 1.0f },
    [SOM_RETORNO]         = { "retorno", 1.0f },
    [SOM_PORTA_BAR]       = { "porta_bar", 0.9f },
    [SOM_PORTAO]          = { "portao", 1.0f },
    [SOM_PASSO_METAL]     = { "passo_metal", 0.55f },
    [SOM_PASSO_POEIRA]    = { "passo_poeira", 0.7f },
    [SOM_PASSO_GRAMA]     = { "passo_grama", 0.7f },
    [SOM_PASSO_ASFALTO]   = { "passo_asfalto", 0.6f },
    [SOM_PASSO_MADEIRA]   = { "passo_madeira", 0.6f },
    [SOM_PASSO_SANTUARIO] = { "passo_santuario", 0.6f },
    [SOM_PLANTA]          = { "planta", 0.9f },
    [SOM_HABITANTE]       = { "habitante", 0.7f },
    [SOM_GIGANTE_REAGE]   = { "gigante_reage", 1.0f },
    [SOM_GIGANTE_PEGA]    = { "gigante_pega", 1.0f },
    [SOM_GIGANTE_ACALMA]  = { "gigante_acalma", 0.9f },
    [SOM_OXIGENIO]        = { "oxigenio", 0.9f },
    [SOM_VAZAMENTO]       = { "vazamento", 0.9f },
    [SOM_ALARME]          = { "alarme", 0.6f },
    [SOM_LARGAR]          = { "largar", 0.8f },
    [SOM_SER_APARECE]     = { "ser_aparece", 0.9f },
    [SOM_DESEJO]          = { "desejo", 0.9f },
    [SOM_ROBO_COMANDO]    = { "robo_comando", 0.6f },
    [SOM_ROBO_APAGA]      = { "robo_apaga", 0.6f },
    [SOM_ROBO_PASSO]      = { "robo_passo", 0.6f },
    [SOM_CARTA_MEXE]      = { "carta_mexe", 0.8f },
    [SOM_CARTA_JOGA]      = { "carta_joga", 0.9f },
    [SOM_RODADA_GANHA]    = { "rodada_ganha", 0.8f },
    [SOM_RODADA_PERDE]    = { "rodada_perde", 0.8f },
    [SOM_FLIP_TIRO]       = { "flip_tiro", 0.6f },
    [SOM_FLIP_EXPLOSAO]   = { "flip_explosao", 0.8f },
    [SOM_FLIP_ATINGIDO]   = { "flip_atingido", 0.9f },
    [SOM_FLIP_ONDA]       = { "flip_onda", 0.8f },
    [SOM_FLIP_FIM]        = { "flip_fim", 0.9f },
    [SOM_FLIP_INICIO]     = { "flip_inicio", 0.8f },
};

/* Arquivo de cada ambiente (NULL = silêncio). */
static const char *arquivosAmbiente[TOTAL_AMBIENTES] = {
    [AMBIENTE_NENHUM]    = NULL,
    [AMBIENTE_NAVE]      = "amb_nave",
    [AMBIENTE_CIDADE]    = "amb_cidade",
    [AMBIENTE_BAR]       = "amb_bar",
    [AMBIENTE_SEM_AR]    = "amb_sem_ar",
    [AMBIENTE_GIGANTE]   = "amb_gigante",
    [AMBIENTE_SER]       = "amb_ser",
    [AMBIENTE_ROBO]      = "amb_robo",
    [AMBIENTE_FLIPERAMA] = "flip_musica",
};

static const Som somDoPiso[TOTAL_PISOS] = {
    [PISO_METAL] = SOM_PASSO_METAL, [PISO_POEIRA] = SOM_PASSO_POEIRA, [PISO_GRAMA] = SOM_PASSO_GRAMA,
    [PISO_ASFALTO] = SOM_PASSO_ASFALTO, [PISO_MADEIRA] = SOM_PASSO_MADEIRA,
    [PISO_SANTUARIO] = SOM_PASSO_SANTUARIO,
};

static bool pronto = false;
static Sound efeitos[TOTAL_SONS];
static bool temEfeito[TOTAL_SONS];
static Music ambientes[TOTAL_AMBIENTES];
static bool temAmbiente[TOTAL_AMBIENTES];
static float volumeAmbiente[TOTAL_AMBIENTES];   /* volume atual de cada um, de 0 a 1 */
static Ambiente ambienteAtual = AMBIENTE_NENHUM;
static Piso pisoAtual = PISO_METAL;

static Music tensao;                 /* laço da fúria do gigante */
static bool temTensao = false;
static float tensaoAlvo = 0, tensaoAgora = 0;

/* ---------- carregamento ---------- */

void iniciarSons(void)
{
    if (pronto || !IsAudioDeviceReady()) {
        return;
    }
    for (int i = 0; i < TOTAL_SONS; i++) {
        const char *caminho = TextFormat("%s%s.qoa", PASTA_SONS, tabela[i].arquivo);
        temEfeito[i] = false;
        if (tabela[i].arquivo != NULL && FileExists(caminho)) {
            efeitos[i] = LoadSound(caminho);
            temEfeito[i] = efeitos[i].frameCount > 0;
        }
        if (temEfeito[i]) {
            SetSoundVolume(efeitos[i], tabela[i].volume * VOLUME_EFEITOS);
        }
    }
    for (int i = 0; i < TOTAL_AMBIENTES; i++) {
        temAmbiente[i] = false;
        volumeAmbiente[i] = 0;
        if (arquivosAmbiente[i] == NULL) {
            continue;
        }
        const char *caminho = TextFormat("%s%s.qoa", PASTA_SONS, arquivosAmbiente[i]);
        if (FileExists(caminho)) {
            ambientes[i] = LoadMusicStream(caminho);
            temAmbiente[i] = ambientes[i].frameCount > 0;
        }
        if (temAmbiente[i]) {
            ambientes[i].looping = true;
            SetMusicVolume(ambientes[i], 0);
        }
    }
    const char *caminho = TextFormat("%sgigante_tensao.qoa", PASTA_SONS);
    if (FileExists(caminho)) {
        tensao = LoadMusicStream(caminho);
        temTensao = tensao.frameCount > 0;
    }
    if (temTensao) {
        tensao.looping = true;
        SetMusicVolume(tensao, 0);
    }
    pronto = true;
}

void encerrarSons(void)
{
    if (!pronto) {
        return;
    }
    for (int i = 0; i < TOTAL_SONS; i++) {
        if (temEfeito[i]) UnloadSound(efeitos[i]);
    }
    for (int i = 0; i < TOTAL_AMBIENTES; i++) {
        if (temAmbiente[i]) UnloadMusicStream(ambientes[i]);
    }
    if (temTensao) UnloadMusicStream(tensao);
    pronto = false;
}

/* ---------- efeitos ---------- */

void tocarSomVariado(Som som, float variacao)
{
    if (!pronto || som < 0 || som >= TOTAL_SONS || !temEfeito[som]) {
        return;
    }
    float tom = 1;
    if (variacao > 0) {
        tom += variacao * (float)GetRandomValue(-100, 100) / 100.0f;
    }
    SetSoundPitch(efeitos[som], tom);
    PlaySound(efeitos[som]);
}

void tocarSom(Som som)
{
    tocarSomVariado(som, 0);
}

void tocarPasso(bool correndo)
{
    Som som = somDoPiso[pisoAtual];
    if (!pronto || !temEfeito[som]) {
        return;
    }
    /* Cada passo sai um pouco diferente do anterior, para não parecer máquina. */
    SetSoundVolume(efeitos[som], tabela[som].volume * VOLUME_EFEITOS * (correndo ? 1.0f : 0.75f));
    tocarSomVariado(som, 0.12f);
}

/* ---------- ambiente ---------- */

void definirAmbiente(Ambiente ambiente)
{
    if (ambiente >= 0 && ambiente < TOTAL_AMBIENTES) {
        ambienteAtual = ambiente;
    }
}

void definirPiso(Piso piso)
{
    if (piso >= 0 && piso < TOTAL_PISOS) {
        pisoAtual = piso;
    }
}

void definirTensao(float nivel)
{
    if (nivel < 0) nivel = 0;
    if (nivel > 1) nivel = 1;
    tensaoAlvo = nivel;
}

static bool ehPlaneta(Tela tela)
{
    return tela == TELA_EXTRATERRESTRE || tela == TELA_GIGANTE || tela == TELA_ROBO ||
           tela == TELA_SER || tela == TELA_SEM_AR;
}

void sonsMudouDeTela(Tela anterior, Tela nova)
{
    definirTensao(0);
    switch (nova) {
        case TELA_NAVE:           definirAmbiente(AMBIENTE_NAVE);    definirPiso(PISO_METAL); break;
        case TELA_EXTRATERRESTRE: definirAmbiente(AMBIENTE_CIDADE);  definirPiso(PISO_ASFALTO); break;
        case TELA_GIGANTE:        definirAmbiente(AMBIENTE_GIGANTE); definirPiso(PISO_GRAMA); break;
        case TELA_ROBO:           definirAmbiente(AMBIENTE_ROBO);    definirPiso(PISO_METAL); break;
        case TELA_SER:            definirAmbiente(AMBIENTE_SER);     definirPiso(PISO_SANTUARIO); break;
        case TELA_SEM_AR:         definirAmbiente(AMBIENTE_SEM_AR);  definirPiso(PISO_POEIRA); break;
        default:                  definirAmbiente(AMBIENTE_NENHUM);  break;
    }
    if (anterior == TELA_NAVE && ehPlaneta(nova)) {
        tocarSom(SOM_DESCIDA);       /* a escotilha abre e o módulo pousa */
    } else if (ehPlaneta(anterior) && nova == TELA_NAVE) {
        tocarSom(SOM_RETORNO);
    } else if (nova == TELA_FINAL) {
        tocarSom(SOM_VITORIA);
    }
}

/* Leva `atual` na direção de `alvo`, no máximo `passo` por vez. */
static float aproximar(float atual, float alvo, float passo)
{
    if (atual < alvo) {
        atual += passo;
        if (atual > alvo) atual = alvo;
    } else if (atual > alvo) {
        atual -= passo;
        if (atual < alvo) atual = alvo;
    }
    return atual;
}

void atualizarSons(void)
{
    if (!pronto) {
        return;
    }
    float dt = GetFrameTime();
    if (dt > 0.1f) {
        dt = 0.1f;
    }

    /* Resposta às teclas de sempre: confirmar e voltar. As telas tocam
       os sons próprios de cada ação por cima destes. */
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E)) {
        tocarSom(SOM_UI_CONFIRMAR);
    } else if (IsKeyPressed(KEY_ESCAPE)) {
        tocarSom(SOM_UI_VOLTAR);
    }

    /* O ambiente do lugar atual sobe; os outros descem e param. */
    for (int i = 0; i < TOTAL_AMBIENTES; i++) {
        if (!temAmbiente[i]) {
            continue;
        }
        float alvo = (Ambiente)i == ambienteAtual ? 1.0f : 0.0f;
        volumeAmbiente[i] = aproximar(volumeAmbiente[i], alvo, dt * RAPIDEZ_TROCA);
        bool tocando = IsMusicStreamPlaying(ambientes[i]);
        if (volumeAmbiente[i] > 0 && !tocando) {
            PlayMusicStream(ambientes[i]);
        } else if (volumeAmbiente[i] <= 0 && tocando) {
            StopMusicStream(ambientes[i]);
        }
        if (volumeAmbiente[i] > 0) {
            SetMusicVolume(ambientes[i], volumeAmbiente[i] * VOLUME_AMBIENTE);
            UpdateMusicStream(ambientes[i]);
        }
    }
    /* No fliperama, a música do jogo dá lugar à da máquina. */
    definirVolumeMusica(ambienteAtual == AMBIENTE_FLIPERAMA ? 0.0f : 1.0f);

    /* Tensão do gigante: mais forte e mais rápida quanto mais furioso. */
    if (temTensao) {
        tensaoAgora = aproximar(tensaoAgora, tensaoAlvo, dt * 0.8f);
        bool tocando = IsMusicStreamPlaying(tensao);
        if (tensaoAgora > 0 && !tocando) {
            PlayMusicStream(tensao);
        } else if (tensaoAgora <= 0 && tocando) {
            StopMusicStream(tensao);
        }
        if (tensaoAgora > 0) {
            SetMusicVolume(tensao, tensaoAgora * VOLUME_TENSAO);
            SetMusicPitch(tensao, 0.8f + 0.7f * tensaoAgora);
            UpdateMusicStream(tensao);
        }
    }
}

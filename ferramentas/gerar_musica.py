#!/usr/bin/env python3
"""gerar_musica.py: compõe e grava a música de fundo do jogo.

Gera, a partir da pasta do projeto:
  assets/musica/tema.qoa   (cerca de 1 min e 50 s, em laço contínuo)

Rodar:  python3 ferramentas/gerar_musica.py
Precisa de numpy e scipy, e da raylib já compilada em build/ (o jogo
compilado uma vez com cmake). O jogo não precisa deste arquivo para
rodar: ele só é usado para refazer a música.

A música é sintetizada nota por nota: um órgão de tubos suave, uma
figura repetida de duas notas que lembra um relógio, um baixo grave e,
no meio, uma melodia aguda. Tudo passa por um eco longo de catedral.
Ela é calma de propósito: sem agudos fortes e sem batidas, para tocar
o jogo inteiro sem cansar.

O arquivo final é QOA, o formato de áudio comprimido que a raylib lê.
O script grava um WAV temporário em build/ e usa um programa pequeno
(ferramentas/wav_para_qoa.c) para converter.
"""

import os
import subprocess
import wave

import numpy as np
from scipy.signal import butter, fftconvolve, sosfilt

TAXA = 32000                 # amostras por segundo
BPM = 70
TEMPO = 60 / BPM             # um tempo (semínima), em segundos
COMPASSO = 4 * TEMPO
COMPASSOS = 32
DURACAO = COMPASSOS * COMPASSO
N = int(DURACAO * TAXA)
SAIDA = 'assets/musica/tema.qoa'

# Duas faixas (esquerda e direita). O que passa do fim volta para o
# começo, para o laço não ter emenda.
faixa = np.zeros((2, N))

NOTAS = {'C': -9, 'C#': -8, 'D': -7, 'D#': -6, 'E': -5, 'F': -4, 'F#': -3,
         'G': -2, 'G#': -1, 'A': 0, 'A#': 1, 'B': 2}


def freq(nome):
    """'A4' -> 440 Hz."""
    letra, oitava = nome[:-1], int(nome[-1])
    return 440.0 * 2 ** ((NOTAS[letra] + 12 * (oitava - 4)) / 12)


def envelope(n, ataque, soltura, sustentado):
    """Volume ao longo da nota: sobe, segura e some, tudo suave."""
    t = np.arange(n) / TAXA
    sobe = np.clip(t / max(ataque, 1e-3), 0, 1)
    sobe = 0.5 - 0.5 * np.cos(np.pi * sobe)
    desce = np.clip((t - sustentado) / max(soltura, 1e-3), 0, 1)
    desce = 0.5 + 0.5 * np.cos(np.pi * desce)
    return sobe * desce


def somar(sinal, inicio, pan):
    """Soma o sinal a partir de `inicio` (s); pan de -1 (esq.) a 1 (dir.)."""
    i = int(inicio * TAXA) % N
    esq = np.cos((pan + 1) * np.pi / 4)
    dir_ = np.sin((pan + 1) * np.pi / 4)
    pos = (i + np.arange(len(sinal))) % N
    np.add.at(faixa[0], pos, sinal * esq)
    np.add.at(faixa[1], pos, sinal * dir_)


def orgao(f, inicio, dur, vol, pan=0.0, ataque=1.6, soltura=2.4, brilho=1.0):
    """Órgão de tubos: soma de harmônicos, com duas vozes levemente
    desafinadas entre si (dá o "coro" dos tubos)."""
    n = int((dur + soltura) * TAXA)
    t = np.arange(n) / TAXA
    harmonicos = [(1, 1.0), (2, 0.55), (3, 0.22 * brilho), (4, 0.16 * brilho),
                  (6, 0.06 * brilho), (8, 0.035 * brilho)]
    env = envelope(n, ataque, soltura, dur)
    for desvio, lado in ((-0.0012, -0.35), (0.0012, 0.35)):
        onda = np.zeros(n)
        fase = np.random.default_rng(int(f * 10)).random() * 6.28
        for h, a in harmonicos:
            if f * h < 6000:
                onda += a * np.sin(2 * np.pi * f * h * (1 + desvio) * t + fase * h)
        somar(onda * env * vol * 0.5, inicio, np.clip(pan + lado, -1, 1))


def sino(f, inicio, vol, pan=0.0, soltura=1.6):
    """Nota curta e redonda, entre um piano abafado e uma celesta."""
    n = int(soltura * 1.6 * TAXA)
    t = np.arange(n) / TAXA
    env = np.clip(t / 0.006, 0, 1) * np.exp(-t / (soltura * 0.45))
    onda = np.sin(2 * np.pi * f * t) + 0.28 * np.sin(2 * np.pi * 2 * f * t) * np.exp(-t / 0.25) \
        + 0.08 * np.sin(2 * np.pi * 3 * f * t) * np.exp(-t / 0.12)
    somar(onda * env * vol, inicio, pan)


def tique(inicio, vol):
    """O tique de um relógio: um estalo bem baixo e abafado."""
    n = int(0.03 * TAXA)
    ruido = np.random.default_rng(int(inicio * 1000)).standard_normal(n)
    ruido = sosfilt(butter(2, [1800, 4200], 'bandpass', fs=TAXA, output='sos'), ruido)
    somar(ruido * np.exp(-np.arange(n) / (0.004 * TAXA)) * vol, inicio, 0.15)


# ---------- harmonia ----------
# Quatro acordes de dois compassos cada, em lá menor. A sequência se
# repete quatro vezes; o que muda é quem está tocando.
ACORDES = [
    # baixo, notas do órgão, as duas notas do relógio
    ('A2', ['A3', 'E4', 'B4', 'C5'], ('A4', 'E5')),
    ('F2', ['F3', 'C4', 'E4', 'A4'], ('A4', 'F5')),
    ('C3', ['G3', 'C4', 'E4', 'B4'], ('G4', 'E5')),
    ('E2', ['E3', 'A3', 'B3', 'E4'], ('B4', 'E5')),   # suspenso: pede para voltar ao lá
]

# Intensidade de cada camada em cada volta da sequência (0 a 1).
#            relógio  órgão  baixo  melodia  tique
CAMADAS = [(0.85, 0.45, 0.00, 0.0, 0.0),     # começo: só o relógio e um órgão leve
           (1.00, 0.80, 0.70, 0.0, 1.0),     # entra o órgão cheio e o baixo
           (1.00, 1.00, 1.00, 1.0, 1.0),     # a melodia aguda por cima
           (0.80, 0.55, 0.35, 0.0, 0.0)]     # volta a esvaziar, para emendar com o começo

# Melodia da terceira volta: (compasso dentro da volta, tempo, nota, duração em tempos)
MELODIA = [(0, 0, 'E5', 7), (2, 0, 'F5', 3), (2, 3, 'E5', 5), (4, 0, 'G5', 3),
           (4, 3, 'E5', 2), (4, 5, 'C5', 3), (6, 0, 'B4', 4), (7, 0, 'G#4', 4)]

for volta, (v_rel, v_org, v_bai, v_mel, v_tiq) in enumerate(CAMADAS):
    for k, (baixo, pad, relogio) in enumerate(ACORDES):
        compasso = volta * 8 + k * 2
        t0 = compasso * COMPASSO
        dur = 2 * COMPASSO
        # órgão: acorde sustentado, um pouco mais brilhante quando cheio
        for i, nota in enumerate(pad):
            orgao(freq(nota), t0, dur, 0.055 * v_org, pan=-0.3 + 0.2 * i, brilho=0.6 + 0.5 * v_org)
        if v_bai:
            orgao(freq(baixo), t0, dur, 0.11 * v_bai, ataque=2.2, soltura=2.8, brilho=0.4)
            orgao(freq(baixo) / 2, t0, dur, 0.07 * v_bai, ataque=2.6, soltura=3.0, brilho=0.2)
        # relógio: colcheias alternando as duas notas; no último
        # compasso do acorde suspenso, a nota de cima desce para o sol#
        a, b = relogio
        for c in range(16):
            nota = a if c % 2 == 0 else b
            if k == 3 and c >= 8 and nota == 'B4':
                nota = 'G#4'
            acento = 1.0 if c % 4 == 0 else 0.72
            sino(freq(nota), t0 + c * TEMPO / 2, 0.11 * v_rel * acento, pan=-0.25 if c % 2 == 0 else 0.25)
        if v_tiq:
            for c in range(8):
                tique(t0 + c * TEMPO, 0.012 * v_tiq)
    if v_mel:
        for comp, tempo_, nota, n_tempos in MELODIA:
            inicio = (volta * 8 + comp) * COMPASSO + tempo_ * TEMPO
            orgao(freq(nota) * 2, inicio, n_tempos * TEMPO, 0.03 * v_mel, pan=0.1,
                  ataque=0.9, soltura=2.5, brilho=0.35)
            orgao(freq(nota), inicio, n_tempos * TEMPO, 0.04 * v_mel, pan=-0.1,
                  ataque=0.9, soltura=2.5, brilho=0.5)

# ---------- eco de catedral ----------
# Resposta de uma sala grande: ruído que some devagar, mais abafado
# no fim, diferente em cada lado.
duracao_eco = 4.5
m = int(duracao_eco * TAXA)
t = np.arange(m) / TAXA
molhado = np.zeros_like(faixa)
for lado in range(2):
    ruido = np.random.default_rng(90 + lado).standard_normal(m) * np.exp(-t / 1.15)
    ruido = sosfilt(butter(2, 3200, 'lowpass', fs=TAXA, output='sos'), ruido)
    ruido /= np.sqrt(np.sum(ruido ** 2))
    # convolução circular: o eco do fim cai no começo do laço
    entrada = np.concatenate([faixa[lado], faixa[lado][:m]])
    eco = fftconvolve(entrada, ruido)[:N + m]
    eco[:m] += eco[N:N + m]
    molhado[lado] = eco[:N]
mix = 0.62 * faixa + 0.55 * molhado

# ---------- acabamento ----------
# Corta o que ficaria áspero (agudos e graves que só fazem vibrar a
# caixa de som) e arredonda os picos.
# Os filtros também dão a volta: começam já "aquecidos" com o fim da
# música, senão o laço teria um estalo na emenda.
def filtrar_em_laco(sos, x):
    k = 2 * TAXA
    return sosfilt(sos, np.concatenate([x[-k:], x]))[k:]


for lado in range(2):
    mix[lado] = filtrar_em_laco(butter(2, 6500, 'lowpass', fs=TAXA, output='sos'), mix[lado])
    mix[lado] = filtrar_em_laco(butter(1, 35, 'highpass', fs=TAXA, output='sos'), mix[lado])
mix /= np.sqrt(np.mean(mix ** 2)) / 0.09        # volume médio baixo (cerca de -21 dB)
mix = np.tanh(mix * 1.4) / 1.4
mix *= 0.85 / max(0.85, np.abs(mix).max())

os.makedirs('build', exist_ok=True)
os.makedirs(os.path.dirname(SAIDA), exist_ok=True)
temporario = 'build/tema.wav'
with wave.open(temporario, 'wb') as w:
    w.setnchannels(2)
    w.setsampwidth(2)
    w.setframerate(TAXA)
    w.writeframes((mix.T * 32767).astype('<i2').tobytes())

# ---------- conversão para QOA ----------
conversor = 'build/wav_para_qoa'
if not os.path.exists(conversor) or os.path.getmtime(conversor) < os.path.getmtime('ferramentas/wav_para_qoa.c'):
    subprocess.run(['cc', '-std=c99', '-Ibuild/_deps/raylib-src/src', 'ferramentas/wav_para_qoa.c',
                    'build/_deps/raylib-build/raylib/libraylib.a', '-lm', '-lpthread', '-ldl',
                    '-lGL', '-lX11', '-o', conversor], check=True)
subprocess.run([conversor, temporario, SAIDA], check=True)
print(f'{SAIDA}: {DURACAO:.1f} s, {os.path.getsize(SAIDA) / 1e6:.1f} MB')

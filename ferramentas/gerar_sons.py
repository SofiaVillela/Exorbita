#!/usr/bin/env python3
"""gerar_sons.py: sintetiza os efeitos sonoros e os sons de ambiente.

Gera, a partir da pasta do projeto, um arquivo por som em:
  assets/sons/*.qoa

Rodar:  python3 ferramentas/gerar_sons.py
Precisa de numpy e scipy, e da raylib já compilada em build/ (usa o
mesmo conversor de ferramentas/gerar_musica.py). O jogo não precisa
deste arquivo para rodar: ele só é usado para refazer os sons.

Tudo é feito com ondas simples (senos, ondas quadradas) e ruído
filtrado. Os nomes dos arquivos são os mesmos da tabela de
src/sons.c: som novo aqui precisa de uma linha lá.

São três famílias:
  - efeitos curtos (passos, portas, avisos, cartas, fliperama);
  - ambientes: laços de 16 s, um para cada lugar do jogo;
  - laços especiais: a tensão do gigante e a música do fliperama.
"""

import os
import subprocess
import wave

import numpy as np
from scipy.signal import butter, sosfilt

T = 32000                 # amostras por segundo dos efeitos
TA = 24000                # dos ambientes (são sons graves e suaves)
PASTA = 'assets/sons'
TEMPORARIOS = 'build/sons'
gerados = []


# ---------- peças ----------
def tempo(dur, taxa=T):
    return np.arange(int(dur * taxa)) / taxa


def ruido(dur, semente, taxa=T):
    return np.random.default_rng(semente).standard_normal(int(dur * taxa))


def filtro(x, tipo, corte, taxa=T, ordem=2):
    """tipo: 'lowpass', 'highpass' ou 'bandpass' (corte = [de, até])."""
    return sosfilt(butter(ordem, corte, tipo, fs=taxa, output='sos'), x)


def some(x, taxa=T, ataque=0.004, soltura=0.02):
    """Sobe e desce o volume nas pontas, para não estalar."""
    n = len(x)
    a, s = max(1, int(ataque * taxa)), max(1, int(soltura * taxa))
    env = np.ones(n)
    env[:a] = np.linspace(0, 1, a)
    env[-s:] *= np.linspace(1, 0, s)
    return x * env


def decai(dur, meia, taxa=T):
    """Volume que cai pela metade a cada `meia` segundos."""
    return 0.5 ** (tempo(dur, taxa) / meia)


def varre(f0, f1, dur, taxa=T, forma='seno'):
    """Nota que desliza de f0 a f1 (Hz)."""
    t = tempo(dur, taxa)
    f = f0 * (f1 / f0) ** (t / dur)
    fase = 2 * np.pi * np.cumsum(f) / taxa
    if forma == 'quadrada':
        return np.sign(np.sin(fase))
    return np.sin(fase)


def nota(f, dur, taxa=T, forma='seno'):
    return varre(f, f, dur, taxa, forma)


def sino(f, dur, meia=0.15):
    """Nota de sininho: seno com um harmônico que some rápido."""
    t = tempo(dur)
    return (np.sin(2 * np.pi * f * t) + 0.35 * np.sin(2 * np.pi * 2.01 * f * t) * 0.5 ** (t / 0.05)) \
        * 0.5 ** (t / meia)


def junta(*partes):
    """Soma sons que começam em instantes diferentes: (início em s, som)."""
    fim = max(int(inicio * T) + len(x) for inicio, x in partes)
    saida = np.zeros(fim)
    for inicio, x in partes:
        i = int(inicio * T)
        saida[i:i + len(x)] += x
    return saida


def em_laco(x, taxa, emenda=1.0):
    """Faz um som dar a volta sem emenda: os últimos `emenda` segundos
    são misturados com o começo. x precisa ter essa sobra no fim."""
    f = int(emenda * taxa)
    n = x.shape[-1] - f
    rampa = np.linspace(0, 1, f)
    y = x[..., :n].copy()
    y[..., :f] = x[..., :f] * rampa + x[..., n:n + f] * (1 - rampa)
    return y


def salvar(nome, x, pico=0.6, taxa=T):
    """Grava o som (mono, ou estéreo se x tiver duas linhas) e converte."""
    x = np.asarray(x, float)
    x = x / max(1e-9, np.abs(x).max()) * pico
    canais = 1 if x.ndim == 1 else 2
    dados = x if canais == 1 else x.T
    os.makedirs(TEMPORARIOS, exist_ok=True)
    caminho = f'{TEMPORARIOS}/{nome}.wav'
    with wave.open(caminho, 'wb') as w:
        w.setnchannels(canais)
        w.setsampwidth(2)
        w.setframerate(taxa)
        w.writeframes((dados * 32767).astype('<i2').tobytes())
    gerados.append(nome)


# =====================================================================
# 1. INTERFACE
# =====================================================================
salvar('ui_mover', some(nota(660, 0.05) * decai(0.05, 0.02)), 0.35)
salvar('ui_confirmar', junta((0, some(sino(520, 0.09, 0.04))), (0.05, some(sino(780, 0.14, 0.05)))), 0.4)
salvar('ui_voltar', junta((0, some(sino(520, 0.09, 0.04))), (0.05, some(sino(390, 0.14, 0.05)))), 0.4)
# aviso de "não pode": dois zumbidos graves
zumbido = some(filtro(nota(140, 0.11, forma='quadrada'), 'lowpass', 900), soltura=0.03)
salvar('negado', junta((0, zumbido), (0.15, zumbido)), 0.4)
# terminal aceitou: dois bipes agudos
salvar('terminal', junta((0, some(nota(1320, 0.07))), (0.1, some(nota(1760, 0.12) * decai(0.12, 0.06)))), 0.35)
# pegou um item: duas notas de vidro subindo
salvar('item', junta((0, sino(880, 0.2, 0.06)), (0.07, sino(1320, 0.35, 0.1))), 0.45)
# vitória: quatro notas subindo, a última sustentada
salvar('vitoria', junta((0, sino(523, 0.3, 0.12)), (0.13, sino(659, 0.3, 0.12)), (0.26, sino(784, 0.3, 0.12)),
                        (0.40, sino(1047, 1.0, 0.3)), (0.40, 0.5 * sino(784, 1.0, 0.3))), 0.5)
# derrota: três notas descendo, em menor
salvar('derrota', junta((0, sino(440, 0.4, 0.15)), (0.22, sino(349, 0.4, 0.15)), (0.44, sino(262, 1.1, 0.35)),
                        (0.44, 0.5 * sino(311, 1.1, 0.35))), 0.5)
# perda de um tripulante: acorde grave que cresce e some
t = tempo(3.2)
perda = sum(np.sin(2 * np.pi * f * t) * a for f, a in ((110, 1), (130.8, 0.7), (164.8, 0.5), (55, 0.8)))
perda *= np.clip(t / 0.8, 0, 1) * 0.5 ** (np.clip(t - 1.0, 0, 9) / 0.7)
salvar('perda', some(filtro(perda, 'lowpass', 900), soltura=0.3), 0.5)
# fala de alienígena: quatro sílabas trêmulas
silabas = []
for i, (f, d) in enumerate(((420, 0.07), (560, 0.06), (380, 0.09), (500, 0.07))):
    tt = tempo(d)
    silabas.append((i * 0.085, some(np.sin(2 * np.pi * f * tt + 3 * np.sin(2 * np.pi * 38 * tt)), soltura=0.02)))
salvar('fala_alien', filtro(junta(*silabas), 'lowpass', 2500), 0.3)

# =====================================================================
# 2. NAVE, PORTAS E VIAGENS
# =====================================================================
# descida: a escotilha solta ar, o motor ronca e o módulo toca o chão
chiado = filtro(ruido(0.6, 1), 'highpass', 2500) * decai(0.6, 0.12)
servo = some(filtro(varre(320, 170, 0.5, forma='quadrada'), 'lowpass', 1200), soltura=0.1) * 0.25
ronco = filtro(ruido(1.5, 2), 'lowpass', 220, ordem=3) * np.sin(np.pi * tempo(1.5) / 1.5) ** 0.7
batida = (nota(52, 0.5) + 0.4 * filtro(ruido(0.5, 3), 'lowpass', 500)) * decai(0.5, 0.09)
salvar('descida', junta((0, chiado * 0.6), (0.1, servo), (0.45, ronco * 2.5), (1.75, batida * 1.2)), 0.6)
# retorno: a escotilha fecha e o motor sobe
tranca = (nota(90, 0.25) + filtro(ruido(0.25, 4), 'bandpass', [600, 1800])) * decai(0.25, 0.04)
subida = filtro(ruido(1.7, 5), 'lowpass', 260, ordem=3) * (tempo(1.7) / 1.7) ** 1.5
assobio = varre(120, 420, 1.7) * (tempo(1.7) / 1.7) ** 2 * 0.25
salvar('retorno', some(junta((0, tranca), (0.3, subida * 2.5 + assobio)), soltura=0.25), 0.6)
# porta de vaivém do bar: rangido e as duas folhas batendo de volta
tt = tempo(0.32)
rangido = np.sin(2 * np.pi * np.cumsum(430 - 260 * tt + 30 * np.sin(2 * np.pi * 21 * tt)) / T)
rangido = some(filtro(np.sign(rangido) * 0.4 + rangido, 'bandpass', [300, 1600]), ataque=0.03, soltura=0.08)
folha = (nota(150, 0.12) + 0.6 * filtro(ruido(0.12, 6), 'lowpass', 900)) * decai(0.12, 0.03)
salvar('porta_bar', junta((0, rangido * 0.5), (0.3, folha), (0.47, folha * 0.5), (0.6, folha * 0.25)), 0.45)
# portão pesado da estação: arrasto de metal e trava no fim
arrasto = filtro(ruido(1.2, 7), 'lowpass', 180, ordem=3) * 2 + 0.5 * filtro(ruido(1.2, 8), 'bandpass', [700, 1500])
arrasto *= np.clip(tempo(1.2) / 0.1, 0, 1) * (0.7 + 0.3 * np.sin(2 * np.pi * 13 * tempo(1.2)))
trava = (nota(70, 0.4) + filtro(ruido(0.4, 9), 'bandpass', [400, 1400])) * decai(0.4, 0.06)
salvar('portao', junta((0, some(arrasto, soltura=0.05)), (1.15, trava * 1.5)), 0.6)

# =====================================================================
# 3. PASSOS (um por tipo de piso; o jogo varia o tom a cada passo)
# =====================================================================
def passo(grave, corpo, banda, dur, mistura):
    """Batida grave do pé mais o som do material."""
    pe = nota(grave, dur) * decai(dur, 0.018)
    material = filtro(ruido(dur, int(grave)), 'bandpass', banda) * decai(dur, corpo)
    return some(pe * (1 - mistura) + material * mistura, ataque=0.002, soltura=0.02)


salvar('passo_metal', passo(180, 0.02, [1500, 3200], 0.11, 0.5) + 0.25 * np.pad(sino(640, 0.11, 0.025), (0, 0)), 0.3)
salvar('passo_poeira', filtro(passo(90, 0.03, [150, 500], 0.14, 0.7), 'lowpass', 600), 0.3)
salvar('passo_grama', passo(120, 0.04, [2500, 6000], 0.15, 0.85), 0.26)
salvar('passo_asfalto', passo(140, 0.022, [700, 1800], 0.10, 0.55), 0.3)
salvar('passo_madeira', passo(170, 0.02, [500, 1100], 0.12, 0.4), 0.34)
salvar('passo_santuario', passo(130, 0.03, [900, 2000], 0.2, 0.35)[:int(0.2 * T)] + 0.12 * sino(1568, 0.2, 0.06), 0.26)

# =====================================================================
# 4. PLANETA DO GIGANTE
# =====================================================================
# pisou numa planta: folhas quebrando e duas notas tristes
estalos = junta(*[(0.03 * i, filtro(ruido(0.04, 20 + i), 'bandpass', [2200, 5500]) * decai(0.04, 0.008))
                  for i in range(6)])
salvar('planta', junta((0, estalos), (0.08, 0.5 * sino(784, 0.2, 0.07)), (0.2, 0.5 * sino(622, 0.35, 0.1))), 0.4)
# esbarrou num habitante: guincho de susto
tt = tempo(0.3)
guincho = np.sin(2 * np.pi * np.cumsum(900 + 700 * np.sin(np.pi * tt / 0.3) + 60 * np.sin(2 * np.pi * 30 * tt)) / T)
salvar('habitante', some(guincho, ataque=0.01, soltura=0.08), 0.35)
# o gigante reage: rosnado grave que treme, com pedras rolando
tt = tempo(2.6)
rosnado = sum(np.sin(2 * np.pi * f * tt + 2 * np.sin(2 * np.pi * 7 * tt)) for f in (46, 69, 92)) \
    * (0.6 + 0.4 * np.sin(2 * np.pi * 9 * tt))
pedras = filtro(ruido(2.6, 30), 'lowpass', 300, ordem=3) * 3
reage = (rosnado + pedras) * np.clip(tt / 0.25, 0, 1) * 0.5 ** (np.clip(tt - 1.2, 0, 9) / 0.5)
salvar('gigante_reage', some(filtro(reage, 'lowpass', 700), soltura=0.3), 0.7)
# o gigante pega o tripulante: a mão desce (vento), bate e um acorde grave fica
vento = filtro(ruido(0.6, 31), 'bandpass', [300, 1500]) * (tempo(0.6) / 0.6) ** 2
impacto = (varre(70, 34, 1.4) * 1.5 + filtro(ruido(1.4, 32), 'lowpass', 400)) * decai(1.4, 0.25)
tt = tempo(2.0)
acorde = sum(np.sin(2 * np.pi * f * tt) + 0.3 * np.sin(2 * np.pi * 2 * f * tt) for f in (55, 65.4, 82.4)) \
    * np.clip(tt / 0.15, 0, 1) * 0.5 ** (tt / 0.6)
salvar('gigante_pega', some(junta((0, vento * 0.7), (0.55, impacto), (0.6, acorde * 0.5)), soltura=0.3), 0.75)
# o gigante se acalma: um suspiro longo e uma nota grave que descansa
tt = tempo(1.8)
suspiro = filtro(ruido(1.8, 33), 'bandpass', [200, 900]) * np.sin(np.pi * tt / 1.8) ** 2
descanso = (np.sin(2 * np.pi * np.cumsum(98 - 16 * tt / 1.8) / T) + 0.5 * np.sin(2 * np.pi * 164.8 * tt)) \
    * np.sin(np.pi * tt / 1.8)
salvar('gigante_acalma', suspiro * 0.6 + descanso * 0.5, 0.45)

# =====================================================================
# 5. PLANETA SEM AR
# =====================================================================
# cilindro de oxigênio: o ar entra no traje e um bipe confirma
ar = filtro(ruido(0.6, 40), 'bandpass', [1200, 5000]) * np.sin(np.pi * tempo(0.6) / 0.6)
salvar('oxigenio', junta((0, ar * 0.5), (0.5, sino(988, 0.3, 0.1))), 0.4)
# o traje vazando no jato de gás: chiado forte e alarme
vaza = filtro(ruido(0.9, 41), 'highpass', 3000) * decai(0.9, 0.3)
alerta = some(nota(880, 0.1, forma='quadrada')) * 0.3
salvar('vazamento', junta((0, vaza), (0.05, alerta), (0.25, alerta)), 0.4)
# oxigênio baixo: bipe duplo
bipe = some(nota(740, 0.09))
salvar('alarme', junta((0, bipe), (0.16, bipe)), 0.3)
# largar um item na poeira
salvar('largar', filtro((nota(80, 0.2) + filtro(ruido(0.2, 42), 'lowpass', 500)) * decai(0.2, 0.04), 'lowpass', 700), 0.4)

# =====================================================================
# 6. PLANETA DO SER
# =====================================================================
# o Ser aparece: vozes de vidro que crescem devagar
tt = tempo(3.5)
vozes = sum(np.sin(2 * np.pi * f * tt) * (0.6 + 0.4 * np.sin(2 * np.pi * v * tt))
            for f, v in ((440, 0.9), (659.3, 1.3), (880, 1.7), (1108.7, 2.3), (1318.5, 0.6)))
salvar('ser_aparece', vozes * np.sin(np.pi * tt / 3.5) ** 1.5, 0.4)
# desejo atendido: sininhos subindo
salvar('desejo', junta(*[(i * 0.09, sino(f, 1.2 - i * 0.09, 0.3))
                         for i, f in enumerate((659, 880, 1109, 1319, 1760, 2217))]), 0.4)

# =====================================================================
# 7. LABORATÓRIO DO ROBÔ
# =====================================================================
salvar('robo_comando', some(filtro(nota(520, 0.06, forma='quadrada'), 'lowpass', 2500)), 0.25)
salvar('robo_apaga', some(filtro(nota(300, 0.07, forma='quadrada'), 'lowpass', 2000)), 0.25)
motor = filtro(varre(180, 240, 0.12, forma='quadrada'), 'lowpass', 1500)
salvar('robo_passo', junta((0, some(motor, soltura=0.03)), (0.11, some(nota(900, 0.03)) * 0.6)), 0.28)

# =====================================================================
# 8. CARTAS
# =====================================================================
salvar('carta_mexe', filtro(ruido(0.05, 50), 'highpass', 2500) * decai(0.05, 0.012), 0.3)
tapa = filtro(ruido(0.12, 51), 'bandpass', [500, 3500]) * decai(0.12, 0.02) + nota(120, 0.12) * decai(0.12, 0.02)
salvar('carta_joga', junta((0, filtro(ruido(0.06, 52), 'highpass', 3000) * 0.4), (0.05, tapa)), 0.4)
salvar('rodada_ganha', junta((0, sino(659, 0.2, 0.08)), (0.1, sino(880, 0.45, 0.14))), 0.4)
salvar('rodada_perde', junta((0, sino(494, 0.2, 0.08)), (0.1, sino(370, 0.45, 0.14))), 0.4)

# =====================================================================
# 9. FLIPERAMA (ondas quadradas, como nas máquinas antigas)
# =====================================================================
def quadrada(f, dur, meia=None):
    x = filtro(nota(f, dur, forma='quadrada'), 'lowpass', 5000)
    if meia:
        x = x * decai(dur, meia)
    return some(x, ataque=0.002, soltura=0.01)


def ruido_antigo(dur, passo_, semente):
    """Ruído "em degraus": cada valor dura alguns instantes, como nos chips antigos."""
    n = int(dur * T)
    valores = np.random.default_rng(semente).uniform(-1, 1, n // passo_ + 1)
    return np.repeat(valores, passo_)[:n]


salvar('flip_tiro', some(filtro(varre(1300, 320, 0.09, forma='quadrada'), 'lowpass', 5000), soltura=0.02), 0.22)
salvar('flip_explosao', ruido_antigo(0.3, 9, 60) * decai(0.3, 0.07), 0.3)
salvar('flip_atingido', junta((0, ruido_antigo(0.7, 14, 61) * decai(0.7, 0.18)),
                              (0, some(filtro(varre(500, 60, 0.7, forma='quadrada'), 'lowpass', 3000)) * 0.6)), 0.34)
salvar('flip_onda', junta(*[(i * 0.08, quadrada(f, 0.1 if i < 4 else 0.3, 0.1))
                            for i, f in enumerate((440, 523, 659, 880, 1047))]), 0.25)
salvar('flip_fim', junta(*[(i * 0.22, quadrada(f, 0.24 if i < 3 else 0.7, 0.2))
                           for i, f in enumerate((440, 392, 330, 220))]), 0.28)
salvar('flip_inicio', junta((0, quadrada(988, 0.07)), (0.07, quadrada(1319, 0.3, 0.1))), 0.25)

# música do fliperama: 8 compassos em lá menor, bem ritmada
BPM = 150
S = 60 / BPM / 2                      # uma colcheia
ESCALA = {'A3': 220, 'C4': 261.6, 'D4': 293.7, 'E4': 329.6, 'G4': 392, 'A4': 440, 'B4': 493.9,
          'C5': 523.3, 'D5': 587.3, 'E5': 659.3, 'G5': 784, 'A5': 880, 'F4': 349.2, 'F5': 698.5,
          'A2': 110, 'F2': 87.3, 'C3': 130.8, 'G2': 98, 'E3': 164.8, 'G3': 196, 'F3': 174.6}
MELODIA = ['A4', 'E5', 'A5', 'E5', 'C5', 'E5', 'A4', 'E5',   'F4', 'C5', 'F5', 'C5', 'A4', 'C5', 'F4', 'C5',
           'C5', 'G5', 'E5', 'G5', 'C5', 'E5', 'G4', 'E5',   'G4', 'D5', 'G5', 'D5', 'B4', 'D5', 'G4', 'B4',
           'A4', 'E5', 'A5', 'E5', 'C5', 'E5', 'A5', 'G5',   'F5', 'C5', 'A4', 'C5', 'F5', 'E5', 'D5', 'C5',
           'E5', 'G5', 'C5', 'E5', 'G4', 'C5', 'E5', 'G5',   'D5', 'B4', 'G4', 'B4', 'D5', 'G5', 'B4', 'D5']
BAIXO = ['A2', 'F2', 'C3', 'G2', 'A2', 'F2', 'C3', 'G2']
partes = []
for i, n in enumerate(MELODIA):
    partes.append((i * S, quadrada(ESCALA[n], S * 0.9, 0.12) * 0.5))
for compasso, n in enumerate(BAIXO):
    for k in range(8):                # baixo em onda triangular, saltando de oitava
        f = ESCALA[n] * (2 if k % 2 else 1)
        tri = np.arcsin(np.sin(2 * np.pi * f * tempo(S * 0.85))) * 2 / np.pi
        partes.append(((compasso * 8 + k) * S, some(tri, soltura=0.02) * 0.8))
        if k % 2 == 1:                # chimbal no contratempo
            partes.append(((compasso * 8 + k) * S, ruido_antigo(0.04, 2, compasso * 8 + k) * decai(0.04, 0.01) * 0.25))
        if k % 4 == 0:                # bumbo
            partes.append(((compasso * 8 + k) * S, varre(160, 50, 0.1) * decai(0.1, 0.03) * 0.9))
chip = junta(*partes)
chip = np.pad(chip, (0, max(0, int(64 * S * T) - len(chip))))[:int(64 * S * T)]
salvar('flip_musica', filtro(chip, 'lowpass', 6000), 0.3)

# =====================================================================
# 10. LAÇOS DE AMBIENTE (16 s, estéreo)
# =====================================================================
D = 16.0                              # duração do laço
E = 1.5                               # sobra para a emenda
N = int((D + E) * TA)
ta = np.arange(N) / TA


def r2(semente):
    """Ruído estéreo (um para cada lado)."""
    g = np.random.default_rng(semente)
    return g.standard_normal((2, N))


def f2(x, tipo, corte, ordem=2):
    return np.stack([filtro(x[0], tipo, corte, TA, ordem), filtro(x[1], tipo, corte, TA, ordem)])


def lento(periodo, fase=0.0, minimo=0.0):
    """Volume que sobe e desce devagar (cabe um número inteiro de vezes no laço)."""
    return minimo + (1 - minimo) * (0.5 + 0.5 * np.sin(2 * np.pi * ta / periodo + fase))


def evento(x, inicio, pan=0.0, taxa=TA):
    """Coloca um som curto num instante do laço, mais para um lado."""
    saida = np.zeros((2, N))
    i = int(inicio * taxa)
    saida[0, i:i + len(x)] = x * np.cos((pan + 1) * np.pi / 4)
    saida[1, i:i + len(x)] = x * np.sin((pan + 1) * np.pi / 4)
    return saida


def bip(f, dur, meia=0.05):
    t_ = tempo(dur, TA)
    return some(np.sin(2 * np.pi * f * t_) * 0.5 ** (t_ / meia), TA)


def ambiente(nome, x, pico):
    salvar(nome, em_laco(x, TA, E), pico, TA)


# nave: o motor ao fundo, o ar circulando e bipes dos consoles
motor_ = np.sin(2 * np.pi * 55 * ta) * lento(8) + 0.5 * np.sin(2 * np.pi * 110.5 * ta) * lento(16, 1, 0.4) \
    + 0.3 * np.sin(2 * np.pi * 165 * ta) * lento(4, 2, 0.3)
x = np.stack([motor_, motor_]) * 0.5 + f2(r2(70), 'lowpass', 700) * 0.6 * lento(16, 0.5, 0.6)
for inicio, f, pan in ((2.1, 1480, -0.6), (2.3, 1760, -0.6), (7.4, 1175, 0.5), (11.0, 1568, 0.2),
                       (11.15, 1568, 0.2), (13.8, 1320, -0.3)):
    x += evento(bip(f, 0.12, 0.04), inicio, pan) * 0.12
ambiente('amb_nave', x, 0.3)

# cidade: ronco distante, zumbido de neon que falha e um tom longínquo
neon = np.sign(np.sin(2 * np.pi * 100 * ta)) * (lento(4, 0, 0.5) * (np.sin(2 * np.pi * 0.37 * ta * 17) > -0.8))
x = f2(r2(71), 'lowpass', 300, 3) * 2.2 * lento(16, 0, 0.6) + f2(np.stack([neon, neon * 0.6]), 'bandpass', [900, 2400]) * 0.05
sirene = np.sin(2 * np.pi * np.cumsum(330 + 40 * np.sin(2 * np.pi * ta / 4)) / TA) * lento(16, 3) ** 3
x += np.stack([sirene, sirene * 0.5]) * 0.05
for inicio, pan in ((3.0, 0.7), (3.25, 0.7), (9.6, -0.5), (12.9, 0.3)):      # goteiras
    x += evento(bip(1900, 0.08, 0.015), inicio, pan) * 0.06
ambiente('amb_cidade', x, 0.26)

# bar: vozes misturadas (ruído "falando"), copos e um baixo de vitrola
x = np.zeros((2, N))
g = np.random.default_rng(72)
for voz in range(7):
    silabas_ = np.clip(filtro(g.standard_normal(N), 'lowpass', 4 + voz * 0.4, TA), 0, None)
    silabas_ = silabas_ / silabas_.max()
    centro = 240 + 70 * voz
    fala_ = filtro(g.standard_normal(N), 'bandpass', [centro, centro * 1.7], TA) * silabas_
    pan = -0.8 + 1.6 * voz / 6
    x[0] += fala_ * np.cos((pan + 1) * np.pi / 4)
    x[1] += fala_ * np.sin((pan + 1) * np.pi / 4)
x *= 1.6
for inicio, f, pan in ((1.7, 2600, -0.5), (5.9, 3100, 0.6), (6.05, 2900, 0.6), (10.3, 2400, 0.1), (14.2, 3300, -0.7)):
    x += evento(bip(f, 0.25, 0.05), inicio, pan) * 0.12
for i, f in enumerate((55, 55, 65.4, 82.4, 55, 55, 98, 82.4) * 2):          # baixo: uma nota por segundo
    t_ = tempo(0.7, TA)
    x += evento(np.sin(2 * np.pi * f * t_) * 0.5 ** (t_ / 0.2) * np.clip(t_ / 0.01, 0, 1), i * 1.0 + 0.02) * 0.5
ambiente('amb_bar', x, 0.3)

# sem ar: só a respiração dentro do traje e um zumbido do equipamento
ciclo = 4.0                                          # um fôlego a cada 4 s
fase_ = (ta % ciclo) / ciclo
inspira = np.where(fase_ < 0.4, np.sin(np.pi * fase_ / 0.4) ** 2, 0)
expira = np.where((fase_ > 0.48) & (fase_ < 0.95), np.clip(np.sin(np.pi * (fase_ - 0.48) / 0.47), 0, None) ** 1.5, 0)
sopro = r2(73)
x = f2(sopro, 'bandpass', [900, 2600]) * inspira * 0.9 + f2(sopro, 'bandpass', [350, 1300]) * expira * 1.1
x += np.stack([np.sin(2 * np.pi * 50 * ta), np.sin(2 * np.pi * 50.3 * ta)]) * 0.12
ambiente('amb_sem_ar', x, 0.22)

# gigante: vento em rajadas, bichos que cricrilam e a respiração enorme ao longe
rajada = lento(16, 0, 0.35) * lento(8, 1, 0.5)
x = f2(r2(74), 'bandpass', [180, 900]) * rajada * 1.4
resp = np.sin(2 * np.pi * 43 * ta) * lento(8, 4) ** 2
x += np.stack([resp, resp]) * 0.5
g = np.random.default_rng(75)
for inicio in (1.2, 4.6, 6.3, 9.9, 12.4, 14.0):
    f_, pan = g.uniform(3000, 4200), g.uniform(-0.8, 0.8)
    for k in range(int(g.integers(3, 7))):
        x += evento(bip(f_, 0.035, 0.012), inicio + k * 0.07, pan) * 0.05
ambiente('amb_gigante', x, 0.3)

# Ser: notas de vidro que entram e saem, vento fino e sininhos
x = f2(r2(76), 'bandpass', [1500, 4000]) * 0.1 * lento(16, 0, 0.5)
for f, per, fs in ((880, 8, 0), (1318.5, 16, 1), (1108.7, 16, 3), (1760, 8, 2), (659.3, 16, 4.5)):
    tom = np.sin(2 * np.pi * f * ta) * lento(per, fs) ** 2
    x += np.stack([tom * (0.5 + 0.5 * np.sin(fs)), tom * (0.5 + 0.5 * np.cos(fs))]) * 0.16
for inicio, f, pan in ((2.6, 2637, 0.6), (7.2, 3520, -0.5), (7.5, 2960, -0.3), (12.8, 2217, 0.2)):
    x += evento(bip(f, 0.9, 0.2), inicio, pan) * 0.07
ambiente('amb_ser', x, 0.22)

# laboratório do robô: zumbido elétrico, ventoinha e computadores conversando
x = np.stack([np.sin(2 * np.pi * 100 * ta) + 0.4 * np.sign(np.sin(2 * np.pi * 50 * ta)),
              np.sin(2 * np.pi * 100.4 * ta) + 0.4 * np.sign(np.sin(2 * np.pi * 50 * ta))]) * 0.12
x = f2(x, 'lowpass', 900) + f2(r2(77), 'bandpass', [400, 1600]) * 0.35
g = np.random.default_rng(78)
for inicio in (1.0, 5.2, 8.8, 12.6):
    pan = g.uniform(-0.7, 0.7)
    for k in range(int(g.integers(4, 9))):
        x += evento(bip(g.choice([988, 1175, 1480, 1760, 2093]), 0.05, 0.02), inicio + k * 0.075, pan) * 0.07
ambiente('amb_robo', x, 0.26)

# tensão do gigante: zumbido grave com batidas, como um coração enorme.
# O jogo aumenta o volume e a velocidade conforme ele fica furioso.
DT = 4.0
NT = int((DT + E) * TA)
tt = np.arange(NT) / TA
base = np.sin(2 * np.pi * 55 * tt) + 0.7 * np.sin(2 * np.pi * 58 * tt) + 0.4 * np.sin(2 * np.pi * 82.5 * tt)
tensao = base * 0.5
for inicio in np.arange(0, DT + E - 0.5, 1.0):            # tum-tum a cada segundo
    for atraso, forca in ((0, 1.0), (0.22, 0.6)):
        i = int((inicio + atraso) * TA)
        t_ = tempo(0.3, TA)
        pulso = np.sin(2 * np.pi * np.cumsum(70 - 90 * t_) / TA) * 0.5 ** (t_ / 0.05) * forca
        tensao[i:i + len(pulso)] += pulso[:max(0, NT - i)] * 1.6
salvar('gigante_tensao', em_laco(np.stack([tensao, tensao]), TA, E), 0.5, TA)

# =====================================================================
# conversão para QOA
# =====================================================================
conversor = 'build/wav_para_qoa'
if not os.path.exists(conversor):
    subprocess.run(['cc', '-std=c99', '-Ibuild/_deps/raylib-src/src', 'ferramentas/wav_para_qoa.c',
                    'build/_deps/raylib-build/raylib/libraylib.a', '-lm', '-lpthread', '-ldl',
                    '-lGL', '-lX11', '-o', conversor], check=True)
os.makedirs(PASTA, exist_ok=True)
total = 0
for nome in gerados:
    subprocess.run([conversor, f'{TEMPORARIOS}/{nome}.wav', f'{PASTA}/{nome}.qoa'], check=True)
    total += os.path.getsize(f'{PASTA}/{nome}.qoa')
print(f'{len(gerados)} sons em {PASTA}/ ({total / 1e6:.1f} MB)')

#!/usr/bin/env python3
"""gerar_extraterrestre.py: desenha os gráficos do planeta extraterrestre.

Gera, a partir da pasta do projeto:
  assets/imagens/planetas/et_rua.png   (a cidade vista de cima, 1280x720)
  assets/mascaras/et_rua_mascara.png   (branco = piso, 1280x720)
  assets/imagens/planetas/et_bar.png   (o bar por dentro, 640x360)
  assets/mascaras/et_bar_mascara.png   (branco = piso, 640x360)
  assets/imagens/planetas/et_bar_frente.png  (só os móveis do bar)
  assets/imagens/planetas/et_mesa.png  (mesa de cartas: fundo em cima,
                                        tampo em baixo, 640x720)
  assets/imagens/et_aliens.png         (alienígenas, quadros de 44x58)
  assets/imagens/et_objetos.png        (poste de luz, 24x72)
  assets/imagens/et_cartas.png         (cartas, 76x104 cada)
  assets/imagens/et_naves.png          (fliperama: naves e explosões, 32x32 cada)
  assets/imagens/planetas/et_fliperama.png  (o gabinete do fliperama, com a
                                        tela transparente, 640x360)

Rodar:  python3 ferramentas/gerar_extraterrestre.py
Precisa de numpy e Pillow. O jogo não precisa deste arquivo para rodar:
ele só é usado para refazer as imagens.

Como em gerar_sem_ar.py, tudo é desenhado pequeno e ampliado 2x. As
posições (portas, postes, mesa) precisam bater com as de
src/tela_extraterrestre.c, que usa as mesmas medidas multiplicadas por 2.
"""

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import numpy as np
from PIL import Image, ImageDraw, ImageFont

from pixel_arte import Cena, Tela, ampliar, cor, desenhar_modulo, figura

FONTE = 'assets/fontes/PressStart2P-Regular.ttf'

# ---------- posições da rua (grade de 640x360) ----------
RUA_CIMA = (86, 132)            # y de onde a rua de cima começa e acaba
RUA_BAIXO = (214, 264)          # rua principal
PRACA = (298, 380)              # x do vão entre os prédios do meio (praça do holograma)
BECO = (146, 182)               # x do beco, à esquerda
PATIO = (16, 154, 346)          # pátio de pouso: x0, x1 e y do muro de baixo
MODULO_X, MODULO_BASE = 84, 326
PORTA_BAR = (538, 558)          # x da porta do bar, na rua de cima
HOLOGRAMA = (339, 172)          # centro da base do holograma
POSTES = [(212, 128), (430, 128), (60, 260), (236, 260), (430, 260), (590, 260)]

# ---------- cores ----------
CONTORNO = '#16121f'
CIANO, CIANO_ESCURO, CIANO_CLARO = '#4fe0d8', '#2b8a8f', '#b9f5ee'
LARANJA, LARANJA_CLARO, LARANJA_ESCURO = '#f2a541', '#ffd98a', '#b06a26'
MAGENTA, MAGENTA_CLARO, MAGENTA_ESCURO = '#d0489a', '#f08cc8', '#7a2a66'
VERDE, VERDE_ESCURO = '#6fbf5a', '#3f7a45'

# Cada prédio usa uma destas paletas (parede da frente e telhado).
PREDIOS = [
    dict(parede='#4a4366', sombra='#3a3452', teto='#5d5580', teto_claro='#746c9a', teto_escuro='#4c4570'),
    dict(parede='#3f4263', sombra='#31344f', teto='#515780', teto_claro='#67709a', teto_escuro='#42476b'),
    dict(parede='#54405f', sombra='#42314c', teto='#6a5278', teto_claro='#836a92', teto_escuro='#574265'),
    dict(parede='#3b3a55', sombra='#2e2d44', teto='#4b4a6c', teto_claro='#5f5e85', teto_escuro='#3e3d5a'),
]

# =====================================================================
# 1. A RUA
# =====================================================================
rua = Cena(640, 360, '#2c2839')
xx, yy, bayer = rua.xx, rua.yy, rua.bayer

# ---------- chão ----------
# Asfalto em placas, com manchas mais claras e mais escuras.
n = 0.6 * rua.ruido(40, 1) + 0.4 * rua.ruido(12, 2)
tom = np.digitize(n + (bayer - 0.5) * 0.08, [0.40, 0.58])
rua.img[:] = np.array([cor('#282435'), cor('#2e2a3d'), cor('#353046')])[tom]
fiada = yy // 12
junta = ((xx + (fiada % 2) * 12) % 24 == 0) | (yy % 12 == 0)
rua.pintar(junta, '#221e2e')
rua.pintar(np.roll(junta, 1, 0) & ~junta & (bayer < 0.4), '#3a354c')
s = rua.sorteio(3)
rua.pintar(s > 0.992, '#3f3a52')
rua.pintar(s < 0.006, '#1e1a29')

# Praça do holograma e pátio de pouso: piso mais claro, em ladrilhos.
praca = rua.area(PRACA[0], RUA_CIMA[1], PRACA[1] - PRACA[0], RUA_BAIXO[0] - RUA_CIMA[1])
patio = rua.area(PATIO[0], RUA_BAIXO[1], PATIO[1] - PATIO[0], PATIO[2] - RUA_BAIXO[1])
for zona, c1, c2 in ((praca, '#3d3852', '#353049'), (patio, '#3a364e', '#322e45')):
    rua.pintar(zona, c1)
    rua.pintar(zona & (((xx // 8) + (yy // 8)) % 2 == 0), c2)
    rua.pintar(zona & ((xx % 8 == 0) | (yy % 8 == 0)), '#2a2639')

# Guia de luz no meio da rua principal: leva do pátio até a praça.
ymeio = (RUA_BAIXO[0] + RUA_BAIXO[1]) // 2 + 2
for x in range(24, 620):
    if x % 12 < 6:
        rua.ret(x, ymeio, 1, 1, CIANO_ESCURO)
for x in range(30, 620, 48):
    rua.ret(x, ymeio, 3, 1, CIANO)
    rua.luz(x + 1, ymeio, 9, 4, CIANO, 0.35)

# Círculo de pouso pintado no pátio.
for raio, c in ((40, LARANJA_ESCURO), (30, '#5a4a3a')):
    anel = rua.elipse(MODULO_X, MODULO_BASE - 2, raio, raio * 0.5) & \
        ~rua.elipse(MODULO_X, MODULO_BASE - 2, raio - 1.6, raio * 0.5 - 1)
    ang = np.arctan2((yy - MODULO_BASE + 2) * 2, xx - MODULO_X)
    rua.pintar(anel & patio & ((ang * 6 / np.pi).astype(int) % 2 == 0), c)

# Poças: refletem as luzes da cidade.
for i, (px, py, rx, ry) in enumerate([(120, 244, 14, 5), (410, 238, 18, 5), (520, 108, 16, 5),
                                      (250, 112, 12, 4), (340, 150, 10, 4), (196, 246, 9, 3),
                                      (600, 244, 11, 4), (64, 118, 10, 4)]):
    ang = np.arctan2((yy - py) / ry, (xx - px) / rx)
    raio = 1 + 0.2 * np.sin(3 * ang + i) + 0.12 * np.sin(5 * ang + i * 2)
    poca = ((xx - px) / rx) ** 2 + ((yy - py) / ry) ** 2 <= raio ** 2
    rua.pintar(poca, '#1c2033')
    rua.pintar(poca & ~np.roll(poca, 1, 0), '#151827')
    reflexo = [MAGENTA_ESCURO, CIANO_ESCURO, '#3a3f66'][i % 3]
    rua.pintar(poca & (yy % 3 == 0) & (np.abs(xx - px) < rx * 0.45) & (bayer < 0.5), reflexo)


# ---------- prédios ----------
def predio(x0, x1, ybase, fundo, altura, p, semente, friso=CIANO_ESCURO):
    """Prédio visto de cima e de frente: telhado em cima, parede embaixo.

    A base no chão vai de (ybase - fundo) até ybase. O telhado é a base
    deslocada `altura` para cima; a parede da frente fica entre os dois.
    Tudo bloqueia a passagem. Devolve o y de onde a parede começa.
    """
    w = x1 - x0
    ytopo = ybase - fundo - altura
    yparede = ybase - altura
    r = np.random.default_rng(semente)

    # telhado
    rua.ret(x0, ytopo, w, fundo, p['teto'])
    teto = rua.area(x0, ytopo, w, fundo)
    manchas = rua.ruido(14, semente)
    rua.pontilhar(teto & (manchas > 0.70), p['teto_claro'], 0.5)
    rua.pontilhar(teto & (manchas < 0.28), p['teto_escuro'], 0.5)
    rua.pintar(teto & (((xx - x0) % 18 == 17) | ((yy - ytopo) % 14 == 13)), p['teto_escuro'])
    # mureta em volta do telhado
    rua.ret(x0, ytopo, w, 2, p['teto_claro'])
    rua.ret(x0, ytopo, 2, fundo, p['teto_claro'])
    rua.ret(x1 - 2, ytopo, 2, fundo, p['teto_escuro'])
    rua.ret(x0 + 2, ytopo + 2, w - 4, 1, p['sombra'])
    rua.ret(x0 + 2, ytopo + 2, 1, fundo - 4, p['sombra'])
    rua.ret(x0, yparede - 2, w, 2, p['teto_claro'])
    rua.ret(x0, yparede - 3, w, 1, p['teto_escuro'])

    # parede da frente
    rua.ret(x0, yparede, w, altura, p['parede'])
    parede = rua.area(x0, yparede, w, altura)
    rua.pintar(parede & ((xx - x0) % 22 == 21), p['sombra'])
    rua.ret(x0, yparede, w, 2, CONTORNO)                     # sombra da beirada do telhado
    rua.pontilhar(rua.area(x0, yparede + 2, w, 2), p['sombra'], 0.5)
    rua.ret(x0, ybase - 3, w, 3, p['sombra'])                # rodapé
    rua.ret(x0, ybase - 4, w, 1, CONTORNO)
    if friso:
        for x in range(x0 + 4, x1 - 4):
            if (x - x0) % 22 not in (20, 21, 0):
                rua.ret(x, yparede + 5, 1, 1, friso)
    # sujeira que escorre
    for _ in range(w // 9):
        x, comprimento = int(r.integers(x0 + 2, x1 - 2)), int(r.integers(3, 9))
        rua.pontilhar(rua.area(x, yparede + 7, 1, comprimento), p['sombra'], 0.6)

    rua.ret(x0, ytopo, 1, fundo + altura, CONTORNO)
    rua.ret(x1 - 1, ytopo, 1, fundo + altura, CONTORNO)
    rua.ret(x0, ytopo - 1, w, 1, CONTORNO)
    rua.bloquear(x0, ytopo, w, fundo + altura)
    return yparede


def sombra_no_teto(x, y, w, h):
    rua.escurecer(rua.area(x + 2, y + h, w, 2) | rua.area(x + w, y + 2, 2, h), 0.4)


def ar_condicionado(x, y):
    sombra_no_teto(x, y, 12, 8)
    rua.ret(x, y, 12, 8, '#8a84a0')
    rua.ret(x, y, 12, 1, '#b8b2cc')
    rua.ret(x, y + 7, 12, 1, '#55506a')
    rua.pintar(rua.elipse(x + 4, y + 4, 2.6, 2.6), '#2b2740')
    rua.ret(x + 4, y + 4, 1, 1, '#8a84a0')
    for k in range(3):
        rua.ret(x + 8, y + 2 + k * 2, 3, 1, '#55506a')


def claraboia(x, y, w, h, c=CIANO):
    rua.ret(x, y, w, h, CONTORNO)
    rua.ret(x + 1, y + 1, w - 2, h - 2, '#1d3a48' if c == CIANO else '#5a3a22')
    for k in range(y + 2, y + h - 1, 2):
        rua.ret(x + 2, k, w - 4, 1, c)
    rua.luz(x + w / 2, y + h / 2, w * 0.9, h * 1.2, c, 0.3)


def grade(x, y, w, h):
    rua.ret(x, y, w, h, '#2b2740')
    for k in range(y + 1, y + h, 2):
        rua.ret(x + 1, k, w - 2, 1, '#55506a')


def antena(x, y, altura, luz=LARANJA):
    rua.escurecer(rua.area(x + 1, y, altura // 2, 1), 0.4)
    rua.ret(x, y - altura, 1, altura, '#8a84a0')
    rua.ret(x - 1, y - 1, 3, 2, '#55506a')
    rua.ret(x - 1, y - altura - 2, 3, 2, luz)
    rua.luz(x, y - altura - 1, 7, 7, luz, 0.4)


def caixa_dagua(x, y, r):
    rua.escurecer(rua.elipse(x + 3, y + 3, r, r * 0.8), 0.4)
    rua.pintar(rua.elipse(x, y, r, r * 0.8), '#55506a')
    rua.pintar(rua.elipse(x, y - 1, r - 1, r * 0.8 - 1), '#7d7796')
    rua.pintar(rua.elipse(x - 1, y - 2, r - 3, r * 0.8 - 2.5), '#9892b0')
    rua.pintar(rua.elipse(x, y - 1, 1.5, 1.2), '#55506a')


def cano(x0, x1, y):
    rua.ret(x0, y, x1 - x0, 2, '#7d7796')
    rua.ret(x0, y + 2, x1 - x0, 1, '#3a3452')
    for x in range(x0 + 4, x1, 12):
        rua.ret(x, y - 1, 2, 4, LARANJA_ESCURO)


def enfeitar_teto(x0, x1, ytopo, fundo, semente):
    """Espalha equipamentos pelo telhado, sem um bater no outro."""
    r = np.random.default_rng(semente)
    x = x0 + 6
    while x < x1 - 22:
        y = ytopo + 6 + int(r.integers(0, max(1, fundo - 24)))
        tipo = int(r.integers(0, 6))
        if y < 2:
            y = 2
        if tipo == 0:
            ar_condicionado(x, y)
        elif tipo == 1:
            claraboia(x, y, 16, 9, CIANO if r.random() > 0.3 else LARANJA)
        elif tipo == 2:
            grade(x, y, 12, 8)
        elif tipo == 3:
            antena(x + 5, y + 10, 9 + int(r.integers(0, 5)), LARANJA if r.random() > 0.4 else MAGENTA)
        elif tipo == 4:
            caixa_dagua(x + 7, y + 6, 7)
        else:
            cano(x, x + 18, y + 4)
        x += 22 + int(r.integers(0, 14))


# ---------- peças das fachadas ----------
def janela(x, y, w, h, c=CIANO):
    """Janela acesa com persiana: listras de luz."""
    rua.ret(x - 1, y - 1, w + 2, h + 2, CONTORNO)
    rua.ret(x, y, w, h, '#1d3a48' if c == CIANO else '#4a2f1e' if c == LARANJA else '#3d1d3a')
    for k in range(y + 1, y + h, 2):
        rua.ret(x + 1, k, w - 2 - ((k * 7 + x) % 3), 1, c)
    rua.ret(x - 1, y + h + 1, w + 2, 1, '#6a6488')


def lampada(x, y, c=LARANJA):
    rua.ret(x, y, 2, 4, c)
    rua.ret(x, y, 2, 1, LARANJA_CLARO if c == LARANJA else CIANO_CLARO)
    rua.ret(x - 1, y - 1, 4, 1, CONTORNO)


def porta(x, ybase, w, h, folha='#6a5f8c', moldura=LARANJA_ESCURO):
    y = ybase - h
    rua.ret(x - 2, y - 2, w + 4, h + 2, CONTORNO)
    rua.ret(x - 1, y - 1, w + 2, h + 1, moldura)
    rua.ret(x, y, w, h, folha)
    rua.ret(x, y, w, 2, '#3a3452')
    rua.ret(x + w // 2, y, 1, h, '#3a3452')
    rua.ret(x + 1, y + 3, w // 2 - 2, h - 6, '#7d72a0')
    rua.ret(x + w // 2 + 2, y + 3, w // 2 - 3, h - 6, '#7d72a0')
    rua.ret(x + w // 2 - 2, y + h // 2, 1, 2, LARANJA_CLARO)
    rua.ret(x + w // 2 + 2, y + h // 2, 1, 2, LARANJA_CLARO)


def portao_oficina(x, ybase, w, h):
    y = ybase - h
    rua.ret(x - 1, y - 1, w + 2, h + 1, CONTORNO)
    rua.ret(x, y, w, h, '#6a6488')
    for k in range(y + 1, ybase, 3):
        rua.ret(x, k, w, 1, '#4a4466')
    for k in range(x + 2, x + w - 2):
        if (k + 1) % 6 < 3:
            rua.ret(k, ybase - 3, 1, 2, LARANJA)
        else:
            rua.ret(k, ybase - 3, 1, 2, CONTORNO)


def toldo(x0, x1, y, c1, c2):
    """Toldo listrado, com a sombra dele na parede."""
    rua.escurecer(rua.area(x0, y + 6, x1 - x0, 4), 0.45)
    for x in range(x0, x1):
        c = c1 if (x - x0) // 5 % 2 == 0 else c2
        rua.ret(x, y, 1, 5, c)
        rua.ret(x, y + 5, 1, 1 if (x - x0) % 5 in (0, 4) else 2, c)
    rua.ret(x0, y - 1, x1 - x0, 1, CONTORNO)
    rua.ret(x0, y, x1 - x0, 1, '#f4ecd8')
    rua.ret(x0 - 1, y - 1, 1, 7, CONTORNO)
    rua.ret(x1, y - 1, 1, 7, CONTORNO)


def faixa(x, y, h, c=MAGENTA, simbolo=True):
    """Faixa de pano pendurada, com o planeta com anel."""
    rua.ret(x - 1, y - 1, 12, h + 2, CONTORNO)
    rua.ret(x, y, 10, h, c)
    rua.ret(x + 8, y, 2, h, MAGENTA_ESCURO)
    rua.ret(x + 3, y + h - 3, 4, 3, CONTORNO)                 # recorte em V
    rua.ret(x + 4, y + h - 2, 2, 3, CONTORNO)
    rua.ret(x - 2, y - 2, 14, 2, '#8a84a0')
    if simbolo:
        rua.pintar(rua.elipse(x + 4.5, y + 6, 2.2, 2.2), MAGENTA_CLARO)
        for dx in range(-4, 5):
            rua.ret(x + 4 + dx, y + 6 - dx // 3, 1, 1, '#f4ecd8')
        rua.ret(x + 2, y + h - 7, 6, 1, MAGENTA_CLARO)


ICONES = {
    'copo': ["#.....#", "#.....#", ".#...#.", "..#.#..", "...#...", "...#...", "..###.."],
    'carta': [".####..", ".#..#..", ".#.##..", ".#..#.#", ".####.#", "...#..#", "...####"],
    'engrenagem': ["..#.#..", ".#####.", "##...##", ".#...#.", "##...##", ".#####.", "..#.#.."],
    'lua': ["..###..", ".##....", "##.....", "##.....", "##.....", ".##....", "..###.."],
    'tigela': ["..#.#..", "...#...", "#######", "#.....#", ".#...#.", "..###..", ".#####."],
    'raio': ["...##..", "..##...", ".##....", "#####..", "..##...", ".##....", "##....."],
}


def icone(nome, x, y, c):
    for j, linha in enumerate(ICONES[nome]):
        for i, letra in enumerate(linha):
            if letra == '#':
                rua.ret(x + i, y + j, 1, 1, c)


def letreiro(x, y, w, h, c, nomes, fundo='#1a1626'):
    """Placa de neon: moldura acesa e desenhos (ninguém lê a língua daqui)."""
    rua.ret(x - 1, y - 1, w + 2, h + 2, CONTORNO)
    rua.ret(x, y, w, h, c)
    rua.ret(x + 1, y + 1, w - 2, h - 2, fundo)
    passo = (w - 2) // len(nomes)
    for i, nome in enumerate(nomes):
        icone(nome, x + 1 + i * passo + (passo - 7) // 2, y + (h - 7) // 2, c)


def rabiscos(x, y, n, c):
    """Escrita alienígena: tracinhos que parecem letras."""
    r = np.random.default_rng(x * 31 + y)
    for i in range(n):
        forma = int(r.integers(0, 4))
        px = x + i * 4
        if forma == 0:
            rua.ret(px, y, 1, 5, c), rua.ret(px, y + 2, 3, 1, c)
        elif forma == 1:
            rua.ret(px, y, 3, 1, c), rua.ret(px + 1, y, 1, 5, c), rua.ret(px, y + 4, 3, 1, c)
        elif forma == 2:
            rua.ret(px, y, 3, 1, c), rua.ret(px + 2, y, 1, 3, c), rua.ret(px, y + 2, 1, 3, c)
        else:
            rua.ret(px, y + 1, 1, 4, c), rua.ret(px + 2, y, 1, 4, c), rua.ret(px + 1, y + 4, 1, 1, c)


def floreira(x, ybase, w, planta=MAGENTA):
    """Canteiro com plantas, encostado na parede (bloqueia)."""
    rua.escurecer(rua.area(x + 1, ybase + 5, w, 2), 0.4)
    r = np.random.default_rng(x)
    for i in range(x + 1, x + w - 1, 2):
        alt = int(r.integers(4, 9))
        c = planta if r.random() > 0.3 else (VERDE if planta == MAGENTA else MAGENTA)
        rua.ret(i, ybase - alt, 1, alt, c)
        rua.ret(i + (1 if i % 4 else -1), ybase - alt + 2, 1, 2, MAGENTA_CLARO if c == MAGENTA else VERDE_ESCURO)
    rua.ret(x, ybase, w, 5, '#55506a')
    rua.ret(x, ybase, w, 1, '#8a84a0')
    rua.ret(x + 2, ybase + 3, w - 4, 1, CIANO_ESCURO)
    rua.ret(x - 1, ybase - 1, 1, 6, CONTORNO)
    rua.ret(x + w, ybase - 1, 1, 6, CONTORNO)
    rua.ret(x, ybase + 5, w, 1, CONTORNO)
    rua.bloquear(x, ybase, w, 5)


def caixote(x, y, w, h, altura, c='#6a5f8c'):
    """Caixa solta na rua: base em (x, y), com volume. Bloqueia."""
    rua.escurecer(rua.area(x + 2, y + 1, w + 2, h), 0.4)
    rua.ret(x - 1, y - altura - 1, w + 2, h + altura + 2, CONTORNO)
    rua.ret(x, y - altura, w, h, '#8a80ac')
    rua.ret(x, y - altura, w, 1, '#aaa0cc')
    rua.ret(x, y - altura + h, w, altura, c)
    rua.ret(x + w // 2, y - altura + h, 1, altura, '#4a4366')
    rua.ret(x + 1, y - altura + h + 2, 3, 1, LARANJA)
    rua.bloquear(x, y - altura, w, h + altura)


def barril(x, y):
    rua.escurecer(rua.elipse(x + 3, y + 1, 5, 2.5), 0.4)
    rua.ret(x - 4, y - 10, 9, 11, CONTORNO)
    rua.ret(x - 3, y - 9, 7, 9, '#8f5a3a')
    rua.ret(x - 3, y - 9, 2, 9, '#b0764a')
    rua.ret(x - 3, y - 7, 7, 1, '#4a4366')
    rua.ret(x - 3, y - 3, 7, 1, '#4a4366')
    rua.pintar(rua.elipse(x, y - 10, 3.6, 1.6), '#c08a58')
    rua.bloquear(x - 4, y - 6, 9, 7)


def cerca_de_energia(x, y0, y1):
    """Fecha o fim da rua: postes e barras de luz."""
    rua.ret(x, y0, 8, y1 - y0, '#191524')
    rua.pontilhar(rua.area(x, y0, 8, y1 - y0), '#221e2e', 0.5)
    for y in range(y0 + 4, y1 - 2, 9):
        rua.ret(x + 1, y, 6, 1, MAGENTA)
        rua.ret(x + 1, y + 1, 6, 1, MAGENTA_ESCURO)
    for px in (x, x + 6):
        rua.ret(px, y0, 2, y1 - y0, '#55506a')
        rua.ret(px, y0, 2, 2, LARANJA)
    rua.luz(x + 4, (y0 + y1) / 2, 22, (y1 - y0) * 0.7, MAGENTA, 0.3)
    rua.bloquear(x, y0, 8, y1 - y0)


def calcada(x0, x1, y, h=7):
    """Faixa mais clara em frente às fachadas."""
    rua.ret(x0, y, x1 - x0, h, '#474260')
    faixa_ = rua.area(x0, y, x1 - x0, h)
    rua.pintar(faixa_ & ((xx - x0) % 12 == 11), '#3a3552')
    rua.ret(x0, y, x1 - x0, 1, '#57526f')
    rua.ret(x0, y + h, x1 - x0, 1, '#1e1a29')


# ---------- fileira de cima (as fachadas dão para a rua de cima) ----------
Y1 = RUA_CIMA[0]
calcada(0, 640, Y1)
fila_cima = [  # x0, x1, altura, paleta
    (0, 104, 32, 0), (108, 214, 37, 2), (218, 330, 30, 1), (334, 452, 34, 3), (456, 640, 42, 2),
]
rua.ret(0, 0, 640, Y1, CONTORNO)
rua.bloquear(0, 0, 640, Y1)
for i, (x0, x1, alt, pal) in enumerate(fila_cima):
    predio(x0, x1, Y1, 52, alt, PREDIOS[pal], 100 + i)
    enfeitar_teto(x0, x1, Y1 - 52 - alt, 52, 200 + i)

# oficina
portao_oficina(14, Y1, 40, 22)
letreiro(20, Y1 - 31, 28, 9, LARANJA, ['engrenagem', 'raio'])
janela(66, Y1 - 22, 26, 10)
lampada(58, Y1 - 24)
cano(60, 100, Y1 - 8)
rua.luz(34, Y1 + 6, 30, 12, LARANJA, 0.3, ~rua.solido)

# pousada
porta(152, Y1, 16, 24)
toldo(144, 176, Y1 - 31, MAGENTA, MAGENTA_ESCURO)
letreiro(116, Y1 - 33, 20, 11, MAGENTA_CLARO, ['lua'])
rabiscos(118, Y1 - 19, 4, MAGENTA_CLARO)
janela(184, Y1 - 24, 22, 8, LARANJA)
janela(184, Y1 - 12, 22, 6, LARANJA)
faixa(124, Y1 - 15, 13)
lampada(146, Y1 - 22), lampada(172, Y1 - 22)
floreira(182, Y1, 26)
rua.luz(160, Y1 + 6, 30, 12, MAGENTA, 0.3, ~rua.solido)

# loja
janela(226, Y1 - 20, 38, 14)
porta(276, Y1, 14, 22, '#5f7a8c', CIANO_ESCURO)
letreiro(296, Y1 - 24, 26, 9, CIANO, ['carta', 'raio'])
rabiscos(298, Y1 - 12, 5, CIANO_ESCURO)
lampada(268, Y1 - 22, CIANO)
rua.luz(246, Y1 + 5, 30, 10, CIANO, 0.3, ~rua.solido)

# depósito, com tanque e canos
portao_oficina(366, Y1, 30, 20)
janela(342, Y1 - 24, 16, 6, LARANJA)
janela(406, Y1 - 24, 36, 8)
cano(402, 448, Y1 - 10)
rua.ret(442, Y1 - 28, 3, 20, '#7d7796')
rua.ret(445, Y1 - 28, 1, 20, '#3a3452')
rabiscos(408, Y1 - 13, 6, LARANJA_ESCURO)

# o bar: o prédio mais alto, com o maior letreiro
XB = 456
for x in range(XB + 6, 634, 4):                                # tábuas na fachada
    rua.ret(x, Y1 - 38, 1, 34, '#42314c')
rua.ret(XB + 4, Y1 - 39, 176, 2, '#836a92')
porta_x0, porta_x1 = PORTA_BAR
# porta de vaivém: duas folhas curtas, com a luz de dentro por trás
rua.ret(porta_x0 - 3, Y1 - 28, 26, 28, CONTORNO)
rua.ret(porta_x0 - 2, Y1 - 27, 24, 27, LARANJA_ESCURO)
rua.ret(porta_x0, Y1 - 25, 20, 25, '#3a2030')
rua.ret(porta_x0, Y1 - 25, 20, 6, '#f2a541')
rua.pontilhar(rua.area(porta_x0, Y1 - 19, 20, 4), '#f2a541', 0.5)
for fx in (porta_x0, porta_x0 + 11):
    rua.ret(fx, Y1 - 17, 9, 13, '#8f5a3a')
    rua.ret(fx, Y1 - 17, 9, 1, '#c08a58')
    for k in range(Y1 - 15, Y1 - 5, 2):
        rua.ret(fx + 1, k, 7, 1, '#6a4028')
    rua.ret(fx, Y1 - 5, 9, 1, '#6a4028')
rua.ret(porta_x0 + 9, Y1 - 17, 2, 13, '#2a1622')
# letreiro grande em cima do telhado: copo e carta
letreiro(porta_x0 - 26, Y1 - 62, 72, 17, MAGENTA_CLARO, ['copo', 'carta', 'copo'], '#2a1230')
rua.ret(porta_x0 - 20, Y1 - 45, 2, 5, '#55506a')
rua.ret(porta_x0 + 38, Y1 - 45, 2, 5, '#55506a')
rua.luz(porta_x0 + 10, Y1 - 54, 60, 22, MAGENTA, 0.35)
toldo(porta_x0 - 30, porta_x0 + 50, Y1 - 36, '#8f5a3a', '#6a4028')
janela(470, Y1 - 24, 30, 12, LARANJA)
janela(586, Y1 - 24, 30, 12, LARANJA)
rabiscos(474, Y1 - 9, 6, MAGENTA)
rabiscos(590, Y1 - 9, 6, MAGENTA)
lampada(porta_x0 - 8, Y1 - 24), lampada(porta_x0 + 26, Y1 - 24)
faixa(570, Y1 - 27, 15, '#8f5a3a')
rua.luz(porta_x0 + 10, Y1 + 8, 40, 18, LARANJA, 0.45, ~rua.solido)
barril(516, Y1 + 5), barril(526, Y1 + 6)
caixote(596, Y1 + 5, 14, 6, 7, '#8f5a3a')

# ---------- fileira do meio (as fachadas dão para a rua principal) ----------
Y2 = RUA_BAIXO[0]
fila_meio = [
    (0, BECO[0], 30, 1), (BECO[1], PRACA[0], 34, 0), (PRACA[1], 500, 28, 2), (504, 640, 34, 3),
]
for x0, x1, *_ in fila_meio:
    calcada(x0, x1, Y2)
for i, (x0, x1, alt, pal) in enumerate(fila_meio):
    predio(x0, x1, Y2, 52, alt, PREDIOS[pal], 110 + i)
    enfeitar_teto(x0, x1, Y2 - 52 - alt, 52, 210 + i)

# prédio de moradia
porta(40, Y2, 14, 22)
janela(12, Y2 - 22, 20, 8, LARANJA), janela(64, Y2 - 22, 20, 8), janela(96, Y2 - 22, 20, 8, MAGENTA)
janela(64, Y2 - 10, 20, 5), janela(96, Y2 - 10, 20, 5, LARANJA)
lampada(34, Y2 - 22)
floreira(118, Y2, 22, VERDE)

# loja de peças
portao_oficina(192, Y2, 34, 22)
letreiro(232, Y2 - 28, 30, 9, CIANO, ['engrenagem', 'engrenagem'])
janela(236, Y2 - 15, 26, 9)
porta(274, Y2, 14, 22, '#5f7a8c', CIANO_ESCURO)
lampada(266, Y2 - 24, CIANO)
rua.luz(246, Y2 + 6, 34, 12, CIANO, 0.3, ~rua.solido)

# barraca de comida: balcão aberto com toldo
rua.ret(392, Y2 - 20, 52, 16, CONTORNO)
rua.ret(393, Y2 - 19, 50, 14, '#5a3a22')
rua.pontilhar(rua.area(393, Y2 - 19, 50, 8), LARANJA, 0.6)
for k, c in enumerate((MAGENTA_CLARO, CIANO, LARANJA_CLARO, VERDE, MAGENTA_CLARO, CIANO)):
    rua.ret(397 + k * 8, Y2 - 12, 3, 5, c)                    # garrafas e potes
    rua.ret(398 + k * 8, Y2 - 14, 1, 2, c)
rua.ret(390, Y2 - 6, 56, 3, '#8a80ac')
rua.ret(390, Y2 - 6, 56, 1, '#b8b2cc')
toldo(388, 448, Y2 - 26, LARANJA, '#f4ecd8')
letreiro(454, Y2 - 25, 12, 11, LARANJA_CLARO, ['tigela'])
porta(476, Y2, 14, 20)
rua.luz(418, Y2 + 6, 40, 14, LARANJA, 0.4, ~rua.solido)
for bx in (398, 414, 430):                                     # banquinhos
    rua.escurecer(rua.elipse(bx + 2, Y2 + 9, 4, 2), 0.4)
    rua.ret(bx - 3, Y2 + 3, 7, 3, CONTORNO)
    rua.ret(bx - 2, Y2 + 3, 5, 2, MAGENTA)
    rua.ret(bx, Y2 + 5, 1, 4, '#8a84a0')
    rua.bloquear(bx - 3, Y2 + 3, 7, 6)

# último prédio do meio
janela(514, Y2 - 24, 44, 10)
janela(514, Y2 - 10, 20, 5, LARANJA), janela(540, Y2 - 10, 18, 5, MAGENTA)
porta(576, Y2, 14, 22)
letreiro(598, Y2 - 26, 30, 9, MAGENTA_CLARO, ['raio', 'lua'])
faixa(566, Y2 - 30, 13)
floreira(600, Y2, 26)

# ---------- fileira de baixo: só os telhados aparecem ----------
Y3 = RUA_BAIXO[1]
fila_baixo = [(158, 300, 30, 3), (304, 470, 26, 0), (474, 640, 32, 1)]
for i, (x0, x1, alt, pal) in enumerate(fila_baixo):
    predio(x0, x1, Y3 + 150, 150 - alt, alt, PREDIOS[pal], 120 + i, friso=None)
    rua.ret(x0, Y3 + 1, x1 - x0, 1, CONTORNO)
    enfeitar_teto(x0, x1, Y3 + 4, 44, 220 + i)
    enfeitar_teto(x0 + 8, x1, Y3 + 46, 44, 230 + i)
rua.ret(154, Y3, 4, 96, CONTORNO)
rua.ret(300, Y3, 4, 96, CONTORNO)
rua.ret(470, Y3, 4, 96, CONTORNO)
rua.bloquear(154, Y3, 486, 96)

# ---------- pátio de pouso ----------
# muro baixo com faixas de aviso, à esquerda e embaixo
rua.ret(0, Y3, PATIO[0], 96, '#191524')
rua.ret(PATIO[0] - 4, Y3, 4, 96, '#55506a')
rua.ret(PATIO[0] - 1, Y3, 1, 96, '#8a84a0')
rua.bloquear(0, Y3, PATIO[0], 96)
rua.ret(0, PATIO[2], PATIO[1], 14, '#191524')
rua.ret(0, PATIO[2], PATIO[1], 4, '#55506a')
rua.ret(0, PATIO[2], PATIO[1], 1, '#8a84a0')
for x in range(0, PATIO[1]):
    rua.ret(x, PATIO[2] + 5, 1, 4, LARANJA if (x // 4) % 2 == 0 else CONTORNO)
rua.bloquear(0, PATIO[2], PATIO[1], 14)
# luzes de pista nos cantos
for lx, ly in ((22, Y3 + 6), (146, Y3 + 6), (22, PATIO[2] - 6), (146, PATIO[2] - 6)):
    rua.ret(lx - 1, ly - 1, 3, 3, '#55506a')
    rua.ret(lx, ly, 1, 1, CIANO_CLARO)
    rua.luz(lx, ly, 9, 5, CIANO, 0.4, ~rua.solido)

# o módulo de pouso
modulo = desenhar_modulo()
ox, oy = MODULO_X - 36, MODULO_BASE - 69
rua.escurecer(rua.elipse(MODULO_X + 5, MODULO_BASE - 1, 36, 8), 0.45)
rua.colar(modulo, ox, oy)
alfa = modulo.c[..., 3] > 0
for y in range(17, modulo.h):
    xs = np.nonzero(alfa[y])[0]
    if len(xs):
        rua.solido[oy + y, ox + xs.min():ox + xs.max() + 1] = True

# ---------- beco, praça e fim das ruas ----------
caixote(150, 150, 12, 6, 7), caixote(151, 142, 9, 5, 6, '#55506a')
caixote(170, 196, 9, 5, 6, '#8f5a3a')
rua.luz(164, 170, 16, 40, CIANO, 0.18, ~rua.solido)

# base do holograma, no meio da praça (o planeta é desenhado pelo jogo)
hx, hy = HOLOGRAMA
for raio, c in ((30, '#4a4566'), (22, '#2a2639')):
    anel = rua.elipse(hx, hy, raio, raio * 0.5) & ~rua.elipse(hx, hy, raio - 1.5, raio * 0.5 - 1)
    rua.pintar(anel, c)
rua.escurecer(rua.elipse(hx + 3, hy + 2, 13, 6), 0.4)
rua.pintar(rua.elipse(hx, hy, 12, 5.5) | rua.area(hx - 12, hy - 4, 24, 4), CONTORNO)
rua.pintar(rua.elipse(hx, hy, 11, 4.5) | rua.area(hx - 11, hy - 4, 22, 4), '#55506a')
rua.pintar(rua.elipse(hx, hy - 4, 11, 4.5), '#8a84a0')
rua.pintar(rua.elipse(hx, hy - 4, 8, 3), '#1d3a48')
rua.pintar(rua.elipse(hx, hy - 4, 5, 1.8), CIANO)
rua.pintar(rua.elipse(hx, hy - 4, 2, 0.8), CIANO_CLARO)
rua.ret(hx - 9, hy + 1, 18, 1, CIANO_ESCURO)
rua.solido |= rua.elipse(hx, hy - 1, 13, 6)
rua.luz(hx, hy - 2, 46, 30, CIANO, 0.4, ~rua.solido)
# bancos e canteiros nos cantos da praça
for fx in (PRACA[0] + 4, PRACA[1] - 24):
    floreira(fx, RUA_CIMA[1] + 10, 20, MAGENTA)
    floreira(fx, RUA_BAIXO[0] - 12, 20, VERDE)

for y0, y1 in (RUA_CIMA, RUA_BAIXO):
    cerca_de_energia(0, y0, y1 + 2)
    cerca_de_energia(632, y0, y1 + 2)

# moto flutuante parada em frente ao bar, presa a uma barra de luz
MOTO = figura([
    "..........kk............",
    ".........kOOk....kk.....",
    "....kkkkkkOOkkkkkSSk....",
    "..kkMMMMMMkkKKKKKSSkkk..",
    ".kMMmmMMMMMMKKKKMMMMMMk.",
    "kMMmMMMMMMMMMMMMMMMMMMMk",
    "kMMMMMMMMMMRRRRMMMMMMMRk",
    ".kRRRRRRRRRRkkRRRRRRRRk.",
    "..kkSSSkkkkk..kkkSSSkk..",
    "...kCCCk........kCCCk...",
], {'k': CONTORNO, 'M': MAGENTA, 'm': MAGENTA_CLARO, 'R': MAGENTA_ESCURO, 'K': '#2b2740',
    'S': '#8a84a0', 'O': LARANJA, 'C': CIANO})
mx, my = 486, Y1 + 12
rua.luz(mx + 12, my + 1, 16, 5, CIANO, 0.6, ~rua.solido)
rua.escurecer(rua.elipse(mx + 13, my + 3, 12, 2.5), 0.35)
rua.colar(MOTO, mx, my - 12)
rua.ret(mx - 4, my - 4, 2, 6, '#8a84a0')                       # barra onde ela fica presa
rua.ret(mx + 26, my - 4, 2, 6, '#8a84a0')
rua.ret(mx - 4, my - 5, 32, 1, CIANO)
rua.bloquear(mx - 4, my - 5, 32, 7)

# ---------- clarão dos postes (os postes são desenhados pelo jogo) ----------
for px, py in POSTES:
    rua.luz(px, py - 2, 34, 17, LARANJA, 0.5, ~rua.solido)
    rua.ret(px - 2, py - 1, 4, 2, '#3a3552')                  # sapata do poste
    rua.bloquear(px - 2, py - 2, 4, 3)

# ---------- escurecer as bordas, para a noite ----------
vinheta = np.clip((np.abs(xx - 320) - 250) / 90, 0, 1) * 0.25
rua.img[:] = (rua.img * (1 - vinheta[..., None]) + cor(CONTORNO) * vinheta[..., None]).astype(np.uint8)

Image.fromarray(ampliar(rua.img)).save('assets/imagens/planetas/et_rua.png')
mascara = np.where(rua.solido[..., None], 0, 255).astype(np.uint8).repeat(3, 2)
Image.fromarray(ampliar(mascara)).save('assets/mascaras/et_rua_mascara.png')
print('rua ok')


# =====================================================================
# 2. O BAR POR DENTRO (grade de 320x180)
# =====================================================================
bar = Cena(320, 180, CONTORNO)
bx, by, bb = bar.xx, bar.yy, bar.bayer
SALA = (10, 310, 62, 164)        # x0, x1 do salão e y do chão (começo e fim)
PORTA_SAIDA = (148, 172)         # vão na parede de baixo
BALCAO = (20, 152, 82, 93)       # x0, x1 e y da frente do balcão
MESA_JOGO = (236, 124)           # centro do tampo da mesa de cartas
MESA_CANTO = (64, 138)

# assoalho de tábuas
x0, x1, y0, y1 = SALA
tabua = (by - y0) // 6
desloc = (tabua * 37) % 50
piso = bar.area(x0, y0, x1 - x0, y1 - y0)
tons = np.array([cor('#4a3550'), cor('#51395a'), cor('#432f49'), cor('#573e5f')])
bar.img[piso] = tons[(tabua * 5 + (bx + desloc) // 50) % 4][piso]
bar.pintar(piso & ((by - y0) % 6 == 5), '#33233a')
bar.pintar(piso & ((bx + desloc) % 50 == 0), '#33233a')
s = bar.sorteio(7)
bar.pintar(piso & (s > 0.985) & ((by - y0) % 6 < 4), '#5f4568')
bar.pintar(piso & (s < 0.008), '#33233a')

# tapete debaixo da mesa de cartas
tx, ty = MESA_JOGO[0], MESA_JOGO[1] + 6
for raio, c in ((46, '#3a1d3a'), (43, '#7a2a66'), (38, '#5a2050'), (35, '#7a2a66')):
    bar.pintar(bar.elipse(tx, ty, raio, raio * 0.48) & piso, c)
ang = np.arctan2((by - ty) / 0.48, bx - tx)
bar.pintar(bar.elipse(tx, ty, 41, 19.5) & ~bar.elipse(tx, ty, 39, 18.5) &
           ((ang * 12 / np.pi).astype(int) % 2 == 0), LARANJA_ESCURO)

# parede do fundo: lambri de madeira
bar.ret(x0, 18, x1 - x0, y0 - 18, '#4f3550')
fundo_ = bar.area(x0, 22, x1 - x0, y0 - 22)
bar.pintar(fundo_ & (bx % 10 == 0), '#3d2840')
bar.pintar(fundo_ & (bx % 10 == 1) & (bb < 0.5), '#5c4060')
bar.ret(x0, 18, x1 - x0, 4, '#836a92')                        # topo da parede
bar.ret(x0, 18, x1 - x0, 1, '#a08ab0')
bar.ret(x0, 22, x1 - x0, 2, CONTORNO)
bar.ret(x0, y0 - 6, x1 - x0, 6, '#3d2840')                    # rodapé
bar.ret(x0, y0 - 7, x1 - x0, 1, '#6a4a6c')
bar.ret(x0, y0 - 1, x1 - x0, 1, CONTORNO)

# prateleiras de garrafas atrás do balcão
bar.ret(22, 27, 128, 27, CONTORNO)
bar.ret(23, 28, 126, 25, '#2a1a30')
bar.pontilhar(bar.area(23, 28, 126, 25), '#3a2440', 0.5)
r = np.random.default_rng(12)
for prateleira in (40, 53):
    bar.ret(23, prateleira - 1, 126, 2, '#8f5a3a')
    bar.ret(23, prateleira - 1, 126, 1, '#c08a58')
    x = 26
    while x < 144:
        c = [MAGENTA_CLARO, CIANO, LARANJA_CLARO, VERDE, '#9b8ce0', MAGENTA][int(r.integers(0, 6))]
        alt = int(r.integers(5, 10))
        larg = int(r.integers(2, 5))
        bar.ret(x, prateleira - 1 - alt, larg, alt, c)
        bar.ret(x, prateleira - 1 - alt, 1, alt, '#f4ecd8')
        bar.ret(x + larg // 2, prateleira - 3 - alt, 1, 2, c)
        x += larg + int(r.integers(2, 5))
bar.luz(86, 40, 70, 20, MAGENTA, 0.25)

# letreiro de neon: planeta com anel e um copo
bar.ret(176, 26, 62, 26, CONTORNO)
bar.ret(177, 27, 60, 24, '#2a1230')
anel_ = bar.elipse(196, 39, 15, 5) & ~bar.elipse(196, 39, 13.5, 3.8)
bar.pintar(bar.elipse(196, 39, 7.5, 7.5), MAGENTA)
bar.pintar(bar.elipse(194, 37, 4, 4), MAGENTA_CLARO)
bar.pintar(anel_ & ~(bar.elipse(196, 39, 7.5, 7.5) & (by < 39)), CIANO)
for j, linha in enumerate(ICONES['copo']):
    for i, letra in enumerate(linha):
        if letra == '#':
            bar.ret(216 + i * 2, 32 + j * 2, 2, 2, LARANJA_CLARO)
bar.luz(206, 40, 56, 30, MAGENTA, 0.4)

# vitrola de luzes, no canto direito
bar.ret(268, 30, 28, 36, CONTORNO)
bar.ret(269, 31, 26, 34, '#5a2050')
bar.pintar(bar.elipse(282, 42, 11, 10) & (by < 43), '#1d3a48')
for k in range(34, 43, 2):
    bar.ret(274, k, 16, 1, CIANO)
bar.ret(271, 46, 22, 3, LARANJA)
bar.ret(271, 46, 22, 1, LARANJA_CLARO)
for k in range(4):
    bar.ret(272 + k * 6, 52, 3, 3, [MAGENTA_CLARO, CIANO, LARANJA_CLARO, VERDE][k])
bar.ret(271, 58, 22, 5, '#3a1d3a')
bar.ret(269, 31, 2, 34, MAGENTA_CLARO)
bar.ret(293, 31, 2, 34, MAGENTA_ESCURO)
bar.bloquear(268, 60, 28, 8)
bar.luz(282, 76, 30, 16, CIANO, 0.35, piso)

# quadros na parede
for qx, c in ((246, CIANO), (156, LARANJA)):
    bar.ret(qx, 30, 14, 12, CONTORNO)
    bar.ret(qx + 1, 31, 12, 10, '#8f5a3a')
    bar.ret(qx + 2, 32, 10, 8, '#1c2033')
    bar.pintar(bar.elipse(qx + 7, 36, 2.5, 2.5), c)

# paredes dos lados e de baixo (vistas de cima)
for wx in (4, x1):
    bar.ret(wx, 18, 6, 156, '#6a5278')
    bar.ret(wx, 18, 1, 156, '#836a92')
    bar.ret(wx + 5, 18, 1, 156, '#42314c')
bar.ret(4, y1, 312, 8, '#6a5278')
bar.ret(4, y1, 312, 1, '#836a92')
bar.ret(4, y1 + 7, 312, 1, '#42314c')
bar.bloquear(0, 0, 320, y0)
bar.bloquear(0, 0, x0, 180), bar.bloquear(x1, 0, 10, 180), bar.bloquear(0, y1, 320, 16)
# porta de saída: vão na parede de baixo, com capacho
px0, px1 = PORTA_SAIDA
bar.ret(px0, y1, px1 - px0, 9, '#1c2033')
bar.pontilhar(bar.area(px0, y1 + 3, px1 - px0, 6), '#2e2a3d', 0.6)
bar.ret(px0 - 2, y1, 2, 8, LARANJA_ESCURO), bar.ret(px1, y1, 2, 8, LARANJA_ESCURO)
bar.ret(px0 + 1, y1 - 8, px1 - px0 - 2, 7, '#8f5a3a')
bar.ret(px0 + 2, y1 - 7, px1 - px0 - 4, 5, '#b0764a')
for k in range(px0 + 4, px1 - 3, 3):
    bar.ret(k, y1 - 6, 1, 3, '#6a4028')
bar.solido[y1:y1 + 6, px0:px1] = False


solido_sem_moveis = bar.solido.copy()


def movel(x, y, w, h, altura, tampo, frente, brilho, escuro):
    """Móvel com volume: base em (x, y) de w por h; bloqueia."""
    bar.escurecer(bar.area(x + 2, y + 2, w + 1, h), 0.4)
    bar.ret(x - 1, y - altura - 1, w + 2, h + altura + 2, CONTORNO)
    bar.ret(x, y - altura, w, h, tampo)
    bar.ret(x, y - altura, w, 1, brilho)
    bar.ret(x, y - altura + h, w, altura, frente)
    bar.ret(x, y - altura + h, w, 1, escuro)
    bar.bloquear(x, y - altura, w, h + altura)


# balcão comprido, com copos e uma luz embutida na frente
cx0, cx1, cy0, cy1 = BALCAO
movel(cx0, cy0, cx1 - cx0, cy1 - cy0, 12, '#b0764a', '#6a4028', '#d8a070', '#4a2a1c')
for k in range(cx0 + 6, cx1 - 4, 14):
    bar.ret(k, cy0 + 1, 1, cy1 - cy0 - 2, '#4a2a1c')
bar.ret(cx0 + 2, cy1 - 3, cx1 - cx0 - 4, 1, CIANO)
bar.luz((cx0 + cx1) / 2, cy1 + 3, 70, 9, CIANO, 0.3, piso & ~bar.solido)
for gx, c in ((34, MAGENTA_CLARO), (58, CIANO), (96, LARANJA_CLARO), (120, VERDE)):
    bar.ret(gx, cy0 - 14, 3, 4, c)                              # copos no balcão
    bar.ret(gx, cy0 - 14, 3, 1, '#f4ecd8')
bar.ret(74, cy0 - 13, 8, 3, '#8a84a0')                          # bandeja
# banquinhos
for sx in (36, 62, 88, 114, 140):
    bar.escurecer(bar.elipse(sx + 1, cy1 + 11, 4, 2), 0.4)
    bar.ret(sx - 3, cy1 + 5, 7, 3, CONTORNO)
    bar.ret(sx - 2, cy1 + 5, 5, 2, MAGENTA)
    bar.ret(sx - 2, cy1 + 5, 5, 1, MAGENTA_CLARO)
    bar.ret(sx, cy1 + 7, 1, 4, '#8a84a0')
    bar.bloquear(sx - 3, cy1 + 5, 7, 6)


def mesa_redonda(cx, cy, rx, ry, altura, feltro=None):
    """Mesa redonda: (cx, cy) é o centro do tampo."""
    bar.escurecer(bar.elipse(cx + 3, cy + altura + 2, rx, ry), 0.45)
    bar.ret(cx - 2, cy, 5, altura + 3, '#4a2a1c')              # pé
    bar.pintar(bar.elipse(cx, cy + altura + 3, 6, 2.5), '#4a2a1c')
    bar.pintar(bar.elipse(cx, cy + 2, rx + 1, ry + 1) | bar.elipse(cx, cy, rx + 1, ry + 1), CONTORNO)
    bar.pintar(bar.elipse(cx, cy + 2, rx, ry), '#6a4028')
    bar.pintar(bar.elipse(cx, cy, rx, ry), '#b0764a')
    bar.pintar(bar.elipse(cx, cy, rx, ry) & ~bar.elipse(cx, cy + 1, rx, ry), '#d8a070')
    if feltro:
        bar.pintar(bar.elipse(cx, cy, rx - 3, ry - 2), feltro[0])
        bar.pontilhar(bar.elipse(cx - 2, cy - 1, rx - 8, ry - 4), feltro[1], 0.6)
    bar.solido |= bar.elipse(cx, cy + 1, rx + 1, ry + 2) | bar.area(cx - 6, cy, 12, altura + 5)


# mesa de cartas: feltro verde, baralho e fichas
mx_, my_ = MESA_JOGO
mesa_redonda(mx_, my_, 27, 12, 9, ('#1f5a5a', '#2a7a72'))
for i, (dx, dy) in enumerate(((-11, -2), (-4, 2), (3, -1))):
    bar.ret(mx_ + dx - 1, my_ + dy - 1, 7, 6, CONTORNO)
    bar.ret(mx_ + dx, my_ + dy, 5, 4, '#3a2a5c' if i else '#f1e8d2')
    bar.ret(mx_ + dx + 2, my_ + dy + 1, 1, 2, CIANO if i else MAGENTA)
for dx, c in ((13, LARANJA), (16, CIANO), (11, MAGENTA_CLARO)):
    bar.ret(mx_ + dx, my_ + 3, 3, 2, c)
    bar.ret(mx_ + dx, my_ + 2, 3, 1, '#f4ecd8')
bar.luz(mx_, my_ - 2, 44, 26, LARANJA_CLARO, 0.45)
# cadeiras dos dois lados
for kx in (mx_ - 40, mx_ + 34):
    movel(kx, my_ + 4, 7, 5, 5, '#8f5a3a', '#6a4028', '#c08a58', '#4a2a1c')
    bar.ret(kx, my_ - 9, 7, 8, '#6a4028')
    bar.ret(kx - 1, my_ - 10, 9, 1, CONTORNO)

# mesa do canto, com copos
mesa_redonda(MESA_CANTO[0], MESA_CANTO[1], 15, 7, 8)
bar.ret(MESA_CANTO[0] - 6, MESA_CANTO[1] - 4, 3, 4, CIANO)
bar.ret(MESA_CANTO[0] + 3, MESA_CANTO[1] - 2, 3, 4, MAGENTA_CLARO)
bar.luz(MESA_CANTO[0], MESA_CANTO[1], 30, 18, LARANJA_CLARO, 0.3)
# Só o balcão, as mesas e as cadeiras: o jogo desenha esta camada por
# cima de quem está atrás deles (quem atende, o viajante, o freguês).
moveis = bar.solido & ~solido_sem_moveis

# barris no canto de baixo
for qx, qy in ((22, 158), (34, 160), (296, 158)):
    bar.escurecer(bar.elipse(qx + 3, qy + 1, 6, 3), 0.4)
    bar.ret(qx - 5, qy - 12, 11, 13, CONTORNO)
    bar.ret(qx - 4, qy - 11, 9, 11, '#8f5a3a')
    bar.ret(qx - 4, qy - 11, 2, 11, '#b0764a')
    bar.ret(qx - 4, qy - 8, 9, 1, '#4a4366'), bar.ret(qx - 4, qy - 3, 9, 1, '#4a4366')
    bar.pintar(bar.elipse(qx, qy - 12, 4.6, 2), '#c08a58')
    bar.bloquear(qx - 5, qy - 7, 11, 8)

# clarões no chão: luz da porta e das lâmpadas
bar.luz(160, 158, 30, 14, LARANJA, 0.3, piso & ~bar.solido)
bar.luz(206, 70, 50, 16, MAGENTA, 0.3, piso & ~bar.solido)

Image.fromarray(ampliar(bar.img)).save('assets/imagens/planetas/et_bar.png')
mascara = np.where(bar.solido[..., None], 0, 255).astype(np.uint8).repeat(3, 2)
Image.fromarray(ampliar(mascara)).save('assets/mascaras/et_bar_mascara.png')
frente = np.zeros((180, 320, 4), np.uint8)
frente[..., :3] = bar.img
frente[..., 3] = np.where(moveis, 255, 0)
Image.fromarray(ampliar(frente)).save('assets/imagens/planetas/et_bar_frente.png')
print('bar ok')


# =====================================================================
# 3. ALIENÍGENAS (quadros de 44x58, como os tripulantes)
# =====================================================================
# Cada linha da folha é um alienígena. Colunas: de frente, de frente
# piscando, de lado (direita), de lado piscando, esquerda e esquerda
# piscando. Os pés ficam na última linha do quadro, no meio.
OLHO, OLHO_BRILHO = '#1a1426', '#f4ecd8'
CONTORNO_ALIEN = '#2a1830'

ALIENS = [
    # 0: o viajante (chapéu de aba larga e poncho)
    dict(pele='#5fc9b4', pele_c='#9fe8d6', pele_s='#3f9a8e', roupa='#b0407e', roupa_c='#d86aa4',
         roupa_s='#7a2a5c', detalhe=LARANJA, calca='#4a4366', bota='#2b2740', bota_c='#55506a',
         antena=LARANJA_CLARO, chapeu=True, poncho=True),
    # 1: o morador (jaqueta com listra de luz)
    dict(pele='#a99be6', pele_c='#d6cbf8', pele_s='#7a6cc0', roupa='#3a5a78', roupa_c='#5a82a6',
         roupa_s='#28405a', detalhe=CIANO, calca='#3a3452', bota='#2b2740', bota_c='#55506a',
         antena=CIANO_CLARO, chapeu=False, poncho=False),
    # 2: quem atende no balcão (avental)
    dict(pele='#e8a05a', pele_c='#ffd098', pele_s='#b0703a', roupa='#55506a', roupa_c='#7d7796',
         roupa_s='#3a3452', detalhe='#f4ecd8', calca='#3a3452', bota='#2b2740', bota_c='#55506a',
         antena=MAGENTA_CLARO, chapeu=False, poncho=False, avental=True),
    # 3: freguês da mesa do canto
    dict(pele='#8fd070', pele_c='#c8f0a8', pele_s='#5a9a4a', roupa='#8f5a3a', roupa_c='#c08a58',
         roupa_s='#6a4028', detalhe=MAGENTA_CLARO, calca='#4a4366', bota='#2b2740', bota_c='#55506a',
         antena=LARANJA_CLARO, chapeu=False, poncho=False),
]


def alien(p, lado, pisca):
    t = Tela(44, 58)
    cx = 22 if not lado else 23
    # pernas e botas
    pernas = (15, 24) if not lado else (17, 23)
    for x in pernas:
        t.ret(x, 46, 6, 8, p['calca'])
        t.ret(x - 1 + (1 if lado else 0), 53, 7, 4, p['bota'])
        t.ret(x - 1 + (1 if lado else 0), 53, 7, 1, p['bota_c'])
    if lado:
        t.ret(27, 55, 3, 2, p['bota'])                         # ponta da bota para a frente

    # braços (de frente: um de cada lado; de lado: só o de cá, por cima do tronco)
    if not lado:
        for x, c, mao in ((8, p['roupa'], p['pele']), (32, p['roupa_s'], p['pele_s'])):
            t.ret(x, 34, 4, 11, c)
            t.ret(x, 44, 4, 3, mao)

    # tronco
    if p.get('poncho'):
        esq, dir_ = (9, 35) if not lado else (13, 33)
        t.poligono([(cx - 7, 30), (cx + 7, 30), (dir_, 47), (esq, 47)], p['roupa'])
        t.poligono([(cx + 3, 30), (cx + 7, 30), (dir_, 47), (dir_ - 6, 47)], p['roupa_s'])
        for x in range(esq, dir_):                               # barra do poncho, em zigue-zague
            t.px(x, 46 + (x % 2), p['detalhe'])
        t.ret(esq + 2, 41, dir_ - esq - 4, 1, p['roupa_c'])
        t.ret(esq + 3, 43, dir_ - esq - 6, 1, p['roupa_s'])
    else:
        esq, dir_ = (12, 32) if not lado else (15, 30)
        t.ret(esq, 31, dir_ - esq, 16, p['roupa'])
        t.ret(dir_ - 4, 31, 4, 16, p['roupa_s'])
        t.ret(esq, 31, 2, 16, p['roupa_c'])
        t.ret(esq, 45, dir_ - esq, 2, p['roupa_s'])
        if p.get('avental'):
            t.ret(cx - 5, 35, 10, 12, '#f4ecd8')
            t.ret(cx + 3, 35, 2, 12, '#c9c0b0')
            t.ret(cx - 5, 35, 10, 1, '#c9c0b0')
            t.ret(cx - 2, 39, 4, 3, p['roupa_c'])                # bolso
        else:
            t.ret(cx - 1 if not lado else cx + 2, 32, 2, 14, p['detalhe'])   # listra de luz
            t.ret(esq + 2, 37, 4, 1, p['roupa_c'])
    if lado:
        t.ret(21, 34, 5, 11, p['roupa_c'] if not p.get('poncho') else p['roupa'])
        t.ret(25, 34, 1, 11, p['roupa_s'])
        t.ret(21, 44, 5, 3, p['pele'])

    # antenas (atrás da cabeça)
    if not p.get('chapeu'):
        pontas = ((16, 11, 12, 3), (28, 11, 32, 3)) if not lado else ((20, 11, 15, 3),)
        for x0, y0, x1, y1 in pontas:
            t.linha(x0, y0, x1, y1, p['pele_s'], 2)
            t.ret(x1 - 1, y1 - 2, 3, 3, p['antena'])

    # cabeça grande
    t.oval(cx, 20, 13.2, 11.3, p['pele_s'])
    t.oval(cx - 1, 19, 12, 10.2, p['pele'])
    t.oval(cx - 4, 14, 5, 2.6, p['pele_c'])
    olhos = ((cx - 6.5, 21.5), (cx + 6.5, 21.5)) if not lado else ((cx + 6, 21.5),)
    for ex, ey in olhos:
        if pisca:
            t.ret(int(ex - 3), int(ey), 7, 2, p['pele_s'])
        else:
            t.oval(ex, ey, 3.7 if not lado else 3.2, 4.8, OLHO)
            t.px(int(ex - 1), int(ey - 3), OLHO_BRILHO)
            t.px(int(ex - 2), int(ey - 2), OLHO_BRILHO)
            t.px(int(ex + 1), int(ey + 2), '#4a4366')
    if not lado:
        t.ret(cx - 2, 28, 4, 1, p['pele_s'])                     # boca
    else:
        t.ret(cx + 6, 28, 4, 1, p['pele_s'])
        t.ret(cx + 12, 22, 1, 3, p['pele_s'])                    # narizinho

    # chapéu de aba larga
    if p.get('chapeu'):
        hx = cx
        t.oval(hx, 12, 18, 3.6, '#3a2a30')
        t.oval(hx, 11.4, 17, 2.6, '#5a4038')
        t.ret(hx - 8, 3, 16, 9, '#5a4038')
        t.oval(hx, 3.5, 8, 2.5, '#6e5046')
        t.ret(hx - 8, 3, 3, 8, '#6e5046')
        t.ret(hx + 5, 3, 3, 9, '#3a2a30')
        t.ret(hx - 8, 9, 16, 2, p['detalhe'])
        t.ret(hx + 5, 9, 3, 2, LARANJA_ESCURO)
        t.px(hx - 4, 9, LARANJA_CLARO)
    t.contornar(CONTORNO_ALIEN)
    return t


folha = np.zeros((58 * len(ALIENS), 44 * 6, 4), np.uint8)
for i, p in enumerate(ALIENS):
    quadros = [alien(p, False, False), alien(p, False, True), alien(p, True, False), alien(p, True, True)]
    quadros += [quadros[2].espelhar(), quadros[3].espelhar()]
    for k, q in enumerate(quadros):
        folha[i * 58:(i + 1) * 58, k * 44:(k + 1) * 44] = q.c
Image.fromarray(folha).save('assets/imagens/et_aliens.png')

# =====================================================================
# 4. OBJETOS: poste de luz (24x72)
# =====================================================================
POSTE = figura([
    "....kkkkkkk.",
    "...kSSSSSSSk",
    "..kSsskkkkOk",
    "..kSk..kYYYk",
    "..kSk..kYWYk",
    "..kSk..kYYYk",
    "..kSk...kkk.",
] + ["..kSk......."] * 25 + [
    ".kkSkk......",
    "kSSSsSk.....",
    "kssssssk....",
    ".kkkkkk.....",
], {'k': CONTORNO, 'S': '#8a84a0', 's': '#55506a', 'O': LARANJA, 'Y': LARANJA_CLARO, 'W': '#fff6d8'})
Image.fromarray(ampliar(POSTE.c)).save('assets/imagens/et_objetos.png')

# =====================================================================
# 5. CARTAS (76x104 cada): verso, valores de 1 a 9 e lugar vazio
# =====================================================================
LC, AC = 38, 52
fonte8 = ImageFont.truetype(FONTE, 8)
fonte16 = ImageFont.truetype(FONTE, 16)


def numero(texto, fonte):
    """Desenho do número, sem suavização: True onde há tinta."""
    im = Image.new('1', (20, 20), 0)
    ImageDraw.Draw(im).text((0, 0), texto, font=fonte, fill=1)
    a = np.array(im)
    ys, xs = np.nonzero(a)
    return a[ys.min():ys.max() + 1, xs.min():xs.max() + 1]


def carimbar(t, tinta, x, y, c):
    for j in range(tinta.shape[0]):
        for i in range(tinta.shape[1]):
            if tinta[j, i]:
                t.px(x + i, y + j, c)


def carta_base(papel, borda):
    t = Tela(LC, AC)
    t.ret(0, 0, LC, AC, '#241a3a')
    t.ret(1, 1, LC - 2, AC - 2, papel)
    t.ret(2, 2, LC - 4, 1, borda), t.ret(2, AC - 3, LC - 4, 1, borda)
    t.ret(2, 2, 1, AC - 4, borda), t.ret(LC - 3, 2, 1, AC - 4, borda)
    for x, y in ((0, 0), (1, 0), (0, 1), (LC - 1, 0), (LC - 2, 0), (LC - 1, 1),
                 (0, AC - 1), (1, AC - 1), (0, AC - 2), (LC - 1, AC - 1), (LC - 2, AC - 1), (LC - 1, AC - 2)):
        t.c[y, x] = 0                                              # cantos arredondados
    for x, y in ((1, 1), (LC - 2, 1), (1, AC - 2), (LC - 2, AC - 2)):
        t.px(x, y, '#241a3a')
    return t


# naipes pelas cores: cartas baixas, médias e altas
NAIPES = [('#2b8a8f', '#4fe0d8', '#b9f5ee'), ('#5a4ab8', '#9b8ce0', '#d6cbf8'), ('#a0306e', '#e868b0', '#ffc0e0')]
cartas = []

verso = carta_base('#3a2a5c', '#e8c86a')
for y in range(4, AC - 4):
    for x in range(4, LC - 4):
        if (x + y) % 6 == 0 or (x - y) % 6 == 0:
            verso.px(x, y, '#4f3c7a')
verso.oval(19, 26, 9.5, 9.5, '#241a3a')
verso.oval(19, 26, 8, 8, '#2b8a8f')
verso.oval(17, 24, 5, 5, '#4fe0d8')
verso.oval(16, 23, 2, 2, '#b9f5ee')
for dx in range(-13, 14):
    dy = round(dx * -0.3)
    if abs(dx) > 8 or dy > 0 or dx > 5:
        verso.px(19 + dx, 26 + dy, '#e8c86a')
        verso.px(19 + dx, 27 + dy, '#8f7632')
for x, y in ((7, 7), (30, 7), (7, 44), (30, 44)):
    verso.px(x, y, '#e8c86a'), verso.px(x - 1, y, '#8f7632'), verso.px(x + 1, y, '#8f7632')
    verso.px(x, y - 1, '#8f7632'), verso.px(x, y + 1, '#8f7632')
cartas.append(verso)

for valor in range(1, 10):
    escuro, medio, claro = NAIPES[(valor - 1) // 3]
    t = carta_base('#f1e8d2', medio)
    r = np.random.default_rng(valor)
    for _ in range(26):                                            # papel com fibras
        t.px(int(r.integers(4, LC - 4)), int(r.integers(4, AC - 4)), '#e4d8bc')
    pequeno = numero(str(valor), fonte8)
    carimbar(t, pequeno, 5, 5, escuro)
    carimbar(t, pequeno[::-1, ::-1], LC - 5 - pequeno.shape[1], AC - 5 - pequeno.shape[0], escuro)
    # planeta com anel no meio, e o número grande por cima
    t.oval(19, 26, 11, 11, escuro)
    t.oval(18, 25, 9.6, 9.6, medio)
    t.oval(15, 21, 4, 3, claro)
    for dx in range(-15, 16):
        dy = round(dx * -0.25)
        if abs(dx) > 11:
            t.px(19 + dx, 27 + dy, escuro)
    grande = numero(str(valor), fonte16)
    gx, gy = 19 - grande.shape[1] // 2, 26 - grande.shape[0] // 2
    carimbar(t, grande, gx + 1, gy + 1, '#241a3a')
    carimbar(t, grande, gx, gy, '#fff8e8')
    # uma estrelinha para cada ponto do valor
    for k in range(valor):
        t.px(19 - (valor - 1) + k * 2, 44, escuro)
    cartas.append(t)

vazio = Tela(LC, AC)
for x in range(2, LC - 2):
    if x % 4 < 2:
        vazio.px(x, 1, '#514464'), vazio.px(x, AC - 2, '#514464')
for y in range(2, AC - 2):
    if y % 4 < 2:
        vazio.px(1, y, '#514464'), vazio.px(LC - 2, y, '#514464')
cartas.append(vazio)

folha = np.zeros((AC, LC * len(cartas), 4), np.uint8)
for i, t in enumerate(cartas):
    folha[:, i * LC:(i + 1) * LC] = t.c
Image.fromarray(ampliar(folha)).save('assets/imagens/et_cartas.png')

# =====================================================================
# 6. MESA DE CARTAS (tela do jogo): fundo em cima, tampo embaixo
# =====================================================================
# O jogo desenha o fundo, depois o viajante e por último o tampo, que
# tem a parte de cima transparente. Assim ele aparece sentado atrás da mesa.
fundo = Cena(320, 180, '#1a1422')
fx_, fy_ = fundo.xx, fundo.yy
fundo.pintar((fx_ % 14 == 0) & (fy_ < 100), '#150f1c')
fundo.pontilhar(fundo.area(0, 0, 320, 100), '#20182a', 0.5)
# prateleira com garrafas ao longe, fora de foco
fundo.ret(0, 52, 320, 2, '#3a2438')
r = np.random.default_rng(21)
for x in range(4, 316, 7):
    c = [MAGENTA_ESCURO, CIANO_ESCURO, LARANJA_ESCURO, '#3f7a45', '#5a4ab8'][int(r.integers(0, 5))]
    alt = int(r.integers(6, 13))
    fundo.ret(x, 52 - alt, 4, alt, c)
    fundo.pontilhar(fundo.area(x, 52 - alt, 2, alt), '#f4ecd8', 0.2)
for lx, ly, lr, c in ((40, 30, 46, MAGENTA), (268, 26, 52, CIANO), (160, 14, 60, LARANJA),
                      (300, 70, 30, MAGENTA), (14, 74, 26, CIANO)):
    fundo.luz(lx, ly, lr, lr * 0.7, c, 0.4)
fundo.luz(160, 92, 150, 60, LARANJA_CLARO, 0.25)

tampo = Cena(320, 180, '#000000')
tx_, ty_ = tampo.xx, tampo.yy
CXM, CYM, RXM, RYM = 160, 208, 214, 136                      # o tampo é o alto de uma elipse grande
borda_m = ((tx_ - CXM) / RXM) ** 2 + ((ty_ - CYM) / RYM) ** 2 <= 1
feltro = ((tx_ - CXM) / (RXM - 9)) ** 2 + ((ty_ - CYM) / (RYM - 7)) ** 2 <= 1
tampo.pintar(borda_m, '#6a4028')
tampo.pintar(borda_m & ~np.roll(borda_m, 1, 0), '#241a3a')
tampo.pintar(borda_m & np.roll(borda_m, 1, 0) & ~np.roll(borda_m, 2, 0), '#d8a070')
tampo.pintar(borda_m & np.roll(borda_m, 2, 0) & ~np.roll(borda_m, 3, 0), '#b0764a')
tampo.pintar(borda_m & ~feltro & (tx_ % 23 == 0), '#4a2a1c')
tampo.pintar(feltro, '#1b4f52')
tampo.pintar(feltro & ~np.roll(feltro, 1, 0), '#241a3a')
tampo.pintar(feltro & np.roll(feltro, 1, 0) & ~np.roll(feltro, 2, 0), '#123a3e')
d = np.sqrt(((tx_ - 160) / 170) ** 2 + ((ty_ - 124) / 70) ** 2)
luz_ = np.clip(1 - d, 0, 1)
nivel = np.floor(luz_ * 3.2 + tampo.bayer)
for k, c in ((1, '#1f5a5a'), (2, '#256b66'), (3, '#2e7c72')):
    tampo.pintar(feltro & np.roll(feltro, 2, 0) & (nivel >= k), c)
sf = tampo.sorteio(5)
tampo.pintar(feltro & np.roll(feltro, 2, 0) & (sf > 0.99), '#3a8a7c')
# desenho bordado no feltro: uma órbita e estrelas
orbita = tampo.elipse(160, 122, 118, 34) & ~tampo.elipse(160, 122, 116.6, 33)
tampo.pintar(orbita & feltro & (tx_ % 4 < 3), '#3a9488')
for ex, ey in ((52, 118), (268, 118), (160, 88), (96, 150), (224, 150)):
    tampo.ret(ex, ey - 1, 1, 3, '#4fb0a0'), tampo.ret(ex - 1, ey, 3, 1, '#4fb0a0')
# copo com bebida acesa e pilha de fichas
tampo.escurecer(tampo.elipse(24, 131, 9, 3), 0.4)
tampo.ret(15, 108, 14, 22, '#241a3a')
tampo.ret(16, 109, 12, 20, '#3a5a78')
tampo.ret(16, 116, 12, 13, MAGENTA), tampo.ret(16, 116, 12, 1, MAGENTA_CLARO)
tampo.ret(17, 110, 2, 18, '#b9f5ee')
tampo.luz(22, 124, 22, 14, MAGENTA, 0.3, feltro)
for i, (px_, n_, c) in enumerate(((292, 5, LARANJA), (302, 3, CIANO), (284, 2, MAGENTA_CLARO))):
    tampo.escurecer(tampo.elipse(px_ + 5, 133, 6, 2), 0.4)
    for k in range(n_):
        y = 130 - k * 3
        tampo.ret(px_ - 1, y - 1, 10, 4, '#241a3a')
        tampo.ret(px_, y, 8, 2, c)
        tampo.ret(px_ + 2, y, 1, 2, '#f4ecd8'), tampo.ret(px_ + 5, y, 1, 2, '#f4ecd8')

mesa = np.zeros((360, 320, 4), np.uint8)
mesa[:180, :, :3] = fundo.img
mesa[:180, :, 3] = 255
mesa[180:, :, :3] = tampo.img
mesa[180:, :, 3] = np.where(borda_m, 255, 0)
Image.fromarray(ampliar(mesa)).save('assets/imagens/planetas/et_mesa.png')
print('aliens, objetos, cartas e mesa ok')


# =====================================================================
# 7. FLIPERAMA: naves (16x16, ampliadas 2x) e o gabinete
# =====================================================================
# Ordem na folha: jogador, batedor (2 quadros), caça (2), comandante (2),
# explosão (4). O jogo usa a mesma ordem (src/fliperama.c).
PALETA_NAVES = {
    'k': '#16121f', 'W': '#f4ecd8', 'w': '#c9c0b0', 'O': LARANJA, 'o': LARANJA_ESCURO,
    'C': CIANO, 'c': CIANO_ESCURO, 'Y': LARANJA_CLARO, 'S': '#8a84a0',
    'G': '#8fd070', 'g': '#4f8a40', 'L': '#c8f0a8',
    'M': MAGENTA, 'm': MAGENTA_ESCURO, 'P': MAGENTA_CLARO,
    'D': '#e8c86a', 'd': '#8f7632', 'R': '#ee6a5a',
}

NAVES = [
    # jogador: a nave da tripulação, branca e laranja como o traje
    ["................",
     ".......kk.......",
     "......kWWk......",
     "......kCCk......",
     ".....kWCCWk.....",
     ".....kWccWk.....",
     "....kWWWWWWk....",
     "...kOWWWWWWOk...",
     "..kOOWWOOWWOOk..",
     ".kOOkWWOOWWkOOk.",
     "kOOk.kWwwWk.kOOk",
     "kok..kWWWWk..kok",
     "kk...kSSSSk...kk",
     ".....kkYYkk.....",
     "......kYYk......",
     ".......kk......."],
    # batedor, quadro 1: besouro verde de patinhas
    ["................",
     "................",
     "....k......k....",
     ".....k....k.....",
     "....kkkkkkkk....",
     "...kGGGGGGGGk...",
     "..kGLLGGGGLLGk..",
     "..kGkLGGGGkLGk..",
     ".kGGGGGGGGGGGGk.",
     ".kgGGGggggGGGgk.",
     "..kgGkkkkkkGgk..",
     "...kk.k..k.kk...",
     "..k..k....k..k..",
     "................",
     "................",
     "................"],
    # batedor, quadro 2: patinhas para dentro
    ["................",
     "................",
     "...k........k...",
     "....k......k....",
     "....kkkkkkkk....",
     "...kGGGGGGGGk...",
     "..kGLLGGGGLLGk..",
     "..kGkLGGGGkLGk..",
     ".kGGGGGGGGGGGGk.",
     ".kgGGGggggGGGgk.",
     "..kgGkkkkkkGgk..",
     "....k.kkkk.k....",
     "...k..k..k..k...",
     "................",
     "................",
     "................"],
    # caça, quadro 1: asas magenta abertas
    ["................",
     "................",
     "kk............kk",
     "kPk....kk....kPk",
     "kMPk..kMMk..kPMk",
     "kMMPkkMCCMkkPMMk",
     "kMMMMMCCCCMMMMMk",
     ".kmMMMMCCMMMMmk.",
     "..kmMMMMMMMMmk..",
     "...kmmMMMMmmk...",
     "....kkmMMmkk....",
     "......kOOk......",
     ".......kk.......",
     "................",
     "................",
     "................"],
    # caça, quadro 2: asas um pouco fechadas
    ["................",
     "................",
     "................",
     ".kk....kk....kk.",
     ".kPk..kMMk..kPk.",
     ".kMPkkMCCMkkPMk.",
     "kMMMMMCCCCMMMMMk",
     "kmMMMMMCCMMMMMmk",
     ".kmMMMMMMMMMMmk.",
     "..kmmMMMMMMmmk..",
     "....kkmMMmkk....",
     "......kYYk......",
     ".......kk.......",
     "................",
     "................",
     "................"],
    # comandante, quadro 1: disco dourado com cúpula
    ["................",
     "......kkkk......",
     ".....kCCCCk.....",
     "....kCcCCCCk....",
     "....kCCCCCCk....",
     "..kkkkkkkkkkkk..",
     ".kDDDDDDDDDDDDk.",
     "kDDRDDDRDDDRDDDk",
     "kdDDDDDDDDDDDDdk",
     ".kddddddddddddk.",
     "..kkdkkkkkkdkk..",
     "...kOk....kOk...",
     "....k......k....",
     "................",
     "................",
     "................"],
    # comandante, quadro 2: as luzes andam
    ["................",
     "......kkkk......",
     ".....kCCCCk.....",
     "....kCcCCCCk....",
     "....kCCCCCCk....",
     "..kkkkkkkkkkkk..",
     ".kDDDDDDDDDDDDk.",
     "kDDDDRDDDRDDDRDk",
     "kdDDDDDDDDDDDDdk",
     ".kddddddddddddk.",
     "..kkdkkkkkkdkk..",
     "...kYk....kYk...",
     "....k......k....",
     "................",
     "................",
     "................"],
]
for desenho in NAVES:
    assert len(desenho) == 16 and all(len(linha) == 16 for linha in desenho), desenho

quadros = [figura(d, PALETA_NAVES) for d in NAVES]
# explosões: anel que cresce e se desfaz em faíscas
for k in range(4):
    t = Tela(16, 16)
    r = np.random.default_rng(40 + k)
    raio = 2 + k * 2
    for y in range(16):
        for x in range(16):
            d = np.hypot(x - 7.5, y - 7.5)
            if abs(d - raio) < 1.1 and r.random() > 0.15 * k:
                t.px(x, y, ['#fff6d8', LARANJA_CLARO, LARANJA, MAGENTA][k])
            elif d < raio - 1.5 and r.random() > 0.55 + 0.15 * k:
                t.px(x, y, [LARANJA_CLARO, LARANJA, MAGENTA_ESCURO, '#3a2440'][k])
    quadros.append(t)
folha = np.zeros((16, 16 * len(quadros), 4), np.uint8)
for i, q in enumerate(quadros):
    folha[:, i * 16:(i + 1) * 16] = q.c
Image.fromarray(ampliar(folha)).save('assets/imagens/et_naves.png')

# ---------- o gabinete ----------
# Grade de 320x180. A tela fica em (60, 20), com 200x148: no jogo,
# (120, 40) com 400x296, o tamanho de FLIP_LARGURA x FLIP_ALTURA.
TX0, TY0, TW, TH = 60, 20, 200, 148
gab = Cena(320, 180, '#1a1422')
gx, gy = gab.xx, gab.yy
# parede do bar atrás, em lambri escuro e com o brilho do neon
gab.pintar(gx % 10 == 0, '#150f1c')
gab.pontilhar(gx % 10 == 1, '#241a2c', 0.5)
gab.ret(0, 150, 320, 30, '#2a1e30')
gab.pintar(gab.area(0, 150, 320, 30) & ((gx + (gy // 6) * 17) % 25 == 0), '#1f1626')
gab.ret(0, 149, 320, 1, '#3d2840')
gab.luz(160, 90, 190, 110, MAGENTA, 0.35)
gab.luz(160, 10, 120, 30, CIANO, 0.25)

# corpo do gabinete: laterais magenta, frente escura
gab.escurecer(gab.area(48, 4, 230, 176), 0.5)
gab.ret(42, 0, 236, 180, CONTORNO)
gab.ret(43, 0, 234, 180, '#5a2050')
gab.ret(43, 0, 4, 180, '#7a2a66')
gab.ret(273, 0, 4, 180, '#3a1236')
gab.ret(47, 0, 1, 180, MAGENTA_CLARO)                      # filete de luz nas laterais
gab.ret(272, 0, 1, 180, MAGENTA)
# moldura da tela (bisel preto, com reflexo)
gab.ret(TX0 - 8, TY0 - 6, TW + 16, TH + 12, '#0e0a14')
gab.ret(TX0 - 8, TY0 - 6, TW + 16, 1, '#3a2a48')
gab.ret(TX0 - 4, TY0 - 3, TW + 8, TH + 6, '#221a2e')
gab.ret(TX0 - 1, TY0 - 1, TW + 2, TH + 2, '#06040a')
# letreiro em cima: naves e símbolos acesos
gab.ret(52, 1, 216, 12, CONTORNO)
gab.ret(53, 2, 214, 10, '#2a1230')
for i, (x, c) in enumerate(((60, CIANO), (246, CIANO))):
    for j, linha in enumerate(NAVES[0][2:12]):
        for k_, letra in enumerate(linha[2:14]):
            if letra not in '.k' and j % 1 == 0 and k_ % 2 == 0 and j % 2 == 0:
                gab.ret(x + k_ // 2 + 2, 3 + j // 2, 1, 1, c)
rabiscos_cores = (MAGENTA_CLARO, LARANJA_CLARO)
for i in range(9):
    x = 92 + i * 15
    forma = i % 3
    c = rabiscos_cores[i % 2]
    if forma == 0:
        gab.ret(x, 4, 7, 1, c), gab.ret(x + 3, 4, 1, 6, c), gab.ret(x, 9, 7, 1, c)
    elif forma == 1:
        gab.ret(x, 4, 1, 6, c), gab.ret(x + 6, 4, 1, 6, c), gab.ret(x, 7, 7, 1, c)
    else:
        gab.ret(x, 4, 7, 1, c), gab.ret(x + 6, 4, 1, 6, c), gab.ret(x, 9, 4, 1, c)
gab.luz(160, 7, 110, 12, MAGENTA, 0.35, gab.area(42, 0, 236, 18))
# painel de controle embaixo: alavanca e dois botões
gab.ret(46, 170, 228, 10, '#3a1236')
gab.ret(46, 170, 228, 1, '#7a2a66')
gab.ret(100, 172, 14, 6, '#16121f')
gab.ret(105, 166, 4, 8, '#8a84a0')
gab.pintar(gab.elipse(107, 165, 4, 3.5), '#d0489a')
gab.pintar(gab.elipse(106, 164, 1.5, 1.2), MAGENTA_CLARO)
for bx_, c, c2 in ((196, CIANO, CIANO_CLARO), (212, LARANJA, LARANJA_CLARO)):
    gab.pintar(gab.elipse(bx_, 175, 5, 2.6), '#16121f')
    gab.pintar(gab.elipse(bx_, 174, 4, 2), c)
    gab.pintar(gab.elipse(bx_ - 1, 173.5, 1.5, 0.8), c2)

alfa = np.full((180, 320), 255, np.uint8)
alfa[TY0:TY0 + TH, TX0:TX0 + TW] = 0                         # a tela fica transparente
moldura = np.dstack([gab.img, alfa])
Image.fromarray(ampliar(moldura)).save('assets/imagens/planetas/et_fliperama.png')
print('fliperama ok')

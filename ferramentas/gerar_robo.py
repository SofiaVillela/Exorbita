#!/usr/bin/env python3
"""gerar_robo.py: desenha os gráficos do planeta do robô e o tradutor.

Gera, a partir da pasta do projeto:
  assets/imagens/planetas/robo_laboratorio.png  (o laboratório, 640x360)
  assets/mascaras/robo_laboratorio_mascara.png  (branco = piso, 640x360)
  assets/imagens/planetas/robo_laboratorio_frente.png  (só os móveis)
  assets/imagens/planetas/robo_bancada.png      (fundo do desafio, 640x360)
  assets/imagens/robo_cientista.png             (o cientista, quadros de 44x58)
  assets/imagens/robo_circuito.png              (peças do desafio, 40x40 cada)
  assets/imagens/tradutor.png                   (o aparelho, 32x32)

Rodar:  python3 ferramentas/gerar_robo.py
Precisa de numpy e Pillow. O jogo não precisa deste arquivo para rodar:
ele só é usado para refazer as imagens.

Tudo é desenhado numa grade de 320x180 e ampliado 2x. As posições
(bancada, redoma, porta) precisam bater com as de src/tela_robo.c, que
usa as mesmas medidas multiplicadas por 2.
"""

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import numpy as np
from PIL import Image

from pixel_arte import Cena, Tela, ampliar, cor, figura

# ---------- posições do laboratório (grade de 320x180) ----------
SALA = (10, 310, 62, 164)        # x0, x1 do salão e y do chão (começo e fim)
PORTA = (36, 60)                 # vão na parede de baixo: a saída para o módulo
BANCADA = (118, 202, 94, 106)    # x0, x1 e y da base da bancada de comandos
REDOMA = (262, 100)              # centro da base da redoma do tradutor

# ---------- cores ----------
CONTORNO = '#16121f'
CIANO, CIANO_ESCURO, CIANO_CLARO = '#68d8cd', '#2b8a8f', '#b9f5ee'
LARANJA, LARANJA_CLARO, LARANJA_ESCURO = '#f2a541', '#ffd98a', '#b06a26'
LILAS, LILAS_CLARO, LILAS_ESCURO = '#a99be6', '#d6cbf8', '#6a5cb0'
VERDE, VERDE_CLARO, VERDE_ESCURO = '#6fbf5a', '#b8f0a0', '#3f7a45'
ACO, ACO_CLARO, ACO_ESCURO = '#8a84a0', '#c4bfd6', '#55506a'
PAREDE, PAREDE_ESCURA, PAREDE_CLARA = '#4d5873', '#3a4359', '#6a7693'

# =====================================================================
# 1. O LABORATÓRIO
# =====================================================================
lab = Cena(320, 180, CONTORNO)
xx, yy, bayer = lab.xx, lab.yy, lab.bayer
x0, x1, y0, y1 = SALA

# piso de chapas de aço, com rebites nos cantos
piso = lab.area(x0, y0, x1 - x0, y1 - y0)
n = lab.ruido(30, 1)
lab.img[piso] = np.array([cor('#333a50'), cor('#394158'), cor('#3f485f')])[np.digitize(n + (bayer - 0.5) * 0.1, [0.42, 0.6])][piso]
lab.pintar(piso & (((xx - x0) % 20 == 0) | ((yy - y0) % 20 == 0)), '#272d40')
lab.pintar(piso & (((xx - x0) % 20 == 1) | ((yy - y0) % 20 == 1)) & (bayer < 0.35), '#4a5470')
lab.pintar(piso & ((xx - x0) % 20 == 3) & ((yy - y0) % 20 == 3), '#566080')
lab.pintar(piso & ((xx - x0) % 20 == 17) & ((yy - y0) % 20 == 17), '#262c3e')
# trilhos de luz no chão: levam da porta até a bancada
for x in range(x0 + 6, x1 - 6):
    if x % 8 < 5:
        lab.ret(x, 128, 1, 1, CIANO_ESCURO)
for x in range(x0 + 10, x1 - 6, 40):
    lab.ret(x, 128, 3, 1, CIANO)
    lab.luz(x + 1, 128, 8, 4, CIANO, 0.3, piso)
# faixa de aviso em volta do lugar da bancada
bx0, bx1, by0, by1 = BANCADA
faixa = lab.area(bx0 - 8, by1 + 2, bx1 - bx0 + 16, 4)
lab.pintar(faixa & (((xx + yy) // 3) % 2 == 0), LARANJA_ESCURO)
lab.pintar(faixa & (((xx + yy) // 3) % 2 == 1), '#262c3e')

# parede do fundo, em painéis
lab.ret(x0, 18, x1 - x0, y0 - 18, PAREDE)
fundo = lab.area(x0, 22, x1 - x0, y0 - 22)
lab.pintar(fundo & (xx % 26 == 0), PAREDE_ESCURA)
lab.pintar(fundo & (xx % 26 == 1) & (bayer < 0.5), PAREDE_CLARA)
lab.ret(x0, 18, x1 - x0, 4, '#7d89a6')
lab.ret(x0, 18, x1 - x0, 1, '#a0abc6')
lab.ret(x0, 22, x1 - x0, 2, CONTORNO)
lab.ret(x0, y0 - 6, x1 - x0, 6, PAREDE_ESCURA)
lab.ret(x0, y0 - 7, x1 - x0, 1, PAREDE_CLARA)
lab.ret(x0, y0 - 3, x1 - x0, 1, CIANO_ESCURO)
lab.ret(x0, y0 - 1, x1 - x0, 1, CONTORNO)

# telão do meio: gráficos e a grade do desafio
lab.ret(108, 25, 104, 30, CONTORNO)
lab.ret(109, 26, 102, 28, '#0f2a30')
lab.pintar(lab.area(109, 26, 102, 28) & (yy % 2 == 0), '#0b2025')
for gx in range(116, 152, 5):                                 # grade pequena à esquerda
    lab.ret(gx, 30, 1, 21, CIANO_ESCURO)
for gy in range(30, 52, 5):
    lab.ret(116, gy, 36, 1, CIANO_ESCURO)
for gx_, gy_, c in ((121, 35, LARANJA), (131, 40, LARANJA), (141, 45, CIANO_CLARO), (126, 30, '#ee6a5a')):
    lab.ret(gx_ + 1, gy_ + 1, 4, 4, c)
onda_x = np.arange(158, 206)
for i, x in enumerate(onda_x):                                # curva no gráfico da direita
    y = int(40 + 7 * np.sin(i / 5) * np.cos(i / 13))
    lab.ret(int(x), y, 1, 1, CIANO)
    lab.ret(int(x), y + 1, 1, 1, CIANO_ESCURO)
for bx_, h in ((160, 5), (166, 9), (172, 4), (178, 7)):
    lab.ret(bx_, 52 - h, 4, h, VERDE)
lab.ret(186, 46, 20, 1, LARANJA), lab.ret(186, 49, 14, 1, LARANJA_ESCURO)
lab.luz(160, 40, 80, 26, CIANO, 0.3)
# telas pequenas dos lados
for sx, c in ((84, VERDE), (218, LARANJA)):
    lab.ret(sx, 30, 18, 14, CONTORNO)
    lab.ret(sx + 1, 31, 16, 12, '#152028')
    for k in range(33, 42, 2):
        lab.ret(sx + 3, k, 12 - (k * 3 + sx) % 6, 1, c)

# tanques de líquido verde, à esquerda
for tx in (22, 46):
    lab.ret(tx, 24, 18, 44, CONTORNO)
    lab.ret(tx + 1, 25, 16, 42, '#1c3a34')
    lab.ret(tx + 1, 31, 16, 36, '#3f9a5a')
    lab.pontilhar(lab.area(tx + 1, 31, 16, 10), VERDE_CLARO, 0.45)
    lab.ret(tx + 2, 26, 2, 40, '#b8f0d8')                       # reflexo do vidro
    r = np.random.default_rng(tx)
    for _ in range(7):
        lab.ret(tx + 5 + int(r.integers(0, 10)), 34 + int(r.integers(0, 30)), 1, 1, VERDE_CLARO)
    lab.ret(tx - 1, 22, 20, 4, ACO), lab.ret(tx - 1, 22, 20, 1, ACO_CLARO)
    lab.ret(tx - 1, 64, 20, 6, ACO_ESCURO), lab.ret(tx - 1, 64, 20, 1, ACO)
    lab.bloquear(tx - 1, 62, 20, 8)
lab.luz(44, 76, 44, 16, VERDE, 0.4, piso)
# canos no alto da parede
lab.ret(66, 27, 16, 2, ACO), lab.ret(66, 29, 16, 1, PAREDE_ESCURA)
lab.ret(238, 27, 70, 2, ACO), lab.ret(238, 29, 70, 1, PAREDE_ESCURA)
for cx_ in (70, 250, 280):
    lab.ret(cx_, 26, 2, 4, LARANJA_ESCURO)

# estante de peças, à direita
lab.ret(242, 33, 62, 26, CONTORNO)
lab.ret(243, 34, 60, 24, '#2a3145')
for prateleira in (45, 57):
    lab.ret(243, prateleira, 60, 2, ACO_ESCURO)
    lab.ret(243, prateleira, 60, 1, ACO)
r = np.random.default_rng(5)
for prateleira in (45, 57):
    x = 246
    while x < 298:
        w, h = int(r.integers(4, 9)), int(r.integers(4, 9))
        c = [LILAS, ACO, LARANJA_ESCURO, CIANO_ESCURO, '#6a7693'][int(r.integers(0, 5))]
        lab.ret(x, prateleira - h, w, h, c)
        lab.ret(x, prateleira - h, w, 1, ACO_CLARO)
        x += w + int(r.integers(2, 4))

# paredes dos lados e de baixo (vistas de cima)
for wx in (4, x1):
    lab.ret(wx, 18, 6, 156, '#5d6985')
    lab.ret(wx, 18, 1, 156, '#7d89a6')
    lab.ret(wx + 5, 18, 1, 156, PAREDE_ESCURA)
lab.ret(4, y1, 312, 8, '#5d6985')
lab.ret(4, y1, 312, 1, '#7d89a6')
lab.ret(4, y1 + 7, 312, 1, PAREDE_ESCURA)
lab.bloquear(0, 0, 320, y0)
lab.bloquear(0, 0, x0, 180), lab.bloquear(x1, 0, 10, 180), lab.bloquear(0, y1, 320, 16)
# porta de saída, com faixas de aviso
px0, px1 = PORTA
lab.ret(px0, y1, px1 - px0, 9, '#151a28')
lab.pontilhar(lab.area(px0, y1 + 3, px1 - px0, 6), '#2a3145', 0.6)
lab.ret(px0 - 2, y1, 2, 8, LARANJA), lab.ret(px1, y1, 2, 8, LARANJA)
capacho = lab.area(px0 + 1, y1 - 7, px1 - px0 - 2, 6)
lab.pintar(capacho & (((xx + yy) // 3) % 2 == 0), LARANJA_ESCURO)
lab.pintar(capacho & (((xx + yy) // 3) % 2 == 1), '#262c3e')
lab.solido[y1:y1 + 6, px0:px1] = False
lab.luz((px0 + px1) / 2, y1 - 6, 24, 12, LARANJA, 0.3, piso)

# cabos pelo chão, da bancada até as paredes
for (ax, ay), (bx_, by_), c in (((bx0 + 6, by1 + 6), (44, 70), '#22283a'), ((bx1 - 6, by1 + 6), (290, 72), '#22283a'),
                                ((160, by1 + 6), (160, 150), '#2c2440')):
    passos = max(abs(bx_ - ax), abs(by_ - ay))
    for i in range(passos + 1):
        t = i / passos
        x = int(ax + (bx_ - ax) * t + 5 * np.sin(t * 9))
        y = int(ay + (by_ - ay) * t)
        if piso[y, x]:
            lab.ret(x, y, 1, 1, c)
            lab.ret(x, y + 1, 1, 1, '#4a5470' if i % 7 == 0 else c)

solido_sem_moveis = lab.solido.copy()


def movel(x, y, w, h, altura, tampo, frente, brilho, escuro):
    """Móvel com volume: base em (x, y) de w por h; bloqueia."""
    lab.escurecer(lab.area(x + 2, y + 2, w + 1, h), 0.4)
    lab.ret(x - 1, y - altura - 1, w + 2, h + altura + 2, CONTORNO)
    lab.ret(x, y - altura, w, h, tampo)
    lab.ret(x, y - altura, w, 1, brilho)
    lab.ret(x, y - altura + h, w, altura, frente)
    lab.ret(x, y - altura + h, w, 1, escuro)
    lab.bloquear(x, y - altura, w, h + altura)


# bancada de comandos: teclado, telinhas e a base do holograma no meio
movel(bx0, by0, bx1 - bx0, by1 - by0, 12, '#7d89a6', '#4d5873', '#a0abc6', PAREDE_ESCURA)
ty = by0 - 12
for k in range(bx0 + 4, bx0 + 28, 3):                          # teclas
    for j in range(ty + 3, ty + 10, 3):
        lab.ret(k, j, 2, 2, '#2a3145')
lab.ret(bx0 + 5, ty + 3, 2, 2, LARANJA), lab.ret(bx0 + 20, ty + 9, 2, 2, CIANO)
lab.ret(bx1 - 28, ty + 2, 24, 9, CONTORNO)                     # telinha
lab.ret(bx1 - 27, ty + 3, 22, 7, '#0f2a30')
lab.ret(bx1 - 25, ty + 5, 12, 1, CIANO), lab.ret(bx1 - 25, ty + 7, 16, 1, CIANO_ESCURO)
cxm = (bx0 + bx1) // 2
lab.pintar(lab.elipse(cxm, ty + 6, 11, 4.5), CONTORNO)        # base do holograma
lab.pintar(lab.elipse(cxm, ty + 6, 10, 3.6), '#1d3a48')
lab.pintar(lab.elipse(cxm, ty + 6, 6, 2), CIANO)
lab.pintar(lab.elipse(cxm, ty + 6, 2.5, 0.9), CIANO_CLARO)
lab.ret(bx0 + 2, by1 - 3, bx1 - bx0 - 4, 1, CIANO)
for k in range(bx0 + 10, bx1 - 6, 18):
    lab.ret(k, by0 + 2, 1, by1 - by0 - 5, PAREDE_ESCURA)
lab.luz(cxm, by1 + 4, 54, 10, CIANO, 0.35, piso & ~lab.solido)
lab.luz(cxm, ty + 2, 30, 16, CIANO, 0.3)

# redoma do tradutor: coluna com cúpula de vidro (o aparelho é desenhado pelo jogo)
rx, ry = REDOMA
lab.escurecer(lab.elipse(rx + 3, ry + 2, 11, 5), 0.45)
lab.pintar(lab.elipse(rx, ry, 11, 5) | lab.area(rx - 11, ry - 8, 22, 8), CONTORNO)
lab.pintar(lab.elipse(rx, ry, 10, 4) | lab.area(rx - 10, ry - 8, 20, 8), ACO_ESCURO)
lab.ret(rx - 10, ry - 8, 3, 9, ACO)
lab.pintar(lab.elipse(rx, ry - 8, 10, 4), ACO_CLARO)
lab.pintar(lab.elipse(rx, ry - 8, 8, 3), '#1d3a48')
lab.ret(rx - 6, ry - 1, 12, 1, LARANJA)
# cúpula: só o contorno e os reflexos, para o aparelho aparecer por dentro
cupula = lab.elipse(rx, ry - 20, 9, 13) & (yy < ry - 8)
borda_cupula = cupula & ~(lab.elipse(rx, ry - 20, 8, 12))
lab.pintar(cupula, '#22384a')
lab.pintar(borda_cupula, '#8fd0e0')
lab.ret(rx - 6, ry - 28, 1, 12, '#d8f4f8'), lab.ret(rx - 5, ry - 30, 1, 5, '#d8f4f8')
lab.solido |= lab.elipse(rx, ry - 1, 11, 6)
lab.luz(rx, ry + 3, 30, 12, CIANO, 0.4, piso & ~lab.solido)

# posto de recarga com um robozinho dormindo, no canto de baixo à direita
movel(278, 150, 24, 10, 10, '#55506a', '#3a3552', ACO, '#262236')
lab.ret(282, 132, 16, 9, CONTORNO)
lab.ret(283, 133, 14, 7, LILAS), lab.ret(283, 133, 14, 1, LILAS_CLARO)
lab.ret(285, 135, 10, 3, '#1d3a48'), lab.ret(287, 136, 2, 1, CIANO_ESCURO), lab.ret(291, 136, 2, 1, CIANO_ESCURO)
lab.ret(289, 129, 1, 3, ACO), lab.ret(288, 128, 3, 1, LARANJA_ESCURO)
lab.ret(280, 156, 20, 1, VERDE)
# caixas de peças, no canto de baixo à esquerda
movel(72, 152, 16, 8, 9, '#8a80ac', '#6a5f8c', '#aaa0cc', '#4a4366')
movel(92, 156, 12, 6, 7, ACO, ACO_ESCURO, ACO_CLARO, '#3a3552')
lab.ret(75, 156, 4, 1, LARANJA)

# Camada da frente: só os móveis (e o robozinho do posto de recarga).
# O jogo desenha cada móvel na ordem certa em relação a quem anda, para
# o tripulante poder passar por trás da bancada e da redoma. O vidro da
# cúpula entra só com a borda e os reflexos: o aparelho aparece por dentro.
moveis = (lab.solido & ~solido_sem_moveis) | borda_cupula | lab.area(rx - 6, ry - 30, 2, 14) \
    | lab.area(281, 127, 18, 15)
frente = np.zeros((180, 320, 4), np.uint8)
frente[..., :3] = lab.img
frente[..., 3] = np.where(moveis, 255, 0)
Image.fromarray(ampliar(frente)).save('assets/imagens/planetas/robo_laboratorio_frente.png')
Image.fromarray(ampliar(lab.img)).save('assets/imagens/planetas/robo_laboratorio.png')
mascara = np.where(lab.solido[..., None], 0, 255).astype(np.uint8).repeat(3, 2)
Image.fromarray(ampliar(mascara)).save('assets/mascaras/robo_laboratorio_mascara.png')
print('laboratório ok')

# =====================================================================
# 2. O ROBÔ CIENTISTA (quadros de 44x58, como os tripulantes)
# =====================================================================
# Colunas: de frente, de frente piscando, direita, direita piscando,
# esquerda e esquerda piscando. Ele não tem pernas: flutua sobre um
# propulsor (o jogo faz subir e descer).
CONTORNO_ROBO = '#231a36'


def cientista(lado, pisca):
    t = Tela(44, 58)
    cx = 22
    # propulsor e chama
    t.poligono([(cx - 5, 47), (cx + 5, 47), (cx + 3, 52), (cx - 3, 52)], ACO_ESCURO)
    t.ret(cx - 3, 52, 6, 2, CIANO)
    t.ret(cx - 2, 54, 4, 2, CIANO_CLARO)
    t.ret(cx - 1, 56, 2, 1, '#ffffff')
    # corpo: um barril de metal lilás, com painel de luzes
    larg = 9 if not lado else 7
    t.poligono([(cx - larg, 30), (cx + larg, 30), (cx + larg + 1, 46), (cx - larg - 1, 46)], LILAS)
    t.ret(cx + larg - 3, 30, 4, 17, LILAS_ESCURO)
    t.ret(cx - larg, 30, 2, 16, LILAS_CLARO)
    t.ret(cx - larg - 1, 45, 2 * larg + 3, 2, LILAS_ESCURO)
    if not lado:
        t.ret(cx - 5, 34, 10, 7, '#2b2740')                    # painel no peito
        t.ret(cx - 4, 35, 2, 2, CIANO), t.ret(cx - 1, 35, 2, 2, LARANJA), t.ret(cx + 2, 35, 2, 2, VERDE)
        t.ret(cx - 4, 38, 8, 1, CIANO_ESCURO)
        # jaleco: duas abas brancas abertas
        t.poligono([(cx - larg, 30), (cx - 5, 30), (cx - 7, 46), (cx - larg - 1, 46)], '#f4ecd8')
        t.poligono([(cx + 5, 30), (cx + larg, 30), (cx + larg + 1, 46), (cx + 7, 46)], '#d9d0bc')
        # braços finos com garras
        for x, c in ((cx - larg - 4, ACO), (cx + larg + 2, ACO_ESCURO)):
            t.ret(x, 33, 3, 9, c)
            t.ret(x - 1, 42, 5, 2, ACO_ESCURO)
            t.ret(x - 1, 44, 1, 2, ACO_ESCURO), t.ret(x + 3, 44, 1, 2, ACO_ESCURO)
    else:
        t.ret(cx - larg, 30, 6, 16, '#f4ecd8')                 # o jaleco, visto de lado
        t.ret(cx - 1, 33, 3, 9, ACO)
        t.ret(cx - 2, 42, 5, 2, ACO_ESCURO)
        t.ret(cx + 3, 43, 2, 1, ACO_ESCURO)
    # pescoço
    t.ret(cx - 3, 26, 6, 4, ACO_ESCURO)
    t.ret(cx - 3, 27, 6, 1, ACO)
    # cabeça: uma tela grande, com olhos de luz
    meia = 13 if not lado else 10
    hx = cx if not lado else cx + 1
    t.ret(hx - meia, 8, 2 * meia, 18, LILAS)
    t.ret(hx - meia, 8, 2 * meia, 2, LILAS_CLARO)
    t.ret(hx - meia, 8, 2, 18, LILAS_CLARO)
    t.ret(hx + meia - 3, 10, 3, 16, LILAS_ESCURO)
    t.ret(hx - meia, 24, 2 * meia, 2, LILAS_ESCURO)
    if not lado:
        t.ret(hx - 10, 11, 20, 12, '#16202a')                   # o rosto é uma tela
        t.ret(hx - 10, 11, 20, 1, '#0b1218')
        olhos = (hx - 7, hx + 3)
    else:
        t.ret(hx - 2, 11, 10, 12, '#16202a')
        t.ret(hx - 2, 11, 10, 1, '#0b1218')
        olhos = (hx + 3,)
    for ex in olhos:
        if pisca:
            t.ret(ex, 17, 4, 1, CIANO)
        else:
            t.ret(ex, 14, 4, 5, CIANO)
            t.ret(ex, 14, 4, 1, CIANO_CLARO)
            t.ret(ex + 1, 15, 1, 1, '#ffffff')
    if not lado:
        t.ret(hx - 3, 21, 6, 1, CIANO_ESCURO)                   # boca: um tracinho de luz
    # parafusos e antena com lâmpada
    t.px(hx - meia + 1, 22, LILAS_ESCURO)
    t.ret(hx - 1, 3, 2, 5, ACO)
    t.ret(hx - 2, 0, 4, 4, LARANJA)
    t.ret(hx - 2, 0, 4, 1, LARANJA_CLARO)
    if not lado:
        t.ret(hx - meia - 2, 14, 2, 6, ACO_ESCURO)              # "orelhas"
        t.ret(hx + meia, 14, 2, 6, ACO_ESCURO)
    t.contornar(CONTORNO_ROBO)
    return t


quadros = [cientista(False, False), cientista(False, True), cientista(True, False), cientista(True, True)]
quadros += [quadros[2].espelhar(), quadros[3].espelhar()]
folha = np.zeros((58, 44 * 6, 4), np.uint8)
for k, q in enumerate(quadros):
    folha[:, k * 44:(k + 1) * 44] = q.c
Image.fromarray(folha).save('assets/imagens/robo_cientista.png')

# =====================================================================
# 3. PEÇAS DO DESAFIO (20x20 na grade, 40x40 no jogo)
# =====================================================================
# Linha 0: piso A, piso B, parede, lava 1, lava 2, saída 1, saída 2, início.
# Linha 1: robozinho para baixo (2 quadros), cima (2), esquerda (2),
#          direita (2) e quebrado.
# Linha 2: setas de comando: cima, baixo, esquerda, direita.
C = 20


def piso_peca(variacao):
    t = Tela(C, C)
    t.ret(0, 0, C, C, '#2b3246')
    t.ret(1, 1, C - 2, C - 2, '#394158' if variacao == 0 else '#353c52')
    t.ret(1, 1, C - 2, 1, '#4a5470'), t.ret(1, 1, 1, C - 2, '#4a5470')
    t.ret(1, C - 2, C - 2, 1, '#2f364b'), t.ret(C - 2, 1, 1, C - 2, '#2f364b')
    for x, y in ((3, 3), (C - 4, 3), (3, C - 4), (C - 4, C - 4)):
        t.px(x, y, '#566080')
    if variacao:
        t.ret(8, 9, 4, 1, '#2f364b'), t.ret(9, 8, 1, 4, '#2f364b')
    return t


def parede_peca():
    t = piso_peca(0)
    t.ret(1, 0, C - 2, 13, '#7d89a6')                          # topo do bloco
    t.ret(1, 0, C - 2, 1, '#a0abc6'), t.ret(1, 0, 1, 13, '#a0abc6')
    t.ret(4, 3, C - 8, 7, '#6a7693')
    t.ret(4, 3, C - 8, 1, '#5d6985')
    t.ret(1, 13, C - 2, 6, '#4d5873')                          # frente
    t.ret(1, 13, C - 2, 1, '#3a4359')
    for x in range(3, C - 2, 4):
        t.ret(x, 15, 2, 2, LARANJA_ESCURO if (x // 4) % 2 == 0 else '#262c3e')
    t.ret(0, 0, 1, C, CONTORNO), t.ret(C - 1, 0, 1, C, CONTORNO), t.ret(0, C - 1, C, 1, CONTORNO)
    return t


def lava_peca(quadro):
    t = Tela(C, C)
    t.ret(0, 0, C, C, '#3a1a1e')
    r = np.random.default_rng(7)
    campo = r.random((C, C))
    for y in range(1, C - 1):
        for x in range(1, C - 1):
            onda = np.sin((x + quadro * 3) * 0.7) + np.cos((y - quadro * 2) * 0.9) + campo[y, x] * 0.8
            c = '#b03a26' if onda < -0.3 else '#e0602a' if onda < 0.6 else '#f2a541' if onda < 1.5 else '#ffd98a'
            t.px(x, y, c)
    for x, y in ((5, 6), (13, 12), (9, 15)):                      # bolhas
        yb = y - quadro
        t.ret(x, yb, 2, 2, '#ffe9b0'), t.px(x, yb, '#ffffff')
    t.ret(0, 0, C, 1, '#5a2a2a'), t.ret(0, 0, 1, C, '#5a2a2a')
    return t


def saida_peca(quadro):
    t = piso_peca(0)
    for i, c in enumerate((CIANO_ESCURO, CIANO, CIANO_CLARO)):
        m = 3 + i * 3 - (quadro if i == 0 else 0)
        m = max(2, m)
        t.ret(m, m, C - 2 * m, 1, c), t.ret(m, C - 1 - m, C - 2 * m, 1, c)
        t.ret(m, m, 1, C - 2 * m, c), t.ret(C - 1 - m, m, 1, C - 2 * m, c)
    t.ret(9, 9, 2, 2, '#ffffff' if quadro else CIANO_CLARO)
    return t


def inicio_peca():
    t = piso_peca(0)
    for x in range(3, C - 3):
        if x % 4 < 2:
            t.px(x, 3, LARANJA), t.px(x, C - 4, LARANJA)
    for y in range(3, C - 3):
        if y % 4 < 2:
            t.px(3, y, LARANJA), t.px(C - 4, y, LARANJA)
    return t


def robozinho(direcao, quadro, quebrado=False):
    """direcao: 0 baixo, 1 cima, 2 esquerda, 3 direita."""
    t = Tela(C, C)
    corpo, claro, escuro = ('#8a84a0', '#a39db8', '#55506a') if quebrado else ('#f4ecd8', '#ffffff', '#c9c0b0')
    visor = '#3a3552' if quebrado else CIANO
    # esteiras, com os gomos andando
    if direcao < 2:
        for x in (3, 14):
            t.ret(x, 5, 3, 12, '#2b2740')
            for y in range(5 + quadro, 17, 3):
                t.px(x + 1, y, ACO)
        t.ret(6, 6, 8, 10, corpo)
        t.ret(6, 6, 8, 1, claro), t.ret(6, 6, 1, 10, claro)
        t.ret(12, 7, 2, 9, escuro)
        t.ret(6, 12, 8, 1, LARANJA)
        if direcao == 0:
            t.ret(7, 8, 6, 3, '#2b2740')                        # visor na frente
            t.ret(8, 9, 1, 1, visor), t.ret(11, 9, 1, 1, visor)
        else:
            t.ret(8, 8, 4, 1, escuro), t.ret(8, 10, 4, 1, escuro)   # grade nas costas
    else:
        t.ret(4, 13, 12, 3, '#2b2740')
        for x in range(4 + quadro, 16, 3):
            t.px(x, 14, ACO)
        t.ret(5, 5, 10, 9, corpo)
        t.ret(5, 5, 10, 1, claro)
        t.ret(5, 11, 10, 1, LARANJA)
        t.ret(5, 12, 10, 1, escuro)
        if direcao == 3:
            t.ret(11, 7, 4, 3, '#2b2740'), t.ret(13, 8, 1, 1, visor)
            t.ret(5, 6, 1, 6, escuro)
        else:
            t.ret(5, 7, 4, 3, '#2b2740'), t.ret(6, 8, 1, 1, visor)
            t.ret(14, 6, 1, 6, escuro)
    # antena
    ax = 10 if direcao < 2 else (8 if direcao == 3 else 11)
    t.ret(ax, 2, 1, 4 if direcao < 2 else 3, ACO)
    t.ret(ax - 1, 1, 3, 2, '#55506a' if quebrado else LARANJA)
    t.contornar(CONTORNO_ROBO)
    if quebrado:                                                # fumaça
        for x, y in ((6, 1), (12, 0), (9, 3)):
            t.px(x, y, '#8a84a0')
    return t


def seta(direcao):
    """direcao: 0 cima, 1 baixo, 2 esquerda, 3 direita (ordem dos comandos)."""
    desenho = ["....WW....",
               "...WWWW...",
               "..WWWWWW..",
               ".WWWWWWWW.",
               "WWWWWWWWWW",
               "...WWWW...",
               "...WWWW...",
               "...WWWW...",
               "...WWWW...",
               "...wwww..."]
    t = Tela(C, C)
    mini = figura(desenho, {'W': '#f4ecd8', 'w': '#c9c0b0'}, CONTORNO)
    a = mini.c
    if direcao == 1:
        a = a[::-1]
    elif direcao == 2:
        a = np.rot90(a, 1)
    elif direcao == 3:
        a = np.rot90(a, -1)
    t.c[5:15, 5:15] = a
    return t


linhas = [
    [piso_peca(0), piso_peca(1), parede_peca(), lava_peca(0), lava_peca(1), saida_peca(0), saida_peca(1), inicio_peca()],
    [robozinho(d, q) for d in range(4) for q in range(2)] + [robozinho(0, 0, True)],
    [seta(d) for d in range(4)],
]
folha = np.zeros((C * 3, C * 9, 4), np.uint8)
for j, linha in enumerate(linhas):
    for i, q in enumerate(linha):
        folha[j * C:(j + 1) * C, i * C:(i + 1) * C] = q.c
Image.fromarray(ampliar(folha)).save('assets/imagens/robo_circuito.png')

# =====================================================================
# 4. O TRADUTOR (16x16 na grade, 32x32 no jogo)
# =====================================================================
TRADUTOR = figura([
    "...........O....",
    "...........O....",
    "...........S....",
    "....WWWWWWWW....",
    "...WWWWWWWWWw...",
    "...WKKKKKKKKw...",
    "...WKCKKCKKKw...",
    "...WKCCCKCCKw...",
    "...WKKCKKKCKw...",
    "...WKKKKKKKKw...",
    "...WWWWWWWWWw...",
    "...WLLWssWOOw...",
    "...WLLWssWOOw...",
    "...WWWWWWWWWw...",
    "....wwwwwwww....",
    "................",
], {'W': '#f4ecd8', 'w': '#c9c0b0', 'K': '#16202a', 'C': CIANO, 'O': LARANJA, 'S': ACO,
    'L': LILAS, 's': ACO_ESCURO}, CONTORNO_ROBO)
Image.fromarray(ampliar(TRADUTOR.c)).save('assets/imagens/tradutor.png')

# =====================================================================
# 5. FUNDO DO DESAFIO: a bancada de testes vista de cima
# =====================================================================
# A arena fica em (24, 46) no jogo, com 7x5 casas de 40 px. À direita,
# o monitor; embaixo, a esteira com as 16 casas da fila de comandos.
ban = Cena(320, 180, '#141824')
bxx, byy = ban.xx, ban.yy
n = ban.ruido(24, 3)
ban.img[:] = np.array([cor('#1a1f2e'), cor('#1e2434'), cor('#232a3c')])[np.digitize(n + (ban.bayer - 0.5) * 0.1, [0.42, 0.6])]
ban.pintar((bxx % 40 == 0) | (byy % 40 == 17), '#141824')
for x, y in ((6, 20), (314, 20), (6, 174), (314, 174)):
    ban.ret(x - 1, y - 1, 3, 3, ACO_ESCURO), ban.ret(x, y, 1, 1, ACO_CLARO)


def moldura(x, y, w, h, fundo_):
    """Recorte rebaixado na bancada, com borda de metal."""
    ban.escurecer(ban.area(x - 1, y - 1, w + 6, h + 6), 0.4)
    ban.ret(x - 4, y - 4, w + 8, h + 8, CONTORNO)
    ban.ret(x - 3, y - 3, w + 6, h + 6, '#5d6985')
    ban.ret(x - 3, y - 3, w + 6, 1, '#a0abc6'), ban.ret(x - 3, y - 3, 1, h + 6, '#a0abc6')
    ban.ret(x - 3, y + h + 2, w + 6, 1, PAREDE_ESCURA), ban.ret(x + w + 2, y - 3, 1, h + 6, PAREDE_ESCURA)
    ban.ret(x - 1, y - 1, w + 2, h + 2, CONTORNO)
    ban.ret(x, y, w, h, fundo_)


# arena (as casas são desenhadas pelo jogo)
moldura(12, 23, 140, 100, '#10141e')
for k in range(12, 152, 6):                                   # faixa de aviso em cima e embaixo
    c = LARANJA_ESCURO if (k // 6) % 2 == 0 else '#262c3e'
    ban.ret(k, 19, 6, 1, c), ban.ret(k, 126, 6, 1, c)
# monitor de estado
moldura(166, 23, 142, 100, '#0f2a30')
tela_ = ban.area(166, 23, 142, 100)
ban.pintar(tela_ & (byy % 2 == 0), '#0b2025')
ban.luz(237, 73, 110, 80, CIANO, 0.18, tela_)
ban.ret(166, 23, 142, 1, '#1d4a50')
# esteira da fila: 16 casas
moldura(12, 136, 296, 24, '#10141e')
for i in range(16):
    x = 16 + i * 18
    ban.ret(x, 139, 16, 18, '#232a3c')
    ban.ret(x, 139, 16, 1, '#141824'), ban.ret(x, 139, 1, 18, '#141824')
    ban.ret(x, 156, 16, 1, '#39425a')
for k in range(12, 308, 4):                                   # roletes nas bordas da esteira
    ban.ret(k, 133, 2, 1, ACO_ESCURO), ban.ret(k, 162, 2, 1, ACO_ESCURO)
# cabos e luzes de enfeite
for x in range(152, 166):
    ban.ret(x, 60 + int(3 * np.sin(x / 2)), 1, 2, '#2c2440')
    ban.ret(x, 90 + int(2 * np.cos(x / 2)), 1, 1, '#22283a')
for i, c in enumerate((VERDE, LARANJA, CIANO)):
    ban.ret(157, 30 + i * 6, 3, 3, CONTORNO), ban.ret(158, 31 + i * 6, 1, 1, c)
Image.fromarray(ampliar(ban.img)).save('assets/imagens/planetas/robo_bancada.png')
print('cientista, peças, tradutor e bancada ok')

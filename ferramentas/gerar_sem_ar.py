#!/usr/bin/env python3
"""gerar_sem_ar.py: desenha os gráficos do planeta sem ar.

Gera, a partir da pasta do projeto:
  assets/imagens/planetas/sem_ar_superficie.png  (cenário, 1280x720)
  assets/mascaras/sem_ar_mascara.png             (branco = piso, 1280x720)
  assets/imagens/sem_ar_itens.png                (itens, 32x32 cada)

Rodar:  python3 ferramentas/gerar_sem_ar.py
Precisa de numpy e Pillow. O jogo não precisa deste arquivo para rodar:
ele só é usado para refazer as imagens.

Tudo é desenhado numa grade de 640x360 e ampliado 2x, para os pixels
ficarem do mesmo tamanho dos outros cenários. As posições (módulo,
portão, respiradouros) precisam bater com as de src/tela_sem_ar.c,
que usa as mesmas medidas multiplicadas por 2.
"""

import numpy as np
from PIL import Image

W, H = 640, 360
ESCALA = 2

# ---------- posições (grade de 640x360) ----------
MODULO_X, MODULO_BASE = 84, 320          # centro e linha do chão do módulo de pouso
ESTACAO = (470, 44, 612, 128)            # x0, y0, x1, y1 da base das paredes
PORTAO = (530, 554)                      # vão na parede de baixo
ALTURA_PAREDE = 12
ESPESSURA_PAREDE = 6
CRATERA = (318, 196, 60, 34)             # cratera funda: cx, cy, rx, ry
RESPIRADOUROS = [(162, 176), (466, 240), (402, 160), (250, 262), (500, 184)]

# ---------- cores ----------
def cor(h):
    return np.array([int(h[i:i + 2], 16) for i in (1, 3, 5)], np.uint8)

ESPACO = cor('#15121c')
CHAO = [cor(c) for c in ('#a9688a', '#c07f9b', '#d597ad', '#e3adc0', '#f0c6d3')]
CHAO_SOMBRA = cor('#8b5479')
CREME = cor('#ffe6cf')
PENHASCO = [cor(c) for c in ('#8a5578', '#754768', '#63395a', '#4c2a48')]

ROCHA = dict(contorno=cor('#1f1830'), frente=[cor('#33284d'), cor('#3d3160'), cor('#2b2142')],
             topo=cor('#6a5a8e'), topo_escuro=cor('#5a4a7d'), topo_claro=cor('#7d6da6'),
             brilho=cor('#a99bd6'))
METAL = dict(contorno=cor('#1c1826'), frente=[cor('#5e566b')], emenda=cor('#4a4459'),
             topo=cor('#8a84a0'), topo_escuro=cor('#77718c'), topo_claro=cor('#a39db8'),
             brilho=cor('#c4bfd6'))

yy, xx = np.mgrid[0:H, 0:W]
BAYER = np.array([[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]]) / 16 + 1 / 32
bayer = np.tile(BAYER, (H // 4 + 1, W // 4 + 1))[:H, :W]


def ruido(escala, semente):
    """Ruído suave entre 0 e 1 (grade aleatória ampliada com interpolação)."""
    g = np.random.default_rng(semente).random((H // escala + 2, W // escala + 2))
    ys, xs = np.arange(H) / escala, np.arange(W) / escala
    y0, x0 = ys.astype(int), xs.astype(int)
    fy, fx = (ys - y0)[:, None], (xs - x0)[None, :]
    fy, fx = fy * fy * (3 - 2 * fy), fx * fx * (3 - 2 * fx)
    a, b = g[y0][:, x0], g[y0][:, x0 + 1]
    c, d = g[y0 + 1][:, x0], g[y0 + 1][:, x0 + 1]
    return a * (1 - fx) * (1 - fy) + b * fx * (1 - fy) + c * (1 - fx) * fy + d * fx * fy


def sorteio(semente):
    """Um número aleatório fixo por pixel (para manchas e listras)."""
    return np.random.default_rng(semente).random((H, W))


def mover(m, dx, dy):
    """Desloca uma máscara; o que sai da imagem some."""
    r = np.zeros_like(m)
    ys, yd = (slice(0, H - dy), slice(dy, H)) if dy >= 0 else (slice(-dy, H), slice(0, H + dy))
    xs, xd = (slice(0, W - dx), slice(dx, W)) if dx >= 0 else (slice(-dx, W), slice(0, W + dx))
    r[yd, xd] = m[ys, xs]
    return r


def elipse(cx, cy, rx, ry):
    return ((xx - cx) / rx) ** 2 + ((yy - cy) / ry) ** 2 <= 1


def pedra(cx, cy, rx, ry, semente):
    """Elipse de borda irregular."""
    r = np.random.default_rng(semente)
    ang = np.arctan2((yy - cy) / ry, (xx - cx) / rx)
    raio = 1 + 0.16 * np.sin(3 * ang + r.random() * 6.3) + 0.09 * np.sin(5 * ang + r.random() * 6.3)
    return ((xx - cx) / rx) ** 2 + ((yy - cy) / ry) ** 2 <= raio ** 2


def cordilheira(pontos, rx, ry, semente):
    """Várias pedras encostadas ao longo de uma linha."""
    m = np.zeros((H, W), bool)
    r = np.random.default_rng(semente)
    for (x0, y0), (x1, y1) in zip(pontos, pontos[1:]):
        passos = max(2, int(np.hypot(x1 - x0, y1 - y0) / 9))
        for i in range(passos + 1):
            t = i / passos
            m |= pedra(x0 + (x1 - x0) * t + r.integers(-2, 3), y0 + (y1 - y0) * t + r.integers(-2, 3),
                       rx * (0.8 + 0.5 * r.random()), ry * (0.8 + 0.5 * r.random()), int(r.integers(1 << 30)))
    return m


# ---------- 1. terreno ----------
borda = np.minimum.reduce([xx, W - 1 - xx, yy - 4, H - 1 - yy - 2]).astype(float)
# O chão cobre o mapa inteiro: quem anda num planeta não vê a beirada
# dele. O que fecha o mapa é um paredão de pedra em toda a volta.
terra = np.ones((H, W), bool)
paredao = borda + (ruido(48, 1) - 0.5) * 22 + (ruido(16, 2) - 0.5) * 8 <= 9

img = np.zeros((H, W, 3), np.uint8)
img[:] = ESPACO
solido = ~terra                     # onde não se pode andar

# estrelas e pedras soltas no espaço
r = np.random.default_rng(11)
for _ in range(260):
    x, y = int(r.integers(W)), int(r.integers(H))
    if not terra[y, x]:
        img[y, x] = cor('#ece8f0') if r.random() > 0.3 else cor('#9b8ce0')

# chão: tons por ruído, com pontilhado nas passagens de um tom a outro
n = 0.6 * ruido(56, 3) + 0.3 * ruido(20, 4) + 0.1 * ruido(8, 5)
tom = np.digitize(n + (bayer - 0.5) * 0.07, [0.36, 0.47, 0.60, 0.70])
chao = np.array(CHAO)[tom]
s1, s2, s3 = sorteio(21), sorteio(22), sorteio(23)
chao[s1 > 0.985] = CHAO[1]                                   # grãos escuros
chao[s1 < 0.012] = CHAO[4]                                   # grãos claros
risco = (s2 > 0.992) & (yy % 2 == 0)                         # riscos de poeira
for k in range(4):
    chao[mover(risco, k, 0)] = CHAO[4] if k < 3 else CREME
pedrinha = s3 > 0.9965
chao[pedrinha] = CHAO_SOMBRA
chao[mover(pedrinha, 1, 0)] = CHAO_SOMBRA
chao[mover(pedrinha, 0, -1)] = CHAO[4]
img[terra] = chao[terra]

# penhasco na beirada de baixo e contorno claro na de cima
for k in range(1, 12):
    face = mover(terra, 0, k) & ~terra
    for kk in range(1, k):
        face &= ~mover(terra, 0, kk)
    listra = (xx + (sorteio(31)[0] * 5).astype(int)[None, :]) % 5 == 0
    c = np.where(listra[..., None], PENHASCO[3], PENHASCO[min(3, k // 4)])
    some = bayer > (12 - k) / 5 if k > 7 else np.zeros((H, W), bool)
    face &= ~some
    img[face] = c[face]
beira_cima = terra & ~mover(terra, 0, 1)
img[beira_cima] = CHAO[4]
beira_baixo = terra & ~mover(terra, 0, -1)
img[beira_baixo] = PENHASCO[0]
beira_lado = terra & (~mover(terra, 1, 0) | ~mover(terra, -1, 0))
img[beira_lado & ~beira_baixo] = CHAO[4]


piso_metal = np.zeros((H, W), bool)      # preenchido quando a estação é desenhada


def sombrear(m):
    """Escurece o chão onde a máscara cai (sombra pontilhada)."""
    alvo = m & terra & ~solido
    img[alvo & ~piso_metal & (bayer < 0.8)] = CHAO_SOMBRA
    img[alvo & ~piso_metal & (bayer >= 0.8)] = CHAO[0]
    img[alvo & piso_metal] = cor('#3a3550')


def cratera_rasa(cx, cy, rx):
    """Cratera de enfeite: dá para andar por cima."""
    ry = rx * 0.5
    dentro = elipse(cx, cy, rx, ry) & terra & ~solido
    anel = elipse(cx, cy, rx + 1.5, ry + 1.2) & ~elipse(cx, cy, rx, ry) & terra & ~solido
    img[dentro] = CHAO[1]
    img[dentro & (bayer < 0.35)] = CHAO[0]
    img[dentro & elipse(cx, cy - ry * 0.35, rx * 0.95, ry * 0.8) & ~elipse(cx, cy + 1, rx * 0.9, ry * 0.75)] = CHAO_SOMBRA
    img[anel & (yy > cy)] = CHAO[4]
    img[anel & (yy <= cy)] = CHAO[3]
    img[dentro & elipse(cx, cy + ry * 0.45, rx * 0.5, ry * 0.25)] = CHAO[2]


def bloco(m, altura, p, semente, pedra_natural=True):
    """Desenha um obstáculo com volume: topo, frente e sombra.

    m é a base no chão. O topo é a base deslocada `altura` para cima e a
    frente é o que fica entre os dois. Tudo o que foi desenhado bloqueia.
    """
    global solido
    topo = mover(m, 0, -altura)
    cheio = np.zeros_like(m)
    for k in range(altura + 1):
        cheio |= mover(m, 0, -k)
    frente = cheio & ~topo
    sombrear(mover(m, altura // 2 + 2, 1) | mover(m, 2, 2))

    s = sorteio(semente)
    coluna = (s[0] * len(p['frente']) * 3).astype(int)[None, :] + xx * 0
    if pedra_natural:
        f = np.array(p['frente'])[coluna % len(p['frente'])]
        f[(xx + coluna) % 4 == 0] = p['frente'][-1]
    else:
        f = np.zeros((H, W, 3), np.uint8)
        f[:] = p['frente'][0]
        f[xx % 12 == 0] = p['emenda']
    img[frente] = f[frente]
    img[frente & mover(topo, 0, 1)] = p['contorno']            # sombra logo abaixo do topo

    t = np.zeros((H, W, 3), np.uint8)
    t[:] = p['topo']
    manchas = ruido(6, semente + 1)
    t[manchas > 0.62] = p['topo_claro']
    t[manchas < 0.36] = p['topo_escuro']
    if pedra_natural:
        t[s > 0.965] = CHAO[3]                                  # poeira rosa pousada na pedra
    img[topo] = t[topo]
    img[topo & (~mover(topo, 0, 1) | ~mover(topo, 1, 0))] = p['brilho']
    img[topo & ~mover(topo, 0, -1) & mover(cheio, 0, -1)] = p['topo_claro']
    img[cheio & (~mover(cheio, 0, -1) | ~mover(cheio, -1, 0))] = p['contorno']
    solido |= cheio


# crateras de enfeite (antes dos obstáculos, que passam por cima)
r = np.random.default_rng(5)
for cx, cy, rx in [(120, 250, 16), (232, 96, 13), (392, 64, 11), (212, 318, 12), (540, 188, 10),
                   (420, 300, 9), (60, 118, 9), (300, 44, 8), (584, 262, 8), (146, 44, 7)]:
    cratera_rasa(cx, cy, rx)

# respiradouros: mancha amarelada e rachadura no chão
for vx, vy in RESPIRADOUROS:
    mancha = elipse(vx, vy, 11, 5.5) & terra
    img[mancha & (bayer < 0.45)] = cor('#e9d9a6')
    img[elipse(vx, vy, 7, 3.5) & (bayer < 0.7)] = cor('#d8c486')
    for dx, dy in [(-7, 1), (-6, 0), (-5, 0), (-4, -1), (-3, -1), (-2, 0), (2, 0), (3, 1), (4, 1), (5, 0), (6, -1), (7, -1)]:
        img[vy + dy, vx + dx] = cor('#4c2a48')
    img[vy - 1:vy + 2, vx - 2:vx + 3] = cor('#2b1a30')
    img[vy, vx - 3:vx + 4] = cor('#2b1a30')
    img[vy - 1, vx - 1:vx + 2] = cor('#150d18')

# ---------- 2. cratera funda ----------
cx, cy, rx, ry = CRATERA
fundo_cratera = pedra(cx, cy, rx, ry, 77)
aro = pedra(cx, cy, rx + 5, ry + 4, 77) & ~fundo_cratera
sombrear(mover(aro, 3, 2))
img[aro] = CHAO[3]
img[aro & (yy < cy) & (bayer < 0.5)] = CHAO[2]
img[aro & ~mover(aro | fundo_cratera, 0, 1)] = CHAO[4]
img[aro & ~mover(aro | fundo_cratera, 0, -1)] = PENHASCO[0]
for i, (escala, c) in enumerate([(1.0, '#7d4a6c'), (0.86, '#63395a'), (0.7, '#4c2a48'), (0.52, '#36203c'), (0.32, '#241733')]):
    degrau = pedra(cx + i, cy + i * 2.2, rx * escala, ry * escala, 77) & fundo_cratera
    img[degrau] = cor(c)
img[fundo_cratera & ~mover(fundo_cratera, 0, 1)] = cor('#4c2a48')        # parede de cima, na sombra
img[fundo_cratera & ~mover(fundo_cratera, 0, -1)] = CHAO[1]             # parede de baixo, iluminada
solido |= fundo_cratera | aro

# ---------- 3. pedras e cordilheiras ----------
# (base no chão, altura). As cordilheiras separam os cantos do mapa.
obstaculos = [
    (cordilheira([(18, 176), (60, 180), (104, 174), (128, 178)], 12, 7, 101), 11),     # oeste: fecha o canto da placa
    (cordilheira([(196, 22), (200, 60), (194, 100), (198, 142)], 10, 8, 102), 12),     # norte: parede do canto da placa
    (cordilheira([(440, 262), (444, 300), (438, 334), (442, 352)], 10, 8, 103), 12),   # sudeste: parede do canto da célula
    (cordilheira([(496, 226), (540, 222), (584, 228), (624, 224)], 12, 7, 104), 11),   # sudeste: teto do canto da célula
    (pedra(262, 152, 12, 7, 201), 9), (pedra(246, 228, 9, 6, 202), 7), (pedra(380, 262, 13, 7, 203), 10),
    (pedra(300, 300, 10, 6, 204), 8), (pedra(352, 108, 9, 6, 205), 7), (pedra(424, 96, 11, 7, 206), 9),
    (pedra(150, 300, 12, 7, 207), 9), (pedra(40, 232, 9, 6, 208), 7), (pedra(96, 96, 8, 5, 209), 6),
    (pedra(532, 296, 10, 6, 210), 8), (pedra(272, 60, 8, 5, 211), 6), (pedra(402, 204, 8, 5, 212), 6),
    (pedra(180, 236, 7, 4, 213), 5), (pedra(332, 318, 8, 5, 214), 6), (pedra(600, 300, 7, 5, 215), 6),
]
bloco(paredao, 10, ROCHA, 290)          # primeiro o paredão, que fica por baixo do resto
for i, (m, altura) in sorted(enumerate(obstaculos), key=lambda o: np.nonzero(o[1][0])[0].max()):
    bloco(m & terra, altura, ROCHA, 300 + i)

# ---------- 4. estação abandonada ----------
ex0, ey0, ex1, ey1 = ESTACAO
E = ESPESSURA_PAREDE
dentro_estacao = (xx >= ex0) & (xx < ex1) & (yy >= ey0) & (yy < ey1)
solido &= ~dentro_estacao

# piso de placas de metal por dentro, e uma calçada em frente ao portão
piso = np.zeros((H, W, 3), np.uint8)
piso[:] = cor('#4a4560')
piso[(xx % 12 == 0) | (yy % 12 == 8)] = cor('#3a3550')
piso[((xx % 12 == 1) | (yy % 12 == 9)) & (bayer < 0.5)] = cor('#5b5672')
img[dentro_estacao] = piso[dentro_estacao]
calcada = (xx >= PORTAO[0] - 10) & (xx < PORTAO[1] + 10) & (yy >= ey1) & (yy < ey1 + 22)
placas = np.zeros((H, W, 3), np.uint8)
placas[:] = cor('#8a84a0')
placas[(xx % 10 == 0) | (yy % 10 == 4)] = cor('#6f6985')
poeira = ruido(10, 41) > 0.56
img[calcada & ~poeira] = placas[calcada & ~poeira]
entrou = dentro_estacao & (yy > ey1 - 34) & (ruido(9, 42) > 0.6 + (ey1 - yy) / 60)   # poeira que entrou pela porta
img[entrou] = CHAO[2]
# marca no chão onde fica o pedaço do mapa
mx, my = 590, 92
for raio, c in [(9, '#68d8cd'), (6, '#3a8a88')]:
    anel_m = elipse(mx, my, raio, raio * 0.5) & ~elipse(mx, my, raio - 1.2, raio * 0.5 - 0.8)
    img[anel_m] = cor(c)

piso_metal |= dentro_estacao
paredes = dentro_estacao & ~((xx >= ex0 + E) & (xx < ex1 - E) & (yy >= ey0 + E) & (yy < ey1 - E))
vao = (xx >= PORTAO[0]) & (xx < PORTAO[1]) & (yy >= ey1 - E) & (yy < ey1)
bloco(paredes & ~vao, ALTURA_PAREDE, METAL, 400, pedra_natural=False)

# consoles apagados por dentro, na frente da parede de cima
for x0 in (486, 506, 546, 566):
    y0 = ey0 + E - ALTURA_PAREDE + 2
    img[y0:y0 + 9, x0:x0 + 14] = cor('#2b2142')
    img[y0 + 1:y0 + 6, x0 + 1:x0 + 13] = cor('#1f4458')
    img[y0 + 2, x0 + 2:x0 + 8] = cor('#3a8a88')
    img[y0 + 4, x0 + 2:x0 + 11:2] = cor('#3a6a88')
    img[y0 + 7, x0 + 2:x0 + 12:3] = cor('#8a84a0')
# faixas de aviso dos dois lados do portão, na frente da parede de baixo
frente_sul = (yy >= ey1 - ALTURA_PAREDE + 1) & (yy < ey1 - 1)
for lado in (range(PORTAO[0] - 9, PORTAO[0] - 1), range(PORTAO[1] + 1, PORTAO[1] + 9)):
    for x in lado:
        col = frente_sul[:, x] & ((np.arange(H) + x) % 6 < 3)
        img[col, x] = cor('#e8c86a')
        img[frente_sul[:, x] & ~col, x] = cor('#2b2142')
# soleira do portão
img[vao] = cor('#3a3550')
img[vao & ((xx + yy) % 6 < 3)] = cor('#8f7632')
# caixa do terminal, à direita do portão (a tela é desenhada pelo jogo)
tx = PORTAO[1] + 12
img[ey1 - 13:ey1 - 1, tx:tx + 12] = cor('#2b2142')
img[ey1 - 12:ey1 - 2, tx + 1:tx + 11] = cor('#4a4560')
img[ey1 - 11:ey1 - 5, tx + 2:tx + 10] = cor('#1c1826')
img[ey1 - 4, tx + 2:tx + 10:2] = cor('#8a84a0')
# luzes apagadas e manchas de desgaste nas paredes
r = np.random.default_rng(9)
for _ in range(70):
    x, y = int(r.integers(ex0, ex1)), int(r.integers(ey0 - ALTURA_PAREDE, ey1))
    if solido[y, x] and paredes[min(H - 1, y + ALTURA_PAREDE // 2), x]:
        img[y, x] = cor('#3a3550')
# montes de poeira encostados na base das paredes de fora
for x in range(ex0 - 2, ex1 + 2, 3):
    alt = int(r.integers(0, 3))
    if alt and not (PORTAO[0] - 12 < x < PORTAO[1] + 26):
        img[ey1:ey1 + alt, x:x + 3] = CHAO[3]

# torre de antena tombada e caixas, do lado de fora
caixas = np.zeros((H, W), bool)
caixas[146:153, 478:489] = True
caixas[150:156, 492:500] = True
bloco(caixas, 7, METAL, 410, pedra_natural=False)
img[139:141, 479:488] = cor('#e8c86a')
for i in range(34):                                             # mastro caído
    x, y = 432 + i, 150 - i // 3
    if not solido[y, x]:
        img[y, x] = cor('#5e566b')
        img[y - 1, x] = cor('#a39db8') if i % 6 else cor('#2b2142')
for x in range(600, 640):                                        # cabo solto no chão
    y = 150 + int(4 * np.sin(x / 7))
    if terra[y, x] and not solido[y, x]:
        img[y, x] = cor('#3a3550')

# ---------- 5. módulo de pouso ----------
class Tela:
    """Tela pequena transparente, para desenhar uma figura e contornar."""

    def __init__(self, w, h):
        self.w, self.h = w, h
        self.c = np.zeros((h, w, 4), np.uint8)

    def ret(self, x, y, w, h, c):
        self.c[max(0, y):y + h, max(0, x):x + w, :3] = cor(c)
        self.c[max(0, y):y + h, max(0, x):x + w, 3] = 255

    def px(self, x, y, c):
        self.ret(x, y, 1, 1, c)

    def poligono(self, pontos, c):
        ys, xs = np.mgrid[0:self.h, 0:self.w]
        dentro = np.zeros((self.h, self.w), bool)
        j = len(pontos) - 1
        for i in range(len(pontos)):
            (xi, yi), (xj, yj) = pontos[i], pontos[j]
            corta = ((yi > ys + 0.5) != (yj > ys + 0.5)) & (xs + 0.5 < (xj - xi) * (ys + 0.5 - yi) / (yj - yi + 1e-9) + xi)
            dentro ^= corta
            j = i
        self.c[dentro, :3] = cor(c)
        self.c[dentro, 3] = 255

    def oval(self, cx, cy, rx, ry, c):
        ys, xs = np.mgrid[0:self.h, 0:self.w]
        dentro = ((xs - cx) / rx) ** 2 + ((ys - cy) / ry) ** 2 <= 1
        self.c[dentro, :3] = cor(c)
        self.c[dentro, 3] = 255

    def linha(self, x0, y0, x1, y1, c, grosso=1):
        passos = max(abs(x1 - x0), abs(y1 - y0), 1)
        for i in range(passos + 1):
            self.ret(round(x0 + (x1 - x0) * i / passos), round(y0 + (y1 - y0) * i / passos), grosso, 1, c)

    def contornar(self, c):
        a = self.c[..., 3] > 0
        viz = np.zeros_like(a)
        viz[1:] |= a[:-1]
        viz[:-1] |= a[1:]
        viz[:, 1:] |= a[:, :-1]
        viz[:, :-1] |= a[:, 1:]
        borda = viz & ~a
        self.c[borda, :3] = cor(c)
        self.c[borda, 3] = 255


LILAS, LILAS_CLARO, LILAS_SOMBRA, ROXO = '#b7a7f2', '#d6cbf8', '#8e7bd9', '#5e4fa8'
LARANJA, LARANJA_ESCURO = '#f2a541', '#c97a2a'
ACO, ACO_CLARO, ACO_ESCURO = '#8a84a0', '#c4bfd6', '#5e566b'
OURO, OURO_CLARO, OURO_ESCURO = '#c9a94e', '#f2efa6', '#8f7632'
ESCURO = '#2b2142'

m = Tela(72, 74)
# pernas de trás e sapatas
m.linha(23, 52, 15, 61, '#4a3f86', 2)
m.linha(49, 52, 57, 61, '#4a3f86', 2)
m.ret(10, 61, 8, 2, '#3d3160')
m.ret(54, 61, 8, 2, '#3d3160')
# bocal do motor
m.poligono([(30, 54), (42, 54), (47, 64), (25, 64)], ESCURO)
m.ret(27, 63, 18, 1, ACO_ESCURO)
m.linha(30, 55, 27, 62, ACO, 1)
m.ret(32, 58, 8, 1, '#4a3f86')
# estágio de descida, forrado de folha dourada
m.poligono([(19, 43), (53, 43), (57, 47), (57, 51), (53, 55), (19, 55), (15, 51), (15, 47)], OURO)
r = np.random.default_rng(3)
for _ in range(60):
    x, y = int(r.integers(16, 56)), int(r.integers(44, 55))
    if m.c[y, x, 3]:
        m.px(x, y, OURO_CLARO if r.random() > 0.45 else OURO_ESCURO)
m.ret(19, 43, 34, 1, OURO_CLARO)
m.ret(19, 54, 34, 1, OURO_ESCURO)
m.ret(53, 47, 4, 5, OURO_ESCURO)
for x in (24, 36, 48):                                       # luzes de posição (o jogo acende)
    m.ret(x, 50, 2, 2, '#7a5220')
# pernas da frente
for x0, x1, pad in ((18, 8, 3), (53, 63, 58)):
    m.linha(x0, 49, x1, 66, LILAS_SOMBRA, 2)
    m.linha(x0, 49, x1, 65, LILAS_CLARO, 1)
    m.linha(x0 + (3 if x0 < 36 else -3), 55, x1 + (2 if x0 < 36 else -1), 63, ROXO, 1)
    m.ret(pad, 66, 11, 3, ACO_ESCURO)
    m.ret(pad, 66, 11, 1, ACO_CLARO)
# cabine
m.poligono([(20, 17), (52, 17), (57, 23), (57, 39), (52, 44), (20, 44), (15, 39), (15, 23)], LILAS)
m.poligono([(15, 23), (20, 17), (20, 44), (15, 39)], LILAS_CLARO)
m.poligono([(52, 17), (57, 23), (57, 39), (52, 44)], LILAS_SOMBRA)
m.ret(20, 17, 32, 2, LILAS_CLARO)
m.ret(20, 41, 32, 3, LILAS_SOMBRA)
m.ret(40, 19, 1, 22, LILAS_SOMBRA)                           # emenda das chapas
for x, y in ((17, 26), (17, 36), (55, 26), (55, 36), (38, 21), (22, 39)):
    m.px(x, y, ROXO)                                         # rebites
# faixa laranja (a mesma cor do traje)
m.ret(15, 37, 42, 2, LARANJA)
m.ret(15, 39, 42, 1, LARANJA_ESCURO)
# janela grande, com reflexo
m.ret(20, 21, 19, 14, ESCURO)
m.ret(21, 22, 17, 12, '#2e5c7a')
m.poligono([(21, 22), (33, 22), (21, 31)], '#3f8fa0')
m.poligono([(30, 34), (38, 26), (38, 34)], '#1f4458')
m.linha(23, 29, 29, 23, '#b9f5de', 1)
m.linha(25, 31, 33, 23, '#7fe3c9', 1)
m.px(20, 21, LILAS)
m.px(38, 21, LILAS)
m.px(20, 34, LILAS)
m.px(38, 34, LILAS)
# escotilha, com vigia e puxador
m.ret(42, 21, 12, 21, ROXO)
m.ret(43, 22, 10, 19, '#c7bcf6')
m.ret(43, 37, 10, 2, LARANJA)
m.ret(45, 24, 5, 5, ESCURO)
m.ret(46, 25, 3, 3, '#4fb6b0')
m.px(46, 25, '#b9f5de')
m.ret(51, 30, 1, 4, LARANJA_ESCURO)
m.ret(47, 33, 2, 2, '#e8c86a')                               # emblema
# escada da escotilha até o chão
for y in range(44, 70):
    m.px(44, y, ACO)
    m.px(51, y, ACO_ESCURO)
for y in range(46, 70, 4):
    m.ret(45, y, 6, 1, ACO_CLARO)
# teto e tampa de acoplamento
m.poligono([(24, 12), (48, 12), (52, 17), (20, 17)], LILAS_CLARO)
m.poligono([(44, 12), (48, 12), (52, 17), (46, 17)], LILAS)
m.ret(30, 8, 12, 4, ACO)
m.ret(30, 8, 12, 1, ACO_CLARO)
m.ret(40, 9, 2, 3, ACO_ESCURO)
# antena com a luz de sinal (apagada; o jogo faz piscar) e o prato
m.ret(25, 3, 1, 9, ACO_ESCURO)
m.ret(24, 0, 3, 3, '#7a2a20')
m.linha(45, 4, 51, 4, ACO_CLARO, 1)
m.px(44, 3, ACO_CLARO)
m.px(52, 3, ACO_CLARO)
m.px(43, 2, ACO)
m.px(53, 2, ACO)
m.ret(48, 5, 1, 3, ACO_ESCURO)
m.px(48, 1, LARANJA)
# tanques laterais
for cxm in (11, 61):
    m.oval(cxm, 32, 4.2, 6.5, ACO)
    m.oval(cxm - 1, 30, 2, 3.5, ACO_CLARO)
    m.ret(cxm - 4, 33, 9, 1, ACO_ESCURO)
    m.ret(cxm + 2, 34, 2, 4, ACO_ESCURO)
# propulsores de manobra
for x in (12, 57):
    m.ret(x, 20, 4, 3, ACO_ESCURO)
    m.px(x + (0 if x < 36 else 3), 19, ESCURO)
m.contornar('#241a3a')

# sombra e marca de queimado no chão, depois o módulo por cima
ox, oy = MODULO_X - 36, MODULO_BASE - 69
sombrear(elipse(MODULO_X + 5, MODULO_BASE - 1, 36, 8))
queimado = elipse(MODULO_X, MODULO_BASE - 4, 14, 4)
img[queimado & (bayer < 0.6)] = cor('#63395a')
alfa = m.c[..., 3] > 0
img[oy:oy + m.h, ox:ox + m.w][alfa] = m.c[..., :3][alfa]
# bloqueia da cabine para baixo, de ponta a ponta em cada linha
for y in range(17, m.h):
    xs = np.nonzero(alfa[y])[0]
    if len(xs):
        solido[oy + y, ox + xs.min():ox + xs.max() + 1] = True

# ---------- 6. salvar cenário e máscara ----------
def ampliar(a):
    return np.repeat(np.repeat(a, ESCALA, 0), ESCALA, 1)


Image.fromarray(ampliar(img)).save('assets/imagens/planetas/sem_ar_superficie.png')
mascara = np.where(solido[..., None], 0, 255).astype(np.uint8).repeat(3, 2)
Image.fromarray(ampliar(mascara)).save('assets/mascaras/sem_ar_mascara.png')

# ---------- 7. itens ----------
# Cada item é um desenho de 16x16 feito com letras; cada letra é uma cor.
PALETA = {
    'T': '#4fb6b0', 't': '#3a8a88', 'h': '#b9f5de', 'v': '#3a6a88', 'W': '#ecfff8',
    'C': '#f2efa6', 'c': '#c9c27c', 'L': '#8e7bd9', 'O': '#f2a541', 'o': '#c97a2a',
    'K': '#4a4560', 'k': '#2b2142', 'Y': '#fff6a8', 'y': '#e8c86a',
    'S': '#8a84a0', 's': '#5e566b', 'G': '#c4bfd6',
    'R': '#8e7bd9', 'r': '#5e4fa8', 'B': '#d6cbf8', 'w': '#ffffff',
    'M': '#fbf9cf', 'm': '#d9d49a', 'P': '#ee9a8a', 'E': '#68d8cd',
}
ITENS = [
    # 0: cilindro de oxigênio
    ["................", "......vv........", ".....vvvv.......", "......vv........",
     ".....hTTt.......", "....hTTTTt......", "....hTTTTt......", "....hWWWWt......",
     "....hWEEWt......", "....hWWWWt......", "....hTTTTt......", "....hTTTTt......",
     "....hTTTTt......", ".....tttt.......", "................", "................"],
    # 1: placa de acesso
    ["................", "................", "................", "...CCCCCCCCCC...",
     "...CLLLLLCCOC...", "...CCCCCCCCCC...", "...CLLLCCyyyC...", "...CCCCCCyYyC...",
     "...CLLLLCyyyC...", "...cccccccccc...", "................", "................",
     "................", "................", "................", "................"],
    # 2: célula de energia
    ["................", "......SS........", ".....KKKK.......", "....KKKKKK......",
     "....KyYYyK......", "....KYYYYK......", "....KYYwYK......", "....KYwYYK......",
     "....KYYwYK......", "....KYwYYK......", "....KYYYYK......", "....KyYYyK......",
     "....KKKKKK......", ".....kkkk.......", "................", "................"],
    # 3: sucata
    ["................", "................", "................", ".......ss.......",
     "......sSSs......", "...GG.sSSs.s....", "..GSSSSSSSSSs...", "..SSSsSSSSsSs...",
     "..SSssSSSssSs...", "...ssSSSSSss....", ".....ssss..s....", "................",
     "................", "................", "................", "................"],
    # 4: rocha brilhante
    ["................", "................", ".......w........", "......BB........",
     ".....BBRr.......", "....BBRRr..B....", "....BRRRrr.Rr...", "...BBRRRrrBRr...",
     "...BRRRRrrRRr...", "...BRRRrrrRrr...", "....rrrrrrrr....", "................",
     "................", "................", "................", "................"],
    # 5: pedaço do mapa
    ["................", "................", "...MMMMMMMM.....", "...MLLLMMMMM....",
     "...MMMLMMPMMM...", "...MMMLLMMMMM...", "...MEMMLMMmMM...", "...MMMMLLLMM....",
     "...MMmMMMLMM....", "....MMMMMMMM....", "....mmMmmMm.....", "................",
     "................", "................", "................", "................"],
]
folha = np.zeros((16, 16 * len(ITENS), 4), np.uint8)
for i, linhas in enumerate(ITENS):
    t = Tela(16, 16)
    for y, linha in enumerate(linhas):
        for x, letra in enumerate(linha):
            if letra != '.':
                t.px(x, y, PALETA[letra])
    t.contornar('#241a3a')
    folha[:, i * 16:(i + 1) * 16] = t.c
Image.fromarray(ampliar(folha)).save('assets/imagens/sem_ar_itens.png')

# ---------- 8. conferência: dá para chegar a todos os lugares? ----------
from collections import deque

livre = ~ampliar(solido)
A, L = livre.shape
soma = np.pad(livre.astype(int), ((1, 0), (1, 0))).cumsum(0).cumsum(1)


def cabe(x, y):
    """A caixa dos pés (16x8) cabe inteira no piso?"""
    if x - 8 < 0 or y - 8 < 0 or x + 8 > L or y > A:
        return False
    return soma[y, x + 8] - soma[y - 8, x + 8] - soma[y, x - 8] + soma[y - 8, x - 8] == 128


inicio = (216, 656)
dist = {inicio: 0}
fila = deque([inicio])
while fila:
    x, y = fila.popleft()
    for dx, dy in ((4, 0), (-4, 0), (0, 4), (0, -4)):
        p = (x + dx, y + dy)
        if p not in dist and cabe(*p):
            dist[p] = dist[(x, y)] + 4
            fila.append(p)
print('começo no piso:', cabe(*inicio))
for nome, (x, y) in {'placa': (116, 132), 'celula': (1180, 620), 'portao': (1084, 280),
                     'dentro': (1084, 216), 'mapa': (1180, 196)}.items():
    perto = min((abs(px - x) + abs(py - y), d) for (px, py), d in dist.items())
    print(f'{nome}: ponto alcançável mais próximo a {perto[0]} px, caminho de {perto[1]} px')

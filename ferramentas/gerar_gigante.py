#!/usr/bin/env python3
"""gerar_gigante.py: amplia o planalto do gigante.

O desenho original (assets/imagens/planetas/gigante_planalto.png, 640x360)
continua sendo o centro do mapa, sem nenhuma mudança. Este script constrói
o resto em volta dele, com as mesmas cores, e separa o rosto do gigante
numa camada de fundo que fica parada na tela enquanto o chão corre.

Gera, a partir da pasta do projeto:
  assets/imagens/planetas/gigante_ceu.png       (rosto e céu, 704x180)
  assets/imagens/planetas/gigante_terreno.png   (chão, 2560x360, com transparência)
  assets/mascaras/gigante_terreno_mascara.png   (branco = piso; verde = planta)
  assets/imagens/gigante_itens.png              (cristal e célula, 32x32 cada)

Rodar:  python3 ferramentas/gerar_gigante.py
Precisa de numpy, scipy e Pillow. O jogo não precisa deste arquivo para
rodar: ele só é usado para refazer as imagens.

A parte nova é desenhada numa grade de 1280x180 e ampliada 2x. As
posições dos cristais, da célula e do posto precisam bater com as de
src/tela_gigante.c. As plantas brilhantes não: cada pixel verde puro
da máscara é uma planta, e o jogo lê a máscara para saber onde estão.
"""

import numpy as np
from PIL import Image
from scipy import ndimage

from pixel_arte import cor, Tela, figura, BAYER

ORIGINAL = 'assets/imagens/planetas/gigante_planalto.png'
MASCARA_ORIGINAL = 'assets/mascaras/gigante_planalto_mascara.png'

LARGURA, ALTURA = 2560, 360        # mapa inteiro
W, H = LARGURA // 2, ALTURA // 2   # grade em que a parte nova é desenhada
CENTRO = 960                       # x do mapa onde começa o desenho original
FAIXA = 80                         # o original continua, espelhado, por esta largura de cada lado
MARGEM_CEU = 32                    # sobra do céu de cada lado, para ele deslizar um pouco

# ---------- posições (em pixels do mapa; as mesmas de tela_gigante.c) ----------
POUSO = (CENTRO + 83, 214)
POSTO = (CENTRO + 489, 248)
CRISTAIS = [(400, 196), (2010, 176), (2466, 232)]   # bosque, ninho, altar
CELULA = (150, 288)                                 # destroços, no extremo oeste

# Plantas do desenho original (as mesmas de antes), em pixels do original.
PLANTAS_ORIGINAIS = [(62, 178), (118, 200), (150, 236), (215, 258), (250, 230), (293, 213), (327, 250),
                     (357, 200), (387, 233), (445, 252), (483, 263), (317, 293), (340, 318), (550, 277)]

# ---------- cores (tiradas do desenho original) ----------
CHAO, CHAO_ESCURO, CHAO_CLARO = cor('#9888c0'), cor('#7e6eaa'), cor('#b0a0d0')
POEIRA = [cor('#92747c'), cor('#836f83'), cor('#8d8084'), cor('#7b6470')]
TOPO, TOPO_CLARO, TOPO_ESCURO, TOPO_BEIRA = cor('#6f5978'), cor('#796481'), cor('#605172'), cor('#836f83')
FRENTE = [cor(c) for c in ('#4b3d65', '#413559', '#3c3451', '#372e4c', '#322a46', '#2f2943')]
CONTORNO = cor('#2b273d')
FUNDO_POCO = [cor('#2e283f'), cor('#282439'), cor('#242334'), cor('#222033')]
MENTA, MENTA_ESCURA = cor('#a5dcc4'), cor('#5e9c8c')
CREME, CINZA = cor('#e9d9b3'), cor('#9e9f92')

yy, xx = np.mgrid[0:H, 0:W]
bayer = np.tile(BAYER, (H // 4 + 1, W // 4 + 1))[:H, :W]


def ruido(escala, semente, w=W, h=H):
    """Ruído suave entre 0 e 1 (grade aleatória ampliada com interpolação)."""
    g = np.random.default_rng(semente).random((h // escala + 2, w // escala + 2))
    ys, xs = np.arange(h) / escala, np.arange(w) / escala
    y0, x0 = ys.astype(int), xs.astype(int)
    fy, fx = (ys - y0)[:, None], (xs - x0)[None, :]
    fy, fx = fy * fy * (3 - 2 * fy), fx * fx * (3 - 2 * fx)
    a, b = g[y0][:, x0], g[y0][:, x0 + 1]
    c, d = g[y0 + 1][:, x0], g[y0 + 1][:, x0 + 1]
    return a * (1 - fx) * (1 - fy) + b * fx * (1 - fy) + c * (1 - fx) * fy + d * fx * fy


def sorteio(semente):
    return np.random.default_rng(semente).random((H, W))


def mover(m, dx, dy):
    """Desloca uma máscara; o que sai da imagem some."""
    r = np.zeros_like(m)
    h, w = m.shape
    ys, yd = (slice(0, h - dy), slice(dy, h)) if dy >= 0 else (slice(-dy, h), slice(0, h + dy))
    xs, xd = (slice(0, w - dx), slice(dx, w)) if dx >= 0 else (slice(-dx, w), slice(0, w + dx))
    r[yd, xd] = m[ys, xs]
    return r


def engordar(m, n):
    return ndimage.binary_dilation(m, iterations=n) if n > 0 else m


def elipse(cx, cy, rx, ry):
    return ((xx - cx) / rx) ** 2 + ((yy - cy) / ry) ** 2 <= 1


def mancha(cx, cy, rx, ry, semente):
    """Elipse de borda irregular."""
    r = np.random.default_rng(semente)
    ang = np.arctan2((yy - cy) / ry, (xx - cx) / rx)
    raio = 1 + 0.14 * np.sin(3 * ang + r.random() * 6.3) + 0.08 * np.sin(5 * ang + r.random() * 6.3)
    return ((xx - cx) / rx) ** 2 + ((yy - cy) / ry) ** 2 <= raio ** 2


def trilha(pontos, raio, semente=0):
    """Um caminho: discos encostados ao longo de uma linha quebrada."""
    m = np.zeros((H, W), bool)
    r = np.random.default_rng(semente)
    for (x0, y0), (x1, y1) in zip(pontos, pontos[1:]):
        passos = max(2, int(np.hypot(x1 - x0, y1 - y0) / 4))
        for i in range(passos + 1):
            t = i / passos
            rr = raio * (0.9 + 0.2 * r.random())
            m |= elipse(x0 + (x1 - x0) * t, y0 + (y1 - y0) * t, rr * 1.25, rr)
    return m


# =====================================================================
# 1. Por onde se anda (grade de 1280x180)
# =====================================================================
# O centro (x de 480 a 800) é o desenho original; as faixas de 440 a 480
# e de 800 a 840 são o original espelhado. Aqui só se planeja o resto.

LIMITE_OESTE = (CENTRO - FAIXA) // 2      # 440
LIMITE_LESTE = (CENTRO + 640 + FAIXA) // 2   # 840

# Linha do horizonte: acima dela não há chão, só o gigante.
horizonte = (68 + (ruido(36, 7)[0] - 0.5) * 9).astype(int)
terra = yy >= horizonte[None, :]

caminho = np.zeros((H, W), bool)

# --- oeste: clareira, bosque das plantas e destroços ---
caminho |= trilha([(446, 98), (410, 99), (372, 104)], 15, 1)          # entrada, vinda do centro
caminho |= mancha(350, 106, 30, 20, 2)                                # clareira
caminho |= trilha([(340, 100), (308, 98)], 13, 3)                     # passagem para o bosque
caminho |= mancha(222, 100, 88, 24, 4)                                # bosque
caminho |= trilha([(350, 116), (326, 142), (280, 154), (214, 152), (150, 150), (108, 146)], 10, 5)   # trilha do sul
caminho |= mancha(76, 142, 44, 20, 6)                                 # destroços
caminho |= trilha([(148, 112), (118, 124), (92, 134)], 9, 7)          # atalho do bosque aos destroços

# --- leste: clareira, ninho dos habitantes, pontes e altar ---
caminho |= trilha([(834, 148), (868, 146), (900, 136)], 13, 11)       # entrada
caminho |= mancha(914, 130, 26, 18, 12)                               # clareira
caminho |= trilha([(916, 122), (946, 108)], 12, 13)                   # subida para o ninho
caminho |= mancha(1004, 100, 64, 24, 14)                              # ninho
ponte_norte = trilha([(1058, 100), (1100, 102), (1150, 100), (1196, 104)], 13, 15)
ponte_sul = trilha([(930, 142), (980, 154), (1050, 156), (1120, 154), (1196, 144)], 9, 16)
caminho |= ponte_norte | ponte_sul
caminho |= mancha(1232, 122, 38, 34, 17)                              # altar
caminho &= terra

# O abismo entre o ninho e o altar: só as duas pontes atravessam.
abismo = (xx > 1066) & (xx < 1188) & (yy > 84) & ~engordar(caminho, 1)
abismo |= mancha(1128, 128, 52, 16, 18) & ~engordar(caminho, 1)

# Na parte nova, perto das faixas espelhadas, fora do caminho é sempre pedra.
perto_do_centro = ((xx > LIMITE_OESTE - 22) & (xx < LIMITE_OESTE)) | ((xx >= LIMITE_LESTE) & (xx < LIMITE_LESTE + 22))

# Altura máxima das pedras comuns: ao sul de todo caminho fica uma
# faixa sem pedra, senão a pedra (que sobe na tela) cobriria o caminho.
ALTURA_PEDRA = 16
sombra_do_caminho = np.zeros((H, W), bool)
for k in range(ALTURA_PEDRA + 2):
    sombra_do_caminho |= mover(engordar(caminho, 2), 0, k)

nr = 0.6 * ruido(22, 31) + 0.4 * ruido(9, 32)
np_ = 0.6 * ruido(26, 33) + 0.4 * ruido(11, 34)
crista = terra & (yy < horizonte[None, :] + 8)                         # a fileira de pedras do horizonte
beira = (xx < 9) | (xx > W - 10)
# Fora dos caminhos quase tudo é pedra ou poço; sobra um pouco de chão aberto.
quer_pedra = (nr > 0.27) | perto_do_centro | beira
rocha = terra & (crista | (quer_pedra & ~sombra_do_caminho & ~abismo))
rocha &= ~engordar(caminho, 2) | crista & ~caminho
poco = terra & ~rocha & ~engordar(caminho, 3) & (abismo | (quer_pedra & sombra_do_caminho) | ((np_ > 0.5) & (yy > 92)))
poco = ndimage.binary_opening(poco, iterations=2)
rocha &= ~poco
# Pedaços pequenos demais de pedra viram chão.
rotulos, n = ndimage.label(rocha)
tamanhos = ndimage.sum(rocha, rotulos, range(1, n + 1))
rocha &= ~np.isin(rotulos, 1 + np.nonzero(tamanhos < 14)[0])

# =====================================================================
# 2. Chão
# =====================================================================
img = np.zeros((H, W, 3), np.uint8)
alfa = terra.copy()          # onde o terreno cobre o céu
solido = ~terra | poco       # onde não se pode andar

s1, s2 = sorteio(21), sorteio(22)
chao = np.zeros((H, W, 3), np.uint8)
chao[:] = CHAO
n1 = ruido(12, 41)
s4 = sorteio(23)
chao[(n1 > 0.66) & (s4 < 0.10)] = CHAO_CLARO              # manchas claras e escuras, em pinceladas
chao[(n1 < 0.30) & (s4 < 0.12)] = CHAO_ESCURO
pincelada = (s1 > 0.988)
chao[pincelada | mover(pincelada, 1, 0)] = CHAO_ESCURO
pincelada = (s1 < 0.006)
chao[pincelada | mover(pincelada, 1, 0)] = CHAO_CLARO
# poeira castanha junto das pedras e dos poços
dist = ndimage.distance_transform_edt(~(rocha | poco))
chao[(dist < 3.5) & (s2 > 0.45)] = POEIRA[0]
chao[(dist < 2.2) & (s2 > 0.7)] = POEIRA[1]
chao[(dist >= 3.5) & (dist < 6) & (s2 > 0.9)] = POEIRA[2]
pedrinha = (s2 < 0.0035) & (dist > 3)
chao[pedrinha] = TOPO_ESCURO
chao[mover(pedrinha, 1, 0)] = FRENTE[1]
chao[mover(pedrinha, 0, -1)] = TOPO_CLARO
img[terra] = chao[terra]

# =====================================================================
# 3. Poços
# =====================================================================
fundo = np.zeros((H, W, 3), np.uint8)
fundo[:] = FUNDO_POCO[2]
fundo[(ruido(8, 51) > 0.6) & (bayer < 0.4)] = FUNDO_POCO[3]
fundo[s1 > 0.992] = FUNDO_POCO[0]
fundo[(s2 > 0.9975)] = MENTA_ESCURA                       # brilhos lá no fundo
img[poco] = fundo[poco]
coluna = (sorteio(52)[0] * 3).astype(int)[None, :] + xx * 0
PROFUNDIDADE = 13
for k in range(PROFUNDIDADE, 0, -1):                      # parede de cima do poço, vista de frente
    parede = poco & ~mover(poco, 0, k)
    tom = np.clip(coluna + (k * 4) // PROFUNDIDADE, 0, 5)
    c = np.array(FRENTE)[tom]
    c[(xx + coluna) % 4 == 0] = FRENTE[5]
    some = (bayer > (PROFUNDIDADE + 1 - k) / 4) if k > PROFUNDIDADE - 3 else np.zeros((H, W), bool)
    alvo = parede & ~some
    img[alvo] = c[alvo]
img[poco & ~mover(poco, 0, 1)] = CONTORNO                 # sombra logo abaixo da beirada
beirada_cima = terra & ~poco & mover(poco, 0, -1)
img[beirada_cima] = TOPO_BEIRA
beirada_baixo = terra & ~poco & mover(poco, 0, 1)
img[beirada_baixo] = CHAO_CLARO
img[poco & ~mover(poco, 1, 0) | poco & ~mover(poco, -1, 0)] = CONTORNO


# =====================================================================
# 4. Pedras
# =====================================================================
textura_topo = ruido(5, 61)
textura_po = ruido(7, 62)
s3 = sorteio(63)


def subir(m, k):
    r = np.zeros_like(m)
    if k == 0:
        return m.copy()
    r[:-k] = m[k:]
    return r


def pilar(base, altura, semente, com_po=None):
    """Desenha uma pedra com volume: topo, frente e sombra.

    base é a parte que encosta no chão. O topo é a base deslocada
    `altura` para cima e a frente é o que fica entre os dois. Tudo o
    que foi desenhado bloqueia a passagem.
    """
    ys, xs = np.nonzero(base)
    if len(ys) < 3:
        return
    r = np.random.default_rng(semente)
    y0, y1 = max(0, ys.min() - altura - 1), min(H, ys.max() + 3)
    x0, x1 = max(0, xs.min() - 1), min(W, xs.max() + 5)
    rec = (slice(y0, y1), slice(x0, x1))
    b = base[rec]
    topo = subir(b, altura)
    corpo = np.zeros_like(b)
    nivel = np.zeros(b.shape, int)
    for k in range(altura, -1, -1):
        camada = subir(b, k)
        corpo |= camada
        nivel[camada] = k
    frente = corpo & ~topo
    lx = xx[rec]

    # sombra no chão, à direita
    sombra = (mover(b, 2, 1) | mover(b, 3, 0)) & ~corpo & ~solido[rec] & terra[rec]
    pedaco = img[rec]
    pedaco[sombra & (bayer[rec] < 0.75)] = CHAO_ESCURO

    tom_coluna = (r.random(W) * 3).astype(int)[lx]
    tom = tom_coluna + (nivel * 3 < altura) + ((lx + int(r.integers(4))) % 3 == 0)
    c = np.array(FRENTE)[np.clip(tom, 0, 5)]
    pedaco[frente] = c[frente]
    pedaco[frente & mover(topo, 0, 1)] = FRENTE[5]                       # sombra logo abaixo do topo

    t = np.zeros(b.shape + (3,), np.uint8)
    t[:] = TOPO
    t[textura_topo[rec] > 0.6] = TOPO_CLARO
    t[textura_topo[rec] < 0.36] = TOPO_ESCURO
    t[s3[rec] > 0.93] = POEIRA[0]
    if com_po if com_po is not None else r.random() < 0.22:              # pó lilás pousado em cima
        po = textura_po[rec] > 0.5
        t[po] = CHAO
        t[po & ~mover(po, 0, 1)] = CHAO_ESCURO
        t[po & (s3[rec] < 0.1)] = CHAO_CLARO
    pedaco[topo] = t[topo]
    pedaco[topo & ~mover(topo, 0, -1)] = TOPO_BEIRA                      # quina da frente, mais clara
    pedaco[topo & ~mover(topo, 0, 1)] = cor('#53446d')                   # quina de trás
    pedaco[corpo & (~mover(corpo, 1, 0) | ~mover(corpo, 0, -1))] = CONTORNO
    solido[rec] |= corpo
    alfa[rec] |= corpo


# Divide a pedra em colunas: cada pixel fica com a semente mais próxima
# de uma grade torta. É isso que dá o jeito de pedras encostadas.
GX, GY = 11, 7
rg = np.random.default_rng(71)
nx, ny = W // GX + 3, H // GY + 3
sx = (np.arange(nx)[None, :] - 1 + 0.15 + 0.7 * rg.random((ny, nx))) * GX
sy = (np.arange(ny)[:, None] - 1 + 0.15 + 0.7 * rg.random((ny, nx))) * GY
melhor = np.full((H, W), 1e9)
celula = np.zeros((H, W), int)
for dy in (-1, 0, 1):
    for dx in (-1, 0, 1):
        cy, cx = yy // GY + 1 + dy, xx // GX + 1 + dx
        d = (xx - sx[cy, cx]) ** 2 + (1.6 * (yy - sy[cy, cx])) ** 2
        perto = d < melhor
        melhor[perto] = d[perto]
        celula[perto] = (cy * nx + cx)[perto]

dentro = ndimage.distance_transform_edt(rocha)
ruido_crista = ruido(30, 72)[0]
ordem = sorted(np.unique(celula[rocha]), key=lambda i: sy.flat[i])       # de trás para a frente
for i in ordem:
    base = rocha & (celula == i)
    if not base.any():
        continue
    ys, xs = np.nonzero(base)
    y_meio, x_meio = int(ys.mean()), int(xs.mean())
    if crista[y_meio, x_meio] or y_meio < horizonte[x_meio] + 9:
        # pedras do horizonte: altas, mas sem cobrir os olhos do gigante
        altura = int(min(10 + 14 * ruido_crista[x_meio] + rg.integers(0, 5), ys.min() - 47))
    else:
        # no meio de um maciço as alturas variam bastante: é o que deixa ver a frente das pedras de trás
        altura = int(min(ALTURA_PEDRA, (4 + 1.9 * min(dentro[y_meio, x_meio], 6)) * (0.4 + 0.6 * rg.random()) + rg.integers(0, 3)))
    pilar(base, max(2, altura), int(i))

# pedras soltas pelo chão, na beira dos caminhos
rs = np.random.default_rng(73)
livre_para_pedra = terra & ~solido & ~engordar(caminho, 1) & ~sombra_do_caminho
for _ in range(900):
    x, y = int(rs.integers(12, W - 12)), int(rs.integers(60, H - 4))
    if LIMITE_OESTE - 6 <= x < LIMITE_LESTE + 6:
        continue
    base = mancha(x, y, rs.integers(2, 5), rs.integers(1, 3), int(rs.integers(1 << 30)))
    if (base & ~livre_para_pedra).any():
        continue
    pilar(base, int(rs.integers(2, 5)), int(rs.integers(1 << 30)), com_po=False)

# =====================================================================
# 5. Enfeites desenhados à mão
# =====================================================================

def colar(tela, x, y, bloqueia=None):
    """Cola uma Tela no cenário. bloqueia = (x, y, w, h) relativo à tela."""
    for ty in range(tela.h):
        for tx in range(tela.w):
            if tela.c[ty, tx, 3] and 0 <= y + ty < H and 0 <= x + tx < W:
                img[y + ty, x + tx] = tela.c[ty, tx, :3]
                alfa[y + ty, x + tx] = True
    if bloqueia:
        bx, by, bw, bh = bloqueia
        solido[y + by:y + by + bh, x + bx:x + bx + bw] = True


def sombra_oval(cx, cy, rx, ry):
    m = elipse(cx, cy, rx, ry) & ~solido
    img[m & (bayer < 0.7)] = CHAO_ESCURO


def planta_enfeite(x, y, r):
    """Plantinha de luz, só de enfeite (fica onde ninguém pisa)."""
    if not (2 < x < W - 3 and 8 < y < H):
        return
    alto = r.random() < 0.25
    h = int(r.integers(7, 12)) if alto else int(r.integers(2, 5))
    for k in range(h):
        img[y - k, x] = MENTA_ESCURA
    if alto:
        img[y - h - 2:y - h + 1, x - 1:x + 2] = MENTA
        img[y - h - 2, x - 1] = cor('#d6f5e6')
    else:
        img[y - h, x] = MENTA
        if r.random() < 0.6:
            img[y - h + 1, x - 1] = MENTA
            img[y - 1, x + 1] = MENTA_ESCURA
            img[y - 2, x + 1] = MENTA
    alfa[y - h - 2:y + 1, x - 1:x + 2] |= True


# plantinhas nos topos das pedras e no fundo dos poços
r = np.random.default_rng(81)
topos = solido & ~poco & terra & (np.abs(img.astype(int) - TOPO.astype(int)).sum(2) < 40)
for _ in range(520):
    x, y = int(r.integers(4, W - 4)), int(r.integers(50, H - 2))
    if (LIMITE_OESTE <= x < LIMITE_LESTE):
        continue
    if topos[y, x] and topos[y - 1, x] and r.random() < 0.8:
        planta_enfeite(x, y, r)
    elif poco[y, x] and poco[y - 14:y + 1, x].all() and r.random() < 0.5:
        planta_enfeite(x, y, r)

# --- destroços no extremo oeste: uma sonda antiga, caída ---
DX, DY = CELULA[0] // 2, CELULA[1] // 2          # 75, 144
sombra_oval(DX - 20, DY - 4, 24, 6)
sonda = Tela(46, 30)
sonda.poligono([(4, 22), (10, 8), (30, 4), (40, 12), (38, 24), (12, 27)], '#e9d9b3')
sonda.poligono([(10, 8), (30, 4), (33, 9), (13, 13)], '#f4ead0')
sonda.poligono([(30, 4), (40, 12), (38, 24), (33, 21), (33, 9)], '#9e9f92')
sonda.ret(15, 15, 9, 7, '#2b273d')
sonda.ret(16, 16, 7, 5, '#3c5c6e')
sonda.px(17, 17, '#a5dcc4')
sonda.ret(12, 23, 22, 2, '#b0917e')
sonda.linha(26, 14, 31, 22, '#605172')
sonda.linha(8, 20, 13, 25, '#9e9f92')
sonda.linha(34, 3, 39, -3, '#9e9f92')                 # antena torta
sonda.poligono([(0, 26), (6, 22), (9, 27), (3, 29)], '#5e5c71')   # painel solto
sonda.px(21, 10, '#c2a183')
sonda.px(25, 9, '#c2a183')
for x_, y_ in ((12, 11), (19, 21), (29, 17), (36, 15)):
    sonda.px(x_, y_, '#836f83')                       # ferrugem
sonda.contornar('#2b273d')
colar(sonda, DX - 44, DY - 27, (2, 12, 40, 16))
caixote = Tela(13, 12)
caixote.ret(1, 3, 11, 8, '#5e5c71')
caixote.ret(1, 1, 11, 3, '#9e9f92')
caixote.ret(1, 6, 11, 1, '#37324b')
caixote.ret(5, 3, 1, 8, '#37324b')
caixote.contornar('#2b273d')
colar(caixote, DX + 12, DY - 22, (1, 5, 11, 7))
# marcas no chão onde fica a célula
for dx_, dy_ in ((-6, 3), (6, 3), (-7, -3), (7, -3)):
    img[DY + dy_, DX + dx_] = CREME

# --- pedestais dos três cristais: um anel de pedras no chão ---
for cx_, cy_ in CRISTAIS:
    px_, py_ = cx_ // 2, cy_ // 2
    anel = elipse(px_, py_, 9, 4.5) & ~elipse(px_, py_, 7, 3.2)
    img[anel & ~solido] = TOPO_CLARO
    img[anel & ~solido & (yy > py_)] = TOPO_BEIRA
    img[elipse(px_, py_, 7, 3.2) & ~solido & (bayer < 0.5)] = CHAO_CLARO
    for ang in range(0, 360, 45):
        qx = int(round(px_ + 11 * np.cos(np.radians(ang))))
        qy = int(round(py_ + 5.5 * np.sin(np.radians(ang))))
        if not solido[qy, qx]:
            img[qy, qx] = CREME

# --- altar: dois marcos com runas ao lado do terceiro cristal ---
AX, AY = CRISTAIS[2][0] // 2, CRISTAIS[2][1] // 2
marco = figura([
    '..aaaa..',
    '.abbbba.',
    '.bccccb.',
    '.bcrrcb.',
    '.bcr.cb.',
    '.bcrrcb.',
    '.bccccb.',
    '.bcrrcb.',
    '.bccccb.',
    '.bcrccb.',
    '.bcrccb.',
    '.bccccb.',
    '.bddddb.',
    '.dddddd.',
], {'a': '#836f83', 'b': '#3c3451', 'c': '#4b3d65', 'd': '#322a46', 'r': '#c8c4d6'}, '#2b273d')
for dx_ in (-20, 13):
    sombra_oval(AX + dx_ + 6, AY - 5, 6, 2.5)
    colar(marco, AX + dx_, AY - 20, (1, 9, 6, 5))

# --- ninho: tocas dos habitantes ---
for tx_, ty_ in ((972, 84), (1034, 86), (1052, 112), (958, 112)):
    if solido[ty_, tx_]:
        continue
    monte = elipse(tx_, ty_, 6, 3)
    img[monte & ~solido] = POEIRA[1]
    img[monte & ~solido & (yy < ty_)] = POEIRA[0]
    img[elipse(tx_, ty_ + 0.5, 3.2, 1.6) & ~solido] = FUNDO_POCO[2]
    img[ty_ - 1, tx_ - 3:tx_ + 4] = CONTORNO

# =====================================================================
# 6. Junta a parte nova com o desenho original
# =====================================================================

def ampliar(a):
    return np.repeat(np.repeat(a, 2, 0), 2, 1)


original = np.array(Image.open(ORIGINAL).convert('RGB'))
piso_original = np.array(Image.open(MASCARA_ORIGINAL).convert('RGB'))[..., 0] > 128

# Original com um pedaço espelhado de cada lado: 800x360.
largo = np.concatenate([original[:, FAIXA - 1::-1], original, original[:, :-FAIXA - 1:-1]], 1)
piso_largo = np.concatenate([piso_original[:, FAIXA - 1::-1], piso_original, piso_original[:, :-FAIXA - 1:-1]], 1)
X0 = CENTRO - FAIXA                                   # onde o pedaço largo começa no mapa

# Recorte de cima: acima desta linha o original fica só na camada do céu.
pontos_corte = [(-FAIXA, 96), (0, 96), (120, 96), (170, 112), (230, 128), (300, 138), (440, 138),
                (472, 100), (640, 94), (640 + FAIXA, 94)]
xs_corte = np.arange(-FAIXA, 640 + FAIXA)
corte = np.interp(xs_corte, [p[0] for p in pontos_corte], [p[1] for p in pontos_corte])
# A linha vira uma fileira de topos de pedra, para o recorte parecer natural.
rc = np.random.default_rng(91)
degraus = np.zeros(len(xs_corte))
i = 0
while i < len(xs_corte):
    largura = int(rc.integers(12, 28))
    sobe = int(rc.integers(0, 11))
    trecho = np.arange(min(largura, len(xs_corte) - i))
    canto = np.minimum(trecho, trecho[::-1])
    degraus[i:i + largura] = sobe - np.where(canto < 2, 3 - canto * 1.5, 0)
    i += largura
corte = (corte - degraus).astype(int)

final = ampliar(img)
final_alfa = ampliar(alfa)
piso = ampliar(~solido)

yf, xf = np.mgrid[0:ALTURA, 0:800]
usa_original = yf >= corte[None, :]
# Nas pontas das faixas espelhadas o original se mistura com a parte nova, em pontilhado.
MISTURA = 20
bayer_largo = np.tile(BAYER, (ALTURA // 4 + 1, 800 // 4 + 1))[:ALTURA, :800]
dist_ponta = np.minimum(xf, 799 - xf)
some = (dist_ponta < MISTURA) & (bayer_largo[(yf // 2) % 4, (xf // 2) % 4] > dist_ponta / MISTURA)
usa_original &= ~some
regiao = (slice(0, ALTURA), slice(X0, X0 + 800))
final[regiao][usa_original] = largo[usa_original]
final_alfa[regiao] = np.where(yf >= corte[None, :], True, False) & (usa_original | final_alfa[regiao])
# Dentro do pedaço original vale a máscara original; na mistura, as duas juntas.
piso[regiao] = np.where(dist_ponta < MISTURA, piso[regiao] | piso_largo, piso_largo) & (yf >= corte[None, :])
# quina clara no alto do recorte, como nos topos das pedras
quina = final_alfa & ~np.roll(final_alfa, 1, 0)
quina[:, :X0] = False
quina[:, X0 + 800:] = False
final[quina] = TOPO_ESCURO

# =====================================================================
# 7. Máscara: só o que se alcança a pé, e as plantas em verde
# =====================================================================
piso[ALTURA - 6:] = False                             # a barra de teclas cobre o fim da tela
piso[ALTURA - 6:, CENTRO:CENTRO + 640] = piso_original[ALTURA - 6:]
rotulos, n = ndimage.label(piso)
piso = rotulos == rotulos[POUSO[1], POUSO[0]]

# Onde os pés cabem: retângulo de 16x8 inteiro no piso (como no jogo).
cabe = ndimage.binary_erosion(piso, structure=np.ones((8, 16), bool), origin=(3, 0))


def pes_cabem(x, y):
    return 8 <= x < LARGURA - 8 and 8 <= y < ALTURA and piso[y - 7:y + 1, x - 8:x + 8].all()


lugares = dict(pouso=POUSO, posto=POSTO, celula=(CELULA[0], CELULA[1] + 14),
               cristal1=(CRISTAIS[0][0], CRISTAIS[0][1] + 14), cristal2=(CRISTAIS[1][0], CRISTAIS[1][1] + 14),
               cristal3=(CRISTAIS[2][0], CRISTAIS[2][1] + 14))
alcance, _ = ndimage.label(np.array([[pes_cabem(x, y) for x in range(0, LARGURA, 4)] for y in range(0, ALTURA, 4)]))
for nome, (x, y) in lugares.items():
    ok = alcance[y // 4, x // 4] != 0 and alcance[y // 4, x // 4] == alcance[POUSO[1] // 4, POUSO[0] // 4]
    print(f'{nome:9} {(x, y)}  {"alcancavel" if ok else "*** FORA DO CAMINHO ***"}')

# Plantas brilhantes da parte nova: sorteadas no piso, longe umas das
# outras o bastante para se passar entre elas, e longe dos lugares de uso.
rp = np.random.default_rng(101)
plantas = [(CENTRO + x, y) for x, y in PLANTAS_ORIGINAIS]
proibido = list(lugares.values()) + [CRISTAIS[0], CRISTAIS[1], CRISTAIS[2], CELULA]


def tenta_planta(x, y, afastamento):
    if not (pes_cabem(x, y) and pes_cabem(x - 10, y) and pes_cabem(x + 10, y)):
        return False
    if X0 - 10 <= x < X0 + 810:
        return False
    if any(np.hypot(x - px, y - py) < afastamento for px, py in plantas):
        return False
    if any(np.hypot(x - px, y - py) < 34 for px, py in proibido):
        return False
    plantas.append((x, y))
    return True


zonas = [  # x0, y0, x1, y1, tentativas, afastamento
    (270, 150, 620, 250, 900, 46),      # bosque: cheio
    (0, 150, 270, 346, 200, 78),        # destroços e trilha do sul
    (270, 250, 760, 346, 200, 74),
    (620, 150, 880, 250, 120, 80),      # clareira do oeste
    (1680, 150, 1880, 346, 120, 80),    # clareira do leste
    (1880, 150, 2130, 250, 160, 70),    # ninho
    (1860, 270, 2400, 346, 500, 62),    # ponte do sul: é desviar das plantas
    (2130, 170, 2400, 240, 100, 84),    # ponte do norte
    (2400, 170, 2560, 330, 160, 60),    # altar
]
for x0, y0, x1, y1, tentativas, afastamento in zonas:
    for _ in range(tentativas):
        tenta_planta(int(rp.integers(x0, x1)), int(rp.integers(y0, y1)), afastamento)
print(len(plantas), 'plantas')

mascara = np.zeros((ALTURA, LARGURA, 3), np.uint8)
mascara[piso] = 255
for x, y in plantas:
    mascara[y, x] = (0, 255, 0)

# =====================================================================
# 8. Camada do céu: o alto do desenho original, com sobra dos lados
# =====================================================================
ceu = np.pad(original[:180], ((0, 0), (MARGEM_CEU, MARGEM_CEU), (0, 0)), mode='reflect')

# =====================================================================
# 9. Itens: cristal e célula de combustível (32x32 cada)
# =====================================================================
cristal = figura([
    '......a.....',
    '.....aba....',
    '.....abb....',
    '....aabbc...',
    '....abbbc...',
    '..a.abbbcc..',
    '.aba.bbbcc..',
    '.abbabbbccd.',
    '.abbcbbbccd.',
    'aabbcbbcccd.',
    'abbbccbccdd.',
    'abbbccccddd.',
    '.bbcccccdd..',
    '..cccccdd...',
], {'a': '#f4fff8', 'b': '#a5dcc4', 'c': '#5fae9a', 'd': '#3f8a78'}, '#1f3a3a')
celula_t = figura([
    '...aaaa...',
    '...abba...',
    '..cccccc..',
    '.cddddddc.',
    '.cdeeeedc.',
    '.cdeffedc.',
    '.cdeffedc.',
    '.cdeffedc.',
    '.cdeeeedc.',
    '.cddddddc.',
    '.cgggggdc.',
    '.cddddddc.',
    '..cccccc..',
], {'a': '#c4bfd6', 'b': '#8a84a0', 'c': '#5e566b', 'd': '#8a84a0', 'e': '#c97a2a', 'f': '#f2c14e', 'g': '#e9d9b3'},
    '#241a3a')
itens = np.zeros((32, 64, 4), np.uint8)
for i, t in enumerate((cristal, celula_t)):
    grande = ampliar(t.c)
    h_, w_ = grande.shape[:2]
    x_, y_ = i * 32 + (32 - w_) // 2, 30 - h_
    itens[y_:y_ + h_, x_:x_ + w_] = grande

# =====================================================================
# 10. Salva
# =====================================================================
saida = np.dstack([final, np.where(final_alfa, 255, 0).astype(np.uint8)])
Image.fromarray(saida, 'RGBA').save('assets/imagens/planetas/gigante_terreno.png')
Image.fromarray(mascara, 'RGB').save('assets/mascaras/gigante_terreno_mascara.png')
Image.fromarray(ceu, 'RGB').save('assets/imagens/planetas/gigante_ceu.png')
Image.fromarray(itens, 'RGBA').save('assets/imagens/gigante_itens.png')
print('ok')

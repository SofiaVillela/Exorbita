"""pixel_arte.py: peças de desenho usadas por gerar_extraterrestre.py e
gerar_robo.py.

Tem a telinha transparente (Tela), a cena com marcação de piso (Cena)
e o desenho do módulo de pouso, que é
o mesmo de gerar_sem_ar.py: a tripulação desce com o mesmo módulo em
qualquer planeta. Se o módulo mudar lá, mude aqui também.
"""

import numpy as np


def cor(h):
    return np.array([int(h[i:i + 2], 16) for i in (1, 3, 5)], np.uint8)


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



    def espelhar(self):
        """Devolve a mesma figura virada para o outro lado."""
        nova = Tela(self.w, self.h)
        nova.c = self.c[:, ::-1].copy()
        return nova


def desenhar_modulo():
    """Módulo de pouso, 72x74, já com contorno."""
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
    return m


ESCALA = 2                     # tudo é desenhado pequeno e ampliado 2x
CONTORNO_PADRAO = '#16121f'     # o escuro usado em sombras e contornos

BAYER = np.array([[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]]) / 16 + 1 / 32


class Cena:
    """Uma imagem em construção, com a marcação do que bloqueia a passagem."""

    def __init__(self, w, h, fundo):
        self.w, self.h = w, h
        self.img = np.zeros((h, w, 3), np.uint8)
        self.img[:] = cor(fundo)
        self.solido = np.zeros((h, w), bool)
        self.yy, self.xx = np.mgrid[0:h, 0:w]
        self.bayer = np.tile(BAYER, (h // 4 + 1, w // 4 + 1))[:h, :w]

    def area(self, x, y, w, h):
        return (self.xx >= x) & (self.xx < x + w) & (self.yy >= y) & (self.yy < y + h)

    def elipse(self, cx, cy, rx, ry):
        return ((self.xx - cx) / rx) ** 2 + ((self.yy - cy) / ry) ** 2 <= 1

    def pintar(self, m, c):
        self.img[m] = cor(c)

    def ret(self, x, y, w, h, c):
        x0, y0 = max(0, x), max(0, y)
        self.img[y0:max(y0, y + h), x0:max(x0, x + w)] = cor(c)

    def pontilhar(self, m, c, quanto):
        """Pinta só uma parte dos pixels, em xadrez (0 = nada, 1 = tudo)."""
        self.img[m & (self.bayer < quanto)] = cor(c)

    def ruido(self, escala, semente):
        """Ruído suave entre 0 e 1."""
        g = np.random.default_rng(semente).random((self.h // escala + 2, self.w // escala + 2))
        ys, xs = np.arange(self.h) / escala, np.arange(self.w) / escala
        y0, x0 = ys.astype(int), xs.astype(int)
        fy, fx = (ys - y0)[:, None], (xs - x0)[None, :]
        fy, fx = fy * fy * (3 - 2 * fy), fx * fx * (3 - 2 * fx)
        a, b = g[y0][:, x0], g[y0][:, x0 + 1]
        c, d = g[y0 + 1][:, x0], g[y0 + 1][:, x0 + 1]
        return a * (1 - fx) * (1 - fy) + b * fx * (1 - fy) + c * (1 - fx) * fy + d * fx * fy

    def sorteio(self, semente):
        return np.random.default_rng(semente).random((self.h, self.w))

    def luz(self, cx, cy, rx, ry, c, forca=0.5, onde=None):
        """Clarão de luz colorida: mistura a cor em degraus, com pontilhado."""
        d = np.sqrt(((self.xx - cx) / rx) ** 2 + ((self.yy - cy) / ry) ** 2)
        t = np.clip(1 - d, 0, 1) * forca
        degrau = np.floor(t * 4 + self.bayer) / 4
        if onde is not None:
            degrau = np.where(onde, degrau, 0)
        f = degrau[..., None]
        self.img[:] = (self.img * (1 - f) + cor(c) * f).astype(np.uint8)

    def escurecer(self, m, quanto=0.35):
        f = np.where(m, quanto, 0)[..., None]
        self.img[:] = (self.img * (1 - f) + cor(CONTORNO_PADRAO) * f).astype(np.uint8)

    def colar(self, tela, x, y):
        """Cola uma Tela (com transparência) no ponto (x, y)."""
        for ty in range(tela.h):
            for tx in range(tela.w):
                if tela.c[ty, tx, 3] and 0 <= y + ty < self.h and 0 <= x + tx < self.w:
                    self.img[y + ty, x + tx] = tela.c[ty, tx, :3]

    def bloquear(self, x, y, w, h):
        self.solido[max(0, y):max(0, y + h), max(0, x):max(0, x + w)] = True


def ampliar(a):
    return np.repeat(np.repeat(a, ESCALA, 0), ESCALA, 1)


def figura(linhas, paleta, contorno=None):
    """Monta uma Tela a partir de um desenho feito com letras."""
    t = Tela(len(linhas[0]), len(linhas))
    for y, linha in enumerate(linhas):
        for x, letra in enumerate(linha):
            if letra != '.':
                t.px(x, y, paleta[letra])
    if contorno:
        t.contornar(contorno)
    return t

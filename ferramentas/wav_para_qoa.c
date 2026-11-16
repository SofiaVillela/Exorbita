/* wav_para_qoa.c: converte um WAV em QOA, o formato de áudio comprimido
   que a raylib lê. Usado por ferramentas/gerar_musica.py, que compila
   e roda este programa sozinho.

   Uso:  wav_para_qoa entrada.wav saida.qoa */

#include <stdio.h>
#include "raylib.h"

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "uso: %s entrada.wav saida.qoa\n", argv[0]);
        return 1;
    }
    SetTraceLogLevel(LOG_WARNING);
    Wave onda = LoadWave(argv[1]);
    if (onda.data == NULL) {
        return 1;
    }
    bool deuCerto = ExportWave(onda, argv[2]);
    UnloadWave(onda);
    return deuCerto ? 0 : 1;
}

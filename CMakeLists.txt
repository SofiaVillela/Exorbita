# CMakeLists.txt: diz ao CMake como montar o projeto. O CMake não compila
# nada sozinho: ele gera as instruções que o Ninja executa. As linhas que
# começam com # são comentários.
#
# Passo 1 (configurar):  cmake -S . -B build -G Ninja
# Passo 2 (compilar):    cmake --build build

# Versão mínima do CMake exigida.
cmake_minimum_required(VERSION 3.20)

# Nome do projeto e a linguagem usada (C).
project(exorbita C)

# Usar a versão C99 da linguagem C.
set(CMAKE_C_STANDARD 99)
# Se o compilador não souber C99, dar erro em vez de usar outra versão.
set(CMAKE_C_STANDARD_REQUIRED ON)
# Não usar extras do GCC fora do padrão (mantém o código portátil).
set(CMAKE_C_EXTENSIONS OFF)

# Ativa o FetchContent, que baixa bibliotecas da internet na configuração.
include(FetchContent)

# Pede para NÃO compilar os exemplos e jogos que vêm junto com a raylib
# (deixa o build mais rápido).
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

# Diz de onde baixar a raylib e qual versão:
#   GIT_REPOSITORY = endereço do código
#   GIT_TAG 5.5    = versão 5.5, fixada: todo o grupo usa a mesma
#   GIT_SHALLOW    = baixa só essa versão, sem o histórico (mais rápido)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE
)

# Efetivamente baixa e prepara a raylib para ser compilada junto.
FetchContent_MakeAvailable(raylib)

# Cria a lista FONTES com todos os .c dentro de src/ (inclusive subpastas).
# CONFIGURE_DEPENDS: se você criar um .c novo, o CMake percebe sozinho.
file(GLOB_RECURSE FONTES CONFIGURE_DEPENDS src/*.c)

# Define o programa "exorbita" (vira exorbita.exe) feito com esses arquivos.
add_executable(exorbita ${FONTES})

# Diz onde ficam os .h, para #include "config.h" funcionar.
target_include_directories(exorbita PRIVATE include)

# Liga o programa à raylib.
target_link_libraries(exorbita PRIVATE raylib)

# Liga os avisos do compilador: -Wall e -Wextra mostram possíveis erros.
# Regra do grupo: nenhum aviso no nosso código.
target_compile_options(exorbita PRIVATE -Wall -Wextra)

# Depois de cada compilação, copia a pasta assets/ para ao lado do .exe.
# Necessário porque o jogo abre arquivos como "assets/fontes/...",
# e esse caminho é relativo a onde o .exe está.
add_custom_command(TARGET exorbita POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:exorbita>/assets)

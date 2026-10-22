# Versão mínima do CMake necessária.
cmake_minimum_required(VERSION 3.21)

# Nome do projeto e linguagem utilizada.
project(CalcRender LANGUAGES CXX)

# Utiliza o padrão C++17.
set(CMAKE_CXX_STANDARD 17)

# Exige suporte ao padrão escolhido.
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Evita extensões específicas de um compilador.
set(CMAKE_CXX_EXTENSIONS OFF)

# Bibliotecas necessárias.
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)

# Cria o executável
add_executable(CalcRender main.cpp)

# Associa as bibliotecas ao executável.
target_link_libraries(CalcRender PRIVATE OpenGL::GL glfw GLEW::GLEW)
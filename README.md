<div align="center">

# CalcRender

**Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua**

</div>

# Antes de rodar o programa

Garanta que tenha as bibliotecas necessarias baixadas, exemplo no ArchLinux:

```
sudo pacman -S glm glfw glew glu
```

# Compilar e executar o programa

Verifique se esta na pasta que tem o main.cpp e execute:

```
cmake -S . -B build -G Ninja
```

para gerar os arquivos de compilacao. Apos, execute o comando para compilar o projeto na pasta build/:

```
cmake --build build
```

depois basta rodar o programa:

```
./build/CalcRender
```

## Autores

- **Cauã Galdino Garcia**, Universidade Federal de Rondônia (UNIR)
- **Raul Vilela**, Universidade Federal de Rondônia (UNIR)

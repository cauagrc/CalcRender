<div align="center">

# CalcRender

**Iluminando o Cálculo III: construção de um motor de iluminação 3D em C++ e OpenGL**

![Etapa](https://img.shields.io/badge/etapa-prot%C3%B3tipo%20alfa-orange)
![Entrega](https://img.shields.io/badge/entrega-AV1-blue)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white)
![OpenGL](https://img.shields.io/badge/OpenGL-5586A4?logo=opengl&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.21%2B-064F8C?logo=cmake&logoColor=white)

</div>

> [!NOTE]
> Este projeto integra a ação de extensão **Aplicação do Cálculo como Ferramenta Computacional: Modelagem, Otimização e Soluções Tecnológicas para a Comunidade**, na linha temática **Computação Gráfica e Motores 3D**, sob orientação do **Prof. Esp. Rodrigo Duarte de Oliveira Tolêdo**.

O **CalcRender** é um projeto acadêmico de visualização educacional que investiga como conceitos de Cálculo III participam da iluminação em computação gráfica. A proposta final é permitir a exploração de uma superfície tridimensional, relacionando sua inclinação local, seu vetor normal e a direção da luz à intensidade de iluminação difusa.

O protótipo alfa apresenta uma visualização 2D de um círculo, direções normais e raios paralelos animados. O cálculo de Lambert, a malha 3D e o plano tangente fazem parte das próximas etapas.

---

## Sumário

- [Problema e objetivo](#problema-e-objetivo)
- [Protótipo alfa](#protótipo-alfa)
- [Conceitos matemáticos](#conceitos-matemáticos)
- [Funcionamento da alfa](#funcionamento-da-alfa)
- [Como executar](#como-executar)
- [Estrutura do repositório](#estrutura-do-repositório)
- [Pitch](#pitch)
- [Resultados parciais e limitações](#resultados-parciais-e-limitações)
- [Roadmap](#roadmap)
- [Referências](#referências)
- [Autores](#autores)

---

## Problema e objetivo

Conceitos como derivadas parciais, vetores normais e planos tangentes podem ser difíceis de relacionar ao funcionamento de um programa gráfico. O projeto utiliza a iluminação de superfícies como exemplo para tornar essa relação observável.

**Objetivo:** desenvolver um visualizador educacional que mostre como a orientação local de uma superfície influencia sua iluminação, aproximando os conteúdos de Cálculo III da implementação em computação gráfica.

O público pretendido inclui estudantes do ensino médio, técnico e superior interessados em matemática e programação. A disponibilização pública do código e das instruções de execução busca permitir estudo e experimentação pela comunidade acadêmica e externa.

## Protótipo alfa

- Criação de uma janela gráfica com GLFW e OpenGL.
- Desenho de um círculo por aproximação com segmentos da circunferência e triângulos.
- Representação de direções normais por segmentos radiais.
- Criação de raios paralelos com direção horizontal constante.
- Atualização da posição dos raios a cada quadro.
- Desenho de trajetórias com transparência variável.

**O círculo ainda utiliza cor fixa.** A imagem dos raios e de suas trajetórias representa a preparação da cena; ela não comprova, por si só, a aplicação do modelo de Lambert ou uma simulação física completa de luz.

## Conceitos matemáticos

### 1. Parametrização da circunferência — utilizada na alfa

Para um círculo de centro $C=(c_x,c_y)$ e raio $r$, os pontos da borda são calculados por:

$$
x(\theta)=c_x+r\cos\theta
$$

$$
y(\theta)=c_y+r\sin\theta
$$

O código utiliza 100 segmentos para aproximar a circunferência e preenche a região com triângulos que compartilham o centro. Outros segmentos radiais representam visualmente as direções normais.

### 2. Normal do círculo — representação atual e cálculo planejado

Em um ponto $P$ da borda, a direção normal externa coincide com a direção do vetor $P-C$. Sua forma unitária é:

$$
\widehat{N}=\frac{P-C}{\|P-C\|}
$$

Na alfa, as direções são representadas por segmentos radiais calculados com seno e cosseno. A normalização para avaliar a iluminação nos pontos de contato será incorporada nas próximas etapas. O comprimento desenhado desses segmentos é uma escolha visual.

### 3. Produto escalar e Lambert — fundamentação da próxima etapa

Com a normal $\widehat{N}$ e a direção até a fonte $\widehat{L}$ normalizadas:

$$
\widehat{N}\cdot\widehat{L}=\cos\theta
$$

O termo difuso simplificado é:

$$
I_d=k_d\,I_l\,\max(0,\widehat{N}\cdot\widehat{L})
$$

- $I_d$: contribuição difusa da iluminação.
- $k_d$: coeficiente de resposta difusa do material.
- $I_l$: intensidade da luz.
- $\theta$: ângulo entre a normal e a direção até a fonte.

O máximo com zero evita uma contribuição negativa. Para raios que se propagam em direção à superfície, a direção **até a fonte** é oposta à direção de propagação. Todos os vetores devem estar no mesmo sistema de coordenadas.

Essa expressão ainda não é avaliada pelo código da alfa.

### 4. Derivadas parciais e plano tangente — proposta para a versão 3D

Para uma superfície diferenciável $z=f(x,y)$, uma normal unitária orientada para cima é:

$$
\widehat{N}=\frac{(-f_x,-f_y,1)}{\sqrt{f_x^2+f_y^2+1}}
$$

O plano tangente no ponto $(a,b,f(a,b))$ é:

$$
z=f(a,b)+f_x(a,b)(x-a)+f_y(a,b)(y-b)
$$

As derivadas parciais descrevem a inclinação local e permitem calcular a normal e o plano tangente. Esses cálculos integram a proposta de evolução do visualizador.

## Funcionamento da alfa

1. O `Engine` inicializa GLFW, cria a janela e carrega as funções OpenGL com GLEW.
2. Uma projeção ortográfica estabelece as coordenadas da cena 2D.
3. O objeto `Circle` desenha o círculo e os segmentos radiais.
4. Os objetos `Raio` representam trajetórias paralelas, com direção inicial $(1,0)$.
5. A cada quadro, os raios são desenhados, suas posições avançam e um novo ponto é adicionado à trajetória.
6. A aplicação apresenta o quadro e processa os eventos da janela.

A alfa utiliza o **modo imediato do OpenGL**, com funções como `glBegin`, `glVertex2f` e `glOrtho`. Ela depende de um contexto que disponibilize essas funções legadas. A versão atual não utiliza shaders próprios nem solicita um perfil OpenGL Core.

## Como executar

**Requisitos:** Git, compilador com suporte a C++17, CMake 3.21 ou superior, Ninja e bibliotecas de desenvolvimento OpenGL, GLFW, GLEW e GLM.

No Linux Mint ou Ubuntu:

```bash
# Instalar as ferramentas e dependências
sudo apt update
sudo apt install git build-essential cmake ninja-build libglfw3-dev libglew-dev libglm-dev libglu1-mesa-dev

# Clonar a versão alfa
git clone --branch pitch --single-branch https://github.com/cauagrc/CalcRender.git
cd CalcRender

# Configurar e compilar
cmake -S . -B build -G Ninja
cmake --build build

# Executar
./build/CalcRender
```

Para encerrar a aplicação, feche a janela.

## Estrutura do repositório

| Arquivo | Responsabilidade |
|---|---|
| `main.cpp` | Inicialização gráfica, círculo, raios e laço de execução |
| `CMakeLists.txt` | Configuração da compilação e das dependências |
| `README.md` | Apresentação do projeto e instruções de uso |

A pasta `build/` é gerada localmente durante a configuração. A organização do código em módulos poderá evoluir com a implementação dos cálculos e da visualização 3D.

## Pitch

O pitch **Iluminando o Cálculo III** apresenta a motivação educacional, a base matemática e o protótipo alfa, além da proposta final de iluminação em uma malha 3D.

A apresentação acompanha a entrega. O código correspondente está na branch [`pitch`](https://github.com/cauagrc/CalcRender/tree/pitch).

Os tópicos abordados incluem:

- Orientação das superfícies e vetores normais.
- Relação entre ângulo de incidência e intensidade difusa no modelo de Lambert.
- Visualização 2D construída nesta etapa.
- Evolução prevista para derivadas parciais, plano tangente e exploração de uma superfície 3D.

## Resultados parciais e limitações

A versão alfa reúne o desenho da geometria 2D e a animação de raios paralelos em uma mesma cena. Ela constitui o ponto de partida para relacionar os dados geométricos aos cálculos de iluminação.

As limitações atuais são:

- Os raios ainda não identificam nem param nos pontos de contato com o círculo.
- A cor do círculo é fixa e ainda não depende do produto escalar de Lambert.
- O histórico de pontos das trajetórias cresce durante a execução.
- O avanço dos raios é definido por quadro, sem compensação pelo tempo decorrido.
- A cena ainda não possui malha 3D, câmera orbital ou plano tangente.
- Não há resultados quantitativos de desempenho ou avaliação educacional apresentados nesta entrega.

## Roadmap

- [ ] Calcular as interseções entre os raios e o círculo.
- [ ] Obter a normal unitária e o fator de Lambert nos pontos analisados.
- [ ] Variar a intensidade visual conforme a iluminação calculada.
- [ ] Limitar o armazenamento das trajetórias e controlar a animação pelo tempo.
- [ ] Construir uma malha para uma superfície $z=f(x,y)$.
- [ ] Calcular suas normais por derivadas parciais e representar o plano tangente.
- [ ] Implementar câmera para girar, aproximar e afastar a visualização.
- [ ] Validar os cálculos com exemplos conhecidos e organizar a documentação técnica.
- [ ] Preparar a versão beta e a demonstração pública da Mostra Matemática em Código.

## Referências

- **SANTIAGO, Judson.** Aula 16: Iluminação — luz ambiente, difusa e especular, Fresnel e Lambert. [Vídeo](https://www.youtube.com/watch?v=KX-f3-JBg9w) e [material de apoio](https://github.com/JudsonSS/CG).
- **LOW LEVEL GAME DEV.** Simulating a black hole in C++! [Vídeo](https://www.youtube.com/watch?v=EtNBtEwo4PI).
- **KAVAN.** Simulating Black Holes in C++. [Vídeo](https://www.youtube.com/watch?v=8-B6ryuBkCM) e [repositório de referência](https://github.com/kavan010/black_hole).

Os projetos de buracos negros servem de inspiração para a organização de uma aplicação gráfica nativa. A proposta do CalcRender nesta entrega é estudar a geometria e a iluminação difusa.

## Autores

- **Cauã Galdino Garcia** — Ciência da Computação, Universidade Federal de Rondônia (UNIR).
- **Raul Vilela** — Ciência da Computação, Universidade Federal de Rondônia (UNIR).

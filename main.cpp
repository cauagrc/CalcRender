#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>

// Define o valor de PI caso não esteja disponível.
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace glm;
using namespace std;


// ============================================================
// ENGINE
// Responsável por inicializar o ambiente gráfico e a janela.
// ============================================================

struct Engine {
    GLFWwindow* window;

    // Dimensões da janela, em pixels.
    int WIDTH = 1200;
    int HEIGHT = 720;

    // Limites do sistema de coordenadas do mundo.
    float width = 1000.0f;
    float height = 600.0f;

    // Construtor: inicializa o GLFW, a janela e o GLEW.
    Engine() {

        // Inicializa a biblioteca GLFW.
        if (!glfwInit()) {
            cerr << "Erro ao inicializar o GLFW.\n";
            exit(EXIT_FAILURE);
        }

        // Cria a janela da aplicação.
        // Os nullptr indicam que será usada uma janela normal,
        // sem modo de ecrã inteiro nem partilha de contexto.
        window = glfwCreateWindow(
            WIDTH,
            HEIGHT,
            "CalcRender",
            nullptr,
            nullptr
        );

        // Verifica se a janela foi criada corretamente.
        if (window == nullptr) {
            cerr << "Erro ao criar a janela.\n";

            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        // Define o contexto OpenGL que será utilizado.
        glfwMakeContextCurrent(window);

        // Permite ao GLEW carregar as funções do OpenGL.
        glewExperimental = GL_TRUE;

        if (glewInit() != GLEW_OK) {
            cerr << "Erro ao inicializar o GLEW.\n";

            glfwDestroyWindow(window);
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        // Define a área de renderização inicial.
        glViewport(0, 0, WIDTH, HEIGHT);
    }


    // Prepara a tela antes de desenhar cada quadro.
    void run() {

        // Define a cor de fundo como preto.
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        // Limpa o buffer de cor.
        glClear(GL_COLOR_BUFFER_BIT);

        // Seleciona e reinicia a matriz de projeção.
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        // Define uma projeção ortográfica.
        // O eixo X vai de -1000 a 1000.
        // O eixo Y vai de -600 a 600.
        glOrtho(
            -width, width,
            -height, height,
            -1.0, 1.0
        );

        // Reinicia a matriz utilizada para as transformações
        // dos objetos que serão desenhados.
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
};


// ============================================================
// CIRCLE
// Representa um círculo e contém o método responsável
// pelo seu desenho no ecrã.
// ============================================================

struct Circle {
    vec2 position;  // Posição do centro do círculo.
    double radius;  // Raio do círculo.

    // Recebe a posição inicial e define o raio.
    Circle(vec2 pos) : position(pos), radius(250) {}


    // Desenha o círculo utilizando vários triângulos.
    void draw() {

        glColor3f(1.0f, 1.0f, 1.0f);
        glLineWidth(2.0f);

        int numeroDeLinhas = 12;

        glBegin(GL_LINES);

        for(int i=0; i<numeroDeLinhas; i++){
          float angle = 2.0f * M_PI * i / numeroDeLinhas;

          float x1 = position.x;
          float y1 = position.y;

          float x2 = cos(angle) * radius * 1.5f + x1;
          float y2 = sin(angle) * radius * 1.5f + y1;

          glVertex2f(x1,y1);
          glVertex2f(x2,y2);
        }

        glEnd();
        // Define a cor do círculo como vermelho.
        glColor3f(1.0f, 0.0f, 0.0f);

        // Inicia um conjunto de triângulos que partilham
        // o mesmo vértice central.
        glBegin(GL_TRIANGLE_FAN);

        // Define o centro do círculo.
        glVertex2f(position.x, position.y);

        // Calcula os pontos da circunferência.
        // Os 100 segmentos aproximam a forma de um círculo.
        for (int i = 0; i <= 100; i++) {

            // Converte a posição do segmento num ângulo.
            float angle = 2.0f * M_PI * i / 100;

            // Calcula as coordenadas do ponto na circunferência.
            float x = cos(angle) * radius + position.x;
            float y = sin(angle) * radius + position.y;

            // Adiciona o ponto à geometria.
            glVertex2f(x, y);
        }

        // Termina a definição dos triângulos.
        glEnd();

    }
};

// ============================================================
// RAIO DE LUZ 
// Representa os raios de luz que serao
// usados para o calculo de Lambert
// ============================================================
struct Raio{
  double x; double y;
  vec2 dir;
  vector<vec2> trail;

  Raio(vec2(pos), vec2(dir)) : x(pos.x), y(pos.y), dir(dir){trail.push_back(pos);}

  void draw(){
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(1.0f);

    size_t N = trail.size();
    if(N<2) return;

    glBegin(GL_LINE_STRIP);
    for(size_t i=0; i<N; i++){
      float alpha = float(i) / float(N-1);
      glColor4f(1.0f,1.0f,1.0f, std::max(alpha, 0.05f));
      glVertex2f(trail[i].x, trail[i].y);
    }
    glEnd();
  }

  void step(){
    x+=dir.x*4;
    y+=dir.y*4;
    trail.push_back({x,y});
  }
};

// ============================================================
// OBJETOS GLOBAIS
// ============================================================

// Cria o motor gráfico, um círculo com centro em (0, 0) e raios de luzes.
vector<Raio> raios;
Engine engine;
Circle circle(vec2(0.0f, 0.0f));


// ============================================================
// MAIN
// Controla o ciclo principal da aplicação.
// ============================================================

int main() {
    for(float y = -engine.height; y<engine.height; y+=100){
      raios.push_back(Raio(vec2(-engine.width, y), vec2(1.0,0.0)));
    };

    // Mantém a aplicação em execução enquanto a janela
    // não receber um pedido de encerramento.
    while (!glfwWindowShouldClose(engine.window)) {

        // Limpa a tela e prepara as matrizes.
        engine.run();

        // Desenha o círculo.
        circle.draw();

        for (auto& raio : raios){
          raio.draw();
          raio.step();
        }

        // Apresenta o quadro desenhado na janela.
        glfwSwapBuffers(engine.window);

        // Processa eventos, como o fecho da janela.
        glfwPollEvents();
    }

    // Liberta os recursos utilizados pela aplicação.
    glfwDestroyWindow(engine.window);
    glfwTerminate();

    return 0;
}

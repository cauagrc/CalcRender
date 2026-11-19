#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cstdlib>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <vector>

// Define o valor de PI caso não esteja disponível.
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace glm;
using namespace std;

// ============================================================
// CAMERA
// Ponto onde o usuario estara vendo a simulacao
// ============================================================
struct Camera{
  float yaw=0.6f, pitch=0.4f, dist=1800.0f;
  vec3 target = vec3(0.0f);

  vec3 position() const {
    return target + dist * vec3(cos(pitch)*sin(yaw),
        sin(pitch),
        cos(pitch) * cos(yaw)
        );
  }
  mat4 view() const { return lookAt(position(), target, vec3(0, 1, 0));}
};

Camera camera;
bool dragging = false;
double lastX = 0, lastY = 0;

void mouseButtonCallback(GLFWwindow *w, int button, int action, int) {
  if(button == GLFW_MOUSE_BUTTON_LEFT){
    dragging = (action == GLFW_PRESS);
    glfwGetCursorPos(w, &lastX, &lastY);
  }
};

void cursorCallback(GLFWwindow*, double x, double y){
  if(dragging) {
    camera.yaw -= float(x-lastX) * 0.005f;
    camera.pitch += float(y-lastY) * 0.005f;
    camera.pitch = glm::clamp(camera.pitch, -1.5f, 1.5f);
  }
  lastX=x, lastY=y;
}

void scrollCallback(GLFWwindow*, double, double yoff) {
  camera.dist *= (yoff>0) ? 0.92f : 1.08f;
  camera.dist = glm::clamp(camera.dist, 300.0f, 6000.0f);
}

// ============================================================
// ENGINE
// Responsável por inicializar o ambiente gráfico e a janela.
// ============================================================

struct Engine {
  GLFWwindow *window;

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
    window = glfwCreateWindow(WIDTH, HEIGHT, "CalcRender", nullptr, nullptr);

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
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, WIDTH, HEIGHT);

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorCallback);
    glfwSetScrollCallback(window, scrollCallback);
  }

  // Prepara a tela antes de desenhar cada quadro.
  void run() {
    int fbw, fbh;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    if (fbh == 0)
      fbh = 1;
    glViewport(0, 0, fbw, fbh);

    // Define a cor de fundo como preto.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Limpa o buffer de cor.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4 proj = perspective(radians(60.0f), float(fbw) / fbh, 10.0f, 10000.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(value_ptr(proj));

    mat4 view = camera.view();
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(value_ptr(view));
  }
};

// ============================================================
// CIRCLE
// Representa um círculo e contém o método responsável
// pelo seu desenho no ecrã.
// ============================================================

struct Circle {
  vec3 position; // Posição do centro do círculo.
  double radius; // Raio do círculo.

  // Recebe a posição inicial e define o raio.
  Circle(vec3 pos) : position(pos), radius(250) {}

  // Desenha o círculo utilizando vários triângulos.
  void draw() {

    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);

    int numeroDeLinhas = 12;

    glBegin(GL_LINES);

    for (int i = 0; i < numeroDeLinhas; i++) {
      float angle = 2.0f * M_PI * i / numeroDeLinhas;

      float x1 = position.x;
      float y1 = position.y;

      float x2 = cos(angle) * radius * 1.5f + x1;
      float y2 = sin(angle) * radius * 1.5f + y1;

      glVertex2f(x1, y1);
      glVertex2f(x2, y2);
    }

    glEnd();
    // Define a cor do círculo como vermelho.
    /*
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
    */
    const int stacks = 40, slices = 80;
    glColor3f(1.0f, 0.2f, 0.2f);

    for(int i = 0; i<stacks; i++) {
      float phi1 = M_PI * i / stacks;
      float phi2 = M_PI * (i + 1) / stacks;

      glBegin(GL_TRIANGLE_STRIP);
      for(int j=0; j<=slices; j++) {
        float theta = 2.0f * M_PI * j / slices;
        for (int k=0; k<2; k++) {
          float phi = (k == 0) ? phi1 : phi2;
          vec3 n(sin(phi)*cos(theta), cos(phi), sin(phi) * sin(theta));
          vec3 p = position + n * float(radius);
          glVertex3f(p.x, p.y, p.z);
        }
      }
      glEnd();
    }
  }
};

// ============================================================
// RAIO DE LUZ
// Representa os raios de luz que serao
// usados para o calculo de Lambert
// ============================================================
struct Raio {
  vec3 pos;
  vec3 dir;
  vector<vec3> trail;

  Raio(vec3 p, vec3 d) : pos(p), dir(normalize(d)) { trail.push_back(pos); }

  void draw() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(1.0f);

    size_t N = trail.size();
    if (N < 2)
      return;

    glBegin(GL_LINE_STRIP);
    for (size_t i = 0; i < N; i++) {
      float alpha = float(i) / float(N - 1);
      glColor4f(1.0f, 1.0f, 1.0f, std::max(alpha, 0.05f));
      glVertex3f(trail[i].x, trail[i].y, trail[i].z);
    }
    glEnd();
  }

  void step() {
    pos += dir * 4.0f;
    trail.push_back(pos);
  }
};



// ============================================================
// OBJETOS GLOBAIS
// ============================================================

// Cria o motor gráfico, um círculo com centro em (0, 0) e raios de luzes.
vector<Raio> raios;
Engine engine;
Circle circle(vec3(0.0f));

// ============================================================
// MAIN
// Controla o ciclo principal da aplicação.
// ============================================================

int main() {
  for (float y = -engine.height; y < engine.height; y += 100) {
    raios.push_back(Raio(vec3(-engine.width, y, 0.0f), vec3(1.0f, 0.0f, 0.0f)));
  };

  // Mantém a aplicação em execução enquanto a janela
  // não receber um pedido de encerramento.
  while (!glfwWindowShouldClose(engine.window)) {

    // Limpa a tela e prepara as matrizes.
    engine.run();

    // Desenha o círculo.
    circle.draw();

    for (auto &raio : raios) {
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

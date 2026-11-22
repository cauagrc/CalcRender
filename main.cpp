#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <GL/glew.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <limits>
#include <vector>

// Define o valor de PI caso não esteja disponível.
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace glm;
using namespace std;

const float ALTURA = 240.0f;
const float ESCALA = 260.0f;
const float DOMINIO = 450.0f;
const float VELOCIDADE = 280.0f;
const float RASTRO = 180.0f;

const vec3 COR_BASE(1.0f, 0.12f, 0.08f);

// ============================================================
// CAMERA
// Ponto onde o usuario estara vendo a simulacao
// ============================================================
struct Camera{
  float yaw=0.6f, pitch=0.4f, dist=1800.0f;
  vec3 target = vec3(0.0f);

  vec3 position() const {
    //Invertia a direcao do mouse
    /**
     * return target + dist * vec3(cos(pitch)*sin(yaw),
        sin(pitch),
        cos(pitch) * cos(yaw)
        );
     */
    return target + dist * vec3(
      cos(pitch) * cos(yaw),
      cos(pitch) * sin(yaw),
      sin(pitch)
    );
  }

  mat4 view() const {
    vec3 eye = position();

    // Fixa a câmera de lado.
    // eye = target + vec3(-1450.0f, 0.0f, 0.0f);

    return lookAt(eye, target, vec3(0, 0, 1));
  }
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

void keyCallback(GLFWwindow *w, int key, int, int action, int);

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

  float dt = 0.0f;
  double lastTime = 0.0;

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

    // Esconde o que está atrás de outro objeto.
    glEnable(GL_DEPTH_TEST);

    // Suaviza a cor entre os vértices.
    glShadeModel(GL_SMOOTH);
    // Sincroniza os quadros com a tela.
    glfwSwapInterval(1);

    // Registra os controles de mouse e teclado.
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);

    // Guarda o tempo inicial.
    lastTime = glfwGetTime();
  }

  // Prepara a tela antes de desenhar cada quadro.
  bool run() {
    // Calcula o tempo entre dois quadros.
    double now = glfwGetTime();

    dt = float(std::clamp(now - lastTime, 0.0, 0.1));

    lastTime = now;

    int fbw, fbh;
    glfwGetFramebufferSize(window, &fbw, &fbh);

    if (fbw <= 0 || fbh <= 0) {
      glfwWaitEventsTimeout(0.05);
      return false;
    }

    // Ajusta o desenho ao tamanho atual da janela.
    glViewport(0, 0, fbw, fbh);

    // Define a cor de fundo como preto.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // TESTE VISUAL: troca o fundo preto por cinza.
    // glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    // Limpa o buffer de cor.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TESTE VISUAL: mostra também o que está atrás da esfera.
    // glDisable(GL_DEPTH_TEST);

    float aspect = float(fbw) / fbh;

    // Define a perspectiva da cena.
    mat4 proj = perspective(
      radians(50.0f),
      aspect,
      5.0f,
      10000.0f
    );

    // TESTE VISUAL: aproxima a imagem sem mover a câmera.
    // proj = perspective(radians(25.0f), aspect, 5.0f, 10000.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(value_ptr(proj));

    // Usa a posição atual da câmera.
    mat4 view = camera.view();

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(value_ptr(view));

    return true;
  }
};

// Os dois vetores recebidos devem ter tamanho 1.
static float lambert(vec3 normal, vec3 lightDir) {
  return std::max(0.0f, dot(normal, -lightDir));
}

// ============================================================
// SPHERE
// Desenha a esfera usando triângulos.
// ============================================================

struct Sphere {
  // Posição do centro da esfera.
  vec3 position;
  // Raio da esfera.
  float radius;

  // Recebe a posição inicial e define o raio.
  Sphere(vec3 pos) : position(pos), radius(250.0f) {}

  void draw(vec3 lightDir, bool usarLambert = true) {
    // Define as divisões da esfera.
    int stacks = 40, slices = 80;

    // TESTE VISUAL: usa menos divisões na esfera.
    // stacks = 8; slices = 12;

    // TESTE VISUAL: mostra somente as linhas dos triângulos.
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    for (int i = 0; i < stacks; i++) {
      // Define os dois limites da faixa.
      float phi1 = M_PI * i / stacks;
      float phi2 = M_PI * (i + 1) / stacks;

      glBegin(GL_TRIANGLE_STRIP);

      for (int j = 0; j <= slices; j++) {
        float theta = 2.0f * M_PI * j / slices;

        for (int k = 0; k < 2; k++) {
          float phi = (k == 0) ? phi1 : phi2;

          // Calcula a direção do centro até a superfície.
          vec3 n(
            sin(phi) * cos(theta),
            sin(phi) * sin(theta),
            cos(phi)
          );

          // Calcula a posição do ponto na esfera.
          vec3 p = position + n * radius;

          // TESTE VISUAL: usa a mesma normal em toda a esfera.
          // n = vec3(-1.0f, 0.0f, 0.0f);

          // Calcula a iluminação neste ponto.
          float light = usarLambert
            ? lambert(n, lightDir)
            : 1.0f;

          // TESTE VISUAL: deixa toda a esfera com a mesma cor.
          // light = 1.0f;

          // TESTE VISUAL: mantém uma claridade mínima na sombra.
          // light = 0.1f + 0.9f * light;

          // Aplica a iluminação à cor da esfera.
          vec3 c = COR_BASE * light;

          // TESTE VISUAL: representa a direção das normais com cores.
          // c = (n + vec3(1.0f)) * 0.5f;

          glColor3f(c.r, c.g, c.b);
          // Adiciona o ponto à geometria.
          glVertex3f(p.x, p.y, p.z);
        }
      }

      glEnd();
    }

    // Restaura o preenchimento depois de terminar os triângulos.
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
};

// ============================================================
// RAIO DE LUZ
// Guarda o movimento e desenha o rastro de um raio.
// ============================================================

struct Raio {
  // Posição de onde o raio parte.
  vec3 origem;
  // Direção em que o raio viaja.
  vec3 dir;

  // Distância máxima percorrida pelo raio.
  float limite;
  // Distância já percorrida.
  float progresso;

  // Indica se o raio atingiu a superfície.
  bool atingiu;

  // Desenha o rastro do raio.
  void draw() {
    // Limita o rastro ao ponto de impacto.
    float frente = glm::clamp(progresso, 0.0f, limite);

    float cauda = glm::clamp(
      progresso - RASTRO,
      0.0f,
      limite
    );

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDepthMask(GL_FALSE);
    glLineWidth(1.0f);

    // TESTE VISUAL: engrossa o rastro dos raios.
    // glLineWidth(4.0f);

    if (frente > cauda) {
      glBegin(GL_LINES);

      // Deixa o começo do rastro mais transparente.
      glColor4f(1.0f, 1.0f, 1.0f, 0.05f);

      vec3 inicio = origem + dir * cauda;
      glVertex3f(inicio.x, inicio.y, inicio.z);

      glColor4f(1.0f, 1.0f, 1.0f, 0.85f);

      vec3 fim = origem + dir * frente;
      glVertex3f(fim.x, fim.y, fim.z);

      glEnd();
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glLineWidth(1.0f);

    if (atingiu) {
      // Marca onde o raio atinge a superfície.
      glPointSize(5.0f);

      // TESTE VISUAL: aumenta os pontos de impacto.
      // glPointSize(12.0f);

      glColor3f(1.0f, 0.55f, 0.1f);

      vec3 p = origem + dir * (limite - 0.7f);

      glBegin(GL_POINTS);
      glVertex3f(p.x, p.y, p.z);
      glEnd();

      glPointSize(1.0f);
    }
  }

  // Atualiza o movimento do raio.
  void step(float dt) {
    float passo = VELOCIDADE * dt;

    // TESTE VISUAL: deixa os raios mais rápidos.
    // passo *= 3.0f;

    // TESTE VISUAL: congela os raios.
    // passo = 0.0f;

    // Reinicia depois que o rastro desaparece.
    progresso = fmod(
      progresso + passo,
      limite + RASTRO + 100.0f
    );
  }
};

// ============================================================
// OBJETOS GLOBAIS
// ============================================================

vector<Raio> raios;

Engine engine;
Sphere sphere(vec3(0.0f));

vec3 lightDir = normalize(vec3(1.0f, 0.0f, 0.0f));

enum class Cena {
  Esfera,
  Gaussiana
};

Cena cena = Cena::Esfera;

float pontoA = -2.8f;
float pontoB = 0.35f;

bool mostrarNormal = true;
bool mostrarPlano = true;
bool mostrarRaios = true;

bool usarLambert = true;
bool pausado = false;

// ============================================================
// CÁLCULOS DA SUPERFÍCIE
// ============================================================

// Calcula a altura da superfície.
float funcao(float x, float y) {
  return ALTURA * exp(
    -(x * x + y * y) / (ESCALA * ESCALA)
  );
}

// Calcula a inclinação na direção de X.
float derivadaX(float x, float y) {
  return -2.0f * x * funcao(x, y) / (ESCALA * ESCALA);
}

// Calcula a inclinação na direção de Y.
float derivadaY(float x, float y) {
  return -2.0f * y * funcao(x, y) / (ESCALA * ESCALA);
}

// Calcula a normal a partir das derivadas.
vec3 normalFuncao(float x, float y) {
  return normalize(
    vec3(-derivadaX(x, y), -derivadaY(x, y), 1.0f)
  );
}

struct PontoEstudo {
  vec3 p;
  vec3 n;
  vec3 u;
  vec3 v;
};

// Obtém o ponto, a normal e as direções do plano.
PontoEstudo pontoEstudo() {
  if (cena == Cena::Esfera) {
    vec3 n(
      cos(pontoB) * cos(pontoA),
      cos(pontoB) * sin(pontoA),
      sin(pontoB)
    );

    vec3 u(-sin(pontoA), cos(pontoA), 0.0f);

    return {
      sphere.position + n * sphere.radius,
      n,
      u,
      normalize(cross(n, u))
    };
  }

  float x = pontoA * DOMINIO;
  float y = pontoB * DOMINIO;

  return {
    vec3(x, y, funcao(x, y)),
    normalFuncao(x, y),
    normalize(vec3(1.0f, 0.0f, derivadaX(x, y))),
    normalize(vec3(0.0f, 1.0f, derivadaY(x, y)))
  };
}

// Infinito indica que o raio não atingiu a superfície.
float semImpacto() {
  return numeric_limits<float>::infinity();
}

// Calcula onde o raio atinge a esfera.
float intersecaoEsfera(vec3 origem, vec3 direcao) {
  vec3 p = origem - sphere.position;

  float a = dot(direcao, direcao);

  if (a <= 0.0f) {
    return semImpacto();
  }

  float b = 2.0f * dot(p, direcao);

  float c =
    dot(p, p) - sphere.radius * sphere.radius;

  float delta = b * b - 4.0f * a * c;

  if (delta < 0.0f) {
    return semImpacto();
  }

  float raiz = sqrt(delta);

  float t1 = (-b - raiz) / (2.0f * a);
  float t2 = (-b + raiz) / (2.0f * a);

  // Escolhe o primeiro impacto à frente do raio.
  float t = t1 >= 0.0f
    ? t1
    : (t2 >= 0.0f ? t2 : semImpacto());

  return t * sqrt(a);
}

// Usa a direção +X dos raios desta cena.
float intersecaoGaussiana(vec3 origem) {
  if (
    abs(origem.y) > DOMINIO ||
    origem.z <= 0.0f ||
    origem.z > ALTURA
  ) {
    return semImpacto();
  }

  float x2 =
    -ESCALA * ESCALA * log(origem.z / ALTURA)
    - origem.y * origem.y;

  if (x2 < 0.0f) {
    return semImpacto();
  }

  float x = sqrt(x2);

  if (x > DOMINIO) {
    return semImpacto();
  }

  for (float candidato : {-x, x}) {
    if (candidato >= origem.x) {
      return candidato - origem.x;
    }
  }

  return semImpacto();
}

// Cria os raios e calcula seus impactos.
void criarRaios() {
  // Remove os raios da cena anterior.
  raios.clear();

  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      float y = -240.0f + i * 120.0f;

      float z = cena == Cena::Esfera
        ? -240.0f + j * 120.0f
        : 30.0f + j * 45.0f;

      vec3 origem(-850.0f, y, z);

      // Calcula a distância até a superfície.
      float impacto = cena == Cena::Esfera
        ? intersecaoEsfera(origem, lightDir)
        : intersecaoGaussiana(origem);

      // TESTE VISUAL: deixa os raios seguirem depois da superfície.
      // impacto = semImpacto();

      bool atingiu = std::isfinite(impacto);

      float limite = atingiu ? impacto : 1700.0f;

      // Faz os rastros começarem em posições diferentes.
      float inicio = fmod(
        float(i * 113 + j * 157),
        limite + RASTRO + 100.0f
      );

      raios.push_back({
        origem,
        lightDir,
        limite,
        inicio,
        atingiu
      });
    }
  }
}

// Troca a superfície e recria os raios.
void selecionarCena(Cena nova) {
  cena = nova;

  pontoA = cena == Cena::Esfera ? -2.8f : -0.35f;
  pontoB = cena == Cena::Esfera ? 0.35f : -0.20f;

  criarRaios();
}

// Processa as teclas pressionadas.
void keyCallback(
  GLFWwindow *w,
  int key,
  int,
  int action,
  int
) {
  if (action != GLFW_PRESS) {
    return;
  }

  switch (key) {
    case GLFW_KEY_ESCAPE:
      glfwSetWindowShouldClose(w, GLFW_TRUE);
      break;

    case GLFW_KEY_1:
      selecionarCena(Cena::Esfera);
      break;

    case GLFW_KEY_2:
      selecionarCena(Cena::Gaussiana);
      break;

    case GLFW_KEY_N:
      mostrarNormal = !mostrarNormal;
      break;

    case GLFW_KEY_P:
      mostrarPlano = !mostrarPlano;
      break;

    case GLFW_KEY_R:
      mostrarRaios = !mostrarRaios;
      break;

    case GLFW_KEY_L:
      usarLambert = !usarLambert;
      break;

    case GLFW_KEY_SPACE:
      pausado = !pausado;
      break;

    case GLFW_KEY_HOME:
      camera = Camera{};
      break;
  }
}

// Move o ponto selecionado pelas setas do teclado.
void moverPonto(float dt) {
  float passo =
    (cena == Cena::Esfera ? 1.0f : 0.6f) * dt;

  if (glfwGetKey(engine.window, GLFW_KEY_LEFT) == GLFW_PRESS)
    pontoA -= passo;

  if (glfwGetKey(engine.window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    pontoA += passo;

  if (glfwGetKey(engine.window, GLFW_KEY_UP) == GLFW_PRESS)
    pontoB += passo;

  if (glfwGetKey(engine.window, GLFW_KEY_DOWN) == GLFW_PRESS)
    pontoB -= passo;

  if (cena == Cena::Esfera) {
    pontoA = remainder(pontoA, 2.0f * float(M_PI));
    pontoB = glm::clamp(pontoB, -1.45f, 1.45f);
  } else {
    pontoA = glm::clamp(pontoA, -1.0f, 1.0f);
    pontoB = glm::clamp(pontoB, -1.0f, 1.0f);
  }
}

// ============================================================
// DESENHO DA GAUSSIANA E DO PLANO
// ============================================================

void vertice(vec3 p) {
  glVertex3f(p.x, p.y, p.z);
}

// Desenha a gaussiana usando vários triângulos.
void desenharGaussiana() {
  const int faixas = 48, divisoes = 96;

  // TESTE VISUAL: mostra os triângulos da gaussiana.
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  for (int i = 0; i < faixas; i++) {
    glBegin(GL_TRIANGLE_STRIP);

    for (int j = 0; j <= divisoes; j++) {
      for (int k = 0; k < 2; k++) {
        float x =
          -DOMINIO + 2.0f * DOMINIO * j / divisoes;

        float y =
          -DOMINIO + 2.0f * DOMINIO * (i + k) / faixas;

        vec3 n = normalFuncao(x, y);

        float light = usarLambert
          ? lambert(n, lightDir)
          : 1.0f;

        vec3 c = COR_BASE * light;

        // TESTE VISUAL: representa as normais da gaussiana com cores.
        // c = (n + vec3(1.0f)) * 0.5f;

        glColor3f(c.r, c.g, c.b);
        vertice(vec3(x, y, funcao(x, y)));
      }
    }

    glEnd();
  }

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// Desenha o plano tangente, a normal e o ponto selecionado.
void desenharEstudo() {
  PontoEstudo q = pontoEstudo();

  // Desenha o plano tangente.
  if (mostrarPlano) {
    vec3 u = q.u * 110.0f;
    vec3 v = q.v * 110.0f;

    // TESTE VISUAL: aumenta o plano tangente.
    // u *= 2.0f; v *= 2.0f;

    float alpha = 0.35f;

    // TESTE VISUAL: tira a transparência do plano.
    // alpha = 1.0f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDepthMask(GL_FALSE);

    // Evita piscadas onde o plano encosta na superfície.
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f);

    glColor4f(0.1f, 0.65f, 1.0f, alpha);

    glBegin(GL_TRIANGLES);

    vertice(q.p - u - v);
    vertice(q.p + u - v);
    vertice(q.p + u + v);

    vertice(q.p - u - v);
    vertice(q.p + u + v);
    vertice(q.p - u + v);

    glEnd();

    glDisable(GL_POLYGON_OFFSET_FILL);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
  }

  // Desenha a seta da normal.
  if (mostrarNormal) {
    vec3 ponta = q.p + q.n * 130.0f;

    // TESTE VISUAL: aumenta a seta da normal.
    // ponta = q.p + q.n * 260.0f;

    glColor3f(1.0f, 0.9f, 0.1f);

    glBegin(GL_LINES);

    vertice(q.p + q.n);
    vertice(ponta);

    vertice(ponta);
    vertice(ponta - q.n * 20.0f + q.u * 9.0f);

    vertice(ponta);
    vertice(ponta - q.n * 20.0f - q.u * 9.0f);

    glEnd();
  }

  // Marca o ponto selecionado em verde.
  glPointSize(8.0f);
  glColor3f(0.2f, 1.0f, 0.4f);

  glBegin(GL_POINTS);
  vertice(q.p + q.n * 1.5f);
  glEnd();

  glPointSize(1.0f);
}

// ============================================================
// MAIN
// Executa o programa.
// ============================================================

int main() {
  criarRaios();

  // Mantém a aplicação em execução enquanto a janela
  // não receber um pedido de encerramento.
  while (!glfwWindowShouldClose(engine.window)) {

    // Limpa a tela e prepara as matrizes.
    if (!engine.run()) {
      continue;
    }

    // Atualiza o ponto selecionado.
    moverPonto(engine.dt);

    // Desenha a superfície selecionada.
    if (cena == Cena::Esfera) {
      sphere.draw(lightDir, usarLambert);
    } else {
      desenharGaussiana();
    }

    // Desenha e movimenta os raios.
    for (auto &raio : raios) {
      if (mostrarRaios) {
        raio.draw();
      }

      if (!pausado) {
        raio.step(engine.dt);
      }
    }

    // Desenha o plano tangente e a normal.
    desenharEstudo();

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

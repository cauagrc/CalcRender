#ifndef NOMINMAX
#define NOMINMAX
#include <glm/common.hpp>
#include <glm/geometric.hpp>
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

bool sliderPress(double mx, double my);
bool sliderDrag(double mx);
void sliderRelease();

struct Slider {
  float x, y, w, h;
  float minV, maxV;
  float* value;          // variável que este slider controla
  void (*onChange)();    // chamada quando o valor muda
  bool active = false;

  bool inside(double mx, double my) const {
    return mx >= x && mx <= x + w && my >= y && my <= y + h;
  }

  void drag(double mx) {
    float t = glm::clamp(float((mx - x) / w), 0.0f, 1.0f);
    *value = minV + t * (maxV - minV);
    if (onChange) onChange();
  }

  void draw(int winW, int winH) const {
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    glOrtho(0, winW, winH, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);  glPushMatrix(); glLoadIdentity();
    glDisable(GL_DEPTH_TEST);

    glColor3f(0.3f, 0.3f, 0.3f);                       // trilho
    glBegin(GL_QUADS);
    glVertex2f(x, y + h/2 - 2);     glVertex2f(x + w, y + h/2 - 2);
    glVertex2f(x + w, y + h/2 + 2); glVertex2f(x, y + h/2 + 2);
    glEnd();

    float hx = x + (*value - minV) / (maxV - minV) * w;
    glColor3f(1, 1, 1);                                // alça
    glBegin(GL_QUADS);
    glVertex2f(hx - 5, y);     glVertex2f(hx + 5, y);
    glVertex2f(hx + 5, y + h); glVertex2f(hx - 5, y + h);
    glEnd();

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
  }
};
Slider yawSlider{20, 20, 300, 20, -3.14159f, 3.14159f};

// ============================================================
// CAMERA
// Ponto onde o usuario estara vendo a simulacao
// ============================================================
struct Camera {
  float yaw = 0.6f, pitch = 0.4f, dist = 1800.0f;
  vec3 target = vec3(0.0f);

  vec3 position() const {
    // Invertia a direcao do mouse
    /**
     * return target + dist * vec3(cos(pitch)*sin(yaw),
        sin(pitch),
        cos(pitch) * cos(yaw)
        );
     */
    return target + dist * vec3(cos(pitch) * cos(yaw), cos(pitch) * sin(yaw),
                                sin(pitch));
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
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    glfwGetCursorPos(w, &lastX, &lastY);
    if (action == GLFW_PRESS) {
      dragging = !sliderPress(lastX, lastY);   // se clicou no slider, não orbita
    } else {
      sliderRelease();
      dragging = false;
    }
  }
}

void cursorCallback(GLFWwindow*, double x, double y) {
  if (sliderDrag(x)) {
    // o slider cuidou do movimento
  } else if (dragging) {
    camera.yaw   -= float(x - lastX) * 0.005f;
    camera.pitch += float(y - lastY) * 0.005f;
    camera.pitch  = glm::clamp(camera.pitch, -1.5f, 1.5f);
  }
  lastX = x; lastY = y;
}

void scrollCallback(GLFWwindow *, double, double yoff) {
  camera.dist *= (yoff > 0) ? 0.92f : 1.08f;
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

    // troca o fundo preto por cinza.
    // glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    // Limpa o buffer de cor.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // mostra também o que está atrás da esfera.
    // glDisable(GL_DEPTH_TEST);

    float aspect = float(fbw) / fbh;

    // Define a perspectiva da cena.
    mat4 proj = perspective(radians(50.0f), aspect, 5.0f, 10000.0f);

    // aproxima a imagem sem mover a câmera.
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

// Os dois vetores recebidos devem ter tamanho 1. L aponta da superficie para a
// luz.
static float lambert(vec3 normal, vec3 L) {
  return std::max(0.0f, dot(normal, L));
}

// ============================================================
// ILUMINAÇÃO
// Ajusta a claridade e o brilho das superfícies.
// ============================================================

// Claridade nas partes sem luz direta.
const float LUZ_AMBIENTE = 0.15f;

// Força da fonte de luz.
const float INTENSIDADE_LUZ = 1.0f;

// Quanto o material responde à luz difusa.
const float DIFUSA_MATERIAL = 0.82f;

// Força da mancha de brilho. Use 0.0f para removê-la.
const float ESPECULAR_MATERIAL = 0.65f;

// Valores maiores deixam o brilho mais concentrado.
const float BRILHO_MATERIAL = 48.0f;

// Cor branca para a luz.
const vec3 COR_LUZ(1.0f, 1.0f, 1.0f);

// Calcula a cor iluminada de um ponto da superfície.
vec3 calcularIluminacao(vec3 p, vec3 n, vec3 L, float light) {
  // Mantém uma claridade mesmo sem luz direta.
  vec3 ambiente = COR_BASE * COR_LUZ * LUZ_AMBIENTE;

  // remove a claridade das partes escuras.
  // ambiente = vec3(0.0f);

  // Usa Lambert, a força da luz e a resposta do material.
  vec3 difusa =
      COR_BASE * COR_LUZ * (INTENSIDADE_LUZ * DIFUSA_MATERIAL * light);

  // L ja aponta do ponto da superficie para a luz.
  vec3 luz = L;

  // Aponta do ponto da superfície para a câmera.
  vec3 vista = camera.position() - p;
  float distancia = length(vista);

  float especular = 0.0f;

  // Só cria brilho no lado que recebe luz direta.
  if (dot(n, luz) > 0.0f && distancia > 0.0f) {
    vista /= distancia;

    // Encontra a direção entre a luz e a câmera.
    vec3 meio = luz + vista;
    float tamanho = length(meio);

    // Evita dividir por zero se as direções se anularem.
    if (tamanho > 0.0f) {
      meio /= tamanho;

      // Calcula o brilho pelo modelo de Blinn–Phong.
      float alinhamento = std::max(0.0f, dot(n, meio));
      especular = pow(alinhamento, BRILHO_MATERIAL);
    }
  }

  // Usa a cor da luz na mancha de brilho.
  vec3 brilho = COR_LUZ * (INTENSIDADE_LUZ * ESPECULAR_MATERIAL * especular);

  // remove a mancha de brilho.
  // brilho = vec3(0.0f);

  // Soma os efeitos e mantém as cores entre 0 e 1.
  return glm::clamp(ambiente + difusa + brilho, vec3(0.0f), vec3(1.0f));
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

  void draw(vec3 lightPos, bool usarLambert = true) {
    // Define as divisões da esfera.
    int stacks = 40, slices = 80;

    // usa menos divisões na esfera.
    // stacks = 8; slices = 12;

    // mostra somente as linhas dos triângulos.
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
          vec3 n(sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi));

          // Calcula a posição do ponto na esfera.
          vec3 p = position + n * radius;

          // usa a mesma normal em toda a esfera.
          // n = vec3(-1.0f, 0.0f, 0.0f);

          /*
          // Calcula a iluminação neste ponto.
          float light = usarLambert
            ? lambert(n, lightDir)
            : 1.0f;

          // deixa toda a esfera com a mesma cor.
          // light = 1.0f;

          // mantém uma claridade mínima na sombra.
          // light = 0.1f + 0.9f * light;

          // Aplica a iluminação à cor da esfera.
          vec3 ilu(1.0f, 1.0f, 1.0f);
          vec3 c = (light > 0 ? (COR_BASE * (light*ilu)) : COR_BASE * light);

          // representa a direção das normais com cores.
          // c = (n + vec3(1.0f)) * 0.5f;

          glColor3f(c.r, c.g, c.b);
          */

          vec3 L = normalize(lightPos - p);
          float light = usarLambert ? lambert(n, L) : 1.0f;

          // Aplica a iluminação à cor da esfera.
          vec3 c = usarLambert ? calcularIluminacao(p, n, L, light) : COR_BASE;

          // deixa toda a esfera com a mesma cor.
          // c = COR_BASE;

          // mantém uma claridade mínima na sombra.
          // c = COR_BASE * (0.1f + 0.9f * light);

          // representa a direção das normais com cores.
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

    float cauda = glm::clamp(progresso - RASTRO, 0.0f, limite);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDepthMask(GL_FALSE);
    glLineWidth(1.0f);

    // engrossa o rastro dos raios.
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

      // aumenta os pontos de impacto.
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

    // deixa os raios mais rápidos.
    // passo *= 3.0f;

    // congela os raios.
    // passo = 0.0f;

    // Reinicia depois que o rastro desaparece.
    progresso = fmod(progresso + passo, limite + RASTRO + 100.0f);
  }
};

// ============================================================
// OBJETOS GLOBAIS
// ============================================================

vector<Raio> raios;

Engine engine;
Sphere sphere(vec3(0.0f));

// vec3 lightDir = normalize(vec3(1.0f, 0.0f, 0.0f));
vec3 lightPos(-800.0f, 0.0f, 0.0f);
float luzAz = M_PI, luzEl = 0.3f, luzDist = 900.0f;

void calcularLuz() {
  lightPos = luzDist * vec3(cos(luzEl) * cos(luzAz),
                            cos(luzEl) * sin(luzAz),
                            sin(luzEl));
}

enum class Cena { Esfera, Gaussiana };

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
  return ALTURA * exp(-(x * x + y * y) / (ESCALA * ESCALA));
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
  return normalize(vec3(-derivadaX(x, y), -derivadaY(x, y), 1.0f));
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
    vec3 n(cos(pontoB) * cos(pontoA), cos(pontoB) * sin(pontoA), sin(pontoB));

    vec3 u(-sin(pontoA), cos(pontoA), 0.0f);

    return {sphere.position + n * sphere.radius, n, u, normalize(cross(n, u))};
  }

  float x = pontoA * DOMINIO;
  float y = pontoB * DOMINIO;

  return {vec3(x, y, funcao(x, y)), normalFuncao(x, y),
          normalize(vec3(1.0f, 0.0f, derivadaX(x, y))),
          normalize(vec3(0.0f, 1.0f, derivadaY(x, y)))};
}

// Infinito indica que o raio não atingiu a superfície.
float semImpacto() { return numeric_limits<float>::infinity(); }

// Calcula onde o raio atinge a esfera.
float intersecaoEsfera(vec3 origem, vec3 direcao) {
  vec3 p = origem - sphere.position;

  float a = dot(direcao, direcao);

  if (a <= 0.0f) {
    return semImpacto();
  }

  float b = 2.0f * dot(p, direcao);

  float c = dot(p, p) - sphere.radius * sphere.radius;

  float delta = b * b - 4.0f * a * c;

  if (delta < 0.0f) {
    return semImpacto();
  }

  float raiz = sqrt(delta);

  float t1 = (-b - raiz) / (2.0f * a);
  float t2 = (-b + raiz) / (2.0f * a);

  // Escolhe o primeiro impacto à frente do raio.
  float t = t1 >= 0.0f ? t1 : (t2 >= 0.0f ? t2 : semImpacto());

  return t * sqrt(a);
}

float intersecaoGeral(vec3 o, vec3 d) {
  const float passo = 2.0f;
  for (float t = 0.0f; t < 3000.0f; t += passo) {
    vec3 p = o + d * t;
    if (abs(p.x) <= DOMINIO && abs(p.y) <= DOMINIO && p.z <= funcao(p.x, p.y)) {
      return t;
    }
  }
  return semImpacto();
}

// Usa a direção +X dos raios desta cena.
float intersecaoGaussiana(vec3 origem) {
  if (abs(origem.y) > DOMINIO || origem.z <= 0.0f || origem.z > ALTURA) {
    return semImpacto();
  }

  float x2 = -ESCALA * ESCALA * log(origem.z / ALTURA) - origem.y * origem.y;

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
  vector<Raio> antigos = raios;
  raios.clear();

  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      float y = -240.0f + i * 120.0f;

      float z = cena == Cena::Esfera ? -240.0f + j * 120.0f : 30.0f + j * 45.0f;

      vec3 origem = lightPos;
      vec3 alvo(0.0f, y, z);
      vec3 dir = normalize(alvo - origem);

      // Calcula a distância até a superfície.
      float impacto = cena == Cena::Esfera ? intersecaoEsfera(origem, dir)
                                           : intersecaoGeral(origem, dir);

      // deixa os raios seguirem depois da superfície.
      // impacto = semImpacto();

      bool atingiu = std::isfinite(impacto);

      float limite = atingiu ? impacto : 1700.0f;

      // Faz os rastros começarem em posições diferentes.
      float inicio = fmod(float(i * 113 + j * 157), limite + RASTRO + 100.0f);

      int idx = i * 5 + j;
      if (idx < (int)antigos.size()) {
        inicio = antigos[idx].progresso;
      }

      raios.push_back({origem, dir, limite, inicio, atingiu});
    }
  }
}

void atualizarLuz() {
  calcularLuz();
  criarRaios();
}

Slider sliders[3] = {
  {20, 20, 300, 20, -3.14159f, 3.14159f, &luzAz,   atualizarLuz},  // azimute
  {20, 50, 300, 20,  0.05f,    1.5f,     &luzEl,   atualizarLuz},  // elevação
  {20, 80, 300, 20,  300.0f,   1500.0f,  &luzDist, atualizarLuz},  // distância
};

// Devolve true se o clique foi em algum slider.
bool sliderPress(double mx, double my) {
  for (auto &s : sliders)
    if (s.inside(mx, my)) { s.active = true; s.drag(mx); return true; }
  return false;
}

// Devolve true se algum slider está sendo arrastado.
bool sliderDrag(double mx) {
  for (auto &s : sliders)
    if (s.active) { s.drag(mx); return true; }
  return false;
}

void sliderRelease() {
  for (auto &s : sliders) s.active = false;
}

// Troca a superfície e recria os raios.
void selecionarCena(Cena nova) {
  cena = nova;

  pontoA = cena == Cena::Esfera ? -2.8f : -0.35f;
  pontoB = cena == Cena::Esfera ? 0.35f : -0.20f;

  criarRaios();
}

// Processa as teclas pressionadas.
void keyCallback(GLFWwindow *w, int key, int, int action, int) {
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
  float passo = (cena == Cena::Esfera ? 1.0f : 0.6f) * dt;

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

void vertice(vec3 p) { glVertex3f(p.x, p.y, p.z); }

void desenharLuz() {
  glPointSize(12.0f);
  glColor3f(1.0f, 1.0f, 0.3f);
  glBegin(GL_POINTS);
  vertice(lightPos);
  glEnd();
  glPointSize(1.0f);
}

// Desenha a gaussiana usando vários triângulos.
void desenharGaussiana() {
  const int faixas = 48, divisoes = 96;

  // mostra os triângulos da gaussiana.
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  for (int i = 0; i < faixas; i++) {
    glBegin(GL_TRIANGLE_STRIP);

    for (int j = 0; j <= divisoes; j++) {
      for (int k = 0; k < 2; k++) {
        float x = -DOMINIO + 2.0f * DOMINIO * j / divisoes;

        float y = -DOMINIO + 2.0f * DOMINIO * (i + k) / faixas;

        /*
        vec3 n = normalFuncao(x, y);

        float light = usarLambert
          ? lambert(n, lightDir)
          : 1.0f;

        vec3 c = COR_BASE * light;

        // representa as normais da gaussiana com cores.
        // c = (n + vec3(1.0f)) * 0.5f;

        glColor3f(c.r, c.g, c.b);
        vertice(vec3(x, y, funcao(x, y)));
        */

        vec3 n = normalFuncao(x, y);
        vec3 p(x, y, funcao(x, y));

        vec3 L = normalize(lightPos - p);

        float light = usarLambert ? lambert(n, L) : 1.0f;

        vec3 c = usarLambert ? calcularIluminacao(p, n, L, light) : COR_BASE;

        // representa as normais da gaussiana com cores.
        // c = (n + vec3(1.0f)) * 0.5f;

        glColor3f(c.r, c.g, c.b);
        vertice(p);
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

    // aumenta o plano tangente.
    // u *= 2.0f; v *= 2.0f;

    float alpha = 0.35f;

    // tira a transparência do plano.
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

    // aumenta a seta da normal.
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
      sphere.draw(lightPos, usarLambert);
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

    desenharLuz();

    int ww, wh;
    glfwGetWindowSize(engine.window, &ww, &wh);
    for (auto &s : sliders) s.draw(ww, wh);

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

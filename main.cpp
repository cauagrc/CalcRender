// Carrega as funções do OpenGL.
#include <GL/glew.h>
// Permite criar a janela e processar eventos.
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>

struct Engine {
    GLFWwindow* window;
    // Dimensões iniciais da janela, em pixels.
    int WIDTH = 1200;
    int HEIGHT = 720;

    Engine() {
        if (!glfwInit()) {
            std::cerr << "Erro ao inicializar o GLFW.\n";
            std::exit(EXIT_FAILURE);
        }

        // Solicita a versão 3.3 do OpenGL.
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

        // Seleciona o perfil Core, usado no OpenGL moderno.
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Cria a janela.
        // Os dois nullptr indicam:
        // 1. Não usar um monitor em modo de tela cheia.
        // 2. Não compartilhar recursos com outro contexto.
        window = glfwCreateWindow(WIDTH, HEIGHT, "CalcRender", nullptr, nullptr);

        if (window == nullptr) {
            std::cerr << "Erro ao criar a janela.\n";

            // Encerra o GLFW, que já havia sido inicializado.
            glfwTerminate();

            std::exit(EXIT_FAILURE);
        }

        // Seleciona o contexto OpenGL da nossa janela.
        // Os comandos gráficos passam a utilizar esse contexto.
        glfwMakeContextCurrent(window);

        glewExperimental = GL_TRUE;

        const GLenum glewErr = glewInit();

        if (glewErr != GLEW_OK) {
            std::cerr << "Erro ao inicializar o GLEW: "
                      << glewGetErrorString(glewErr)
                      << '\n';

            // Libera a janela e encerra o GLFW antes de sair.
            glfwDestroyWindow(window);
            glfwTerminate();

            // Impede que o programa continue com a janela destruída.
            std::exit(EXIT_FAILURE);
        }

        // Solicita sincronização da apresentação com a tela.
        glfwSwapInterval(1);
    }

    void run() {
        // Cria um cor: cor preta. Para a tela.
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        // Limpa o buffer de cor utilizando a cor definida acima.
        glClear(GL_COLOR_BUFFER_BIT);
    }
} engine;

int main() {

    while (!glfwWindowShouldClose(engine.window)) {
        // Prepara um quadro com o fundo preto.
        engine.run();

        // Inicializa o quadro.
        glfwSwapBuffers(engine.window);

        // Processa os eventos da janela.
        glfwPollEvents();
    }

    glfwDestroyWindow(engine.window);
    glfwTerminate();

    return 0;
}
// Carrega as funções do OpenGL.
#include <GL/glew.h>
// Permite criar a janela e processar eventos.
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>

struct Engine {
    // Ponteiro para a janela.
    GLFWwindow* window;
    // Dimensões iniciais da janela, em pixels.
    int WIDTH = 1200;
    int HEIGHT = 720;

    // Construtor: executado automaticamente quando criamos um Engine.
    Engine() {
        // Os parênteses são necessários para CHAMAR glfwInit().
        // Se a inicialização falhar, encerramos o programa.
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

        // Solicita ao GLEW que tente carregar também funções
        // que não sejam anunciadas da maneira esperada pelo driver.
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
        // Precisa ser chamado depois de selecionar o contexto.
        glfwSwapInterval(1);
    }

    // Prepara o desenho de UM quadro.
    void run() {
        // Define a cor usada na limpeza:
        // vermelho = 0, verde = 0, azul = 0, alfa = 1.
        // O resultado é preto, com alfa totalmente opaco.
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        // Limpa o buffer de cor utilizando a cor definida acima.
        glClear(GL_COLOR_BUFFER_BIT);
    }
} engine;

int main() {

    // Repete enquanto a janela não tiver uma solicitação de fechamento.
    while (!glfwWindowShouldClose(engine.window)) {
        // Prepara um quadro com o fundo preto.
        engine.run();

        // Apresenta o quadro que acabamos de preparar.
        glfwSwapBuffers(engine.window);

        // Processa os eventos da janela, como o clique no botão X.
        glfwPollEvents();
    }

    glfwDestroyWindow(engine.window);
    glfwTerminate();

    return 0;
}
#include <iostream>

#include <glad/gl.h>
#include <SDL3/SDL.h>

const char* vertexShaderSource = R"(
#version 420 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vertexColor;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    vertexColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 420 core

in vec3 vertexColor;

out vec4 FragColor;

void main()
{
    FragColor = vec4(vertexColor, 1.0);
}
)";

GLuint createShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);

        std::cerr << "Shader compilation error:\n"
                    << infoLog << std::endl;
    }

    return shader;
}

GLuint createShaderProgram()
{
    GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if(!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);

        std::cerr << "Program linking error:\n"
                    << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int main(int argc, char *argv[])
{
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL_Init failed: "
                    << SDL_GetError() << std:: endl;
        
        return -1;
    }

    //OpenGL version
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE
    );

    SDL_Window* window = SDL_CreateWindow(
        "SDL3 + OpenGL + GLAD Test",
        1280,
        720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if(!window)
    {
        std::cerr << "Window creation failed: "
                    << SDL_GetError() << std::endl;
        
        SDL_Quit();
        return -1;
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    if(!glContext)
    {
        std::cerr << "OpenGL context creation failed: "
                    << SDL_GetError() << std::endl;
        
        SDL_DestroyWindow(window);
        SDL_Quit();

        return -1;
    }

    if(!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        
        SDL_GL_DestroyContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();

        return -1;
    }

    std::cout << "OpenGL Version: "
                << glGetString(GL_VERSION)
                << std::endl;
    
    GLuint shaderProgram = createShaderProgram();

    float vertices[] =
    {
         0.0f,  0.6f, 0.0f, 1.0f, 0.0f, 0.0f, //Red
        -0.6f, -0.6f, 0.0f, 0.0f, 1.0f, 0.0f, //Green
         0.6f, -0.6f, 0.0f, 0.0f, 0.0f, 1.0f  //Blue
    };

    GLuint VAO;
    GLuint VBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vertices),
        vertices,
        GL_STATIC_DRAW
    );

    // Position attribute

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        (void*)0
    );

    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        (void *)(3 * sizeof(float))
    );

    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    bool running = true;
    
    while(running)
    {
        SDL_Event event;

        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
        }

        int width;
        int height;

        SDL_GetWindowSize(window, &width, &height);

        glViewport(0,0, width, height);

        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glBindVertexArray(VAO);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        SDL_GL_SwapWindow(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glDeleteProgram(shaderProgram);

    SDL_GL_DestroyContext(glContext);

    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
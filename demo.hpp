#pragma once

#include <GLFW/glfw3.h>

class Demo
{
private:
    GLFWwindow *wnd;
    GLuint font_texture;
    GLuint vao;
    GLuint vbo[2];
    GLuint program;
    GLuint elements_to_draw;
    GLfloat total_draw_length;

    void prepare_message();
    GLuint build_shader(const char* vs, const char* fs);

public:
    Demo();     //Init
    ~Demo();    //cleanup

    void run(); //main loop
};

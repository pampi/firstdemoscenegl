#include <cmath>
#include <cstring>
#include <cstdio>
#include <vector>

#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>
#include "demo.hpp"

#include "courier_new_bold_8.inl"

//Why 1024? To edit text easily in any hex editor :)
//Don't forget about null-termination!
volatile static char printable_text[1024] = {
												'L','O', 'R', 'E', 'M', ' ', 'I', 'P', 'S', 'U', 'M', ' ',
												'L','O', 'R', 'E', 'M', ' ', 'I', 'P', 'S', 'U', 'M', ' ',
												0};

//#define VERBOSE

static const char* sz_vertex_shader = "\n"
        "#version 430\n"
        ""
        "layout(location = 0) in vec2 vertPos;\n"
        "layout(location = 1) in vec2 vertexUV;\n"
        "layout(location = 2) uniform float time;\n"
        ""
        "out vec2 UV;\n"
        ""
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(vertPos.x - time, 0.7 * cos(1.1 * time + vertPos.x) + vertPos.y, 0.0, 1.0);\n"
        "    UV = vertexUV;\n"
        "}\n";

static const char* sz_fragment_shader = "\n"
        "#version 430\n"
        ""
        "layout(location = 2) uniform float time;\n"
        "in vec2 UV;\n"
        "out vec4 color;\n"
        "uniform sampler2D myTexSampler;\n"
        ""
        "void main(){\n"
        "    color = vec4(vec3(abs(cos(time * 1.5)), 0.25 * abs(cos(time*5.0)), abs(sin(time*13.0))) * texture(myTexSampler, UV).a, 1.0 - texture(myTexSampler, UV).a);\n"
        "}\n";

Demo::Demo() : elements_to_draw(0)
{
    if (!glfwInit())
    {
        wnd = nullptr;
        return;
    }

    //TODO: query for fullscreen measurements

    //set wnd hint for gl3.3+ Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    wnd = glfwCreateWindow(800, 600, "", nullptr, nullptr);

    if (!wnd)
    {
        glfwTerminate();
    }

    glfwMakeContextCurrent(wnd);
    glfwSwapInterval(1);

    program = build_shader(sz_vertex_shader, sz_fragment_shader);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //upload texture to gpu
    glGenTextures(1, &font_texture);
    glBindTexture(GL_TEXTURE_2D, font_texture);

    glGenBuffers(2, vbo);
    prepare_message();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bit_font.width, bit_font.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bit_font.pixel_data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    //just a good practice :)
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_SRC_ALPHA);
}

Demo::~Demo()
{
    if (!wnd)
    {
        return;
    }

    glUseProgram(0);
    glDeleteProgram(program);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(2, vbo);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);

    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &font_texture);

    glfwMakeContextCurrent(nullptr);
    glfwTerminate();
}

void Demo::run()
{
    constexpr GLfloat TIME_TEXT_SPEED_MULTIPLER = 0.75f;

    glUseProgram(program);
    while (!glfwWindowShouldClose(wnd))
    {
        double dtime = glfwGetTime();
        glClearColor(fabs(sin(dtime * 1.5) ),
                     fabs(sin(dtime)),
                     fabs(cos(dtime)),
                     1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (dtime * TIME_TEXT_SPEED_MULTIPLER > total_draw_length)
        {
            glfwSetTime(0.0);
        }

        //draw
        glUniform1f(2, static_cast<float>(dtime * TIME_TEXT_SPEED_MULTIPLER));
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glVertexAttribPointer(0,
                              2,
                              GL_FLOAT,
                              GL_FALSE,
                              0,
                              nullptr);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glVertexAttribPointer(1,
                              2,
                              GL_FLOAT,
                              GL_FALSE,
                              0,
                              nullptr);
        glBindTexture(GL_TEXTURE_2D, font_texture);

        glDrawArrays(GL_TRIANGLES, 0, 6 * elements_to_draw);
        glDisableVertexAttribArray(0);

        glfwSwapBuffers(wnd);
        glfwPollEvents();

        //Quit demo with pressed escape key!
        if (glfwGetKey(wnd, GLFW_KEY_ESCAPE))
        {
            break;
        }
    }
}

void Demo::prepare_message()
{
    constexpr GLfloat VERTEX_CHAR_SIZE = 0.125f;
    constexpr GLfloat VERTEX_CHAR_WIDTH = 0.0625f;

    std::vector<GLfloat> vertex_buffer;
    std::vector<GLfloat> uv_buffer;
	 size_t length = strlen(const_cast<const char*>(printable_text));

    GLfloat initial_position = 1.0f;

    for (size_t i = 0; i < length; i++)
    {
        int mapped_char_position = 0;
        for (; mapped_char_position < CHAR_COUNT; mapped_char_position++)
        {
            if (char_mapping[mapped_char_position][1] == printable_text[i])
            {
                //we've found a char
                break;
            }
        }

        //if char is not found, then mapped_char_position is equal to CHAR_COUNT
        if (mapped_char_position < CHAR_COUNT)
        {
            //add vertices coords
            vertex_buffer.push_back(initial_position); vertex_buffer.push_back(VERTEX_CHAR_SIZE);   //0
            vertex_buffer.push_back(initial_position + VERTEX_CHAR_WIDTH); vertex_buffer.push_back(VERTEX_CHAR_SIZE);   //1
            vertex_buffer.push_back(initial_position + VERTEX_CHAR_WIDTH); vertex_buffer.push_back(0.0f);   //2
            vertex_buffer.push_back(initial_position); vertex_buffer.push_back(VERTEX_CHAR_SIZE);   //3
            vertex_buffer.push_back(initial_position); vertex_buffer.push_back(0.0f);   //4
            vertex_buffer.push_back(initial_position + VERTEX_CHAR_WIDTH); vertex_buffer.push_back(0.0f);   //5

            //add uv coords
            uv_buffer.push_back(static_cast<GLfloat>(char_mapping[mapped_char_position][2]) / static_cast<GLfloat>(bit_font.width)); uv_buffer.push_back(0.0f);   //0
            uv_buffer.push_back(static_cast<GLfloat>(char_mapping[mapped_char_position][2] + char_mapping[mapped_char_position][0]) / static_cast<GLfloat>(bit_font.width)); uv_buffer.push_back(0.0f);   //1
            uv_buffer.push_back(static_cast<GLfloat>(char_mapping[mapped_char_position][2] + char_mapping[mapped_char_position][0]) / static_cast<GLfloat>(bit_font.width)); uv_buffer.push_back(1.0f);   //2
            uv_buffer.push_back(static_cast<GLfloat>(char_mapping[mapped_char_position][2]) / static_cast<GLfloat>(bit_font.width)); uv_buffer.push_back(0.0f);   //3 - ok
            uv_buffer.push_back(static_cast<GLfloat>(char_mapping[mapped_char_position][2]) / static_cast<GLfloat>(bit_font.width)); uv_buffer.push_back(1.0f);   //4
            uv_buffer.push_back(static_cast<GLfloat>(char_mapping[mapped_char_position][2] + char_mapping[mapped_char_position][0]) / static_cast<GLfloat>(bit_font.width)); uv_buffer.push_back(1.0f);   //5

            elements_to_draw++;
        }
        initial_position += VERTEX_CHAR_SIZE;

    }

    total_draw_length = initial_position + 1.5f;

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertex_buffer.size(), vertex_buffer.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * uv_buffer.size() , uv_buffer.data(), GL_STATIC_DRAW);

    vertex_buffer.clear();
    uv_buffer.clear();
}

GLuint Demo::build_shader(const char *vs, const char *fs)
{
    GLint result=GL_TRUE;
    char szResult[16384];   //max 16k log
    GLint resultLen=0;
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint glProgramId = glCreateProgram();

    if(vs)
    {
#ifdef VERBOSE
        printf("Compiling Vertex Shader: %s\n", vs);
#endif
        glShaderSource(vertexShaderId, 1, &vs, nullptr);
        glCompileShader(vertexShaderId);

        glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &result);
        glGetShaderiv(vertexShaderId, GL_INFO_LOG_LENGTH, &resultLen);

        memset(szResult, 0, 16384);
        glGetShaderInfoLog(vertexShaderId, resultLen, nullptr, szResult);
#ifdef VERBOSE
        printf("GLSL vertex compiler log: %s\n", szResult);
#endif

        if(result==GL_TRUE)
            glAttachShader(glProgramId, vertexShaderId);
    }

    if(fs)
    {
#ifdef VERBOSE
        printf("Compiling Fragment Shader: %s\n", fs);
#endif
        glShaderSource(fragmentShaderId, 1, &fs, nullptr);
        glCompileShader(fragmentShaderId);

        glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &result);
        glGetShaderiv(fragmentShaderId, GL_INFO_LOG_LENGTH, &resultLen);

        memset(szResult, 0, 16384);
        glGetShaderInfoLog(fragmentShaderId, resultLen, nullptr, szResult);
#ifdef VERBOSE
        printf("GLSL fragment compiler log: %s\n", szResult);
#endif
        if(result==GL_TRUE)
            glAttachShader(glProgramId, fragmentShaderId);
    }

    glLinkProgram(glProgramId);

    glGetProgramiv(glProgramId, GL_LINK_STATUS, &result);
    glGetProgramiv(glProgramId, GL_INFO_LOG_LENGTH, &resultLen);

    memset(szResult, 0, 16384);
    glGetProgramInfoLog(glProgramId, result, nullptr, szResult);
#ifdef VERBOSE
    printf("GLSL linking log: %s\n", szResult);
#endif

    if(vs) glDeleteShader(vertexShaderId);
    if(fs) glDeleteShader(fragmentShaderId);

    return glProgramId;
}

//sample quad
/*static const GLfloat vert_buff[] = {
    -1.0f, 0.125f,
    1.0f, 0.125f,
    1.0f, 0.0f,

    -1.0f, 0.125f,
    -1.0f, 0.0f,
    1.0f, 0.0f
};

static const GLfloat uv_buff[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
};*/

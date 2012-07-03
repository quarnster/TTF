#include "TTF.h"
#include <GL/glew.h>
#include <GL/glfw.h>

void key(int k, int state)
{
    if (state)
        return;

    switch (k)
    {
        case GLFW_KEY_ESC:
            glfwCloseWindow();
            break;
        default:
            break;
    }
}


const char *fragCodeSimple = "                                  \n\
varying vec3 tpos;                                              \n\
float round(float val)                                          \n\
{                                                               \n\
    return sign(val)*floor(abs(val)+0.5);                       \n\
}                                                               \n\
void main()                                                     \n\
{                                                               \n\
    float alpha = round((tpos.x*tpos.x-tpos.y)*tpos.z+0.5);     \n\
    gl_FragColor = alpha *vec4(1.0,1.0,1.0,1.0);                \n\
}                                                               \n\
";


const char *fragCode ="                                        \n\
varying vec3 tpos;                                              \n\
void main()                                                     \n\
{                                                               \n\
    float alpha = 1.0;                                          \n\
    if (tpos.z != 0.0)                                          \n\
    {                                                           \n\
        vec2 p = tpos.xy;                                       \n\
        // Gradients                                            \n\
        vec2 px = dFdx(p);                                      \n\
        vec2 py = dFdy(p);                                      \n\
        // Chain rule                                           \n\
        float fx = ((2.0*p.x)*px.x - px.y);                     \n\
        float fy = ((2.0*p.x)*py.x - py.y);                     \n\
        // Signed distance                                      \n\
        float dist = fx*fx + fy*fy;                             \n\
        float sd = (p.x*p.x - p.y)*-tpos.z/sqrt(dist);          \n\
        // Linear alpha                                         \n\
        alpha = clamp(0.5 - sd, 0.0, 1.0);                      \n\
    }                                                           \n\
    gl_FragColor = alpha * vec4(1.0, 1.0, 1.0, 1.0);            \n\
/*                                                              \n\
    if (tpos.z == 1.0)                                          \n\
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);                \n\
    else if (tpos.z == 0.0)                                     \n\
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);                \n\
    else                                                        \n\
        gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);                \n\
*/                                                              \n\
}                                                               \n\
";

const char *vertCode = "                                        \n\
attribute float t;                                              \n\
attribute float c;                                              \n\
attribute vec2 pos;                                             \n\
varying vec3 tpos;                                              \n\
void main(void)                                                 \n\
{                                                               \n\
    tpos = vec3(t*0.5, max(t - 1.0, 0.0), c);                   \n\
    gl_Position = gl_ModelViewProjectionMatrix * vec4(0.001*pos, 0.0, 1.0);\n\
}                                                               \n\
";


int glsl_log(GLuint obj, GLenum check_compile)
{
    if (check_compile == GL_COMPILE_STATUS)
    {
        int len = 0;
        glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &len);
        if(len > 0)
        {
            char *str = (char *) malloc(len * sizeof(char));
            if (str)
            {
                glGetShaderInfoLog(obj, len, NULL, str);
                printf("shader_debug: %s\n", str);
                free(str);
            }
        }
    }
    else
    {
        int len = 0;
        glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &len);
        if(len > 0)
        {
            char *str = (char *)malloc(len * sizeof(char));
            if (str)
            {
                glGetProgramInfoLog(obj, len, NULL, str);
                printf("shader_debug: %s\n", str);
                free(str);
            }
        }
    }
    return 0;
}
GLint compileShader()
{
    GLint frag = glCreateShader(GL_FRAGMENT_SHADER);

    int size = strlen(fragCode);
    glShaderSource(frag, 1, (const char **) &fragCode, &size);
    glCompileShader(frag);
    glsl_log(frag, GL_COMPILE_STATUS);
    GLint vert = glCreateShader(GL_VERTEX_SHADER);
    size = strlen(vertCode);
    glShaderSource(vert, 1, (const char **) &vertCode, &size);
    glCompileShader(vert);
    glsl_log(vert, GL_COMPILE_STATUS);

    GLint ret = glCreateProgram();
    glAttachShader(ret, frag);
    glAttachShader(ret, vert);
    glLinkProgram(ret);
    glsl_log(ret, GL_LINK_STATUS);

    return ret;
}

using namespace Utility::TTF;

int main(int argc, char const *argv[])
{
    glfwInit();
//    glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 8);
    glfwOpenWindow(640, 480, 8, 8, 8, 8, 0, 0, GLFW_WINDOW);
    glViewport(0, 0, 640, 480);
    glewInit();
    glfwSwapInterval(1);
    glfwSetKeyCallback(key);
    Font f("/Library/Fonts/Andale Mono.ttf");
    f.PreCacheBasicLatin();
    const char *msg = "Hello World";
    GLint program = compileShader();
    glUseProgram(program);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1, 1, -1, 1, 1, 100);

    glMatrixMode(GL_MODELVIEW);
    bool first = true;

    while (glfwGetWindowParam(GLFW_OPENED))
    {
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        float scale = 4.0 + 3*cos(glfwGetTime());
        glScalef(scale, scale, 1);
        glTranslatef(-8, 0, -10);

        for (int i = 0; i < strlen(msg); i++)
        {
            CodePoint cp(msg[i]);
            const Mesh &m = f.GetTriangulation(cp);

            if (i > 0)
            {
                Utility::TTFCore::vec2f kerning = f.GetKerning(CodePoint(msg[i-1]), cp);
                glTranslatef(kerning.x*0.001, kerning.y*0.001, 0);
            }


            if (m.verts.size())
            {
                GLint loc = glGetAttribLocation(program, "t");
                glEnableVertexAttribArray(loc);
                glVertexAttribPointer(loc, 1, GL_BYTE, GL_FALSE, sizeof(MeshVertex), &m.verts[0].texCoord);
                loc = glGetAttribLocation(program, "c");
                glEnableVertexAttribArray(loc);
                glVertexAttribPointer(loc, 1, GL_BYTE, GL_FALSE, sizeof(MeshVertex), &m.verts[0].coef);
                loc = glGetAttribLocation(program, "pos");
                glEnableVertexAttribArray(loc);
                glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), &m.verts[0].pos.x);

                glDrawArrays(GL_TRIANGLES, 0, m.verts.size());
            }
            for (int j = 0; j < m.verts.size(); j++)
            {
                const MeshVertex &mv = m.verts[j];
                printf("%c %d, %d: (%f, %f), %d, %d\n", msg[i], j/3, j%3, mv.pos.x, mv.pos.y, mv.texCoord, mv.coef);
            }
        }
        first = false;

        glfwSwapBuffers();
        glfwPollEvents();
    }
    glfwCloseWindow();
    glfwTerminate();

    return 0;
}

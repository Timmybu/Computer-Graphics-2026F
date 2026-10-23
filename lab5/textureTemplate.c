
#ifdef __APPLE__  // include Mac OS X verions of headers
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else // non-Mac OS X operating systems
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>
#endif  // __APPLE__

// Remove these definitions of vec2, vec4 and mat4 and use your library
typedef struct {
    float x;
    float y;
} vec2;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} vec4;

typedef struct {
    vec4 x;
    vec4 y;
    vec4 z;
    vec4 w;
} mat4;


#include "initShader.h"
#include <stdio.h>

int num_vertices = 6;
GLuint ctm_location;
mat4 ctm = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
GLuint model_view_location;
mat4 model_view = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
GLuint projection_location;
mat4 projection = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
int isAnimating = 0;

void init(void)
{
    // Generate a square
    
    vec4 *positions = (vec4 *) malloc(sizeof(vec4) * 6);
    positions[0] = (vec4) { 0.5, -0.5,  0.0,  1.0};
    positions[1] = (vec4) {-0.5,  0.5,  0.0,  1.0};
    positions[2] = (vec4) {-0.5, -0.5,  0.0,  1.0};
    positions[3] = (vec4) { 0.5, -0.5,  0.0,  1.0};
    positions[4] = (vec4) { 0.5,  0.5,  0.0,  1.0};
    positions[5] = (vec4) {-0.5,  0.5,  0.0,  1.0};

    // Texture coordinate of the square
    
    vec2 *tex_coords = (vec2 *) malloc(sizeof(vec2) * 6);
    tex_coords[0] = (vec2) {0.25, 0.25};
    tex_coords[1] = (vec2) {0.00, 0.00};
    tex_coords[2] = (vec2) {0.00, 0.25};
    tex_coords[3] = (vec2) {0.25, 0.25};
    tex_coords[4] = (vec2) {0.25, 0.00};
    tex_coords[5] = (vec2) {0.00, 0.00};

    printf("[textureTemplate] num_vertices: %i\n", num_vertices);

    int tex_width = 64;
    int tex_height = 64;
    GLubyte my_texels[tex_width][tex_height][3];

    FILE *fp = fopen("textures01.raw", "r");
    if(fp != NULL)
	printf("[textureTemplate] Successfully open a texture file.\n");
    fread(my_texels, tex_width * tex_height * 3, 1, fp);
    fclose(fp);

    GLuint program = initShader("vshader_pmct.glsl", "fshader_t.glsl");
    glUseProgram(program);

    GLuint mytex[1];
    glGenTextures(1, mytex);
    glBindTexture(GL_TEXTURE_2D, mytex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, my_texels);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    int param;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &param);

    GLuint vao;
    #ifdef __APPLE__
    glGenVertexArraysAPPLE(1, &vao);
    glBindVertexArrayAPPLE(vao);
    #else
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    #endif

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * num_vertices + sizeof(vec2) * num_vertices, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec4) * num_vertices, positions);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * num_vertices, sizeof(vec2) * num_vertices, tex_coords);

    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *) 0);

    GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
    glEnableVertexAttribArray(vTexCoord);
    glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *) (sizeof(vec4) * num_vertices));

    ctm_location = glGetUniformLocation(program, "ctm");
    printf("[textureTemplate] ctm_location: %i\n", ctm_location);
    model_view_location = glGetUniformLocation(program, "model_view");
    printf("[textureTemplate] model_view_location: %i\n", model_view_location);
    projection_location = glGetUniformLocation(program, "projection");
    printf("[textureTemplate] projection_location: %i\n", projection_location);
    
    GLuint texture_location = glGetUniformLocation(program, "texture");
    printf("[textureTemplate] texture_location: %i\n", texture_location);
    glUniform1i(texture_location, 0);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glDepthRange(1,0);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUniformMatrix4fv(ctm_location, 1, GL_FALSE, (GLfloat *) &ctm);
    glUniformMatrix4fv(model_view_location, 1, GL_FALSE, (GLfloat *) &model_view);
    glUniformMatrix4fv(projection_location, 1, GL_FALSE, (GLfloat *) &projection);
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    
    glutSwapBuffers();
}

void keyboard(unsigned char key, int mousex, int mousey)
{
    if(key == 'q') {
	#ifndef __APPLE__
	exit(0);
	#else
	glutLeaveMainLoop();
	#endif
    }
}

void mouse(int button, int state, int x, int y) {
}

void motion(int x, int y) {
}

void idle(void) {
    if(!isAnimating)
	return;

    glutPostRedisplay();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    glutCreateWindow("Texture Template");
    #ifndef __APPLE__
    glewInit();
    #endif
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);
    glutMainLoop();

    return 0;
}

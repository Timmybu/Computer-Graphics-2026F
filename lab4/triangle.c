#ifdef __APPLE__  // include Mac OS verions of headers
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else             // non-Mac OS operating systems
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>
#endif            // __APPLE__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "initShader.h"
#include "myLib.h"
#include <time.h>


int num_vertices;    // The number of vertices
int first_index;     // The first index to draw

GLuint ctm_location; // The index of the uniform variable named ctm
// ALWAYS initialize a transformation matrix to the identity matrix
mat4 ctm = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};

GLfloat degree = 0.0;     // The current degree
int is_animating = 0;     // Whether the program is currently animating


void init(void) {
    num_vertices = 333;
    vec4 *positions = (vec4*) malloc(sizeof(vec4) * num_vertices);
    vec4 *colors = (vec4*) malloc(sizeof(vec4) * num_vertices);
    mat4 m; 
    mat4 n;
    srand(time(NULL));

    // Origin triangle
    positions[0] = (vec4) { 0.0,  0.5,  0.0, 1.0};  // top
    positions[1] = (vec4) { -.25,  0.0,  0.0, 1.0};	// bottom left
    positions[2] = (vec4) { 0.25, 0.0,  0.0, 1.0};	// bottom right 
    
   
    float r = (float)rand()/(float)(RAND_MAX);
    float g = (float)rand()/(float)(RAND_MAX);
    float b = (float)rand()/(float)(RAND_MAX);
    colors[0] = (vec4) {r, g, b, 1.0};
    colors[1] = (vec4) {r, g, b, 1.0};
    colors[2] = (vec4) {r, g, b, 1.0};		

    
    float r1;
    float g1;
    float b1;
    float r2;
    float g2;
    float b2;
    // triangles in front
    for(int i = 1; i < 6; i++) {
        m = mat_mul(mat_translate(-0.1*(float)i,-0.1*(float)i,0.1*(float)i), mat_scale(1.0-(0.1 * (float) i),1.0-(0.1 * (float) i),1.0-(0.1 * (float) i)));
        n = mat_mul(mat_translate(0.1*(float)i,0.1*(float)i,-0.1*(float)i), mat_scale(1.0+(0.1 * (float) i),1.0+(0.1 * (float) i),1.0+(0.1 * (float) i)));
        float r1 = (float)rand()/(float)(RAND_MAX);
        float g1 = (float)rand()/(float)(RAND_MAX);
        float b1 = (float)rand()/(float)(RAND_MAX);
        float r2 = (float)rand()/(float)(RAND_MAX);
        float g2 = (float)rand()/(float)(RAND_MAX);
        float b2 = (float)rand()/(float)(RAND_MAX);


        // Shrinking triangles
        positions[3*i + 3] = mat_v4_mul(m, positions[0]);
        positions[3*i + 4] = mat_v4_mul(m, positions[1]);
        positions[3*i + 5] = mat_v4_mul(m, positions[2]);

        colors[3*i + 3] = (vec4) {r1, b1, g1, 1.0};
        colors[3*i + 4] = (vec4) {r1, b1, g1, 1.0};
        colors[3*i + 5] = (vec4) {r1, b1, g1, 1.0};

        // growing triangles
        positions[3*i + 18 + 0] = mat_v4_mul(n, positions[0]);
        positions[3*i + 18 + 1] = mat_v4_mul(n, positions[1]);
        positions[3*i + 18 + 2] = mat_v4_mul(n, positions[2]);        
        
        colors[3*i + 18 + 0] = (vec4) {r2, g2, b2, 1.0};
        colors[3*i + 18 + 1] = (vec4) {r2, g2, b2, 1.0};
        colors[3*i + 18 + 2] = (vec4) {r2, g2, b2, 1.0};
    }



    


    GLuint program = initShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    GLuint vao;
#ifdef __APPLE__                                     // include Mac OS verions of headers
    glGenVertexArraysAPPLE(1, &vao);
    glBindVertexArrayAPPLE(vao);
#else                                                // Non-mac operating system
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
#endif                                               // __APPLE__

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER,
		 sizeof(vec4) * num_vertices * 2,    // The number of bytes to allocate
		 NULL,
		 GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER,
		    0,                               // The starting offset of the allocated memory
		    sizeof(vec4) * num_vertices,     // The number of bytes to transfer
		    positions);                      // The array of data to transfer
    glBufferSubData(GL_ARRAY_BUFFER,
		    sizeof(vec4) * num_vertices,     // The starting offset of the allocated memory
		    sizeof(vec4) * num_vertices,     // The number of bytes to transfer
		    colors);                         // The array of data to transfer

    free(positions);                                 // positions is no longer needed
    free(colors);                                    // colors is no longer needed
    
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    if(vPosition == -1)
	printf("[WARNING] Cannot allocate the attribute 'vPosition' or it is not used in a glsl.\n");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition,                 // The index
			  4,                         // The number of components in each data
			  GL_FLOAT,                  // The type of each component
			  GL_FALSE,                  // Whether to normalize
			  0,                         // Number of byte offset between data
			  (GLvoid *) 0);             // The offset of the first data

    GLuint vColor = glGetAttribLocation(program, "vColor");
    if(vColor == -1)
	printf("[WARNING] Cannot allocate the attribute 'vColor' or it is not used in a glsl.\n");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor,                    // The index
			  4,                         // The number of components in each data
			  GL_FLOAT,                  // The type of each component
			  GL_FALSE,                  // Whether to normalize
			  0,                         // Number of byte offset between data
			  (GLvoid *) (sizeof(vec4) * num_vertices)); // The offset of the first data

    ctm_location = glGetUniformLocation(program, "ctm");
    if(ctm_location == -1)                           // Check ctm_location
	printf("[WARNING] Cannot allocate the uniform variable 'ctm' or it is not used in a glsl.\n");
    
    glEnable(GL_DEPTH_TEST);                         // Enable hidden surface removal
    glClearColor(0.0, 0.0, 0.0, 1.0);                // The color to be used for glClear()
    glDepthRange(1,0);                               // Make the near plane maps to 1.0

    glPolygonMode(GL_FRONT, GL_FILL);                // Fill a front face with a solid color
    glPolygonMode(GL_BACK, GL_LINE);                 // Draw a back face with just lines
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT |                    // Clear the color buffer
	    GL_DEPTH_BUFFER_BIT);                    // Clear the depth buffer

    glUniformMatrix4fv(ctm_location,                 // The index of the uniform variable
		       1,                            // The number of components to be sent (1 matrix)
		       GL_FALSE,                     // Do not transpose
		       (GLfloat *) &ctm);             // The data to be sent
    
    glDrawArrays(GL_TRIANGLES,                       // Type of primitive mode to draw
		 first_index,                        // The first index
		 num_vertices);                      // The number of vertices to draw

    glutSwapBuffers();                               // Swap the drawing buffer with the display buffer
}

void keyboard(unsigned char key, int mousex, int mousey) {
    if(key == 'q') {
#ifdef __APPLE__
	exit(0);                // Exit the program
#else
    	glutLeaveMainLoop();    // Terminate the main loop (to exit the program)
#endif
    }
    else if(key == '1') {
	first_index = 0;
	num_vertices = 3;
    }
    else if(key == '2') {
	first_index = 3;
	num_vertices = 6;
    }
    else if(key == '3') {
	first_index = 9;
	num_vertices = 63;
    }
    else if(key == '4') {
	first_index = 0;
    num_vertices = 69;
    }

    glutPostRedisplay();        // Redraw the whole scene
}

void idle(void) {
    if(is_animating) {
	degree += 1.0;
	if(degree == 360)
	    degree = 0;
	GLfloat radian = degree * M_PI / 180.0;   // Convert degree to radian
	ctm.x.x = cos(radian);
	ctm.x.y = sin(radian);
	ctm.y.x = -ctm.x.y;
	ctm.y.y = ctm.x.x;
    }

    glutPostRedisplay();
}

void usage(void) {
    printf("Usage:\n");
    printf(" q - quit\n");
    printf(" f - the front triangle only\n");
    printf(" b - the back triangle only\n");
    printf(" a - both triangles\n");
    printf(" SPACE - start/stop animation\n");
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA |         // Use Red, Green, Blue, and Alpha
			GLUT_DOUBLE |       // Double buffers
			GLUT_DEPTH);        // Allow depth
    glutInitWindowSize(512, 512);           // Width and height of the window in number of pixels
    glutInitWindowPosition(100,100);        // The position from the top-left corner
    glutCreateWindow("Lab 3");           // The name of the window
#ifndef __APPLE__                           // If running on a non-mac OS
    glewInit();
#endif                                      // End if
    init();                                 // Call the init function (user-defined above)
    glutDisplayFunc(display);               // The function to call when redrawing is required
    glutKeyboardFunc(keyboard);             // The function to call when a key is pressed
    glutIdleFunc(idle);                     // The function to call when idle
    usage();                                // Print the usage on the screen
    glutMainLoop();                         // Enter an infinite loop

    return 0;
}

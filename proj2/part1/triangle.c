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
#include "stl.h"
#include <time.h>
#define ROUNDNESS 32
#define SPRING 3
#define SIZE (float)0.5
#define PI 3.14159265359

int num_vertices;    // The number of vertices
int first_index;     // The first index to draw
int sphere_index;   // sphere's last index
int torus_index;
int spring_index;
int stl_index;
char* stl_file = NULL;

GLuint ctm_location; // The index of the uniform variable named ctm
// ALWAYS initialize a transformation matrix to the identity matrix
mat4 ctm = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
mat4 idty = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};

GLfloat degree = 0.0;     // The current degree
int is_animating = 0;     // Whether the program is currently animating

void init(void) {
    num_vertices = 1999990;
    float max, s;  
    vec4* colors = (vec4*) malloc(sizeof(vec4) * num_vertices);
    vec4* positions = (vec4*) malloc(sizeof(vec4) * num_vertices); 
    srand((unsigned)time(NULL));   /* seed once */
    
    int idx = 0;
    // SPHERE, CCW
    vec4 start = {0,0,1,1};
    mat4 r;
    mat4 x;
    const float step = 360.f / ROUNDNESS;

    //top half
    for(int j = 0; j < (ROUNDNESS/4); j++) {
        for(int i = 0; i < ROUNDNESS; i++) {
            vec4 A = mat_v4_mul(mat_mul(mat_rot_y(i*step),mat_rot_x(j*-step)),start);
            vec4 B = mat_v4_mul(mat_mul(mat_rot_y((i+1)*step),mat_rot_x((j)*-step)), start);
            vec4 C = mat_v4_mul(mat_mul(mat_rot_y(i*step),mat_rot_x((j+1)*-step)), start);
            vec4 D = mat_v4_mul(mat_mul(mat_rot_y((i+1)*step),mat_rot_x((j+1)*-step)), start);
            // 0
            positions[idx] = A;
            idx++;
            // 1 
            positions[idx] = B;
            idx++;
            // 2
            positions[idx] = C;
            idx++;
            // 3
            positions[idx] = B;
            idx++;
            // 4 
            positions[idx] = D;
            idx++;
            // 5
            positions[idx] = C;
            idx++;

        }
    }
    // bottom half
    for(int j = 0; j < (ROUNDNESS/4); j++) {
        for(int i = 0; i < ROUNDNESS; i++) {
            vec4 A = mat_v4_mul(mat_mul(mat_rot_y(i*-step),mat_rot_x(j*step)),start);
            vec4 B = mat_v4_mul(mat_mul(mat_rot_y((i+1)*-step),mat_rot_x((j)*step)), start);
            vec4 C = mat_v4_mul(mat_mul(mat_rot_y(i*-step),mat_rot_x((j+1)*step)), start);
            vec4 D = mat_v4_mul(mat_mul(mat_rot_y((i+1)*-step),mat_rot_x((j+1)*step)), start);
            // 0
            positions[idx] = A;
            idx++;
            // 1 
            positions[idx] = B;
            idx++;
            // 2
            positions[idx] = C;
            idx++;
            // 3
            positions[idx] = B;
            idx++;
            // 4 
            positions[idx] = D;
            idx++;
            // 5
            positions[idx] = C;
            idx++;
        }
    }
    sphere_index = idx;

    // TORUS
    start = (vec4) {0,0,0.5,1};
    mat4 T    = mat_translate(0,0,1.0);       
    mat4 next = mat_mul(mat_rot_y(step),T);
    mat4 A,B,C,D;
    for(int j = 0; j < ROUNDNESS; j++) { 
        for(int i = 0; i < ROUNDNESS; i++) {
            vec4 A = mat_v4_mul(mat_mul(mat_rot_y(j*step), mat_mul(T, mat_rot_x((float)i*(-step)))),start);
            vec4 B = mat_v4_mul(mat_mul(mat_rot_y(j*step), mat_mul(next, mat_rot_x((float)i*(-step)))),start);
            vec4 C = mat_v4_mul(mat_mul(mat_rot_y(j*step), mat_mul(T, mat_rot_x(((float)i+1)*(-step)))), start);
            vec4 D = mat_v4_mul(mat_mul(mat_rot_y(j*step), mat_mul(mat_mul(mat_rot_y(step),T), mat_rot_x((i+1)*-step))),start);
            // 0
            positions[idx] = A;
            idx++;
            // 1 
            positions[idx] = B;
            idx++;
            // 2
            positions[idx] = C;
            idx++;
            // 3
            positions[idx] = B;
            idx++;
            // 4
            positions[idx] = D;
            idx++;
            // 5
            positions[idx] = C;
            idx++;
        }
    }
    torus_index = idx;
    
    // SPRING
    start = (vec4) {0,0,0.1,1};
    T = mat_translate(0,0,0.5);
    next = mat_mul(mat_rot_y(step),T); // gotta update values
    const vec4 og = {0,0,0,1};

    for(int j = 0; j < ROUNDNESS * SPRING; j++) {
        for(int i = 0; i < ROUNDNESS; i++) {
            mat4 S = mat_translate(0,0.01*j,0);
            mat4 PS = mat_translate(0,0.01*(j+1),0); // B and D need pre-step up 
            vec4 A = mat_v4_mul(mat_mul(S, mat_mul(mat_rot_y(j*step), mat_mul(T, mat_rot_x((float)i*(-step))))),start);
            vec4 B = mat_v4_mul(mat_mul(PS, mat_mul(mat_rot_y(j*step), mat_mul(next, mat_rot_x((float)i*(-step))))),start);
            vec4 C = mat_v4_mul(mat_mul(S, mat_mul(mat_rot_y(j*step), mat_mul(T, mat_rot_x(((float)i+1)*(-step))))), start);
            vec4 D = mat_v4_mul(mat_mul(PS, mat_mul(mat_rot_y(j*step), mat_mul(mat_mul(mat_rot_y(step),T), mat_rot_x((i+1)*-step)))),start);
            // 0
            positions[idx] = A;
            idx++;
            // 1 
            positions[idx] = B;
            idx++;
            // 2
            positions[idx] = C;
            idx++;
            // 3
            positions[idx] = B;
            idx++;
            // 4
            positions[idx] = D;
            idx++;
            // 5
            positions[idx] = C;
            idx++;
            // cap
            if(j==(ROUNDNESS * SPRING - 1)) {
                vec4 c =  mat_v4_mul(mat_mul(S, mat_mul(mat_rot_y((j+1)*step), mat_mul(T, mat_rot_x(i*-step)))), og);
                // center vertex
                positions[idx] = c;
                idx++;
                // D
                positions[idx] = D;
                idx++;
                // B
                positions[idx] = B;
                idx++;
            }
        }

        for(int i = 0; i < ROUNDNESS; i++) {
            mat4 S = mat_translate(0,-0.01*j,0);
            mat4 PS = mat_translate(0,-0.01*(j-1),0); // B and D need pre-step DOWN 
            vec4 A = mat_v4_mul(mat_mul(S, mat_mul(mat_rot_y(j*-step), mat_mul(T, mat_rot_x((float)i*(-step))))),start);
            vec4 B = mat_v4_mul(mat_mul(PS, mat_mul(mat_rot_y(j*-step), mat_mul(next, mat_rot_x((float)i*(-step))))),start);
            vec4 C = mat_v4_mul(mat_mul(S, mat_mul(mat_rot_y(j*-step), mat_mul(T, mat_rot_x(((float)i+1)*(-step))))), start);
            vec4 D = mat_v4_mul(mat_mul(PS, mat_mul(mat_rot_y(j*-step), mat_mul(mat_mul(mat_rot_y(step),T), mat_rot_x((i+1)*-step)))),start);
            // 0
            positions[idx] = A;
            idx++;
            // 1 
            positions[idx] = B;
            idx++;
            // 2
            positions[idx] = C;
            idx++;
            // 3
            positions[idx] = B;
            idx++;
            // 4
            positions[idx] = D;
            idx++;
            // 5
            positions[idx] = C;
            idx++;
            if(j==(ROUNDNESS * SPRING - 1)) {
                vec4 c =  mat_v4_mul(mat_mul(S, mat_mul(mat_rot_y((j)*-step), mat_mul(T, mat_rot_x((i)*(-step))))), og);
                // center vertex
                positions[idx] = c;
                idx++;
                // D
                positions[idx] = A;
                idx++;
                // B
                positions[idx] = C;
                idx++;
            }
        }
    }
    spring_index = idx;

    if(stl_file != NULL) {
        int stl_count;
        vec4 *stl = stl_read(stl_file, &stl_count, &max);

         // find the bounding box
            vec4 lo = stl[0], hi = stl[0];
            for(int i = 1; i < stl_count; i++) {
                if(stl[i].x < lo.x) lo.x = stl[i].x;
                if(stl[i].y < lo.y) lo.y = stl[i].y;
                if(stl[i].z < lo.z) lo.z = stl[i].z;
                if(stl[i].x > hi.x) hi.x = stl[i].x;
                if(stl[i].y > hi.y) hi.y = stl[i].y;
                if(stl[i].z > hi.z) hi.z = stl[i].z;
            }

            // center of the box
            float cx = (lo.x + hi.x) / 2;
            float cy = (lo.y + hi.y) / 2;
            float cz = (lo.z + hi.z) / 2;

            // half of the longest side
            float half = (hi.x - lo.x) / 2;
            if((hi.y - lo.y) / 2 > half) half = (hi.y - lo.y) / 2;
            if((hi.z - lo.z) / 2 > half) half = (hi.z - lo.z) / 2;

        s = 1.0f / half;
        mat4 fit = mat_mul(mat_scale(s,s,s), mat_translate(-cx,-cy,-cz));

        for(int i = 0; i < stl_count; i++) {
            positions[idx] = mat_v4_mul(fit, stl[i]);
            idx++;
        }

    }
    stl_index = idx;
    num_vertices = idx;


    // COLORS
    for(int i = 0; i < (num_vertices / 3); i++) {
        float r = (float)rand()/(float)(RAND_MAX);
        float g = (float)rand()/(float)(RAND_MAX);
        float b = (float)rand()/(float)(RAND_MAX);
        colors[3 * i + 0] = (vec4) {r, b, g, 1.0};
        colors[3 * i + 1] = (vec4) {r, b, g, 1.0};
        colors[3 * i + 2] = (vec4) {r, b, g, 1.0};
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
    else if(key == '1') { // SPHERE
	    first_index = 0;
	    num_vertices = sphere_index;
    }
    else if(key == '2') { // TORUS
	    first_index = sphere_index;
	    num_vertices = torus_index - sphere_index;
    }
    else if(key == '3') { // SPRING
	    first_index = torus_index;
	    num_vertices = spring_index - torus_index;
    }
    else if(key == '4') {
        first_index = spring_index;
	    num_vertices = stl_index - spring_index;
    }
    else if(key == 'a') {
        float s = 1.02;
        mat4 r = mat_rot_y(4);
        ctm = mat_mul(ctm, r);
    }
    else if(key == 'w') {
        float s = 1.02;
        mat4 r = mat_rot_z(4);
        ctm = mat_mul(ctm, r);
    }
    glutPostRedisplay();        // Redraw the whole scene
}

vec4 prev = {0,0,0,0};
void motion(int x, int y) {
    // convert the 0-512 window into -256-256 with center (0,0)
    float mx = (((float)x - 256.0f)/256.0f);
    float my = ((256.0f - (float)y)/256.0f);
    // mat_v4_mul(mat_mul(mat_rot_y(mx*90),mat_rot_x(my*90)),c);

    // check if cursor is outside the sphere. If so, reset prev and return
    if((pow(mx,2.0f) + pow(my,2.0f)) > 1.0f) {
        prev.w=0;
        return;
    }
    
    // where the mouse is relative to the sphere (mouse pos -> glass ball pos)
    vec4 cur = {mx,my,sqrt(1-pow(mx,2)-pow(my,2)),1};

    // check if prev is valid, if not, set prev to cur and return
    if(prev.w==0) {
        prev = cur;
        return;
    }
   
    // CALCULATE rotation matrix
    // R = Rx(−θx) Ry(θy) Rz(θ) Ry(−θy) Rx(θx)
    vec4 u = v4_cross(prev, cur);   // "u is the cross prodcut of two vectors"
    vec4 v = v4_norm(u);            // normalized
    float d = sqrt(pow(v.y,2) + pow(v.z,2));   
    float conv = 180.0f / PI;
    float cost = v4_dot(prev,cur) - 1;

    float th_x = acos(v.z/d) * conv;
        if(v.y < 0) {th_x = -th_x;}
    float th_y = asin(v.x) * conv;
    float th = acos(cost) * conv;

    mat4 r = mat_mul(mat_rot_y(-th_y), mat_rot_x(th_x));
    r = mat_mul(mat_rot_z(th), r);
    r = mat_mul(mat_rot_y(th_y), r);
    r = mat_mul(mat_rot_x(-th_x), r);


    ctm = mat_mul(r, ctm);
    prev = cur;
}

void mouse(int button, int state, int x, int y) {
    float s;
    if(button == 0) {
        prev.w=0;
    }
    if(button == 4) { // UP - ZOOM IN
        s = 1.02;
        ctm = mat_mul(ctm, mat_scale(s,s,s));
    }
    else if(button == 3) { // DOWN - ZOOM OUT
        float s = 1/1.02;
        ctm = mat_mul(ctm, mat_scale(s,s,s));
    }


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
    if(argc > 1) stl_file = argv[1];
    glutInitDisplayMode(GLUT_RGBA |         // Use Red, Green, Blue, and Alpha
			GLUT_DOUBLE |       // Double buffers
			GLUT_DEPTH);        // Allow depth
    glutInitWindowSize(512, 512);           // Width and height of the window in number of pixels
    glutInitWindowPosition(100,100);        // The position from the top-left corner
    glutCreateWindow("Triangle");           // The name of the window
#ifndef __APPLE__                           // If running on a non-mac OS
    glewInit();
#endif                                      // End if
    init();                                 // Call the init function (user-defined above)
    glutDisplayFunc(display);               // The function to call when redrawing is required
    glutKeyboardFunc(keyboard);             // The function to call when a key is pressed
    glutMouseFunc(mouse);

    glutMotionFunc(motion);
    glutIdleFunc(idle);                     // The function to call when idle
    usage();                                // Print the usage on the screen
    glutMainLoop();                         // Enter an infinite loop

    return 0;
}

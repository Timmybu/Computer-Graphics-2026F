# Check the kernel name
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Darwin)

OPTIONS = -framework GLUT -framework OpenGL
DEFINES = -D GL_SILENCE_DEPRECATION

triangle: triangle.c initShader.o myLib.o stl.o
	gcc -Wall -o triangle triangle.c initShader.o myLib.o stl.o $(OPTIONS) $(DEFINES)

initShader.o: initShader.c initShader.h
	gcc -c initShader.c $(DEFINES)

else

LIBS = -lXi -lXmu -lglut -lGLEW -lGLU -lm -lGL

triangle: triangle.c initShader.o myLib.o stl.o
	gcc -o triangle triangle.c initShader.o myLib.o stl.o $(LIBS)

initShader.o: initShader.c initShader.h
	gcc -c initShader.c

endif

myLib.o: myLib.c myLib.h
	gcc -c myLib.c

stl.o: stl.c stl.h myLib.h
	gcc -c stl.c

clean:
	rm -f *.o triangle
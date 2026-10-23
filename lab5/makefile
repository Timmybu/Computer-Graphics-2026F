# Check the kernel name
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Darwin)

OPTIONS = -framework GLUT -framework OpenGL
DEFINES = -D GL_SILENCE_DEPRECATION

lab05: lab05.c initShader.o myLib.o
	gcc -Wall -o lab05 lab05.c initShader.o myLib.o $(OPTIONS) $(DEFINES)

initShader.o: initShader.c initShader.h
	gcc -c initShader.c $(DEFINES)

else

LIBS = -lXi -lXmu -lglut -lGLEW -lGLU -lm -lGL

lab05: lab05.c initShader.o myLib.o
	gcc -Wall -o lab05 lab05.c initShader.o myLib.o $(LIBS)

initShader.o: initShader.c initShader.h
	gcc -c initShader.c

endif

myLib.o: myLib.c myLib.h
	gcc -c myLib.c

clean:
	rm -f *.o lab05

.PHONY: clean

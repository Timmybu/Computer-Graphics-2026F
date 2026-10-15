#version 120

attribute vec4 vPosition;   // Attribute variable named vPosition (value assigned from GPU memory)
attribute vec4 vColor;      // Attribute variable named vColor (value assigned from GPU memory)

varying vec4 color;         // Varing variable that connects to a fragment shader

uniform mat4 ctm;           // Uniform variable named ctm (current transformation matrix)

void main()
{
    color = vColor;                   // Send the unmodified color to a fragment shader
    gl_Position = ctm * vPosition;    // Modify vPosition by ctm and use the result as the final position
}

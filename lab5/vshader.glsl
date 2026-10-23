#version 120

attribute vec4 vPosition;   // Attribute variable named vPosition (value assigned from GPU memory)
attribute vec2 vTexCoord;      // Attribute variable named vColor (value assigned from GPU memory)

varying vec2 texCoord;         // Varing variable that connects to a fragment shader

uniform mat4 ctm;           // Uniform variable named ctm (current transformation matrix)

void main()
{
    texCoord = vTexCoord;                   // Send the unmodified color to a fragment shader
    gl_Position = ctm * vPosition;    // Modify vPosition by ctm and use the result as the final position
}

#version 120

attribute vec4 vPosition;
attribute vec4 vColor;
attribute vec4 vNormal;

varying vec4 color;

uniform mat4 ctm;

vec4 light_position = vec4(0, 0, 2, 1);

void main()
{
    vec4 L = normalize(light_position);
    vec4 N = normalize(ctm * vNormal);
    color = max(dot(L, N), 0.0) * vec4(0.8, 0.8, 0.8, 1.0);
    gl_Position = ctm * vPosition;
}

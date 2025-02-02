#version 410 core
layout(location=0) in vec3 aPosition;
uniform mat4 lightSpaceTrMatrix;
uniform mat4 model;
out vec4 fragPosLightSpace;


void main()
{

gl_Position = lightSpaceTrMatrix * model * vec4(aPosition, 1.0f);

}
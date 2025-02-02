#version 410 core

layout(location = 0) in vec3 aPosition; // Vertex position
layout(location = 1) in vec3 aNormal;   // Vertex normal
layout(location = 2) in vec2 aTexCoords; // Texture coordinates
layout(location = 3) in vec3 aColor;

out vec3 fPosition;
out vec3 fNormal;
out vec2 fTexCoords;
out vec4 fragPosLightSpace;
out vec3 fragColor;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;


//For the firework
uniform bool isFirework;
uniform vec3 particleColor; // Color for fireworks


void main() 
{
    // Compute vertex position in world space
    vec4 worldPosition = model * vec4(aPosition, 1.0);
    fPosition = worldPosition.xyz;

    // Transform vertex position to light's space for shadow mapping
    fragPosLightSpace = lightSpaceMatrix * worldPosition;

    // Pass other attributes to the fragment shader
    fNormal = mat3(transpose(inverse(model))) * aNormal; // Correct normal transformation
    fTexCoords = aTexCoords;

    //Pass firework color or default to white
    fragColor = isFirework ? particleColor : vec3(1.0); 

    // Transform vertex position for rendering
    gl_Position = projection * view * worldPosition;
}

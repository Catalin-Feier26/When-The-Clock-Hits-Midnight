#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec3 fragColor; // color for fireworks

out vec4 fColor;

// Matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;

// Lighting
uniform vec3 viewPos;

//Uniforms for the image filters.
uniform bool isBlackAndWhite;
uniform bool isInverted;
uniform bool isSepia;
uniform bool isCartoon;
uniform float hueShift; //
uniform bool isHueShift;
uniform bool isNightVision;
uniform bool isOldTV;
uniform bool isPixelated;
uniform vec2 pixelSize; // Size of each pixel block

//fireworks
uniform bool isFirework;

// Fog
uniform bool fogToggle;

// Directional light
struct DirectionalLight {
    vec3 direction;
    vec3 color;
    bool enabled;
};
uniform DirectionalLight dirLight;

// Point light
struct PointLight {
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
    bool enabled;
};
uniform PointLight pointLight;
uniform PointLight lamp1;
uniform PointLight lamp2;
uniform PointLight dogLamp;

// Textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

// Shadow map
uniform mat4 lightSpaceMatrix;
uniform sampler2D shadowMap;
in vec4 fragPosLightSpace;


// Components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

void computeDirLight(vec3 normal, vec3 viewDir) {
    if (!dirLight.enabled) return;

    vec3 lightDirN = normalize(view * vec4(dirLight.direction, 0.0f)).xyz;

    // Ambient
    ambient += ambientStrength * dirLight.color;

    // Diffuse
    float diff = max(dot(normal, lightDirN), 0.0f);
    diffuse += diff * dirLight.color;

    // Specular
    vec3 reflectDir = reflect(-lightDirN, normal);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular += specularStrength * specCoeff * dirLight.color;
}

void computePointLight(PointLight pointLight, vec3 fragPos, vec3 normal, vec3 viewDir) {
    if (!pointLight.enabled) return;

    vec3 lightPosEye = (view * vec4(pointLight.position, 1.0f)).xyz;
    vec3 lightDir = normalize(lightPosEye - fragPos);
    float distance = length(lightPosEye - fragPos);

    // Attenuation
    float attenuation = 1.0 / (pointLight.constant + pointLight.linear * distance + pointLight.quadratic * (distance * distance));

    // Ambient
    ambient += ambientStrength * pointLight.color * attenuation;

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0f);
    diffuse += diff * pointLight.color * attenuation;

    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular += specularStrength * specCoeff * pointLight.color * attenuation;
}

float computeShadowFactor(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w; // Perspective divide
    projCoords = projCoords * 0.5 + 0.5; // Transform to [0, 1]

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    // Bias to prevent shadow acne
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.0005);
    return currentDepth > closestDepth + bias ? 1.0 : 0.0;
}

vec3 shiftHue(vec3 color, float angle) {
    float cosA = cos(radians(angle));
    float sinA = sin(radians(angle));
    mat3 hueRotation = mat3(
        vec3(0.299 + 0.701 * cosA + 0.168 * sinA, 0.587 - 0.587 * cosA + 0.330 * sinA, 0.114 - 0.114 * cosA - 0.497 * sinA),
        vec3(0.299 - 0.299 * cosA - 0.328 * sinA, 0.587 + 0.413 * cosA + 0.035 * sinA, 0.114 - 0.114 * cosA + 0.292 * sinA),
        vec3(0.299 - 0.300 * cosA + 1.250 * sinA, 0.587 - 0.588 * cosA - 1.050 * sinA, 0.114 + 0.886 * cosA - 0.203 * sinA)
    );
    return clamp(hueRotation * color, 0.0, 1.0);
}

void main() {
    vec4 fragPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);
    vec3 viewDir = normalize(-fragPosEye.xyz);

    // Reset lighting components
    ambient = vec3(0.0f);
    diffuse = vec3(0.0f);
    specular = vec3(0.0f);

    //compute fog
    float fogDensity = 0.02f;
    float fragmentDistance = length(fragPosEye.xyz);
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2.0));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    vec4 fogColor = vec4(1.0, 0.682, 0.988, 1.0);


    // Compute lighting
    computeDirLight(normalEye, viewDir);
    computePointLight(pointLight, fragPosEye.xyz, normalEye, viewDir);
    computePointLight(lamp1, fragPosEye.xyz, normalEye, viewDir);
    computePointLight(lamp2, fragPosEye.xyz, normalEye, viewDir);
    computePointLight(dogLamp, fragPosEye.xyz, normalEye, viewDir);

   float shadow = computeShadowFactor(fragPosLightSpace, normalEye, -dirLight.direction);


    diffuse *= (1.0 - shadow);
    specular *= (1.0 - shadow);

      // Combine lighting components
    vec3 lighting = ambient + diffuse + specular;

    // Apply textures
    vec3 texColor = texture(diffuseTexture, fTexCoords).rgb;
    vec3 finalColor = lighting * texColor;

    //Fireworks
    if(isFirework){
		finalColor = fragColor;
	}

    // Apply fog if enabled
    if (fogToggle) {
        finalColor = mix(fogColor.rgb, finalColor, fogFactor);
    }

    //Apply black and white filter if enabled
    if (isBlackAndWhite) {
        float luminance = 0.299 * finalColor.r + 0.587 * finalColor.g + 0.114 * finalColor.b;
        finalColor = vec3(luminance); // Set RGB to luminance
    }
    // Apply the inverted filter if enabled
    if (isInverted) {
        finalColor = vec3(1.0) - finalColor; // Invert the color
    }
    // Apply the sepia filter if enabled
    vec3 sepiaColor = vec3(
    finalColor.r * 0.393 + finalColor.g * 0.769 + finalColor.b * 0.189,
    finalColor.r * 0.349 + finalColor.g * 0.686 + finalColor.b * 0.168,
    finalColor.r * 0.272 + finalColor.g * 0.534 + finalColor.b * 0.131);
    if (isSepia) {
	    finalColor = sepiaColor*1.2; // Set the color to sepia
    }

    if (isCartoon) {
        
        float colorLevels = 10.0;
        finalColor = floor(finalColor * colorLevels) / colorLevels;

        // Calculate light direction in view space
        vec3 lightDirN = normalize((view * vec4(dirLight.direction, 0.0)).xyz);

        // Quantize lighting for stylized shading
        float lightLevels = 10.0;
        float diffuseIntensity = max(dot(normalEye, lightDirN), 0.0);
        diffuseIntensity = floor(diffuseIntensity * lightLevels) / lightLevels;

        // Combine ambient and quantized diffuse lighting
        vec3 ambientLight = vec3(0.2); // Boost ambient lighting slightly
        vec3 cartoonLighting = ambientLight + diffuseIntensity * dirLight.color;

        // Apply lighting to quantized color
        finalColor *= cartoonLighting;

    }

    if(isHueShift){
        if (hueShift != 0.0) {
            finalColor = shiftHue(finalColor, hueShift);
        }

    }
    if (isNightVision) {
        float luminance = dot(finalColor, vec3(0.299, 0.587, 0.114));
        vec3 greenTint = vec3(0.15, 0.8, 0.15);
        float noise = fract(sin(dot(fTexCoords.xy, vec2(12.9898, 78.233))) * 43758.5453);
        finalColor = greenTint * (luminance + noise * 0.1) * 2.0;
    }
    if (isOldTV) {
        vec2 center = vec2(0.5, 0.5); // Screen center in normalized coordinates
        vec2 texCoords = fTexCoords - center;

        // Distortion effect
        float distortion = 0.1; // Adjust for more or less warping
        texCoords += texCoords * distortion * dot(texCoords, texCoords);
        vec3 distortedColor = texture(diffuseTexture, texCoords + center).rgb;

        // Chromatic aberration effect
        vec2 offset = vec2(0.032, 0.032); // Adjust for more or less distortion
        vec3 redChannel = texture(diffuseTexture, fTexCoords + offset).rgb;
        vec3 greenChannel = texture(diffuseTexture, fTexCoords).rgb;
        vec3 blueChannel = texture(diffuseTexture, fTexCoords - offset).rgb;

        // Combine chromatic aberration and distortion
        vec3 combinedColor = vec3(redChannel.r, greenChannel.g, blueChannel.b) * distortedColor;

        // Scanline effect
        float scanline = sin(fTexCoords.y * 500.0) * 0.2; // Adjust frequency
        combinedColor *= 1.0 - abs(scanline); // Darken based on scanline position

        // Noise effect
        float noise = fract(sin(dot(fTexCoords.xy, vec2(12.9898, 78.233))) * 43758.5453);
        combinedColor += noise * 0.1; // Add subtle noise
        combinedColor = clamp(combinedColor, 0.0, 1.0); // Ensure valid color values

        finalColor = combinedColor;
    }

    if (isPixelated) {
        vec2 blockCoords = floor(fTexCoords / pixelSize) * pixelSize; // Snap to nearest pixel block
        finalColor = texture(diffuseTexture, blockCoords).rgb; // Sample the texture
    }

    // Output the final color
    fColor = vec4(finalColor, 1.0);

}
#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in int aTextureIndex;

out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;
flat out int TextureIndex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

void main() {
    TexCoord = vec2(aTexCoord.x, 1.0 - aTexCoord.y);
    TextureIndex = aTextureIndex;
    Normal = aNormal;
    vec3 modifiedPos = aPos;
    
    // Leaf texture animation
    if (TextureIndex == 4) {
        float windStrength = 0.1; 
        float windSpeed = 1.5;    
        float waveX = sin(time * windSpeed + aPos.x * 0.5 + aPos.z * 0.3) * windStrength;
        float waveZ = cos(time * windSpeed * 0.7 + aPos.x * 0.3 + aPos.z * 0.5) * windStrength * 0.8;
        
        modifiedPos.x += waveX;
        modifiedPos.z += waveZ;
        
        modifiedPos.y += (waveX + waveZ) * 0.2;
    }
    
    FragPos = vec3(model * vec4(modifiedPos, 1.0));
    gl_Position = projection * view * model * vec4(modifiedPos, 1.0);
}

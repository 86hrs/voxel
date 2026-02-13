#version 410 core
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
flat in int TextureIndex;

out vec4 FragColor;

uniform sampler2D textures[7];
uniform float time;

uniform vec3 lightDir =
    normalize(vec3(-0.5, -1.0, -0.5));           // Directional light (sun)
uniform vec3 lightColor = vec3(1.0, 0.98, 0.95); // Slightly yellowish light
uniform float ambientStrength = 0.3;             // Ambient light intensity

void main() {
    vec4 texColor = texture(textures[TextureIndex], TexCoord);
    if (TextureIndex == 0) { // Grass Top
        texColor.rgb *= vec3(0.61, 0.8, 0.42);
    } else if (TextureIndex == 4) { // Leaf
        texColor.rgb *= vec3(0.61, 0.8, 0.42);
    }

    // vec3 norm = normalize(Normal);
    vec3 norm = Normal;
    float diff = max(dot(norm, -lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    vec3 ambient = ambientStrength * lightColor;
    vec3 result = (ambient + diffuse) * texColor.rgb;

    FragColor = vec4(result, texColor.a);
}

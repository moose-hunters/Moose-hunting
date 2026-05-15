#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D texture_diffuse0;
uniform vec4  baseColorFactor;
uniform bool  hasTexture;

uniform vec3  lightPos;
uniform vec3  lightColor;
uniform vec3  viewPos;
uniform float ambientStrength;

void main() {
    // Базовый цвет
    vec4 texColor = hasTexture
        ? texture(texture_diffuse0, TexCoords) * baseColorFactor
        : baseColorFactor;
    
    if(texColor.a < 0.1) {
        discard; // Если пиксель прозрачный - просто выкидываем его, не пишем в буфер!
    }

    // Нормали
    vec3 norm    = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    // Ambient
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    float diff   = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular (Blinn-Phong)
    vec3 viewDir    = normalize(viewPos - FragPos);
    vec3 halfDir    = normalize(lightDir + viewDir);
    float spec      = pow(max(dot(norm, halfDir), 0.0), 32.0);
    vec3 specular   = 0.3 * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * texColor.rgb;
    FragColor = vec4(result, texColor.a);

    // Гамма-коррекция
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/2.2));
}
#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D texture_diffuse;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

out vec4 FragColor;

void main() {
    // Получаем цвет из текстуры
    vec4 texColor = texture(texture_diffuse, TexCoord);
    
    // Если текстура не загрузилась (черная), показываем UV для отладки
    if (texColor.rgb == vec3(0.0)) {
        // Показываем UV координаты цветом
        FragColor = vec4(TexCoord.x, TexCoord.y, 0.5, 1.0);
        return;
    }
    
    // Освещение
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;
    
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    vec3 result = (ambient + diffuse + specular) * texColor.rgb;
    FragColor = vec4(result, 1.0);
}
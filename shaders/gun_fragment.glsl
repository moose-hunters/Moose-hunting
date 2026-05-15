#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D gunTexture;
void main() {
    vec4 texColor = texture(gunTexture, TexCoords);
    if(texColor.a < 0.1) discard; // На всякий случай отсекаем пустые пиксели
    FragColor = texColor;
}
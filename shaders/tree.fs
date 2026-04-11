#version 330 core
out vec4 FragColor;
in vec2 UV;
uniform sampler2D tex;
void main() {
    vec4 color = texture(tex, UV);
    if(color.a < 0.1) discard; // Убираем прозрачные пиксели
    FragColor = color;
}
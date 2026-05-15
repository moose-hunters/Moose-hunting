#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model; // Добавили матрицу

void main() {
    // Умножаем позицию на матрицу
    gl_Position = model * vec4(aPos, 0.0, 1.0);
    TexCoords = aTexCoords;
}
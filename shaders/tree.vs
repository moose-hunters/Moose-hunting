#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
out vec2 UV;
uniform mat4 mvp; // Projection * View * Model
void main() {
    UV = aTex;
    gl_Position = mvp * vec4(aPos, 1.0);
}
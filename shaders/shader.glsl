// This will be replaced with the backend-specific layout
layout(std140, binding = 0) uniform Matrices { mat4 MVP; };

layout(location = 0) in vec3 position;

void main() {
    gl_Position = MVP * vec4(position, 1.0);
}

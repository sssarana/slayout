// This will be replaced with the backend-specific layout
cbuffer Matrices : register(b0) { float4x4 MVP; };

layout(location = 0) in vec3 position;

void main() {
    gl_Position = MVP * vec4(position, 1.0);
}

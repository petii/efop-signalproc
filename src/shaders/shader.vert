#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    //gl_Position = vec4(inPosition,1.0);
    fragColor = vec3(
        //rgba
        1.0 * (inPosition.z - 1),
        //0.0, 
        0.75*(1.0 - (abs(inPosition.z - 0.5))),
        1.0 * (1-inPosition.z)
    ); //inColor;
}

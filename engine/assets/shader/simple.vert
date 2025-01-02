#version 450
layout (location = 0) in vec3 iPos;

layout(set = 0, binding = 0) uniform ColorData {
    vec4 uColor; // 从 uniform buffer 获取颜色
};
layout (location = 0) out vec4 vColor;

void main()
{
    gl_Position = vec4(iPos, 1.0);
    vColor = uColor; // 使用 uniform 颜色
}

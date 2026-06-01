#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float gamma;
uniform bool applyTonemap;

vec3 acesTonemap(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    vec3 color = texture(screenTexture, TexCoord).rgb;

    if (applyTonemap) {
        color = acesTonemap(color);
    }

    color = pow(color, vec3(1.0 / gamma));
    FragColor = vec4(color, 1.0);
}

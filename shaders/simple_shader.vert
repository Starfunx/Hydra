#version 450


vec2 positions[3] = {
    vec2(0.0, -0.5),
    vec2(0.5,  0.5),
    vec2(-0.5, 0.5)
}

void main() {
    glPosition = vec4 (positions[glVertexIndex], 0.0, 1.0);
}
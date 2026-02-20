#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2DArray image;
uniform int frameNum;

float getLuminance(vec3 linearColor) {
    return dot(linearColor, vec3(0.2126, 0.7152, 0.0722));
}

float getContrast(vec3 a, vec3 b) {
    float L_a = getLuminance(a);
    float L_b = getLuminance(b);

    float L_lighter = max(L_a, L_b);
    float L_darker = min(L_a, L_b);

    return (L_lighter + 0.05) / (L_darker + 0.05);
}

float getAverageContrast(vec2 center, float STEP) {
    const int offsetNum = 8;

    ivec2 size = textureSize(image, 0).xy;
    vec2 step = 1.0 / vec2(size);
    vec2 offsets[offsetNum] = { 
        vec2(-step.x, 0.0),
        vec2(0.0, -step.y),
        vec2(+step.x, 0.0),
        vec2(0.0, +step.y),
        vec2(+step.x, +step.y),
        vec2(-step.x, -step.y),
        vec2(+step.x, -step.y),
        vec2(-step.x, +step.y)
    };

    float totalContrast = 0.0;
    for (int i = 0; i < offsetNum; ++i) {
        vec3 a = texture(image, vec3(center, frameNum)).rgb;
        vec3 b = texture(image, vec3(center + offsets[i] * STEP, frameNum)).rgb;

        totalContrast += getContrast(a, b);
    }

    return totalContrast / float(offsetNum);
}

void main() {
    float STEP = 1.0f;

    const float THRESHOLD_PERC = 5.0;

    const float MAX_CONTRAST = getContrast(vec3(1.0), vec3(0.0));
    
    float contrast = getAverageContrast(TexCoords, STEP) / MAX_CONTRAST;

    vec3 textureColor = texture(image, vec3(TexCoords, frameNum)).rgb;

    // vec3 finalColor = mix(textureColor, vec3(contrast), 1.0);
    vec3 finalColor = textureColor;

    if (contrast > THRESHOLD_PERC / 100.0f) {
        finalColor = vec3(1.0, 0.0, 0.0);
    }

    FragColor = vec4(finalColor, 1.0f);
}

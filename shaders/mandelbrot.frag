#version 330 core

uniform int   width;
uniform int   height;
uniform vec2  offset;
uniform float scale;
uniform int   theme;
uniform float time;

const int   MAX_ITER   = 1024;
const float MAX_RADIUS = 4.0;

const int   MAX_COLOR_THEME_INDEX = 6;

vec3 colorScheme(float t) {
    if (theme % MAX_COLOR_THEME_INDEX == 0) {

        float wave1 = sin(t * 12.0  + time * 2.0) * 0.5 + 0.5;
        float wave2 = cos(t * 12.0  + time * 3.0) * 0.3 + 0.34;
        float wave3 = sin(t * 120.0 - time * 4.0) * 0.4 + 0.6;
        
        vec3 deepBlue     = vec3( 0.9, -0.2, 0.8);
        vec3 electricBlue = vec3(-0.1, -0.5, 1.0);
        vec3 neonPink     = vec3(-2.0, -0.1, 0.7);
        
        vec3 color = mix(deepBlue, electricBlue, wave1);

        color  = mix(color, neonPink, wave2 * pow(t, 9.0));
        color *= wave3 * 0.8 + 0.2;
        
        return color * (1.0 + t * 2.0);

    } else if (theme % MAX_COLOR_THEME_INDEX == 1) {

        float wave1 = sin(t * 12.0  + time * 2.0) * 0.5 + 0.5;
        float wave2 = cos(t * 12.0  + time * 3.0) * 0.3 + 0.34;
        float wave3 = sin(t * 120.0 - time * 4.0) * 0.4 + 0.6;

        vec3 darkRed    = vec3(0.1, 0.0, 0.0);
        vec3 brightRed  = vec3(1.0, 0.2, 0.2);

        float mixFactor = smoothstep(0.3, 0.7, wave1);

        vec3 color = mix(darkRed, brightRed, mixFactor);

        float lineIntensity = sin(t * 50.0 + time * 5.0) * 0.5 + 0.5;

        color += vec3(0.3, 0.0, 0.0) * lineIntensity * pow(t, 2.0);

        vec3 orangeShift = vec3(0.1, 0.05, 0.0);
        color = mix(color, color + orangeShift, wave2 * pow(t, 9.0));

        color *= wave3 * 0.8 + 0.2;

        return color * (1.0 + t * 2.0);

    } else if (theme % MAX_COLOR_THEME_INDEX == 2) {

        float wave1 = sin(t * 12.0  + time * 2.0) * 0.5 + 0.5;
        float wave2 = cos(t * 12.0  + time * 3.0) * 0.3 + 0.34;
        float wave3 = sin(t * 120.0 - time * 4.0) * 0.4 + 0.6;

        vec3 darkGold  = vec3(0.6, 0.4, 0.0);
        vec3 lightGold = vec3(1.0, 0.8, 0.0);

        vec3 color = mix(darkGold, lightGold, wave1);

        vec3 yellowShift = vec3(0.1, 0.1, -0.1);

        color  = mix(color, color + yellowShift, wave2 * pow(t, 9.0));
        color *= wave3 * 0.8 + 0.2;

        return color * (1.0 + t * 2.0);

    } else if (theme % MAX_COLOR_THEME_INDEX == 3) {

        float wave1      = sin(t * 12.0 + time * 2.0) * 0.1;
        float wave2      = cos(t * 12.0 + time * 3.0) * 0.05;
        float modified_t = t + wave1 + wave2;

        float frequency  = 5.0;
        float phaseShift = time * 0.5;

        vec3 color;
        
        color.r = 0.5 + 0.5 * sin(frequency * modified_t + phaseShift + 0.0);
        color.g = 0.5 + 0.5 * sin(frequency * modified_t + phaseShift + 2.0);
        color.b = 0.5 + 0.5 * sin(frequency * modified_t + phaseShift + 4.0);

        float wave3 = sin(t * 120.0 - time * 4.0) * 0.4 + 0.6;

        color *= wave3 * 0.8 + 0.2;

        return color;

    } else if (theme % MAX_COLOR_THEME_INDEX == 4) {

        float wave1 = sin(t * 12.0 + time * 2.0) * 0.5 + 0.5;
        float wave2 = cos(t * 12.0 + time * 3.0) * 0.3 + 0.34;

        vec3 darkBase      = vec3(0.0, 0.0, 0.05);
        vec3 darkTurquoise = vec3(0.0, 0.3, 0.9);
        
        vec3 color = mix(darkBase, darkTurquoise, wave1 * wave2);
        color     += 0.1 * sin(t * 20.0 + time * 4.0);

        return color;

    } else if (theme % MAX_COLOR_THEME_INDEX == 5) {
        
        float frequency = 20.0 + 5.0 * sin(time);
        float stripe    = sin(t * frequency * 3.1415926535);

        stripe = (stripe > 0.0) ? 1.0 : 0.0;

        return vec3(stripe);
    }
}

void main() {
    vec2 resolution = vec2(width, height);
    vec2 uv = (2.0 * gl_FragCoord.xy - resolution) / min(resolution.x, resolution.y);
    
    uv = uv * scale + offset;
    
    vec2 z = vec2(0.0);
    vec2 c = uv;
    
    float smoothIter = 0.0;
    bool     escaped = false;
    
    for (int i = 0; i < MAX_ITER; i++) {
        z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;
        
        if (dot(z, z) > MAX_RADIUS) {
            float log_zn = log(dot(z, z)) / 2.0;

            float nu   = log(log_zn / log(2.0)) / log(2.0);
            smoothIter = float(i) + 1.0 - nu;

            escaped = true;
            break;
        }
    }
    
    if (!escaped) {

        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);

    } else {

        float t    = smoothIter / float(MAX_ITER);
        vec3 color = colorScheme(t);
        
    if (theme % MAX_COLOR_THEME_INDEX == 0) {

        color += 0.8 * exp(-5.0 * t) * vec3(1.0, 0.7, 0.5);

    } else if (theme % MAX_COLOR_THEME_INDEX == 1) {

        color += 0.8 * exp(-5.0 * t) * vec3(-2.0, 0.2, 0.5);

    } else if (theme % MAX_COLOR_THEME_INDEX == 2) {

        color += 0.8 * exp(-5.0 * t) * vec3(0.5, 0.4, 0.0);

    } else if (theme % MAX_COLOR_THEME_INDEX == 3) {

        color += 0.8 * exp(-5.0 * t) * vec3(0.3, 0.3, 0.3);

    } else if (theme % MAX_COLOR_THEME_INDEX == 4) {

        color += 0.8 * exp(-5.0 * t) * vec3(-2.0, 0.2, 0.2);

    }
        
        color = pow(color, vec3(1.0 / 2.2));
        
        gl_FragColor = vec4(color, 1.0);
    }
}
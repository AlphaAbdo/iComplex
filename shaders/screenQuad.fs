#version 430

layout(std430, binding = 1) buffer SSBO {
    int data[];
};

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texOuto; 

uniform int SCR_HEIGHT;
uniform int SCR_WIDTH;
uniform int crunch;
uniform float animationFrame;
uniform vec3 colors;

float colorMapper(float tmul) {
    float inttmul = floor(tmul);
    float rotational = mod(inttmul, 256.0);
    float direction = floor(inttmul / 256.0);
    float fractional = fract(tmul);
    
    float toreturn = mix(
        rotational + fractional,
        255.0 - rotational - fractional,
        mod(direction, 2.0)
    );
    
    return clamp(toreturn, 0.0, 255.0);
}

#define NEGATIVE_OFFSET -5

void main()
{             
    // Assuming the texture coordinates map directly to the SSBO indices
    // Here, we map the 2D texture coordinates to a 1D index for simplicity
    int x = int(TexCoords.x * SCR_WIDTH);
    int y = int(TexCoords.y * SCR_HEIGHT);
    int index = y * SCR_WIDTH + x;
    vec4 local_FragColor = vec4(0.0, 0.0, 0.0, 1.0); // unscoped default value

    // Ensure the index is within bounds of the SSBO data array
    if (index >= 0 && index < data.length()) {

        int depth = data[index];
        if(depth <= NEGATIVE_OFFSET){
            //very rare case, but it can be covered here,
            depth = -depth + NEGATIVE_OFFSET;
        }
        if (depth == -1){
            local_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
        else if (depth <= -2){
            local_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
        }
        else if (depth != crunch) {
            
            float idepth = animationFrame + depth;
            local_FragColor.r = colorMapper(idepth * colors.r)/255.0f;
            local_FragColor.g = colorMapper(idepth * colors.g)/255.0f;
            local_FragColor.b = colorMapper(idepth * colors.b)/255.0f;
        } 
    }
    FragColor = local_FragColor;
}

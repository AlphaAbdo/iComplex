#version 430

layout(std430, binding = 0) buffer RawSSBO {
    int rawSSBO[];
};

layout(std430, binding = 1) buffer FinalSSBO {
    int finalSSBO[];
};

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

uniform int SCR_HEIGHT;
uniform int SCR_WIDTH;

#define KERNEL_SIZE 5
#define SIGMA_S 2.0
#define SIGMA_R 25.0

float gaussian(float x, float sigma) {
    return exp(-(x * x) / (2.0 * sigma * sigma));
}

void main() {
    ivec2 globalID = ivec2(gl_GlobalInvocationID.xy);
    int x = globalID.x;
    int y = globalID.y;

    // Ensure the coordinates are within the image bounds
    if (x >= 0 && x < SCR_WIDTH && y >= 0 && y < SCR_HEIGHT) {
        float sum = 0.0;
        float weightSum = 0.0;
        int centerValue = rawSSBO[SCR_WIDTH * y + x];

        // Apply a bilateral filter in a (KERNEL_SIZE x KERNEL_SIZE) neighborhood
        for (int dy = -KERNEL_SIZE / 2; dy <= KERNEL_SIZE / 2; ++dy) {
            for (int dx = -KERNEL_SIZE / 2; dx <= KERNEL_SIZE / 2; ++dx) {
                int nx = x + dx;
                int ny = y + dy;

                // Make sure we don't access out-of-bounds elements
                if (nx >= 0 && nx < SCR_WIDTH && ny >= 0 && ny < SCR_HEIGHT) {
                    int neighborValue = rawSSBO[SCR_WIDTH * ny + nx];
                    
                    // Calculate the spatial weight
                    float spatialWeight = gaussian(length(vec2(dx, dy)), SIGMA_S);
                    
                    // Calculate the range weight
                    float rangeWeight = gaussian(float(neighborValue - centerValue), SIGMA_R);
                    
                    // Calculate the combined weight
                    float weight = spatialWeight * rangeWeight;
                    
                    // Accumulate the weighted sum
                    sum += neighborValue * weight;
                    weightSum += weight;
                }
            }
        }

        // The filtered value is the normalized weighted sum
        finalSSBO[SCR_WIDTH * y + x] = int(sum / weightSum);
    }
}

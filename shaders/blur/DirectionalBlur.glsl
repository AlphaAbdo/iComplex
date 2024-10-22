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

#define BLOCKDIM_X 16
#define BLOCKDIM_Y 16

void main() {
    ivec2 gID = ivec2(gl_GlobalInvocationID.xy);
    ivec2 lID = ivec2(gl_LocalInvocationID.xy);

    // Coordinates of the current pixel
    int x = gID.x;
    int y = gID.y;

    // Ensure the coordinates are within bounds
    if (x >= 0 && x < SCR_WIDTH && y >= 0 && y < SCR_HEIGHT) {
        // Array to hold the 3x3 neighborhood
        int neighborhood[9];
        int index = 0;

        // Collect values from the 3x3 neighborhood
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                int nx = x + dx;
                int ny = y + dy;

                // Clamp coordinates to the image bounds
                if (nx >= 0 && nx < SCR_WIDTH && ny >= 0 && ny < SCR_HEIGHT) {
                    neighborhood[index++] = rawSSBO[SCR_WIDTH * ny + nx];
                } else {
                    neighborhood[index++] = 0; // Default to 0 if out of bounds
                }
            }
        }

        // Sort the neighborhood array to find the median
        // A simple sorting algorithm like bubble sort is used for clarity
        for (int i = 0; i < 8; ++i) {
            for (int j = i + 1; j < 9; ++j) {
                if (neighborhood[i] > neighborhood[j]) {
                    int temp = neighborhood[i];
                    neighborhood[i] = neighborhood[j];
                    neighborhood[j] = temp;
                }
            }
        }

        // The median value is at index 4 (5th element) in a sorted array of 9 elements
        int median = neighborhood[4];

        // Write the result to the final buffer
        finalSSBO[SCR_WIDTH * y + x] = median;
    }
}


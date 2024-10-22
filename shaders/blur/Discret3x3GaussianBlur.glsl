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
#define NEGATIVE_OFFSET -1  // Adjust this value as needed

shared int DepthData[BLOCKDIM_Y+2][BLOCKDIM_X+2];

int offseteR(int depth) {
    // Implement the offseteR function if needed
    return depth;
}

void main() {
    ivec2 gID = ivec2(gl_GlobalInvocationID.xy);
    ivec2 lID = ivec2(gl_LocalInvocationID.xy);
    
    int offsetX = int(gl_WorkGroupID.x * gl_WorkGroupSize.x);
    int offsetY = int(gl_WorkGroupID.y * gl_WorkGroupSize.y);

    if((offsetX > -BLOCKDIM_X) && (offsetX < SCR_WIDTH) && (offsetY > -BLOCKDIM_Y) && (offsetY < SCR_HEIGHT)) {
        // Load data into shared memory
        for(int id = int(gl_LocalInvocationIndex); id < (BLOCKDIM_X+2)*(BLOCKDIM_Y+2); id += int(gl_WorkGroupSize.x * gl_WorkGroupSize.y)) {
            int idx = id % (BLOCKDIM_X+2);
            int idy = id / (BLOCKDIM_X+2);
            bool withinBuffer = (offsetY + idy - 1 >= 0) && (offsetY + idy - 1 < SCR_HEIGHT) &&
                                (offsetX + idx - 1 >= 0) && (offsetX + idx - 1 < SCR_WIDTH);
            int depth = withinBuffer ? rawSSBO[SCR_WIDTH * (offsetY + idy - 1) + offsetX + idx - 1] : -1;
            DepthData[idy][idx] = depth;
        }

        barrier();

        int idx = offsetX + lID.x;
        int idy = offsetY + lID.y;
        int pixel = SCR_WIDTH * idy + idx;

        if (idx >= 0 && idx < SCR_WIDTH && idy >= 0 && idy < SCR_HEIGHT) {
            int depth = DepthData[lID.y + 1][lID.x + 1];
            
            if(idx > 0 && idx < SCR_WIDTH-1 && idy > 0 && idy < SCR_HEIGHT-1) {
                float depth1 = 0.0;
                depth1 += float(offseteR(depth)) * 4.0;
                depth1 += (float(offseteR(DepthData[lID.y][lID.x+1])) + float(offseteR(DepthData[lID.y+1][lID.x])) + 
                           float(offseteR(DepthData[lID.y+1][lID.x+2])) + float(offseteR(DepthData[lID.y+2][lID.x+1]))) * 2.0;
                depth1 += (float(offseteR(DepthData[lID.y][lID.x])) + float(offseteR(DepthData[lID.y][lID.x+2])) + 
                           float(offseteR(DepthData[lID.y+2][lID.x])) + float(offseteR(DepthData[lID.y+2][lID.x+2]))) * 1.0;
                depth = int(depth1 / 16.0);
            } 
            finalSSBO[pixel] = depth;
        }
    }
}

// #version 430

// layout(std430, binding = 0) buffer RawSSBO {
//     int rawSSBO[];
// };

// layout(std430, binding = 1) buffer FinalSSBO {
//     int finalSSBO[];
// };

// layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

// uniform int SCR_HEIGHT;
// uniform int SCR_WIDTH;

// void main()
// {
//     // Get global IDs as integers
//     int globalX = int(gl_GlobalInvocationID.x);
//     int globalY = int(gl_GlobalInvocationID.y);

//     // Calculate 1D index based on 2D grid position
//     int index = globalY * SCR_WIDTH + globalX;

//     // Ensure index is within bounds before accessing the buffers
//     if (index >= 0 && index < rawSSBO.length()) {
//         finalSSBO[index] = rawSSBO[index];
//     }
// }
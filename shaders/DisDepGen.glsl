#version 430 core

layout(std430, binding = 0) buffer RawSSBO {
    int rawSSBO[];
};

layout(std430, binding = 1) buffer FinalSSBO {
    int finalSSBO[];
};

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(location = 1) uniform bool HasEffect;
layout(location = 2) uniform dvec2 Offset;
layout(location = 3) uniform double scale;
layout(location = 4) uniform int crunch;
layout(location = 5) uniform int SCR_WIDTH;
layout(location = 6) uniform int SCR_HEIGHT;


//Definiton
#define double_float_r vec2

// #define TOPE_IS_STRUCT


#ifdef TOPE_IS_STRUCT
#define TOPE double_float_r

struct TYPE
{
    double_float_r x;
    double_float_r y;
} ;

#else
#define TOPE float
#define TYPE vec2
#endif

double_float_r dsfeq(double b) ;
double_float_r dsfeq(float b) ;

double_float_r dsfADD(double_float_r a, double_float_r b) ;

double_float_r dsfSUB(double_float_r a, double_float_r b) ;

double_float_r dsmul(double_float_r a, double_float_r b) ;

double_float_r dsdiv(double_float_r a, double_float_r b) ;

vec2 dsassign(double_float_r sourceDF);

#ifndef TOPE_IS_STRUCT

bool iterationCritiria(TOPE x, TOPE y);

TYPE forwardpass(TOPE x, TOPE y, TOPE x0, TOPE y0);

#else

bool iterationCritiria(double_float_r x, double_float_r y);

TYPE forwardpass(double_float_r x, double_float_r y, double_float_r x0, double_float_r y0);

TYPE initialize(double x, double y) {
    
    return initialize(dsfeq(x), dsfeq(y));
}
#endif

TYPE initialize(TOPE x, TOPE y)
{
    TYPE result;
    result.x = x;
    result.y = y;
    return result;
}


int mainCore(TOPE transCx, TOPE transCy,int iterationBarrier)
{
    int iterations = 0;

    TYPE z = initialize(0,0);

    for(;iterations < iterationBarrier && iterationCritiria(z.x,z.y);iterations++)
    {
        z = forwardpass(z.x,z.y, transCx, transCy);
    }

    return iterations;
}


void main() {
    if(gl_GlobalInvocationID.x < SCR_WIDTH && gl_GlobalInvocationID.x >= 0 && gl_GlobalInvocationID.y < SCR_HEIGHT && gl_GlobalInvocationID.y >= 0)
    {
        

    #ifndef TOPE_IS_STRUCT
    TOPE transformedCoordinates[2] = {
        TOPE(gl_GlobalInvocationID.x * scale + Offset.x),
        TOPE(gl_GlobalInvocationID.y * scale + Offset.y)
    };
    #else
    TOPE transformedCoordinates[2] = {
        dsfeq(gl_GlobalInvocationID.x * scale + Offset.x),
        dsfeq(gl_GlobalInvocationID.y * scale + Offset.y)
    };
    #endif
    
    int depth = mainCore(transformedCoordinates[0],transformedCoordinates[1],crunch);

    if(HasEffect)
        rawSSBO[gl_GlobalInvocationID.y * SCR_WIDTH + gl_GlobalInvocationID.x] = depth;
    else
        finalSSBO[gl_GlobalInvocationID.y * SCR_WIDTH + gl_GlobalInvocationID.x] = depth;
    
    }
}

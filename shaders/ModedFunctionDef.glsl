#ifndef TOPE_IS_STRUCT

bool iterationCritiria(TOPE x, TOPE y)
{
    return ((x * x + y * y) < 4.0);
}

TYPE forwardpass(TOPE x, TOPE y, TOPE x0, TOPE y0)
{
    TYPE result = {  x*x - y*y + x0
                    , 2.0*x*y + y0};
    return result;
}

#else

bool iterationCritiria(double_float_r x, double_float_r y)
{
    double_float_r x2 = dsfADD(dsmul(x, x), dsmul(y, y));
    return ( x2.y < 4.0 - x2.x );
}

TYPE forwardpass(double_float_r x, double_float_r y, double_float_r x0, double_float_r y0)
{

    TYPE result = {  dsfADD(dsfSUB(dsmul(x, x), dsmul(y, y)), x0)
                    , dsfADD(dsmul(dsfeq(2.0f), dsmul(x, y)), y0)};
    return result;
}

#endif
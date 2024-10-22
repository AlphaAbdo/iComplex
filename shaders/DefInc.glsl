
vec2 dsassign(double_float_r sourceDF) {
    return vec2(sourceDF.x, sourceDF.y);
}

double_float_r dsfeq(double b) {
  double_float_r a;
  a.x = float(b);
  a.y = float(b - a.x);
  return a;
}  // dsdcp

// This function sets the DS number A equal to the single precision floating
// point number B.
double_float_r dsfeq(float b) {
  double_float_r a; 
  a.x = b;
  a.y = 0.0f;
  return a;
}  // dsfeq

// This function computes c = a + b.
double_float_r dsfADD( double_float_r a,  double_float_r b) {
  double_float_r c;
  // Compute dsa + dsb using Knuth's trick.
  float t1 = a.x + b.x;
  float e = t1 - a.x;
  float t2 = ((b.x - e) + (a.x - (t1 - e))) + a.y + b.y;

  // The result is t1 + t2, after normalization.
  c.x = e = t1 + t2;
  c.y = t2 - (e - t1);
  return c;
}  // dsadd

// This function computes c = a - b.
double_float_r dsfSUB( double_float_r a,  double_float_r b) {
  double_float_r c;
  // Compute dsa - dsb using Knuth's trick.
  float t1 = a.x - b.x;
  float e = t1 - a.x;
  float t2 = ((-b.x - e) + (a.x - (t1 - e))) + a.y - b.y;

  // The result is t1 + t2, after normalization.
  c.x = e = t1 + t2;
  c.y = t2 - (e - t1);
  return c;
}  // dssub

double_float_r dsmul( double_float_r a,  double_float_r b) {
  double_float_r c;
    // This splits dsa(1) and dsb(1) into high-order and low-order words.
    float cona = a.x * 8193.0f;
    
    float conb = b.x * 8193.0f;

    float sa1 = (a.x - cona) + cona;
    float sb1 = (b.x - conb) + conb;
    float sa2 = a.x - sa1;
    float sb2 = b.x - sb1;

    // Multilply a0 * b0 using Dekker's method.
    float c11 = a.x * b.x;
    float c21 = (((sa1 * sb1 - c11) + sa1 * sb2) + sa2 * sb1) + sa2 * sb2;

    // Compute a0 * b1 + a1 * b0 (only high-order word is needed).
    float c2 = a.x * b.y + a.y * b.x;

    // Compute (c11, c21) + c2 using Knuth's trick, also adding low-order product.
    float t1 = c11 + c2;
    float e = t1 - c11;
    float t2 = ((c2 - e) + (c11 - (t1 - e))) + c21 + a.y * b.y;

    // The result is t1 + t2, after normalization.
    c.x = e = t1 + t2;
    c.y = t2 - (e - t1);

  return c;
}  // dsmul

double_float_r dsdiv( double_float_r a,  double_float_r b) {
  double_float_r c;
  double_float_r U = {a.x / b.x,0};
  double_float_r sB = {b.x,0};
  double_float_r T = dsmul(U, sB);
  // dsmul(&T, &U, &sB);
  float L = (a.x - T.x - T.y + a.y - U.x * b.y  )/b.x;

  c.x = U.x + L;
  c.y = L - (c.x - U.x);
  return c;
}  // dsdiv


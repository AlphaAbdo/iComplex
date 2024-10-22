#pragma once

#ifdef __CUDACC__
    #define ADDFLAG __host__ __device__
#else
    #define ADDFLAG
#endif

#ifdef __cplusplus
  #include <iostream>
  #include <ostream>
#else
  #include <stdio.h>  
#endif

#ifndef __cplusplus
typdef struct
{
    float hi;
    float lo;
} double_float;

ADDFLAG inline double_float dsdeq(double b);
ADDFLAG inline double_float dsfeq(float b);
ADDFLAG inline double_float dsadd(const double_float a, const double_float b);
ADDFLAG inline double_float dssub(const double_float a, const double_float b);
ADDFLAG inline double_float dsmul(const double_float a, const double_float b);
ADDFLAG inline double_float dsdiv(const double_float a, const double_float b);

ADDFLAG double_float dsdeq(double b) {
  double_float a;
  a.hi = (float)b;
  a.lo = (float)(b - a.hi);
  return a;
}  // dsdcp

// This function sets the DS number A equal to the single precision floating
// point number B.
ADDFLAG double_float dsfeq(float b) {
  double_float a; 
  a.hi = b;
  a.lo = 0.0f;
  return a;
}  // dsfeq

// This function computes c = a + b.
ADDFLAG double_float dsadd(const double_float a, const double_float b) {
  double_float c;
  // Compute dsa + dsb using Knuth's trick.
  float t1 = a.hi + b.hi;
  float e = t1 - a.hi;
  float t2 = ((b.hi - e) + (a.hi - (t1 - e))) + a.lo + b.lo;

  // The result is t1 + t2, after normalization.
  c.hi = e = t1 + t2;
  c.lo = t2 - (e - t1);
  return c;
}  // dsadd

// This function computes c = a - b.
ADDFLAG double_float dssub(const double_float a, const double_float b) {
  double_float c;
  // Compute dsa - dsb using Knuth's trick.
  float t1 = a.hi - b.hi;
  float e = t1 - a.hi;
  float t2 = ((-b.hi - e) + (a.hi - (t1 - e))) + a.lo - b.lo;

  // The result is t1 + t2, after normalization.
  c.hi = e = t1 + t2;
  c.lo = t2 - (e - t1);
  return c;
}  // dssub

ADDFLAG double_float dsmul(const double_float a, const double_float b) {
  double_float c;
    // This splits dsa(1) and dsb(1) into high-order and low-order words.
    float cona = a.hi * 8193.0f;
    
    float conb = b.hi * 8193.0f;

    float sa1 = (a.hi - cona) + cona;
    float sb1 = (b.hi - conb) + conb;
    float sa2 = a.hi - sa1;
    float sb2 = b.hi - sb1;

    // Multilply a0 * b0 using Dekker's method.
    float c11 = a.hi * b.hi;
    float c21 = (((sa1 * sb1 - c11) + sa1 * sb2) + sa2 * sb1) + sa2 * sb2;

    // Compute a0 * b1 + a1 * b0 (only high-order word is needed).
    float c2 = a.hi * b.lo + a.lo * b.hi;

    // Compute (c11, c21) + c2 using Knuth's trick, also adding low-order product.
    float t1 = c11 + c2;
    float e = t1 - c11;
    float t2 = ((c2 - e) + (c11 - (t1 - e))) + c21 + a.lo * b.lo;

    // The result is t1 + t2, after normalization.
    c.hi = e = t1 + t2;
    c.lo = t2 - (e - t1);

  return c;
}  // dsmul

ADDFLAG double_float dsdiv(const double_float a, const double_float b) {
  double_float c;
  double_float U = {a.hi / b.hi,0};
  double_float sB = {b.hi,0};
  double_float T = dsmul(U, sB);
  // dsmul(&T, &U, &sB);
  float L = (a.hi - T.hi - T.lo + a.lo - U.hi * b.lo  )/b.hi;

  c.hi = U.hi + L;
  c.lo = L - (c.hi - U.hi);
  return c;
}  // dsdiv





#else

template<typename R>
class double_float_r
{
public:
    R hi;
    R lo;

    template<typename T>
    ADDFLAG double_float_r(T b) {
        hi = (R)b;
        lo = (R)(b - hi);
    }
    template<typename T>
    ADDFLAG double_float_r(double_float_r<T> b) {
        hi = (R)b.hi + (R)b.lo;
        lo = ((R)b.hi - hi) + (R)b.lo;
    }

    ADDFLAG double_float_r() : hi(0), lo(0) {}
    double_float_r(R h, R l) : hi(h), lo(l) {}
    ADDFLAG double_float_r(std::initializer_list<double> init) {
        auto it = init.begin();
        hi = (it != init.end()) ? *it : 0;
        lo = (it + 1 != init.end()) ? *(it + 1) : 0;
    }

    ADDFLAG operator double();
    ADDFLAG operator float();

    template<typename T>
    ADDFLAG double_float_r operator+(const T& b) const;
    template<typename T>
    ADDFLAG double_float_r operator-(const T& b) const ;
    template<typename T>
    ADDFLAG double_float_r operator*(const T& b) const ;
    template<typename T>
    ADDFLAG double_float_r operator/(const T& b) const ;

    template<typename T>
    ADDFLAG bool operator==(const T& other) const ;
    template<typename T> 
    ADDFLAG bool operator!=(const T& other) const ;
    template<typename T>
    ADDFLAG bool operator<=(const T& other) const ;
    template<typename T>
    ADDFLAG bool operator>=(const T& other) const ;
    template<typename T>
    ADDFLAG bool operator<(const T& other) const ;
    template<typename T>
    ADDFLAG bool operator>(const T& other) const ;

    template<typename T>
    ADDFLAG bool operator==(const double_float_r<T>& other) const ;
    template<typename T> 
    ADDFLAG bool operator!=(const double_float_r<T>& other) const ;
    template<typename T>
    ADDFLAG bool operator<=(const double_float_r<T>& other) const ;
    template<typename T>
    ADDFLAG bool operator>=(const double_float_r<T>& other) const ;
    template<typename T>
    ADDFLAG bool operator<(const double_float_r<T>& other) const ;
    template<typename T>
    ADDFLAG bool operator>(const double_float_r<T>& other) const ;

    template<typename T>
    friend std::ostream& operator<<(std::ostream& os, double_float_r<T>& obj);

};


template<typename R>
ADDFLAG double_float_r<R>::operator double() {
    return static_cast<double>(hi) + static_cast<double>(lo);
}

template<typename R>
ADDFLAG double_float_r<R>::operator float(){
    return hi;
}

template<typename R>
template<typename T>
ADDFLAG double_float_r<R> double_float_r<R>::operator+(const T& Tb) const {
  double_float_r<R> c;
  double_float_r<R> b = double_float_r<R>(Tb);
  // Compute dsa + dsb using Knuth's trick.
  R t1 = this->hi + b.hi;
  R e = t1 - this->hi;
  R t2 = ((b.hi - e) + (this->hi - (t1 - e))) + this->lo + b.lo;

  // The result is t1 + t2, after normalization.
  c.hi = e = t1 + t2;
  c.lo = t2 - (e - t1);
  return c;
}

template<typename R>
template<typename T>
ADDFLAG double_float_r<R> double_float_r<R>::operator-(const T& Tb) const {
  double_float_r<R> c;
  double_float_r<R> b = double_float_r<R>(Tb);
  // Compute dsa - dsb using Knuth's trick.
  R t1 = this->hi - b.hi;
  R e = t1 - this->hi;
  R t2 = ((-b.hi - e) + (this->hi - (t1 - e))) + this->lo - b.lo;

  // The result is t1 + t2, after normalization.
  c.hi = e = t1 + t2;
  c.lo = t2 - (e - t1);
  return c;
}


template<typename R,typename T>
ADDFLAG double_float_r<R> dsmul(const T& a, const T& b) {
  double_float_r<R> c;
  // This splits dsa(1) and dsb(1) into high-order and low-order words.
  R magicNumber = (sizeof(R) == 4) ? 8193.0f : 134217729.0;
  R cona = a.hi * magicNumber;
  
  R conb = b.hi * magicNumber;

  R sa1 = (a.hi - cona) + cona;
  R sb1 = (b.hi - conb) + conb;
  R sa2 = a.hi - sa1;
  R sb2 = b.hi - sb1;

  // Multilply a0 * b0 using Dekker's method.
  R c11 = a.hi * b.hi;
  R c21 = (((sa1 * sb1 - c11) + sa1 * sb2) + sa2 * sb1) + sa2 * sb2;

  // Compute a0 * b1 + a1 * b0 (only high-order word is needed).
  R c2 = a.hi * b.lo + a.lo * b.hi;

  // Compute (c11, c21) + c2 using Knuth's trick, also adding low-order product.
  R t1 = c11 + c2;
  R e = t1 - c11;
  R t2 = ((c2 - e) + (c11 - (t1 - e))) + c21 + a.lo * b.lo;

  // The result is t1 + t2, after normalization.
  c.hi = e = t1 + t2;
  c.lo = t2 - (e - t1);

  return c;
}  // dsmul


template<>
template<typename T>
ADDFLAG double_float_r<float> double_float_r<float>::operator*(const T& Tb) const {
  
  return dsmul<float,T>(*this, double_float_r<float>(Tb));
}

template<>
template<typename T>
ADDFLAG double_float_r<double> double_float_r<double>::operator*(const T& Tb) const {
  
  return dsmul<double,T>(*this, double_float_r<double>(Tb));
}

template<typename R>
template<typename T>
ADDFLAG double_float_r<R> double_float_r<R>::operator/(const T& Tb) const 
{  
  double_float_r<R> c;
  double_float_r<R> b = double_float_r<R>(Tb);
  double_float_r<R> U = {this->hi / b.hi,0};
  double_float_r<R> sB = {b.hi,0};
  double_float_r<R> Tm = dsmul(U, sB);
  // dsmul(&T, &U, &sB);
  R L = (this->hi - Tm.hi - Tm.lo + this->lo - U.hi * b.lo  )/b.hi;

  c.hi = U.hi + L;
  c.lo = L - (c.hi - U.hi);
  return c;
}

template<typename R>
template<typename T>
ADDFLAG bool double_float_r<R>::operator==(const T& other) const {
  double_float_r<R> R_other = static_cast<double_float_r<R>>(other);
  R t1 = this->hi - R_other.hi;
  t1 = t1 + (this->lo - R_other.lo);

  return t1 == 0;
}


template<typename R>
template<typename T>
ADDFLAG bool double_float_r<R>::operator!=(const T& other) const {
  double_float_r<R> R_other = static_cast<double_float_r<R>>(other);
  R t1 = this->hi - R_other.hi;
  t1 = t1 + (this->lo - R_other.lo);

  return t1 != 0;
}


template<typename R>
template<typename T>
ADDFLAG bool double_float_r<R>::operator<=(const T& other) const {
  double_float_r<R> R_other = static_cast<double_float_r<R>>(other);
  R t1 = this->hi - R_other.hi;
  t1 = t1 + (this->lo - R_other.lo);

  return t1 <= 0;
}

template<typename R>
template<typename T>
ADDFLAG bool double_float_r<R>::operator>=(const T& other) const {
  double_float_r<R> R_other = static_cast<double_float_r<R>>(other);
  R t1 = this->hi - R_other.hi;
  t1 = t1 + (this->lo - R_other.lo);

  return t1 >= 0;
}


template<typename R>
template<typename T>
ADDFLAG bool double_float_r<R>::operator<(const T& other) const {
  double_float_r<R> R_other = static_cast<double_float_r<R>>(other);
  R t1 = this->hi - R_other.hi;
  t1 = t1 + (this->lo - R_other.lo);

  return t1 < 0;
}

template<typename R>
template<typename T>
ADDFLAG bool double_float_r<R>::operator>(const T& other) const {
  double_float_r<R> R_other = static_cast<double_float_r<R>>(other);
  R t1 = this->hi - R_other.hi;
  t1 = t1 + (this->lo - R_other.lo);

  return t1 > 0;
}



template<typename X>
template<typename Y>
ADDFLAG bool double_float_r<X>::operator==(const double_float_r<Y>& other) const {
  return (this->hi - other.hi) + (this->lo - other.lo) == 0;
}
template<typename X>
template<typename Y>
ADDFLAG bool double_float_r<X>::operator!=(const double_float_r<Y>& other) const {
  return (this->hi - other.hi) + (this->lo - other.lo) != 0;
}
template<typename X>
template<typename Y>
ADDFLAG bool double_float_r<X>::operator>=(const double_float_r<Y>& other) const {
  return (this->hi - other.hi) + (this->lo - other.lo) >= 0;
}
template<typename X>
template<typename Y>
ADDFLAG bool double_float_r<X>::operator<=(const double_float_r<Y>& other) const {
  return (this->hi - other.hi) + (this->lo - other.lo) <= 0;
}
template<typename X>
template<typename Y>
ADDFLAG bool double_float_r<X>::operator>(const double_float_r<Y>& other) const {
  return (this->hi - other.hi) + (this->lo - other.lo) > 0;
}
template<typename X>
template<typename Y>
ADDFLAG bool double_float_r<X>::operator<(const double_float_r<Y>& other) const {
  return (this->hi - other.hi) + (this->lo - other.lo) <= 0;
}




template<typename R>
std::ostream& operator<<(std::ostream& os, double_float_r<R>& obj)
{
  int whole = static_cast<int>(obj.hi);
  double_float_r<R> copy = obj - whole;
  os << whole << ".";
  double_float_r<R> ten = 10;
  if (copy.hi < 0) {
      copy.hi = -copy.hi;
      copy.lo = -copy.lo;
  }
  for (int i = 0; i < 10; i++) {
      copy = copy * ten;
      whole = static_cast<int>(copy.hi);
      os << whole;
      copy = copy - whole;
  }

  return os;
} 
#endif


// int main() {
  
//     double a_reff = 234.00012345;
//     double b_reff = 65.4321098765;
    
//     double_float_r<float> a = (a_reff);
//     double_float_r<float> b = (b_reff);


//     double_float_r<float> product = (a*b);
//     double_float_r<double> product2 = product;
//     // product = 
//     double reference = a_reff * b_reff;
//     printf("Expected:\t %0.15lf,\nActual:\t\t %0.15lf,\nActual:\t\t %0.15lf,\nCast:\t\t %0.15lf,\nDiff:\t\t %0.15lf\n", 
//             reference , product.hi, product.lo, ((double)product.hi + (double)product.lo),reference - (double)product.hi - (double)product.lo);
//     std::cout << (product==product2) << std::endl;

//     return 0;
// }
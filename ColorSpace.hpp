#pragma once
#include <Eigen/Dense>

Eigen::Matrix3d calcConversionMatrix(float xr, float yr, float xg, float yg, float xb, float yb, float xw, float yw);

class ColorSpace {
    public:
        float xr, yr, xg, yg, xb, yb, xw, yw;
        Eigen::Matrix3d matrix; /* Color conversion matrix */
        ColorSpace(float xr, float yr, float xg, float yg, float xb, float yb, float xw, float yw) :
            xr(xr), yr(yr), xg(xg), yg(yg), xb(xb), yb(yb), xw(xw), yw(yw) {
                this->matrix = calcConversionMatrix(xr, yr, xg, yg, xb, yb, xw, yw);
            }
        ColorSpace() {}
};


Eigen::Matrix3d calcConversionMatrix(float xr, float yr, float xg, float yg, float xb, float yb, float xw, float yw) {
    /*
    Args: chromaticity coordinates of given RGB system

    Source for math: http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html

    Terms for variables:
    Primaries: xr, yr, xg, yg, xb, yb
    White point: xw, yw
    XYZ unscaled: Xr, Yr, Xg, Yg, Xb, Yb
    XYZ white point: Xw, Yw

    */

   /* Convert RGB primaries to XYZ (unscaled) */
   float Xr = xr/yr;
   float Yr = 1;
   float Zr = (1-xr-yr)/yr;
   
   float Xg = xg/yg;
   float Yg = 1;
   float Zg = (1-xg-yg)/yg;

   float Xb = xb/yb;
   float Yb = 1;
   float Zb = (1-xb-yb)/yb;

   /* Convert white point to XYZ (unscaled) */
   float Xw = xw/yw;
   float Yw = 1;
   float Zw = (1-xw-yw)/yw;


    /* Put unscaled XYZ values in matrix */
    Eigen::Matrix3d m;
    m << Xr, Xg, Xb,
         Yr, Yg, Yb,
         Zr, Zg, Zb;

    Eigen::Matrix3d mInv = m.inverse();

    /* Put unscaled XYZ white point values in vector */
    Eigen::Vector3d vecW;
    vecW << Xw, Yw, Zw;

    /* Calculate scaling vector.
    These values are also the luminance
    values for converting from RGB to CIE.
    These values end up as the Y component of each
    column in the final "result" matrix.
    (This is something I still don't fully understand)
    See example (for sRGB): 
    https://en.wikipedia.org/wiki/SRGB#Primaries */
    Eigen::Vector3d vecS = mInv * vecW;

    /* Extract the scaling factors */
    float Sr = vecS.x();
    float Sg = vecS.y();
    float Sb = vecS.z();

    /* Put together final color conversion matrix using the
    unscaled XYZ chromaticities and the scaling factors */
    Eigen::Matrix3d result;
    result << Sr*Xr, Sg*Xg, Sb*Xb,
              Sr*Yr, Sg*Yg, Sb*Yb,
              Sr*Zr, Sg*Zg, Sb*Zb;

    /* Print stuff out for debugging */
    #ifdef VERBOSE_MATRIX
    {
        std::cout << "red: ";
        print3f(Xr, Yr, Zr);
        std::cout << "green: ";
        print3f(Xg, Yg, Zg);
        std::cout << "blue: ";
        print3f(Xb, Yb, Zb);

        std::cout << "m: \n";
        std::cout << m << std::endl;
        std::cout << std::endl;

        std::cout << "mInv: \n";
        std::cout << mInv << std::endl;
        std::cout << std::endl;

        std::cout << "vecW: \n";
        std::cout << vecW << std::endl;
        std::cout << std::endl;

        std::cout << "mInv * vecW -> vecS: \n";
        std::cout << vecS << std::endl;
        std::cout << std::endl;

        std::cout << "result: \n";
        std::cout << result << std::endl;
        std::cout << std::endl;
    }
    #endif

    return result;


}
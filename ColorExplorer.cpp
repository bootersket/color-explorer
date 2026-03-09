#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include <iostream>
#include <Eigen/Dense>

static bool VERBOSE = false;
static bool VERBOSE_MATRIX = false;

Eigen::Matrix3d calcConversionMatrix_sRGB();


void print2f(float a, float b) {
    std::cout << "(" << a << ", " << b << ")" << std::endl;
}
void print3f(float a, float b, float c) {
    std::cout << "(" << a << ", " << b << ", " << c << ")" << std::endl;
}
void print4f(float a, float b, float c, float d) {
    std::cout << "(" << a << ", " << b << ", " << c << ", " << d << ")" << std::endl;
}
void print2i(int a, int b) {
    std::cout << "(" << a << ", " << b << ")" << std::endl;
}
void print3i(int a, int b, int c) {
    std::cout << "(" << a << ", " << b << ", " << c << ")" << std::endl;
}
void print4i(int a, int b, int c, int d) {
    std::cout << "(" << a << ", " << b << ", " << c << ", " << d << ")" << std::endl;
}

float inverseTransferFunction(float value) {
    /*
    Source: https://en.wikipedia.org/wiki/SRGB#Transfer_function_(%22gamma%22)
    */
    if (value <= 0.04045) return value / 12.92;
    else return std::pow(((value + 0.055)/1.055), 2.4);
}

float transferFunction(float value) {
    /*
    Source: https://en.wikipedia.org/wiki/SRGB#Transfer_function_(%22gamma%22)
    */
   if (value <= 0.0031308) return value * 12.92;
   else return 1.055*std::pow(value, 1/2.4) - 0.055;
}

Eigen::Vector2d convert_sRGB_to_CIE_chromaticity(int r, int g, int b) {
    /*
    Assumes args are in range of 0-255.
    Currently just prints values out during the process; returns nothing.

    Converts from RGB values to the X Y CIE chromaticity coordinates.
    The process is as follows:
    RGB (0 - 255)
    RGB normalized (0.0 - 1.0)
    RGB linear (gamma decoded)
    CIE XYZ (sRGB to XYZ color conversion matrix applied)
    CIE XY chromaticity coordinates (XYZ normalized and chromaticity derived)
    */

    /* Starting sRGB values */
    if (VERBOSE) {
        std::cout << "sRGB: ";
        print3i(r, g, b);
    }

    /* Normalize RGB */
    float rN = r / 255.0;
    float gN = g / 255.0;
    float bN = b / 255.0;
    if (VERBOSE) {
        std::cout << "Normalized RGB: ";
        print3f(rN, gN, bN);
    }

    /* Convert to linear RGB (decode gamma) */
    float rL = inverseTransferFunction(rN);
    float gL = inverseTransferFunction(gN);
    float bL = inverseTransferFunction(bN);
    if (VERBOSE) {
        std::cout << "Linear RGB:";
        print3f(rL, gL, bL);
    }

    /* Convert to CIE XYZ (aka device-independent values) */
    Eigen::Matrix3d conversionMatrix = calcConversionMatrix_sRGB();
    Eigen::Vector3d linearRGB(rL, gL, bL);
    Eigen::Vector3d CIE_XYZ = conversionMatrix * linearRGB;
    float X = CIE_XYZ.x();
    float Y = CIE_XYZ.y();
    float Z = CIE_XYZ.z();
    if (VERBOSE) {
        std::cout << "CIE XYZ: ";
        print3f(X, Y, Z);
    }

    /* Calculate chromaticity */
    /*
    My understanding of this is like:
    X Y and Z all combine to give us The Color. Technically, X is red/green
    and Z is blue, while Y is luminance. But all that matters is that 
    the mixture of all of these values gives us The Color. We don't care about
    luminance though, we just want to know the values of the color (hue & saturation, in a sense).
    So we total up all the values that comprise The Color, and divide the x or y value
    by that total, so we can determine the proportion of x or y relative to the overall value sum.
    It's kind of like distributing the luminance to the color values, in a way?
    Idk, I don't have a great handle on it.
    */
    float x = X / (X + Y + Z);
    float y = Y / (X + Y + Z);
    if (true) {
        std::cout << "Chromaticity: ";
        print2f(x, y);
    }
    
    Eigen::Vector2d result(x, y);
    return result;

}

/* This is the same as convert_sRG_to_CIEXYZ, except without the final step of calculating
chromaticity, which turns it from 3-dimensional to 2-dimensional */
Eigen::Vector3d convert_sRGB_to_CIEXYZ(int r, int g, int b) {
    /*
    Assumes args are in range of 0-255.
    Currently just prints values out during the process; returns nothing.

    Converts from RGB values to the X Y CIE chromaticity coordinates.
    The process is as follows:
    RGB (0 - 255)
    RGB normalized (0.0 - 1.0)
    RGB linear (gamma decoded)
    CIE XYZ (sRGB to XYZ color conversion matrix applied)
    CIE XY chromaticity coordinates (XYZ normalized and chromaticity derived)
    */

    /* Starting sRGB values */
    if (VERBOSE) {
        std::cout << "sRGB: ";
        print3i(r, g, b);
    }

    /* Normalize RGB */
    float rN = r / 255.0;
    float gN = g / 255.0;
    float bN = b / 255.0;
    if (VERBOSE) {
        std::cout << "Normalized RGB: ";
        print3f(rN, gN, bN);
    }

    /* Convert to linear RGB (decode gamma) */
    float rL = inverseTransferFunction(rN);
    float gL = inverseTransferFunction(gN);
    float bL = inverseTransferFunction(bN);
    if (VERBOSE) {
        std::cout << "Linear RGB:";
        print3f(rL, gL, bL);
    }

    /* Convert to CIE XYZ (aka device-independent values) */
    Eigen::Matrix3d conversionMatrix = calcConversionMatrix_sRGB();
    Eigen::Vector3d linearRGB(rL, gL, bL);
    Eigen::Vector3d CIE_XYZ = conversionMatrix * linearRGB;
    float X = CIE_XYZ.x();
    float Y = CIE_XYZ.y();
    float Z = CIE_XYZ.z();
    if (VERBOSE) {
        std::cout << "CIE XYZ: ";
        print3f(X, Y, Z);
    }

    
    Eigen::Vector3d result(X, Y, Z);
    return result;

}


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
    if (VERBOSE_MATRIX) {
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

    return result;


}

Eigen::Matrix3d calcConversionMatrix_sRGB() {
    float xr, yr, xg, yg, xb, yb, xw, yw;
    xr = .64;
    yr = .33;
    xg = .3;
    yg = .6;
    xb = .15;
    yb = .06;
    xw = .3127;
    yw = .3290;
    Eigen::Matrix3d m = calcConversionMatrix(xr, yr, xg, yg, xb, yb, xw, yw);
    return m;
}

/* Assuming output channels is 4 but pixels are is 3 (default to full alpha channel) */
void createImage(std::vector<Eigen::Vector3d> pixels, int width, int height) {
    const int channels = 4;
    // static unsigned char pixelsBytes[width*height*channels];
    unsigned char *pixelsBytes = (unsigned char*)malloc(width*height*channels);
    int pixelsBytesIndex = 0;
    for (int i=0; i<pixels.size(); i++) {
        Eigen::Vector3d pixel = pixels[i];
        int r = pixel[0];
        int g = pixel[1];
        int b = pixel[2];

        pixelsBytes[pixelsBytesIndex+0] = r;
        pixelsBytes[pixelsBytesIndex+1] = g;
        pixelsBytes[pixelsBytesIndex+2] = b;
        pixelsBytes[pixelsBytesIndex+3] = 255;
        pixelsBytesIndex+=channels;
    }
    stbi_write_png("output.png", width, height, channels, pixelsBytes, width*channels);
}


Eigen::Vector3d convert_CIEXYZ_to_other_rgb(Eigen::Vector3d CIEXYZ) {
    float xr, yr, xg, yg, xb, yb, xw, yw;
    // xr = .64;
    // yr = .33;
    // xg = .3;
    // yg = .6;
    // xb = .15;
    // yb = .06;
    // xw = .3127;
    // yw = .3290;
    xr = .55;
    yr = .4;
    xg = .3;
    yg = .6;
    xb = .15;
    yb = .06;
    xw = .3127;
    yw = .3290;
    // ! this is white piint D93 (approximately)
    // xw = .285;
    // yw = .293;

    Eigen::Matrix3d m = calcConversionMatrix(xr, yr, xg, yg, xb, yb, xw, yw);
    

    // Convert from CIEXYZ to linear RGB
    Eigen::Vector3d linearRGB = m.inverse() * CIEXYZ;
    float rL = linearRGB[0];
    float gL = linearRGB[1];
    float bL = linearRGB[2];

    // Encode gamma
    // N because these are still normalized (i.e. in range 0.0 - 1.0)
    float rN = transferFunction(rL);
    float gN = transferFunction(gL);
    float bN = transferFunction(bL);

    // Convert to range 0 - 255
    float r = rN * 255;
    float g = gN * 255;
    float b = bN * 255;

    Eigen::Vector3d rgb(r, g, b);
    return rgb;
}


int main() {
    int width, height, channels;
    // unsigned char* data = stbi_load("inbetween.png", &width, &height, &channels, 0);
    unsigned char* data = stbi_load("color_test.png", &width, &height, &channels, 0);
    if (!data) {
        std::cout << "ERROR: failed to load image" << std::endl;
        return -1;
    }
    std::cout << "channels: " << channels << std::endl;
    std::cout << "width: " << width << std::endl;
    std::cout << "height: " << height << std::endl;
    std::cout << std::endl;

    std::vector<Eigen::Vector3d> newImagePixels;

    for (int i=0; i<width*height*channels; i+=channels) {
        int pixel = (int)data[i];
        int r = data[i + 0];
        int g = data[i + 1];
        int b = data[i + 2];
        int a = data[i + 3];

        Eigen::Vector3d CIE_XYZ = convert_sRGB_to_CIEXYZ(r, g, b);
        // std::cout << CIE_XYZ << std::endl;
        // std::cout << std::endl;

        Eigen::Vector3d otherRGB = convert_CIEXYZ_to_other_rgb(CIE_XYZ);
        newImagePixels.push_back(otherRGB);
        


        // convert_sRGB_to_CIE_chromaticity(r, g, b);
        // std::cout << std::endl;
    }

    createImage(newImagePixels, width, height);

    // Eigen::Vector2d xy = convert_sRGB_to_CIE_chromaticity(255, 255, 255);
    // std::cout << "xy: " << std::endl;
    // std::cout << xy << std::endl;

    // float xr, yr, xg, yg, xb, yb, xw, yw;
    // xr = .64;
    // yr = .33;
    // xg = .3;
    // yg = .6;
    // xb = .15;
    // yb = .06;
    // // xw = .3127;
    // // yw = .3290;
    // xw = ..285;
    // yw = ..293;
    // Eigen::Matrix3d mat = calcConversionMatrix(xr, yr, xg, yg, xb, yb, xw, yw);


    stbi_image_free(data);

}
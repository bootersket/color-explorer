#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include <iostream>
#include <algorithm>
#include <Eigen/Dense>
#include "ColorSpace.hpp"
#include "PrintFuncs.hpp"
#include "ColorRGBi.hpp"

static bool VERBOSE = false;
static bool VERBOSE_MATRIX = false;
ColorSpace g_colorSpaceA;
ColorSpace g_colorSpaceB;
float g_gammaExponentA;
float g_gammaExponentB;




Eigen::Vector3d convert_color_space_to_CIEXYZ(ColorRGBi color);



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

void printColorRGBi(ColorRGBi color) {
    print3i(color.r, color.g, color.b);
}

/* Gamma decode */
/* Step 1 */
float inverseTransferFunction(float value) {
    /*
    Source: https://en.wikipedia.org/wiki/SRGB#Transfer_function_(%22gamma%22)
    */
    if (value <= 0.04045) return value / 12.92;
    else return std::pow(((value + 0.055)/1.055), 2.4);
}

/* Gamma encode */
/* Step 2 */
float transferFunction(float value) {
    /*
    Source: https://en.wikipedia.org/wiki/SRGB#Transfer_function_(%22gamma%22)
    */
   if (value <= 0.0031308) return value * 12.92;
   else return 1.055*std::pow(value, 1/2.4) - 0.055;
}

// Eigen::Vector2d convert_color_space_to_CIE_chromaticity(ColorRGBi color, ColorSpace colorSpace) {
Eigen::Vector2d convert_color_space_to_CIE_chromaticity(ColorRGBi color) {
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

    // Eigen::Vector3d CIEXYZ = convert_color_space_to_CIEXYZ(color, colorSpace);
    // todo havent really used this func since changing the overall logic to use global g_colorSpaceX vars, so not sure logically if this makes sense for this func i.e. should
    // todo it be using colorSpaceA aka the starting color space? So beware that this may have a weird/unexpected result
    Eigen::Vector3d CIEXYZ = convert_color_space_to_CIEXYZ(color);
    float X = CIEXYZ.x();
    float Y = CIEXYZ.y();
    float Z = CIEXYZ.z();

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
    if (VERBOSE) {
        std::cout << "Chromaticity: ";
        print2f(x, y);
    }
    
    Eigen::Vector2d result(x, y);
    return result;

}

/* This is the same as convert_sRG_to_CIEXYZ, except without the final step of calculating
chromaticity, which turns it from 3-dimensional to 2-dimensional */
/* STEP 1 */
// Eigen::Vector3d convert_color_space_to_CIEXYZ(ColorRGBi color, ColorSpace colorSpace) {
Eigen::Vector3d convert_color_space_to_CIEXYZ(ColorRGBi color) {
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
   int r = color.r;
   int g = color.g;
   int b = color.b;

    /* Starting values */
    if (VERBOSE) {
        std::cout << "\nstarting values: ";
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
    // float rL = inverseTransferFunction(rN);
    // float gL = inverseTransferFunction(gN);
    // float bL = inverseTransferFunction(bN);
    float rL = std::pow(rN, g_gammaExponentA);
    float gL = std::pow(gN, g_gammaExponentA);
    float bL = std::pow(bN, g_gammaExponentA);
    if (VERBOSE) {
        std::cout << "Linear RGB:";
        print3f(rL, gL, bL);
    }

    /* Convert to CIE XYZ (aka device-independent values) */
    Eigen::Vector3d linearRGB{rL, gL, bL};
    Eigen::Vector3d CIE_XYZ = g_colorSpaceA.matrix * linearRGB;
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

bool g_useToneError = false;
float g_toneError = 1.0f;
/* STEP 2 */
// ColorRGBi convert_CIEXYZ_to_color_space(Eigen::Vector3d CIEXYZ, ColorSpace colorSpace) {
ColorRGBi convert_CIEXYZ_to_color_space(Eigen::Vector3d CIEXYZ) {

    /* Convert from CIEXYZ to linear RGB */
    Eigen::Vector3d linearRGB = g_colorSpaceB.matrix.inverse() * CIEXYZ;
    float rL = linearRGB[0];
    float gL = linearRGB[1];
    float bL = linearRGB[2];

// ! tone error disabled for now as I tinker with gamma stuff
    // if (g_useToneError) {
    //     rL = std::pow(rL, g_toneError);
    //     gL = std::pow(gL, g_toneError);
    //     bL = std::pow(bL, g_toneError);
    // }

    /* Encode gamma */
    /* N because these are still normalized (i.e. in range 0.0 - 1.0) */
    // float rN = transferFunction(rL);
    // float gN = transferFunction(gL);
    // float bN = transferFunction(bL);
    float rN = std::pow(rL, 1/g_gammaExponentB);
    float gN = std::pow(gL, 1/g_gammaExponentB);
    float bN = std::pow(bL, 1/g_gammaExponentB);

    /* Convert to range 0 - 255 */
    int r = rN * 255;
    int g = gN * 255;
    int b = bN * 255;

    /* Clamp negative values */
    r = std::max(0, r);
    g = std::max(0, g);
    b = std::max(0, b);

    /* Clamp values over 255 */
    r = std::min(r, 255);
    g = std::min(g, 255);
    b = std::min(b, 255);

    if (VERBOSE) {
        std::cout << "\nConverting CIEXYZ to color space" << std::endl;
        std::cout << "Starting CIEXYZ values: ";
        print3f(CIEXYZ.x(), CIEXYZ.y(), CIEXYZ.z());
        std::cout << "Linear RGB values: ";
        print3f(rL, gL, bL);
        std::cout << "Gamma encoded normalized: ";
        print3f(rN, gN, bN);
        std::cout << "0-255 range (aka final result): ";
        print3f(r, g, b);
    }

    ColorRGBi color = {r, g, b};
    return color;
}


/* Assuming output channels is 4 but pixels are is 3 (default to full alpha channel) */
void saveImage(std::string filename, std::vector<ColorRGBi> pixels, int width, int height) {
    const int channels = 4;
    // static unsigned char pixelsBytes[width*height*channels];
    unsigned char *pixelsBytes = (unsigned char*)malloc(width*height*channels);
    int pixelsBytesIndex = 0;
    for (int i=0; i<pixels.size(); i++) {
        ColorRGBi pixel = pixels[i];

        pixelsBytes[pixelsBytesIndex+0] = pixel.r;
        pixelsBytes[pixelsBytesIndex+1] = pixel.g;
        pixelsBytes[pixelsBytesIndex+2] = pixel.b;
        pixelsBytes[pixelsBytesIndex+3] = 255;
        pixelsBytesIndex+=channels;
    }
    stbi_write_png(filename.c_str(), width, height, channels, pixelsBytes, width*channels);
}




// void convertAndSave(ColorSpace spaceA, ColorSpace spaceB, std::string fileIn, std::string fileOut) {
void convertAndSave(std::string fileIn, std::string fileOut) {
    int width, height, channels;
    unsigned char* data = stbi_load(fileIn.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cout << "ERROR: failed to load image" << std::endl;
        return;
    }
    // std::cout << "channels: " << channels << std::endl;
    // std::cout << "width: " << width << std::endl;
    // std::cout << "height: " << height << std::endl;
    // std::cout << std::endl;

    std::vector<ColorRGBi> newImagePixels;

    for (int i=0; i<width*height*channels; i+=channels) {
        int pixel = (int)data[i];
        int r = data[i + 0];
        int g = data[i + 1];
        int b = data[i + 2];
        int a = data[i + 3];

        ColorRGBi color = {r, g, b};

        Eigen::Vector3d CIE_XYZ = convert_color_space_to_CIEXYZ(color);
        ColorRGBi convertedColor = convert_CIEXYZ_to_color_space(CIE_XYZ);

        newImagePixels.push_back(convertedColor);
    }
    saveImage(fileOut, newImagePixels, width, height);

    stbi_image_free(data);

}

// void makeBatch(ColorSpace spaceA, ColorSpace spaceB) {
void makeBatch() {
    std::vector<std::string> filenames_in = {
        "batch/color_test_ref.png",
        "batch/landscape_ref.png",
        "batch/gi0_ref.png",
        "batch/gi1_ref.png",
        "batch/gi2_ref.png",
        "batch/gi3_ref.png",
        "batch/gi4_ref.png",
        "batch/gi5_ref.png",
        "batch/gi6_ref.png",
        "batch/gi7_ref.png",
        "batch/gi8_ref.png",
        "batch/gi9_ref.png"
    };
    std::vector<std::string> filenames_out = {
        "batch/color_test_miscal.png",
        "batch/landscape_miscal.png",
        "batch/gi0_miscal.png",
        "batch/gi1_miscal.png",
        "batch/gi2_miscal.png",
        "batch/gi3_miscal.png",
        "batch/gi4_miscal.png",
        "batch/gi5_miscal.png",
        "batch/gi6_miscal.png",
        "batch/gi7_miscal.png",
        "batch/gi8_miscal.png",
        "batch/gi9_miscal.png"
    };
    for (int i=0; i<filenames_in.size(); i++) {
        std::cout << "converting " << filenames_in[i] << " (" << i+1 << "/" << filenames_in.size() << ")" << std::endl;
        convertAndSave(filenames_in[i], filenames_out[i]);
    }
}



int main() {
    /* Buncha color spaces */
    ColorSpace sRGB             {.64, .33, .3, .6, .15, .06, .3127, .3290};
    ColorSpace native           {.64, .33, .3, .6, .15, .06, .285, .321};
    ColorSpace sRGB_D93         {.64, .33, .3, .6, .15, .06, .291, .283};
    ColorSpace orangeBiasedRGB  {.55, .4,  .3, .6, .15, .06, .3127, .3290};
    ColorSpace orangeBiasedRGB2 {.55, .4, .32, .65, .14, .038, .3127, .3290};
    ColorSpace magentaBiasedRGB {.55, .4,  .3, .6, 0.320938, 0.154190, .3127, .3290};
    ColorSpace wideGamutRGB     {.7347, .2653, .1152, .8264, .1566, .0177, .3457, .3585};
    ColorSpace yellowedGreenRGB {.64, .33, .38, .55, .15, .06, .3127, .3290};
    ColorSpace magentaedBlueRGB {.64, .33, .3, .6, .22, .06, .3127, .3290};
    ColorSpace allOffRGB        {.6, .36, .35, .55, .25, .06, .3127, .3290};
    ColorSpace cyanOffRGB       {.64, .33, .34, .50, .17, .06, .3127, .3290};

    std::cout << "sRGB matrix: \n" << sRGB.matrix << std::endl;
    std::cout << std::endl;
    std::cout << "sRGB inv matrix: \n" << sRGB.matrix.inverse() << std::endl;
    std::cout << std::endl;
    std::cout << "sRGB_D93 matrix: \n" << sRGB_D93.matrix << std::endl;
    std::cout << std::endl;
    std::cout << "sRGB_D93 inv matrix: \n" << sRGB_D93.matrix.inverse() << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;

    std::cout << "sRGB.matrix.inverse() * sRGB_D93.matrix: " << std::endl;
    std::cout << sRGB.matrix.inverse() * sRGB_D93.matrix << std::endl;

    /* All the heavy lifting done through here */
    // convertAndSave(sRGB, sRGB, "landscape.png", "batch/landscape_tone_error.png");
    // convertAndSave(sRGB, sRGB_D93, "batch/color_test_ref.png", "batch/color_test_miscal.png");

    g_colorSpaceA = sRGB;
    g_colorSpaceB = yellowedGreenRGB;
    std::cout << std::endl;
    std::cout << "g_colorSpaceA.matrix.inverse() * g_colorSpaceB.matrix: " << std::endl;
    std::cout << g_colorSpaceA.matrix.inverse() * g_colorSpaceB.matrix << std::endl;

    // g_useToneError = false;
    // g_toneError = 1.8;
    g_gammaExponentA = 2.4;
    g_gammaExponentB = 2.4;

    // for (int i=0; i<256; i++) {
    //     std::cout << "transferFunc(" << i << "): " << transferFunction(i) << std::endl;
    // }

    // convertAndSave("landscape.png", "batch/landscape_tone_error.png");
    // convertAndSave("landscape.png", "batch/landscape_miscal.png");

    makeBatch();

}

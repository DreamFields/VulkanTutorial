#pragma once
#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmimgle/dcmimage.h"
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
class VolumeRender
{
private:
    const Uint16* pixelDataUint16 = nullptr;
    const Uint8* pixelDataUint8 = nullptr;
public:
    VolumeRender(/* args */);
    ~VolumeRender();

    bool loadDicom(std::string path);
    const Uint8* getPixelUint8();
    const Uint16* getPixelUint16();

    bool getPixelRGBA(std::string path, int& width, int& height, int& numSlice, unsigned char*& rgba);

};



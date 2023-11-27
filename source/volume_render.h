#pragma once
#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmimgle/dcmimage.h"
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>
#include <math.h>

struct DicomTags{
    std::string folderPath;
    std::vector<int> fileIndex;
    int numSlice;
    int boxWidth;
    int boxHeight;
    int boxDepth;
    int minVal;
    int maxVal;
    double windowCenter;
    double windowWidth;
    double rescaleIntercept;
    double rescaleSlope;
};

class VolumeRender
{
private:
    DicomTags dicomTags;
public:
    VolumeRender(/* args */);
    ~VolumeRender();

    bool loadDicom(std::string path, int numSlice = 0);
    bool getPixelRGBA(int& width, int& height, int& numSlice, unsigned char*& rgba);

};



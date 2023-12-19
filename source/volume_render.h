#pragma once
#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmimgle/dcmimage.h"
#include "struct.h"

#include <glm.hpp>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>
#include <math.h>

struct DicomTags {
    std::string folderPath;
    std::vector<int> fileIndex;
    int numSlice;
    int minVal;
    int maxVal;
    double windowCenter;
    double windowWidth;
    double rescaleIntercept;
    double rescaleSlope;
    glm::vec3 voxelSize;
    glm::vec3 realSize;
    glm::vec3 boxSize;
    glm::vec3 voxelResolution;
};

struct DicomParamControl {
    float windowCenter;
    float windowWidth;
    float alphaCorrection;
    int steps;
    float stepLength;
    float glow;
};

class VolumeRender
{
private:
    DicomTags dicomTags;
public:
    DicomParamControl dicomParamControl;
public:
    VolumeRender(/* args */);
    ~VolumeRender();

    bool loadDicom(std::string path, int numSlice = 0);
    bool getPixelRGBA(int& width, int& height, int& numSlice, unsigned char*& rgba);
    DicomTags getDicomTags();
    const std::vector<Vertex> getBoxVertices();
};



#pragma once
#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"
#include <string>
class VolumeRender
{
private:
    /* data */
public:
    VolumeRender(/* args */);
    ~VolumeRender();

    bool loadDicom(std::string path);
};



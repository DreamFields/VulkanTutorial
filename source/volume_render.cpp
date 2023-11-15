#include "volume_render.h"
VolumeRender::VolumeRender(/* args */)
{
}

VolumeRender::~VolumeRender()
{
}

bool VolumeRender::loadDicom(std::string path)
{
    // DICOM解析
    DcmFileFormat fileformat;
    OFCondition status = fileformat.loadFile(path.c_str());
    DcmDataset* dataset = fileformat.getDataset();
    if (!status.good())
    {
        std::cout << "Load Dimcom File Error: " << status.text() << std::endl;
        return false;
    }
    OFString PatientName;
    status = fileformat.getDataset()->findAndGetOFString(DCM_PatientName, PatientName);
    if (status.good())
    {
        std::cout << "Get PatientName:" << PatientName << std::endl;
    }
    else
    {
        std::cout << "Get PatientName Error:" << status.text() << std::endl;
        return false; 
    }
    // 获取图像像素数据
    const Uint16 * pixelData = NULL;
    dataset->findAndGetUint16Array(DCM_PixelData, pixelData);
    return true;
}
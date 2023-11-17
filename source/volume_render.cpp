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

    // 获取像素数据
    unsigned long numPixelsUint16 = 0;
    dataset->findAndGetUint16Array(DCM_PixelData, pixelDataUint16, &numPixelsUint16);
    // dataset->findAndGetSint16Array(DCM_PixelData, pixelData, &numPixels);
    if (pixelDataUint16 == nullptr)
    {
        std::cout << "Get Pixel Data Error: " << status.text() << std::endl;
        return false;
    }
    std::cout << "Get Pixel Data numPixelsUint16: " << numPixelsUint16 << std::endl;

    unsigned long numPixelsUint8 = 0;
    dataset->findAndGetUint8Array(DCM_PixelData, pixelDataUint8, &numPixelsUint8);
    if (pixelDataUint8 == nullptr)
    {
        std::cout << "Get Pixel Data Error: " << status.text() << std::endl;
        return false;
    }
    std::cout << "Get Pixel Data numPixelsUint8: " << numPixelsUint8 << std::endl;

    // 获取PatientName
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

    // 获取图像宽高
    DicomImage* image = new DicomImage(path.c_str());
    if (image == nullptr)
    {
        std::cout << "Load Dimcom File Error: " << status.text() << std::endl;
        return false;
    }
    if (image->getStatus() != EIS_Normal)
    {
        std::cout << "Load Dimcom File Error: " << DicomImage::getString(image->getStatus()) << std::endl;
        return false;
    }
    Uint16 width = image->getWidth();
    Uint16 height = image->getHeight();
    std::cout << "Image Width: " << width << std::endl;
    std::cout << "Image Height: " << height << std::endl;

    double windowCenter = 0.0;
    if (dataset->findAndGetFloat64(DCM_WindowCenter, windowCenter).good())
    {
        std::cout << "Window Center: " << windowCenter << std::endl;
    }
    else { std::runtime_error("windowCenter not found.\r\n"); }

    // window_width
    double windowWidth = 0.0;
    if (dataset->findAndGetFloat64(DCM_WindowWidth, windowWidth).good())
    {
        std::cout << "Window Width: " << windowWidth << std::endl;
    }
    else { std::runtime_error("windowWidth not found.\r\n"); }

    int maxValue = INT_MIN, minValue = INT_MAX;
    for (int i = 0; i < width * height; ++i)
    {
        if (pixelDataUint16[i] > maxValue) maxValue = pixelDataUint16[i];
        if (pixelDataUint16[i] < minValue) minValue = pixelDataUint16[i];
    }
    std::cout << "Max Value: " << maxValue << std::endl;
    std::cout << "Min Value: " << minValue << std::endl;

    // samples per pixel
    Uint16 samplesPerPixel = 0;
    if (dataset->findAndGetUint16(DCM_SamplesPerPixel, samplesPerPixel).good())
    {
        std::cout << "SamplesPerPixel: " << samplesPerPixel << std::endl;
    }
    else { std::runtime_error("samplesPerPixel not found.\r\n"); }

    // slope
    double slope = 0.0;
    if (dataset->findAndGetFloat64(DCM_RescaleSlope, slope).good())
    {
        std::cout << "Slope: " << slope << std::endl;
    }
    else { std::runtime_error("slope not found.\r\n"); }
    //intercept
    double intercept = 0.0;
    if (dataset->findAndGetFloat64(DCM_RescaleIntercept, intercept).good())
    {
        std::cout << "Intercept: " << intercept << std::endl;
    }
    else { std::runtime_error("intercept not found.\r\n"); }

    //BitsAllocated
    Uint16 bitsAllocated = 0;
    if (dataset->findAndGetUint16(DCM_BitsAllocated, bitsAllocated).good())
    {
        std::cout << "BitsAllocated: " << bitsAllocated << std::endl;
    }
    else { std::runtime_error("bitsAllocated not found.\r\n"); }

    // PixelSpacing
    OFString pixelSpacing;
    if (dataset->findAndGetOFString(DCM_PixelSpacing, pixelSpacing).good())
    {
        std::cout << "PixelSpacing: " << pixelSpacing << std::endl;
    }
    else { std::runtime_error("pixelSpacing not found.\r\n"); }

    // SliceThickness
    OFString sliceThickness;
    if (dataset->findAndGetOFString(DCM_SliceThickness, sliceThickness).good())
    {
        std::cout << "SliceThickness: " << sliceThickness << std::endl;
    }
    else { std::runtime_error("sliceThickness not found.\r\n"); }

    // pixelRepresentation
    Uint16 pixelRepresentation = 0;
    if (dataset->findAndGetUint16(DCM_PixelRepresentation, pixelRepresentation).good())
    {
        std::cout << "PixelRepresentation: " << pixelRepresentation << std::endl;
    }
    else { std::runtime_error("pixelRepresentation not found.\r\n"); }

    //  highbit
    Uint16 highbit = 0;
    if (dataset->findAndGetUint16(DCM_HighBit, highbit).good())
    {
        std::cout << "HighBit: " << highbit << std::endl;
    }
    else { std::runtime_error("highbit not found.\r\n"); }



    // DcmElement* pixelDataElement = nullptr;
    // dataset->findAndGetElement(DCM_PixelData, pixelDataElement);
    // if (pixelDataElement == nullptr)
    // {
    //     std::cout << "Get Pixel Data Error: " << status.text() << std::endl;
    //     return false;
    // }
    // pixelDataElement->getUint8Array(pixelDataUint8);
    // if (pixelDataUint8 == nullptr)
    // {
    //     std::cout << "Get Pixel Data Error: " << status.text() << std::endl;
    //     return false;
    // }

    return true;
}

const Uint8* VolumeRender::getPixelUint8()
{
    return pixelDataUint8;
}

const Uint16* VolumeRender::getPixelUint16()
{
    return pixelDataUint16;
}

bool VolumeRender::getPixelRGBA(std::string path, int& width, int& height, int& numSlice, unsigned char*& rgba)
{
    rgba = new unsigned char[width * height * numSlice * 4];
    // tags
    double windowCenter = 0.0;
    double windowWidth = 0.0;
    // int maxValue = INT_MIN, minValue = INT_MAX;
    int maxValue = 4355, minValue = 937;
    double slope = 0.0;
    double intercept = 0.0;
    OFString modality;
    for (size_t index = 0; index < numSlice; index++)
    {
        std::stringstream ss;
        ss << std::setw(2) << std::setfill('0') << index;
        std::string fileIdx;
        ss >> fileIdx;
        std::string filePath = path + "CT0000" + fileIdx + ".dcm";
        // std::cout << "Load Dimcom File: " << filePath << std::endl;

        // DICOM读入
        DcmFileFormat fileformat;
        OFCondition status = fileformat.loadFile(filePath.c_str());
        DcmDataset* dataset = fileformat.getDataset();
        if (!status.good())
        {
            std::cout << "Load Dimcom File Error: " << filePath << std::endl;
        }

        if (index == 0) {
            // 获取图像宽高
            DicomImage* image = new DicomImage(filePath.c_str());
            if (image == nullptr)
            {
                std::cout << "Load Dimcom File Error: " << filePath << std::endl;
                return false;
            }
            if (image->getStatus() != EIS_Normal)
            {
                std::cout << "Load Dimcom File Error: " << DicomImage::getString(image->getStatus()) << std::endl;
                return false;
            }
            width = static_cast<int>(image->getWidth());
            height = static_cast<int>(image->getHeight());
            std::cout << "Image Width: " << width << std::endl;
            std::cout << "Image Height: " << height << std::endl;

            // window_center
            if (dataset->findAndGetFloat64(DCM_WindowCenter, windowCenter).good())
            {
                std::cout << "Window Center: " << windowCenter << std::endl;
            }
            else { std::runtime_error("windowCenter not found.\r\n"); }

            // window_width
            if (dataset->findAndGetFloat64(DCM_WindowWidth, windowWidth).good())
            {
                std::cout << "Window Width: " << windowWidth << std::endl;
            }
            else { std::runtime_error("windowWidth not found.\r\n"); }

            // modality
            if (dataset->findAndGetOFString(DCM_Modality, modality).good())
            {
                std::cout << "Modality: " << modality << std::endl;
            }
            else { std::runtime_error("modality not found.\r\n"); }
        }

        // 获取像素数据
        unsigned long numPixelsUint16 = 0;
        const Uint16* pixelData = nullptr;
        dataset->findAndGetUint16Array(DCM_PixelData, pixelData, &numPixelsUint16);
        // dataset->findAndGetSint16Array(DCM_PixelData, pixelData, &numPixels);
        if (pixelData == nullptr)
        {
            std::cout << "Get Pixel Data Error: " << status.text() << std::endl;
            return false;
        }
        // std::cout << "Get Pixel Data numPixelsUint16: " << numPixelsUint16 << std::endl;

        // max and min value
        for (int i = 0; i < width * height; ++i)
        {
            if (pixelData[i] > maxValue) maxValue = pixelData[i];
            if (pixelData[i] < minValue) minValue = pixelData[i];
        }

        // slope
        if (dataset->findAndGetFloat64(DCM_RescaleSlope, slope).good())
        {
            std::cout << "Slope: " << slope << std::endl;
        }
        else { std::runtime_error("slope not found.\r\n"); }
        //intercept
        if (dataset->findAndGetFloat64(DCM_RescaleIntercept, intercept).good())
        {
            std::cout << "Intercept: " << intercept << std::endl;
        }
        else { std::runtime_error("intercept not found.\r\n"); }

        //if (modality == "CT" && 225 < maxValue)
        //{
        //    minValue = -125;
        //    maxValue = 225;
        //    windowCenter = (maxValue + minValue) / 2;
        //    windowWidth = maxValue - minValue;
        //}
        //std::cout << "Window Center: " << windowCenter << std::endl;
        //std::cout << "Window Width: " << windowWidth << std::endl;

        for (int i = 0; i < width * height; ++i)
        {
            unsigned char value = static_cast<unsigned char>(pixelData[i] * slope + intercept);
            float intensity = static_cast<float>(value - windowCenter) / windowWidth + 0.5f;
            if (intensity < 0.0f) intensity = 0.0f;
            if (intensity > 1.0f) intensity = 1.0f;
            rgba[index * width * height * 4 + i * 4 + 0] = static_cast<unsigned char>(intensity * 255);
            rgba[index * width * height * 4 + i * 4 + 1] = static_cast<unsigned char>(intensity * 255);
            rgba[index * width * height * 4 + i * 4 + 2] = static_cast<unsigned char>(intensity * 255);
            rgba[index * width * height * 4 + i * 4 + 3] = 255;
        }

    }
    std::cout << "Max Value: " << maxValue << std::endl;
    std::cout << "Min Value: " << minValue << std::endl;





    return true;
}
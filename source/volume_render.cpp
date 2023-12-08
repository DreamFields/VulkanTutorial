#include "volume_render.h"
VolumeRender::VolumeRender(/* args */)
{
}

VolumeRender::~VolumeRender()
{
}

bool VolumeRender::loadDicom(std::string path, int numSlice /*= 0*/)
{
	// folder path
	dicomTags.folderPath = path;
	dicomTags.numSlice = numSlice;
	dicomTags.maxVal = INT_MIN;
	dicomTags.minVal = INT_MAX;
	dicomTags.fileIndex.clear();
	dicomTags.fileIndex.resize(numSlice);
	std::vector<std::pair<int, int>> fileIndex(numSlice);
	for (size_t index = 0; index < numSlice; index++)
	{
		std::stringstream ss;
		ss << std::setw(2) << std::setfill('0') << index;
		std::string fileIdx;
		ss >> fileIdx;
		std::string filePath = path + "CT0000" + fileIdx + ".dcm";
		DcmFileFormat fileformat;
		OFCondition status = fileformat.loadFile(filePath.c_str());
		DcmDataset* dataset = fileformat.getDataset();
		if (!status.good())
		{
			std::cout << "Load Dimcom File Error: " << filePath << std::endl;
		}
		DicomImage* image = new DicomImage(filePath.c_str());
		if (index == 0) {
			// 获取图像宽高
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
			dicomTags.boxWidth = static_cast<int>(image->getWidth());
			dicomTags.boxHeight = static_cast<int>(image->getHeight());

			// window_center
			if (!dataset->findAndGetFloat64(DCM_WindowCenter, dicomTags.windowCenter).good())
			{
				std::runtime_error("windowCenter not found.\r\n");
			}

			// window_width
			if (!dataset->findAndGetFloat64(DCM_WindowWidth, dicomTags.windowWidth).good())
			{
				std::runtime_error("windowWidth not found.\r\n");
			}

			// slope
			if (!dataset->findAndGetFloat64(DCM_RescaleSlope, dicomTags.rescaleSlope).good())
			{
				std::runtime_error("slope not found.\r\n");
			}
			//intercept
			if (!dataset->findAndGetFloat64(DCM_RescaleIntercept, dicomTags.rescaleIntercept).good())
			{
				std::runtime_error("intercept not found.\r\n");
			}
		}
		// max and min value
		const DiPixel* pix = (image->getInterData());
		EP_Representation rep = pix->getRepresentation();
		Sint16* pixelData = (Sint16*)pix->getData();
		unsigned long numPixels = pix->getCount();
		for (int i = 0; i < dicomTags.boxWidth * dicomTags.boxHeight; ++i)
		{
			Sint16 value = static_cast<Sint16> (pixelData[i]);
			if (value > dicomTags.maxVal) dicomTags.maxVal = value;
			if (value < dicomTags.minVal) dicomTags.minVal = value;
		}
		// ImagePositionPatient
		OFString imagePositionPatient;
		if (!dataset->findAndGetOFString(DCM_ImagePositionPatient, imagePositionPatient, 2).good())
		{
			std::runtime_error("imagePositionPatient not found.\r\n");
		}
		fileIndex[index].first = index;
		fileIndex[index].second = std::stoi(imagePositionPatient.c_str());
	}
	std::sort(fileIndex.begin(), fileIndex.end(), [](std::pair<int, int> a, std::pair<int, int> b) {return a.second < b.second; });
	// cout fileIndex
	for (size_t index = 0; index < numSlice; index++)
	{
		std::cout << fileIndex[index].first << " " << fileIndex[index].second << std::endl;
	}
	for (size_t index = 0; index < numSlice; index++)
	{
		dicomTags.fileIndex[index] = fileIndex[index].first;
	}

	// cout dicomtags
	std::cout << "Folder Path: " << dicomTags.folderPath << std::endl;
	std::cout << "Num Slice: " << dicomTags.numSlice << std::endl;
	std::cout << "Box Width: " << dicomTags.boxWidth << std::endl;
	std::cout << "Box Height: " << dicomTags.boxHeight << std::endl;
	std::cout << "Window Center: " << dicomTags.windowCenter << std::endl;
	std::cout << "Window Width: " << dicomTags.windowWidth << std::endl;
	std::cout << "Rescale Slope: " << dicomTags.rescaleSlope << std::endl;
	std::cout << "Rescale Intercept: " << dicomTags.rescaleIntercept << std::endl;
	std::cout << "Max Value: " << dicomTags.maxVal << std::endl;
	std::cout << "Min Value: " << dicomTags.minVal << std::endl;
	std::cout << "File Index: ";
	for (size_t index = 0; index < numSlice; index++)
	{
		std::cout << dicomTags.fileIndex[index] << " ";
	}

	dicomParamControl.windowCenter = dicomTags.windowCenter;
	dicomParamControl.windowWidth = dicomTags.windowWidth;
	dicomParamControl.tau = 0.003f;
	dicomParamControl.steps = 800;

	return true;
}

bool VolumeRender::getPixelRGBA(int& width, int& height, int& numSlice, unsigned char*& rgba)
{
	width = dicomTags.boxWidth;
	height = dicomTags.boxHeight;
	numSlice = dicomTags.numSlice;
	rgba = new unsigned char[width * height * numSlice * 4];
	std::string path = dicomTags.folderPath;
	for (size_t i = 0; i < numSlice; i++)
	{
		int index = dicomTags.fileIndex[i];
		std::stringstream ss;
		ss << std::setw(2) << std::setfill('0') << index;
		std::string fileIdx;
		ss >> fileIdx;
		std::string filePath = path + "CT0000" + fileIdx + ".dcm";

		// DICOM读入
		DicomImage* image = new DicomImage(filePath.c_str());

		// get instance uid
		DcmFileFormat fileformat;
		OFCondition status = fileformat.loadFile(filePath.c_str());
		DcmDataset* dataset = fileformat.getDataset();
		OFString instanceUID;
		if (!dataset->findAndGetOFString(DCM_SOPInstanceUID, instanceUID).good())
		{
			std::runtime_error("instanceUID not found.\r\n");
		}
		std::cout << "Instance UID: " << instanceUID << std::endl;
		if (image == nullptr)
		{
			std::cout << "Load Dimcom File Error: " << filePath << std::endl;
			return false;
		}
		std::cout << "Load Dimcom File: " << filePath << std::endl;

		// 获取像素数据
		const DiPixel* pix = (image->getInterData());
		EP_Representation rep = pix->getRepresentation();
		// void* pixelData = nullptr;
		unsigned long numPixels = 0;
		Sint16* pixelData = (Sint16*)pix->getData();
		numPixels = pix->getCount();

		//if (modality == "CT" && 225 < maxValue)
		//{
		//	//int tmpminValue = minValue * slope + intercept;
		//	//int tmpmaxValue = maxValue * slope + intercept;
		//	//int tmpminValue = minValue;
		//	//int tmpmaxValue = maxValue;
		//	int tmpminValue = -125;
		//	int tmpmaxValue = 225;
		//	windowCenter = (tmpmaxValue + tmpminValue) / 2;
		//	windowWidth = tmpmaxValue - tmpminValue;
		//}

			for (int j = 0; j < width * height; ++j)
			{
				// Sint16 value = static_cast<Sint16> (pixelData[j]);
				// double intensity = static_cast<double>((value - dicomTags.windowCenter) / dicomTags.windowWidth) + 0.5f;
				// if (intensity < 0.0f) intensity = 0.0f;
				// if (intensity > 1.0f) intensity = 1.0f;
				// rgba[index * width * height * 4 + j * 4 + 0] = static_cast<unsigned char>(intensity * 255);
				// rgba[index * width * height * 4 + j * 4 + 1] = static_cast<unsigned char>(intensity * 255);
				// rgba[index * width * height * 4 + j * 4 + 2] = static_cast<unsigned char>(intensity * 255);
				// rgba[index * width * height * 4 + j * 4 + 3] = 255;

				int value = static_cast<int> (pixelData[j]);
				value += +abs(dicomTags.minVal);
				rgba[index * width * height * 4 + j * 4 + 0] = static_cast<unsigned char>(value & 0xff);
				rgba[index * width * height * 4 + j * 4 + 1] = static_cast<unsigned char>((value >> 8) & 0xff);
				rgba[index * width * height * 4 + j * 4 + 2] = 255;
				rgba[index * width * height * 4 + j * 4 + 3] = 255;
			}

	}

	return true;
}

DicomTags VolumeRender::getDicomTags()
{
	return dicomTags;
}
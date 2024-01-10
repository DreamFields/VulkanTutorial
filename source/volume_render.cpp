#include "volume_render.h"
VolumeRender::VolumeRender(/* args */)
{
}

VolumeRender::~VolumeRender()
{
}

bool VolumeRender::loadDicom(std::string path)
{
	std::vector<std::string> allFiles = GetAllFilesInDirectory(path);
	int numSlice = allFiles.size();
	// folder path
	dicomTags.folderPath = path;
	dicomTags.numSlice = numSlice;
	dicomTags.maxVal = INT_MIN;
	dicomTags.minVal = INT_MAX;
	dicomTags.fileIndex.clear();
	dicomTags.fileIndex.resize(numSlice);
	dicomTags.voxelSize = glm::vec3(1.0f, 1.0f, 1.0f);
	dicomTags.realSize = glm::vec3(1.0f, 1.0f, 1.0f);
	dicomTags.boxSize = glm::vec3(1.0f, 1.0f, 1.0f);
	dicomTags.voxelResolution = glm::vec3(1.0f, 1.0f, static_cast<float>(numSlice));
	std::vector<std::pair<std::string, float>> fileIndex(numSlice);
	bool isHaveImageIndex = false;
	bool isResetWindowWW_WL = true;
	for (size_t index = 0; index < numSlice; index++)
	{
		// std::stringstream ss;
		// ss << std::setw(2) << std::setfill('0') << index;
		// std::string fileIdx;
		// ss >> fileIdx;
		// std::string filePath = path + "CT0000" + fileIdx + ".dcm";
		std::string filePath = allFiles[index];
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
			// dicomTags.voxelResolution[0] = static_cast<int>(image->getWidth());
			// dicomTags.voxelResolution[1] = static_cast<int>(image->getHeight());
			dicomTags.voxelResolution[0] = static_cast<float>(image->getWidth());
			dicomTags.voxelResolution[1] = static_cast<float>(image->getHeight());

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

			// pixelSpacing
			OFString pixelSpacing0;
			if (!dataset->findAndGetOFString(DCM_PixelSpacing, pixelSpacing0, 0).good())
			{
				std::runtime_error("pixelSpacing not found.\r\n");
			}
			dicomTags.voxelSize[0] = std::stof(pixelSpacing0.c_str());
			// std::cout << "Pixel Spacing0 = " << pixelSpacing0 << std::endl;

			OFString pixelSpacing1;
			if (!dataset->findAndGetOFString(DCM_PixelSpacing, pixelSpacing1, 1).good())
			{
				std::runtime_error("pixelSpacing not found.\r\n");
			}
			dicomTags.voxelSize[1] = std::stof(pixelSpacing1.c_str());
			// std::cout << "Pixel Spacing1 = " << pixelSpacing1 << std::endl;

			// thickness
			OFString sliceThickness;
			if (!dataset->findAndGetOFString(DCM_SliceThickness, sliceThickness).good())
			{
				std::runtime_error("sliceThickness not found.\r\n");
			}
			dicomTags.voxelSize[2] = std::stof(sliceThickness.c_str());
			// std::cout << "Slice Thickness = " << sliceThickness << std::endl;

			// realSize
			dicomTags.realSize[0] = dicomTags.voxelSize[0] * dicomTags.voxelResolution[0];
			dicomTags.realSize[1] = dicomTags.voxelSize[1] * dicomTags.voxelResolution[1];
			dicomTags.realSize[2] = dicomTags.voxelSize[2] * dicomTags.numSlice;
			std::cout << "Real Size = " << dicomTags.realSize[0] << ", " << dicomTags.realSize[1] << ", " << dicomTags.realSize[2] << std::endl;

			// boxGeometry
			glm::float32 maxSize = std::max(dicomTags.realSize[0], std::max(dicomTags.realSize[1], dicomTags.realSize[2]));
			dicomTags.boxSize[0] = dicomTags.realSize[0] / maxSize;
			dicomTags.boxSize[1] = dicomTags.realSize[1] / maxSize;
			dicomTags.boxSize[2] = dicomTags.realSize[2] / maxSize;
			std::cout << "Box Size = " << dicomTags.boxSize[0] << ", " << dicomTags.boxSize[1] << ", " << dicomTags.boxSize[2] << std::endl;

			// get image index
			OFString imageIndex;
			if (!dataset->findAndGetOFString(DCM_ImageIndex, imageIndex).good())
			{
				isHaveImageIndex = false;
			}
			else isHaveImageIndex = true;
		}

		// max and min value
		const DiPixel* pix = (image->getInterData());
		EP_Representation rep = pix->getRepresentation();
		Sint16* pixelData = (Sint16*)pix->getData();
		unsigned long numPixels = pix->getCount();
		for (int i = 0; i < dicomTags.voxelResolution[0] * dicomTags.voxelResolution[1]; ++i)
		{
			Sint16 value = static_cast<Sint16> (pixelData[i]);
			if (value > dicomTags.maxVal) dicomTags.maxVal = value;
			if (value < dicomTags.minVal) dicomTags.minVal = value;
		}

		if (isHaveImageIndex) {
			// get image index
			OFString imageIndex;
			dataset->findAndGetOFString(DCM_ImageIndex, imageIndex);
			// std::cout << "cur file: " << filePath << std::endl;
			// std::cout << "Image Index: " << imageIndex << std::endl;

			fileIndex[index].first = filePath.c_str();
			fileIndex[index].second = std::stoi(imageIndex.c_str());
		}
		else {
			// ImagePositionPatient
			OFString imagePositionPatient;
			if (!dataset->findAndGetOFString(DCM_ImagePositionPatient, imagePositionPatient, 2).good())
			{
				std::runtime_error("imagePositionPatient not found.\r\n");
			}
			// fileIndex[index].second = imagePositionPatient.c_str();
			fileIndex[index].first = filePath.c_str();
			fileIndex[index].second = std::stod(imagePositionPatient.c_str());
		}

		// begin test
		OFString imagePositionPatient0, imagePositionPatient1, imagePositionPatient2;
		if (!dataset->findAndGetOFString(DCM_ImagePositionPatient, imagePositionPatient0, 0).good())
		{
			std::runtime_error("imagePositionPatient0 not found.\r\n");
		}
		if (!dataset->findAndGetOFString(DCM_ImagePositionPatient, imagePositionPatient1, 1).good())
		{
			std::runtime_error("imagePositionPatient1 not found.\r\n");
		}
		if (!dataset->findAndGetOFString(DCM_ImagePositionPatient, imagePositionPatient2, 2).good())
		{
			std::runtime_error("imagePositionPatient2 not found.\r\n");
		}
		// std::cout << "Image Position Patient = " << imagePositionPatient0 << ", " << imagePositionPatient1 << ", " << imagePositionPatient2 << std::endl;
		// std::cout << "--------------------------------" << std::endl;

	}
	std::sort(fileIndex.begin(), fileIndex.end(), [](std::pair<std::string, int> a, std::pair<std::string, int> b) {return a.second < b.second; });
	// cout fileIndex
	for (size_t index = 0; index < numSlice; index++)
	{
		std::cout << fileIndex[index].first << " " << fileIndex[index].second << std::endl;
	}
	for (size_t index = 0; index < numSlice; index++)
	{
		dicomTags.fileIndex[index] = fileIndex[index].first;
	}

	// 重新设置windowCenter和windowWidth
	if (isResetWindowWW_WL) {
		dicomTags.windowWidth = 2000.0f;
		dicomTags.windowCenter = 50.0f;
	}

	// cout dicomtags
	std::cout << "Folder Path: " << dicomTags.folderPath << std::endl;
	std::cout << "Num Slice: " << dicomTags.numSlice << std::endl;
	std::cout << "Box Width: " << dicomTags.voxelResolution[0] << std::endl;
	std::cout << "Box Height: " << dicomTags.voxelResolution[1] << std::endl;
	std::cout << "Window Center: " << dicomTags.windowCenter << std::endl;
	std::cout << "Window Width: " << dicomTags.windowWidth << std::endl;
	std::cout << "Rescale Slope: " << dicomTags.rescaleSlope << std::endl;
	std::cout << "Rescale Intercept: " << dicomTags.rescaleIntercept << std::endl;
	std::cout << "Max Value: " << dicomTags.maxVal << std::endl;
	std::cout << "Min Value: " << dicomTags.minVal << std::endl;
	std::cout << "File Index: ";
	// for (size_t index = 0; index < numSlice; index++)
	// {
	// 	std::cout << dicomTags.fileIndex[index] << " ";
	// }

	dicomParamControl.windowCenter = static_cast<float>(dicomTags.windowCenter);
	dicomParamControl.windowWidth = static_cast<float>(dicomTags.windowWidth);
	dicomParamControl.alphaCorrection = 25.0f;
	dicomParamControl.steps = 130;
	dicomParamControl.stepLength = 0.01f;
	dicomParamControl.glow = 2.5f;

	return true;
}

bool VolumeRender::getPixelRGBA(int& width, int& height, int& numSlice, unsigned char*& rgba,short channel)
{
	width = dicomTags.voxelResolution[0];
	height = dicomTags.voxelResolution[1];
	numSlice = dicomTags.numSlice;
	rgba = new unsigned char[width * height * numSlice * channel];
	// std::string path = dicomTags.folderPath;
	// std::vector<std::string> allFiles = GetAllFilesInDirectory(path);
	for (size_t i = 0; i < numSlice; i++)
	{
		// int index = dicomTags.fileIndex[i];
		// std::stringstream ss;
		// ss << std::setw(2) << std::setfill('0') << index;
		// std::string fileIdx;
		// ss >> fileIdx;
		// std::string filePath = path + "CT0000" + fileIdx + ".dcm";
		std::string filePath = dicomTags.fileIndex[i];

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
		// std::cout << "Instance UID: " << instanceUID << std::endl;
		// if (image == nullptr)
		// {
		// 	std::cout << "Load Dimcom File Error: " << filePath << std::endl;
		// 	return false;
		// }
		// std::cout << "Load Dimcom File: " << filePath << std::endl;

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
			// 未排序
			// int value = static_cast<int> (pixelData[j]);
			// value += +abs(dicomTags.minVal);
			// rgba[index * width * height * 4 + j * 4 + 0] = static_cast<unsigned char>(value & 0xff);
			// rgba[index * width * height * 4 + j * 4 + 1] = static_cast<unsigned char>((value >> 8) & 0xff);
			// rgba[index * width * height * 4 + j * 4 + 2] = 255;
			// rgba[index * width * height * 4 + j * 4 + 3] = 255;

			// 排序
			int value = static_cast<int> (pixelData[j]);
			value += +abs(dicomTags.minVal);
			rgba[i * width * height * channel + j * channel + 0] = static_cast<unsigned char>(value & 0xff);
			rgba[i * width * height * channel + j * channel + 1] = static_cast<unsigned char>((value >> 8) & 0xff);
		}

	}

	return true;
}

DicomTags VolumeRender::getDicomTags()
{
	return dicomTags;
}

const std::vector<Vertex> VolumeRender::getBoxVertices() {
	const std::vector<Vertex> vertices = {
		{{0.0f, 									0.0f,									 static_cast<float>(dicomTags.boxSize[2])}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		{{static_cast<float>(dicomTags.boxSize[0]), 0.0f,									 static_cast<float>(dicomTags.boxSize[2])}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{static_cast<float>(dicomTags.boxSize[0]), static_cast<float>(dicomTags.boxSize[1]),static_cast<float>(dicomTags.boxSize[2])}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
		{{0.0f, 									static_cast<float>(dicomTags.boxSize[1]),static_cast<float>(dicomTags.boxSize[2])}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
		{{0.0f, 									0.0f,									 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		{{static_cast<float>(dicomTags.boxSize[0]), 0.0f,									 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
		{{static_cast<float>(dicomTags.boxSize[0]), static_cast<float>(dicomTags.boxSize[1]),0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		{{0.0f, 									static_cast<float>(dicomTags.boxSize[1]),0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}
	};

	// const std::vector<Vertex> vertices = {
	//     {{0.0f, 0.0f,1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
	//     {{1.0f, 0.0f,1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	//     {{1.0f, 1.0f,1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
	//     {{0.0f, 1.0f,1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
	//     {{0.0f, 0.0f,0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
	//     {{1.0f, 0.0f,0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
	//     {{1.0f, 1.0f,0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
	//     {{0.0f, 1.0f,0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}
	// };

	return vertices;
}

double VolumeRender::GetDiagonal() {
	return glm::sqrt(dicomTags.realSize[0] * dicomTags.realSize[0] + dicomTags.realSize[1] * dicomTags.realSize[1] + dicomTags.realSize[2] * dicomTags.realSize[2]);
}

void VolumeRender::initExtCoefficients() {
	basegaussianlevel = 1.0f;
}

void VolumeRender::GenerateConeSamples() {
	DestroyConeSamples();

	// init cone sampler
	initExtCoefficients();

	// Cone Occlusion
	// bind_cone_occlusion_vars = true;
	glsl_apply_occlusion = true;
	// glsl_occ_sectionsinfo = nullptr
	// sampler_occlusion.SetUIWeightPercentage(0.350f);
	sampler_occlusion.SetConeHalfAngle(20.0);
	sampler_occlusion.SetMaxGaussianPacking(ConeGaussianSampler::CONEPACKING::_3);
	sampler_occlusion.SetCoveredDistance(GetDiagonal() * 0.5f);
	sampler_occlusion.SetInitialStep(3.0f * basegaussianlevel);

	std::cout << "size of OcclusionUniformBufferObject" << sizeof(OcclusionUniformBufferObject) << std::endl;

	// 1. generate cone sections info
	sampler_occlusion.ComputeConeIntegrationSteps(basegaussianlevel);
	// glsl_occ_sectionsinfo = sampler_occlusion.GetConeSectionsInfoTex();

}

void VolumeRender::DestroyConeSamples() {

}
#pragma once
#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmimgle/dcmimage.h"
#include "struct.h"
#include "coneGaussianSampler.h"

#include <vtkNrrdReader.h>
//vtkSmartPointer 
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageCast.h>

#include <glm.hpp>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>
#include <math.h>

// 将NRRD文件的数据也转换到这个结构体中
struct DicomTags {
    std::string folderPath;
    std::vector<std::string> fileIndex;
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
    int steps = 200;
    float stepLength;
    float glow;
    float attenuation=0.0f;
    int falloffID=1;
};



class VolumeRender
{
private:
    DicomTags dicomTags;
    int currentExampleID;
public:
    DicomParamControl dicomParamControl;

    // Ext Coefficients
    float basegaussianlevel;

    // Cone Lighting Parameters
    bool glsl_apply_occlusion;
    // gl::Texture1D* glsl_occ_sectionsinfo;
    ConeGaussianSampler sampler_occlusion;

    // ground truth
    float m_u_light_ray_initial_step;
    float m_u_light_ray_step_size;
    int m_occ_num_rays_sampled; // 次级光线的数量
    float m_occ_cone_aperture_angle; // 次级光线的覆盖角度
    float m_occ_cone_distance_eval; // 次级光线的覆盖距离
    std::vector<glm::vec3> occ_kernel_vectors; // 次级光线的采样方向
public:
    VolumeRender(int exampleID);
    ~VolumeRender();

    bool loadDicom(std::string path); // dcmtk
    bool loadNRRD(std::string path); // vtk
    bool loadDICOM(std::string path); // vtk
    bool getNRRDPixelRGBA(int& width, int& height, int& numSlice, unsigned char*& rgba, short channel);
    bool getDICOMPixelRGBA(int& width, int& height, int& numSlice, unsigned char*& rgba, short channel);
    bool getPixelRGBA(int& width, int& height, int& numSlice, unsigned char*& rgba, short channel);
    DicomTags getDicomTags();
    const std::vector<Vertex> getBoxVertices();
    double GetDiagonal();

    // Ext Coefficients
    void initExtCoefficients();

    // Cone Lighting
    void GenerateConeSamples();
    void DestroyConeSamples();

    // ground truth(single scattering path tracing)
    void GenerateGroundTruthRay();
};



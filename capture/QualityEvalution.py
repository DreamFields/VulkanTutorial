
import cv2
import numpy as np
import math
from skimage.metrics import structural_similarity

# 测试两张图片的PSNR值
def psnr(img1, img2):
    mse = np.mean((img1 - img2)**2) # 均方误差
    if mse == 0:
        return 100
    PIXEL_MAX = 255.0
    return 20*math.log10(PIXEL_MAX/math.sqrt(mse))

# 测试两张图片的SSIM值
def ssim(img1, img2):
    return structural_similarity(img1, img2,channel_axis=2) #channel_axis参数指定了颜色通道在图像数组中的位置。在这个函数中，channel_axis=2表示颜色通道是图像数组的第三个维度。

if __name__ == '__main__':
    PT_0 = cv2.imread('capture/PT_0.png')

    # Gaussian0,只有第0层进行了高斯模糊
    Cone4_0 = cv2.imread('capture/Gaussian0/Cone4_0.png')
    Cone4_intensity_0 = cv2.imread('capture/Gaussian0/Cone4_intensity_0.png')
    Cone14_0 = cv2.imread('capture/Gaussian0/Cone14_0.png')
    Cone14_intensity_0 = cv2.imread('capture/Gaussian0/Cone14_intensity_0.png')
    
    # psnr
    # print(psnr(PT_0, Cone4_0)) #39.355689110408406
    # print(psnr(PT_0, Cone4_intensity_0)) #39.40280895526096
    # print(psnr(PT_0, Cone14_0)) #39.49104835533386
    # print(psnr(PT_0, Cone14_intensity_0)) #39.57041110801239

    # ssim
    # print(ssim(PT_0, Cone4_0)) #0.9843733793572578
    # print(ssim(PT_0, Cone4_intensity_0)) #0.9853546963898095
    # print(ssim(PT_0, Cone14_0)) #0.9854573488696623
    # print(ssim(PT_0, Cone14_intensity_0)) #0.986568126785349
    print(ssim(Cone14_0, Cone14_intensity_0)) #0.9992832640299332

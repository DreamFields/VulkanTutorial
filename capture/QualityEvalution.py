
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
    Example1_Cone10_intensity = cv2.imread('capture/Gaussian0/Example1_Cone10_intensity.png')
    Example1_Cone10 = cv2.imread('capture/Gaussian0/Example1_Cone10.png')
    
    # psnr
    # print(psnr(PT_0, Example1_Cone10)) #41.19564316224877
    # print(psnr(PT_0, Example1_Cone10_intensity)) #41.1715885826881

    # ssim
    # print(ssim(PT_0, Example1_Cone10)) #0.9904818940658605
    print(ssim(PT_0, Example1_Cone10_intensity)) #0.9911353996601653

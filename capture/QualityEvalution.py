
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


# 创建一张新的图片，大小与img1和img2相同，显示的是两张图片的差异，差异越大，颜色越红，差异越小，颜色越蓝
def diff(img1, img2):
    img1 = cv2.cvtColor(img1, cv2.COLOR_BGR2GRAY) # 转换为灰度图
    img2 = cv2.cvtColor(img2, cv2.COLOR_BGR2GRAY)
    diff = cv2.absdiff(img1, img2) # 计算两张图片的差异
    diff = cv2.threshold(diff, 0, 255, cv2.THRESH_BINARY_INV | cv2.THRESH_OTSU)[1] # 二值化
    diff = cv2.cvtColor(diff, cv2.COLOR_GRAY2BGR) # 转换为BGR图
    cv2.imwrite('capture/Gaussian0/diff.png', diff)
    return diff

if __name__ == '__main__':
    PT_0 = cv2.imread('capture/PT_0.png')
    PT_0_large = cv2.imread('capture/PT_0_large.png')
    PT_1 = cv2.imread('capture/PT_1.png')

    # Gaussian0,只有第0层进行了高斯模糊
    Example1_Cone10_intensity_0 = cv2.imread('capture/Gaussian0/Example1_Cone10_intensity_0.png')
    Example1_Cone10_0 = cv2.imread('capture/Gaussian0/Example1_Cone10_0.png')
    Example1_Cone10_large_intensity_0 = cv2.imread('capture/Gaussian0/Example1_Cone10_large_intensity_0.png')
    Example1_Cone10_large_0 = cv2.imread('capture/Gaussian0/Example1_Cone10_large_0.png') 

    Example1_Cone10_1 = cv2.imread('capture/Gaussian0/Example1_Cone10_1.png')
    Example1_Cone10_intensity_1 = cv2.imread('capture/Gaussian0/Example1_Cone10_intensity_1.png')
    
    # psnr
    # print(psnr(PT_0, Example1_Cone10_0)) #41.19564316224877
    # print(psnr(PT_0, Example1_Cone10_intensity_0)) #41.1715885826881
    # 图像越大，PSNR值越小
    # print(psnr(PT_0_large, Example1_Cone10_large_0)) #38.145132031029725
    # print(psnr(PT_0_large, Example1_Cone10_large_intensity_0)) #38.115230249950955
    # 多个角度的图像，是否使用intensity存储，对PSNR值影响不大
    # print(psnr(PT_1, Example1_Cone10_intensity_1)) #40.281781309378374
    # print(psnr(PT_1, Example1_Cone10_1)) #40.19909398894565

    # ssim
    # print(ssim(PT_0, Example1_Cone10)) #0.9904818940658605
    # print(ssim(PT_0, Example1_Cone10_intensity)) #0.9911353996601653
    # print(ssim(PT_0_large, Example1_Cone10_large)) #0.9833840725927138
    # print(ssim(PT_0_large, Example1_Cone10_large_intensity)) #0.984821021641029

    # diff
    diff(PT_1, Example1_Cone10_1)

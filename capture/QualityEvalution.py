
import cv2
import numpy as np
import math
from skimage.metrics import structural_similarity

curExampleID = 0
angleID = 1

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
    cv2.imwrite('capture/Gaussian0/example'+str(curExampleID)+'/diff/diff.png', diff)
    return diff

if __name__ == '__main__':
    curPT = cv2.imread('capture/PT/example'+str(curExampleID)+'/angle'+str(angleID)+'.png') 
    angleX = cv2.imread('capture/Gaussian0/example'+str(curExampleID)+'/angle'+str(angleID)+'.png')
    angleX_intensity = cv2.imread('capture/Gaussian0/example'+str(curExampleID)+'/angle'+str(angleID)+'_intensity.png')
    # print(psnr(curPT, angle0)) #37.44070642943377
    # print(ssim(curPT, angle0)) #0.9693760312983354
    # diff(curPT, angle0)

    # print(psnr(curPT, angle0_intensity)) #38.05124064030779
    # print(ssim(curPT, angle0_intensity)) #0.9647451026433513
    # diff(curPT, angle0_intensity)

    # print(psnr(curPT, angleX)) #36.309629798918394
    # print(ssim(curPT, angleX)) #0.9698171643386634
    # diff(curPT, angleX)

    print(psnr(curPT, angleX_intensity)) #37.92412527586112
    print(ssim(curPT, angleX_intensity)) #0.974892510737444
    diff(curPT, angleX_intensity)

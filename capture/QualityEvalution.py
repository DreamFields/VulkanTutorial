
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
    angle1_highRes = cv2.imread('capture/Gaussian0/example'+str(curExampleID)+'/angle'+str(angleID)+'_highRes.png')
    angle1_sigm_fixed_highRes = cv2.imread('capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_fixed_highRes.png')
    angle1_sigm_fixed_highRes_Cone8 = cv2.imread('capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_fixed_highRes_Cone8.png')
    angle1_sigm_fixed = cv2.imread('capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_fixed.png')
    angle1_sigm_vary = cv2.imread('capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_vary.png')
    angle1_sigm_fixed_lod= cv2.imread('capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_fixed_lod.png')
    angle1_sigm_vary_lod = cv2.imread('capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_vary_lod.png')

    angleX_intensity = cv2.imread('capture/Gaussian0/example'+str(curExampleID)+'/angle'+str(angleID)+'_intensity.png')
    angle1_sigm_fixed_intensity = cv2.imread('capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_fixed_intensity.png')
    angle1_sigm_vary_intensity = cv2.imread('capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_vary_intensity.png')
    angle1_sigm_fixed_intensity_lod = cv2.imread('capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_fixed_intensity_lod.png')
    angle1_sigm_vary_intensity_lod = cv2.imread('capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_vary_intensity_lod.png')


    print("----------psnr-----------")
    print(psnr(curPT, angleX)) #36.309629798918394
    print(psnr(curPT, angle1_highRes)) # 36.81266191148056
    print(psnr(curPT, angle1_sigm_fixed_highRes)) # 36.74755612418086
    print(psnr(curPT, angle1_sigm_fixed_highRes_Cone8)) # 36.352496083533794
    print(psnr(curPT, angle1_sigm_fixed)) #36.61542375432933
    print(psnr(curPT, angle1_sigm_vary)) #36.27420600443534
    print(psnr(curPT, angle1_sigm_fixed_lod)) #36.61542375432933
    print(psnr(curPT, angle1_sigm_vary_lod)) #36.24794802172406
    # print("----------psnr_intensity-----------")
    # print(psnr(curPT, angleX_intensity)) #37.92412527586112
    # print(psnr(curPT, angle1_sigm_fixed_intensity)) #37.89633017011268
    # print(psnr(curPT, angle1_sigm_vary_intensity)) #! 38.198993179552005
    # print(psnr(curPT, angle1_sigm_fixed_intensity_lod)) #37.95013853543789
    # print(psnr(curPT, angle1_sigm_vary_intensity_lod)) #38.15523852205405

    print("----------ssim-----------")
    print(ssim(curPT, angleX)) #0.9698171643386634
    print(ssim(curPT, angle1_highRes)) #0.9785990552768546
    print(ssim(curPT, angle1_sigm_fixed_highRes)) #0.9785904879706786
    print(ssim(curPT, angle1_sigm_fixed_highRes_Cone8)) #0.9746597403066031
    print(ssim(curPT, angle1_sigm_fixed)) #0.9735029343157304
    print(ssim(curPT, angle1_sigm_vary)) #0.9698835268702423
    print(ssim(curPT, angle1_sigm_fixed_lod)) #0.9735029343157304
    print(ssim(curPT, angle1_sigm_vary_lod)) #0.9692338226701316
    # print("----------ssim_intensity-----------")
    # print(ssim(curPT, angleX_intensity)) #! 0.974892510737444
    # print(ssim(curPT, angle1_sigm_fixed_intensity)) #0.9738586277718134
    # print(ssim(curPT, angle1_sigm_vary_intensity)) #0.9725451701517486
    # print(ssim(curPT, angle1_sigm_fixed_intensity_lod)) #0.9738369617295769
    # print(ssim(curPT, angle1_sigm_vary_intensity_lod)) #0.9723973537708711
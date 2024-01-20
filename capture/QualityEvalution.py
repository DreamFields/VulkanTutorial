
import cv2
import numpy as np
import math
from skimage.metrics import structural_similarity
from PIL import Image
import lpips
import torch

curExampleID = 0
angleID = 2

# 测试两张图片的PSNR值，值越大，表示两张图片越相似
""" 
针对彩色图像，通常用以下三种方法来计算。
1.分别计算 RGB 三个通道的 PSNR，然后取平均值。
2.计算 RGB 三通道的 MSE ，然后再除以 3 。
3.将图片转化为 YCbCr 格式，然后只计算 Y 分量也就是亮度分量的 PSNR。
其中，第二和第三种方法比较常见。 
"""
def psnr(image1_path, image2_path):
    img1 = cv2.imread(image1_path, cv2.IMREAD_UNCHANGED)
    img2 = cv2.imread(image2_path, cv2.IMREAD_UNCHANGED)

    # 分别计算 RGB 三个通道的 PSNR，然后取平均值。
    # img1 = img1.astype(float) # 转换为浮点数
    # img2 = img2.astype(float)
    # mse1 = np.mean((img1[:, :, 0] - img2[:, :, 0])**2) # 均方误差
    # mse2 = np.mean((img1[:, :, 1] - img2[:, :, 1])**2)
    # mse3 = np.mean((img1[:, :, 2] - img2[:, :, 2])**2)
    # mse = (mse1 + mse2 + mse3) / 3

    #* 计算 RGB 三通道的 MSE ，然后再除以 3
    img1 = img1.astype(float) # 转换为浮点数
    img2 = img2.astype(float)
    # mse = np.mean((img1[:, :, :3] - img2[:, :, :3])**2) / 3 # 均方误差
    mse = np.mean((img1[:, :, :4] - img2[:, :, :4])**2) / 4 # 均方误差

    # 这里将图片转化为灰度图，只测试一个通道的值，废弃
    # img1 = cv2.cvtColor(img1, cv2.COLOR_BGR2GRAY) # 转换为灰度图
    # img2 = cv2.cvtColor(img2, cv2.COLOR_BGR2GRAY)
    # mse = np.mean((img1 - img2)**2) # 均方误差

    # 将图片转化为 YCbCr 格式，然后只计算 Y 分量也就是亮度分量的 PSNR
    # img1 = cv2.cvtColor(img1, cv2.COLOR_BGR2YCrCb) # 转换为YCbCr图
    # img2 = cv2.cvtColor(img2, cv2.COLOR_BGR2YCrCb)
    # mse = np.mean((img1[:, :, 0] - img2[:, :, 0])**2) # 均方误差

    if mse == 0:
        return 100
    PIXEL_MAX = 255.0
    return 20*math.log10(PIXEL_MAX/math.sqrt(mse))

# 测试两张图片的SSIM值，值越大，表示两张图片越相似
def ssim(image1_path, image2_path):
    img1 = cv2.imread(image1_path, cv2.IMREAD_UNCHANGED)
    img2 = cv2.imread(image2_path, cv2.IMREAD_UNCHANGED)
    return structural_similarity(img1, img2,channel_axis=2) #channel_axis参数指定了颜色通道在图像数组中的位置。在这个函数中，channel_axis=2表示颜色通道是图像数组的第三个维度。

# 测试两张图片的LPIPS值，值越小，表示两张图片越相似
def LPIPS(image1_path, image2_path):
    img1 = lpips.im2tensor(lpips.load_image(image1_path)) # RGB image from [-1,1]
    img2 = lpips.im2tensor(lpips.load_image(image2_path))

    # Load the model
    loss_fn = lpips.LPIPS(net='alex') # best forward scores
    # loss_fn = lpips.LPIPS(net='vgg') # closer to "traditional" perceptual loss, when used for optimization

    # Compute distance
    dist01 = loss_fn(img1, img2)
    return dist01.item()

# 创建一张新的图片，大小与img1和img2相同，显示的是两张图片的差异，差异越大，颜色越红，差异越小，颜色越蓝
def diff(image1_path, image2_path):
    img1 = cv2.imread(image1_path)
    img2 = cv2.imread(image2_path)
    img1 = cv2.cvtColor(img1, cv2.COLOR_BGR2GRAY) # 转换为灰度图
    img2 = cv2.cvtColor(img2, cv2.COLOR_BGR2GRAY)
    diff = cv2.absdiff(img1, img2) # 计算两张图片的差异
    diff = cv2.threshold(diff, 0, 255, cv2.THRESH_BINARY_INV | cv2.THRESH_OTSU)[1] # 二值化
    diff = cv2.cvtColor(diff, cv2.COLOR_GRAY2BGR) # 转换为BGR图
    cv2.imwrite('capture/Gaussian0/example'+str(curExampleID)+'/diff/diff.png', diff)
    return diff

def testAngle1():
    angleID = 1
    curPT = 'capture/PT/example'+str(curExampleID)+'/angle'+str(angleID)+'.png' 

    angleX = 'capture/Gaussian0/example'+str(curExampleID)+'/angle'+str(angleID)+'.png'
    angle1_highRes = 'capture/Gaussian0/example'+str(curExampleID)+'/angle'+str(angleID)+'_highRes.png'
    angle1_sigm_fixed_highRes = 'capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_fixed_highRes.png'
    angle1_sigm_fixed_highRes_Cone8 = 'capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_fixed_highRes_Cone8.png'
    angle1_sigm_fixed = 'capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_fixed.png'
    angle1_sigm_vary = 'capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_vary.png'
    angle1_sigm_fixed_lod= 'capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_fixed_lod.png'
    angle1_sigm_vary_lod = 'capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_vary_lod.png'

    angleX_intensity = 'capture/Gaussian0/example'+str(curExampleID)+'/angle'+str(angleID)+'_intensity.png'
    angle1_sigm_fixed_intensity = 'capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_fixed_intensity.png'
    angle1_sigm_vary_intensity = 'capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_vary_intensity.png'
    angle1_sigm_fixed_intensity_lod = 'capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_fixed_intensity_lod.png'
    angle1_sigm_vary_intensity_lod = 'capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_vary_intensity_lod.png'


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

"""
测试方法：角度+是否使用intensity+是否使用高分辨率
"""
def testHeadAngle2():
    angleID = 2
    curPT = 'capture/PT/example'+str(curExampleID)+'/angle'+str(angleID)+'.png'

    angle2_sigm_fixed_lowRes = 'capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_fixed_lowRes.png'
    angle2_sigm_vary_lowRes = 'capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_vary_lowRes.png'
    angle2_sigm_fixed_highRes = 'capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_fixed_highRes.png'
    angle2_sigm_vary_highRes = 'capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_vary_highRes.png'

    angle2_sigm_fixed_lowRes_intensity = 'capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_fixed_lowRes_intensity.png'
    angle2_sigm_vary_lowRes_intensity = 'capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_vary_lowRes_intensity.png'
    angle2_sigm_fixed_highRes_intensity = 'capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_fixed_highRes_intensity.png'
    angle2_sigm_vary_highRes_intensity = 'capture/GaussianMip/example'+str(curExampleID)+'/angle'+str(angleID)+'_sigm_vary_highRes_intensity.png'

    print("----------psnr-----------")
    # print(psnr(curPT, angle2_sigm_fixed_lowRes)) # 34.236170970305004 
    print(psnr(curPT, angle2_sigm_vary_lowRes)) # 33.9792714836159
    # print(psnr(curPT, angle2_sigm_fixed_highRes)) # 35.896154543860625
    print(psnr(curPT, angle2_sigm_vary_highRes)) # 35.77892888310162
    print("----------psnr_intensity-----------")
    # print(psnr(curPT, angle2_sigm_fixed_lowRes_intensity)) # 
    print(psnr(curPT, angle2_sigm_vary_lowRes_intensity)) # 31.45566439924559
    # print(psnr(curPT, angle2_sigm_fixed_highRes_intensity)) # 
    print(psnr(curPT, angle2_sigm_vary_highRes_intensity)) # 39.60437649787264
    # 结论：高分辨率intensity>高分辨率高低8位>低分辨率高低8位>低分辨率intensity

    print("----------ssim-----------")
    # print(ssim(curPT, angle2_sigm_fixed_lowRes)) # 
    print(ssim(curPT, angle2_sigm_vary_lowRes)) # 0.9857400794862187
    # print(ssim(curPT, angle2_sigm_fixed_highRes)) # 
    print(ssim(curPT, angle2_sigm_vary_highRes)) # 0.9892738495638302
    print("----------ssim_intensity-----------")
    # print(ssim(curPT, angle2_sigm_fixed_lowRes_intensity)) # 
    print(ssim(curPT, angle2_sigm_vary_lowRes_intensity)) # 0.9770597676123973
    # print(ssim(curPT, angle2_sigm_fixed_highRes_intensity)) # 
    print(ssim(curPT, angle2_sigm_vary_highRes_intensity)) # 0.9943743766202002
    # 结论：高分辨率intensity>高分辨率高低8位>低分辨率高低8位>低分辨率intensity

    print("----------LPIPS-----------")
    # print(LPIPS(curPT, angle2_sigm_fixed_lowRes)) # 
    print(LPIPS(curPT, angle2_sigm_vary_lowRes)) # 0.025452062487602234
    # print(LPIPS(curPT, angle2_sigm_fixed_highRes)) # 
    print(LPIPS(curPT, angle2_sigm_vary_highRes)) # 0.01834018900990486
    print("----------LPIPS_intensity-----------")
    # print(LPIPS(curPT, angle2_sigm_fixed_lowRes_intensity)) # 
    print(LPIPS(curPT, angle2_sigm_vary_lowRes_intensity)) # 0.04378775879740715
    # print(LPIPS(curPT, angle2_sigm_fixed_highRes_intensity)) # 
    print(LPIPS(curPT, angle2_sigm_vary_highRes_intensity)) # 0.008849206380546093
    # 结论：高分辨率intensity>高分辨率高低8位>低分辨率高低8位>低分辨率intensity

def testPSNR():
    print("----------psnr-----------")

if __name__ == '__main__':
    # testAngle1()
    testHeadAngle2()

import cv2
import numpy as np
import math
from skimage.metrics import structural_similarity
from PIL import Image
import lpips
import torch

# 测试配置
def setConfig(curID, angleID, wlwwID):
    curExampleID = curID # 测试用例ID
    angleID = angleID # 角度ID
    wlwwID = wlwwID # 窗宽窗位ID
    basePath = 'capture/example'+str(curExampleID)+'/angle'+str(angleID)+'/wlww'+str(wlwwID)+'/' # 测试用例路径
    return basePath

# 测试两张图片的PSNR值，值越大，表示两张图片越相似
""" 
针对彩色图像，通常用以下三种方法来计算。
1.分别计算 RGB 三个通道的 PSNR，然后取平均值。
2.计算 RGB 三通道的 MSE ，然后再除以 3 。
3.将图片转化为 YCbCr 格式，然后只计算 Y 分量也就是亮度分量的 PSNR。
其中，第二和第三种方法比较常见。 
"""
def psnr(image1_path, image2_path):
    if image1_path == '' or image2_path == '':
        return 0
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
    if image1_path == '' or image2_path == '':
        return 0
    img1 = cv2.imread(image1_path, cv2.IMREAD_UNCHANGED)
    img2 = cv2.imread(image2_path, cv2.IMREAD_UNCHANGED)
    return structural_similarity(img1, img2,channel_axis=2) #channel_axis参数指定了颜色通道在图像数组中的位置。在这个函数中，channel_axis=2表示颜色通道是图像数组的第三个维度。

# 测试两张图片的LPIPS值，值越小，表示两张图片越相似
def LPIPS(image1_path, image2_path):
    if image1_path == '' or image2_path == '':
        return 0
    img1 = lpips.im2tensor(lpips.load_image(image1_path)) # RGB image from [-1,1]
    img2 = lpips.im2tensor(lpips.load_image(image2_path))

    # Load the model
    loss_fn = lpips.LPIPS(net='alex') # best forward scores
    # loss_fn = lpips.LPIPS(net='vgg') # closer to "traditional" perceptual loss, when used for optimization

    # Compute distance
    dist01 = loss_fn(img1, img2)
    # 禁止输出warning
    torch.autograd.set_detect_anomaly(True)
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

"""
测试方法：是否使用intensity+是否使用高分辨率
"""
def mainTestHeadAngle1():
    basePath = setConfig(0,1,0)
    curPT = basePath+'PT.png'

    fixed_lowRes = basePath+'fixed_lowRes.png'
    vary_lowRes = basePath+'vary_lowRes.png'
    fixed_highRes = basePath+'fixed_highRes.png'
    vary_highRes = basePath+'vary_highRes.png'

    fixed_lowRes_intensity = basePath+'fixed_lowRes_intensity.png'
    vary_lowRes_intensity = basePath+'vary_lowRes_intensity.png'
    fixed_highRes_intensity = basePath+'fixed_highRes_intensity.png'
    vary_highRes_intensity = basePath+'vary_highRes_intensity.png'

    print("----------psnr-----------")
    # print(psnr(curPT, fixed_lowRes)) # 34.236170970305004 
    print(psnr(curPT, vary_lowRes)) # 33.9792714836159
    # print(psnr(curPT, fixed_highRes)) # 35.896154543860625
    print(psnr(curPT, vary_highRes)) # 35.77892888310162
    print("----------psnr_intensity-----------")
    # print(psnr(curPT, fixed_lowRes_intensity)) # 
    print(psnr(curPT, vary_lowRes_intensity)) # 31.45566439924559
    # print(psnr(curPT, fixed_highRes_intensity)) # 
    print(psnr(curPT, vary_highRes_intensity)) # 39.60437649787264
    # 结论：高分辨率intensity>高分辨率高低8位>低分辨率高低8位>低分辨率intensity

    print("----------ssim-----------")
    # print(ssim(curPT, fixed_lowRes)) # 
    print(ssim(curPT, vary_lowRes)) # 0.9857400794862187
    # print(ssim(curPT, fixed_highRes)) # 
    print(ssim(curPT, vary_highRes)) # 0.9892738495638302
    print("----------ssim_intensity-----------")
    # print(ssim(curPT, fixed_lowRes_intensity)) # 
    print(ssim(curPT, vary_lowRes_intensity)) # 0.9770597676123973
    # print(ssim(curPT, fixed_highRes_intensity)) # 
    print(ssim(curPT, vary_highRes_intensity)) # 0.9943743766202002
    # 结论：高分辨率intensity>高分辨率高低8位>低分辨率高低8位>低分辨率intensity

    print("----------LPIPS-----------")
    # print(LPIPS(curPT, fixed_lowRes)) # 
    print(LPIPS(curPT, vary_lowRes)) # 0.025452062487602234
    # print(LPIPS(curPT, fixed_highRes)) # 
    print(LPIPS(curPT, vary_highRes)) # 0.01834018900990486
    print("----------LPIPS_intensity-----------")
    # print(LPIPS(curPT, fixed_lowRes_intensity)) # 
    print(LPIPS(curPT, vary_lowRes_intensity)) # 0.04378775879740715
    # print(LPIPS(curPT, fixed_highRes_intensity)) # 
    print(LPIPS(curPT, vary_highRes_intensity)) # 0.008849206380546093
    # 结论：高分辨率intensity>高分辨率高低8位>低分辨率高低8位>低分辨率intensity

def gaussianTest():
    basePath = setConfig(0,0,0)
    PT = basePath+'PT.png'
    lowRes = basePath+'lowRes.png'
    highRes = basePath+'highRes.png'
    lowRes_intensity = basePath+'lowRes_intensity.png'
    highRes_intensity = basePath+'highRes_intensity.png'
    highRes_intensity_combine = basePath+'highRes_intensity_combine.png' # 双边高斯滤波的结果
    highRes_intensity_split = basePath+'highRes_intensity_split.png' # 拆分高斯滤波的结果
    highRes_intensity_split_combine = basePath+'highRes_intensity_split_combine.png' # 拆分、双边高斯滤波的结果
    highRes_lpls = basePath+'highRes_lpls.png' # 拉普拉斯滤波的结果（并未更改积分的过程，因此不准确）

    print("----------psnr-----------")
    print(psnr(PT, lowRes)) # 
    print(psnr(PT, highRes)) #
    print("----------psnr_intensity-----------")
    print(psnr(PT, lowRes_intensity)) #
    print(psnr(PT, highRes_intensity)) #
    print(psnr(PT, highRes_intensity_combine)) #
    print(psnr(PT, highRes_intensity_split)) #
    print(psnr(PT, highRes_intensity_split_combine)) #
    print(psnr(PT, highRes_lpls)) #

    print("----------ssim-----------")
    print(ssim(PT, lowRes)) #
    print(ssim(PT, highRes)) #
    print("----------ssim_intensity-----------")
    print(ssim(PT, lowRes_intensity)) #
    print(ssim(PT, highRes_intensity)) #
    print(ssim(PT, highRes_intensity_combine)) #
    print(ssim(PT, highRes_intensity_split)) #
    print(ssim(PT, highRes_intensity_split_combine)) #
    print(ssim(PT, highRes_lpls)) #

    print("----------LPIPS-----------")
    print(LPIPS(PT, lowRes)) #
    print(LPIPS(PT, highRes)) #
    print("----------LPIPS_intensity-----------")
    print(LPIPS(PT, lowRes_intensity)) #
    print(LPIPS(PT, highRes_intensity)) #
    print(LPIPS(PT, highRes_intensity_combine)) #
    print(LPIPS(PT, highRes_intensity_split)) #
    print(LPIPS(PT, highRes_intensity_split_combine)) #
    print(LPIPS(PT, highRes_lpls)) #


def mainTestHead(curExampleID, angleID, wlwwID):
    basePath = setConfig(curExampleID, angleID, wlwwID)
    PT = basePath+'PT.png'
    idao = basePath + "lowRes_intensity.png"
    ours = basePath + "highRes_intensity_split.png"

    print("----------psnr-----------")
    print(psnr(PT, idao)) #
    print(psnr(PT, ours)) #
    print("----------ssim-----------")
    print(ssim(PT, idao)) #
    print(ssim(PT, ours)) #
    print("----------LPIPS-----------")
    print(LPIPS(PT, idao)) #
    print(LPIPS(PT, ours)) #

def mainTestMouse(curExampleID, angleID, wlwwID):
    basePath = setConfig(curExampleID, angleID, wlwwID)
    PT = basePath+'PT.png'
    idao = basePath + "origin.png"
    ours128 = basePath + "ours128.png"
    ours128_detail = basePath + "ours128_detail.png"
    # ours256 = basePath + "ours256.png"
    # ours256_detail = basePath + "ours256_detail.png"

    print("----------psnr-----------")
    print(psnr(PT, idao)) #
    print(psnr(PT, ours128)) #
    print(psnr(PT, ours128_detail)) #
    # print(psnr(PT, ours256)) #
    # print(psnr(PT, ours256_detail)) #
    print("----------ssim-----------")
    print(ssim(PT, idao)) #
    print(ssim(PT, ours128)) #
    print(ssim(PT, ours128_detail)) #
    # print(ssim(PT, ours256)) #
    # print(ssim(PT, ours256_detail)) #
    print("----------LPIPS-----------")
    print(LPIPS(PT, idao)) #
    print(LPIPS(PT, ours128)) #
    print(LPIPS(PT, ours128_detail)) #
    # print(LPIPS(PT, ours256)) #
    # print(LPIPS(PT, ours256_detail)) #

def mainTestAllExample(exampleID):
    basePath = 'capture/allExample/example'+str(exampleID)+'/'
    PT = basePath+'PT.png'
    IDAO = basePath+'IDAO.png'
    Ours = basePath+'Ours.png'

    print("----------psnr-----------")
    print(psnr(PT, IDAO)) #
    print(psnr(PT, Ours)) #
    print("----------ssim-----------")
    print(ssim(PT, IDAO)) #
    print(ssim(PT, Ours)) #
    print("----------LPIPS-----------")
    print(LPIPS(PT, IDAO)) #
    print(LPIPS(PT, Ours)) #

def mainTestAttenuation(exampleID):
    basePath = 'capture/allExample/example'+str(exampleID)+'/'
    PT = basePath+'PT.png'
    IDAO = basePath+'IDAO.png'
    Ours = basePath+'Ours.png'
    attenuationPath = 'capture/attenuation/example'+str(exampleID)+'/'
    at_2 = attenuationPath+'0.2.png'
    at_4 = attenuationPath+'0.4.png'
    at_6 = attenuationPath+'0.6.png'
    at_8 = attenuationPath+'0.8.png'
    at_10 = attenuationPath+'1.0.png'

    print("----------psnr-----------")
    print(psnr(PT, IDAO)) #
    print(psnr(PT, Ours)) #
    print(psnr(PT, at_2)) #
    print(psnr(PT, at_4)) #
    print(psnr(PT, at_6)) #
    print(psnr(PT, at_8)) #
    print(psnr(PT, at_10)) #
    print("----------ssim-----------")
    print(ssim(PT, IDAO)) #
    print(ssim(PT, Ours)) #
    print(ssim(PT, at_2)) #
    print(ssim(PT, at_4)) #
    print(ssim(PT, at_6)) #
    print(ssim(PT, at_8)) #
    print(ssim(PT, at_10)) #
    print("----------LPIPS-----------")
    print(LPIPS(PT, IDAO)) #
    print(LPIPS(PT, Ours)) #
    print(LPIPS(PT, at_2)) #
    print(LPIPS(PT, at_4)) #
    print(LPIPS(PT, at_6)) #
    print(LPIPS(PT, at_8)) #
    print(LPIPS(PT, at_10)) #

# exampleID=7
def mainTestFalloffFuncEx7_old(exampleID,falloffFuncID):
    basePath = 'capture/allExample/example'+str(exampleID)+'/'
    PT = basePath+'PT.png'
    IDAO = basePath+'IDAO.png'
    Ours = basePath+'Ours.png'
    falloffFuncPath = 'capture/falloffFunc_old/example'+str(exampleID)+'/func'+str(falloffFuncID)+'/'

    test0=''
    test1=''
    test2=''
    test3=''
    test4=''

    if falloffFuncID==0:
        test0 = falloffFuncPath+'10.png'
        test1 = falloffFuncPath+'20.png'
        test2 = falloffFuncPath+'30.png'
        test3 = falloffFuncPath+'40.png' # 好 31.64269519646496 0.9852977788445867 0.021769333630800247
        test4 = falloffFuncPath+'50.png' # 好 31.36236873838996 0.9852232374949281 0.021529026329517365
    elif falloffFuncID==1:
        test0 = falloffFuncPath+'0.00.png' # 好 32.539003217592665 0.9814613311967355 0.03026559203863144
        test1 = falloffFuncPath+'0.02.png'
        test2 = falloffFuncPath+'0.04.png'
        test3 = falloffFuncPath+'0.06.png'
        test4 = falloffFuncPath+'0.08.png'
    elif falloffFuncID==2:
        test0 = falloffFuncPath+'0.2.png'
        test1 = falloffFuncPath+'0.4.png'
        test2 = falloffFuncPath+'0.6.png'
        test3 = falloffFuncPath+'0.8.png'
        test4 = falloffFuncPath+'1.0.png' # 好 31.584380593507785 0.9831183175178744 0.027021821588277817
    elif falloffFuncID==3: # 无好结果
        test0 = falloffFuncPath+'0001.png'
        test1 = falloffFuncPath+'0002.png'
        test2 = falloffFuncPath+'0003.png'
        test3 = falloffFuncPath+'0004.png'
        test4 = falloffFuncPath+'0005.png'
    elif falloffFuncID==4:
        test0 = falloffFuncPath+'0.05.png'
        test1 = falloffFuncPath+'0.10.png'
        test2 = falloffFuncPath+'0.15.png'
        test3 = falloffFuncPath+'0.20.png'
        test4 = falloffFuncPath+'0.25.png' # 好 31.72809632497495 0.9826095879148253 0.028057480230927467
    elif falloffFuncID==5:
        test0 = falloffFuncPath+'0.11.png'
        test1 = falloffFuncPath+'0.12.png'
        test2 = falloffFuncPath+'0.13.png'
        test3 = falloffFuncPath+'0.14.png'
        test4 = falloffFuncPath+'0.15.png'
    elif falloffFuncID==6:
        test0 = falloffFuncPath+'0.1.png'
        test1 = falloffFuncPath+'0.2.png'
        test2 = falloffFuncPath+'0.3.png'
        test3 = falloffFuncPath+'0.4.png'
        test4 = falloffFuncPath+'0.5.png'
    # print(test0)
    # print(test1)
    # print(test2)
    # print(test3)
    # print(test4)
    
    print("----------psnr-----------")
    print(psnr(PT, IDAO)) #
    print(psnr(PT, Ours)) #
    print(psnr(PT, test0)) #
    print(psnr(PT, test1)) #
    print(psnr(PT, test2)) #
    print(psnr(PT, test3)) #
    print(psnr(PT, test4)) #
    print("----------ssim-----------")
    print(ssim(PT, IDAO)) #
    print(ssim(PT, Ours)) #
    print(ssim(PT, test0)) #
    print(ssim(PT, test1)) #
    print(ssim(PT, test2)) #
    print(ssim(PT, test3)) #
    print(ssim(PT, test4)) #
    print("----------LPIPS-----------")
    print(LPIPS(PT, IDAO)) #
    print(LPIPS(PT, Ours)) #
    print(LPIPS(PT, test0)) #
    print(LPIPS(PT, test1)) #
    print(LPIPS(PT, test2)) #
    print(LPIPS(PT, test3)) #
    print(LPIPS(PT, test4)) #

# exampleID=6
def mainTestFalloffFuncEx6_old(exampleID,falloffFuncID):
    basePath = 'capture/allExample/example'+str(exampleID)+'/'
    PT = basePath+'PT.png'
    IDAO = basePath+'IDAO.png'
    Ours = basePath+'Ours.png'
    falloffFuncPath = 'capture/falloffFunc_old/example'+str(exampleID)+'/func'+str(falloffFuncID)+'/'

    test0=''
    test1=''
    test2=''
    test3=''
    test4=''

    other0=''
    other1=''
    other2=''

    if falloffFuncID==0:
        test0 = falloffFuncPath+'10.png'
        test1 = falloffFuncPath+'15.png'
        test2 = falloffFuncPath+'20.png'
        test3 = falloffFuncPath+'25.png'
        test4 = falloffFuncPath+'30.png'
    elif falloffFuncID==1:
        test0 = falloffFuncPath+'0.01.png'
        test1 = falloffFuncPath+'0.02.png'
        test2 = falloffFuncPath+'0.03.png' # 好 39.26987423738397 0.9908747488322646 0.012129709124565125
        test3 = falloffFuncPath+'0.04.png' # 好 38.21336107904241 0.9906498341689867 0.011850327253341675
        test4 = falloffFuncPath+'0.05.png'
    elif falloffFuncID==2:
        test0 = falloffFuncPath+'0.4.png' # 好 39.539622059697514 0.9906919071056501 0.01236618310213089
        test1 = falloffFuncPath+'0.6.png' # 好 37.80811168136171 0.9907939878209141 0.011783579364418983
        test2 = falloffFuncPath+'0.8.png'
        test3 = falloffFuncPath+'1.0.png'
        test4 = falloffFuncPath+'1.2.png'
        other0 = falloffFuncPath+'0.1.png'
        other1 = falloffFuncPath+'0.2.png'
        other2 = falloffFuncPath+'0.3.png'
    elif falloffFuncID==3: 
        test0 = falloffFuncPath+'0.001.png' # 好 39.02081208468582 0.9900313124578364 0.013549860566854477
        test1 = falloffFuncPath+'0.002.png' # 好 39.47977196985502 0.9905413724772758 0.012616868130862713
        test2 = falloffFuncPath+'0.003.png' # 好 39.25127281196921 0.9905647429956845 0.012288231402635574
        test3 = falloffFuncPath+'0.004.png'
        test4 = falloffFuncPath+'0.005.png'
    elif falloffFuncID==4:
        test0 = falloffFuncPath+'0.1.png' # 好 38.976310044151525 0.9906733903617395 0.012755008414387703
        test1 = falloffFuncPath+'0.2.png' # 好 37.332291189281165 0.990600233490724 0.012212437577545643
        test2 = falloffFuncPath+'0.3.png'
        test3 = falloffFuncPath+'0.4.png'
        test4 = falloffFuncPath+'0.5.png'
    # print(test0)
    # print(test1)
    # print(test2)
    # print(test3)
    # print(test4)
    
    print("----------psnr-----------")
    print(psnr(PT, IDAO)) #
    print(psnr(PT, Ours)) #
    print(psnr(PT, test0)) #
    print(psnr(PT, test1)) #
    print(psnr(PT, test2)) #
    print(psnr(PT, test3)) #
    print(psnr(PT, test4)) #
    print("-")
    print(psnr(PT, other0)) #
    print(psnr(PT, other1)) #
    print(psnr(PT, other2)) #
    print("----------ssim-----------")
    print(ssim(PT, IDAO)) #
    print(ssim(PT, Ours)) #
    print(ssim(PT, test0)) #
    print(ssim(PT, test1)) #
    print(ssim(PT, test2)) #
    print(ssim(PT, test3)) #
    print(ssim(PT, test4)) #
    print("-")
    print(ssim(PT, other0)) #
    print(ssim(PT, other1)) #
    print(ssim(PT, other2)) #
    print("----------LPIPS-----------")
    print(LPIPS(PT, IDAO)) #
    print(LPIPS(PT, Ours)) #
    print(LPIPS(PT, test0)) #
    print(LPIPS(PT, test1)) #
    print(LPIPS(PT, test2)) #
    print(LPIPS(PT, test3)) #
    print(LPIPS(PT, test4)) #
    print("-")
    print(LPIPS(PT, other0)) #
    print(LPIPS(PT, other1)) #
    print(LPIPS(PT, other2)) #

# exampleID=7
def mainTestFalloffFuncEx7(exampleID,falloffFuncID):
    basePath = 'capture/allExample/example'+str(exampleID)+'/'
    PT = basePath+'PT.png'
    IDAO = basePath+'IDAO0.0003.png'
    Ours = basePath+'Ours0.0003.png'
    falloffFuncPath = 'capture/falloffFunc/example'+str(exampleID)+'/func'+str(falloffFuncID)+'/'

    test0=''
    test1=''
    test2=''
    test3=''
    test4=''

    other0=''
    other1=''
    other2=''

    if falloffFuncID==0:
        test0 = falloffFuncPath+'10.png'
        test1 = falloffFuncPath+'15.png'
        test2 = falloffFuncPath+'20.png'
        test3 = falloffFuncPath+'25.png'
        test4 = falloffFuncPath+'30.png'
    elif falloffFuncID==1:
        test0 = falloffFuncPath+'0.02.png'
        test1 = falloffFuncPath+'0.04.png'
        test2 = falloffFuncPath+'0.06.png'
        test3 = falloffFuncPath+'0.08.png'
        test4 = falloffFuncPath+'0.10.png'
        other0 = falloffFuncPath+'0.001.png'
        other1 = falloffFuncPath+'0.003.png'
        other2 = falloffFuncPath+'0.005.png'
    elif falloffFuncID==2:
        test0 = falloffFuncPath+'0.2.png'
        test1 = falloffFuncPath+'0.4.png'
        test2 = falloffFuncPath+'0.6.png'
        test3 = falloffFuncPath+'0.8.png'
        test4 = falloffFuncPath+'1.0.png' 
        other0 = falloffFuncPath+'0.10.png'
        other1 = falloffFuncPath+'0.2.png' 
        other2 = falloffFuncPath+'0.3.png'
    elif falloffFuncID==3: 
        test0 = falloffFuncPath+'0.002.png' #
        test1 = falloffFuncPath+'0.004.png'
        test2 = falloffFuncPath+'0.006.png'
        test3 = falloffFuncPath+'0.008.png'
        test4 = falloffFuncPath+'0.010.png'
        other0 = falloffFuncPath+'0.001.png' #
        other1 = falloffFuncPath+'0.002.png'
        other2 = falloffFuncPath+'0.003.png'
    elif falloffFuncID==4:
        test0 = falloffFuncPath+'0.02.png'
        test1 = falloffFuncPath+'0.04.png'
        test2 = falloffFuncPath+'0.06.png'
        test3 = falloffFuncPath+'0.08.png'
        test4 = falloffFuncPath+'0.10.png'
        other0 = falloffFuncPath+'0.10.png'
        other1 = falloffFuncPath+'0.15.png'
        other2 = falloffFuncPath+'0.20.png'
    elif falloffFuncID==5:
        test0 = falloffFuncPath+'0.02.png'
        test1 = falloffFuncPath+'0.04.png'
        test2 = falloffFuncPath+'0.06.png'
        test3 = falloffFuncPath+'0.08.png'
        test4 = falloffFuncPath+'0.10.png'
        other0 = falloffFuncPath+'0.04.png'
        other1 = falloffFuncPath+'0.06.png'
        other2 = falloffFuncPath+'0.08.png'
    elif falloffFuncID==6:
        test0 = falloffFuncPath+'0.002.png'
        test1 = falloffFuncPath+'0.004.png'
        test2 = falloffFuncPath+'0.006.png'
        test3 = falloffFuncPath+'0.008.png'
        test4 = falloffFuncPath+'0.010.png'
        other0 = falloffFuncPath+'0.001.png'
        other1 = falloffFuncPath+'0.0015.png'
        other2 = falloffFuncPath+'0.002.png'
    # print(test0)
    # print(test1)
    # print(test2)
    # print(test3)
    # print(test4)
    
    print("----------psnr-----------")
    print(psnr(PT, IDAO)) #
    print(psnr(PT, Ours)) #
    print("-")
    print(psnr(PT, test0)) #
    print(psnr(PT, test1)) #
    print(psnr(PT, test2)) #
    print(psnr(PT, test3)) #
    print(psnr(PT, test4)) #
    print("-")
    print(psnr(PT, other0)) #
    print(psnr(PT, other1)) #
    print(psnr(PT, other2)) #
    print("----------ssim-----------")
    print(ssim(PT, IDAO)) #
    print(ssim(PT, Ours)) #
    print("-")
    print(ssim(PT, test0)) #
    print(ssim(PT, test1)) #
    print(ssim(PT, test2)) #
    print(ssim(PT, test3)) #
    print(ssim(PT, test4)) #
    print("-")
    print(ssim(PT, other0)) #
    print(ssim(PT, other1)) #
    print(ssim(PT, other2)) #
    print("----------LPIPS-----------")
    print(LPIPS(PT, IDAO)) #
    print(LPIPS(PT, Ours)) #
    print("-")
    print(LPIPS(PT, test0)) #
    print(LPIPS(PT, test1)) #
    print(LPIPS(PT, test2)) #
    print(LPIPS(PT, test3)) #
    print(LPIPS(PT, test4)) #
    print("-")
    print(LPIPS(PT, other0)) #
    print(LPIPS(PT, other1)) #
    print(LPIPS(PT, other2)) #

   
def test():
    basePath = 'capture/allExample/example7/'
    PT = basePath+'PT.png'
    IDAO5 = basePath+'IDAO0.0005.png'
    IDAO3 = basePath+'IDAO0.0003.png'
    Ours5 = basePath+'Ours0.0005.png'
    Ours3 = basePath+'Ours0.0003.png'

    print("----------psnr-----------")
    print(psnr(PT, IDAO5)) #
    print(psnr(PT, IDAO3)) #
    print(psnr(PT, Ours5)) #
    print(psnr(PT, Ours3)) #
    print("----------ssim-----------")
    print(ssim(PT, IDAO5)) #
    print(ssim(PT, IDAO3)) #
    print(ssim(PT, Ours5)) #
    print(ssim(PT, Ours3)) #
    print("----------LPIPS-----------")
    print(LPIPS(PT, IDAO5)) #
    print(LPIPS(PT, IDAO3)) #
    print(LPIPS(PT, Ours5)) #
    print(LPIPS(PT, Ours3)) #


if __name__ == '__main__':
    # mainTestHead(1,0,0)
    # mainTestMouse(1,1,1)

    # # 测试用例的id从0到7
    # for i in range(8):
    #     print("=====================example"+str(i)+"=======================")
    #     mainTestAllExample(i) # 测试所有的测试用例 

    mainTestAllExample(6)

    # 测试attenuation
    # mainTestAttenuation(7)

    # test()

    # 测试falloffFunc
    # for i in range(7):
    #     if i==0:
    #         continue
    #     print("=====================func"+str(i)+"=======================")
    #     mainTestFalloffFuncEx7(7,i)
     # 打印黄色字体
    # print("\033[1;33m"+"hello world"+"\033[0m")
    # mainTestFalloffFuncEx7(7,6)
        
    # for i in range(5):
    #     if i==0:
    #         continue
    #     print("=====================func"+str(i)+"=======================")
    #     mainTestFalloffFuncEx6_old(6,i)
    # mainTestFalloffFuncEx6_old(6,2)
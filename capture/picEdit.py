# 将文件夹下的所有图片截取为指定大小，并保存到指定文件夹

import os
import cv2
import numpy as np
import shutil
from PIL import Image
import matplotlib.pyplot as plt

# 将文件夹下的所有图片截取为指定大小，并保存到指定文件夹

# 读取文件夹下的所有图片
def readAllImages(folderPath):
    # 获取文件夹下的所有文件
    files = os.listdir(folderPath)
    # 获取文件夹下的所有图片
    images = []
    for file in files:
        # 判断是否是图片
        if file.endswith('.png') or file.endswith('.jpg') or file.endswith('.jpeg'):
            images.append(file)
    return images

# 截取图片，要求距离顶部指定大小，距离左侧指定大小，截取后的图片大小为指定大小
'''
imagePath: 图片路径
padding: (top, left)
size: (height, width)
'''
def cropImage(imagePath, padding,size):
    # 读取图片，且具备png的透明度通道
    image = cv2.imread(imagePath, cv2.IMREAD_UNCHANGED)
    # image = cv2.imread(imagePath)
    # 截取图片
    cropImg = image[padding[0]:padding[0]+size[0], padding[1]:padding[1]+size[1]]
    return cropImg

# 保存图片
def saveImage(image, savePath, imageName):
    # 判断文件夹是否存在，不存在则创建
    if not os.path.exists(savePath):
        os.makedirs(savePath)
    # 更新图片名字
    imageName = imageName.split('.')[0] + '_crop.' + imageName.split('.')[1]
    print(imageName)
    # 保存图片
    cv2.imwrite(os.path.join(savePath, imageName), image)

# 将文件夹下的所有图片截取为指定大小，并保存到指定文件夹
def cropAllImages(folderPath, savePath,padding, size):
    # 读取文件夹下的所有图片
    images = readAllImages(folderPath)
    # 截取图片
    for image in images:
        # 截取图片
        cropImg = cropImage(os.path.join(folderPath, image),padding, size)
        # 保存图片
        saveImage(cropImg, savePath, image)

# 测试配置
def setConfig(curID, angleID, wlwwID):
    curExampleID = curID # 测试用例ID
    angleID = angleID # 角度ID
    wlwwID = wlwwID # 窗宽窗位ID
    basePath = 'capture/example'+str(curExampleID)+'/angle'+str(angleID)+'/wlww'+str(wlwwID)+'/' # 测试用例路径
    return basePath

# 主函数
# for head
folderPath = setConfig(0, 0, 0)
savePath = 'capture/paper_res'
padding = (330,650)
size = (600, 600)

# for mouse
# folderPath = setConfig(1, 0, 0)
# savePath = 'capture/paper_res'
# padding = (455,580)
# size = (440, 620)
cropAllImages(folderPath, savePath,padding, size)
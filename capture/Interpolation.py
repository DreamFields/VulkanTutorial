# originData = [0,40,80,40,-50,160,80,120,40,-50]
originData = [0,50,50,200,0,100,200,50,0]
# 生成原始索引，范围是0到1
originIndex = []
for i in range(len(originData)):
    # 保留两位小数
    originIndex.append(round(i / (len(originData) - 1), 2))

# 输出原始索引
print("原始索引：", originIndex)

# 生成intensityData，（val-100）/100+0.5，然后clamp到0和1之间
intensityData=[]
for i in range(len(originData)):
    # 对原始数据进行变换
    intensityData.append((originData[i] - 100) / 100 + 0.5)
    # 保留两位小数
    intensityData[i] = round(intensityData[i], 2)
    # clamp到0和1之间
    if intensityData[i] < 0:
        intensityData[i] = 0
    if intensityData[i] > 1:
        intensityData[i] = 1


# 生成100个采样索引，范围是0到1
interpolationIndex = []
for i in range(100):
    # 保留两位小数
    interpolationIndex.append(round(i / 100, 2))

# 生成插值数据，长度是100，每个元素的值是根据插值索引计算出来的
interpolationData1 = []
interpolationData2 = []
for i in range(len(interpolationIndex)):
    # 找到interpolationIndex[i]在originIndex中的位置
    for j in range(len(originIndex) - 1):
        if interpolationIndex[i] >= originIndex[j] and interpolationIndex[i] <= originIndex[j + 1]:
            # 对originData[j]和originData[j + 1]之间的值进行插值，得到interpolationData[i]
            # 计算插值系数
            factor = (interpolationIndex[i] - originIndex[j]) / (originIndex[j + 1] - originIndex[j])
            # 计算插值数据
            interpolationData1.append(originData[j] + (originData[j + 1] - originData[j]) * factor)
            interpolationData2.append(intensityData[j] + (intensityData[j + 1] - intensityData[j]) * factor)
            break

# 对插值数据进行下面的操作：（val-100）/100+0.5，然后clamp到0和1之间
for i in range(len(interpolationData1)):
    # 对插值数据进行变换
    interpolationData1[i] = (interpolationData1[i] - 100) / 100 + 0.5
    # 保留两位小数
    interpolationData1[i] = round(interpolationData1[i], 2)
    # clamp到0和1之间
    if interpolationData1[i] < 0:
        interpolationData1[i] = 0
    if interpolationData1[i] > 1:
        interpolationData1[i] = 1

# originData 每个元素除以200
for i in range(len(originData)):
    # 保留两位小数
    originData[i] = round(originData[i] / 200, 2)

print("originData:", originData)

# 插值数据1和2绘制成折线图，横轴是插值索引，纵轴是插值数据
import matplotlib.pyplot as plt
import numpy as np
# originData绘制成条形图，横轴是原始索引，纵轴是原始数据
# plt.bar(originIndex, originData, label='originData', color='b')
plt.plot(interpolationIndex, interpolationData1, label='interpolationData1', color='r')
plt.plot(interpolationIndex, interpolationData2, label='interpolationData2', color='g')
plt.plot(originIndex, originData, label='originData', color='b')
plt.legend()
plt.show()



import numpy as np
import matplotlib.pyplot as plt

# 生成示例数据
methods = ['IDAO', 'Ours']
conditions = ['Angle = front view, WLWW = bone', 'Angle = front view, WLWW = skin', 'Angle = side view, WLWW = bone', 'Angle = side view, WLWW = skin']
data = [
    # IDAO
    [29.929  , 0.967  , 0.050  , 58.6],
    [30.437  , 0.966  , 0.051  , 62.3],
    [31.456  , 0.977  , 0.044  , 57.8],
    [34.141  , 0.979  , 0.036  , 60.8],
    # Ours
    [38.275  , 0.990  , 0.013  , 58.0],
    [40.473  , 0.994  , 0.013  , 57.0],
    [38.621  , 0.993  , 0.010  , 57.7],
    [41.131  , 0.995  , 0.010  , 57.6]
]

# 设置每组数据中四个不相关值的纵轴比例
scales = np.array([1.0, 25.0, 200.0, 0.6])

# 为8个数据设置颜色，这些颜色自定义rgb值

colors = [
    (0.0, 0.0, 1.0),
    (0.0, 0.5, 0.0),
    (1.0, 0.0, 0.0),
    (0.0, 0.75, 0.75),
    (0.75, 0.0, 0.75),
    (0.75, 0.75, 0.0),
    (0.0, 0.0, 0.0),
    (0.75, 0.75, 0.75)
]

# colors = ['b', 'g', 'r', 'c', 'm', 'y', 'k', 'w']

print(data)
print(scales)

fig, axs = plt.subplots(2, 2, figsize=(12, 12))

for i, condition in enumerate(conditions):
    ax = axs[i//2, i%2] # 2行2列
    index = np.arange(4)
    bar_width = 0.35

    for j, method in enumerate(methods):
        print(data[i*2+j])
        # 为每个方法的四个数据设置不同的颜色
        # bars = ax.bar(index + j * bar_width, data[(i+j*4)%len(data)] * scales, bar_width, label=method,color=colors[i+j*4]) #
        bars = ax.bar(index + j * bar_width, data[(i+j*4)%len(data)] * scales, bar_width, label=method) #

        # 在每个柱子的顶部显示对应的数值
        for barIdx,bar in enumerate(bars):
            # bar = bars[barIdx]
            height = bar.get_height()
            heightValue = data[(i+j*4)%len(data)][barIdx]
            ax.text(bar.get_x() + bar.get_width() / 2, height, f'{heightValue:.3f}', ha='center', va='bottom')

    # 不显示纵轴
    ax.yaxis.set_visible(False)
    ax.set_title(f'{condition}')
    ax.set_xticks(index + bar_width / 2)
    ax.set_xticklabels(['PSNR', 'SSIM', 'LPIPS', 'FPS'])
    ax.legend()


plt.tight_layout()
plt.show()

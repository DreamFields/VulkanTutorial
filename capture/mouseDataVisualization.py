import numpy as np
import matplotlib.pyplot as plt

# 生成示例数据
methods = ['IDAO', 'Ours','Ours Acc']
conditions = ['Angle = right side, WLWW = skin', 'Angle = right side, WLWW = bone', 'Angle = left side, WLWW = skin', 'Angle = left side, WLWW = bone']
data = [
    # IDAO
    [39.073  , 0.987  , 0.015  , 50.3  ],
    [42.134  , 0.995  , 0.005  , 64.9  ],
    [42.058  , 0.991  , 0.011  , 50.6  ],
    [41.589  , 0.994  , 0.005  , 63.3  ],
    # Ours
    [36.525  , 0.986  , 0.017  , 284.5 ],
    [43.948  , 0.996  , 0.005  , 124.3 ],
    [40.447  , 0.991  , 0.012  , 163.8 ],
    [44.041  , 0.996  , 0.004  , 80.8  ],
    # Ours Acc
    [36.831  , 0.991  , 0.010  , 71.5  ],
    [44.184  , 0.997  , 0.004  , 52.6  ],
    [40.731  , 0.995  , 0.007  , 58.1  ],
    [44.308  , 0.997  , 0.003  , 44.7  ]
]

# 设置每组数据中四个不相关值的纵轴比例
scales = np.array([1.0, 25.0, 450.0, 0.2])

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
    bar_width = 0.25

    for j, method in enumerate(methods):
        print(data[i*3+j])
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
    ax.set_xticks(index + bar_width)
    ax.set_xticklabels(['PSNR', 'SSIM', 'LPIPS', 'FPS'])
    ax.legend()


plt.tight_layout()
plt.show()

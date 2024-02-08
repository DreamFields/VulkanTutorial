# 三维高斯分布可视化
import numpy as np
import matplotlib.pyplot as plt

# 三维高斯分布
def Gaussian(x, y, sigma):
    return (1 / (2 * np.pi * sigma ** 2)) * np.exp(-(x ** 2 + y ** 2) / (2 * sigma ** 2))

# 生成高斯分布数据
sigma = 1
x = np.linspace(-3, 3, 100)
y = np.linspace(-3, 3, 100)
X, Y = np.meshgrid(x, y)
Z = Gaussian(X, Y, sigma)

# 绘制高斯分布
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
ax.plot_surface(X, Y, Z, cmap='rainbow')

plt.show()
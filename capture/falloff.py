# 绘制float linearFalloff(float _distance, float k) {return (_distance / k);}的函数曲线

import matplotlib.pyplot as plt
import numpy as np

x = np.linspace(0, 10, 100)
y = x / 2

plt.plot(x, y)

plt.show()
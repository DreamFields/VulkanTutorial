import numpy as np
import matplotlib.pyplot as plt

# 定义k和shadowValue的值
k = 10
shadowValue = np.linspace(0, 10, 100)

# 计算y值
y = np.exp(-k * shadowValue)

# 画图
plt.plot(shadowValue, y)
plt.xlabel('shadowValue')
plt.ylabel('exp(-k * shadowValue)')
plt.title('Plot of exp(-k * shadowValue)')
plt.grid(True)
plt.show()
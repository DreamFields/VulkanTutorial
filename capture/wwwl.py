# 绘制窗宽窗位示意图
# Import libraries
import numpy as np
import matplotlib.pyplot as plt

# Generate some random data (replace with your data),且数据均大于0
my_variable = np.random.normal(80, 10, 2000)

# 只保留大于0的数据
my_variable = my_variable[my_variable > 0]
print("my_variable:", my_variable)

# Compute the histogram of `my_variable` with 40 bins and get the bin edges
my_hist, bin_edges = np.histogram(my_variable, bins=40)

print("my_hist:", my_hist)
print("bin_edges:", bin_edges)

# Define color thresholds
windowWidth = 80
windowCenter = 80

# Define colors for tails and center
lower_tail_color = np.array([169, 169, 169]) / 255.0
upper_tail_color = np.array([105,179, 162]) / 255.0 * 0.8
    
# Init the list containing the color of each bin.
colors = []

for data in my_hist:
    intensity = (data - windowCenter) / windowWidth + 0.5
    intensity = round(intensity, 2)
    # if intensity < 0:
    #     intensity = 0
    # if intensity > 1:
    #     intensity = 1
    
    if intensity <= 0:
        colors.append(lower_tail_color)
    
    # Dark gray: Assign a color to the bin if its edge is greater than or equal to 'upper_bound'
    elif intensity >= 1:
        colors.append(upper_tail_color)  
    
    # Purple: Assign a color to the bin if its edge is between -10 and 10
    # 根据intensity的值,
    else:
        colors.append(((intensity+1)/2 * upper_tail_color + (1 - intensity) * lower_tail_color) )
        # colors.append((intensity+1)/2 * upper_tail_color)
        
# Create a bar plot with specified colors and bin edges
plt.bar(
    bin_edges[:-1], # Remove the last bin edge
    # index, # Set the x values of the bars
    my_hist, # Set the height of the bars
    width=np.diff(bin_edges), # Set the width of the bars
    color=colors, 
    edgecolor='none' # Add a black edge to the bars
) 

# graph customization
plt.title('windowWidth = 80 , windowCenter = 80')  # Set the title of the plot
plt.xlabel('index')  # Set the label for the x-axis
xticks = np.arange(bin_edges[0], bin_edges[-1], 10)
print("xticks:", xticks)
index = []
for i in range(round(bin_edges[-1])):
    index.append(round(i*len(bin_edges)/len(xticks)))
    if len(index) == len(xticks):
        break
print("index:", index)
# 将横轴的刻度显示换为index，且不改变原始数据，只替换显示方式
plt.xticks(xticks, index)
plt.ylabel('intensity')  # Set the label for the y-axis

# 在图上画三个横线
plt.axhline(windowCenter+windowWidth/2, color='black', linestyle='--', linewidth=1)
plt.axhline(windowCenter, color='black', linestyle='--', linewidth=1)
plt.axhline(windowCenter-windowWidth/2, color='black', linestyle='--', linewidth=1)
# 并在横线上标注文字，文字颜色为黑色，文字的位置在横线右侧
# plt.text(bin_edges[-1], windowCenter+windowWidth/2, "windowCenter+windowWidth/2", color='black')
# plt.text(bin_edges[-1], windowCenter, "windowCenter", color='black')
# plt.text(bin_edges[-1], windowCenter-windowWidth/2, "windowCenter-windowWidth/2", color='black')

# 显示三种颜色的图例
plt.plot([], c=upper_tail_color, label='retenion intensity')
plt.plot([], c=lower_tail_color, label='cut off intensity')
# Add a legend
plt.legend()  # Add a legend to the plot

# Display the plot
plt.show()  # Display the plot
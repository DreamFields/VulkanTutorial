
import os
from PIL import Image


# 将文件夹下的图片只取第一行，然后保存到另一个文件夹中
def handle_image():
    fileNum=0
    path = "textures"
    for root, dirs, files in os.walk(path):
        for file in files:
            if file.endswith(".png") and not file.endswith("_handle.png"):
                img = Image.open(os.path.join(root, file))
                img = img.crop((0, 0, img.width, 1))
                img = img.resize((512, 20))
                # 保存的文件名为fileNum，保存到另一个文件夹"handle"中
                img.save(os.path.join("textures/handle", str(fileNum)+"_handle.png"))
                fileNum+=1      
                # img.save(os.path.join("textures/handle", file.split(".")[0]+"_handle.png"))
                print("Image", file, "handled.")                
    print("All images are handled.")
if __name__ == "__main__":
    handle_image()
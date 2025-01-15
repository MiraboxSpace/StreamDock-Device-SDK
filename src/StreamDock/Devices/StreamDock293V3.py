from .StreamDock import StreamDock
from PIL import Image
import ctypes
import ctypes.util
import os, io

class StreamDock293V3(StreamDock):        
    def __init__(self, transport1, devInfo):
        super().__init__(transport1, devInfo)

    # 设置设备的屏幕亮度
    def set_brightness(self, percent):
        return self.transport.setBrightness(percent)
    

    # 设置设备的背景图片 800 * 480
    def set_touchscreen_image(self, path):
        try:
            # 检查图像文件是否存在
            if not os.path.exists(path):
                print(f"Error: The image file '{path}' does not exist.")
                return -1
            
            # 读取图像文件
            image = Image.open(path)
            
            # 判断图片尺寸是否适合
            if image.width > 800 or image.height > 480:
                print("The picture size isn't suitable")
                return -1
            
            # 进行180度旋转
            image = image.rotate(180)
            
            # 临时保存旋转后的图像
            temp_image_path = "rotated_image.jpg"
            image.save(temp_image_path)
            
            # 确保将路径转为字节串类型（c_char_p）
            path_bytes = temp_image_path.encode('utf-8')  # 转换为字节串
            c_path = ctypes.c_char_p(path_bytes)  # 将字节串转换为C字符指针 
            res = self.transport.setBackgroundImgDualDevice(c_path)
            os.remove(temp_image_path)
            
            return res
        except Exception as e:
            print(f"Error: {e}")
            return -1
        
    # 设置设备的按键图标 112 * 112
    def set_key_image(self, key, path):
        try:
            # 检查图像文件是否存在
            if not os.path.exists(path):
                print(f"Error: The image file '{path}' does not exist.")
                return -1
            
            # 读取图像文件
            image = Image.open(path)

            # 判断图片尺寸是否适合
            if image.width > 112 or image.height > 112:
                print("The picture size isn't suitable")
                return -1
            
            # 进行180度旋转
            image = image.rotate(180)
            
            # 临时保存旋转后的图像
            temp_image_path = "rotated_key_image.jpg"
            image.save(temp_image_path)

            # 确保将路径转为字节串类型（c_char_p）
            path_bytes = temp_image_path.encode('utf-8')  # 转换为字节串
            c_path = ctypes.c_char_p(path_bytes)  # 将字节串转换为C字符指针
            res = self.transport.setKeyImgDualDevice(c_path, key)
            os.remove(temp_image_path)
            # print("aaa")
            return res
        except Exception as e:
            print(f"Error: {e}")
            return -1

    # 待补充
    def set_key_imageData(self, key, path):
        pass
    
    # 获取设备的固件版本号
    def get_serial_number(self,length):
        return self.transport.getInputReport(length)

    def key_image_format(self):
        return {
            'size': (112, 112),
            'format': "JPEG",
            'rotation': 180,
            'flip': (False, False)
        }
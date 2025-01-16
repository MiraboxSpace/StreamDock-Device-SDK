# -*- coding: utf-8 -*-

from .StreamDock import StreamDock
from PIL import Image
import ctypes
import ctypes.util
import os, io
class StreamDock293s(StreamDock):


    def __init__(self, transport1, devInfo):
        super().__init__(transport1, devInfo)

    # 设置设备的屏幕亮度
    def set_brightness(self, percent):
        return self.transport.setBrightness(percent)
    

    # 设置设备的背景图片  854*480
    def set_touchscreen_image(self, image):

        image = Image.open(image)
        # 转换为RGB模式
        image = image.convert('RGB')
        width,height=image.size
        # 获取图片的像素数据
        # pixels = list(image.getdata())
        bgr_data = []
    
        # 遍历像素数据，将RGB值转换为BGR值并添加到列表中
        for x in range(width):
            for y in range(height):
                r,g,b = image.getpixel((x,y))
                bgr_data.extend([b,g,r])
        arr_type = ctypes.c_char * len(bgr_data)
        arr_ctypes = arr_type(*bgr_data)

        return self.transport.setBackgroundImg(arr_ctypes,width*height*3)
    
       # 设置设备的按键图标 80 * 80
    def set_key_image(self, key, image_buff):
       # 打开图片
        image1 = Image.open(image_buff)

        # 旋转图片180度
        rotated_image = image1.rotate(90)

        # 保存旋转后的图片
        rotated_image.save("Temporary.jpg","JPEG",subsampling=0,quality=100)
        returnvalue = self.transport.setKeyImg(bytes("Temporary.jpg",'utf-8'), key)
        os.remove("Temporary.jpg")
        return returnvalue
        #向图片传入二进制数据，width：图片的宽默认80，height：图片的高默认80
        
    # def set_key_imageData(self, imagedata, key,width=100,height=100):
    #     print()
    #     if len(imagedata) != width*height*3:
    #         print("width与height与实际大小不符合")
    #         return -1
    #     image = Image.new('RGB', (width, height))
    #     for y in range(height):
    #         for x in range(width):
    #             r=ord(imagedata[width*height*3-y*width*3-x*3-2-1])
    #             g=ord(imagedata[width*height*3-y*width*3-x*3-1-1])
    #             b=ord(imagedata[width*height*3-y*width*3-x*3-1])
    #             image.putpixel((x, y), (r, g, b))
    #     buffer = io.BytesIO()
    #     rotated_image = image.rotate(-90)
    #     rotated_image.save(buffer, format='JPEG')
    #     jpg_data = buffer.getvalue()
    #     returnvalue =self.transport.setKeyImgdata(jpg_data,key,width,height)
    #     return returnvalue

         

    # 获取设备的固件版本号
    def get_serial_number(self,lenth):
        return self.transport.getInputReport(lenth)
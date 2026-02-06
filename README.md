# Arknights Passport ePaper
明日方舟通行证（全彩墨水屏版）


<img src="https://github.com/pupydodo/Arknights-Passport-ePaper/blob/main/1.jpg" width="400px">

<img src="https://github.com/pupydodo/Arknights-Passport-ePaper/blob/main/2.jpg" width="300px">

### 使用说明

##### 图片转换

墨水屏的像素为600*400，建议使用长宽比为3:2的图片，否则在转换时会被强制转换为3:2。转换的图片格式支持.png和.jpg

转换的脚本为转换工具目录下的convert.py，python需要安装pillow

假如你的图片名为amiya.png，目标转换为6.bin，使用如下命令实现转换：

```cmd
python convert.py amiya.png 6.bin
```



##### 图片导入

按住侧边按键，上电，电脑识别到USB设备后再松开（约2s），此时电脑会把通行证识别为U盘

成功识别后便可以将转换好的.bin文件拷贝到通行证中

注意：bin文件必须以 数字.bin 的形式命名，并且必须从1开始（比如1.bin，2.bin，3.bin），如果出现数字不连续的情况，从不连续处开始的文件将不会被识别



##### 图片切换

不要按住侧边按键，上电，等待2s，之后双击侧边按键便能切换图片。切换到最后一张照片后会自动切回第一张。






# Tips when setting up pynq

## KV260

### Setting up

* Make shure you have access to github
* No need to run the snap process, just download the pynq from github and install
* Also notice that you may have error in access snap and in the downloading process
* After you have flashed your img, you should install PYNQ on KV260, you should set system time to avoid error.
* ```
  sudo timedatectl set-timezone Asia/Shanghai
  sudo timedatectl set-ntp false
  sudo timedatectl set-time 'YOUR CURRENT TIME'
  sudo timedatectl set-ntp true

  git clone https://github.com/Xilinx/Kria-PYNQ.git
  cd Kria-PYNQ/
  sudo bash install.sh -b KV260
  ```

### Using ssh to reduce your time

* [Linux 重置root密码和修改用户密码_linux 忘记 root密码重置-CSDN博客](https://blog.csdn.net/qq_42402854/article/details/103821032#:~:text=%E4%BA%8C%E3%80%81%E5%BF%98%E8%AE%B0root%E7%94%A8%E6%88%B7%E5%AF%86%E7%A0%81%EF%BC%8C%E9%87%8D%E7%BD%AEroot%E5%AF%86%E7%A0%81%201%20%23%20chroot%20%20%2F%20sysroot%20,%20%20%20%20%20%20%2F%2F%20%E9%87%8D%E5%90%AF%E7%B3%BB%E7%BB%9F)
* using root to login the board, the jupyter notbook folder is under `/root/jupyter_notebook/` you can simple drag your files through mobaXtermD

### Drivers

* If you want to check the address of the ports in the HLS ip, you can check the path in the format below:
* /hls/fir/prj/baseline/kernel/fir_base/baseline/impl/ip/drivers/fir_wrap_v1_0/src/xfir_wrap_hw.h

## PYNQ Z2

### Using 2.7 version image

* Using 2.7 version image, if you use 3.0 version image, you may run into connection error when driectly connect your board to your computer when using network cable, but **access through your router is fine**
* Also our codes are tested using this version
* 

## Sobel Vision Lib

1. 在用vision lib的Sobel kernel的时候，需要把axi lite启用，同时不设置bundle，防止vitis把它新开一个axilite的port，这样用pynq的时候register_map会无法调用

## 重新在linux上边配置vitis

### dependency

1. 没有pylint3，只有pylint
2. cython 改成cython3
3. python 改成python3

## 连接多个ip kernel

[Vitis_Libraries/vision/L3/examples/gaussiandifference/xf_gaussian_diff_accel.cpp at master · Xilinx/Vitis_Libraries · GitHub](https://github.com/Xilinx/Vitis_Libraries/blob/master/vision/L3/examples/gaussiandifference/xf_gaussian_diff_accel.cpp)

[基于Vitis HLS加速图像处理 | FPGA 开发圈 (eetrend.com)](https://fpga.eetrend.com/blog/2021/100555545.html)

-D BGR2GRAY=1 -D RGBA2IYUV=0"因为这个阿三软件默认RGBA2IYUV是启动的……

export CMAKE_PREFIX_PATH=/home/lanxuan/Desktop/zhanglx/visionLib/opencv_4p4p0:$CMAKE_PREFIX_PATH 自己装的opencv需要设置一下path

## AXI RTL 可能遇到的问题以及文档

### Lab1 AXI-Lite

* 没啥问题

### Lab2 AXI-stream

* 如果在设置axi ip的时候，无法仿真，`ERROR: [XSIM 43-3268] Logical library name '"../../../../keyAxis_1_0/hdl/keyAxis.v"' should not contain white space, new line, /, \, = or .` 可以在tcl command中输入

  * set_property library xil_defaultlib [get_files]
  * 参考    [ERROR: [XSIM 43-3268] Simulating a custon IP with AXI4-Lite interface (xilinx.com)](https://support.xilinx.com/s/question/0D52E00006hpkSJSAY/error-xsim-433268-simulating-a-custon-ip-with-axi4lite-interface?language=en_US)
* 仿真的时候一定要先添加tb，再进行仿真，要不然可能会出现vivado无法正常识别top层级的问题，可能会把待测的module的top识别为仿真的top，从而在仿真的波形中出现z信号。

  * 一个正常的仿真文件层级应该是可以看到tb中module的层级
* 如果碰到ip无法综合的情况，需要重启vivado，并及时的去upgrade ip的文件夹以及refresh IP catalog

# aup服务器上的文件目录 密码aup123
```
~/Desktop/zhanglx
├── AES_AXI
│   ├── keyExpension_vivado 未完成的Axi_stream加keyExpension ip
│   ├── LICENSE
│   ├── ProjectLat          LAT的aes加密实现
│   ├── ProjectZlx          ZLX的aes加密实现
│   ├── README.md
│   ├── vivado_2016664.backup.jou
│   ├── vivado_2016664.backup.log
│   ├── vivado.jou
│   ├── vivado.log
│   └── vivado_pid2016664.str
├── AXI_Interface_RTL_IP    郭老板留下的AXIINTERface工程
│   ├── Lab1 axilite-adder
│   ├── Lab2 axis-vadd
│   ├── Lab3 axi4-vadd
│   └── README.md
├── AXI_Interface_RTL_IP.zip 郭老板留下的AXIINTERface工程
├── cmakeLib        cmake 环境目录
│   ├── cmake-3.29.0
│   ├── cmake-3.29.0-rc1-linux-x86_64
│   └── cmake-3.29.0-rc1-linux-x86_64.sh
├── codeReview      之前systolic的代码
│   ├── filter2d
│   ├── filter2d.7z
│   └── systolic
├── setup.sh
├── visionLib       Vitis Vision Library 2023.2 和 Opencv4.4.0 的安装目录
│   ├── opencv_4p4p0
│   ├── src_opencv
│   ├── Vitis_Libraries
│   └── Vitis_Libraries_2023p2
├── xup_git        xup的工作目录
│   ├── fpgachina24-amd       FPGA竞赛相关资料，可以删除
│   ├── ISP3Kernel            整理过后的ISP工程
│   ├── ISP3kernelLocal       整理前的ISP工程
│   ├── Vitis_Vision_App_Accel ISP的参考工程
│   ├── xup_High-Level-Synthesis-Design-Flow    HLS git
│   └── xup_project_based_learning              PBL git
```
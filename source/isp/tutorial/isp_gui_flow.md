# Starter ISP Lab for Vitis Vision Library

## 1 Overview

The Vitis Vision library is a set of 90+ kernels, optimized for Xilinx™ FPGAs, AI Engine™, and SoCs, based on the OpenCV computer vision library. The kernels in the Vitis Vision library are optimized and supported in the Xilinx Vitis™ Tool Suite.

In this lab, we will learn how to configure and use the Vitis Vision Library, as well as how to modify the corresponding configurations to generate IP cores in Vitis HLS. We will implement We will then integrate the IP cores into a Block Design in Vivado, synthesize and generate the bitstream, and finally validate the functionality of the IP cores using PYNQ.

## 2 Installation of Vitis Vision Library

* Read this document `xup_High-Level-Synthesis-Design-Flow/source/sobel/tutorial/vision_library_guide.md`
* Try to finish **Step0-4** , and run `run_standalone.tcl` **DON'T FORGET** to modify the **PATH variables**

## 3 Kernel Building of Vitis Vision Library

In this lab, we will build `cvtcolor`, `resize` and `sobel` kernel separately, then connect them together through `m_axi` in Block Design.

### 3.1 Build the Vitis Vision Library cvtcolor Kernel
#### 3.1.1 Configure the `xf_config_params.h`
1. Passing Color Space Conversion Parameters

Here we insert a snippet of code from `run_cvtcolor.tcl` for explanation purposes:

```tcl
set CONFIG_DIR "./cvtcolor_config"
set VISION_INC_FLAGS "-I$XF_PROJ_ROOT/L1/include -I$CONFIG_DIR -std=c++14"
set OPENCV_INC_FLAGS "-I$OPENCV_INCLUDE"
set OPENCV_LIB_FLAGS "-L $OPENCV_LIB"
set CVT_FLAGS "-D BGR2GRAY=1 -D RGBA2IYUV=0"
```

- You can find `xf_config_params.h` in `/Vitis_Libraries_2023p2/vision/L1/examples/cvtcolor/config`, where a series of `#ifdef` directives control which color space conversion operation is used when the IP core is compiled.
- You can check the main code in `xf_cvt_color_accel_gen_vitis.cpp` to deepen your understanding. The file is located at `Vitis_Libraries_2023p2/vision/L1/examples/cvtcolor/xf_cvt_color_accel_gen_vitis.cpp`.
- Therefore, during compilation, you need to use `-D` to pass parameters for `#ifdef` directives to compile the corresponding IP core. For example, in the tcl file, we use `set CVT_FLAGS "-D BGR2GRAY=1 -D RGBA2IYUV=0"`. This code enables the `BGR2GRAY` code block and disables the default `RGB2IYUV` code block.

---

2. Modifying Supported Image Dimensions

- Note the following code in lines 337-338 of `xf_config_params.h`:

```cpp
// Image Dimensions
static constexpr int WIDTH = 1920;
static constexpr int HEIGHT = 1080;
```

- From `xf_cvt_color_accel_gen_vitis.cpp`, it can be determined that this code specifies the maximum image size that your IP core can handle. Therefore, if you need the IP core to process larger images, you must modify the corresponding parameters. For example, if you want to process a 4K-sized image, you need to make the following changes in the file:

```cpp
// Image Dimensions
static constexpr int WIDTH = 3840;
static constexpr int HEIGHT = 2160;
```   

#### 3.1.2 Use tcl script to generate cvtcolor kernel
- `vitis_hls -f run_cvtcolor.tcl`

### 3.2 Build the Vitis Vision Library resize Kernel
#### 3.2.1 Configure the `xf_config_params.h`
1. Passing Resize Dimension Parameters

- You can find `xf_config_params.h` in `/Vitis_Libraries_2023p2/vision/L1/examples/resize/config`, where a series of `#ifdef` directives control the input and output image dimensions that the IP core can handle.

```cpp
    #define WIDTH 128
    // Maximum Input image width
    #define HEIGHT 128
    // Maximum Input image height

    #define NEWWIDTH 64
    // Maximum output image width
    #define NEWHEIGHT 64
    // Maximum output image height  
```

- You can check the main code in `xf_cvt_resize_accel.cpp` to deepen your understanding. The file is located at `Vitis_Libraries_2023p2/vision/L1/examples/cvtcolor/xf_cvt_resize_accel.cpp`.
- Therefore, during compilation, you need to use `-D` to pass parameters for `#ifdef` directives to compile the corresponding IP core. For example, in the tcl file, we use `-D WIDTH=4096 -D HEIGHT=2160 -D NEWWIDTH=1920 -D NEWHEIGHT=1080`. This code sets the maximum input image size to 3840x2160 and the maximum output image size to 1920x1080. Alternatively, you can directly modify the corresponding values in `xf_config_params.h` to achieve the same effect.

---

#### 3.2.2 Use Tcl Script to Generate Resize Kernel

- Run the following command:
  ```bash
  vitis_hls -f run_resize.tcl
  ```

---

### 3.3 Build the Vitis Vision Library Sobel Kernel

#### 3.3.1 Configure the `xf_config_params.h`

1. Modify the dimension parameters:
    ```cpp
    /* config width and height */
    #define WIDTH 128
    #define HEIGHT 128
    ```
   - From the code, it can be determined that `WIDTH` and `HEIGHT` directly affect the depth of `m_axi`, so these values need to be modified to match the size of the final input image.
   - You can directly modify the defined values, or you can use `-D` during compilation to pass parameters for `#ifdef` directives to compile the corresponding IP core.

---

#### 3.3.2 Configure the `xf_sobel_accel.cpp`

1. Open `Vitis_Libraries_2023p2/vision/L1/examples/sobelfilter/xf_sobel_accel.cpp` and uncomment the following code:

    ```cpp
    #pragma HLS INTERFACE s_axilite port=rows     
    #pragma HLS INTERFACE s_axilite port=cols     
    #pragma HLS INTERFACE s_axilite port=return   
    ```

    This code enables the AXI Lite interface to pass in **rows** and **cols**, which specify the image size. Note that **rows** and **cols** must not exceed **WIDTH** and **HEIGHT**, respectively.

#### 3.3.2 Use tcl script to generate sobel kernel
- Run the following command:
  ```bash
  vitis_hls -f run_sobel.tcl
  ```

## 4 Block Design Building in Vivado
Since we use 3 different kernel, we should connect them together in Vivado Block Design, so that they can be used in PYNQ. You can follow the steps below to create a bd.

### 4.1 Add User IP Repoitory
1. Left Click **IP Catalog**
2. In the following window, right Click --> **Add Repository**, then select your kernel project folder, Vivado will automaticly find your IP, if they are exported correctly. The result will beike:
![vivado_ip_repo](image/readme/vivado_ip_repo.png)
3. Left Click **Create Block Design**, find the **+** button to add IP, then add the following IP below:
    - PS:
        - Zynq Untrascale+ MPSoC (For KV260) or processing_system_7 (For PYNQ Z2)
    - PL:
        - Your sobel kernel, resize kernel and cvtcolor kernel. Their name will depending on your modification, like:
        - cvtcolor_bgr2gray
        - resize_accel
        - sobel_accel
4. Follow the guide of Vivado, use its auto connnection, then will be ok. And also, you can right click the wires to add **debug** probe, to automatically add the **ILA**, you can notice that the **ILA** block in on the top right of the blockdesign.
    ![isp_bd](image/readme/isp_bd.png)
    
5. Run **Generate Bitstream**, then wait till it finishes. Then export bitsream file as **sobelAll.bit**, and copy the hw handoff file (.hwh) from `/overlay/sobelAll/sobelAll.gen/sources_1/bd/design_1/hw_handoff/design_1.hwh`. Your path maybe different, but the file is the same. And rename .hwh file as **sobelAll.hwh** to match the bitstream.

## 5 Run Bitstream on PYNQ
Open our jupyter notebook reference, click and run.




# ------------------------------------------------------------------------------
# Vitis Vision and OpenCV Libary Path Information
# ------------------------------------------------------------------------------
set XF_PROJ_ROOT "/home/lanxuan/Desktop/zhanglx/visionLib/Vitis_Libraries_2023p2/vision" 	
set OPENCV_INCLUDE "/home/lanxuan/Desktop/zhanglx/visionLib/opencv_4p4p0/include/opencv4" 
set OPENCV_LIB "/home/lanxuan/Desktop/zhanglx/visionLib/opencv_4p4p0/lib" 		

# ------------------------------------------------------------------------------
# Vitis HLS Project Information
# ------------------------------------------------------------------------------
set PROJ_DIR "$XF_PROJ_ROOT/L1/examples/cvtcolor"
set SOURCE_DIR "$PROJ_DIR/"
set PROJ_NAME "cvtcolor_hls"
set PROJ_TOP "cvtcolor_bgr2gray"
set SOLUTION_NAME "sol1"
set SOLUTION_PART "xck26-sfvc784-2LV-c"
set SOLUTION_CLKP 10

# ------------------------------------------------------------------------------
# OpenCV C Simulation / CoSimulation Library References
#------------------------------------------------------------------------------
set CONFIG_DIR "./cvtcolor_config"
set VISION_INC_FLAGS "-I$XF_PROJ_ROOT/L1/include -I$CONFIG_DIR -std=c++14"
set OPENCV_INC_FLAGS "-I$OPENCV_INCLUDE"
set OPENCV_LIB_FLAGS "-L $OPENCV_LIB"
set CVT_FLAGS "-D BGR2GRAY=1 -D RGBA2IYUV=0"

# Windows OpenCV Include Style:
# set OPENCV_LIB_REF   "-lopencv_imgcodecs440 -lopencv_imgproc440 -lopencv_core440 -lopencv_highgui440 -lopencv_flann440 -lopencv_features2d440"

# Linux OpenCV Include Style:
set OPENCV_LIB_REF   "-lopencv_imgcodecs -lopencv_imgproc -lopencv_core -lopencv_highgui -lopencv_flann -lopencv_features2d"

# ------------------------------------------------------------------------------
# Create Project
# ------------------------------------------------------------------------------
open_project -reset $PROJ_NAME

# ------------------------------------------------------------------------------
# Add C++ source and Testbench files with Vision and OpenCV includes
# ------------------------------------------------------------------------------
add_files "${PROJ_DIR}/xf_cvt_color_accel_gen_vitis.cpp" -cflags "${VISION_INC_FLAGS} -I${PROJ_DIR}/build ${CVT_FLAGS}" -csimflags "${VISION_INC_FLAGS} -I${PROJ_DIR}/build ${CVT_FLAGS}"
add_files -tb "${PROJ_DIR}/xf_cvt_color_tb_gen_vitis.cpp" -cflags "${OPENCV_INC_FLAGS} ${VISION_INC_FLAGS} -I${PROJ_DIR}/build ${CVT_FLAGS}" -csimflags "${OPENCV_INC_FLAGS} ${VISION_INC_FLAGS} -I${PROJ_DIR}/build ${CVT_FLAGS}"

# ------------------------------------------------------------------------------
# Create Project and Solution
# ------------------------------------------------------------------------------
set_top $PROJ_TOP
open_solution -reset $SOLUTION_NAME
set_part $SOLUTION_PART
create_clock -period $SOLUTION_CLKP

# ------------------------------------------------------------------------------
# Run Vitis HLS Stages
# Note: CSim and CoSim require datafiles to be included 
# ------------------------------------------------------------------------------
csim_design -ldflags "-L ${OPENCV_LIB} ${OPENCV_LIB_REF}" -argv " ${XF_PROJ_ROOT}/data/input.jpg ${XF_PROJ_ROOT}/data/out_GRAY.png "
csynth_design
cosim_design -ldflags "-L ${OPENCV_LIB} ${OPENCV_LIB_REF}" -argv " ${XF_PROJ_ROOT}/data/input.jpg ${XF_PROJ_ROOT}/data/out_GRAY.png"
export_design -flow syn -rtl verilog
#  export_design -flow impl -rtl verilog
exit
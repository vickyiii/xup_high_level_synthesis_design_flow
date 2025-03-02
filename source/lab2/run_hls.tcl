# check if the directory exists, if not, create it
if {![file exists "./w"]} {
    file mkdir "./w"
}
# change to the working directory
cd ./w

# rebuild the project
open_component -reset yuv_filter -flow_target vivado

# add design files
add_files ../yuv_filter.c

# add test bench & files
add_files -tb ../image_aux.c
add_files -tb ../yuv_filter_test.c
add_files -tb ../test_data

# set the top-level function
set_top yuv_filter

# set the part
set_part {xc7z020clg400-1}

# set the clock period
create_clock -period 10

# run csim 
csim_design

# run csynth
csynth_design

exit

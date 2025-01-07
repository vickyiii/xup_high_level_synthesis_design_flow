#Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
#SPDX-License-Identifier: MIT


# Create a project
open_project -reset maxpool

# Add design files
add_files maxpool.cpp
# Add test bench & files
add_files -tb maxpool_tb.cpp

# Set the top-level function
set_top maxpool_top

# ########################################################
# Create a solution
open_solution -reset maxpool -flow_target vivado

# Define technology and clock rate
set_part  {xc7z020-clg484-1}
create_clock -period 5

# Set variable to select which steps to execute
set hls_exec 2

csim_design
# Set any optimization directives
# End of directives

if {$hls_exec >= 1} {
	# Run Synthesis
   csynth_design
}
if {$hls_exec >= 2} {
	# Run Synthesis, RTL Simulation
   cosim_design
}
if {$hls_exec >= 3} { 
	# Run Synthesis, RTL Simulation, RTL implementation
   #export_design -format ip_catalog -version "1.00a" -library "hls" -vendor "xilinx.com" -description "A memory mapped IP created by Vitis HLS" -evaluate verilog
   export_design -format ip_catalog -evaluate verilog
}

exit
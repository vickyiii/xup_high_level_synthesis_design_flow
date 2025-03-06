#Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
#SPDX-License-Identifier: MIT

set kernel_name "lenet5"
set project_name "lenet5"
set hls_solution "lenet5"
set design_name "design_1"
set pynq_Z2 "xc7z020clg484-1"

# Clean up
file delete {*}[glob *.log]
file delete {*}[glob *.jou]
file delete -force ./${project_name}
file delete -force .crashReporter
file delete -force .Xil

create_project ${project_name} ./${project_name} -part ${pynq_Z2} -force

# Set IP repo
set_property ip_repo_paths ../kernel/${kernel_name}/${project_name}/${hls_solution}/impl/ip [current_project]
update_ip_catalog

create_bd_design ${design_name}
source ./block_design_z2.tcl
validate_bd_design

generate_target all [get_files ./${project_name}/${project_name}.srcs/sources_1/bd/${design_name}/${design_name}.bd]
export_ip_user_files -of_objects [get_files ./${project_name}/${project_name}.srcs/sources_1/bd/${design_name}/${design_name}.bd] -no_script -sync -force -quiet
create_ip_run [get_files -of_objects [get_fileset sources_1] ./${project_name}/${project_name}.srcs/sources_1/bd/${design_name}/${design_name}.bd]

set ooc_runs [get_runs *synth_1 -filter {IS_OUT_OF_CONTEXT}]
if {[llength $ooc_runs] > 0} {
    puts "Launching OOC synthesis for: $ooc_runs"
    launch_runs $ooc_runs -jobs 8
    wait_on_run $ooc_runs
} else {
    puts "No OOC synthesis runs found."
}

export_simulation -of_objects [get_files ./${project_name}/${project_name}.srcs/sources_1/bd/${design_name}/${design_name}.bd] -directory ./${project_name}/${project_name}.ip_user_files/sim_scripts -ip_user_files_dir ./${project_name}/${project_name}.ip_user_files -ipstatic_source_dir ./${project_name}/${project_name}.ip_user_files/ipstatic -lib_map_path [list {modelsim=./${project_name}/${project_name}.cache/compile_simlib/modelsim} {questa=./${project_name}/${project_name}.cache/compile_simlib/questa} {riviera=./${project_name}/${project_name}.cache/compile_simlib/riviera} {activehdl=./${project_name}/${project_name}.cache/compile_simlib/activehdl}] -use_ip_compiled_libs -force -quiet

make_wrapper -files [get_files ./${project_name}/${project_name}.srcs/sources_1/bd/${design_name}/${design_name}.bd] -top
add_files -norecurse ./${project_name}/${project_name}.gen/sources_1/bd/${design_name}/hdl/${design_name}_wrapper.v

# **synthesis**
launch_runs synth_1 -jobs 8
wait_on_run synth_1

# **implementation**
launch_runs impl_1 -jobs 8
wait_on_run impl_1

# **bitstream**
launch_runs impl_1 -to_step write_bitstream -jobs 8 
wait_on_run impl_1

#move and rename bitstream to final location
file copy -force ./${project_name}/${project_name}.runs/impl_1/${design_name}_wrapper.bit ${project_name}_z2.bit
file copy -force ./${project_name}/${project_name}.gen/sources_1/bd/${design_name}/hw_handoff/${design_name}.hwh ${project_name}_z2.hwh
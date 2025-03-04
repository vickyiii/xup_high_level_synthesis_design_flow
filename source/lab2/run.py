import vitis
import os

# Get current working directory
cwd = os.getcwd() + '/'

# Initialize session and set the workspace
client = vitis.create_client()
client.set_workspace(path='./w')

# If the component already exists, delete it (similar to a project reset)
if os.path.exists('./w/yuv_filter'):
    client.delete_component(name='yuv_filter')

# Create a new HLS component with a fresh configuration file
comp = client.create_hls_component(
    name='yuv_filter',
    cfg_file=['hls_config.cfg'],
    template='empty_hls_component'
)

# Get the configuration file handle to set up the design parameters
cfg_file = client.get_config_file(path='./w/yuv_filter/hls_config.cfg')

# Set the target FPGA part (similar to set_part in Tcl)
cfg_file.set_value(key='part', value='xc7z020clg400-1')

# Specify the synthesis source file (yuv_filter.c)
cfg_file.set_value(section='hls', key='syn.file', value=cwd + 'yuv_filter.c')

# Specify testbench files: image_aux.c, yuv_filter_test.c and test_data folder
cfg_file.set_values(
    section='hls',
    key='tb.file',
    values=[cwd + 'image_aux.c', cwd + 'yuv_filter_test.c', cwd + 'test_data']
)

# Set the top function (similar to set_top)
cfg_file.set_value(section='hls', key='syn.top', value='yuv_filter')

# Create clock with a period of 10ns (as in create_clock -period 10)
cfg_file.set_value(section='hls', key='clock', value='10')

# Other generic HLS flow options (flow target, output packaging, etc.)
cfg_file.set_value(section='hls', key='flow_target', value='vitis')
cfg_file.set_value(section='hls', key='package.output.syn', value='0')
cfg_file.set_value(section='hls', key='package.output.format', value='xo')
cfg_file.set_value(section='hls', key='csim.code_analyzer', value='0')

# Retrieve the component handle and run the synthesis flow steps
comp = client.get_component(name='yuv_filter')
comp.run(operation='C_SIMULATION')
comp.run(operation='SYNTHESIS')
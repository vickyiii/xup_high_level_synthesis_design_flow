# Improving Area and Resoudesign.rce Utilization Lab

## Introduction

This lab introduces various techniques and directives which can be used in Vitis to improve design performance as well as area and resource utilization. The design under consideration performs Discrete Cosine Transformation (DCT) on an 8x8 block of data.

This design implements a discrete cosine transformation (DCT), and it is provided as C source. The function leverages a 2D DCT algorithm by first processing each row of the input array via a 1D DCT, then processing the columns of the resulting array through the same 1D DCT. It calls the read_data, dct_2d, and write_data functions.

## Objectives
After completing this lab, you will be able to:
* Add directives in your design
* Improve performance using PIPELINE directive
* Distinguish between DATAFLOW directive and Configuration Command functionality
* Apply memory partitions techniques to improve resource utilization

## Steps

### Validate the Design from Command Line
#### Validate your design in the terminal.
1. Open a terminal.
2. Change directory to **{labs}/lab3**.

  A self-checking program (dct_test.c) is provided. Using that we can validate the design. A Makefile is also provided. Using the Makefile, the necessary source files can be compiled and the compiled program can be executed.

3. For **Windows users**, simply type **`make`** in the terminal to compile and execute the program.

    <p align="center">
    <img src ="./images/lab3/Figure1.jpg">
    </p>
    <p align = "center">
    <i>Validating the design</i>
    </p>
    Note that the source files (dct.c and dct_test.c are compiled, then dct executable program was created, and then it was executed. The program tests the design and outputs Results are good message.)

### Create a New Project
#### Create a new project in Vitis HLS GUI targeting xc7z020clg400-1.
1. Open the **vitis**.
2. In the Vitis GUI, click on **Create Component... > Create Empty HLS Component**.
3. Click **Browse** button of the Location field and browse to **{labs}/lab3** and then click **OK**.
4. For Component Name, type **dct** and click **Next**.
5. In the **Configuration File** set Empty File (default) and click **Next**.
6. In the Source Files for the source files, type **dct** as the top function name (the provided source file contains the function, to be synthesized, called dct).
7. Click the **Add file** button (which in the line of DESIGN FILES), select **dct.c** file from the **{labs}/lab3** folder, and then click **Open**.
8. Then we add the test file, in the next block, click **Add files** button (which in the line of TEST BENCH FILES), button, select **dct_test.c**, **in.dat**, **out.golden.dat** files from the **{labs}/lab3** folder and click **Open**.
9.  Click **Next**.
10. In the *Hardware* page, select **Part** field, enter  **xc7z020clg400-1** in the *Search* field and click **next**.</p>
11. In the *Settings* page, type 10ns in the clock.Click **next**.
12. Click **Finish**.
13. Click on the **dct.c** under the source folder to open its content in the information pane.
    <p align="center">
    <img src ="./images/lab3/Figure2.jpg">
    </p>
    <p align = "center">
    <i>The design under consideration</i>
    </p>
    The top-level function *dct*, is defined at line 104. It implements 2D DCT algorithm by first processing each row of the input array via a 1D DCT then processing the columns of the resulting array through the same 1D DCT. It calls read_data, dct_2d, and write_data functions.
    The read_data function is defined at line 80 and consists of two loops – RD_Loop_Row and RD_Loop_Col. The write_data function is defined at line 92 and consists of two loops to perform writing the result. The dct_2d function, defined at line 49, calls dct_1d function and performs transpose.
    Finally, dct_1d function, defined at line 30, uses dct_coeff_table and performs the required function by implementing a basic iterative form of the 1D Type-II DCT algorithm. Following figure shows the function hierarchy on the left-hand side, the loops in the order they are executes and the flow of data on the right-hand side.
    <p align="center">
    <img src ="./images/lab3/Figure3.png">
    </p>
    <p align = "center">
    <i>Design hierarchy and dataflow</i>
    </p>

### Synthesize the Design

#### Synthesize the design with the defaults. View the synthesis results and answer the question listed in the detailed section of this step.
1. Select **Flow > C SYNTHESIS > Run** to start the synthesis process.
2. When synthesis is completed, several report files will become accessible and the *Synthesis Results* will be displayed in the **Flow > C SYNTHESIS > REPORTS > Synthesis**.

    The dct_1d, dct_2d, read_data and write_data functions are inlined. Verify this by scrolling up into the Vitis Console view.
    <p align="center">
    <img src ="./images/lab3/Figure4.jpg">
    </p>
    <p align = "center">
    <i>Inlining of dct_1d, dct_2d, read_data and write_data functions</i>
    </p>
3. The *Synthesis Report* shows the performance and resource estimates as well as estimated latency in the design. Note that the design is already pipelined.
    <p align="center">
    <img src ="./images/lab3/Figure5.jpg">
    </p>
    <p align = "center">
    <i>Synthesis report</i>
    </p>
4. Using scroll bar on the right, scroll down into the report and answer the following question.

    **Question 1**  
    **Answer the following question:**   
    Estimated clock period:   
    Worst case latency:   
    Number of DSP48E used:   
    Number of BRAMs used:   
    Number of FFs used:   
    Number of LUTs used:   
5. The report also shows the top-level interface signals generated by the tools.
    <p align="center">
    <img src ="./images/lab3/Figure6.jpg">
    </p>
    <p align = "center">
    <i>Generated interface signals</i>
    </p>
    You can see ap_clk, ap_rst are automatically added. The ap_start, ap_done, ap_idle, and ap_ready are top-level signals used as handshaking signals to indicate when the design is able to accept next computation command (ap_idle), when the next computation is started (ap_start), and when the computation is completed (ap_done). The top-level function has input and output arrays, hence an ap_memory interface is generated for each of them.

### Run Co-Simulation

#### Run the Co-simulation. Verify that the simulation passes.
1. Select **Flow > C/RTL COSIMULATION > Run**. Wait for tge COSIMULATION to complete.

   The RTL Co-simulation will run, generating and compiling several files, and then simulating the design. In the console window you can see the progress and also a message that the test is passed.
    <p align="center">
    <img src ="./images/lab3/Figure7.jpg">
    </p>
    <p align = "center">
    <i>RTL Co-Simulation results</i>
    </p>

### Remove the pipeline optimization done by Vitis HLS automatically by adding pipeline off pragma
1. Right click the component, select the **Clone Component**, then name a new component as **dct_solution2** (or the other name you like).

    <p align="center">
    <img src ="./images/lab3/Figure8.jpg">
    </p>
    <p align = "center">
    <i>Select Clone Component</i>
    </p>

    <p align="center">
    <img src ="./images/lab3/Figure9.jpg">
    </p>
    <p align = "center">
    <i>Name the new component</i>
    </p>

    <p align="center">
    <img src ="./images/lab3/Figure10.jpg">
    </p>
    <p align = "center">
    <i>Success create the new component</i>
    </p>

2. Open the new component **dct.c** source and click on the **HLS Directive** tab (this button is located on the right side of the IDE, positioned in the third row.).

    <p align="center">
    <img src ="./images/lab3/Figure11.jpg">
    </p>
    <p align = "center">
    <i>HLS Directive position</i>
    </p>

3. Select function **DCT_Inner_Loop** in the directives pane, then click the **+** icon in the same line.
4. A pop-up menu shows up. Select **PIPELINE** directive. Click on the **off** option to turn off the automatic pipelining. Click **OK**.

    <p align="center">
    <img src ="./images/lab3/Figure12.jpg">
    </p>
    <p align = "center">
    <i>Add PIPELINE off directive</i>
    </p>
   
5. Similarly, apply the **PIPELINE off** directive to **DCT_Outer_Loop**, **Row_DCT_Loop**, **Xpose_Row_Outer_Loop**, **Xpose_Row_Inner_Loop**, **Col_DCT_Loop**, **Xpose_Col_Outer_Loop**, **Xpose_Col_Inner_Loop**, **RD_Loop_Row**, **RD_Loop_Col**, **WR_Loop_Row**, and **WR_Loop_Col** objects. At this point, the *Directive* tab should look like as follows.
    <p align="center">
    <img src ="./images/lab3/Figure13.jpg">
    </p>
    <p align = "center">
    <i>PIPELINE off directive applied_0</i>
    </p>

    <p align="center">
    <img src ="./images/lab3/Figure14.jpg">
    </p>
    <p align = "center">
    <i>PIPELINE off directive applied_1</i>
    </p>
6. Click on the **C > C Synthesis > Run** button.
7.  When the synthesis is completed, report shows the performance and area without the automatic optimization of Vitis HLS.
    <p align="center">
    <img src ="./images/lab3/Figure15.jpg">
    </p>
    <p align = "center">
    <i>Performance after applying PIPELINE off directive</i>
    </p>

### Apply PIPELINE Directive
#### Create a new solution by copying the previous solution settings. Apply the PIPELINE directive to DCT_Inner_Loop, Xpose_Row_Inner_Loop, Xpose_Col_Inner_Loop, RD_Loop_Col, and WR_Loop_Col. Generate the solution and analyze the output.
1. Right-click the component used in the previous chapter when the pipeline was off, 
select the **Clone Component**, then name a new component as **dct_solution3** (or the other name you like).
2. Make sure that the new **dct.c** source is opened in the information pane and click on the **HLS Directive** tab.
3. Select **HLS PINPELINE off** of **DCT_Inner_Loop** in the directives pane, 
then click the **Edit Directive** icon in the same line.
4. In the **Edit Directive** dialog box, click the **off** option to turn on the pipelining.
5. Leave II (Initiation Interval) blank as Vitis HLS will try for an II=1, one new input every clock cycle.
6. Click **OK**.
7. Similarly, apply the **PIPELINE** directive to **Xpose_Row_Inner_Loop** and **Xpose_Col_Inner_Loop** of the dct_2d function, and **RD_Loop_Col** of the read_data function, and **WR_Loop_Col** of the write_data function. But remove the **PIPELINE** directive of **DCT_Outer_Loop**, **Row_DCT_Loop**, **Xpose_Row_Outer_Loop**, **Col_DCT_Loop**, **Xpose_Col_Outer_Loop**, **RD_Loop_Row** and **WR_Loop_Row**. At this point, the Directive tab should look like as follows.
    <p align="center">
    <img src ="./images/lab3/Figure16.jpg">
    </p>
    <p align = "center">
    <i>PIPELINE directive applied</i>
    </p>
8.  Click on the **FLOW > C SYNTHESIS > Run** button.
9.  When the synthesis is completed, select **View > HLS Compare Reports**, select the **dct_solution2** and **dct_solution3** to compare the two solutions.
10. Observe that the latency reduced from *5990* to *1323* clock cycles.
<p align="center">
<img src ="./images/lab3/Figure17.jpg">
</p>
<p align = "center">
<i>Performance comparison after pipelining</i>
</p>


1.  Scroll down in the comparison report to view the resources utilization. Observe that the FFs and/or LUTs utilization increased whereas BRAM and DSP48E remained same.
    <p align="center">
    <img src ="./images/lab3/Figure18.jpg">
    </p>
    <p align = "center">
    <i>Resources utilization after pipelining</i>
    </p>

#### Open the Schedule Viewer and determine where most of the clock cycles are spend, i.e. where the large latencies are.
1. Click on the *FLOW > C SYNTHESIS > REPORTS > Schedule Viewer* .
2. Select the **dct** entry and observe the **RD_Loop_Row_RD_Loop_Col** and **WR_Loop_Row_WR_Loop_Col**entries in the Performance & Resource Estimates. These are two nested loops flattened and given the new names formed by appending inner loop name to the outer loop name. You can verify this by looking in the Console view message.
    <p align="center">
    <img src ="./images/lab3/Figure19.jpg">
    </p>
    <p align = "center">
    <i>The console view content indicating loops flattening</i>
    </p>

3. In the *Performance & Resource Estimates* (in the *REPORTS > Synthesis*)pane, expand all items. Notice that the most of the latency occurs is in **Row_DCT_Loop_DCT_Outer_Loop** and **Col_DCT_Loop_DCT_Outer_Loop** function.
    <p align="center">
    <img src ="./images/lab3/Figure20.jpg">
    </p>
    <p align = "center">
    <i>The Performance & Resource Estimates</i>
    </p>

4. In the Schedule Viewer pane, select the ******Col_DCT_Loop_DCT_Outer_Loop** entry, right-click on the **col_outbuf** (write) block in the **Schedule Viewer**, and select Goto Source. Notice that line 45 is highlighted which is preventing the flattening of the DCT_Outer_Loop
    <p align="center">
    <img src ="./images/lab3/Figure21.jpg">
    </p>
    <p align = "center">
    <i>Understanding what is preventing DCT_Outer_Loop flattening</i>
    </p>

5. Switch to the *Synthesis* perspective.

#### Create a new solution by copying the previous solution settings. Apply fine-grain parallelism of performing multiply and add operations of the inner loop of dct_1d using PIPELINE directive by moving the PIPELINE directive from inner loop to the outer loop of dct_1d. Generate the solution and analyze the output.
1. Right-click the component used in the previous chapter when, select the **Clone Component**, then name a new component as **dct_solution4** (or the other name you like).
2. Select **PIPELINE** directive of **DCT_Inner_Loop** of the **dct_1d** function in the Directive pane, in the line of **HLS PIPELINE** click **Delete Directive**.
3. Select **DCT_Outer_Loop** of the **dct_1d** function in the Directive pane, click the **Add Directive**
4. A pop-up menu shows up listing various directives. Select **PIPELINE** directive.
5. Click **OK**.
    <p align="center">
    <img src ="./images/lab3/Figure22.jpg">
    </p>
    <p align = "center">
    <i>PIPELINE directive applied to DCT_Outer_Loop</i>
    </p>
    By pipelining an outer loop, all inner loops will be unrolled automatically (if legal), so there is no need to explicitly apply an UNROLL directive to DCT_Inner_Loop. Simply move the pipeline to the outer loop: the nested loop will still be pipelined but the operations in the inner-loop body will operate concurrently.
6. Click on the **FLOW > C Synthesis > Run** button.
7. When the synthesis is completed, select **View > HLS Compare Reports** to compare the two solutions (dct_solution3 and dct_solution4).
8.  Observe that the latency reduced from *1323* to *647* clock cycles.
    <p align="center">
    <img src ="./images/lab3/Figure23.jpg">
    </p>
    <p align = "center">
    <i>Performance comparison after pipelining</i>
    </p>
9.  Scroll down in the comparison report to view the resources utilization. Observe that the utilization of DSP and FF increased. Since the DCT_Inner_Loop was unrolled, the parallel computation requires 8 DSP48E.
       <p align="center">
       <img src ="./images/lab3/Figure24.jpg">
       </p>
       <p align = "center">
       <i>Resources utilization after pipelining</i>
       </p>

#### Perform design analysis and look at the dct performance view.
1. Switch to the **Performance & Resource Estimates** in *Report > Synthesis*.
2. Expand, if necessary, and notice that the DCT_Outer_Loop is now pipelined and there is no DCT_Inner_Loop entry.
    <p align="center">
    <img src ="./images/lab3/Figure34.jpg">
    </p>
    <p align = "center">
    <i>DCT_Outer_Loop flattening</i>
    </p>

### Improve Memory Bandwidth

#### Create a new solution by copying the previous solution (Solution4) settings. Apply ARRAY_PARTITION directive to buf_2d_in of dct (since the bottleneck was on src port of the dct_1d function, which was passed via in_block of the dct_2d function, which in turn was passed via buf_2d_in of the dct function) and col_inbuf of dct_2d. Generate the solution.
1. Right-click the component used in the previous chapter when, select the **Clone Component**, then name a new component as **dct_solution5** (or the other name you like).
2. With new *dct.c* open, select **buf_2d_in** and click the **Add Directive**.
    
    The buf_2d_in array is selected since the bottleneck was on src port of the dct_1d function, which was passed via in_block of the dct_2d function, which in turn was passed via buf_2d_in of the **dct** function.

3. A pop-up menu shows up listing various directives. Select **ARRAY_PARTITION** directive.
4. Make sure that the type is *complete*. Enter **2** in the dimension field and click **OK**.
    <p align="center">
    <img src ="./images/lab3/Figure25.jpg">
    </p>
    <p align = "center">
    <i>Applying ARRAY_PARTITION directive to memory buffer</i>
    </p>
5. Similarly, apply the **ARRAY_PARTITION** directive with dimension of 2 to the **col_inbuf** array.
6. Click on the **FLOW > C Synthesis > Run** button.
7. When the synthesis is completed, select **View > HLS Compare Reports** to compare the two solutions (dct_solution4 and dct_solution5).
8.  Observe that the latency reduced from *647* to *583* clock cycles.
    <p align="center">
    <img src ="./images/lab3/Figure26.jpg">
    </p>
    <p align = "center">
    <i>Performance comparison after array partitioning</i>
    </p>
9.  Scroll down in the comparison report to view the resources utilization. Observe the increase in the FF resource utilization.
    <p align="center">
    <img src ="./images/lab3/Figure27.jpg">
    </p>
    <p align = "center">
    <i>Resources utilization after array partitioning</i>
    </p>
10. Expand the Loop entry in the **Performance & Resource Estimates** in *Synthesis Summary* and observe that the Pipeline II is now 1.

### Apply DATAFLOW Directive

#### Create a new solution by copying the previous solution (Solution5) settings. Apply the DATAFLOW directive to improve the throughput. Generate the solution and analyze the output.
1. Right-click the component used in the previous chapter when, select the **Clone Component**, then name a new component as **dct_solution6** (or the other name you like).
2. Open the **HLS Directive** plane and select function **dct**, click the **Add Directive**.
3. Select **DATAFLOW** directive to improve the throughput.
4. Click on the *FLOW > C Synthesis > Run* button.
5. When the synthesis is completed, Open *C SYNTHESIS > REPORTS > Synthesis*.
6. Observe that dataflow type pipeline throughput is listed in the "Performance Estimates*
    <p align="center">
    <img src ="./images/lab3/Figure28.jpg">
    </p>
    <p align = "center">
    <i>Performance estimate after DATAFLOW directive applied</i>
    </p>

 * The Dataflow pipeline throughput indicates the number of clock cycles between each set of
inputs reads (interval parameter). If this value is less than the design latency it indicates the
design can start processing new inputs before the currents input data are output.
 * Note that the dataflow is only supported for the functions and loops at the top-level, not those
which are down through the design hierarchy. Only loops and functions exposed at the toplevel
of the design will get benefit from dataflow optimization.
7. in the **Performance & Resources Estimates** Windows, observe that the number of *BRAM_18K* required at the top-level remained at *3*.
    <p align="center">
    <img src ="./images/lab3/Figure29.jpg">
    </p>
    <p align = "center">
    <i>Resource estimate with DATAFLOW directive applied</i>
    </p>
8.  Look at the console view and notice that **dct_coeff_table** is automatically partitioned in dimension *2*.
9.  The *buf_2d_in* and *col_inbuf* arrays are partitioned as we had applied the directive in the previous run. The dataflow is applied at the top-level which created channels between top-level functions *read_data, dct_2d*, and *write_data*.
    <p align="center">
    <img src ="./images/lab3/Figure30.jpg">
    </p>
    <p align = "center">
    <i>Console view of synthesis process after DATAFLOW directive applied</i>
    </p>

#### Perform performance analysis by switching to the Synthesis Summary and looking at the dct Performance & Resource Estimates view.
1. Switch to the *Synthesis Summary* perspective, expand the *Performance & Resource Estimates* entries, and select the **dct_2d**
     entry.
2. Observe that most of the latency and interval (throughput) is caused by the *dct_2d* function. The interval of the top-level function *dct*, is less than the sum of the intervals of the read_data, dct_2d, and write_data functions indicating that they operate in parallel and dct_2d is the limiting factor. It can be seen that dct_2d is not completely operating in parallel as Row_DCT_Loop and Col_DCT_Loop were not pipelined.
    <p align="center">
    <img src ="./images/lab3/Figure31.jpg">
    </p>
    <p align = "center">
    <i>Performance analysis after the DATAFLOW directive</i>
    </p>
    One of the limitations of the dataflow optimization is that it only works on top-level loops and functions. One way to have the blocks in dct_2d operate in parallel would be to pipeline the entire function. This however would unroll all the loops and can sometimes lead to a large area increase.
    An alternative is to raise these loops up to the top-level of hierarchy, where dataflow optimization can be applied, by removing the dct_2d hierarchy, i.e. inline the dct_2d function.

### Apply INLINE Directive

#### Create a new solution by copying the previous solution (Solution6) settings. Apply INLINE directive to dct_2d. Generate the solution and analyze the output.
1. Right-click the component used in the previous chapter when, select the **Clone Component**, then name a new component as **dct_solution7** (or the other name you like).
2. Open the **HLS Directive** plane and select function **dct_2d**, click the **Add Directive**.
3. A pop-up menu shows up listing various directives. Select **INLINE** directive. The INLINE directive causes the function to which it is applied to be inlined: its hierarchy is dissolved.
4. Click on the *FLOW > C SYNTHESIS > Run* button.
5. Open the report, observe that the latency reduced from *580* to *417* clock cycles, and the Dataflow pipeline throughput drastically reduced from *445* to ** clock cycles.
6. you use the terminal to see what transformations were applied automatically.
   * The dct_1d function calls are now automatically inlined into the loops from which they are called, which allows the loop nesting to be flattened automatically.
   * Note also that the DSP48E usage has doubled (from 8 to 16). This is because, previously a single instance of dct_1d was used to do both row and column processing; now that the row and column loops are executing concurrently, this can no longer be the case and two copies of dct_1d are required: Vitis HLS will seek to minimize the number of clocks, even if it means increasing the area.
    <p align="center">
    <img src ="./images/lab3/Figure32.jpg">
    </p>
    <p align = "center">
    <i>Console view after INLINE directive applied to dct_2d</i>
    </p>   
7. Switch to the *REPORTS > Synthesis* perspective, expand the *Performance & Resource Estimates* entries, and select the **dct** entry.

    Observe that the dct_2d entry is now replaced with dct_Loop_Row_DCT_Loop_proc, dct_Loop_Xpose_Row_Outer_Loop_proc, dct_Loop_Col_DCT_Loop_proc, and dct_Loop_Xpose_Col_Outer_Loop_proc since the dct_2d function is inlined. Also observe that all the functions are operating in parallel, yielding the top-level function interval (throughput) of 72 clock cycles.
    <p align="center">
    <img src ="./images/lab3/Figure33.jpg">
    </p>
    <p align = "center">
    <i>Performance analysis after the INLINE directive</i>
    </p>
8.  Close Vitis HLS by selecting **File > Close Window**.

## Conclusion
In this lab, you learned various techniques to improve the performance and balance resource utilization.
PIPELINE directive when applied to outer loop will automatically cause the inner loop to unroll. When a
loop is unrolled, resources utilization increases as operations are done concurrently. Partitioning memory
may improve performance but will increase BRAM utilization. When INLINE directive is applied to a
function, the lower level hierarchy is automatically dissolved. When DATAFLOW directive is applied, the
default memory buffers (of ping-pong type) are automatically inserted between the top-level functions and
loops. The console logs can provide insight on what is going on.

## Answers

<sub>"Note: These answers are based on the settings from the previous version and may not be accurate for the current version. Please verify with the latest synthesis report."</sub>

**Answers for question 1:**  
Estimated clock period: **6.508 ns**   
Worst case latency: **423 clock cycles**   
Number of DSP48E used: **17**   
Number of BRAMs used: **16**   
Number of FFs used: **929**   
Number of LUTs used: **2012**
<p align="center">Copyright&copy; 2022, Advanced Micro Devices, Inc.</p>
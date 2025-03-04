#include "fully_connected.hpp"

void store_input(int in[IC], int in_buf[IC]) {
  for (int i = 0; i < IC; i += PAR) {
#pragma HLS pipeline
    for (int j = 0; j < PAR; j++) {
#pragma HLS unroll
      in_buf[i + j] = in[i + j];
    }
  }
}

void store_weight(int weight[OC * IC], int weight_buf[OC][IC]) {
  for (int i = 0; i < OC * IC; i += PAR) {
#pragma HLS pipeline
    for (int j = 0; j < PAR; j++) {
#pragma HLS unroll
      weight_buf[i/IC][(i%IC)+j] = weight[i + j];
    }
  }
}

void process(int in_buf[IC],
             int weight_buf[OC][IC],
             int out_buf[OC]) {
  int out_buf_tmp[OC][PAR];
  #pragma HLS array_partition out_buf_tmp complete

  for (int oc = 0; oc < OC; oc++) {
#pragma HLS unroll
    for (int l_ic = 0; l_ic < PAR; l_ic++) {
#pragma HLS unroll
      out_buf_tmp[oc][l_ic] = 0;
    }
  }

  for (int g_ic = 0; g_ic < IC; g_ic += PAR) {
#pragma HLS pipeline II=1
    for (int l_ic = 0; l_ic < PAR; l_ic++) {
#pragma HLS unroll
      auto x = in_buf[g_ic + l_ic];

      for (int oc = 0; oc < OC; oc++) {
#pragma HLS unroll
        auto w = weight_buf[oc][g_ic + l_ic];

        switch (w) {
        case 1:
          out_buf_tmp[oc][l_ic] += x;
          break;
        case -1:
          out_buf_tmp[oc][l_ic] -= x;
          break;
        default:
          // do nothing
          break;
        }
      }
    }
  }

  for (int oc = 0; oc < OC; oc++) {
#pragma HLS unroll
    out_buf[oc] = 0;

    for (int l_ic = 0; l_ic < PAR; l_ic++) {
#pragma HLS unroll
      out_buf[oc] += out_buf_tmp[oc][l_ic];
    }
  }
}

void output(int out_buf[OC], int out[OC]) {
  for (int oc = 0; oc < OC; oc++) {
    out[oc] = out_buf[oc];
  }
}

void fully_connected(
            int in[IC],
            int weight[OC * IC],
            int out[OC]
            ) {
#pragma HLS interface axis port=in
#pragma HLS interface axis port=weight
#pragma HLS interface axis port=out
#pragma HLS array_partition variable=in cyclic factor=PAR
#pragma HLS array_partition variable=weight cyclic factor=PAR
#pragma HLS array_partition variable=out cyclic factor=OC

  int in_buf[IC];
#pragma HLS array_partition variable=in_buf cyclic factor=PAR

  int weight_buf[OC][IC];
#pragma HLS array_partition variable=weight_buf complete dim=1
#pragma HLS array_partition variable=weight_buf cyclic factor=PAR dim=2

  int out_buf[OC];
#pragma HLS array_partition variable=out_buf complete

#pragma HLS dataflow
  store_input(in, in_buf);
  store_weight(weight, weight_buf);
  process(in_buf, weight_buf, out_buf);
  output(out_buf, out);
}
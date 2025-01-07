#pragma once
#include <ap_int.h>
#include <hls_stream.h>
#include <hls_vector.h>
#include <cstdint>

const int WIDTH = 28;
const int HEIGHT = 28;

const int FILTER = 16;
const int KERNEL = 5;
const int CHANNEL = 16;
const int THRESHOLD = 3;

const int FLATTEN = 256;
const int CLASS = 10;

using int2_t = ap_int<2>;
using uint2_t = ap_uint<2>;
using int2x25_t = ap_uint<2 * KERNEL * KERNEL>;
using int2x16_t = ap_uint<2 * CHANNEL>;
template <class T>
using fifo = hls::stream<T>;

extern "C" {
void lenet5(
  int in[28 * 28 * 1],
  int conv0_weight[16 * 5 * 5 * 1],
  int threshold0[3],
  int conv1_weight[16 * 5 * 5 * 16],
  int threshold1[3],
  int matmul0_weight[256 * 10],
  int out[10]
);
}
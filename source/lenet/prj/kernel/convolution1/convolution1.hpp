#pragma once
#include <ap_int.h>
#include <hls_stream.h>
#include <hls_vector.h>
#include <cstdint>

const int WIDTH = 12;
const int HEIGHT = 12;
const int CHANNEL = 16;

const int FILTER = 16;
const int KERNEL = 5;
const int THRESHOLD = 3;

const int OWIDTH = 4;
const int OHEIGHT = 4;

using int2_t = ap_int<2>;
using uint2_t = ap_uint<2>;
using int2x16_t = ap_uint<2 * CHANNEL>;
using pack1_t = hls::vector<int2x16_t, KERNEL * KERNEL>;
template <typename T>
using fifo = hls::stream<T>;

extern "C" {
void convolution1(
  int in[12 * 12 * 16],
  int weight[16 * 5 * 5 * 16],
  int threshold[3],
  int out[4 * 4 * 16]
);
}
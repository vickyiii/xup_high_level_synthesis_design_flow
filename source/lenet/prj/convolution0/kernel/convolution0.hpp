#pragma once
#include <ap_int.h>
#include <hls_stream.h>
#include <hls_vector.h>
#include <cstdint>
const int WIDTH = 28;
const int HEIGHT = 28;

const int FILTER = 16;
const int KERNEL = 5;
const int THRESHOLD = 3;

const int OWIDTH = WIDTH - KERNEL + 1;
const int OHEIGHT = HEIGHT - KERNEL + 1;

using int2_t = ap_int<2>;
using uint2_t = ap_uint<2>;
using int2x25_t = ap_uint<2 * KERNEL * KERNEL>;
using int2x16_t = ap_uint<2 * FILTER>;


template <typename T>
using fifo = hls::stream<T>;

extern "C" {
void convolution0(
  int in[28 * 28 * 1],
  int weight[16 * 5 * 5 * 1],
  int threshold[3],
  int out[24 * 24 * 16]
);
}
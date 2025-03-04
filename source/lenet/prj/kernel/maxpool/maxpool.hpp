#pragma once
#include <ap_int.h>
#include <hls_stream.h>
#include <hls_vector.h>
const int WIDTH = 24;
const int HEIGHT = 24;
const int CHANNEL = 16;

const int OWIDTH = WIDTH / 2;
const int OHEIGHT = HEIGHT / 2;

using int2_t = ap_int<2>;
using uint2_t = ap_uint<2>;
using int2x16_t = ap_uint<2 * CHANNEL>;
template <typename T>
using fifo = hls::stream<T>;

extern "C" {
void maxpool_top(
  int in[24 * 24 * 16],
  int out[12 * 12 * 16]
);
}
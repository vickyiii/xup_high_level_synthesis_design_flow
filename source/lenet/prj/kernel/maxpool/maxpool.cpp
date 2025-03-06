#include "maxpool.hpp"
#include <ap_int.h>
#include <hls_stream.h>
#include <hls_vector.h>

namespace bit {
	template <int S>
	uint2_t getu(const ap_uint<2 * S>& src, const int idx) {
#pragma HLS inline
		int p = 2 * idx;
		return src(p + 2 - 1, p);
	}

	template <int S>
	void setu(ap_uint<2 * S>& src, const int idx, const uint2_t& v) {
#pragma HLS inline
		int p = 2 * idx;
		src(p + 2 - 1, p) = v;
	}
} // namespace bit

template <typename T, int H, int W, int C>
class MaxPool2x2 {
private:
	T maxpool(const T val1, const T val2) {
		T oval;
		for (int z = 0; z < C; z++) {
#pragma HLS unroll
			uint2_t v1 = bit::getu<C>(val1, z);
			uint2_t v2 = bit::getu<C>(val2, z);
			bit::setu<C>(oval, z, v1 > v2 ? v1 : v2);
		}
		return oval;
	}
public:
	void compute_h(fifo<T>& ins, fifo<T>& outs) {
		for (int xy = 0; xy < H * W; xy += 2) {
#pragma HLS pipeline
			T val1 = ins.read();
			T val2 = ins.read();
			T oval = maxpool(val1, val2);
			outs.write(oval);
		}
	}

	void compute_v(fifo<T>& ins, fifo<T>& outs) {
		T buf[W / 2];
#pragma HLS array_partition variable=buf

		for (int y = 0; y < H / 2; y++) {
#pragma HLS pipeline
			for (int x = 0; x < W / 2; x++) {
				T val = ins.read();
				buf[x] = val;
			}
			for (int x = 0; x < W / 2; x++) {
				T val1 = buf[x];
				T val2 = ins.read();
				T oval = maxpool(val1, val2);
				outs.write(oval);
			}
		}
	}
};

using MaxPool0 = MaxPool2x2<int2x16_t, 24, 24, 16>;

template<int H, int W, int C>
void read_input(const int in[H * W * C], fifo<int2x16_t>& ins) {
	int ptr = 0;
	for (int xy = 0; xy < H * W; xy++) {
#pragma HLS pipeline
		int2x16_t val;
		for (int z = 0; z < C; z++) {
#pragma HLS unroll
			bit::setu<C>(val, z, in[ptr++]);
		}
		ins.write(val);
	}
}

template<int H, int W, int C>
void write_result(int out[H * W * C], fifo<int2x16_t>& outs) {
	int ptr = 0;
	for (int xy = 0; xy < H * W; xy++) {
#pragma HLS pipeline
		int2x16_t val = outs.read();
		for (int z = 0; z < C; z++) {
#pragma HLS unroll
			out[ptr++] = bit::getu<C>(val, z);
		}
	}
}

void maxpool_top(int in[HEIGHT * WIDTH * CHANNEL],
	int out[OHEIGHT * OWIDTH * CHANNEL])
{
#pragma HLS interface axis port=in
#pragma HLS interface axis port=out
#pragma HLS array_partition variable=in cyclic factor=CHANNEL
#pragma HLS array_partition variable=out cyclic factor=CHANNEL

	fifo<int2x16_t> ins("input_fifo");
	fifo<int2x16_t> pips("pipe_fifo");
	fifo<int2x16_t> outs("output_fifo");

	MaxPool0 maxpool0;

#pragma HLS dataflow
	read_input<24, 24, 16>(in, ins);
	maxpool0.compute_h(ins, pips);
	maxpool0.compute_v(pips, outs);
	write_result<12, 12, 16>(out, outs);
}
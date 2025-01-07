#include "convolution0.hpp"
#include <ap_int.h>
#include <hls_stream.h>
#include <hls_vector.h>

namespace bit {
	template <int S>
	int2_t get(const ap_uint<2 * S>& src, const int idx) {
#pragma HLS inline
		int p = 2 * idx;
		return src(p + 2 - 1, p);
	}

	template <int S>
	void set(ap_uint<2 * S>& src, const int idx, const int2_t& v) {
#pragma HLS inline
		int p = 2 * idx;
		src(p + 2 - 1, p) = v;
	}

	// @see HD, Figure 3-3
	constexpr int clp2(int x) {
		x = x - 1;
		x = x | (x >> 1);
		x = x | (x >> 2);
		x = x | (x >> 4);
		x = x | (x >> 8);
		x = x | (x >> 16);
		return x + 1;
	}

	template <int S>
	int16_t multiply_add(ap_uint<2 * S>& vu, ap_uint<2 * S>& wi) {
		const int M = clp2(S);
		int16_t t[M];
#pragma HLS array_partition variable=t

		for (int i = 0; i < S; i++) {
#pragma HLS unroll
			uint2_t v = get<S>(vu, i);
			int2_t w = get<S>(wi, i);
			t[i] = v * w;
		}
		for (int i = S; i < M; i++) {
#pragma HLS unroll
			t[i] = 0;
		}

		for (int d = 1; d < M; d *= 2) {
			for (int i = 0; i < M; i += d * 2) {
#pragma HLS unroll
				t[i] += t[i + d];
			}
		}
		return t[0];
	}
} // namespace bit

template <int ROWS, int COLS, typename T>
class LineBuffer {
private:
	hls::vector<T, ROWS * COLS> buf;
public:
	void shift_pixels_up(int col) {
		for (int i = 0; i < ROWS * COLS - 1; i++) {
#pragma HLS unroll
			buf[i] = buf[i + 1];
		}
	}

	void insert_bottom_row(T value, int col) {
		buf[ROWS * COLS - 1] = value;
	}

	void get_col(T value[ROWS], int col) {
		for (int i = 0; i < ROWS; i++) {
#pragma HLS unroll
			value[i] = buf[i * COLS];
		}
	}
};

class Window_0 {
private:
	static const int ROWS = 5;
	static const int COLS = 5;

	int2x25_t buf;
public:
	void shift_pixels_left() {
#pragma HLS inline
		buf >>= 2;
	}

	void insert_right_col(const int2_t value[ROWS]) {
#pragma HLS inline
		for (int i = 0; i < ROWS; i++) {
#pragma HLS unroll
			int idx = (i + 1) * COLS - 1;
			bit::set<ROWS * COLS>(buf, idx, value[i]);
		}
	}

	// xf::cv::Window<ROWS, COLS, T>にI/Fを合わせたかったが
	// 泣く泣くWindowクラスに関数を追加
	// @see ug1399 Virtual Functions and Pointers: Not supported.

	void set_window(const int idx, const int weight[], int& ptr) {
#pragma HLS inline
		bit::set<ROWS * COLS>(buf, idx, weight[ptr++]);
	}

	int16_t muladd(Window_0& op) {
#pragma HLS inline
		return bit::multiply_add<ROWS * COLS>(buf, op.buf);
	}
};

template <typename WT, typename T, int H, int W, int C, int KH, int KW, int F>
class Conv2D {
private:
	static const int OH = H - KH + 1;
	static const int OW = W - KW + 1;

	//xf::cv::LineBuffer<KH - 1, W, T> linebuf;
	LineBuffer<KH - 1, W, T> linebuf;
	WT window;
public:
	void insert_linebuf(const int x, const T v) {
		linebuf.shift_pixels_up(x);
		linebuf.insert_bottom_row(v, x);
	}

	void slide_window(const int x, const T v) {
		T rows[KH];
#pragma HLS array_partition variable=rows

		linebuf.get_col(rows, x);
		rows[KH - 1] = v;
		insert_linebuf(x, v);

		window.shift_pixels_left();
		window.insert_right_col(rows);
	}

	WT& pack_window() {
		return window;
	}

	template <typename OT, int M>
	void compute(const int weight[F * KH * KW], const int thr[M],
		fifo<WT>& ins, fifo<OT>& outs)
	{
		int16_t threshold[M];
#pragma HLS array_partition variable=threshold
		WT filter[F];
#pragma HLS array_partition variable=filter

		int ptr = 0;
		for (int z = 0; z < F; z++) {
#pragma HLS pipeline
			for (int k = 0; k < KH * KW; k++) {
#pragma HLS unroll
				filter[z].set_window(k, weight, ptr);
			}
		}

		for (int n = 0; n < M; n++) {
#pragma HLS unroll
			threshold[n] = thr[n];
		}

		for (int xy = 0; xy < OH * OW; xy++) {
#pragma HLS pipeline
			WT val = ins.read();
			OT oval;
			for (int z = 0; z < F; z++) {
#pragma HLS unroll
				int16_t acc = val.muladd(filter[z]);
				uint2_t m = 0;
				for (int n = 0; n < M; n++) {
					if (acc >= threshold[n]) {
						m = n + 1;
					}
				}
				bit::set<F>(oval, z, m);
			}
			outs.write(oval);
		}
	}
};

using Conv0 = Conv2D<Window_0, int2_t, 28, 28, 1, 5, 5, 16>;

template <int H, int W, int KH, int KW>
void read_input(Conv0& conv, const int in[H * W], fifo<Window_0>& ins) {
	int ptr = 0;
	for (int y = 0; y < KH - 1; y++) {
#pragma HLS pipeline
		for (int x = 0; x < W; x++) {
#pragma HLS unroll
			uint2_t v = in[ptr++];
			conv.insert_linebuf(x, v);
		}
	}
	for (int y = KH - 1; y < H; y++) {
#pragma HLS pipeline
		for (int x = 0; x < KW - 1; x++) {
#pragma HLS unroll
			uint2_t v = in[ptr++];
			conv.slide_window(x, v);
		}
		for (int x = KW - 1; x < W; x++) {
#pragma HLS unroll
			uint2_t v = in[ptr++];
			conv.slide_window(x, v);

			Window_0 val = conv.pack_window();
			ins.write(val);
		}
	}
}

template <int H, int W, int F>
void write_result(int out[H * W * F], fifo<int2x16_t>& outs) {
	int ptr = 0;
	for (int xy = 0; xy < H * W; xy++) {
#pragma HLS pipeline
		int2x16_t val = outs.read();
		for (int z = 0; z < F; z++) {
#pragma HLS unroll
			uint2_t v = bit::get<F>(val, z);
			out[ptr++] = v;
		}
	}
}

void convolution0(
	int in[HEIGHT * WIDTH],
	int weight[FILTER * KERNEL * KERNEL],
	int threshold[THRESHOLD],
	int out[OHEIGHT * OWIDTH * FILTER])
{
#pragma HLS interface axis port=in
#pragma HLS interface axis port=weight
#pragma HLS interface axis port=threshold
#pragma HLS interface axis port=out
#pragma HLS array_partition variable=in cyclic factor=WIDTH
#pragma HLS array_partition variable=weight cyclic factor=KERNEL * KERNEL
#pragma HLS array_partition variable=out cyclic factor=FILTER

	fifo<Window_0> ins("input_fifo");
	fifo<int2x16_t> outs("output_fifo");

	Conv0 conv0;

#pragma HLS dataflow
	read_input<28, 28, 5, 5>(conv0, in, ins);
	conv0.compute<int2x16_t, 3>(weight, threshold, ins, outs);
	write_result<24, 24, 16>(out, outs);
}
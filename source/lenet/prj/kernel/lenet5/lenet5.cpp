#include "lenet5.hpp"
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

	template <int S>
	uint2_t getu(const ap_uint<2 * S>& src, const int idx) {
#pragma HLS inline
		int p = 2 * idx;
		return src(p + 2 - 1, p);
	}

	// find the spallest power of 2 
	// which is greater than or equal to the given x
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
			uint2_t v = getu<S>(vu, i);
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

template <int KH, int W, typename T, typename WT>
class LineBuffer {
private:
	hls::vector<T, (KH - 1) * W> buf;
	WT window;

	void shift_pixels_up() {
#pragma HLS inline
		for (int i = 0; i < (KH - 1) * W - 1; i++) {
#pragma HLS unroll
			buf[i] = buf[i + 1];
		}
	}

	void insert_bottom_row(T value) {
#pragma HLS inline
		buf[(KH - 1) * W - 1] = value;
	}

	void get_col(T value[KH - 1]) {
#pragma HLS inline
		for (int i = 0; i < KH - 1; i++) {
#pragma HLS unroll
			value[i] = buf[i * W];
		}
	}
public:
	void insert_linebuf(const T v) {
		shift_pixels_up();
		insert_bottom_row(v);
	}

	void slide_window(const T v) {
		T rows[KH];
#pragma HLS array_partition variable=rows

		get_col(rows);
		rows[KH - 1] = v;
		shift_pixels_up();
		insert_bottom_row(v);

		window.shift_pixels_left();
		window.insert_right_col(rows);
	}

	WT& get_window() {
		return window;
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
		for (int i = 0; i < ROWS; i++) {
#pragma HLS unroll
			int idx = (i + 1) * COLS - 1;
			bit::set<ROWS * COLS>(buf, idx, value[i]);
		}
	}

	// I wanted to match the interface with xf::cv::Window<ROWS, COLS, T>
	// but had to add functions to Window class instead
	// @see ug1399 Virtual Functions and Pointers: Not supported.

	void set_window(const int idx, const int weight[], int offset) {
#pragma HLS inline
		bit::set<ROWS * COLS>(buf, idx, weight[offset]);
	}

	int16_t muladd(Window_0& op) {
		return bit::multiply_add<ROWS * COLS>(buf, op.buf);
	}
};

class Window_1 {
private:
	static const int ROWS = 5;
	static const int COLS = 5;
	static const int C = 16;

	hls::vector<int2x16_t, ROWS * COLS> buf;
public:
	void shift_pixels_left() {
		for (int i = 0; i < ROWS * COLS - 1; i++) {
#pragma HLS unroll
			buf[i] = buf[i + 1];
		}
	}

	void insert_right_col(const int2x16_t value[ROWS]) {
		for (int i = 0; i < ROWS; i++) {
#pragma HLS unroll
			int idx = (i + 1) * COLS - 1;
			buf[idx] = value[i];
		}
	}

	void set_window(const int idx, const int weight[], int offset) {
		int2x16_t val;
		for (int z = 0; z < C; z++) {
#pragma HLS unroll
			int2_t v = weight[offset + z];
			bit::set<C>(val, z, v);
		}
		buf[idx] = val;
	}

	int16_t muladd(const Window_1& weight) {
		int16_t acc = 0;
		for (int i = 0; i < ROWS * COLS; i++) {
#pragma HLS pipeline
			int2x16_t vu = buf[i];
			int2x16_t wi = weight.buf[i];
			acc += bit::multiply_add<C>(vu, wi);
		}
		return acc;
	}
};

template <typename WT, int H, int W, int C, int KH, int KW, int F, int M>
class Conv2D {
protected:
	static const int OH = H - KH + 1;
	static const int OW = W - KW + 1;

	WT filter[F];
	int16_t threshold[M];

	uint2_t reduce_bit(int16_t sum) {
		uint2_t m = 0;
		for (int n = 0; n < M; n++) {
			if (sum >= threshold[n]) {
				m = n + 1;
			}
		}
		return m;
	}
public:
	void load(const int weight[F * KH * KW], const int thr[M]) {
		int ptr = 0;
		for (int z = 0; z < F; z++) {
#pragma HLS pipeline
			for (int k = 0; k < KH * KW; k++) {
				filter[z].set_window(k, weight, ptr);
				ptr += C;
			}
		}

		for (int n = 0; n < M; n++) {
#pragma HLS unroll
			threshold[n] = thr[n];
		}
	}
};

template <typename WT, int H, int W, int C, int KH, int KW, int F, int M>
class Conv2D_0 : public Conv2D<WT,H,W,C,KH,KW,F,M> {
public:
	template <typename OT>
	void compute(fifo<WT>& ins, fifo<OT>& outs) {
		for (int xy = 0; xy < this->OH * this->OW; xy++) {
#pragma HLS pipeline
			WT val = ins.read();
			OT oval;
			for (int z = 0; z < F; z++) {
				int16_t acc = val.muladd(this->filter[z]);
				uint2_t m = this->reduce_bit(acc);
				bit::set<F>(oval, z, m);
			}
			outs.write(oval);
		}
	}
};

template <typename WT, int H, int W, int C, int KH, int KW, int F, int M>
class Conv2D_1 : public Conv2D<WT,H,W,C,KH,KW,F,M> {
public:
	template <typename OT>
	void compute(fifo<WT>& ins, fifo<OT>& outs) {
		for (int xy = 0; xy < this->OH * this->OW; xy++) {
#pragma HLS pipeline off
// Don't know how to avoid resource overflow, so use pragma to bypass it
// "How to pipeline nested loops in Vivado HLS" 
// @see https://qiita.com/iwatake2222/items/baf307e1dae22ebda43b

			WT val = ins.read();
			OT oval;
			for (int z = 0; z < F; z++) {
#pragma HLS pipeline
				int16_t acc = val.muladd(this->filter[z]);
				// @FIXME
				uint2_t m = this->reduce_bit(acc);
				bit::set<F>(oval, z, m);
			}
			outs.write(oval);
		}
	}
};

template <typename T, int H, int W, int C>
class MaxPool2x2 {
private:
	T maxpool(const T val1, const T val2) {
		T oval;
		for (int z = 0; z < C; z++) {
#pragma HLS unroll
			uint2_t v1 = bit::getu<C>(val1, z);
			uint2_t v2 = bit::getu<C>(val2, z);
			bit::set<C>(oval, z, v1 > v2 ? v1 : v2);
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

template <typename IT, typename OT, int FL, int CL, int K>
class Dense {
private:
	IT mat[CL * FL / K];
public:
	void load(const int weight[CL * FL]) {
		int ptr = 0;
		for (int i = 0; i < CL * FL / K; i++) {
#pragma HLS pipeline
			int p = (i & 0xf) * CL + (i >> 4);
			for (int k = 0; k < K; k++) {
#pragma HLS unroll
				int2_t w = weight[ptr++];
				bit::set<K>(mat[p], k, w);
			}
		}
	}

	void compute_muladd(fifo<IT>& ins, fifo<OT>& outs) {
		for (int j = 0; j < FL / K; j++) {
#pragma HLS pipeline
			IT vu = ins.read();
			for (int i = 0; i < CL; i++) {
#pragma HLS unroll
				IT wi = mat[j * CL + i];
				OT acc = bit::multiply_add<K>(vu, wi);
				outs.write(acc);
			}
		}
	}
};

using Conv0 = Conv2D_0<Window_0, 28, 28, 1, 5, 5, 16, 3>;
using MaxPool0 = MaxPool2x2<int2x16_t, 24, 24, 16>;
using Conv1 = Conv2D_1<Window_1, 12, 12, 16, 5, 5, 16, 3>;
using MaxPool1 = MaxPool2x2<int2x16_t, 8, 8, 16>;
using MatMul0 = Dense<int2x16_t, int16_t, 256, 10, 16>;

template <int H, int W, int KH, int KW>
void read_input(const int in[H * W], fifo<Window_0>& ins) {
	LineBuffer<KH, W, int2_t, Window_0> linebuf;

	int ptr = 0;
	for (int y = 0; y < KH - 1; y++) {
		for (int x = 0; x < W; x++) {
#pragma HLS pipeline
			uint2_t v = in[ptr++];
			linebuf.insert_linebuf(v);
		}
	}
	for (int y = KH - 1; y < H; y++) {
		for (int x = 0; x < KW - 1; x++) {
#pragma HLS pipeline
			uint2_t v = in[ptr++];
			linebuf.slide_window(v);
		}
		for (int x = KW - 1; x < W; x++) {
#pragma HLS pipeline
			uint2_t v = in[ptr++];
			linebuf.slide_window(v);

			Window_0 oval = linebuf.get_window();
			ins.write(oval);
		}
	}
}

template <int H, int W, int C, int KH, int KW>
void pass_through(fifo<int2x16_t>& ins, fifo<Window_1>& outs) {
	LineBuffer<KH, W, int2x16_t, Window_1> linebuf;

	for (int y = 0; y < KH - 1; y++) {
		for (int x = 0; x < W; x++) {
#pragma HLS pipeline
			int2x16_t val = ins.read();
			linebuf.insert_linebuf(val);
		}
	}
	for (int y = KH - 1; y < H; y++) {
		for (int x = 0; x < KW - 1; x++) {
#pragma HLS pipeline
			int2x16_t val = ins.read();
			linebuf.slide_window(val);
		}
		for (int x = KW - 1; x < W; x++) {
#pragma HLS pipeline
			int2x16_t val = ins.read();
			linebuf.slide_window(val);

			Window_1 oval = linebuf.get_window();
			outs.write(oval);
		}
	}
}

template <int FL, int CL, int K>
void write_result(hls::stream<int32_pkt>& out, fifo<int16_t>& outs) {
	static int16_t acc[CL];
	int32_pkt out_temp;
#pragma HLS array_partition variable=acc

	for (int i = 0; i < CL; i++) {
#pragma HLS unroll
		acc[i] = 0;
	}

	for (int j = 0; j < FL / K; j++) {
		for (int i = 0; i < CL; i++) {
#pragma HLS pipeline
			int16_t val = outs.read();
			acc[i] += val;
			if (j == FL / K - 1) {
				out_temp.data = acc[i];
				out_temp.keep = -1;
				if(i == CL-1)
					out_temp.last = 1;
				else
					out_temp.last = 0;
				out.write(out_temp);
			}
		}
	}
}

void lenet5(
	int in[HEIGHT * WIDTH],
	int conv0_weight[FILTER * KERNEL * KERNEL],
	int conv1_weight[FILTER * KERNEL * KERNEL * CHANNEL],
	int matmul0_weight[FLATTEN * CLASS],
	hls::stream<int32_pkt>& out)
{
#pragma HLS INTERFACE mode=s_axilite bundle=CTRL port=return
#pragma HLS interface axis port=in
#pragma HLS interface axis port=conv0_weight
#pragma HLS interface axis port=conv1_weight
#pragma HLS interface axis port=matmul0_weight
#pragma HLS interface axis port=out

	fifo<Window_0> ins("input_fifo");
	fifo<int2x16_t> pips1("pipe_fifo1");
	fifo<int2x16_t> pips2("pipe_fifo2");
	fifo<int2x16_t> pips3("pipe_fifo3");
	fifo<Window_1> pips4("pipe_fifo4");
	fifo<int2x16_t> pips5("pipe_fifo5");
	fifo<int2x16_t> pips6("pipe_fifo6");
	fifo<int2x16_t> pips7("pipe_fifo7");
	fifo<int16_t> outs("output_fifo");

	Conv0 conv0;
	MaxPool0 maxpool0;
	Conv1 conv1;
	MaxPool1 maxpool1;
	MatMul0 matmul0;

	static int threshold0[3] = { 1, 3, 4, };
	static int threshold1[3] = { 3, 9, 14, };

	conv0.load(conv0_weight, threshold0);
	conv1.load(conv1_weight, threshold1);
	matmul0.load(matmul0_weight);

#pragma HLS dataflow
	read_input<28, 28, 5, 5>(in, ins);
	conv0.compute<int2x16_t>(ins, pips1);
	maxpool0.compute_h(pips1, pips2);
	maxpool0.compute_v(pips2, pips3);
	pass_through<12, 12, 16, 5, 5>(pips3, pips4);
	conv1.compute<int2x16_t>(pips4, pips5);
	maxpool1.compute_h(pips5, pips6);
	maxpool1.compute_v(pips6, pips7);
	matmul0.compute_muladd(pips7, outs);
	write_result<256, 10, 16>(out, outs);
}

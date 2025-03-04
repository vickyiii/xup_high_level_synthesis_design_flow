#include "convolution1.hpp"
#include <ap_int.h>
#include <hls_stream.h>
#include <hls_vector.h>

// const int WIDTH = 12;
// const int HEIGHT = 12;
// const int CHANNEL = 16;

// const int FILTER = 16;
// const int KERNEL = 5;
// const int THRESHOLD = 3;

// const int OWIDTH = 4;
// const int OHEIGHT = 4;

// using int2_t = ap_int<2>;
// using uint2_t = ap_uint<2>;
// using int2x16_t = ap_uint<2 * CHANNEL>;
// using pack1_t = hls::vector<int2x16_t, KERNEL * KERNEL>;
// template <typename T>
// using fifo = hls::stream<T>;

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
                for (int k = 0; k < ROWS * COLS; k++) {
                        int2x16_t vu = buf[k];
                        int2x16_t wi = weight.buf[k];
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
class Conv2D_1 : public Conv2D<WT,H,W,C,KH,KW,F,M> {
public:
        template <typename OT>
        void compute(fifo<WT>& ins, fifo<OT>& outs) {
                for (int xy = 0; xy < this->OH * this->OW; xy++) {
#pragma HLS pipeline off
// リソース超過の回避方法がわからないためプラグマで回避
// 「Vivado HLSで多重ループをパイプライン化」
// @see https://qiita.com/iwatake2222/items/baf307e1dae22ebda43b

                        WT val = ins.read();
                        OT oval;
                        for (int z = 0; z < F; z++) {
#pragma HLS pipeline
                                int16_t acc = val.muladd(this->filter[z]);
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
                        for (int x = 0; x < W / 2; x++) {
#pragma HLS pipeline
                                T val = ins.read();
                                buf[x] = val;
                        }
                        for (int x = 0; x < W / 2; x++) {
#pragma HLS pipeline
                                T val1 = buf[x];
                                T val2 = ins.read();
                                T oval = maxpool(val1, val2);
                                outs.write(oval);
                        }
                }
        }
};

using Conv1 = Conv2D_1<Window_1, 12, 12, 16, 5, 5, 16, 3>;
using MaxPool1 = MaxPool2x2<int2x16_t, 8, 8, 16>;

template <int C>
int2x16_t read_channel(const int in[], const int offset) {
        int2x16_t val;
        for (int z = 0; z < C; z++) {
#pragma HLS unroll
                uint2_t v = in[offset + z];
                bit::set<C>(val, z, v);
        }
        return val;
}

template<int H, int W, int C, int KH, int KW>
void read_input(const int in[H * W * C], fifo<Window_1>& ins) {
        LineBuffer<KH, W, int2x16_t, Window_1> linebuf;

        int ptr = 0;
        for (int y = 0; y < KH - 1; y++) {
                for (int x = 0; x < W; x++) {
#pragma HLS pipeline
                        int2x16_t val = read_channel<C>(in, ptr);
                        ptr += C;
                        linebuf.insert_linebuf(val);
                }
        }
        for (int y = KH - 1; y < H; y++) {
                for (int x = 0; x < KW - 1; x++) {
#pragma HLS pipeline
                        int2x16_t val = read_channel<C>(in, ptr);
                        ptr += C;
                        linebuf.slide_window(val);
                }
                for (int x = KW - 1; x < W; x++) {
#pragma HLS pipeline
                        int2x16_t val = read_channel<C>(in, ptr);
                        ptr += C;
                        linebuf.slide_window(val);

                        Window_1 oval = linebuf.get_window();
                        ins.write(oval);
                }
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
                        uint2_t v = bit::getu<C>(val, z);
                        out[ptr++] = v;
                }
        }
}

void convolution1(
        int in[HEIGHT * WIDTH * CHANNEL],
        int weight[FILTER * KERNEL * KERNEL * CHANNEL],
        int threshold[THRESHOLD],
        int out[OHEIGHT * OWIDTH * FILTER])
{
#pragma HLS interface axis port=in
#pragma HLS interface axis port=weight
#pragma HLS interface axis port=threshold
#pragma HLS interface axis port=out
#pragma HLS array_partition variable=in cyclic factor=CHANNEL
#pragma HLS array_partition variable=weight cyclic factor=CHANNEL
#pragma HLS array_partition variable=out cyclic factor=FILTER

        fifo<Window_1> ins("input_fifo");
        fifo<int2x16_t> pips1("pipe_fifo1");
        fifo<int2x16_t> pips2("pipe_fifo2");
        fifo<int2x16_t> outs("output_fifo");

        Conv1 conv1;
        MaxPool1 maxpool1;

        conv1.load(weight, threshold);

#pragma HLS dataflow
        read_input<12, 12, 16, 5, 5>(in, ins);
        conv1.compute<int2x16_t>(ins, pips1);
        maxpool1.compute_h(pips1, pips2);
        maxpool1.compute_v(pips2, outs);
        write_result<4, 4, 16>(out, outs);
}
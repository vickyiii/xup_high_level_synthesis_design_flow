# Implementing and Optimizing LeNet-5 with Vitis HLS

## 1. LeNet-5 Architecture Design

### 1.1 Data Flow Graph

conv0               maxpool0
            ┌────────┐            ┌───────┐
   28x28x1  │ Conv2D │  24x24x16  │MaxPool│  12x12x16
  ─────────►│16x5x5x1├───────────►│  2x2  ├────────────┐
   (Input)  │ no pad │            │       │            │
            └────────┘            └───────┘            │
                                                       │
             ┌─────────────────────────────────────────┘
             │
             │       conv1             maxpool1
             │    ┌─────────┐          ┌───────┐
             │    │ Conv2D  │  8x8x16  │MaxPool│  4x4x16
             └───►│16x5x5x16├─────────►│  2x2  ├──────────┐
                  │ no pad  │          │       │          │
                  └─────────┘          └───────┘          │
                                                          │
                   ┌──────────────────────────────────────┘
                   │
                   │                     matmul0
                   │     ┌───────┐       ┌──────┐
                   │     │Flatten│  256  │MatMul│   10
                   └────►│       ├──────►│10x256├─────────►
                         │       │       │      │ (Output)
                         └───────┘       └──────┘


1. get 和 Set 是怎么

这里提供了两个模板函数，`get` 和 `set`，它们用于从 `ap_uint` 类型的数据中提取和设置特定索引位置的值。下面对每个函数进行解释：

template <int S>
int2_t get(const ap_uint<2 * S>& src, const int idx) {
#pragma HLS inline
    int p = 2 * idx;
    return src(p + 2 - 1, p);
}
</code></div></div></pre>

这个函数模板 `get` 接受一个类型为 `ap_uint<2 * S>` 的参数 `src`，以及一个整数类型的参数 `idx`，表示要提取的值的索引。在函数体内部，`p` 被计算为 `2 * idx`，以确定要提取的位范围。然后，使用 Vivado HLS 提供的位选择操作符，从 `src` 中选择位范围 `(p + 2 - 1, p)`，并将其转换为 `int2_t` 类型后返回。

template <int S>
void set(ap_uint<2 * S>& src, const int idx, const int2_t& v) {
#pragma HLS inline
    int p = 2 * idx;
    src(p + 2 - 1, p) = v;
}
</code></div></div></pre>

这个函数模板 `set` 接受一个类型为 `ap_uint<2 * S>` 的引用参数 `src`，一个整数类型的参数 `idx`，表示要设置的值的索引，以及一个类型为 `int2_t` 的参数 `v`，表示要设置的值。在函数体内部，`p` 被计算为 `2 * idx`，以确定要设置的位范围。然后，使用 Vivado HLS 提供的位选择操作符，将 `v` 的值赋值给 `src` 中的相应位范围。

这两个函数都使用了 `#pragma HLS inline` 指令，以提示 HLS 编译器在综合时尽可能地将函数内联，以便优化性能和资源利用。

#pragma once
constexpr int IC = 256;
constexpr int OC = 10;
constexpr int PAR = 16;

extern "C" {
void fully_connected(
  int in[256],
  int weight[10 * 256],
  int out[10]
);
}
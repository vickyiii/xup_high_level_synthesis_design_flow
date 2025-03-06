# Implementing and Optimizing LeNet-5 with Vitis HLS

1. Overview

LeNet-5 is one of the most fundamental convolution neural networks(CNNs). It was originally designed to recognize handwritten digits and achieved remarkable success on the MNIST dataset. The network features a hierarchical design with convolutional layers to extract features, followed by pooling layers to reduce dimensionality, and finally fully connected layers for classification.

In this project-based lab, we would implement a accelerator to accelerate a simplified LeNet-5, to classify MNIST images(recognize the handwritten digit). The goal is to enhance the computation speed for a simplified LeNet-5 with int2 quantization.

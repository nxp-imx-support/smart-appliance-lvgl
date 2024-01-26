/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.
# Copyright 2024 NXP Semiconductors
#
# This file was copied from TensorFlow respecting its rights. All the modified
# parts below are according to TensorFlow's LICENSE terms.
#
# SPDX-License-Identifier:    Apache-2.0
==============================================================================*/
#pragma once

#include <cstdint>
#include <vector>
#include <chrono>
#include <iostream>
#include <fstream>
#include <cmath>

#include <tensorflow/lite/model.h>
#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/delegates/external/external_delegate.h>

#include <opencv2/opencv.hpp>


struct Prediction
{
    std::vector<cv::Rect> boxes;
    std::vector<float> scores;
    std::vector<int> labels;
};

class YOLOV5
{
public:
    YOLOV5(const std::string model, int npu_tpye, int num_threads);
    ~YOLOV5();

    //Take an image and return a prediction
    void run(cv::Mat frame, Prediction &out_pred);

    void getLabelsName(std::string path, std::vector<std::string> &labelNames);

    // thresh hold
    float confThreshold = 0.3;
    float nmsThreshold = 0.5;

    // number of threads
    int nthreads = 4;
    int npu_tpye = 2;    // 0:none 1:imx8mp 2:imx93 3:imx95

private:
    // model's
    std::unique_ptr<tflite::FlatBufferModel> model_;
    std::unique_ptr<tflite::Interpreter> interpreter_;

    // parameters of interpreter's input
    int _input;
    int _in_height;
    int _in_width;
    int _in_channels;
    int _in_type;

    float _mean = 0.f;
    float _std = 255.f;

    // parameters of interpreter's output
    int _out;
    TfLiteIntArray* _out_dims;
    int  _out_row;
    int  _out_colum;
    // int  _out_type;                   =

    // parameters of original image
    int _img_height;
    int _img_width;

    // Input of the interpreter
    uint8_t *_input_u8;
    float_t *_input_f32;

    template <typename T>
    void seed_data(T *in, cv::Mat &src);

    void preprocess(cv::Mat &image);
    void preprocess(cv::Mat &image, cv::Mat & padded_image);
    std::vector<std::vector<float>> tensorToVector2D(TfLiteTensor *pOutputTensor, const int &row, const int &colum);
    void nonMaximumSupprition(
        std::vector<std::vector<float>> &predV,
        const int &row,
        const int &colum,
        std::vector<cv::Rect> &boxes,
        std::vector<float> &confidences,
        std::vector<int> &classIds,
        std::vector<int> &indices);

    void apply_delegate(int npu_tpye);
};
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

class YOLOV4
{
public:
    YOLOV4(const std::string model, int npu_tpye, int num_threads);
    ~YOLOV4();

    void run(cv::Mat frame, Prediction &result);
    void getLabelsName(std::string path, std::vector<std::string> &labelNames);
    void draw_result(cv::Mat& frame, Prediction result);

    float confThreshold;
    float nmsThreshold;

private:
    // number of threads
    int nthreads;
    int npu_tpye;    // 0:none 1:imx8mp 2:imx93 3:imx95

    // model's
    std::unique_ptr<tflite::FlatBufferModel> model_;
    std::unique_ptr<tflite::Interpreter> interpreter_;

    // parameters of interpreter's input
    int input;
    int in_height;
    int in_width;
    int in_channels;
    int in_type;

    float _mean = 0.f;
    float _std = 255.f;

    // parameters of interpreter's output
    int _out;
    TfLiteIntArray* _out_dims;
    int  _out_row;
    int  _out_colum;

    // parameters of original image
    int padded_img_height;
    int padded_img_width;

    // Input of the interpreter
    uint8_t *_input_u8;
    float_t *_input_f32;

    template <typename T>
    void seed_data(T *in, cv::Mat &src);

    void apply_delegate(int npu_tpye);
    void preprocess(cv::Mat image, cv::Mat & padded_image, cv::Mat& resized_image);
    void draw_img(int classId, float conf, int left, int top, int right, int bottom, cv::Mat& frame);
};
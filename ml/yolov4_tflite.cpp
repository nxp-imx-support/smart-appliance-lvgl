/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.
# Copyright 2024 NXP Semiconductors
#
# This file was copied from TensorFlow respecting its rights. All the modified
# parts below are according to TensorFlow's LICENSE terms.
#
# SPDX-License-Identifier:    Apache-2.0
==============================================================================*/
#include "yolov4_tflite.h"

#include <tensorflow/lite/model.h>
#include <tensorflow/lite/interpreter.h>

#define CLASS_NUM 9

std::string labelNames[CLASS_NUM] = {
    "fresh_apple", "normal_apple", "rotten_apple", "fresh_banana", "normal_banana",
    "rotten_banana", "fresh_orange", "normal_orange", "rotten_orange"
};

YOLOV4::YOLOV4(const std::string model_path, int npu_tpye, int num_threads)
{
    // configs init
    confThreshold = 0.5;
    nmsThreshold = 0.5;
    nthreads = 4;
    npu_tpye = 2;

    // load model
    std::ifstream file(model_path);
    if (!file) {
        printf("Failed to open %s \n", model_path.c_str());
        exit(1);
    }
    model_ = tflite::FlatBufferModel::BuildFromFile(model_path.c_str());
    if (!model_) {
       printf ("Failed to mmap model %s \n", model_path.c_str());
        exit(1);
    }

    // create interpreter
    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder(*model_, resolver)(&interpreter_);
    if (!interpreter_) {
        printf ("Failed to construct TFLite interpreter");
        exit(1);
    }

    // add npu delegate
    if ( npu_tpye ) {
        apply_delegate(npu_tpye);
    } else {
        interpreter_->SetNumThreads(num_threads);
    }

    // alloc tensor
    TfLiteStatus status = interpreter_->AllocateTensors();
    if (status != kTfLiteOk)
    {
        printf ("Failed to allocate the memory for tensors. \n");
        exit(1);
    }

    // input information
    input = interpreter_->inputs()[0];
    TfLiteIntArray *dims = interpreter_->tensor(input)->dims;
    in_height = dims->data[1];
    in_width = dims->data[2];
    in_channels = dims->data[3];
    in_type = interpreter_->tensor(input)->type;

    std::cout << "YOLO Model Input type: " << in_type << "\n";
    if (in_type == kTfLiteFloat32) {
        _input_f32 = interpreter_->typed_tensor<float_t>(input);
    } else if (in_type == kTfLiteUInt8) {
        _input_u8 = interpreter_->typed_tensor<uint8_t>(input);
    } else {
        std::cout << "YOLO Model Input type donot support yet\n";
        exit(0);
    }

    std::cout << "YOLO Model Input Shape:[1][" << in_height << "][" <<in_width
            << "][" << in_channels << "]\n";
}

YOLOV4::~YOLOV4() {}

void YOLOV4::apply_delegate(int npu_tpye)
{
    // assume TFLite v2.0 or newer
    std::map<std::string, tflite::Interpreter::TfLiteDelegatePtr> delegates;
    std::string delegate_path;
    switch (npu_tpye){
        case 1:
            break;
        case 2:
        {
            printf("NPU type: ethou-u65 \n");
            delegate_path = "/usr/lib/libethosu_delegate.so";
        }
            break;
        default:
            break;
    }

    auto ext_delegate_option =
                    TfLiteExternalDelegateOptionsDefault(delegate_path.c_str());
    auto ext_delegate_ptr = TfLiteExternalDelegateCreate(&ext_delegate_option);
    auto delegate = tflite::Interpreter::TfLiteDelegatePtr(ext_delegate_ptr, [](TfLiteDelegate*) {});
    if (!delegate) {
      printf("delegate backend is unsupported on this platform.");
    } else {
      delegates.emplace("IMX NPU", std::move(delegate));
    }

    for (const auto& delegate : delegates) {
        if (interpreter_->ModifyGraphWithDelegate(delegate.second.get()) != kTfLiteOk) {
            printf("Failed to apply %s delegate.", delegate.first.c_str());
            exit(1);
        } else {
                printf("Applied %s delegate. \n", delegate.first.c_str());
        }
    }

}

void YOLOV4::run(cv::Mat frame, Prediction &result)
{

    if (!frame.data) {
      std::cout << "input image is empty!\n";
      std::cout << __FILE__ << ": " << __LINE__ << std::endl;
      exit(-1);
    }

    // preprocess
    cv::Mat padded_frame, resized_frame;
    preprocess(frame, padded_frame, resized_frame);
    padded_img_height = padded_frame.rows;
    padded_img_width = padded_frame.cols;
    if (in_type == kTfLiteFloat32) {
      seed_data(_input_f32, resized_frame);
    } else if (in_type == kTfLiteUInt8) {
      seed_data(_input_u8, resized_frame);
    }

    // Inference
    TfLiteStatus status = interpreter_->Invoke();
    if (status != kTfLiteOk)
    {
        std::cout << "\nFailed to run inference!!\n";
        exit(1);
    }

    // get output
    TfLiteTensor* output_locations = nullptr;
    TfLiteTensor* output_scores = nullptr;

    // output_locations = interpreter_->tensor(interpreter_->outputs()[0]);
    // auto output_data = output_locations->data.f;

    // output_scores = interpreter_->tensor(interpreter_->outputs()[1]);
    // auto scores = output_scores->data.f;

    output_scores = interpreter_->tensor(interpreter_->outputs()[0]);
    auto scores = output_scores->data.f;

    output_locations = interpreter_->tensor(interpreter_->outputs()[1]);
    auto output_data = output_locations->data.f;

    std::vector<cv::Rect> objects;
    std::vector<float> score_vec;
    std::vector<int> class_ids;

    int box_nums = output_locations->dims->data[1];

    for(int i = 0; i < box_nums; i++) {
        auto xmin = (output_data[i * 4] - output_data[i * 4 + 2] / 2.0) / in_width * padded_img_width;
        auto ymin = (output_data[i * 4 + 1] - output_data[i * 4 + 3] / 2.0) / in_height * padded_img_height;
        auto xmax = (output_data[i * 4] + output_data[i * 4 + 2] / 2.0) / in_width * padded_img_width;
        auto ymax = (output_data[i * 4 + 1] + output_data[i * 4 + 3] / 2.0) / in_height * padded_img_height;
        auto width = xmax - xmin;
        auto height = ymax - ymin;

		float score = 0.0f;
		int class_num = 0;
	    for(int j = 0; j < CLASS_NUM; j++){
			if(scores[j + i * CLASS_NUM] > score){
				class_num = j;
				score = scores[j + i * CLASS_NUM];
			}
		}

        class_ids.push_back(class_num);
        score_vec.push_back(score);
        objects.push_back(cv::Rect(xmin, ymin, width, height));
	}

	std::vector<int> indices;
	cv::dnn::NMSBoxes(objects, score_vec, confThreshold, nmsThreshold, indices);



	for (size_t i = 0; i < indices.size(); ++i) {
		int idx = indices[i];
        result.boxes.push_back(objects[idx]);
        result.scores.push_back(score_vec[idx]);
        result.labels.push_back(class_ids[idx]);
	}
}

void YOLOV4::preprocess(cv::Mat image, cv::Mat & padded_image, cv::Mat& resized_image)
{
  // cv::Mat rgb_image;
  int pad_bottom = image.cols - image.rows;
  int pad_right = 0;

  // pad
  cv::copyMakeBorder(image, padded_image, 0, pad_bottom, 0, pad_right, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
  cv::resize(padded_image, resized_image, cv::Size(in_height,in_width), cv::INTER_CUBIC);
}

template <typename T>
void YOLOV4::seed_data(T *in, cv::Mat &src)
{
    if (in != NULL && src.data != NULL) {
      uchar *ptr = src.data;
      for (int i = 0; i < src.rows; i++) {
        for (int j = 0; j < src.cols * 3; j++) {
          in[i * src.cols * 3 + j] = ((T)(ptr[j]) - _mean) / _std;
        }
        ptr += src.step;
      }
    } else {
      std::cout << "input image or input tensor is empty!\n";
      std::cout << __FILE__ << ": " << __LINE__ << std::endl;
      exit(-1);
    }
}

void YOLOV4::draw_img(int classId, float conf, int left, int top, int right, int bottom, cv::Mat& frame)
{
  cv::rectangle(frame, cv::Point(left, top), cv::Point(right, bottom), cv::Scalar(0, 0, 255), 2);

  std::string label = cv::format("%.2f", conf);

  label = labelNames[classId] + ":" + label;


  int baseLine;
  cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.8, 1, &baseLine);
  top = cv::max(top, labelSize.height);
  cv::putText(frame, label, cv::Point(left, top), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
}

void YOLOV4::draw_result(cv::Mat& frame, Prediction result)
{
    for (size_t i = 0; i < result.boxes.size(); ++i) {
        draw_img(result.labels[i], result.scores[i], result.boxes[i].x, result.boxes[i].y,
                result.boxes[i].x + result.boxes[i].width, result.boxes[i].y + result.boxes[i].height, frame);
    }

    cv::cvtColor(frame, frame, cv::COLOR_RGB2BGR);
    cv::imwrite("./results-1.jpg", frame);
}
/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.
# Copyright 2024 NXP Semiconductors
#
# This file was copied from TensorFlow respecting its rights. All the modified
# parts below are according to TensorFlow's LICENSE terms.
#
# SPDX-License-Identifier:    Apache-2.0
==============================================================================*/
#include "yolov5_tflite.h"

#include <tensorflow/lite/model.h>
#include <tensorflow/lite/interpreter.h>

YOLOV5::YOLOV5(const std::string model_path, int npu_tpye, int num_threads)
{
    // load model
    std::ifstream file(model_path);
    if (!file) {
        printf("Failed to open %s", model_path.c_str());
        exit(1);
    }
    model_ = tflite::FlatBufferModel::BuildFromFile(model_path.c_str());
    if (!model_) {
       printf ("Failed to mmap model %s", model_path.c_str());
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
    _input = interpreter_->inputs()[0];
    TfLiteIntArray *dims = interpreter_->tensor(_input)->dims;
    _in_height = dims->data[1];
    _in_width = dims->data[2];
    _in_channels = dims->data[3];
    _in_type = interpreter_->tensor(_input)->type;

    std::cout << "YOLO Model Input type: " << _in_type << "\n";
    if (_in_type == kTfLiteFloat32) {
        _input_f32 = interpreter_->typed_tensor<float_t>(_input);
    } else if (_in_type == kTfLiteUInt8) {
        _input_u8 = interpreter_->typed_tensor<uint8_t>(_input);
    } else {
        std::cout << "YOLO Model Input type donot support yet\n";
        exit(0);
    }

    std::cout << "YOLO Model Input Shape:[1][" << _in_height << "][" << _in_width
            << "][" << _in_channels << "]\n";

}

YOLOV5::~YOLOV5() {}

void YOLOV5::apply_delegate(int npu_tpye)
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

void YOLOV5::run(cv::Mat frame, Prediction &out_pred)
{

    if (!frame.data) {
      std::cout << "input image is empty!\n";
      std::cout << __FILE__ << ": " << __LINE__ << std::endl;
      exit(-1);
    }

    // preprocess
    cv::Mat padded_frame;
    preprocess(frame, padded_frame);
    _img_height = padded_frame.rows;
    _img_width = padded_frame.cols;
    if (_in_type == kTfLiteFloat32) {
      seed_data(_input_f32, frame);
    } else if (_in_type == kTfLiteUInt8) {
      seed_data(_input_u8, frame);
    }


    // Inference
    TfLiteStatus status = interpreter_->Invoke();
    if (status != kTfLiteOk)
    {
        std::cout << "\nFailed to run inference!!\n";
        exit(1);
    }

    // get output
    _out = interpreter_->outputs()[0];
    _out_dims = interpreter_->tensor(_out)->dims;
    _out_row   = _out_dims->data[1];
    _out_colum = _out_dims->data[2];
    // // int _out_type  = interpreter_->tensor(_out)->type;

    // debug
    int out_batch = _out_dims->data[0];
    // printf("output dims: out_batch %d,  out[1] %d out[2] %d \n", out_batch, _out_row, _out_colum);

    TfLiteTensor *pOutputTensor = interpreter_->tensor(interpreter_->outputs()[0]);
    std::vector<std::vector<float>> predV = tensorToVector2D(pOutputTensor, _out_row, _out_colum);

    std::vector<int> indices;
    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    nonMaximumSupprition(predV, _out_row, _out_colum, boxes, confidences, classIds, indices);

    for (int i = 0; i < indices.size(); i++)
    {
        out_pred.boxes.push_back(boxes[indices[i]]);
        out_pred.scores.push_back(confidences[indices[i]]);
        out_pred.labels.push_back(classIds[indices[i]]);
    }
}

void YOLOV5::nonMaximumSupprition(
    std::vector<std::vector<float>> &predV,
    const int &row,
    const int &colum,
    std::vector<cv::Rect> &boxes,
    std::vector<float> &confidences,
    std::vector<int> &classIds,
    std::vector<int> &indices)

{

    std::vector<cv::Rect> boxesNMS;
    // int max_wh = 40960;
    std::vector<float> scores;
    double confidence;
    cv::Point classId;

    for (int i = 0; i < row; i++)
    {
        if (predV[i][4] > confThreshold)
        {
            // height--> image.rows,  width--> image.cols;
            int left = (predV[i][0] - predV[i][2] / 2) * _img_width;
            int top = (predV[i][1] - predV[i][3] / 2) * _img_height;
            int w = predV[i][2] * _img_width;
            int h = predV[i][3] * _img_height;

            for (int j = 5; j < colum; j++)
            {
                // # conf = obj_conf * cls_conf
                scores.push_back(predV[i][j] * predV[i][4]);
            }

            cv::minMaxLoc(scores, 0, &confidence, 0, &classId);
            scores.clear();
            // int c = classId.x * max_wh;
            if (confidence > confThreshold)
            {
                boxes.push_back(cv::Rect(left, top, w, h));
                confidences.push_back(confidence);
                classIds.push_back(classId.x);
                boxesNMS.push_back(cv::Rect(left, top, w, h));
            }
        }
    }
    cv::dnn::NMSBoxes(boxesNMS, confidences, confThreshold, nmsThreshold, indices);
}


void YOLOV5::preprocess(cv::Mat &image)
{
    cv::resize(image, image, cv::Size(_in_height, _in_width), cv::INTER_CUBIC);
}

void YOLOV5::preprocess(cv::Mat &image, cv::Mat & padded_image)
{
  // cv::Mat rgb_image;
  int pad_bottom = 160; //640 x480
  int pad_right = 0;

  // pad
  cv::copyMakeBorder(image, padded_image, 0, pad_bottom, 0, pad_right, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
  cv::resize(padded_image, image, cv::Size(_in_height, _in_width), cv::INTER_CUBIC);
}

template <typename T>
void YOLOV5::seed_data(T *in, cv::Mat &src)
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

std::vector<std::vector<float>> YOLOV5::tensorToVector2D(TfLiteTensor *pOutputTensor, const int &row, const int &colum)
{
  if (interpreter_->outputs().size() == 1) {
    pOutputTensor = interpreter_->tensor(interpreter_->outputs()[0]);
  } else {
    std::cout << "yolov5 don't support this model!\n";
    exit(-1);
  }

  std::vector<std::vector<float>> v;

  if (pOutputTensor->type == kTfLiteInt8) {
    auto scale = pOutputTensor->params.scale;
    auto zero_point = pOutputTensor->params.zero_point;
    for (int32_t i = 0; i < _out_row; i++) {
      std::vector<float> _tem;
      for (int j = 0; j < _out_colum; j++) {
        float val_float =
            (((int32_t)pOutputTensor->data.int8[i * _out_colum + j]) -
             zero_point) *
            scale;
        _tem.push_back(val_float);
      }
      v.push_back(_tem);
    }
  } else if (pOutputTensor->type == kTfLiteFloat32) {
    for (int32_t i = 0; i < _out_row; i++) {
      std::vector<float> _tem;
      for (int j = 0; j < _out_colum; j++) {
        float val_float = pOutputTensor->data.f[i * _out_colum + j];
        _tem.push_back(val_float);
      }
      v.push_back(_tem);
    }
  } else {
    std::cout << "Unsupported output type!\n";
    std::cout << __FILE__ << ": " << __LINE__ << std::endl;
    exit(-1);
  }
  return v;
}

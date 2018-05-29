#ifndef NEURALNET_H
#define NEURALNET_H

#include <neuralnet/neuralnet_api.h>
#include <neuralnet/tinyseg-net-structure.hpp>

#include <tuple>
#include <vector>

#include <dlib/dnn.h>
#include <opencv2/core.hpp>

class NEURALNET_API neuralnet
{
public:
    neuralnet();
    void train(std::vector<std::tuple<std::string,std::string>> image_label_tuples);
    cv::Mat predict(cv::Mat image);

protected:
    const size_t minibatch_size = 50;
    const double initial_learning_rate = 0.1;
    const double weight_decay = 0.0005;
    const double momentum = 0.0;

    const std::vector<cv::Scalar> label_colors = {
        cv::Scalar(127, 127, 127),
        cv::Scalar(255, 0, 0),
        cv::Scalar(0, 255, 0),
        cv::Scalar(0, 0, 255),
    };

    tinyseg::net_type net;
};

#endif // NEURALNET_H

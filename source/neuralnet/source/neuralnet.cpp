#include "neuralnet/neuralnet.h"
#include "neuralnet/tinyseg.hpp"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <dlib/data_io.h>

#include <numeric> // std::iota
#include <random>


neuralnet::neuralnet()
{

}

void neuralnet::train(std::vector<std::tuple<std::string,std::string>> image_label_tuples){
    std::deque<tinyseg::sample> samples = std::deque<tinyseg::sample>{};
    for(auto tup: image_label_tuples){
        samples.push_back(tinyseg::load_image(std::get<0>(tup), std::get<1>(tup), label_colors));
    }

    dlib::dnn_trainer<tinyseg::net_type> trainer(net, dlib::sgd(weight_decay, momentum));
    trainer.be_verbose();
    trainer.set_learning_rate(initial_learning_rate);
    trainer.set_synchronization_file("dnn_state.dat", std::chrono::seconds(10));
    trainer.set_iterations_without_progress_threshold(10);
    trainer.set_learning_rate_shrink_factor(0.95);
    trainer.set_max_num_epochs(100);

    tinyseg::training_dataset minibatch;
    const int input_tile_width = 150;
    const int input_tile_height = 150;

    minibatch.inputs.resize(minibatch_size);
    minibatch.labels.resize(minibatch_size);
    for (size_t i = 0; i < minibatch_size; ++i) {
        minibatch.inputs[i].set_size(input_tile_height, input_tile_width);
        minibatch.labels[i].set_size(input_tile_height, input_tile_width);
    }
    unsigned long epoch = 0;

    while (epoch++ < trainer.get_max_num_epochs()) {

        for (size_t i = 0; i < minibatch_size; ++i) {
            size_t index = rand() % samples.size();

            const tinyseg::sample& sample = samples[index];

            assert(sample.original_image.rows >= input_tile_height);
            assert(sample.original_image.cols >= input_tile_width);
            assert(sample.labels.rows >= input_tile_height);
            assert(sample.labels.cols >= input_tile_width);

            const int x0 = static_cast<size_t>(sample.original_image.cols - input_tile_width) * static_cast<size_t>(rand()) / static_cast<size_t>(RAND_MAX);
            const int y0 = static_cast<size_t>(sample.original_image.rows - input_tile_height) * static_cast<size_t>(rand()) / static_cast<size_t>(RAND_MAX);

            cv::Rect rect(x0, y0, input_tile_width, input_tile_height);

            tinyseg::to_dlib_matrix(cv::Mat_<cv::Vec3b>(sample.original_image(rect)), minibatch.inputs[i]);
            tinyseg::to_dlib_matrix(cv::Mat_<tinyseg::label_t>(sample.labels(rect)), minibatch.labels[i]);
        }

        trainer.train_one_step(minibatch.inputs, minibatch.labels);

    }

    trainer.get_net();
    net.clean();
    dlib::serialize("semantic_segmentation.dnn") << net;
}

cv::Mat neuralnet::predict(cv::Mat image)
{
    tinyseg::runtime_net_type runtime_net = net;
    dlib::deserialize("semantic_segmentation.dnn") >> runtime_net;

    cv::Mat roi; // no ROI

    cv::Mat input_image = image;

    tinyseg::image_t test_input(input_image.rows, input_image.cols);
    tinyseg::to_dlib_matrix(cv::Mat_<cv::Vec3b>(input_image), test_input);

    assert(test_input.nr() == input_image.rows);
    assert(test_input.nc() == input_image.cols);

    const dlib::matrix<tinyseg::label_t> predicted_labels = runtime_net(test_input);

    assert(test_input.nr() == predicted_labels.nr());
    assert(test_input.nc() == predicted_labels.nc());

    cv::Mat result(predicted_labels.nr(), predicted_labels.nc(), CV_8UC3);

    for (int y = 0; y < predicted_labels.nr(); ++y) {
        for (int x = 0; x < predicted_labels.nc(); ++x) {
            const tinyseg::label_t label = predicted_labels(y, x);
            const auto& label_color = label_colors[label];
            result.at<cv::Vec3b>(y, x) = cv::Vec3b(
                        static_cast<unsigned char>(label_color[0]),
                    static_cast<unsigned char>(label_color[1]),
                    static_cast<unsigned char>(label_color[2])
                    );
        }
    }

    cv::resize(result, result, input_image.size(), 0.0, 0.0, cv::INTER_NEAREST);

    return result;
}

#ifndef NEURALNET_H
#define NEURALNET_H

#include <neuralnet/neuralnet_api.h>
#include <neuralnet/tinyseg-net-structure.hpp>
#include <neuralnet/dnn_semantic_segmentation_ex.h>

#include <tuple>
#include <vector>

#include <dlib/dnn.h>
#include <opencv2/core.hpp>

class NEURALNET_API neuralnet
{
public:
    neuralnet();
    void train(std::vector<std::tuple<std::string,std::string>> image_label_tuples);
    cv::Mat predict(std::string imagefile);

    // A single training sample. A mini-batch comprises many of these.
    struct training_sample
    {
        dlib::matrix<dlib::rgb_pixel> input_image;
        dlib::matrix<uint16_t> label_image; // The ground-truth label of each pixel.
    };

    // The names of the input image and the associated RGB label image in the PASCAL VOC 2012
    // data set.
    struct image_info
    {
        std::string image_filename;
        std::string label_filename;
    };

    static dlib::rectangle make_random_cropping_rect_resnet(
            const dlib::matrix<dlib::rgb_pixel>& img,
            dlib::rand& rnd
            );

    static void randomly_crop_image (
            const dlib::matrix<dlib::rgb_pixel>& input_image,
            const dlib::matrix<uint16_t>& label_image,
            training_sample& crop,
            dlib::rand& rnd
            );

    static std::vector<image_info> get_listing(
            std::vector<std::tuple<std::string,std::string>> image_label_tuples);

    static const Voc2012class& find_label_class(const dlib::rgb_pixel& rgb_label);

    // Given an index in the range [0, 20], find the corresponding PASCAL VOC2012 class
    // (e.g., 'dog').
    static const Voc2012class& find_index_class(const uint16_t& index_label);

    // Convert an RGB class label to an index in the range [0, 20].
    inline static uint16_t rgb_label_to_index_label(const dlib::rgb_pixel& rgb_label)
    {
        return neuralnet::find_label_class(rgb_label).index;
    }

    // Convert an index in the range [0, 20] to a corresponding RGB class label.
    inline static dlib::rgb_pixel index_label_to_rgb_label(uint16_t index_label)
    {
        return neuralnet::find_index_class(index_label).rgb_label;
    }

    // Convert an image containing RGB class labels to a corresponding
    // image containing indexes in the range [0, 20].
    static void rgb_label_image_to_index_label_image(
        const dlib::matrix<dlib::rgb_pixel>& rgb_label_image,
        dlib::matrix<uint16_t>& index_label_image
    );

    // Convert an image containing indexes in the range [0, 20] to a corresponding
    // image containing RGB class labels.
    void index_label_image_to_rgb_label_image(
        const dlib::matrix<uint16_t>& index_label_image,
        dlib::matrix<dlib::rgb_pixel>& rgb_label_image
    );

    // Calculate the per-pixel accuracy on a dataset whose file names are supplied as a parameter.
    static double calculate_accuracy(anet_type& anet, const std::vector<image_info>& dataset);

    // Find the most prominent class label from amongst the per-pixel predictions.
    static std::string get_most_prominent_non_background_classlabel(const dlib::matrix<uint16_t>& index_label_image);

protected:
    const size_t minibatch_size = 2;//50;
    static const int input_tile_width = 227; //150;
    static const int input_tile_height = 227; //150;
    const double initial_learning_rate = 0.1;
    const double weight_decay = 0.0001;
    const double momentum = 0.9;

    const std::vector<cv::Scalar> label_colors = {
        cv::Scalar(127, 127, 127),
        cv::Scalar(255, 0, 0),
        cv::Scalar(0, 255, 0),
        cv::Scalar(0, 0, 255),
    };

    tinyseg::net_type net;
};

#endif // NEURALNET_H

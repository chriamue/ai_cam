// source: http://dlib.net/dnn_semantic_segmentation_ex.cpp.html

#include <neuralnet/neuralnet.h>
#include <neuralnet/dnn_semantic_segmentation_ex.h>
#include <logger/logger.h>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <numeric> // std::iota
#include <random>
#include <iostream>
#include <dlib/data_io.h>
#include <dlib/image_transforms.h>
#include <dlib/dir_nav.h>
#include <dlib/opencv.h>
#include <iterator>
#include <thread>


#include <dlib/gui_widgets.h>

using namespace std;
using namespace dlib;

neuralnet::neuralnet()
{
}

double neuralnet::train(std::vector<std::tuple<std::string,std::string>> image_label_tuples, int epochs){
    const auto listing = get_listing(image_label_tuples);
    Logger::getInstance().log("images in dataset: " + std::to_string( listing.size()));
    if (listing.size() == 0)
    {
        cout << "Didn't find the VOC2012 dataset. " << endl;
        return 0.0;
    }


    net_type net;
    dnn_trainer<net_type> trainer(net,sgd(weight_decay, momentum));
    trainer.be_verbose();
    trainer.set_learning_rate(initial_learning_rate);
    trainer.set_synchronization_file("trainer_state_file.dat", std::chrono::seconds(20));
    // This threshold is probably excessively large.
    trainer.set_iterations_without_progress_threshold(5000);
    // Since the progress threshold is so large might as well set the batch normalization
    // stats window to something big too.
    set_all_bn_running_stats_window_sizes(net, 1000);

    // Output training parameters.
    cout << endl << trainer << endl;

    std::vector<matrix<rgb_pixel>> samples;
    std::vector<matrix<uint16_t>> labels;

    // Start a bunch of threads that read images from disk and pull out random crops.  It's
    // important to be sure to feed the GPU fast enough to keep it busy.  Using multiple
    // thread for this kind of data preparation helps us do that.  Each thread puts the
    // crops into the data queue.
    dlib::pipe<training_sample> data(200);
    auto f = [&data, &listing](time_t seed)
    {
        dlib::rand rnd(time(0)+seed);
        matrix<rgb_pixel> input_image;
        //matrix<rgb_pixel> rgb_label_image;
        matrix<uint16_t> index_label_image;
        training_sample temp;
        while(data.is_enabled())
        {
            // Pick a random input image.
            const image_info& image_info = listing[rnd.get_random_32bit_number()%listing.size()];

            // Load the input image.
            load_image(input_image, image_info.image_filename);

            //dlib::matrix<rgb_pixel> resized_input_image;
            //resized_input_image.set_size(input_tile_height, input_tile_width);
            //dlib::resize_image(input_image, resized_input_image, dlib::interpolate_nearest_neighbor());
            //input_image = resized_input_image;
            //input_image.set_size(input_tile_height, input_tile_width);

            // Load the ground-truth (RGB) labels.
            //load_image(rgb_label_image, image_info.label_filename);
            load_image(index_label_image, image_info.label_filename);

            //dlib::matrix<uint16_t> resized_index_label_image;
            //resized_index_label_image.set_size(input_tile_height, input_tile_width);
            //dlib::resize_image(index_label_image, resized_index_label_image, dlib::interpolate_nearest_neighbor());
            //index_label_image = resized_index_label_image;
            //index_label_image.set_size(input_tile_height, input_tile_width);

            // Convert the indexes to RGB values.
            //neuralnet::rgb_label_image_to_index_label_image(rgb_label_image, index_label_image);

            // Randomly pick a part of the image.
            neuralnet::randomly_crop_image(input_image, index_label_image, temp, rnd);

            // no crop -> overfit
            //temp.input_image = resized_input_image;
            //temp.label_image = resized_index_label_image;

            // Push the result to be used by the trainer.
            data.enqueue(temp);
        }
    };
    std::thread data_loader1([f](){ f(1); });
    std::thread data_loader2([f](){ f(2); });
    //std::thread data_loader3([f](){ f(3); });
    //std::thread data_loader4([f](){ f(4); });
    unsigned int epoch = 0;
    // The main training loop.  Keep making mini-batches and giving them to the trainer.
    // We will run until the learning rate has dropped by a factor of 1e-4.
    while(trainer.get_learning_rate() >= 1e-4 &&
          epoch++ < epochs)
    {
        Logger::getInstance().log("epoch #" + std::to_string(epoch));
        samples.clear();
        labels.clear();

        // make a 30-image mini-batch
        training_sample temp;
        while(samples.size() < minibatch_size)
        {
            data.dequeue(temp);

            samples.push_back(std::move(temp.input_image));
            labels.push_back(std::move(temp.label_image));
        }

        trainer.train_one_step(samples, labels);
    }

    // Training done, tell threads to stop and make sure to wait for them to finish before
    // moving on.
    data.disable();
    data_loader1.join();
    data_loader2.join();
    //data_loader3.join();
    //data_loader4.join();

    // also wait for threaded processing to stop in the trainer.
    trainer.get_net();

    net.clean();

    Logger::getInstance().log("saving network");
    serialize("semantic_segmentation.dnn") << net;


    // Make a copy of the network to use it for inference.
    anet_type anet = net;

    Logger::getInstance().log( "Testing the network...");
    // Find the accuracy of the newly trained network on both the training and the validation sets.
    double accuracy = calculate_accuracy(anet, get_listing(image_label_tuples));

    Logger::getInstance().log( "train accuracy  :  " + std::to_string(accuracy));
    return accuracy;
}

cv::Mat neuralnet::predict(std::string imagefile)
{
    matrix<rgb_pixel> input_image;
    load_image(input_image, imagefile);

    return predict(input_image);
}

cv::Mat neuralnet::predict(cv::Mat image)
{
    matrix<rgb_pixel> input_image;
    dlib::assign_image(input_image, cv_image<rgb_pixel>(image));
    return predict(input_image);
}

cv::Mat neuralnet::predict(dlib::matrix<rgb_pixel> input_image)
{
    int rows = input_image.nr();
    int cols = input_image.nc();
    // Read the file containing the trained network from the working directory.
    if(!netLoaded){

        deserialize("semantic_segmentation.dnn") >> net;
        netLoaded = true;
    }

    dlib::matrix<rgb_pixel> resized_input_image;
    resized_input_image.set_size(2*input_tile_height, 2*input_tile_width);
    dlib::resize_image(input_image, resized_input_image, dlib::interpolate_bilinear());
    input_image = resized_input_image;

    matrix<uint16_t> index_label_image;
    matrix<rgb_pixel> rgb_label_image;

    // Create predictions for each pixel. At this point, the type of each prediction
    // is an index (a value between 0 and 20). Note that the net may return an image
    // that is not exactly the same size as the input.
    const matrix<uint16_t> temp = net(input_image);

    // Crop the returned image to be exactly the same size as the input.
    const chip_details chip_details(
                centered_rect(temp.nc() / 2, temp.nr() / 2, input_image.nc(), input_image.nr()),
                chip_dims(input_image.nr(), input_image.nc())
                );
    extract_image_chip(temp, chip_details, index_label_image, interpolate_nearest_neighbor());

    //dlib::matrix<uint16_t> resized_index_label_image;
    //resized_index_label_image.set_size(rows, cols);
    //dlib::resize_image(index_label_image, resized_index_label_image, dlib::interpolate_nearest_neighbor());
    //index_label_image = resized_index_label_image;

    // Convert the indexes to RGB values.
   //neuralnet::index_label_image_to_rgb_label_image(index_label_image, rgb_label_image);

    // Show the input image on the left, and the predicted RGB labels on the right.
    //image_window win;
    //win.set_image(join_rows(input_image, rgb_label_image));

    // Find the most prominent class label from amongst the per-pixel predictions.
    //const std::string classlabel = get_most_prominent_non_background_classlabel(index_label_image);
    //std::cout << classlabel << std::endl;
    return dlib::toMat(index_label_image);
}

dlib::rectangle neuralnet::make_random_cropping_rect_resnet(const dlib::matrix<dlib::rgb_pixel> &img, dlib::rand &rnd)
{
    // figure out what rectangle we want to crop from the image
    double mins = 0.466666666, maxs = 0.875;
    auto scale = mins + rnd.get_random_double()*(maxs-mins);
    auto size = scale*std::min(img.nr(), img.nc());
    rectangle rect(size, size);
    // randomly shift the box around
    point offset(rnd.get_random_32bit_number()%(img.nc()-rect.width()),
                 rnd.get_random_32bit_number()%(img.nr()-rect.height()));
    return move_rect(rect, offset);
}

void neuralnet::randomly_crop_image(const dlib::matrix<rgb_pixel> &input_image, const dlib::matrix<uint16_t> &label_image, neuralnet::training_sample &crop, dlib::rand &rnd)
{
    const auto rect = make_random_cropping_rect_resnet(input_image, rnd);

    const chip_details chip_details(rect, chip_dims(neuralnet::input_tile_height, neuralnet::input_tile_width));

    // Crop the input image.
    extract_image_chip(input_image, chip_details, crop.input_image, interpolate_bilinear());

    // Crop the labels correspondingly. However, note that here bilinear
    // interpolation would make absolutely no sense - you wouldn't say that
    // a bicycle is half-way between an aeroplane and a bird, would you?
    extract_image_chip(label_image, chip_details, crop.label_image, interpolate_nearest_neighbor());

    // Also randomly flip the input image and the labels.
    if (rnd.get_random_double() > 0.5)
    {
        crop.input_image = fliplr(crop.input_image);
        crop.label_image = fliplr(crop.label_image);
    }

    // And then randomly adjust the colors.
    apply_random_color_offset(crop.input_image, rnd);
}

std::vector<neuralnet::image_info> neuralnet::get_listing(std::vector<std::tuple<string, string> > image_label_tuples)
{
    std::vector<image_info> results;
    for(auto tup: image_label_tuples){
        image_info image_info;
        image_info.image_filename = std::get<0>(tup);
        image_info.label_filename = std::get<1>(tup);
        results.push_back(image_info);
    }
    return results;
}

const Voc2012class &neuralnet::find_index_class(const uint16_t &index_label)
{
    return find_voc2012_class(
                [&index_label](const Voc2012class& voc2012class)
    {
        return index_label == voc2012class.index;
    }
    );
}

const Voc2012class &neuralnet::find_label_class(const dlib::rgb_pixel &rgb_label)
{
    return find_voc2012_class(
                [&rgb_label](const Voc2012class& voc2012class)
    {
        return rgb_label == voc2012class.rgb_label;
    }
    );

}

void neuralnet::rgb_label_image_to_index_label_image(const dlib::matrix<rgb_pixel> &rgb_label_image, dlib::matrix<uint16_t> &index_label_image)
{
    const long nr = rgb_label_image.nr();
    const long nc = rgb_label_image.nc();

    index_label_image.set_size(nr, nc);

    for (long r = 0; r < nr; ++r)
    {
        for (long c = 0; c < nc; ++c)
        {
            index_label_image(r, c) = neuralnet::rgb_label_to_index_label(rgb_label_image(r, c));
        }
    }
}

void neuralnet::index_label_image_to_rgb_label_image(const dlib::matrix<uint16_t> &index_label_image, dlib::matrix<rgb_pixel> &rgb_label_image)
{
    const long nr = index_label_image.nr();
    const long nc = index_label_image.nc();

    rgb_label_image.set_size(nr, nc);

    for (long r = 0; r < nr; ++r)
    {
        for (long c = 0; c < nc; ++c)
        {
            rgb_label_image(r, c) = index_label_to_rgb_label(index_label_image(r, c));
        }
    }
}

double neuralnet::calculate_accuracy(anet_type &anet, const std::vector<neuralnet::image_info> &dataset)
{

    int num_right = 0;
    int num_wrong = 0;

    matrix<rgb_pixel> input_image;
    matrix<rgb_pixel> rgb_label_image;
    matrix<uint16_t> index_label_image;
    matrix<uint16_t> net_output;

    for (const auto& image_info : dataset)
    {
        // Load the input image.
        load_image(input_image, image_info.image_filename);

        dlib::matrix<rgb_pixel> resized_input_image;
        resized_input_image.set_size(input_tile_height, input_tile_width);
        dlib::resize_image(input_image, resized_input_image, dlib::interpolate_nearest_neighbor());
        input_image = resized_input_image;
        //input_image.set_size(input_tile_height, input_tile_width);

        // Load the ground-truth (RGB) labels.
        //load_image(rgb_label_image, image_info.label_filename);
        load_image(index_label_image, image_info.label_filename);

        dlib::matrix<uint16_t> resized_index_label_image;
        resized_index_label_image.set_size(input_tile_height, input_tile_width);
        dlib::resize_image(index_label_image, resized_index_label_image, dlib::interpolate_nearest_neighbor());
        index_label_image = resized_index_label_image;
        //rgb_label_image.set_size(input_tile_height, input_tile_width);

        // Create predictions for each pixel. At this point, the type of each prediction
        // is an index (a value between 0 and 20). Note that the net may return an image
        // that is not exactly the same size as the input.
        const matrix<uint16_t> temp = anet(input_image);

        // Convert the indexes to RGB values.
        //rgb_label_image_to_index_label_image(rgb_label_image, index_label_image);

        // Crop the net output to be exactly the same size as the input.
        const chip_details chip_details(
                    centered_rect(temp.nc() / 2, temp.nr() / 2, input_image.nc(), input_image.nr()),
                    chip_dims(input_image.nr(), input_image.nc())
                    );
        extract_image_chip(temp, chip_details, net_output, interpolate_nearest_neighbor());

        const long nr = index_label_image.nr();
        const long nc = index_label_image.nc();

        // Compare the predicted values to the ground-truth values.
        for (long r = 0; r < nr; ++r)
        {
            for (long c = 0; c < nc; ++c)
            {
                const uint16_t truth = index_label_image(r, c);
                if (truth != dlib::loss_multiclass_log_per_pixel_::label_to_ignore)
                {
                    const uint16_t prediction = net_output(r, c);
                    if (prediction == truth)
                    {
                        ++num_right;
                    }
                    else
                    {
                        ++num_wrong;
                    }
                }
            }
        }
    }

    // Return the accuracy estimate.
    return num_right / static_cast<double>(num_right + num_wrong);
}

string neuralnet::get_most_prominent_non_background_classlabel(const dlib::matrix<uint16_t> &index_label_image)
{
    const long nr = index_label_image.nr();
    const long nc = index_label_image.nc();

    std::vector<unsigned int> counters(class_count);

    for (long r = 0; r < nr; ++r)
    {
        for (long c = 0; c < nc; ++c)
        {
            const uint16_t label = index_label_image(r, c);
            ++counters[label];
        }
    }

    const auto max_element = std::max_element(counters.begin() + 1, counters.end());
    const uint16_t most_prominent_index_label = max_element - counters.begin();

    return neuralnet::find_index_class(most_prominent_index_label).classlabel;
}

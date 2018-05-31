#include <superpixellib/imageeditor.h>
#include "ui_imageeditor.h"
#include <QMouseEvent>
#include <QDebug>
#include <memory>
#include <opencv2/ximgproc.hpp>
#include <opencv2/ximgproc/seeds.hpp>


ImageEditor::ImageEditor(std::string imagefile, QWidget *parent)
    : QWidget(parent), ui(new Ui::ImageEditor) {
    ui->setupUi(this);
    this->image = cv::imread(imagefile);
    cv::cvtColor(this->image, this->image, cv::COLOR_BGR2RGB);
    this->superpixel = this->image;
    this->segmentation =
            cv::Mat::zeros(this->image.rows, this->image.cols, CV_16U);
    this->vis_seg = cv::Mat::zeros(this->image.rows, this->image.cols, CV_8UC3);
    on_num_superpixels_valueChanged(0);
    reloadImage();
    ui->imageLabel->installEventFilter(this);
    ui->imageLabel->setMouseTracking(true);
}

ImageEditor::ImageEditor(std::string imagefile, std::string labelfile, bool saveOnExit, QWidget *parent)
    : QWidget(parent), ui(new Ui::ImageEditor) {
    ui->setupUi(this);
    this->saveOnExit = saveOnExit;
    this->labelfile = labelfile;
    this->image = cv::imread(imagefile);

    this->labels = cv::imread(labelfile);
    cv::cvtColor(this->image, this->image, cv::COLOR_BGR2RGB);
    this->labels.convertTo(this->labels, CV_16U);
    this->superpixel = this->image;
    this->segmentation =
            cv::Mat::zeros(this->image.rows, this->image.cols, CV_16U);
    this->vis_seg = cv::Mat::zeros(this->image.rows, this->image.cols, CV_8UC3);

    colorize(labels, vis_seg);
    on_num_superpixels_valueChanged(0);

    reloadImage();
    ui->imageLabel->installEventFilter(this);
    ui->imageLabel->setMouseTracking(true);

}

ImageEditor::~ImageEditor() {
    if(saveOnExit){
        this->labels.convertTo(this->labels, CV_16U);
        //cv::imwrite(this->labelfile, this->labels); // real image is smaller bug
        QPixmap png = fromMat_flat(labels);
        png.save(QString::fromStdString(labelfile), "PNG");
    }
    delete ui; }

QColor ImageEditor::idToColor(uint8_t id) const {
    static double r_ = (1 + sqrt(5)) / 2;
    static double g_ = (3 + sqrt(7)) / 2;
    static double b_ = (11 + sqrt(13)) / 2;

    double r = id * r_ - floor(id * r_);
    double g = id * g_ - floor(id * g_);
    double b = id * b_ - floor(id * b_);
    return QColor(r * 255, g * 255, b * 255);
}

void ImageEditor::colorize(cv::Mat &src, cv::Mat &dst) {
    std::vector<cv::Mat> rgb;
    cv::split(dst, rgb);
    for (int x = 0; x < src.cols; ++x) {
        for (int y = 0; y < src.rows; ++y) {
            QColor color = idToColor(src.at<uint8_t>(y, x, 0));
            rgb[0].at<uchar>(y, x) = color.red();
            rgb[1].at<uchar>(y, x) = color.green();
            rgb[2].at<uchar>(y, x) = color.blue();
        }
    }
    cv::merge(rgb, dst);
}

/*
 * source:
 * http://jepsonsblog.blogspot.de/2012/10/overlay-transparent-image-in-opencv.html
 */
void overlayImage(const cv::Mat &background, const cv::Mat &foreground,
                  cv::Mat &output, cv::Point2i location) {
    background.copyTo(output);

    // start at the row indicated by location, or at row 0 if location.y is
    // negative.
    for (int y = std::max(location.y, 0); y < background.rows; ++y) {
        int fY = y - location.y; // because of the translation

        // we are done of we have processed all rows of the foreground image.
        if (fY >= foreground.rows)
            break;

        // start at the column indicated by location,

        // or at column 0 if location.x is negative.
        for (int x = std::max(location.x, 0); x < background.cols; ++x) {
            int fX = x - location.x; // because of the translation.

            // we are done with this row if the column is outside of the foreground
            // image.
            if (fX >= foreground.cols)
                break;

            // determine the opacity of the foregrond pixel, using its fourth (alpha)
            // channel.
            double opacity =
                    ((double)foreground
                     .data[fY * foreground.step + fX * foreground.channels() + 3])

                    / 255.;

            // and now combine the background and foreground pixel, using the opacity,

            // but only if opacity > 0.
            for (int c = 0; opacity > 0 && c < output.channels(); ++c) {
                unsigned char foregroundPx =
                        foreground
                        .data[fY * foreground.step + fX * foreground.channels() + c];
                unsigned char backgroundPx =
                        background
                        .data[y * background.step + x * background.channels() + c];
                output.data[y * output.step + output.channels() * x + c] =
                        backgroundPx * (1. - opacity) + foregroundPx * opacity;
            }
        }
    }
}

void ImageEditor::reloadImage() {
    cv::Mat combined;
    cv::Mat segmentation2 = superpixel.clone();
    overlayImage(superpixel, vis_seg, combined, cv::Point2i(0, 0));
    ui->imageLabel->setPixmap(fromMat(combined));
}

bool ImageEditor::eventFilter(QObject *watched, QEvent *event) {
    if (watched != ui->imageLabel)
        return false;
    if (event->type() == QEvent::MouseButtonRelease)
        mousePressed = false;
    if (event->type() != QEvent::MouseMove &&
            event->type() != QEvent::MouseButtonPress)
        return false;
    if (event->type() == QEvent::MouseButtonPress)
        mousePressed = true;

    if (!mousePressed)
        return false;
    const QMouseEvent *const me = static_cast<const QMouseEvent *>(event);
    const QPoint p = me->pos();
    paintSuperPixel(p);
    return false;
}

void ImageEditor::paintSuperPixel(QPoint p) {
    uint8_t CLASS = this->segment.at<uint8_t>(p.y(), p.x(), 0);
    this->mask = this->segment.clone();
    std::vector<cv::Mat> rgb;
    cv::split(vis_seg, rgb);
    QColor color = idToColor(ui->classSpinBox->value());
    for (int x = 0; x < this->mask.cols; ++x) {
        for (int y = 0; y < this->mask.rows; ++y) {
            if (this->mask.at<uint8_t>(y, x, 0) == CLASS) {
                rgb[0].at<uchar>(y, x) = color.red();
                rgb[1].at<uchar>(y, x) = color.green();
                rgb[2].at<uchar>(y, x) = color.blue();
                this->labels.at<uint8_t>(y,x) = ui->classSpinBox->value();
            }
        }
    }
    cv::merge(rgb, vis_seg);
    reloadImage();
}

cv::Mat ImageEditor::getSegmentation() { return this->segmentation; }

void ImageEditor::on_num_superpixels_valueChanged(int value) {
    SuperPixel_Seg(this->image, this->segment);
    reloadImage();
}

// source:
// https://github.com/daijialun/Saliency/blob/e8b1c951c1d809012f66f9a512aa0ddfc6ea40ee/SuperPixel_Seg.cpp
int ImageEditor::SuperPixel_Seg(cv::Mat src, cv::Mat &matLabels) {
    int height = src.rows, width = src.cols;
    int num_superpixels = ui->num_superpixels->value();
    int num_level = 1;
    int num_iterations = 5;
    cv::Mat mask = src.clone(), result = src.clone();
    cv::Ptr<cv::ximgproc::SuperpixelSEEDS> seeds;
    seeds = cv::ximgproc::createSuperpixelSEEDS(width, height, src.channels(),
                                                num_superpixels, num_level);
    cv::Mat converted;
    cv::cvtColor(src, converted, cv::COLOR_RGB2HSV);
    seeds->iterate(src, num_iterations);
    int superpixels_num = 0;
    superpixels_num = seeds->getNumberOfSuperpixels();
    seeds->getLabels(matLabels);
    seeds->getLabelContourMask(mask, false);
    result.setTo(cv::Scalar(0, 0, 255), mask);
    this->mask = mask;
    superpixel = result;
    return superpixels_num;
}

QPixmap ImageEditor::fromMat(cv::Mat &img) {
    QImage image = QImage((uchar *)img.data, img.cols, img.rows, img.step,
                          QImage::Format_RGB888);
    QPixmap pixel = QPixmap::fromImage(image);
    return pixel;
}

QPixmap ImageEditor::fromMat_flat(cv::Mat &img) {
    QImage image = QImage((uchar *)img.data, img.cols, img.rows, img.step,
                          QImage::Format_Grayscale8);
    QPixmap pixel = QPixmap::fromImage(image);
    return pixel;
}

void ImageEditor::on_saveButton_clicked()
{
    this->labels.convertTo(this->labels, CV_16U);
    //cv::imwrite(this->labelfile, this->labels); // real image is smaller bug
    QPixmap png = fromMat_flat(labels);
    png.save(QString::fromStdString(labelfile), "PNG");
    qDebug() << QString::fromStdString(this->labelfile) << " saved" << labels.cols << "x" <<labels.rows << "x" << labels.type();
}

#include "videostream/videostream.h"
#include "ui_videostream.h"

#include <QDebug>
#include <QTime>
#include <opencv2/highgui.hpp>


VideoStream::VideoStream(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoStream)
{
    ui->setupUi(this);
    connect(&timer, SIGNAL(timeout()), this, SLOT(on_read_stream()));
}

VideoStream::~VideoStream()
{
    delete ui;
}

void VideoStream::on_connectButton_clicked()
{

    this->videoCapture = std::make_shared<cv::VideoCapture>(ui->sourceLineEdit->text().toStdString());

    if (!this->videoCapture->isOpened()) {
        qDebug() << "error";
    }
    on_read_stream();
}

void VideoStream::on_read_stream()
{
    if (this->videoCapture->isOpened()) {

        this->videoCapture->read(frame);
        QImage * img = new QImage((uchar*)frame.data, frame.cols, frame.rows, QImage::Format_RGB888);
        this->pixmap = std::make_shared<QPixmap>(QPixmap::fromImage(*img));

        ui->imageLabel->setPixmap(*pixmap);
        timer.start(1000/ui->fpsSpinBox->value());
    }
}

void VideoStream::on_stopButton_clicked()
{
    timer.stop();
    ui->imageLabel->setPixmap(QPixmap());
}

void VideoStream::on_saveButton_clicked()
{
    cv::Mat label = cv::Mat::zeros(frame.size(), CV_8U);
    std::string fname =  QDateTime::currentDateTime().toString("yyyyMMdd_hhmmsszz").toStdString();
    std::string filename = "images/" + fname + ".jpg";
    std::string labelfilename = "labels/" + fname + ".png";
    cv::imwrite( filename, frame );
    cv::imwrite( labelfilename, label );
}

void VideoStream::on_pushButton_clicked()
{
    ui->sourceLineEdit->setText("rtsp://service:Xbks8tr8vT@193.159.244.134");
}

#include "videostream/videostream.h"
#include "ui_videostream.h"

#include <QDebug>
#include <QTime>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

VideoStream::VideoStream(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoStream)
{
    ui->setupUi(this);
    connect(&timer, SIGNAL(timeout()), this, SLOT(on_read_stream()));
    this->alarm = std::make_shared<Alarm>();
    this->neuralNet = std::make_shared<neuralnet>();
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
        ui->imageLabel->setPixmap(QPixmap::fromImage(QImage((unsigned char*) frame.data, frame.cols, frame.rows, QImage::Format_RGB888)));
        if(ui->aiCheckBox->isChecked()){
            frame = neuralNet->predict(frame);
            cv::Mat rgb;
            cvtColor(frame, rgb, CV_GRAY2RGB);
            ui->predictLabel->setPixmap(QPixmap::fromImage(QImage((unsigned char*) rgb.data, rgb.cols, rgb.rows, QImage::Format_RGB888)));
            if(ui->alarmCheckBox->isChecked()){
                alarm->handleAlarm(frame);
            }
        }else{
            ui->predictLabel->clear();
        }


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

void VideoStream::on_pushButton_2_clicked()
{
    ui->sourceLineEdit->setText("rtsp://admin:a1b2c3d4@60.191.94.122");
}

void VideoStream::on_pushButton_3_clicked()
{
    ui->sourceLineEdit->setText("rtsp://admin:Uniview2018@61.164.52.166:88:555");
}

void VideoStream::on_pushButton_4_clicked()
{
    ui->sourceLineEdit->setText("rtsp://admin:abcd1234@123.157.208.2:555");
}

void VideoStream::on_pushButton_5_clicked()
{
    ui->sourceLineEdit->setText("/dev/video0");
}

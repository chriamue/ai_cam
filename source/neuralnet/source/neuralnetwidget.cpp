#include "neuralnet/neuralnetwidget.h"
#include "ui_neuralnetwidget.h"

#include "neuralnet/neuralnet.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QList>
#include <QString>
#include <QFileDialog>
#include <QDebug>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

NeuralNetWidget::NeuralNetWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NeuralNetWidget)
{
    ui->setupUi(this);
}

NeuralNetWidget::~NeuralNetWidget()
{
    delete ui;
}

void NeuralNetWidget::on_trainButton_clicked()
{
    ui->progressBar->setValue(20);
    QList<std::string> imagesNames;
    std::vector<std::tuple<std::string,std::string>> image_label_tuples;

    QString dir("images/");
    QDirIterator it(dir, QStringList() << "*.jpg", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()){
        QFile file(it.next());
        QFileInfo fileInfo(file.fileName());
        if(QFile::exists("images/" + fileInfo.baseName() + ".jpg")){
            if(QFile::exists("labels/" + fileInfo.baseName() + ".png")){
                imagesNames.append(fileInfo.baseName().toStdString());
            }
        }
    }
    ui->progressBar->setValue(30);
    for(std::string filename: imagesNames){
        QString jpgFile = QString::fromStdString("images/"+filename+".jpg");
        QString pngFile = QString::fromStdString("labels/"+filename+".png");
        std::tuple<std::string,std::string> tup (jpgFile.toStdString(), pngFile.toStdString());
        image_label_tuples.push_back(tup);
    }
    ui->progressBar->setValue(50);
    neuralnet n;
    double accuracy = n.train(image_label_tuples, ui->epochSpinBox->value());
    ui->progressBar->setValue(100);
    ui->progressBar->setFormat(QString::number(accuracy));
}

void NeuralNetWidget::on_predictButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp)"));
    cv::Mat image = cv::imread(fileName.toStdString());
    ui->imageLabel->setPixmap(QPixmap::fromImage(QImage((unsigned char*) image.data, image.cols, image.rows, QImage::Format_RGB888)));
    neuralnet n;
    image = n.predict(image);
    cv::Mat rgb;
    cvtColor(image, rgb, CV_GRAY2RGB);
    ui->predictLabel->setPixmap(QPixmap::fromImage(QImage((unsigned char*) rgb.data, rgb.cols, rgb.rows, QImage::Format_RGB888)));


}

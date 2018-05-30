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
    for(std::string filename: imagesNames){
        QString jpgFile = QString::fromStdString("images/"+filename+".jpg");
        QString pngFile = QString::fromStdString("labels/"+filename+".png");
        std::tuple<std::string,std::string> tup (jpgFile.toStdString(), pngFile.toStdString());
        image_label_tuples.push_back(tup);
    }

    neuralnet n;
    n.train(image_label_tuples);
}

void NeuralNetWidget::on_predictButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp)"));
    cv::Mat image = cv::imread(fileName.toStdString());
    ui->imageLabel->setPixmap(QPixmap::fromImage(QImage((unsigned char*) image.data, image.cols, image.rows, QImage::Format_RGB888)));

}

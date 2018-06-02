#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include <logger/logger.h>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->videoStream = std::make_shared<VideoStream>(this);
    ui->videoStreamLayout->addWidget(this->videoStream.get());

    this->alarm = std::make_shared<AlarmWidget>(this);
    ui->alarmLayout->addWidget(this->alarm.get());

    this->neuralNet = std::make_shared<NeuralNetWidget>(this);
    ui->neuralNetLayout->addWidget(this->neuralNet.get());
    initFolders();
    initImages();
    ui->loggerGridLayout->addWidget(&Logger::getInstance());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    ImageEditor edit(std::string(""));
    edit.show();
}

void MainWindow::initFolders()
{
    if(!QDir("images/").exists()){
        QDir().mkdir("images");
    }
    if(!QDir("labels/").exists()){
        QDir().mkdir("labels");
    }
}

void MainWindow::initImages()
{
    this->imagesNames.clear();
    ui->imagesListWidget->clear();
    ui->labelsListWidget->clear();
    QString dir("images/");
    QDirIterator it(dir, QStringList() << "*.jpg", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()){
        QFile file(it.next());
        QFileInfo fileInfo(file.fileName());
        if(QFile::exists("images/" + fileInfo.baseName() + ".jpg")){
            if(QFile::exists("labels/" + fileInfo.baseName() + ".png")){
                this->imagesNames.append(fileInfo.baseName().toStdString());
            }
        }
    }
    for(std::string filename: this->imagesNames){
        QString jpgFile = QString::fromStdString("images/"+filename+".jpg");
        QString pngFile = QString::fromStdString("labels/"+filename+".png");

        ui->imagesListWidget->addItem(new QListWidgetItem(QIcon(jpgFile), jpgFile));
        ui->labelsListWidget->addItem(new QListWidgetItem(QIcon(pngFile), pngFile));
    }
}


void MainWindow::on_imageEditButton_clicked()
{
    QString imagefile = ui->imagesListWidget->currentItem()->text();
    QString labelfile = ui->labelsListWidget->item(ui->imagesListWidget->currentRow())->text();
    qDebug() << imagefile;
    imageEditor = std::make_shared<ImageEditor>(imagefile.toStdString(), labelfile.toStdString(), true, this);
    ui->scrollArea->setWidget(imageEditor.get());
}

void MainWindow::on_reloadButton_clicked()
{
initImages();
}

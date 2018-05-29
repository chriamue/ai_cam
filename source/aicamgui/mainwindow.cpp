#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QDirIterator>
#include <QDebug>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->videoStream = std::make_shared<VideoStream>(this);
    ui->videoStreamLayout->addWidget(this->videoStream.get());
    initFolders();
    initImages();
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
    qDebug() << imagefile;
    imageEditor = std::make_shared<ImageEditor>(imagefile.toStdString(), this);
    ui->scrollArea->setWidget(imageEditor.get());
}

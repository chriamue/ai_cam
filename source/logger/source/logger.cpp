#include "logger/logger.h"
#include "ui_logger.h"

#include <iostream>

Logger::Logger(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Logger)
{
    ui->setupUi(this);
}

Logger::~Logger()
{
    delete ui;
}

void Logger::log(std::string logMessage)
{
    ui->plainTextEdit->appendPlainText(QString::fromStdString(logMessage));
    std::cout << logMessage << std::endl;
}

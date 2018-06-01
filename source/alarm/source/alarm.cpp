#include "alarm/alarm.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <functional>
#include <iostream>

Alarm::Alarm()
{
    loadAlarmFile();
}

void Alarm::handleAlarm(cv::Mat labelImage)
{
    int pixels = labelImage.rows * labelImage.cols;
    for(auto rule: rules){
        int c = std::get<0>(rule);
        double d = std::get<1>(rule);
        int i = cv::countNonZero(labelImage == c);
        if((double)i / (double)pixels > d){
            std::cout << "alarm: " << d << " @ " << (double)i / (double)pixels << std::endl;
        }
    }
}

void Alarm::loadAlarmFile()
{
    QFile jsonFile("alarm.json");
    if(jsonFile.exists()){

        jsonFile.open(QFile::ReadOnly);
        QJsonDocument doc = QJsonDocument().fromJson(jsonFile.readAll());
        QJsonObject root = doc.object();
        QJsonArray rules = root["rules"].toArray();
        for(QJsonValue val: rules){
            QJsonObject obj = val.toObject();
            int c = obj.find("class").value().toInt();
            double d = obj.find("thresh").value().toDouble(0.1);
            auto t = std::make_tuple(c,d);
            this->rules.push_back(t);
        }
    }
}

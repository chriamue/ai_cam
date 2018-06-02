#include "alarm/alarm.h"

#include <functional>
#include <iostream>

#include <logger/logger.h>

#include <Poco/Net/MailMessage.h>
#include <Poco/Net/MailRecipient.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/SMTPClientSession.h>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

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
        bool l = std::get<2>(rule);
        cv::Mat cImg = labelImage == c;
        int i = cv::countNonZero(cImg);

        if(l){
            if(((double)i / (double)pixels) < d && (i > 0)){
                std::string message = "alarm (-): class " + std::to_string(c) + " @ " + std::to_string( (double)i / (double)pixels) + " / " + std::to_string(d);
                Logger::getInstance().log(message);
            }
        }else{
            if(((double)i / (double)pixels) > d && (i > 0)){
                std::string message = "alarm (+): class " + std::to_string(c) + " @ " + std::to_string( (double)i / (double)pixels) + " / " + std::to_string(d);
                Logger::getInstance().log(message);
            }
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
            bool l = obj.find("less").value().toBool(true);
            auto t = std::make_tuple(c,d,l);
            this->rules.push_back(t);
        }
        QJsonObject email = root["email"].toObject();
        from = email.find("from").value().toString().toStdString();
        to = email.find("to").value().toString().toStdString();
        mailserver = email.find("server").value().toString().toStdString();
        port = email.find("port").value().toInt();
        login = email.find("login").value().toString().toStdString();
        password = email.find("password").value().toString().toStdString();
        useMail = email.find("enabled").value().toBool();
    }
}

void Alarm::sendMail(std::string message)
{
    if(!useMail)
        return;
    Poco::Net::MailMessage mail;
    mail.setSender(from);
    mail.addRecipient(Poco::Net::MailRecipient(
                          Poco::Net::MailRecipient::PRIMARY_RECIPIENT, to));
    mail.setSubject("Alarm");
    mail.setContentType("text/plain; charset=UTF-8");
    mail.setContent(message, Poco::Net::MailMessage::ENCODING_8BIT);
    try {
        Poco::Net::SMTPClientSession session(mailserver, Poco::UInt16(port));
        session.open();
        try {
            session.login(Poco::Net::SMTPClientSession::AUTH_NONE, login, password);
            session.sendMessage(mail);
            session.close();
        } catch (Poco::Net::SMTPException &e) {
            std::cout << e.displayText() << std::endl;
            session.close();
            return;
        }
    } catch (Poco::Net::NetException &e) {
        std::cout << e.displayText() << std::endl;
        return;
    }
}

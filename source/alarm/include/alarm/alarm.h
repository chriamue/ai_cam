#ifndef ALARM_H
#define ALARM_H


#include <alarm/alarm_api.h>

#include <vector>
#include <tuple>

#include <opencv2/core.hpp>

class ALARM_API Alarm
{
public:
    Alarm();
    void handleAlarm(cv::Mat labelImage);

private:
    std::string from;
    std::string to;
    std::string mailserver;
    int port;
    std::string login;
    std::string password;
    bool useMail = false;
    void loadAlarmFile();
    std::vector<std::tuple<int,double>> rules;
    void sendMail(std::string message);
};

#endif // ALARM_H

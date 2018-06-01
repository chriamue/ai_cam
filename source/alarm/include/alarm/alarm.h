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
    void loadAlarmFile();
    std::vector<std::tuple<int,double>> rules;
};

#endif // ALARM_H

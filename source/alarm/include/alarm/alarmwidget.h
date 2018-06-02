#ifndef ALARMWIDGET_H
#define ALARMWIDGET_H

#include <alarm/alarm_api.h>
#include <QWidget>

#include <opencv2/core.hpp>

namespace Ui {
class AlarmWidget;
}

class ALARM_API AlarmWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AlarmWidget(QWidget *parent = 0);
    ~AlarmWidget();

    void doAlarm(cv::Mat labelsImage);


private slots:
    void on_addButton_clicked();

    void on_saveButton_clicked();

    void on_removeButton_clicked();

private:
    Ui::AlarmWidget *ui;
    void loadAlarmFile();
};

#endif // ALARMWIDGET_H

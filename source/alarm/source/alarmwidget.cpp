#include "alarm/alarmwidget.h"
#include "alarm/ruleitem.h"
#include "ui_alarmwidget.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

AlarmWidget::AlarmWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AlarmWidget)
{
    ui->setupUi(this);
    loadAlarmFile();
}

AlarmWidget::~AlarmWidget()
{
    delete ui;
}

void AlarmWidget::doAlarm(cv::Mat labelsImage)
{

}

void AlarmWidget::loadAlarmFile()
{
    QFile jsonFile("alarm.json");
    if(jsonFile.exists()){

        jsonFile.open(QFile::ReadOnly);
        QJsonDocument doc = QJsonDocument().fromJson(jsonFile.readAll());
        QJsonObject root = doc.object();
        QJsonArray rules = root["rules"].toArray();
        for(QJsonValue val: rules){
            QJsonObject obj = val.toObject();
            QListWidgetItem *item = new QListWidgetItem(ui->listWidget);

            RuleItem *ruleItem = new RuleItem();
            ruleItem->fromJson(obj);
            item->setSizeHint(ruleItem->minimumSizeHint());
            ui->listWidget->setItemWidget(item, ruleItem);
        }
    }
}

void AlarmWidget::on_addButton_clicked()
{
    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);

    RuleItem *ruleItem = new RuleItem();
    item->setSizeHint(ruleItem->minimumSizeHint());
    ui->listWidget->setItemWidget(item, ruleItem);
}

void AlarmWidget::on_saveButton_clicked()
{
    QJsonDocument doc;
    QJsonObject root;
    QJsonArray rules;
    for(int i = 0; i< ui->listWidget->count(); ++i){
        RuleItem * item = static_cast<RuleItem*>(ui->listWidget->itemWidget(ui->listWidget->item(i)));
        rules.append(item->toJson());
    }
    root["rules"] = rules;
    doc.setObject(root);
    QFile jsonFile("alarm.json");
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(doc.toJson());
}

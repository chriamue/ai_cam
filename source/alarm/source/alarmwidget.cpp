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
        QJsonObject email = root["email"].toObject();
        ui->fromLineEdit->setText(email.find("from").value().toString());
        ui->recipientLineEdit->setText(email.find("to").value().toString());
        ui->mailserverLineEdit->setText(email.find("server").value().toString());
        ui->portSpinBox->setValue(email.find("port").value().toInt());
        ui->loginLineEdit->setText(email.find("login").value().toString());
        ui->passwordLineEdit->setText(email.find("password").value().toString());
        ui->sendCheckBox->setChecked(email.find("enabled").value().toBool());
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
    root["email"] = QJsonObject{
            {"enabled", ui->sendCheckBox->isChecked() },
        {"from", ui->fromLineEdit->text() },
        {"to", ui->recipientLineEdit->text()},
        {"server", ui->mailserverLineEdit->text()},
        {"port", ui->portSpinBox->value()},
        {"login", ui->loginLineEdit->text()},
        {"password", ui->passwordLineEdit->text()}
    };
    doc.setObject(root);
    QFile jsonFile("alarm.json");
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(doc.toJson());
}

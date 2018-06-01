#include "alarm/ruleitem.h"
#include "ui_ruleitem.h"

RuleItem::RuleItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RuleItem)
{
    ui->setupUi(this);
}

RuleItem::~RuleItem()
{
    delete ui;
}

QJsonObject RuleItem::toJson()
{
    QJsonObject obj{
        {"class", ui->classSpinBox->value()},
        {"thresh", ui->coverSpinBox->value()}
    };
    return obj;
}

void RuleItem::fromJson(QJsonObject obj)
{
    ui->classSpinBox->setValue(obj.find("class").value().toInt());
    ui->coverSpinBox->setValue(obj.find("thresh").value().toDouble(0.1));
}

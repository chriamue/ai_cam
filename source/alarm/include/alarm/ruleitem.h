#ifndef RULEITEM_H
#define RULEITEM_H

#include <QWidget>
#include <QJsonObject>

namespace Ui {
class RuleItem;
}

class RuleItem : public QWidget
{
    Q_OBJECT

public:
    explicit RuleItem(QWidget *parent = 0);
    ~RuleItem();
    QJsonObject toJson();
    void fromJson(QJsonObject obj);

private:
    Ui::RuleItem *ui;
};

#endif // RULEITEM_H

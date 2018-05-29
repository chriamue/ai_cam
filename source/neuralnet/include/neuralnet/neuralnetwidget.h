#ifndef NEURALNETWIDGET_H
#define NEURALNETWIDGET_H

#include <neuralnet/neuralnet_api.h>

#include <QWidget>

namespace Ui {
class NeuralNetWidget;
}

class NEURALNET_API NeuralNetWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NeuralNetWidget(QWidget *parent = 0);
    ~NeuralNetWidget();

private slots:
    void on_trainButton_clicked();

    void on_predictButton_clicked();

private:
    Ui::NeuralNetWidget *ui;
};

#endif // NEURALNETWIDGET_H

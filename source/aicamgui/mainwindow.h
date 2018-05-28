#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <QMainWindow>
#include <QList>

#include <superpixellib/imageeditor.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_imageEditButton_clicked();

private:
    Ui::MainWindow *ui;
    void initFolders();
    void initImages();

    std::shared_ptr<ImageEditor> imageEditor;
    QList<std::string> imagesNames;
};

#endif // MAINWINDOW_H

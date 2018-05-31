#ifndef VIDEOSTREAM_H
#define VIDEOSTREAM_H

#include <QWidget>

#include <memory>
#include <opencv2/videoio.hpp>

#include <videostream/videostream_api.h>
#include <neuralnet/neuralnet.h>

#include <QTimer>

namespace Ui {
class VideoStream;
}

class VIDEOSTREAM_API VideoStream : public QWidget
{
    Q_OBJECT

public:
    explicit VideoStream(QWidget *parent = 0);
    ~VideoStream();

private slots:
    void on_connectButton_clicked();
    void on_read_stream();

    void on_stopButton_clicked();

    void on_saveButton_clicked();

    void on_pushButton_clicked();

private:
    Ui::VideoStream *ui;
    std::shared_ptr<cv::VideoCapture> videoCapture;
    std::shared_ptr<QPixmap> pixmap;

    QTimer timer;
    int fps = 25;
    cv::Mat frame;
    std::shared_ptr<neuralnet> neuralNet;
};

#endif // VIDEOSTREAM_H

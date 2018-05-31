#ifndef IMAGEEDITOR_H
#define IMAGEEDITOR_H

#include <QWidget>
#include <opencv2/opencv.hpp>

#include <superpixellib/superpixellib_api.h>

namespace Ui {
class ImageEditor;
}

class SUPERPIXELLIB_API ImageEditor : public QWidget {
  Q_OBJECT

public:
  explicit ImageEditor(std::string imagefile, QWidget *parent = nullptr);
    explicit ImageEditor(std::string imagefile, std::string labelfile, bool saveOnExit = false, QWidget *parent = nullptr);
  ~ImageEditor();
  void reloadImage();
  int SuperPixel_Seg(cv::Mat src, cv::Mat &matLabels);
  QPixmap fromMat(cv::Mat &img);
  bool eventFilter(QObject *watched, QEvent *event);
  void paintSuperPixel(QPoint p);
  cv::Mat getSegmentation();
  QColor idToColor(uint8_t id) const;
  void colorize(cv::Mat &src, cv::Mat &dest);

  QPixmap fromMat_flat(cv::Mat &img);
private slots:
  void on_num_superpixels_valueChanged(int value);

  void on_saveButton_clicked();

private:
  bool saveOnExit = false;
  Ui::ImageEditor *ui;
  std::string labelfile;
  cv::Mat image;
  cv::Mat superpixel;
  cv::Mat mask;
  cv::Mat segment;
  cv::Mat segmentation;
  cv::Mat vis_seg;

  cv::Mat labels;

  bool mousePressed = false;
};

#endif // IMAGEEDITOR_H

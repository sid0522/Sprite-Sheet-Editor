#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QStringList>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    qreal scale;
    QGraphicsScene* preview;
    QGraphicsScene* exportPreview;
    QList<QImage> frames;
    QGraphicsPixmapItem* reference;
    QImage spriteSheet;
    QList<QGraphicsPixmapItem*> displayedFrames;
    QTransform matrix;
    Ui::MainWindow* ui;

    void drawEditArea();

public slots:
    void searchFiles();
    void searchReference();
    void cropTransparency();
    void setScaleValue(qint32 s);
    void setReferenceAlpha(qint32 a);
    void setVisibleFrames(qint32 i);
    void drawSpriteSheetPreview(qint32 width);
    void exportSpriteSheet();

};

#endif // MAINWINDOW_H

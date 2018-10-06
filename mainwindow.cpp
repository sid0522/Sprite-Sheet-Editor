#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QImageWriter>
#include <QtMath>

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    scale(1.0),
    frames(QList<QImage>()),
    reference(nullptr),
    displayedFrames(QList<QGraphicsPixmapItem*>()),
    matrix(QTransform()),
    ui(new Ui::MainWindow)

{
    ui->setupUi(this);

    ui->label_6->hide();
    ui->lineEdit_2->hide();
    ui->horizontalSlider_2->hide();

    preview = new QGraphicsScene(QRect(1, 1, ui->graphicsView->width(), ui->graphicsView->height()));
    exportPreview = new QGraphicsScene(QRectF(0, 0, ui->graphicsView_2->width(), ui->graphicsView_2->height()));

   ui->graphicsView->setScene(preview);
   ui->graphicsView_2->setScene(exportPreview);

   ui->horizontalSlider->setDisabled(true);
   ui->horizontalSlider_2->setDisabled(true);
   ui->pushButton->setDisabled(true);
   ui->pushButton_2->setDisabled(true);
   ui->spinBox->setDisabled(true);
   ui->spinBox_2->setDisabled(true);
   ui->spinBox_3->setDisabled(true);

   connect(ui->actionImport, SIGNAL(triggered()), this, SLOT(searchFiles()));
   connect(ui->actionImport_reference, SIGNAL(triggered()), this, SLOT(searchReference()));
   connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
   connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(setScaleValue(int)));
   connect(ui->horizontalSlider_2, SIGNAL(valueChanged(int)), this, SLOT(setReferenceAlpha(int)));
   connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(cropTransparency()));
   connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(exportSpriteSheet()));
   connect(ui->spinBox, SIGNAL(valueChanged(int)), this, SLOT(setVisibleFrames(int)));
   connect(ui->spinBox_2, SIGNAL(valueChanged(int)), this, SLOT(setVisibleFrames(int)));
   connect(ui->spinBox_3, SIGNAL(valueChanged(int)), this, SLOT(drawSpriteSheetPreview(int)));
}

MainWindow::~MainWindow()
{
    delete preview;
    delete ui;
}

void MainWindow::drawSpriteSheetPreview(qint32 width)
{
    exportPreview->clear();
    QPainter* exported = new QPainter();
    spriteSheet = QImage(QSize(width * frames.at(0).width() * scale, qCeil(qreal(frames.length()) / width) * frames.at(0).height() * scale),
                         QImage::Format_ARGB32_Premultiplied);
        spriteSheet.fill(Qt::GlobalColor::transparent);
        exported->begin(&spriteSheet);
        exported->scale(scale, scale);
        for(qint32 i = 0, l = frames.length(); i < l; i++)
            exported->drawImage((i % width) * frames.at(i).width(), qFloor(i / width) * frames.at(i).height(), frames.at(i));
        exported->end();
        delete exported;

        qreal previewScale = qreal(ui->graphicsView_2->width()) / (width * frames.at(0).width() * scale);
        if(previewScale < 1)
        {
            QTransform m;
            m.setMatrix(
                previewScale, m.m12(), m.m13(),
                m.m21(), previewScale, m.m23(),
                m.m31(), m.m32(), m.m33());
            ui->graphicsView_2->setTransform(m);
        }
        ui->spinBox_3->setValue(width);
        exportPreview->addPixmap(QPixmap::fromImage(spriteSheet));
}

void MainWindow::drawEditArea()
{
    QPixmap pic;
    if(reference != nullptr)
        pic = reference->pixmap();
    preview->clear();
    displayedFrames.clear();
    for(auto frame : frames)
        displayedFrames.append(preview->addPixmap(QPixmap::fromImage(frame)));
    if(!pic.isNull()) reference = preview->addPixmap(pic);
}

void MainWindow::searchReference()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select Image"), QDir::homePath(), tr("Images (*.png)"));
    reference = preview->addPixmap(QPixmap::fromImage(QImage(file)));
    drawEditArea();
    ui->horizontalSlider_2->setDisabled(false);
    ui->horizontalSlider_2->setValue(ui->horizontalSlider_2->maximum());
}

void MainWindow::searchFiles()
{
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Select Images"), QDir::homePath(), tr("Images (*.png)"));
    files.sort();

    frames.clear();
    for(auto file : files)
        frames.append(QImage(file));

    drawEditArea();
    drawSpriteSheetPreview(qSqrt(frames.length()));

    ui->horizontalSlider->setDisabled(false);
    ui->pushButton->setDisabled(false);
    ui->pushButton_2->setDisabled(false);
    ui->spinBox->setDisabled(false);
    ui->spinBox_2->setDisabled(false);
    ui->spinBox_3->setDisabled(false);

    ui->horizontalSlider->setValue(ui->horizontalSlider->maximum());
    ui->horizontalSlider->hide();
    ui->spinBox->setMaximum(frames.length() - 1);
    ui->spinBox_2->setMaximum(frames.length() - 1);
    ui->spinBox->setValue(0);
    ui->spinBox_2->setValue(frames.length() - 1);
    ui->spinBox_3->setMaximum(frames.length());
}

void MainWindow::setReferenceAlpha(qint32 a)
{
    qreal alpha = qreal(a) / ui->horizontalSlider->maximum();

    reference->setOpacity(alpha);
    QString text = QString();
    text.setNum(alpha);
    ui->lineEdit_2->setText(text);
    drawSpriteSheetPreview(ui->spinBox_3->value());
}

void MainWindow::setScaleValue(qint32 s)
{
    scale = qreal(s) / ui->horizontalSlider->maximum();
    matrix.setMatrix(
                scale, matrix.m12(), matrix.m13(),
                matrix.m21(), scale, matrix.m23(),
                matrix.m31(), matrix.m32(), matrix.m33());
    for(auto disp : displayedFrames)
        disp->setTransform(matrix);

    QString text = QString();
    text.setNum(scale);
    ui->lineEdit->setText(text);
    drawSpriteSheetPreview(ui->spinBox_3->value());
}

qint32 minOpaqueX(QImage frame)
{
    for(qint32 i = 0, width = frame.width(); i < width; i++)
        for(qint32 j = 0, height = frame.height(); j < height; j++)
            if(qAlpha(frame.pixel(i, j)) != 0)
                return i;
    return 0;
}

qint32 minOpaqueY(QImage frame)
{
    for(qint32 j = 0, height = frame.height(); j < height; j++)
        for(qint32 i = 0, width = frame.width(); i < width; i++)
            if(qAlpha(frame.pixel(i, j)) != 0) return j;
    return 0;
}

qint32 maxOpaqueX(QImage frame)
{
    for(qint32 i = frame.width() - 1; i >= 0; i--)
        for(qint32 j = 0, height = frame.height(); j < height; j++)
            if(qAlpha(frame.pixel(i, j)) != 0) return i;
    return 0;
}

qint32 maxOpaqueY(QImage frame)
{
    for(qint32 j = frame.height() - 1; j >= 0; j--)
        for(qint32 i = 0, width = frame.width(); i < width; i++)
            if(qAlpha(frame.pixel(i, j)) != 0) return j;
    return 0;
}

void MainWindow::cropTransparency()
{
    qint32 minX = INT_MAX;
    qint32 maxX = 0;
    qint32 minY = INT_MAX;
    qint32 maxY = 0;
    for(auto frame : frames)
        if(minX > minOpaqueX(frame)) minX = minOpaqueX(frame);
    for(auto frame : frames)
        if(maxX < maxOpaqueX(frame)) maxX = maxOpaqueX(frame);
    for(auto frame : frames)
        if(minY > minOpaqueY(frame)) minY = minOpaqueY(frame);
    for(auto frame : frames)
        if(maxY < maxOpaqueY(frame)) maxY = maxOpaqueY(frame);

    QRect crop(minX, minY, maxX - minX + 1, maxY - minY + 1);

    auto newFrameList = QList<QImage>();
    for(auto frame : frames)
        newFrameList.append(frame.copy(crop));
    frames = newFrameList;

    drawEditArea();
    drawSpriteSheetPreview(ui->spinBox_3->value());
}

void MainWindow::setVisibleFrames(qint32 i)
{
    qint32 min = ui->spinBox->value();
    qint32 max = ui->spinBox_2->value();

    for(auto d : displayedFrames) d->setVisible(false);

    for(qint32 i = min; i <= max; i++)
        displayedFrames.at(i)->setVisible(true);
}

void MainWindow::exportSpriteSheet()
{
    QString file = QFileDialog::getSaveFileName(this, tr("Save SpriteSheet"), QDir::homePath(), tr("Images (*.png)"));
    if(!file.endsWith(".png")) file.append(".png");
    QImageWriter fileWrite(file);
    fileWrite.write(spriteSheet);
}

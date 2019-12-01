#pragma once

#include <qgraphicsitem.h>
#include <qgraphicsview.h>
#include <qslider.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qdialog.h>

class PixItem : public QGraphicsItem
{
public:
	PixItem(QPixmap* pixmap);
	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
	void updateItem(QString path);

private:
	QPixmap pix;//图元显示的图片
};

class PreviewView : public QGraphicsView
{
	Q_OBJECT
public:
	PreviewView();
	~PreviewView();

	float _scale;						//缩放值
	int x;
	int y;

	PixItem* image;
	QString path;
	QSlider* slider;					//选择层的滑块
	QLabel* label;						//显示层数的标签

private:
	QVBoxLayout* layerLayout;
	QGraphicsScene* scene;
	QPixmap* pixmap;

private slots:
	void valueChange(int num);

protected:
	void wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
	void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
};

class PreviewDialog : public QDialog
{
public:
	PreviewDialog();
	~PreviewDialog();

	PreviewView* previewWidget;

	bool readIni(QString path);
private:
	//QLabel* printTimeName;
	//QLabel* printTimeLabel;
	QLabel* modelVolumeName;
	QLabel* modelVolumeLabel;
	QLabel* numberSlicesName;
	QLabel* numberSlicesLabel;
	QHBoxLayout* layout1;
	QGridLayout* mainLayout;
};
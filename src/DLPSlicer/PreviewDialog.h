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
	QPixmap pix;//ͼԪ��ʾ��ͼƬ
};

class PreviewView : public QGraphicsView
{
	Q_OBJECT
public:
	PreviewView();
	~PreviewView();

	float _scale;						//����ֵ
	int x;
	int y;

	PixItem* image;
	QString path;
	QSlider* slider;					//ѡ���Ļ���
	QLabel* label;						//��ʾ�����ı�ǩ

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
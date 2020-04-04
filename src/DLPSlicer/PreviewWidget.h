#pragma once
#include <qwidget.h>
#include <qlabel.h>
#include <qslider.h>
#include <qlineedit.h>

#include "Gadget.h"
#include "Point.hpp"
#include "DLPrint.h"

class MainWindow;

//���Ĵ����������Ĳ��������࣬��������������������parent����
//���������ڣ���ֻ������Ĺ��ܡ�
class PreviewTopWidget : public QObject
{
	Q_OBJECT

public:
	PreviewTopWidget(MainWindow* parent);
	~PreviewTopWidget();

	QSlider* m_slider;

private:
	MainWindow* m_parent;

	PushButton* m_saveBtn;
	QLabel* m_curLayerLab;

private slots:
	void slot_layerChanged(int);

public:
	void reload(size_t max);
	void setEnable(bool);
	void resize(const QRect& rect);

	void HideWidget();
	void ShowWidget();
};

class PreviewWidget : public QWidget
{
	Q_OBJECT

public:
	PreviewWidget(DLPrint* print);
	~PreviewWidget();

	void reload();//ÿ����Ƭ���ȡ

	DLPrint* m_pdlprint;
	size_t m_cur_layer = 0;
	bool m_dirty = true;//�ж�ģ���Ƿ�ı䣬�ı������ڵ���Ƭ������Ч

private:
	float m_scale = 1.0;

signals:
	void sig_sizeChange();

public slots:
	void slot_layerChanged(int);

protected:
	void paintEvent(QPaintEvent* event);
	void resizeEvent(QResizeEvent* event);
};


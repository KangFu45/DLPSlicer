#pragma once
#include <qwidget.h>
#include <qlabel.h>
#include <qslider.h>
#include <qlineedit.h>

#include "Gadget.h"
#include "Point.hpp"
#include "DLPrint.h"

class MainWindow;

//中心窗口上悬浮的部件管理类，基础部件均能悬浮，但parent部件
//还是主窗口，其只做管理的功能。
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

	void reload();//每次切片完读取

	DLPrint* m_pdlprint;
	size_t m_cur_layer = 0;
	bool m_dirty = true;//判断模型是否改变，改变则现在的切片数据无效

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


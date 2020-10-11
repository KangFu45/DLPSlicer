#pragma once
#include <qwidget.h>
#include <qlabel.h>
#include <qslider.h>
#include <qlineedit.h>

#include "Gadget.h"
#include "Point.hpp"
#include "DLPrint.h"

class MainWindow;

class InfoWidget : public QFrame
{
public:
	InfoWidget(QWidget* parent = nullptr);

	QLabel* m_layersLab;
	QLabel* m_sizeLab;
	QLabel* m_volumeLab;
};

//中心窗口上悬浮的部件管理类，基础部件均能悬浮，但parent部件
//还是主窗口，其只做管理的功能。
class PreviewTopWidget : public QObject
{
	Q_OBJECT

public:
	PreviewTopWidget(MainWindow* parent);
	~PreviewTopWidget();

	QSlider* m_slider;

	void reload(size_t max);
	void setEnable(bool);
	void resize(const QRect& rect);

	void HideWidget();
	void ShowWidget();

	void setLayers(int num) { m_infoWidget->m_layersLab->setText(QString::number(num)); };
	void setSize(int x, int y, int z) { m_infoWidget->m_sizeLab->setText(QString("%1x%2x%3mm").arg(x).arg(y).arg(z)); };
	void setVolume(float num) { m_infoWidget->m_volumeLab->setText(QString("%1ml").arg(num / 1000.0)); };

private:
	MainWindow* m_parent;

	PushButton* m_saveBtn;
	QLabel* m_curLayerLab;
	InfoWidget* m_infoWidget;

private slots:
	void slot_layerChanged(int);
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


#pragma once

#include <qobject.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qstackedwidget.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qprogressbar.h>

#include "Gadget.h"

class MainWindow;

class ViewWidget : public QWidget
{
	Q_OBJECT

public:
	ViewWidget(QWidget* parent = nullptr);
	~ViewWidget();

	QPushButton* m_defViewBtn;
	QPushButton* m_leftViewBtn;
	QPushButton* m_rightViewBtn;
	QPushButton* m_frontViewBtn;
	QPushButton* m_behindViewBtn;

	QPushButton* m_zoomInViewBtn;
	QPushButton* m_zoomOutViewBtn;
};

class OffsetWidget : public QFrame
{
	Q_OBJECT

public:
	OffsetWidget(QWidget* parent = nullptr);
	~OffsetWidget();

	//平移对话框
	QDoubleSpinBox* x_offset_spin;
	QDoubleSpinBox* y_offset_spin;
	QDoubleSpinBox* z_offset_spin;
	QPushButton* ZPosZeroBtn;
};

class RotateWidget : public QFrame
{
	Q_OBJECT

public:
	RotateWidget(QWidget* parent = nullptr);
	~RotateWidget();

	QDoubleSpinBox* x_rotate_spin;
	QDoubleSpinBox* y_rotate_spin;
	QDoubleSpinBox* z_rotate_spin;
};

class ScaleWidget : public QFrame
{
	Q_OBJECT

public:
	ScaleWidget(QWidget* parent = nullptr);
	~ScaleWidget();

	//缩放对话框
	QDoubleSpinBox* x_scale_spin;
	QDoubleSpinBox* y_scale_spin;
	QDoubleSpinBox* z_scale_spin;
	QDoubleSpinBox* x_size_spin;
	QDoubleSpinBox* y_size_spin;
	QDoubleSpinBox* z_size_spin;
	QCheckBox* unify_scale;							//统一缩放
};


//中心窗口上悬浮的部件管理类，基础部件均能悬浮，但parent部件
//还是主窗口，其只做管理的功能。
class CenterTopWidget : public QObject
{
	Q_OBJECT

public:
	CenterTopWidget(MainWindow* parent);
	~CenterTopWidget();

	//点击的按钮枚举
	enum CenterBtn {
		NULLBTN = 0x01,
		OPENBTN,
		OFFSETBTN,
		SCALEBTN,
		ROTATEBTN,
		SLICEBTN,
		SETUPBTN,
		SUPPORTBTN,
		SUPPORTEDITBTN
	}currentBtn = { NULLBTN };
private:
	MainWindow* m_parent;
	QLabel* m_printerLabel;

private slots:
	void setUnifyScale(int state);

public:

	QComboBox* m_printerCombo;

	//按钮
	PushButton* m_openBtn;
	PushButton* m_supportBtn;
	PushButton* m_supportEditBtn;
	PushButton* m_sliceBtn;
	PushButton* m_setupBtn;
	PushButton* m_offsetBtn;
	PushButton* m_rotateBtn;
	PushButton* m_scaleBtn;

	OffsetWidget* m_offsetWidget;
	//TODO:需用model instance里的值替代
	double x_rotate = {0};
	double y_rotate = {0};
	double z_rotate = {0};
	RotateWidget* m_rotateWidget;
	ScaleWidget* m_scaleWidget;
	ViewWidget* m_viewWidget;


	void setXOffsetVal(double val) { this->m_offsetWidget->x_offset_spin->setValue(val); };
	void setYOffsetVal(double val) { this->m_offsetWidget->y_offset_spin->setValue(val); };
	void setZOffsetVal(double val) { this->m_offsetWidget->z_offset_spin->setValue(val); };

	void setXRotateVal(double val) { this->m_rotateWidget->x_rotate_spin->setValue(val); };
	void setYRotateVal(double val) { this->m_rotateWidget->y_rotate_spin->setValue(val); };
	void setZRotateVal(double val) { this->m_rotateWidget->z_rotate_spin->setValue(val); };

	void setXScaleVal(double val) { this->m_scaleWidget->x_scale_spin->setValue(val); };
	void setYScaleVal(double val) { this->m_scaleWidget->y_scale_spin->setValue(val); };
	void setZScaleVal(double val) { this->m_scaleWidget->z_scale_spin->setValue(val); };
	void setXSizeVal(double val) { this->m_scaleWidget->x_size_spin->setValue(val); };
	void setYSizeVal(double val) { this->m_scaleWidget->y_size_spin->setValue(val); };
	void setZSizeVal(double val) { this->m_scaleWidget->z_size_spin->setValue(val); };

	void resize(const QRect& rect);
	void CenterButtonPush(CenterBtn btn);
	bool isUnityScale() { return m_scaleWidget->unify_scale->checkState() == Qt::Unchecked; };

	void HideWidget();
	void ShowWidget();
};


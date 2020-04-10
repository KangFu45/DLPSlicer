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
	};
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
	CenterBtn currentBtn{ NULLBTN };

	//平移对话框
	QDoubleSpinBox* x_offset_spin;
	QDoubleSpinBox* y_offset_spin;
	QDoubleSpinBox* z_offset_spin;
	QPushButton* ZPosZeroBtn;
	QWidget* m_offsetWidget;

	//TODO:需用model instance里的值替代
	double x_rotate;
	double y_rotate;
	double z_rotate;

	QDoubleSpinBox* x_rotate_spin;
	QDoubleSpinBox* y_rotate_spin;
	QDoubleSpinBox* z_rotate_spin;
	QWidget* m_rotateWidget;

	//缩放对话框
	QDoubleSpinBox* x_scale_spin;
	QDoubleSpinBox* y_scale_spin;
	QDoubleSpinBox* z_scale_spin;
	QDoubleSpinBox* x_size_label;
	QDoubleSpinBox* y_size_label;
	QDoubleSpinBox* z_size_label;
	QCheckBox* unify_scale;							//统一缩放
	QWidget* m_scaleWidget;

	void resize(const QRect& rect);
	void CenterButtonPush(CenterBtn btn);
	bool isUnityScale() { return unify_scale->checkState() == Qt::Unchecked; };

	void HideWidget();
	void ShowWidget();
};


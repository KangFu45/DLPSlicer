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

//���Ĵ����������Ĳ��������࣬��������������������parent����
//���������ڣ���ֻ������Ĺ��ܡ�
class CenterTopWidget : public QObject
{
	Q_OBJECT

public:
	CenterTopWidget(MainWindow* parent);
	~CenterTopWidget();

	//����İ�ťö��
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

	//��ť
	PushButton* m_openBtn;
	PushButton* m_supportBtn;
	PushButton* m_supportEditBtn;
	PushButton* m_sliceBtn;
	PushButton* m_setupBtn;
	PushButton* m_offsetBtn;
	PushButton* m_rotateBtn;
	PushButton* m_scaleBtn;
	CenterBtn currentBtn{ NULLBTN };

	//ƽ�ƶԻ���
	QDoubleSpinBox* x_offset_spin;
	QDoubleSpinBox* y_offset_spin;
	QDoubleSpinBox* z_offset_spin;
	QPushButton* ZPosZeroBtn;
	QWidget* m_offsetWidget;

	//TODO:����model instance���ֵ���
	double x_rotate;
	double y_rotate;
	double z_rotate;

	QDoubleSpinBox* x_rotate_spin;
	QDoubleSpinBox* y_rotate_spin;
	QDoubleSpinBox* z_rotate_spin;
	QWidget* m_rotateWidget;

	//���ŶԻ���
	QDoubleSpinBox* x_scale_spin;
	QDoubleSpinBox* y_scale_spin;
	QDoubleSpinBox* z_scale_spin;
	QDoubleSpinBox* x_size_label;
	QDoubleSpinBox* y_size_label;
	QDoubleSpinBox* z_size_label;
	QCheckBox* unify_scale;							//ͳһ����
	QWidget* m_scaleWidget;

	void resize(const QRect& rect);
	void CenterButtonPush(CenterBtn btn);
	bool isUnityScale() { return unify_scale->checkState() == Qt::Unchecked; };

	void HideWidget();
	void ShowWidget();
};


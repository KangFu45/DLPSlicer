#pragma once

#include <qobject.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qstackedwidget.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qprogressbar.h>

//可以被点击按钮的QSS
static QString OnButton("border: 1px outset white; border-radius: 5px;  background-color: rgba(100,225,50,225);");
//不能被点击按钮的QSS
static QString OffButton("border: 1px outset white; border-radius: 5px; background-color: rgba(200,200,200,150);");

class MainWindow;

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


//中心窗口上悬浮的部件管理类，基础部件均能悬浮，但parent部件
//还是主窗口，其只做管理的功能。
class CenterTopWidget : public QObject
{
	Q_OBJECT

public:
	CenterTopWidget(MainWindow* parent);
	~CenterTopWidget();

private:
	MainWindow* m_parent;

	QLabel* m_printerLabel;

private slots:
	void setUnifyScale(int state);

public:
	void resize(const QSize& size);
	void CenterButtonPush(CenterBtn btn);
	bool isUnityScale();

	void P(size_t i);
	void showProgress(int btn);
	void hideProgress();

	//自定义的按钮
	class PushButton : public QPushButton
	{
		//Q_OBJECT
	public:
		explicit PushButton(QWidget* parent = nullptr) : QPushButton(parent) {};
		explicit PushButton(const QString& text, QWidget* parent = nullptr) : QPushButton(text, parent) {};
		PushButton(const QIcon& icon, const QString& text, QWidget* parent = nullptr) : QPushButton(icon, text, parent) {};
		~PushButton() {};

		void push() {
			if (this->checked)
				setStyleSheet(OnButton);
			else
				setStyleSheet(OffButton);
			this->checked = !this->checked;
		};
		bool c_isChecked() { return this->checked; };

	private:
		//给按钮增加是否按下的属性
		//需控制其是否按下或弹起的操作
		bool checked = { false };
	};

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
	CenterBtn currentBtn = { NULLBTN };

	//进度条对话框
	QLabel* m_progressLabel;
	QProgressBar* m_progressBar;
	QWidget* m_progressWidget;

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
};


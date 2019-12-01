#pragma once

#include <qobject.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qstackedwidget.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qprogressbar.h>

//���Ա������ť��QSS
static QString OnButton("border: 1px outset white; border-radius: 5px;  background-color: rgba(100,225,50,225);");

//���ܱ������ť��QSS
static QString OffButton("border: 1px outset white; border-radius: 5px; background-color: rgba(200,200,200,150);");


class MainWindow;

//����İ�ťö��
enum CenterBtn {
	NULLBTN,
	OPENBTN,
	OFFSETBTN,
	SCALEBTN,
	ROTATEBTN,
	SLICEBTN,
	SETUPBTN,
	SUPPORTBTN,
	SUPPORTEDITBTN
};


//���Ĵ����������Ĳ��������࣬��������������������parent����
//���������ڣ���ֻ������Ĺ��ܡ�
class CenterTopWidget : public QObject
{
	Q_OBJECT

public:
	CenterTopWidget(MainWindow* parent);
	~CenterTopWidget();

private:
	MainWindow* m_parent;

private slots:
	void showOffsetWidget();
	void showRotateWidget();
	void showScaleWidget();

	void setUnifyScale(int state);

public:
	void resize(QSize size);
	void CenterButtonPush(CenterBtn btn);

	void initOffsetWidget();
	void initRotateWidget();
	void initScaleWidget();
	void initProgress();

	void P(size_t i);
	void showProgress(int btn);
	void hideProgress();

	//�������Ի���
	QLabel* progressLabel;
	QProgressBar* progress;
	QStackedWidget* progressWidget;

	QLabel* SelectPrinterLabel;
	QComboBox* SelectPrinterCombo;

	//�Զ���İ�ť
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
			this->checked = !this->checked; };
		bool c_isChecked() { return this->checked; };

	private:
		//����ť�����Ƿ��µ�����
		//��������Ƿ��»���Ĳ���
		bool checked = { false };
	};

	//��ť
	PushButton* m_openBtn;
	PushButton* m_supportBtn;
	PushButton* m_supportEditBtn;
	PushButton* m_sliceBtn;
	PushButton* m_setupBtn;
	PushButton* m_offsetBtn;
	PushButton* m_rotateBtn;
	PushButton* m_scaleBtn;
	CenterBtn currentBtn = { NULLBTN };

	//ƽ�ƶԻ���
	QDoubleSpinBox* x_offset_spin;
	QDoubleSpinBox* y_offset_spin;
	QDoubleSpinBox* z_offset_spin;
	QStackedWidget* offsetWidget;

	//��ת�Ի���
	double x_rotate;
	double y_rotate;
	double z_rotate;
	void init_rotate();

	QDoubleSpinBox* x_rotate_spin;
	QDoubleSpinBox* y_rotate_spin;
	QDoubleSpinBox* z_rotate_spin;
	QStackedWidget* rotateWidget;

	//���ŶԻ���
	QDoubleSpinBox* x_scale_spin;
	QDoubleSpinBox* y_scale_spin;
	QDoubleSpinBox* z_scale_spin;
	QDoubleSpinBox* x_size_label;
	QDoubleSpinBox* y_size_label;
	QDoubleSpinBox* z_size_label;
	QCheckBox* unify_scale;							//ͳһ����
	QStackedWidget* scaleWidget;

};


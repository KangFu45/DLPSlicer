#include "CenterTopWidget.h"
#include "mainwindow.h"

#include "Setting.h"
extern Setting e_setting;

//------------------ViewWidget----------------------
ViewWidget::ViewWidget(QWidget* parent) : QWidget(parent)
{
	m_defViewBtn=new QPushButton(QIcon(":/icon/images/iso.png"),"");
	m_defViewBtn->setFixedSize(20, 20);

	m_leftViewBtn = new QPushButton(QIcon(":/icon/images/left.png"), "");
	m_leftViewBtn->setFixedSize(20, 20);

	m_rightViewBtn = new QPushButton(QIcon(":/icon/images/right.png"), "");
	m_rightViewBtn->setFixedSize(20, 20);

	m_frontViewBtn = new QPushButton(QIcon(":/icon/images/front.png"), "");
	m_frontViewBtn->setFixedSize(20, 20);

	m_behindViewBtn = new QPushButton(QIcon(":/icon/images/back.png"), "");
	m_behindViewBtn->setFixedSize(20, 20);

	m_zoomInViewBtn = new QPushButton(QIcon(":/icon/images/add.png"), "");
	m_zoomInViewBtn->setFixedSize(20, 20);

	m_zoomOutViewBtn = new QPushButton(QIcon(":/icon/images/delete.png"), "");
	m_zoomOutViewBtn->setFixedSize(20, 20);

	QGridLayout* layout = new QGridLayout(this);
	layout->addWidget(m_behindViewBtn, 0, 1);
	layout->addWidget(m_leftViewBtn, 1, 0);
	layout->addWidget(m_defViewBtn, 1, 1);
	layout->addWidget(m_rightViewBtn, 1, 2);
	layout->addWidget(m_frontViewBtn, 2, 1);
	layout->addWidget(m_zoomInViewBtn, 3,1);
	layout->addWidget(m_zoomOutViewBtn, 4, 1);
	this->setLayout(layout);

	this->setFixedSize(60, 100);
}

ViewWidget::~ViewWidget() {

}

//-----------------OffsetWidget----------------------
OffsetWidget::OffsetWidget(QWidget* parent) : QFrame(parent)
{
	//--------------initOffsetWidget---------------
	QLabel* offset_title = new QLabel(QStringLiteral("  平  移"));
	QLabel* x_offset_label = new QLabel("  X");
	x_offset_label->setStyleSheet("color: rgb(225,0,0); background-color: rgba(225,225,225,0);");
	QLabel* y_offset_label = new QLabel("  Y");
	y_offset_label->setStyleSheet("color: rgb(0,225,0); background-color: rgba(225,225,225,0);");
	QLabel* z_offset_label = new QLabel("  Z");
	z_offset_label->setStyleSheet("color: rgb(0,0,225); background-color: rgba(225,225,225,0);");
	x_offset_spin = new QDoubleSpinBox();
	x_offset_spin->setRange(-1000, 1000);
	x_offset_spin->setSingleStep(1);
	x_offset_spin->setSuffix("mm");

	y_offset_spin = new QDoubleSpinBox();
	y_offset_spin->setRange(-1000, 1000);
	y_offset_spin->setSingleStep(1);
	y_offset_spin->setSuffix("mm");

	z_offset_spin = new QDoubleSpinBox();
	z_offset_spin->setRange(-1000, 1000);
	z_offset_spin->setSingleStep(1);
	z_offset_spin->setSuffix("mm");

	ZPosZeroBtn = new QPushButton(QStringLiteral("贴底"));

	QGridLayout* mainlayout = new QGridLayout();
	mainlayout->setMargin(7);
	mainlayout->setSpacing(7);
	mainlayout->addWidget(offset_title, 0, 1, 1, 2);
	mainlayout->addWidget(x_offset_label, 1, 0);
	mainlayout->addWidget(x_offset_spin, 1, 1, 1, 3);
	mainlayout->addWidget(y_offset_label, 2, 0);
	mainlayout->addWidget(y_offset_spin, 2, 1, 1, 3);
	mainlayout->addWidget(z_offset_label, 3, 0);
	mainlayout->addWidget(z_offset_spin, 3, 1, 1, 3);
	mainlayout->addWidget(ZPosZeroBtn, 4, 0, 1, 4);
	this->setLayout(mainlayout);
}

OffsetWidget::~OffsetWidget() {
	delete x_offset_spin;
	delete y_offset_spin;
	delete z_offset_spin;
	delete ZPosZeroBtn;
}

//-----------------RotateWidget-----------------------
RotateWidget::RotateWidget(QWidget* parent) : QFrame(parent)
{
	//-----------------initRotateWidget-------------------
	QLabel* rotate_title = new QLabel(QStringLiteral("  旋  转"));
	QLabel* x_rotate_label = new QLabel("  X");
	x_rotate_label->setStyleSheet("color: rgb(225,0,0); background-color: rgba(225,225,225,0);");
	QLabel* y_rotate_label = new QLabel("  Y");
	y_rotate_label->setStyleSheet("color: rgb(0,255,0); background-color: rgba(225,225,225,0);");
	QLabel* z_rotate_label = new QLabel("  Z");
	z_rotate_label->setStyleSheet("color: rgb(0,0,225); background-color: rgba(225,225,225,0);");
	x_rotate_spin = new QDoubleSpinBox();
	x_rotate_spin->setRange(-360.0, 360.0);
	x_rotate_spin->setSingleStep(5.0);
	x_rotate_spin->setSuffix(QStringLiteral("°"));

	y_rotate_spin = new QDoubleSpinBox();
	y_rotate_spin->setRange(-360.0, 360.0);
	y_rotate_spin->setSingleStep(5.0);
	y_rotate_spin->setSuffix(QStringLiteral("°"));

	z_rotate_spin = new QDoubleSpinBox();
	z_rotate_spin->setRange(-360.0, 360.0);
	z_rotate_spin->setSingleStep(5.0);
	z_rotate_spin->setSuffix(QStringLiteral("°"));

	QGridLayout* layout = new QGridLayout();
	layout->setMargin(7);
	layout->setSpacing(7);
	layout->addWidget(rotate_title, 0, 1, 1, 2);
	layout->addWidget(x_rotate_label, 1, 0);
	layout->addWidget(x_rotate_spin, 1, 1, 1, 3);
	layout->addWidget(y_rotate_label, 2, 0);
	layout->addWidget(y_rotate_spin, 2, 1, 1, 3);
	layout->addWidget(z_rotate_label, 3, 0);
	layout->addWidget(z_rotate_spin, 3, 1, 1, 3);
	this->setLayout(layout);
}

RotateWidget::~RotateWidget() {
	delete x_rotate_spin;
	delete y_rotate_spin;
	delete z_rotate_spin;
}

//----------------ScaleWidget------------------------
ScaleWidget::ScaleWidget(QWidget* parent) : QFrame(parent)
{
	//-----------------initScaleWidget---------------------
	QLabel* scale_title = new QLabel(QStringLiteral("  缩   放"));
	QLabel* x_scale_label = new QLabel(" X");
	x_scale_label->setStyleSheet("color: rgb(225,0,0); background-color: rgba(225,225,225,0);");
	QLabel* y_scale_label = new QLabel(" Y");
	y_scale_label->setStyleSheet("color: rgb(0,255,0); background-color: rgba(225,225,225,0);");
	QLabel* z_scale_label = new QLabel(" Z");
	z_scale_label->setStyleSheet("color: rgb(0,0,225); background-color: rgba(225,225,225,0);");
	x_scale_spin = new QDoubleSpinBox();
	x_scale_spin->setRange(1, 100000);
	x_scale_spin->setSingleStep(5);
	x_scale_spin->setSuffix("%");

	y_scale_spin = new QDoubleSpinBox();
	y_scale_spin->setRange(1, 100000);
	y_scale_spin->setSingleStep(5);
	y_scale_spin->setSuffix("%");
	y_scale_spin->setDisabled(true);

	z_scale_spin = new QDoubleSpinBox();
	z_scale_spin->setRange(1, 100000);
	z_scale_spin->setSingleStep(5);
	z_scale_spin->setSuffix("%");
	z_scale_spin->setDisabled(true);

	x_size_spin = new QDoubleSpinBox();
	x_size_spin->setRange(0, 1000);
	x_size_spin->setDisabled(true);
	x_size_spin->setSuffix("mm");

	y_size_spin = new QDoubleSpinBox();
	y_size_spin->setRange(0, 1000);
	y_size_spin->setDisabled(true);
	y_size_spin->setSuffix("mm");

	z_size_spin = new QDoubleSpinBox();
	z_size_spin->setRange(0, 1000);
	z_size_spin->setDisabled(true);
	z_size_spin->setSuffix("mm");

	unify_scale = new QCheckBox();
	unify_scale->setCheckState(Qt::Checked);

	QLabel* unify_scale_label = new QLabel(QStringLiteral("统一缩放"));

	QGridLayout* layout = new QGridLayout();
	layout->setMargin(7);
	layout->setSpacing(8);
	layout->addWidget(scale_title, 0, 3, 1, 3);
	layout->addWidget(x_scale_label, 1, 1);
	layout->addWidget(x_size_spin, 1, 2, 1, 3);
	layout->addWidget(x_scale_spin, 1, 5, 1, 3);
	layout->addWidget(y_scale_label, 2, 1);
	layout->addWidget(y_size_spin, 2, 2, 1, 3);
	layout->addWidget(y_scale_spin, 2, 5, 1, 3);
	layout->addWidget(z_scale_label, 3, 1);
	layout->addWidget(z_size_spin, 3, 2, 1, 3);
	layout->addWidget(z_scale_spin, 3, 5, 1, 3);
	layout->addWidget(unify_scale, 4, 5);
	layout->addWidget(unify_scale_label, 4, 6, 1, 2);
	this->setLayout(layout);
}

ScaleWidget::~ScaleWidget() {
	delete x_scale_spin;
	delete y_scale_spin;
	delete z_scale_spin;
	delete x_size_spin;
	delete y_size_spin;
	delete z_size_spin;
	delete unify_scale;
}

//-----------------CenterTopWidget--------------------

CenterTopWidget::CenterTopWidget(MainWindow* parent)
	:m_parent(parent)
{
	//机型选择部分
	m_printerLabel = new QLabel(QStringLiteral("选择机型"), m_parent);
	m_printerLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_printerLabel->setFixedSize(90, 20);

	m_printerCombo = new QComboBox(m_parent);
	for each (Setting::Printer per in e_setting.m_printers)
		m_printerCombo->addItem(QString(per.name.c_str()));
	m_printerCombo->setFixedSize(100, 20);
	m_printerCombo->setCurrentIndex(0);

	//left
	m_openBtn = new PushButton(QIcon(":/icon/images/load_b.png"), "", m_parent);
	m_openBtn->setIconSize(QSize(120, 140));
	m_openBtn->setStyleSheet(OnButton);
	m_openBtn->setFixedSize(60, 70);
	m_openBtn->setToolTip(QStringLiteral("打开"));
	(void)connect(m_openBtn, SIGNAL(clicked()), m_parent, SLOT(slot_openStl()));

	m_offsetBtn = new PushButton(QIcon(":/icon/images/offset_b.PNG"), "", m_parent);
	m_offsetBtn->setIconSize(QSize(90, 115));
	m_offsetBtn->setStyleSheet(OnButton);
	m_offsetBtn->setEnabled(true);
	m_offsetBtn->setFixedSize(60, 70);
	m_offsetBtn->setToolTip(QStringLiteral("移动"));
	(void)connect(m_offsetBtn, SIGNAL(clicked()), m_parent, SLOT(slot_showOffsetWidget()));

	m_scaleBtn = new PushButton(QIcon(":/icon/images/scale_b.png"), "", m_parent);
	m_scaleBtn->setIconSize(QSize(90, 115));
	m_scaleBtn->setStyleSheet(OnButton);
	m_scaleBtn->setEnabled(true);
	m_scaleBtn->setFixedSize(60, 70);
	m_scaleBtn->setToolTip(QStringLiteral("缩放"));
	(void)connect(m_scaleBtn, SIGNAL(clicked()), m_parent, SLOT(slot_showScaleWidget()));

	m_rotateBtn = new PushButton(QIcon(":/icon/images/rotate_b.png"), "", m_parent);
	m_rotateBtn->setIconSize(QSize(90, 115));
	m_rotateBtn->setStyleSheet(OnButton);
	m_rotateBtn->setEnabled(true);
	m_rotateBtn->setFixedSize(60, 70);
	m_rotateBtn->setToolTip(QStringLiteral("旋转"));
	(void)connect(m_rotateBtn, SIGNAL(clicked()), m_parent, SLOT(slot_showRotateWidget()));

	//right
	m_supportBtn = new PushButton(QIcon(":/icon/images/support_b.png"), "", m_parent);
	m_supportBtn->setIconSize(QSize(90, 115));
	m_supportBtn->setStyleSheet(OnButton);
	m_supportBtn->setFixedSize(60, 70);
	m_supportBtn->setToolTip(QStringLiteral("支撑"));
	(void)connect(m_supportBtn, SIGNAL(clicked()), m_parent, SLOT(slot_generateSupport()));

	m_supportEditBtn = new PushButton(QIcon(":/icon/images/supportEdit_b.png"), "", m_parent);
	m_supportEditBtn->setIconSize(QSize(90, 115));
	m_supportEditBtn->setStyleSheet(OnButton);
	m_supportEditBtn->setFixedSize(60, 70);
	m_supportEditBtn->setToolTip(QStringLiteral("支撑编辑"));
	(void)connect(m_supportEditBtn, SIGNAL(clicked()), m_parent, SLOT(slot_supportEdit()));

	m_sliceBtn = new PushButton(QIcon(":/icon/images/slice_b.png"), "", m_parent);
	m_sliceBtn->setIconSize(QSize(90, 115));
	m_sliceBtn->setStyleSheet(OnButton);
	m_sliceBtn->setFixedSize(60, 70);
	m_sliceBtn->setToolTip(QStringLiteral("切片"));
	(void)connect(m_sliceBtn, SIGNAL(clicked()), m_parent, SLOT(slot_slice()));

	m_setupBtn = new PushButton(QIcon(":/icon/images/setup_b.png"), "", m_parent);
	m_setupBtn->setIconSize(QSize(90, 115));
	m_setupBtn->setStyleSheet(OnButton);
	m_setupBtn->setFixedSize(60, 70);
	m_setupBtn->setToolTip(QStringLiteral("设置"));
	(void)connect(m_setupBtn, SIGNAL(clicked()), m_parent, SLOT(slot_showSetupDialog()));

	m_offsetWidget = new OffsetWidget(m_parent);
	(void)connect(m_offsetWidget->x_offset_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(slot_xoffsetValueChange(double)));
	(void)connect(m_offsetWidget->y_offset_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(slot_yoffsetValueChange(double)));
	(void)connect(m_offsetWidget->z_offset_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(slot_zoffsetValueChange(double)));
	(void)connect(m_offsetWidget->ZPosZeroBtn, SIGNAL(clicked()), m_parent, SLOT(slot_ZPosZero()));
	//m_offsetWidget->setMask(QPolygon(anomalyRect1));
	m_offsetWidget->setMinimumSize(180, 150);
	m_offsetWidget->hide();

	m_rotateWidget = new RotateWidget(m_parent);
	(void)connect(m_rotateWidget->x_rotate_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(slot_xRotateValueChange(double)));
	(void)connect(m_rotateWidget->y_rotate_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(slot_yRotateValueChange(double)));
	(void)connect(m_rotateWidget->z_rotate_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(slot_zRotateValueChange(double)));
	m_rotateWidget->setMinimumSize(170, 100);
	//rotateWidget->setMask(QPolygon(anomalyRect1));
	m_rotateWidget->hide();

	m_scaleWidget = new ScaleWidget(m_parent);
	(void)connect(m_scaleWidget->x_scale_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(slot_xScaleValueChange(double)));
	(void)connect(m_scaleWidget->y_scale_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(slot_yScaleValueChange(double)));
	(void)connect(m_scaleWidget->z_scale_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(slot_zScaleValueChange(double)));
	//(void)connect(x_size_spin, SIGNAL(valueChanged(double)), this, SLOT(xSizeValueChange(double)));
	//(void)connect(y_size_spin, SIGNAL(valueChanged(double)), this, SLOT(ySizeValueChange(double)));
	//(void)connect(z_size_spin, SIGNAL(valueChanged(double)), this, SLOT(zSizeValueChange(double)));
	(void)connect(m_scaleWidget->unify_scale, SIGNAL(stateChanged(int)), this, SLOT(setUnifyScale(int)));
	m_scaleWidget->setMinimumSize(230, 140);
	//scaleWidget->setMask(QPolygon(anomalyRect2));
	m_scaleWidget->hide();

	m_viewWidget = new ViewWidget(m_parent);
}

CenterTopWidget::~CenterTopWidget()
{
	delete m_printerLabel;
	delete m_printerCombo;
	
	delete m_openBtn;
	delete m_supportBtn;
	delete m_supportEditBtn;
	delete m_sliceBtn;
	delete m_setupBtn;
	delete m_offsetBtn;
	delete m_rotateBtn;
	delete m_scaleBtn;
	
	delete m_offsetWidget;
	delete m_rotateWidget;
	delete m_scaleWidget;
	delete m_viewWidget;
}

void CenterTopWidget::CenterButtonPush(CenterBtn btn)
{
	//TODO:命令约束控制

	//关闭其他按钮
	if (currentBtn != NULLBTN && currentBtn != btn)
		CenterButtonPush(currentBtn);

	//获取目标按钮
	PushButton* temp = nullptr;

	//控制同时只有一个btn被按下
	switch (btn)
	{
	case OPENBTN:
		temp = m_openBtn;
		m_openBtn->push();
		break;
	case OFFSETBTN:
		temp = m_offsetBtn;
		if (m_offsetBtn->c_isChecked())
			m_offsetWidget->hide();
		else
			m_offsetWidget->show();

		m_offsetBtn->push();
		break;
	case SCALEBTN:
		temp = m_scaleBtn;
		if (m_scaleBtn->c_isChecked())
			m_scaleWidget->hide();
		else 
			m_scaleWidget->show();

		m_scaleBtn->push();
		break;
	case ROTATEBTN:
		temp = m_rotateBtn;
		if (m_rotateBtn->c_isChecked())
			m_rotateWidget->hide();
		else {
			//使用相对角度值
			m_rotateWidget->x_rotate_spin->setValue(x_rotate = 0);
			m_rotateWidget->y_rotate_spin->setValue(y_rotate = 0);
			m_rotateWidget->z_rotate_spin->setValue(z_rotate = 0);
			m_rotateWidget->show();
		}

		m_rotateBtn->push();
		break;
	case SLICEBTN:
		temp = m_sliceBtn;
		m_sliceBtn->push();
		break;
	case SETUPBTN:
		temp = m_setupBtn;
		m_setupBtn->push();
		break;
	case SUPPORTBTN:
		temp = m_supportBtn;
		m_supportBtn->push();
		break;
	case SUPPORTEDITBTN:
		temp = m_supportEditBtn;
		m_supportEditBtn->push();
		break;
	case NULLBTN:
		break;
	default:
		break;
	}

	if (temp != nullptr) {
		if (temp->c_isChecked())
			currentBtn = btn;
		else
			currentBtn = NULLBTN;
	}
}


void CenterTopWidget::resize(const QRect& rect)
{
	int r = rect.right();
	int l = rect.left();
	int t = rect.top();

	int yh = rect.center().y();
	int xh = rect.center().x();

	m_printerLabel->move(xh - 110, t + 10);
	m_printerCombo->move(xh - 20, t + 10);

	//left
	m_openBtn->move(l + 10, t + 10);
	m_offsetBtn->move(l + 10, t + 90);
	m_scaleBtn->move(l + 10, t + 170);
	m_rotateBtn->move(l + 10, t + 250);

	//rigft
	m_supportBtn->move(l + 10, t + 330);
	m_supportEditBtn->move(l + 10, t + 410);
	m_sliceBtn->move(l + 10, t + 490);
	m_setupBtn->move(l + 10, t + 570);

	m_offsetWidget->move(l + 75, t + 10);
	m_rotateWidget->move(l + 75, t + 10);
	m_scaleWidget->move(l + 75, t + 10);
	m_viewWidget->move(r - 70, t + 10);
}

void CenterTopWidget::setUnifyScale(int state)
{
	switch (state)
	{
	case Qt::Unchecked:
		m_scaleWidget->y_scale_spin->setEnabled(true);
		m_scaleWidget->z_scale_spin->setEnabled(true);
		//y_size_spin->setEnabled(true);
		//z_size_spin->setEnabled(true);
		break;

	case Qt::Checked:
		m_scaleWidget->y_scale_spin->setDisabled(true);
		m_scaleWidget->z_scale_spin->setDisabled(true);
		//y_size_spin->setDisabled(true);
		//z_size_spin->setDisabled(true);
		break;

	default:
		break;
	}
}

void CenterTopWidget::HideWidget()
{
	m_openBtn->hide();
	m_offsetBtn->hide();
	m_rotateBtn->hide();
	m_scaleBtn->hide();

	m_supportBtn->hide();
	m_supportEditBtn->hide();
	m_setupBtn->hide();
	m_sliceBtn->hide();

	m_printerCombo->hide();
	m_printerLabel->hide();

	m_offsetWidget->hide();
	m_scaleWidget->hide();
	m_rotateWidget->hide();
	m_viewWidget->hide();
}

void CenterTopWidget::ShowWidget()
{
	m_openBtn->show();
	m_offsetBtn->show();
	m_rotateBtn->show();
	m_scaleBtn->show();

	m_supportBtn->show();
	m_supportEditBtn->show();
	m_setupBtn->show();
	m_sliceBtn->show();

	m_printerCombo->show();
	m_printerLabel->show();

	m_viewWidget->show();
}
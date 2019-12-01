#include "CenterTopWidget.h"
#include "mainwindow.h"


CenterTopWidget::CenterTopWidget(MainWindow* parent)
	:m_parent(parent)
{
	//机型选择部分
	SelectPrinterLabel = new QLabel(QStringLiteral("选择机型"), m_parent);
	SelectPrinterLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	SelectPrinterLabel->setFixedSize(90, 20);
	SelectPrinterLabel->show();

	SelectPrinterCombo = new QComboBox(m_parent);
	SelectPrinterCombo->insertItem(0, "S288");
	SelectPrinterCombo->insertItem(1, "S250");
	SelectPrinterCombo->setFixedSize(100, 20);
	//if (dlprinter->printer == S288)
		SelectPrinterCombo->setCurrentIndex(0);
	//else if (dlprinter->printer == S250)
	//	SelectPrinterCombo->setCurrentIndex(1);
	SelectPrinterCombo->show();
	connect(SelectPrinterCombo, SIGNAL(currentIndexChanged(QString)), m_parent, SLOT(dlprinterChange(QString)));

	//left
	m_openBtn = new PushButton(QIcon(":/icon/images/load_b.png"), "", m_parent);
	m_openBtn->setIconSize(QSize(120, 140));
	m_openBtn->setStyleSheet(OnButton);
	m_openBtn->setFixedSize(60, 70);
	m_openBtn->setToolTip(QStringLiteral("打开"));
	m_openBtn->show();
	connect(m_openBtn, SIGNAL(clicked()), m_parent, SLOT(openStl()));

	m_offsetBtn = new PushButton(QIcon(":/icon/images/offset_b.PNG"), "", m_parent);
	m_offsetBtn->setIconSize(QSize(90, 115));
	m_offsetBtn->setStyleSheet(OnButton);
	m_offsetBtn->setEnabled(true);
	m_offsetBtn->setFixedSize(60, 70);
	m_offsetBtn->setToolTip(QStringLiteral("移动"));
	m_offsetBtn->show();
	connect(m_offsetBtn, SIGNAL(clicked()), this, SLOT(showOffsetWidget()));

	m_scaleBtn = new PushButton(QIcon(":/icon/images/scale_b.png"), "", m_parent);
	m_scaleBtn->setIconSize(QSize(90, 115));
	m_scaleBtn->setStyleSheet(OnButton);
	m_scaleBtn->setEnabled(true);
	m_scaleBtn->setFixedSize(60, 70);
	m_scaleBtn->setToolTip(QStringLiteral("缩放"));
	m_scaleBtn->show();
	connect(m_scaleBtn, SIGNAL(clicked()), this, SLOT(showScaleWidget()));

	m_rotateBtn = new PushButton(QIcon(":/icon/images/rotate_b.png"), "", m_parent);
	m_rotateBtn->setIconSize(QSize(90, 115));
	m_rotateBtn->setStyleSheet(OnButton);
	m_rotateBtn->setEnabled(true);
	m_rotateBtn->setFixedSize(60, 70);
	m_rotateBtn->setToolTip(QStringLiteral("旋转"));
	m_rotateBtn->show();
	connect(m_rotateBtn, SIGNAL(clicked()), this, SLOT(showRotateWidget()));

	//right
	m_supportBtn = new PushButton(QIcon(":/icon/images/support_b.png"), "", m_parent);
	m_supportBtn->setIconSize(QSize(90, 115));
	m_supportBtn->setStyleSheet(OnButton);
	m_supportBtn->setFixedSize(60, 70);
	m_supportBtn->setToolTip(QStringLiteral("支撑"));
	m_supportBtn->show();
	connect(m_supportBtn, SIGNAL(clicked()), m_parent, SLOT(generateSupport()));

	m_supportEditBtn = new PushButton(QIcon(":/icon/images/supportEdit_b.png"), "", m_parent);
	m_supportEditBtn->setIconSize(QSize(90, 115));
	m_supportEditBtn->setStyleSheet(OnButton);
	m_supportEditBtn->setFixedSize(60, 70);
	m_supportEditBtn->setToolTip(QStringLiteral("支撑编辑"));
	m_supportEditBtn->show();
	connect(m_supportEditBtn, SIGNAL(clicked()), m_parent, SLOT(SupportEdit()));

	m_sliceBtn = new PushButton(QIcon(":/icon/images/slice_b.png"), "", m_parent);
	m_sliceBtn->setIconSize(QSize(90, 115));
	m_sliceBtn->setStyleSheet(OnButton);
	m_sliceBtn->setFixedSize(60, 70);
	m_sliceBtn->setToolTip(QStringLiteral("切片"));
	m_sliceBtn->show();
	connect(m_sliceBtn, SIGNAL(clicked()), m_parent, SLOT(slice()));

	m_setupBtn = new PushButton(QIcon(":/icon/images/setup_b.png"), "", m_parent);
	m_setupBtn->setIconSize(QSize(90, 115));
	m_setupBtn->setStyleSheet(OnButton);
	m_setupBtn->setFixedSize(60, 70);
	m_setupBtn->setToolTip(QStringLiteral("设置"));
	m_setupBtn->show();
	connect(m_setupBtn, SIGNAL(clicked()), m_parent, SLOT(showSetupDialog()));

	//--------------initOffsetWidget---------------
	QLabel* offset_title = new QLabel(QStringLiteral("  平  移"));
	offset_title->setStyleSheet(labelStyle);
	QLabel* x_offset_label = new QLabel("  X");
	x_offset_label->setStyleSheet("color: rgb(225,0,0); background-color: rgba(225,225,225,0);");
	QLabel* y_offset_label = new QLabel("  Y");
	y_offset_label->setStyleSheet("color: rgb(0,225,0); background-color: rgba(225,225,225,0);");
	QLabel* z_offset_label = new QLabel("  Z");
	z_offset_label->setStyleSheet("color: rgb(0,0,225); background-color: rgba(225,225,225,0);");
	x_offset_spin = new QDoubleSpinBox();
	x_offset_spin->setRange(-1000, 1000);
	x_offset_spin->setSingleStep(1);
	x_offset_spin->setStyleSheet(spinStyle);
	x_offset_spin->setSuffix("mm");
	connect(x_offset_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(xoffsetValueChange(double)));

	y_offset_spin = new QDoubleSpinBox();
	y_offset_spin->setRange(-1000, 1000);
	y_offset_spin->setSingleStep(1);
	y_offset_spin->setStyleSheet(spinStyle);
	y_offset_spin->setSuffix("mm");
	connect(y_offset_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(yoffsetValueChange(double)));

	z_offset_spin = new QDoubleSpinBox();
	z_offset_spin->setRange(-1000, 1000);
	z_offset_spin->setSingleStep(1);
	z_offset_spin->setStyleSheet(spinStyle);
	z_offset_spin->setSuffix("mm");
	connect(z_offset_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(zoffsetValueChange(double)));

	QPushButton* ZPosZeroBtn = new QPushButton(QStringLiteral("贴底"));
	connect(ZPosZeroBtn, SIGNAL(clicked()), m_parent, SLOT(ZPosZero()));

	QGridLayout* mainlayout = new QGridLayout(m_parent);
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
	QWidget* w = new QWidget();
	w->setLayout(mainlayout);
	offsetWidget = new QStackedWidget(m_parent);
	offsetWidget->setStyleSheet("background-color: rgba(225,225,225,100);");
	//offsetWidget->setMask(QPolygon(anomalyRect1));
	offsetWidget->setMinimumSize(180, 150);
	offsetWidget->addWidget(w);
	offsetWidget->hide();

	//-----------------initRotateWidget-------------------
	QLabel* rotate_title = new QLabel(QStringLiteral("  旋  转"));
	rotate_title->setStyleSheet(labelStyle);
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
	x_rotate_spin->setStyleSheet(spinStyle);
	connect(x_rotate_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(xRotateValueChange(double)));

	y_rotate_spin = new QDoubleSpinBox();
	y_rotate_spin->setRange(-360.0, 360.0);
	y_rotate_spin->setSingleStep(5.0);
	y_rotate_spin->setSuffix(QStringLiteral("°"));
	y_rotate_spin->setStyleSheet(spinStyle);
	connect(y_rotate_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(yRotateValueChange(double)));

	z_rotate_spin = new QDoubleSpinBox();
	z_rotate_spin->setRange(-360.0, 360.0);
	z_rotate_spin->setSingleStep(5.0);
	z_rotate_spin->setSuffix(QStringLiteral("°"));
	z_rotate_spin->setStyleSheet(spinStyle);
	connect(z_rotate_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(zRotateValueChange(double)));

	QGridLayout* mainlayout1 = new QGridLayout(m_parent);
	mainlayout1->setMargin(7);
	mainlayout1->setSpacing(7);
	mainlayout1->addWidget(rotate_title, 0, 1, 1, 2);
	mainlayout1->addWidget(x_rotate_label, 1, 0);
	mainlayout1->addWidget(x_rotate_spin, 1, 1, 1, 3);
	mainlayout1->addWidget(y_rotate_label, 2, 0);
	mainlayout1->addWidget(y_rotate_spin, 2, 1, 1, 3);
	mainlayout1->addWidget(z_rotate_label, 3, 0);
	mainlayout1->addWidget(z_rotate_spin, 3, 1, 1, 3);
	QWidget* w1 = new QWidget();
	w1->setLayout(mainlayout1);
	rotateWidget = new QStackedWidget(m_parent);
	rotateWidget->setStyleSheet("background-color: rgba(225,225,225,100);");
	rotateWidget->setMinimumSize(130, 100);
	//rotateWidget->setMask(QPolygon(anomalyRect1));
	rotateWidget->addWidget(w1);
	rotateWidget->hide();

	//-----------------initScaleWidget---------------------
	QLabel* scale_title = new QLabel(QStringLiteral("  缩   放"));
	scale_title->setStyleSheet(labelStyle);
	QLabel* x_scale_label = new QLabel(" X");
	x_scale_label->setStyleSheet("color: rgb(225,0,0); background-color: rgba(225,225,225,0);");
	QLabel* y_scale_label = new QLabel(" Y");
	y_scale_label->setStyleSheet("color: rgb(0,255,0); background-color: rgba(225,225,225,0);");
	QLabel* z_scale_label = new QLabel(" Z");
	z_scale_label->setStyleSheet("color: rgb(0,0,225); background-color: rgba(225,225,225,0);");
	x_scale_spin = new QDoubleSpinBox();
	x_scale_spin->setRange(1, 100000);
	x_scale_spin->setSingleStep(5);
	x_scale_spin->setStyleSheet(spinStyle);
	x_scale_spin->setSuffix("%");
	connect(x_scale_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(xScaleValueChange(double)));

	y_scale_spin = new QDoubleSpinBox();
	y_scale_spin->setRange(1, 100000);
	y_scale_spin->setSingleStep(5);
	y_scale_spin->setSuffix("%");
	y_scale_spin->setDisabled(true);
	y_scale_spin->setStyleSheet(spinStyle);
	connect(y_scale_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(yScaleValueChange(double)));

	z_scale_spin = new QDoubleSpinBox();
	z_scale_spin->setRange(1, 100000);
	z_scale_spin->setSingleStep(5);
	z_scale_spin->setSuffix("%");
	z_scale_spin->setDisabled(true);
	z_scale_spin->setStyleSheet(spinStyle);
	connect(z_scale_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(zScaleValueChange(double)));

	x_size_label = new QDoubleSpinBox();
	x_size_label->setRange(0, 1000);
	x_size_label->setDisabled(true);
	x_size_label->setSuffix("mm");
	x_size_label->setStyleSheet(spinStyle);
	//connect(x_size_spin, SIGNAL(valueChanged(double)), this, SLOT(xSizeValueChange(double)));

	y_size_label = new QDoubleSpinBox();
	y_size_label->setRange(0, 1000);
	y_size_label->setDisabled(true);
	y_size_label->setSuffix("mm");
	y_size_label->setStyleSheet(spinStyle);
	//connect(y_size_spin, SIGNAL(valueChanged(double)), this, SLOT(ySizeValueChange(double)));

	z_size_label = new QDoubleSpinBox();
	z_size_label->setRange(0, 1000);
	z_size_label->setDisabled(true);
	z_size_label->setSuffix("mm");
	z_size_label->setStyleSheet(spinStyle);
	//connect(z_size_spin, SIGNAL(valueChanged(double)), this, SLOT(zSizeValueChange(double)));

	unify_scale = new QCheckBox();
	unify_scale->setCheckState(Qt::Checked);
	unify_scale->setStyleSheet(labelStyle);
	connect(unify_scale, SIGNAL(stateChanged(int)), this, SLOT(setUnifyScale(int)));

	QLabel* unify_scale_label = new QLabel(QStringLiteral("统一缩放"));
	unify_scale_label->setStyleSheet(labelStyle);

	QGridLayout* mainlayout2 = new QGridLayout(m_parent);
	mainlayout2->setMargin(7);
	mainlayout2->setSpacing(8);
	mainlayout2->addWidget(scale_title, 0, 3, 1, 3);
	mainlayout2->addWidget(x_scale_label, 1, 1);
	mainlayout2->addWidget(x_size_label, 1, 2, 1, 3);
	mainlayout2->addWidget(x_scale_spin, 1, 5, 1, 3);
	mainlayout2->addWidget(y_scale_label, 2, 1);
	mainlayout2->addWidget(y_size_label, 2, 2, 1, 3);
	mainlayout2->addWidget(y_scale_spin, 2, 5, 1, 3);
	mainlayout2->addWidget(z_scale_label, 3, 1);
	mainlayout2->addWidget(z_size_label, 3, 2, 1, 3);
	mainlayout2->addWidget(z_scale_spin, 3, 5, 1, 3);
	mainlayout2->addWidget(unify_scale, 4, 5);
	mainlayout2->addWidget(unify_scale_label, 4, 6, 1, 2);
	QWidget* w2 = new QWidget();
	w2->setLayout(mainlayout2);
	scaleWidget = new QStackedWidget(m_parent);
	scaleWidget->setStyleSheet("background-color: rgba(225,225,225,100);");
	scaleWidget->setMinimumSize(200, 140);
	//scaleWidget->setMask(QPolygon(anomalyRect2));
	scaleWidget->addWidget(w2);
	scaleWidget->hide();

	//-------------------initProgress-------------------
	progressLabel = new QLabel;
	QFont font("ZYSong18030", 15);
	progressLabel->setFont(font);
	progressLabel->setStyleSheet("color: white");
	progressLabel->setText("waiting ......");
	progress = new QProgressBar;
	progress->setOrientation(Qt::Horizontal);

	QString strStyle = "QProgressBar {border-radius: 5px ; text-align: center; background: rgb(200, 200, 200);}"
		"QProgressBar::chunk {border-radius:5px;border:1px solid black;background: rgb(200,50,0)}";
	progress->setStyleSheet(strStyle);
	progress->setRange(0, 100);
	QGridLayout* layout = new QGridLayout();
	layout->setMargin(10);
	layout->setSpacing(2);
	layout->addWidget(progressLabel, 0, 2);
	layout->addWidget(progress, 1, 0, 2, 5);
	QWidget* widget = new QWidget();
	widget->setLayout(layout);
	progressWidget = new QStackedWidget(m_parent);
	progressWidget->setStyleSheet("background-color: rgba(100,100,100,225);");
	progressWidget->setMask(QPolygon(progressRect));
	progressWidget->setMinimumSize(QSize(300, 80));
	progressWidget->addWidget(widget);
	progressWidget->hide();
}

CenterTopWidget::~CenterTopWidget()
{

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
			offsetWidget->hide();
		else {
			//if (glwidget->selectID >= 0)
			//	setOffsetValue(interface1->find_instance(glwidget->selectID));
			offsetWidget->show();
		}

		m_offsetBtn->push();
		break;
	case SCALEBTN:
		temp = m_scaleBtn;
		if (m_scaleBtn->c_isChecked())
			scaleWidget->hide();
		else {
			//if (glwidget->selectID >= 0) {
			//	ModelInstance* i = interface1->find_instance(glwidget->selectID);
			//	setScaleValue(i);
			//	setSizeValue(i);
			//}
			scaleWidget->show();
		}

		m_scaleBtn->push();
		break;
	case ROTATEBTN:
		temp = m_rotateBtn;
		if (m_rotateBtn->c_isChecked()) {
			init_rotate();
			rotateWidget->hide();
		}
		else {
			init_rotate();
			rotateWidget->show();
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


void CenterTopWidget::resize(QSize size)
{
	int w = size.width();
	int h = size.height();

	SelectPrinterLabel->move(w / 2 - 110, 30);
	SelectPrinterCombo->move(w / 2 - 20, 30);

	//left
	m_openBtn->move(10, h / 2 - 240);
	m_offsetBtn->move(10, h / 2 - 160);
	m_scaleBtn->move(10, h / 2 - 80);
	m_rotateBtn->move(10, h / 2);

	//rigft
	m_supportBtn->move(w - 70, h / 2 - 240);
	m_supportEditBtn->move(w - 70, h / 2 - 160);
	m_sliceBtn->move(w - 70, h / 2 - 80);
	m_setupBtn->move(w - 70, h / 2);

	offsetWidget->move(75, h / 2 - 160 - 10);
	rotateWidget->move(75, h / 2 - 10);
	scaleWidget->move(75, h / 2 - 80 - 30);
	progressWidget->move(w / 2 - 150, h / 2 - 50);
}

void CenterTopWidget::initOffsetWidget()
{
	QLabel* offset_title = new QLabel(QStringLiteral("  平  移"));
	offset_title->setStyleSheet(labelStyle);
	QLabel* x_offset_label = new QLabel("  X");
	x_offset_label->setStyleSheet("color: rgb(225,0,0); background-color: rgba(225,225,225,0);");
	QLabel* y_offset_label = new QLabel("  Y");
	y_offset_label->setStyleSheet("color: rgb(0,225,0); background-color: rgba(225,225,225,0);");
	QLabel* z_offset_label = new QLabel("  Z");
	z_offset_label->setStyleSheet("color: rgb(0,0,225); background-color: rgba(225,225,225,0);");
	x_offset_spin = new QDoubleSpinBox();
	x_offset_spin->setRange(-1000, 1000);
	x_offset_spin->setSingleStep(1);
	x_offset_spin->setStyleSheet(spinStyle);
	x_offset_spin->setSuffix("mm");
	connect(x_offset_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(xoffsetValueChange(double)));

	y_offset_spin = new QDoubleSpinBox();
	y_offset_spin->setRange(-1000, 1000);
	y_offset_spin->setSingleStep(1);
	y_offset_spin->setStyleSheet(spinStyle);
	y_offset_spin->setSuffix("mm");
	connect(y_offset_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(yoffsetValueChange(double)));

	z_offset_spin = new QDoubleSpinBox();
	z_offset_spin->setRange(-1000, 1000);
	z_offset_spin->setSingleStep(1);
	z_offset_spin->setStyleSheet(spinStyle);
	z_offset_spin->setSuffix("mm");
	connect(z_offset_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(zoffsetValueChange(double)));

	QPushButton* ZPosZeroBtn = new QPushButton(QStringLiteral("贴底"));
	connect(ZPosZeroBtn, SIGNAL(clicked()), m_parent, SLOT(ZPosZero()));

	QGridLayout* mainlayout = new QGridLayout(m_parent);
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
	QWidget* w = new QWidget();
	w->setLayout(mainlayout);
	offsetWidget = new QStackedWidget(m_parent);
	offsetWidget->setStyleSheet("background-color: rgba(225,225,225,100);");
	//offsetWidget->setMask(QPolygon(anomalyRect1));
	offsetWidget->setMinimumSize(180, 150);
	offsetWidget->addWidget(w);
	offsetWidget->hide();
}

void CenterTopWidget::initRotateWidget()
{
	QLabel* rotate_title = new QLabel(QStringLiteral("  旋  转"));
	rotate_title->setStyleSheet(labelStyle);
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
	x_rotate_spin->setStyleSheet(spinStyle);
	connect(x_rotate_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(xRotateValueChange(double)));

	y_rotate_spin = new QDoubleSpinBox();
	y_rotate_spin->setRange(-360.0, 360.0);
	y_rotate_spin->setSingleStep(5.0);
	y_rotate_spin->setSuffix(QStringLiteral("°"));
	y_rotate_spin->setStyleSheet(spinStyle);
	connect(y_rotate_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(yRotateValueChange(double)));

	z_rotate_spin = new QDoubleSpinBox();
	z_rotate_spin->setRange(-360.0, 360.0);
	z_rotate_spin->setSingleStep(5.0);
	z_rotate_spin->setSuffix(QStringLiteral("°"));
	z_rotate_spin->setStyleSheet(spinStyle);
	connect(z_rotate_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(zRotateValueChange(double)));

	QGridLayout* mainlayout = new QGridLayout(m_parent);
	mainlayout->setMargin(7);
	mainlayout->setSpacing(7);
	mainlayout->addWidget(rotate_title, 0, 1, 1, 2);
	mainlayout->addWidget(x_rotate_label, 1, 0);
	mainlayout->addWidget(x_rotate_spin, 1, 1, 1, 3);
	mainlayout->addWidget(y_rotate_label, 2, 0);
	mainlayout->addWidget(y_rotate_spin, 2, 1, 1, 3);
	mainlayout->addWidget(z_rotate_label, 3, 0);
	mainlayout->addWidget(z_rotate_spin, 3, 1, 1, 3);
	QWidget* w = new QWidget();
	w->setLayout(mainlayout);
	rotateWidget = new QStackedWidget(m_parent);
	rotateWidget->setStyleSheet("background-color: rgba(225,225,225,100);");
	rotateWidget->setMinimumSize(130, 100);
	//rotateWidget->setMask(QPolygon(anomalyRect1));
	rotateWidget->addWidget(w);
	rotateWidget->hide();
}

void CenterTopWidget::initScaleWidget()
{
	QLabel* scale_title = new QLabel(QStringLiteral("  缩   放"));
	scale_title->setStyleSheet(labelStyle);
	QLabel* x_scale_label = new QLabel(" X");
	x_scale_label->setStyleSheet("color: rgb(225,0,0); background-color: rgba(225,225,225,0);");
	QLabel* y_scale_label = new QLabel(" Y");
	y_scale_label->setStyleSheet("color: rgb(0,255,0); background-color: rgba(225,225,225,0);");
	QLabel* z_scale_label = new QLabel(" Z");
	z_scale_label->setStyleSheet("color: rgb(0,0,225); background-color: rgba(225,225,225,0);");
	x_scale_spin = new QDoubleSpinBox();
	x_scale_spin->setRange(1, 100000);
	x_scale_spin->setSingleStep(5);
	x_scale_spin->setStyleSheet(spinStyle);
	x_scale_spin->setSuffix("%");
	connect(x_scale_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(xScaleValueChange(double)));

	y_scale_spin = new QDoubleSpinBox();
	y_scale_spin->setRange(1, 100000);
	y_scale_spin->setSingleStep(5);
	y_scale_spin->setSuffix("%");
	y_scale_spin->setDisabled(true);
	y_scale_spin->setStyleSheet(spinStyle);
	connect(y_scale_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(yScaleValueChange(double)));

	z_scale_spin = new QDoubleSpinBox();
	z_scale_spin->setRange(1, 100000);
	z_scale_spin->setSingleStep(5);
	z_scale_spin->setSuffix("%");
	z_scale_spin->setDisabled(true);
	z_scale_spin->setStyleSheet(spinStyle);
	connect(z_scale_spin, SIGNAL(valueChanged(double)), m_parent, SLOT(zScaleValueChange(double)));

	x_size_label = new QDoubleSpinBox();
	x_size_label->setRange(0, 1000);
	x_size_label->setDisabled(true);
	x_size_label->setSuffix("mm");
	x_size_label->setStyleSheet(spinStyle);
	//connect(x_size_spin, SIGNAL(valueChanged(double)), this, SLOT(xSizeValueChange(double)));

	y_size_label = new QDoubleSpinBox();
	y_size_label->setRange(0, 1000);
	y_size_label->setDisabled(true);
	y_size_label->setSuffix("mm");
	y_size_label->setStyleSheet(spinStyle);
	//connect(y_size_spin, SIGNAL(valueChanged(double)), this, SLOT(ySizeValueChange(double)));

	z_size_label = new QDoubleSpinBox();
	z_size_label->setRange(0, 1000);
	z_size_label->setDisabled(true);
	z_size_label->setSuffix("mm");
	z_size_label->setStyleSheet(spinStyle);
	//connect(z_size_spin, SIGNAL(valueChanged(double)), this, SLOT(zSizeValueChange(double)));

	unify_scale = new QCheckBox();
	unify_scale->setCheckState(Qt::Checked);
	unify_scale->setStyleSheet(labelStyle);
	connect(unify_scale, SIGNAL(stateChanged(int)), this, SLOT(setUnifyScale(int)));

	QLabel* unify_scale_label = new QLabel(QStringLiteral("统一缩放"));
	unify_scale_label->setStyleSheet(labelStyle);

	QGridLayout* mainlayout = new QGridLayout(m_parent);
	mainlayout->setMargin(7);
	mainlayout->setSpacing(8);
	mainlayout->addWidget(scale_title, 0, 3, 1, 3);
	mainlayout->addWidget(x_scale_label, 1, 1);
	mainlayout->addWidget(x_size_label, 1, 2, 1, 3);
	mainlayout->addWidget(x_scale_spin, 1, 5, 1, 3);
	mainlayout->addWidget(y_scale_label, 2, 1);
	mainlayout->addWidget(y_size_label, 2, 2, 1, 3);
	mainlayout->addWidget(y_scale_spin, 2, 5, 1, 3);
	mainlayout->addWidget(z_scale_label, 3, 1);
	mainlayout->addWidget(z_size_label, 3, 2, 1, 3);
	mainlayout->addWidget(z_scale_spin, 3, 5, 1, 3);
	mainlayout->addWidget(unify_scale, 4, 5);
	mainlayout->addWidget(unify_scale_label, 4, 6, 1, 2);
	QWidget* w = new QWidget();
	w->setLayout(mainlayout);
	scaleWidget = new QStackedWidget(m_parent);
	scaleWidget->setStyleSheet("background-color: rgba(225,225,225,100);");
	scaleWidget->setMinimumSize(200, 140);
	//scaleWidget->setMask(QPolygon(anomalyRect2));
	scaleWidget->addWidget(w);
	scaleWidget->hide();
}

void CenterTopWidget::initProgress()
{
	progressLabel = new QLabel;
	QFont font("ZYSong18030", 15);
	progressLabel->setFont(font);
	progressLabel->setStyleSheet("color: white");
	progressLabel->setText("waiting ......");
	progress = new QProgressBar;
	progress->setOrientation(Qt::Horizontal);

	QString strStyle = "QProgressBar {border-radius: 5px ; text-align: center; background: rgb(200, 200, 200);}"
		"QProgressBar::chunk {border-radius:5px;border:1px solid black;background: rgb(200,50,0)}";
	progress->setStyleSheet(strStyle);
	progress->setRange(0, 100);
	QGridLayout* layout = new QGridLayout();
	layout->setMargin(10);
	layout->setSpacing(2);
	layout->addWidget(progressLabel, 0, 2);
	layout->addWidget(progress, 1, 0, 2, 5);
	QWidget* widget = new QWidget();
	widget->setLayout(layout);
	progressWidget = new QStackedWidget(m_parent);
	progressWidget->setStyleSheet("background-color: rgba(100,100,100,225);");
	progressWidget->setMask(QPolygon(progressRect));
	progressWidget->setMinimumSize(QSize(300, 80));
	progressWidget->addWidget(widget);
	progressWidget->hide();
}

void CenterTopWidget::P(size_t i)
{
	progress->setValue(i);
	QCoreApplication::processEvents();
}

void CenterTopWidget::showProgress(int btn)
{
	switch (btn)
	{
	case OPENBTN:
		progressLabel->setText(QStringLiteral("读取模型"));
		break;
	case SUPPORTBTN:
		progressLabel->setText(QStringLiteral("生成支撑"));
		break;
	case SLICEBTN:
		progressLabel->setText(QStringLiteral("切片"));
		break;
	case SUPPORTEDITBTN:
		progressLabel->setText(QStringLiteral("重新生成支撑"));

	default:
		break;
	}
	progressWidget->show();
	progress->reset();
	P(0);
}

void CenterTopWidget::hideProgress()
{
	progressWidget->hide();
}

void CenterTopWidget::init_rotate()
{
	x_rotate = 0;
	y_rotate = 0;
	z_rotate = 0;
	x_rotate_spin->setValue(x_rotate);
	y_rotate_spin->setValue(y_rotate);
	z_rotate_spin->setValue(z_rotate);
}

void CenterTopWidget::showOffsetWidget()
{
	CenterButtonPush(OFFSETBTN);
}

void CenterTopWidget::showRotateWidget()
{
	CenterButtonPush(ROTATEBTN);
}

void CenterTopWidget::showScaleWidget()
{
	CenterButtonPush(SCALEBTN);
}

void CenterTopWidget::setUnifyScale(int state)
{
	switch (state)
	{
	case Qt::Unchecked:
		y_scale_spin->setEnabled(true);
		z_scale_spin->setEnabled(true);
		//y_size_spin->setEnabled(true);
		//z_size_spin->setEnabled(true);
		break;

	case Qt::Checked:
		y_scale_spin->setDisabled(true);
		z_scale_spin->setDisabled(true);
		//y_size_spin->setDisabled(true);
		//z_size_spin->setDisabled(true);
		break;

	default:
		break;
	}
}
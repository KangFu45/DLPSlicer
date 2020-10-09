#include "SetupDialog.h"
#include "mainwindow.h"

#include <qsettings.h>
#include <qfile.h>
#include <qstandardpaths.h>
#include <qdir.h>

#include "Setting.h"

extern Setting e_setting;

//---------------Config--------------

void Config::writeConfig()
{
	QSettings writeini(e_setting.ConfigFile.c_str(), QSettings::IniFormat);
	writeini.clear();
	writeini.setValue("/illu_config/normIllu", normIlluTime);
	writeini.setValue("/illu_config/normInttersity", norm_inttersity);
	writeini.setValue("/illu_config/overIllu", overIlluTime);
	writeini.setValue("/illu_config/overInttersity", over_inttersity);
	writeini.setValue("/illu_config/overLayer", overLayer);

	writeini.setValue("/support_config/topHeight", support_top_height);
	writeini.setValue("/support_config/topRadius", support_top_radius);
	writeini.setValue("/support_config/supportRadius", support_radius);
	writeini.setValue("/support_config/bottomRadius", support_bottom_radius);
	writeini.setValue("/support_config/supportSpace", space);
	writeini.setValue("/support_config/supportAngle", angle);
	writeini.setValue("/support_config/leafNum", leaf_num);
	writeini.setValue("/support_config/modelLift", model_lift);

	writeini.setValue("/hollow_out_config/hollowOutBox", hollow_out);
	writeini.setValue("/hollow_out_config/fillPattern", fill_pattern);
	writeini.setValue("/hollow_out_config/wallThickness", wall_thickness);
	writeini.setValue("/hollow_out_config/fillDensity", fill_density);

	writeini.setValue("/other/thickness", layer_height);
	writeini.setValue("/other/raft", raft_layers);
	writeini.setValue("/other/raftOffset", raft_offset);
	writeini.setValue("/other/arrangeSpace", arrange_space);
	writeini.setValue("/other/thread", threads);
}

//------------------SetupDialog---------------

SetupDialog::SetupDialog(Config* config)
	:m_config(config)
{
	{
		QFile qssfile(":/icon/base.qss");
		qssfile.open(QFile::ReadOnly);
		QString qss;
		qss = qssfile.readAll();
		this->setStyleSheet(qss);
	}

	initLayout();
	readConfig();

	resize(minimumSize());
	setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
	setWindowTitle(QStringLiteral("����"));
	setWindowIcon(QIcon(":/icon/images/printer.ico"));
}

SetupDialog::~SetupDialog()
{
	delete normIlluSpin;
	delete normIlluLabel;
	delete norm_inttersity_spin;
	delete norm_inttersity_label;
	delete overIlluSpin;
	delete overIlluLabel;
	delete over_inttersity_spin;
	delete over_inttersity_label;
	delete overLayerSpin;
	delete overLayerLabel;

	delete illuLayout;
	delete illuWidget;

	delete top_height_spin;
	delete top_height_label;
	delete top_radius_spin;
	delete top_radius_label;
	delete support_radius_spin;
	delete support_radius_label;
	delete bottom_radius_spin;
	delete bottom_radius_label;
	delete support_space_spin;
	delete support_space_label;
	delete support_angle_spin;
	delete support_angle_label;
	delete leaf_num_spin;
	delete leaf_num_label;
	delete model_lift_spin;
	delete model_lift_label;

	delete supportLayout;
	delete supportWidget;

	delete hollow_out_box;
	delete hollow_out_label;
	delete fill_pattern_combo;
	delete fillPatternLabel;
	delete wall_thickness_spin;
	delete wall_thickness_label;
	delete density_spin;
	delete density_label;

	delete hollowOutLayout;
	delete hollowOutWidget;

	delete thicknessCombo;
	delete thicknessLabel;
	delete raftSpin;
	delete raftLabel;
	delete arrange_space_spin;
	delete arrangeSpaceLabel;
	delete threadSpin;
	delete threadLabel;

	delete otherLayourt;
	delete otherWidget;

	delete setupTab;

	delete defultBtn;
	delete cancalBtn;
	delete okBtn;
	delete mainLayout;
}

void SetupDialog::initLayout()
{
	//��������
	normIlluSpin = new QSpinBox();
	normIlluSpin->setRange(5, 10);
	normIlluSpin->setSuffix("s");
	normIlluLabel = new QLabel(QStringLiteral("�����ع�ʱ��"));

	norm_inttersity_spin = new QSpinBox();
	norm_inttersity_spin->setRange(50, 100);
	norm_inttersity_spin->setSuffix("%");
	norm_inttersity_label = new QLabel(QStringLiteral("�����ع�ǿ��"));

	overIlluSpin = new QSpinBox();
	overIlluSpin->setRange(5, 10);
	overIlluSpin->setSuffix("s");
	overIlluLabel = new QLabel(QStringLiteral("���ع�ʱ��"));
	overIlluLabel->setToolTip(QStringLiteral("ǰ������Ҫ���ع��ʱ��"));

	over_inttersity_spin = new QSpinBox();
	over_inttersity_spin->setRange(50, 100);
	over_inttersity_spin->setSuffix("%");
	over_inttersity_label = new QLabel(QStringLiteral("���ع�ǿ��"));
	over_inttersity_label->setToolTip(QStringLiteral("ǰ������Ҫ���ع�Ĺ���ǿ��"));

	overLayerSpin = new QSpinBox();
	overLayerSpin->setRange(0, 10);
	overLayerSpin->setSuffix(QStringLiteral("��"));
	overLayerLabel = new QLabel(QStringLiteral("���ع����"));
	overLayerLabel->setToolTip(QStringLiteral("ǰ������Ҫ���ع�Ĳ���"));

	illuLayout = new QGridLayout();
	illuLayout->setMargin(10);
	illuLayout->setSpacing(10);
	illuLayout->addWidget(normIlluLabel, 0, 0);
	illuLayout->addWidget(normIlluSpin, 0, 1);
	illuLayout->addWidget(norm_inttersity_label, 1, 0);
	illuLayout->addWidget(norm_inttersity_spin, 1, 1);
	illuLayout->addWidget(overIlluLabel, 2, 0);
	illuLayout->addWidget(overIlluSpin, 2, 1);
	illuLayout->addWidget(over_inttersity_label, 3, 0);
	illuLayout->addWidget(over_inttersity_spin, 3, 1);
	illuLayout->addWidget(overLayerLabel, 4, 0);
	illuLayout->addWidget(overLayerSpin, 4, 1);
	illuWidget = new QWidget(this);
	illuWidget->setLayout(illuLayout);

	//֧������
	top_height_spin = new QDoubleSpinBox();
	top_height_spin->setRange(1, 5);
	top_height_spin->setSingleStep(1);
	top_height_spin->setSuffix("mm");
	top_height_label = new QLabel(QStringLiteral("���˸߶�"));

	top_radius_spin = new QDoubleSpinBox();
	top_radius_spin->setRange(0.5, 2);
	top_radius_spin->setSingleStep(0.1);
	top_radius_spin->setSuffix("mm");
	top_radius_label = new QLabel(QStringLiteral("����ֱ��"));
	top_radius_label->setToolTip(QStringLiteral("����֧�Ŷ�����ģ�ͽӴ������С"));

	support_radius_spin = new QDoubleSpinBox();
	support_radius_spin->setRange(0.5, 3);
	support_radius_spin->setSingleStep(0.1);
	support_radius_spin->setSuffix("mm");
	support_radius_label = new QLabel(QStringLiteral("֧��ֱ��"));
	support_radius_label->setToolTip(QStringLiteral("֧�������˲��ֵĴ�ϸ�̶�"));

	bottom_radius_spin = new QDoubleSpinBox();
	bottom_radius_spin->setRange(0.5, 4);
	bottom_radius_spin->setSingleStep(0.1);
	bottom_radius_spin->setSuffix("mm");
	bottom_radius_label = new QLabel(QStringLiteral("�׶�ֱ��"));
	bottom_radius_label->setToolTip(QStringLiteral("һ���ڵװ��ϵ�֧�����׶˵Ĵ�ϸ�̶�"));

	support_space_spin = new QDoubleSpinBox();
	support_space_spin->setRange(2., 20.);
	support_space_spin->setSingleStep(0.5);
	support_space_spin->setSuffix("mm");
	support_space_label = new QLabel(QStringLiteral("֧�ż��"));
	support_space_label->setToolTip(QStringLiteral("֧���������֧�����ļ��"));

	support_angle_spin = new QSpinBox();
	support_angle_spin->setRange(0, 90);
	support_angle_spin->setSingleStep(5);
	support_angle_spin->setSuffix(QStringLiteral("��"));
	support_angle_label = new QLabel(QStringLiteral("֧�ŽǶ�"));
	support_angle_label->setToolTip(QStringLiteral("������Ƭ��ƽ̨�ļнǵ����趨֧�ŽǶȵ�������Ƭ�ᱻ֧��"));

	leaf_num_spin = new QSpinBox();
	leaf_num_spin->setRange(1, 100);
	leaf_num_spin->setSingleStep(1);
	leaf_num_spin->setSuffix(QStringLiteral("��"));
	leaf_num_label = new QLabel(QStringLiteral("��֦����"));
	leaf_num_label->setToolTip(QStringLiteral("��״֧����һ������֦���������"));

	model_lift_spin = new QDoubleSpinBox();
	model_lift_spin->setRange(0, 10);
	model_lift_spin->setSingleStep(1);
	model_lift_spin->setSuffix(tr("mm"));
	model_lift_label = new QLabel(QStringLiteral("ģ������"));
	model_lift_label->setToolTip(QStringLiteral("�Զ�֧��ʱģ�������߶�"));

	supportLayout = new QGridLayout();
	supportLayout->setMargin(10);
	supportLayout->setSpacing(10);
	supportLayout->addWidget(top_height_label, 0, 0);
	supportLayout->addWidget(top_height_spin, 0, 1);
	supportLayout->addWidget(top_radius_label, 1, 0);
	supportLayout->addWidget(top_radius_spin, 1, 1);
	supportLayout->addWidget(support_radius_label, 2, 0);
	supportLayout->addWidget(support_radius_spin, 2, 1);
	supportLayout->addWidget(bottom_radius_label, 3, 0);
	supportLayout->addWidget(bottom_radius_spin, 3, 1);
	supportLayout->addWidget(support_space_label, 4, 0);
	supportLayout->addWidget(support_space_spin, 4, 1);
	supportLayout->addWidget(support_angle_label, 5, 0);
	supportLayout->addWidget(support_angle_spin, 5, 1);
	supportLayout->addWidget(leaf_num_label, 6, 0);
	supportLayout->addWidget(leaf_num_spin, 6, 1);
	supportLayout->addWidget(model_lift_label, 7, 0);
	supportLayout->addWidget(model_lift_spin, 7, 1);
	supportWidget = new QFrame(this);
	supportWidget->setLayout(supportLayout);

	//�������
	hollow_out_box = new QCheckBox();
	hollow_out_label = new QLabel(QStringLiteral("��        ��"));
	hollow_out_label->setToolTip(QStringLiteral("����ϴ��ģ���ڲ��Ƿ���Ҫ���"));

	fill_pattern_combo = new QComboBox();
	fill_pattern_combo->insertItem(0, QStringLiteral("�������"));
	fill_pattern_combo->insertItem(1, QStringLiteral("3D֧�����"));
	fillPatternLabel = new QLabel(QStringLiteral("�������"));

	wall_thickness_spin = new QDoubleSpinBox();
	wall_thickness_spin->setRange(1, 5);
	wall_thickness_spin->setSuffix("mm");
	wall_thickness_label = new QLabel(QStringLiteral("��        ��"));
	wall_thickness_label->setToolTip(QStringLiteral("���ģ�͵ıں�"));

	density_spin = new QSpinBox();
	density_spin->setRange(0, 100);
	density_spin->setSingleStep(5);
	density_spin->setSuffix("%");
	density_label = new QLabel(QStringLiteral("����ܶ�"));

	hollowOutLayout = new QGridLayout();
	hollowOutLayout->setMargin(10);
	hollowOutLayout->setSpacing(10);
	hollowOutLayout->addWidget(hollow_out_label, 0, 0);
	hollowOutLayout->addWidget(hollow_out_box, 0, 1);
	hollowOutLayout->addWidget(fillPatternLabel, 1, 0);
	hollowOutLayout->addWidget(fill_pattern_combo, 1, 1);
	hollowOutLayout->addWidget(wall_thickness_label, 2, 0);
	hollowOutLayout->addWidget(wall_thickness_spin, 2, 1);
	hollowOutLayout->addWidget(density_label, 3, 0);
	hollowOutLayout->addWidget(density_spin, 3, 1);
	hollowOutWidget = new QFrame(this);
	hollowOutWidget->setLayout(hollowOutLayout);

	//��������
	thicknessCombo = new QComboBox();
	thicknessCombo->insertItem(0, "0.1mm");
	thicknessCombo->insertItem(1, "0.05mm");
	thicknessCombo->insertItem(2, "0.025mm");
	thicknessLabel = new QLabel(QStringLiteral("��    ��"));
	thicknessLabel->setToolTip(QStringLiteral("ģ����Ƭ�Ĳ��"));

	raftSpin = new QSpinBox();
	raftSpin->setRange(0, 10);
	raftSpin->setSuffix(QStringLiteral("��"));
	raftLabel = new QLabel(QStringLiteral("�׷�����"));
	raftLabel->setToolTip(QStringLiteral("ģ�͵ײ��ĵװ����(�װ�߶�����Բ��ֵ)"));

	raftOffsetSpin = new QSpinBox();
	raftOffsetSpin->setRange(0, 100);
	raftOffsetSpin->setSuffix("mm");
	raftOffsetLabel = new QLabel(QStringLiteral("�װ岹��"));

	arrange_space_spin = new QDoubleSpinBox();
	arrange_space_spin->setRange(0, 10);
	arrange_space_spin->setSuffix("mm");
	arrangeSpaceLabel = new QLabel(QStringLiteral("�Զ����м��"));

	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);

	threadSpin = new QSpinBox();
	threadSpin->setRange(1, systemInfo.dwNumberOfProcessors);//��ȡ�������߳���
	threadLabel = new QLabel(QStringLiteral("�߳���"));
	threadLabel->setToolTip(QStringLiteral("ִ�ж��߳�����ʱ���õ����߳���������߳���Ϊ���Դ������߳�����"));

	otherLayourt = new QGridLayout();
	otherLayourt->setMargin(10);
	otherLayourt->setSpacing(10);
	otherLayourt->addWidget(thicknessLabel, 0, 0);
	otherLayourt->addWidget(thicknessCombo, 0, 1);
	otherLayourt->addWidget(raftLabel, 1, 0);
	otherLayourt->addWidget(raftSpin, 1, 1);
	otherLayourt->addWidget(raftOffsetLabel, 2, 0);
	otherLayourt->addWidget(raftOffsetSpin, 2, 1);
	otherLayourt->addWidget(arrangeSpaceLabel, 3, 0);
	otherLayourt->addWidget(arrange_space_spin, 3, 1);
	otherLayourt->addWidget(threadLabel, 4, 0);
	otherLayourt->addWidget(threadSpin, 4, 1);
	otherWidget = new QFrame(this);
	otherWidget->setLayout(otherLayourt);

	setupTab = new QTabWidget();
	setupTab->addTab(supportWidget, QStringLiteral("֧������"));
	setupTab->addTab(hollowOutWidget, QStringLiteral("�������"));
	setupTab->addTab(illuWidget, QStringLiteral("��������"));
	setupTab->addTab(otherWidget, QStringLiteral("��������"));

	defultBtn = new QPushButton(QStringLiteral("Ĭ������"));
	(void)connect(defultBtn, SIGNAL(clicked()), this, SLOT(slot_setDefultValue()));

	cancalBtn = new QPushButton(QStringLiteral("ȡ��"));
	(void)connect(cancalBtn, SIGNAL(clicked()), this, SLOT(close()));

	okBtn = new QPushButton(QStringLiteral("ȷ��"));
	(void)connect(okBtn, SIGNAL(clicked()), this, SLOT(slot_writeConfig()));
	(void)connect(okBtn, SIGNAL(clicked()), this, SLOT(close()));

	mainLayout = new QGridLayout(this);
	mainLayout->addWidget(setupTab, 0, 0, 6, 10);
	mainLayout->addWidget(defultBtn, 7, 0);
	mainLayout->addWidget(cancalBtn, 7, 8);
	mainLayout->addWidget(okBtn, 7, 9);
	mainLayout->setMargin(10);
	mainLayout->setSpacing(10);

	this->setLayout(mainLayout);
}

void SetupDialog::slot_writeConfig()
{
	m_config->normIlluTime = normIlluSpin->value();
	m_config->norm_inttersity = norm_inttersity_spin->value();
	m_config->overIlluTime = overIlluSpin->value();
	m_config->over_inttersity = over_inttersity_spin->value();
	m_config->overLayer = overLayerSpin->value();

	m_config->support_top_height = top_height_spin->value();
	m_config->support_top_radius = top_radius_spin->value();
	m_config->support_radius = support_radius_spin->value();
	m_config->support_bottom_radius = bottom_radius_spin->value();
	m_config->space = support_space_spin->value();
	m_config->angle = support_angle_spin->value();
	m_config->leaf_num = leaf_num_spin->value();
	m_config->model_lift = model_lift_spin->value();

	m_config->hollow_out = int(hollow_out_box->isChecked());
	if (fill_pattern_combo->currentIndex() == 0)
		m_config->fill_pattern = Config::ipHoneycomb;
	else if (fill_pattern_combo->currentIndex() == 1)
		m_config->fill_pattern = Config::ip3DSupport;
	m_config->wall_thickness = wall_thickness_spin->value();
	m_config->fill_density = density_spin->value();

	if (thicknessCombo->currentText() == "0.05mm")
		m_config->layer_height = 0.05;
	else if (thicknessCombo->currentText() == "0.1mm")
		m_config->layer_height = 0.1;
	else if (thicknessCombo->currentText() == "0.025mm")
		m_config->layer_height = 0.025;
	m_config->raft_layers = raftSpin->value();
	m_config->raft_offset = raftOffsetSpin->value();
	m_config->arrange_space = arrange_space_spin->value();
	m_config->threads = threadSpin->value();

	m_config->writeConfig();
}

void SetupDialog::slot_setDefultValue()
{
	normIlluSpin->setValue(6);
	norm_inttersity_spin->setValue(80);
	overIlluSpin->setValue(10);
	over_inttersity_spin->setValue(80);
	overLayerSpin->setValue(5);

	top_height_spin->setValue(3);
	top_radius_spin->setValue(1);
	support_radius_spin->setValue(1.5);
	bottom_radius_spin->setValue(2);
	support_space_spin->setValue(10);
	support_angle_spin->setValue(45);
	leaf_num_spin->setValue(5);
	model_lift_spin->setValue(4);

	hollow_out_box->setChecked(false);
	fill_pattern_combo->setCurrentIndex(0);
	wall_thickness_spin->setValue(3);
	density_spin->setValue(20);

	thicknessCombo->setCurrentIndex(0);
	raftSpin->setValue(10);
	raftOffsetSpin->setValue(5);
	arrange_space_spin->setValue(5);
	threadSpin->setValue(threadSpin->maximum());

	m_config->writeConfig();
}

void SetupDialog::readConfig()
{
	if (QFile(e_setting.ConfigFile.c_str()).exists()) {
		QSettings readini(e_setting.ConfigFile.c_str(), QSettings::IniFormat);
		//��ȡ��ӡ����
		normIlluSpin->setValue(readini.value("/illu_config/normIllu").toInt());

		norm_inttersity_spin->setValue(readini.value("/illu_config/normInttersity").toInt());

		overIlluSpin->setValue(readini.value("/illu_config/overIllu").toInt());

		over_inttersity_spin->setValue(readini.value("/illu_config/overInttersity").toInt());

		overLayerSpin->setValue(readini.value("/illu_config/overLayer").toInt());

		top_height_spin->setValue(readini.value("/support_config/topHeight").toDouble());

		top_radius_spin->setValue(readini.value("/support_config/topRadius").toDouble());

		support_radius_spin->setValue(readini.value("/support_config/supportRadius").toDouble());

		bottom_radius_spin->setValue(readini.value("/support_config/bottomRadius").toDouble());

		support_space_spin->setValue(readini.value("/support_config/supportSpace").toDouble());

		support_angle_spin->setValue(readini.value("/support_config/supportAngle").toInt());

		leaf_num_spin->setValue(readini.value("/support_config/leafNum").toInt());

		model_lift_spin->setValue(readini.value("/support_config/modelLift").toDouble());

		hollow_out_box->setChecked(readini.value("/hollow_out_config/hollowOutBox").toBool());

		fill_pattern_combo->setCurrentIndex(readini.value("/hollow_out_config/fillPattern").toInt());

		wall_thickness_spin->setValue(readini.value("/hollow_out_config/wallThickness").toDouble());

		density_spin->setValue(readini.value("/hollow_out_config/fillDensity").toDouble());

		thicknessCombo->setCurrentIndex(readini.value("/other/thickness").toInt());

		raftSpin->setValue(readini.value("/other/raft").toInt());

		raftOffsetSpin->setValue(readini.value("/other/raftOffset").toInt());

		arrange_space_spin->setValue(readini.value("/other/arrangeSpace").toDouble());

		threadSpin->setValue(readini.value("/other/thread").toInt());
	}
	else
		this->slot_setDefultValue();
}
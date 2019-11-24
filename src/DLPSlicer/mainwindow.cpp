#include "mainwindow.h"
#include "qmenubar.h"
#include "qevent.h"
#include "qdebug.h"
#include "qregion.h"
#include "qsettings.h"
#include "quazip/JlCompress.h"
#include <qplaintextedit.h>
#include <QInputDialog>
#include "qtimer.h"
#include "tool.h"

PixItem::PixItem(QPixmap* pixmap)
{
	pix = *pixmap;
}

QRectF PixItem::boundingRect() const
{
	return QRectF(-2 - pix.width() / 2, -2 - pix.height() / 2, pix.width() + 4, pix.height() + 4);
}

void PixItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->drawPixmap(-pix.width() / 2, -pix.height() / 2, pix);
}

void PixItem::updateItem(QString path)
{
	pix.load(path);
	update();
}

PreviewDialog::PreviewDialog()
{
	previewWidget = new PreviewView();

	//printTimeName = new QLabel(QStringLiteral(""));
	//printTimeLabel = new QLabel();

	modelVolumeName = new QLabel(QStringLiteral("消耗树脂量："));
	modelVolumeLabel = new QLabel();

	numberSlicesName = new QLabel(QStringLiteral("总层数："));
	numberSlicesLabel = new QLabel();

	layout1 = new QHBoxLayout();
	//layout1->addWidget(printTimeName);
	//layout1->addWidget(printTimeLabel);
	layout1->addWidget(modelVolumeName);
	layout1->addWidget(modelVolumeLabel);
	layout1->addWidget(numberSlicesName);
	layout1->addWidget(numberSlicesLabel);
	
	mainLayout = new QGridLayout();
	mainLayout->addLayout(layout1,0,0);
	mainLayout->addWidget(previewWidget, 1, 0, 1, 3);
	mainLayout->setSpacing(2);
	mainLayout->setMargin(1);

	setLayout(mainLayout);
	
	setWindowState(Qt::WindowMaximized);
}

PreviewDialog::~PreviewDialog()
{
	delete previewWidget;
	delete modelVolumeName;
	delete modelVolumeLabel;
	delete numberSlicesName;
	delete numberSlicesLabel;
	delete layout1;
	delete mainLayout;
}

bool PreviewDialog::readIni(QString path)
{
	this->previewWidget->path = path;
	this->previewWidget->path.append("/");
	this->previewWidget->path.append(ImageName);

	QDir *dir = new QDir(path);
	QStringList filter;
	//得到文件名列表
	QList<QFileInfo> *fileInfo = new QList<QFileInfo>(dir->entryInfoList(filter));

	int normIlluminationTime,numberSlices;

	for (auto f = fileInfo->begin(); f != fileInfo->end(); ++f) {
		if ((*f).baseName() == buildsciptName) {
			QFile file((*f).filePath());
			if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
				return false;
			QTextStream in(&file);
			QString line = in.readLine();
			while (!line.isNull())
			{
				if (line.contains("number of slices")) {
					int a = line.lastIndexOf("=");
					QString num = line.mid(a + 2);//得到总层数
					previewWidget->slider->setRange(0, num.toInt() - 1);
					numberSlices = num.toInt();

					num.append(QStringLiteral("层"));
					numberSlicesLabel->setText(num);
					//初始化
					previewWidget->slider->setValue(0);
					previewWidget->image->setScale(1);
					previewWidget->image->setPos(0, 0);
					previewWidget->_scale = 1;
					previewWidget->x = 0;
					previewWidget->y = 0;

					//读取第一张图片
					QString s = path;
					s.append("/");
					s.append(ImageName);
					s.append("0.png");
					previewWidget->image->updateItem(s);

				}
				else if (line.contains("model volume")) {
					int a = line.lastIndexOf("=");
					QString num = line.mid(a + 2);
					num.append("ml");
					modelVolumeLabel->setText(num);
				}
				else if (line.contains("norm illumination time")) {
					int a = line.lastIndexOf("=");
					normIlluminationTime = line.mid(a + 2).toInt();
				}
				line = in.readLine();
			}
		}
	}

	//计算打印时间
	//int printTime= normIlluminationTime*

	return true;
}

PreviewView::PreviewView()
{
	_scale = 1;
	x = 0;
	y = 0;
	//设置场景
	scene = new QGraphicsScene(this);
	scene->setSceneRect(-200, -150, 400, 300);
	setScene(scene);
	pixmap = new QPixmap(":/new/prefix1/images/slice0.png");
	image = new PixItem(pixmap);
	scene->addItem(image);
	image->setPos(0, 0);
	//层数显示标签
	label = new QLabel;
	label->setStyleSheet("color: rgb(255,0,0) ; background-color: rgb(225,225,225); border: 1px outset blue; border-radius: 5px;");
	label->setFixedSize(32, 15);
	label->setText(QString::number(0));
	//用切换层的滑动条
	slider = new QSlider();
	slider->setOrientation(Qt::Vertical);
	slider->setRange(0,0);
	slider->setTickInterval(1);
	slider->setValue(0);
	connect(slider, SIGNAL(valueChanged(int)), this, SLOT(valueChange(int)));	

	layerLayout = new QVBoxLayout();
	layerLayout->addWidget(label);
	layerLayout->addWidget(slider, 10);

	setBackgroundBrush(QColor(0, 0, 0));//设置黑色背景
	setLayout(layerLayout);
	setWindowTitle(QStringLiteral("预览"));
	//setWindowState(Qt::WindowMaximized);
	setWindowIcon(QIcon(":/new/prefix1/images/SM.png"));
	//setAttribute(Qt::WA_QuitOnClose, false);//设置主窗口关闭即程序退出
}

PreviewView::~PreviewView()
{
	delete image;
	delete slider;
	delete label;
	delete layerLayout;
	delete scene;
	delete pixmap;
}

void PreviewView::valueChange(int num)
{
	//更新label
	label->setText(QString::number(num));
	//更新图片
	QString s = this->path;
	s.append(QString::number(num));
	s.append(".png");
	image->updateItem(s);
}

void PreviewView::wheelEvent(QWheelEvent *event)
{
	int numDegrees = event->delta() / 8;//滚动的角度，*8就是鼠标滚动的距离
	int numSteps = numDegrees / 15;//滚动的步数，*15就是鼠标滚动的角度
	_scale = _scale + numSteps*0.1;
	if (_scale > 3)
		_scale = 3;
	if (_scale < 0.1)
		_scale = 0.1;
	image->setScale(_scale);
	update();
}

void PreviewView::mouseMoveEvent(QMouseEvent *event)
{
	image->moveBy(event->x() - x, event->y() - y);
	x = event->x();
	y = event->y();
	update();
}

void PreviewView::mousePressEvent(QMouseEvent *event)
{
	x = event->x();
	y = event->y();
}

SetupDialog::SetupDialog(MainWindow* _mainwindow)
	:mainwindow(_mainwindow)
{
	initLayout();
	readConfig();
	resize(minimumSize());
	setWindowFlags(windowFlags()&~Qt::WindowCloseButtonHint);
	setWindowTitle(QStringLiteral("设置"));
	setWindowIcon(QIcon(":/icon/images/SM.png"));
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
	//光照设置
	normIlluSpin = new QSpinBox();
	normIlluSpin->setRange(5, 10);
	normIlluSpin->setSuffix("s");
	normIlluLabel = new QLabel(QStringLiteral("正常曝光时间"));

	norm_inttersity_spin = new QSpinBox();
	norm_inttersity_spin->setRange(50, 100);
	norm_inttersity_spin->setSuffix("%");
	norm_inttersity_label = new QLabel(QStringLiteral("正常曝光强度"));

	overIlluSpin = new QSpinBox();
	overIlluSpin->setRange(5, 10);
	overIlluSpin->setSuffix("s");
	overIlluLabel = new QLabel(QStringLiteral("长曝光时间"));
	overIlluLabel->setToolTip(QStringLiteral("前几层需要长曝光的时间"));

	over_inttersity_spin = new QSpinBox();
	over_inttersity_spin->setRange(50, 100);
	over_inttersity_spin->setSuffix("%");
	over_inttersity_label = new QLabel(QStringLiteral("长曝光强度"));
	over_inttersity_label->setToolTip(QStringLiteral("前几层需要长曝光的光照强度"));

	overLayerSpin = new QSpinBox(); 
	overLayerSpin->setRange(0, 10);
	overLayerSpin->setSuffix(QStringLiteral("层"));
	overLayerLabel = new QLabel(QStringLiteral("长曝光层数"));
	overLayerLabel->setToolTip(QStringLiteral("前几层需要长曝光的层数"));

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

	//支撑设置
	top_height_spin = new QDoubleSpinBox();
	top_height_spin->setRange(1, 5);
	top_height_spin->setSingleStep(1);
	top_height_spin->setSuffix("mm");
	top_height_label = new QLabel(QStringLiteral("顶端高度"));

	top_radius_spin = new QDoubleSpinBox();
	top_radius_spin->setRange(0.5, 2);
	top_radius_spin->setSingleStep(0.1);
	top_radius_spin->setSuffix("mm");
	top_radius_label = new QLabel(QStringLiteral("顶端直径"));
	top_radius_label->setToolTip(QStringLiteral("控制支撑顶端与模型接触面积大小"));

	support_radius_spin = new QDoubleSpinBox();
	support_radius_spin->setRange(0.5, 3);
	support_radius_spin->setSingleStep(0.1);
	support_radius_spin->setSuffix("mm");
	support_radius_label = new QLabel(QStringLiteral("支撑直径"));
	support_radius_label->setToolTip(QStringLiteral("支撑柱主杆部分的粗细程度"));

	bottom_radius_spin = new QDoubleSpinBox();
	bottom_radius_spin->setRange(0.5, 4);
	bottom_radius_spin->setSingleStep(0.1);
	bottom_radius_spin->setSuffix("mm");
	bottom_radius_label = new QLabel(QStringLiteral("底端直径"));
	bottom_radius_label->setToolTip(QStringLiteral("一端在底板上的支撑柱底端的粗细程度"));

	support_space_spin = new QSpinBox();
	support_space_spin->setRange(2, 20);
	support_space_spin->setSuffix("mm");
	support_space_label = new QLabel(QStringLiteral("支撑间距"));
	support_space_label->setToolTip(QStringLiteral("支撑特征面的支撑柱的间距"));

	support_angle_spin = new QSpinBox();
	support_angle_spin->setRange(0, 90);
	support_angle_spin->setSingleStep(5);
	support_angle_spin->setSuffix(QStringLiteral("°"));
	support_angle_label = new QLabel(QStringLiteral("支撑角度"));
	support_angle_label->setToolTip(QStringLiteral("三角面片与平台的夹角低于设定支撑角度的三角面片会被支撑"));

	leaf_num_spin = new QSpinBox();
	leaf_num_spin->setRange(1, 100);
	leaf_num_spin->setSingleStep(1);
	leaf_num_spin->setSuffix(QStringLiteral("个"));
	leaf_num_label = new QLabel(QStringLiteral("树枝数量"));
	leaf_num_label->setToolTip(QStringLiteral("树状支撑中一颗树树枝的最大数量"));

	model_lift_spin = new QDoubleSpinBox();
	model_lift_spin->setRange(0, 10);
	model_lift_spin->setSingleStep(1);
	model_lift_spin->setSuffix(tr("mm"));
	model_lift_label = new QLabel(QStringLiteral("模型提升"));
	model_lift_label->setToolTip(QStringLiteral("自动支撑时模型提升高度"));

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
	supportWidget = new QWidget(this);
	supportWidget->setLayout(supportLayout);

	//抽空设置
	hollow_out_box = new QCheckBox();
	hollow_out_label = new QLabel(QStringLiteral("抽        空"));
	hollow_out_label->setToolTip(QStringLiteral("体积较大的模型内部是否需要抽空"));

	fill_pattern_combo = new QComboBox();
	fill_pattern_combo->insertItem(0, QStringLiteral("蜂窝填充"));
	fill_pattern_combo->insertItem(1, QStringLiteral("3D支撑填充"));
	fillPatternLabel = new QLabel(QStringLiteral("填充类型"));

	wall_thickness_spin = new QDoubleSpinBox();
	wall_thickness_spin->setRange(1, 5);
	wall_thickness_spin->setSuffix("mm");
	wall_thickness_label = new QLabel(QStringLiteral("壁        厚"));
	wall_thickness_label->setToolTip(QStringLiteral("抽空模型的壁厚"));

	density_spin = new QSpinBox();
	density_spin->setRange(0, 100);
	density_spin->setSingleStep(5);
	density_spin->setSuffix("%");
	density_label = new QLabel(QStringLiteral("填充密度"));

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
	hollowOutWidget = new QWidget(this);
	hollowOutWidget->setLayout(hollowOutLayout);

	//其他设置
	thicknessCombo = new QComboBox();
	thicknessCombo->insertItem(0, "0.1mm");
	thicknessCombo->insertItem(1, "0.05mm");
	thicknessCombo->insertItem(2, "0.025mm");
	thicknessLabel = new QLabel(QStringLiteral("层    厚"));
	thicknessLabel->setToolTip(QStringLiteral("模型切片的层厚"));

	raftSpin = new QSpinBox();
	raftSpin->setRange(0, 10);
	raftSpin->setSuffix(QStringLiteral("层"));
	raftLabel = new QLabel(QStringLiteral("底筏层数"));
	raftLabel->setToolTip(QStringLiteral("模型底部的底板层数(底板高度需乘以层厚值)"));

	raftOffsetSpin = new QSpinBox();
	raftOffsetSpin->setRange(0, 100);
	raftOffsetSpin->setSuffix("mm");
	raftOffsetLabel = new QLabel(QStringLiteral("底板补偿"));

	arrange_space_spin = new QDoubleSpinBox();
	arrange_space_spin->setRange(0, 10);
	arrange_space_spin->setSuffix("mm");
	arrangeSpaceLabel = new QLabel(QStringLiteral("自动排列间距"));

	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);

	threadSpin = new QSpinBox();
	threadSpin->setRange(1, systemInfo.dwNumberOfProcessors);//获取处理器线程数
	threadLabel = new QLabel(QStringLiteral("线程数"));
	threadLabel->setToolTip(QStringLiteral("执行多线程命令时所用到的线程数，最大线程数为电脑处理器线程数。"));

	otherLayourt = new QGridLayout();
	otherLayourt->setMargin(10);
	otherLayourt->setSpacing(10);
	otherLayourt->addWidget(thicknessLabel, 0, 0);
	otherLayourt->addWidget(thicknessCombo, 0, 1);
	otherLayourt->addWidget(raftLabel,1, 0);
	otherLayourt->addWidget(raftSpin, 1, 1);
	otherLayourt->addWidget(raftOffsetLabel, 2, 0);
	otherLayourt->addWidget(raftOffsetSpin, 2, 1);
	otherLayourt->addWidget(arrangeSpaceLabel, 3, 0);
	otherLayourt->addWidget(arrange_space_spin, 3, 1);
	otherLayourt->addWidget(threadLabel, 4, 0);
	otherLayourt->addWidget(threadSpin, 4, 1);
	otherWidget = new QWidget(this);
	otherWidget->setLayout(otherLayourt);

	setupTab = new QTabWidget();
	setupTab->addTab(supportWidget, QStringLiteral("支撑设置"));
	setupTab->addTab(hollowOutWidget, QStringLiteral("抽空设置"));
	setupTab->addTab(illuWidget, QStringLiteral("光照设置"));
	setupTab->addTab(otherWidget, QStringLiteral("其他设置"));

	defultBtn = new QPushButton(QStringLiteral("默认设置"));
	connect(defultBtn, SIGNAL(clicked()), this, SLOT(recoverDefult()));

	cancalBtn = new QPushButton(QStringLiteral("取消"));
	connect(cancalBtn, SIGNAL(clicked()), this, SLOT(BtnChange()));

	okBtn = new QPushButton(QStringLiteral("确认"));
	connect(okBtn, SIGNAL(clicked()), this, SLOT(writeConfig()));
	connect(okBtn, SIGNAL(clicked()), this, SLOT(BtnChange()));

	mainLayout = new QGridLayout(this);
	mainLayout->addWidget(setupTab, 0, 0, 6, 10);
	mainLayout->addWidget(defultBtn, 7, 0);
	mainLayout->addWidget(cancalBtn, 7, 8);
	mainLayout->addWidget(okBtn, 7, 9);
	mainLayout->setMargin(10);
	mainLayout->setSpacing(10);

	this->setLayout(mainLayout);
}

void SetupDialog::writeConfig()
{
	QString file = getFileLocation();
	file.append("/config.ini");
	QSettings* writeini = new QSettings(file, QSettings::IniFormat);
	writeini->clear();
	writeini->setValue("/illu_config/normIllu", normIlluSpin->value());
	writeini->setValue("/illu_config/normInttersity", norm_inttersity_spin->value());
	writeini->setValue("/illu_config/overIllu", overIlluSpin->value());
	writeini->setValue("/illu_config/overInttersity", over_inttersity_spin->value());
	writeini->setValue("/illu_config/overLayer", overLayerSpin->value());

	writeini->setValue("/support_config/topHeight", top_height_spin->value());
	writeini->setValue("/support_config/topRadius", top_radius_spin->value());
	writeini->setValue("/support_config/supportRadius", support_radius_spin->value());
	writeini->setValue("/support_config/bottomRadius", bottom_radius_spin->value());
	writeini->setValue("/support_config/supportSpace", support_space_spin->value());
	writeini->setValue("/support_config/supportAngle", support_angle_spin->value());
	writeini->setValue("/support_config/leafNum", leaf_num_spin->value());
	writeini->setValue("/support_config/modelLift", model_lift_spin->value());

	writeini->setValue("/hollow_out_config/hollowOutBox", int(hollow_out_box->isChecked()));
	writeini->setValue("/hollow_out_config/fillPattern", fill_pattern_combo->currentIndex());
	writeini->setValue("/hollow_out_config/wallThickness", wall_thickness_spin->value());
	writeini->setValue("/hollow_out_config/fillDensity", density_spin->value());

	writeini->setValue("/other/thickness", thicknessCombo->currentIndex());
	writeini->setValue("/other/raft", raftSpin->value());
	writeini->setValue("/other/raftOffset", raftOffsetSpin->value());
	writeini->setValue("/other/arrangeSpace", arrange_space_spin->value());
	writeini->setValue("/other/thread", threadSpin->value());

	delete writeini;

	mainwindow->DlpPrintLoadSetup();
}

void SetupDialog::recoverDefult()
{
	setDefultValue();
	writeConfig();
}

void SetupDialog::BtnChange()
{
	this->close();
	mainwindow->showSetupDialog();
}

void SetupDialog::setDefultValue()
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
}

void SetupDialog::readConfig()
{
	QString file = getFileLocation();
	file.append("/config.ini");
	QFile dir(file);
	if (dir.exists()) {
		QSettings* readini = new QSettings(file, QSettings::IniFormat);
		//读取打印设置
		QString normIllu = readini->value("/illu_config/normIllu").toString();
		normIlluSpin->setValue(normIllu.toInt());

		QString normInttersity = readini->value("/illu_config/normInttersity").toString();
		norm_inttersity_spin->setValue(normInttersity.toInt());

		QString overIllu = readini->value("/illu_config/overIllu").toString();
		overIlluSpin->setValue(overIllu.toInt());

		QString overInttersity = readini->value("/illu_config/overInttersity").toString();
		over_inttersity_spin->setValue(overInttersity.toInt());

		QString overLayer = readini->value("/illu_config/overLayer").toString();
		overLayerSpin->setValue(overLayer.toInt());


		QString topHeight = readini->value("/support_config/topHeight").toString();
		top_height_spin->setValue(topHeight.toDouble());

		QString topRadius = readini->value("/support_config/topRadius").toString();
		top_radius_spin->setValue(topRadius.toDouble());

		QString supportRadius = readini->value("/support_config/supportRadius").toString();
		support_radius_spin->setValue(supportRadius.toDouble());

		QString bottomRadius = readini->value("/support_config/bottomRadius").toString();
		bottom_radius_spin->setValue(bottomRadius.toDouble());

		QString supportSpace = readini->value("/support_config/supportSpace").toString();
		support_space_spin->setValue(supportSpace.toInt());

		QString supportAngle = readini->value("/support_config/supportAngle").toString();
		support_angle_spin->setValue(supportAngle.toInt());

		QString leafNum = readini->value("/support_config/leafNum").toString();
		leaf_num_spin->setValue(leafNum.toInt());

		QString modelLift = readini->value("/support_config/modelLift").toString();
		model_lift_spin->setValue(modelLift.toDouble());

		
		QString hollowOutBox = readini->value("/hollow_out_config/hollowOutBox").toString();
		hollow_out_box->setChecked(hollowOutBox.toInt());

		QString fillPattern = readini->value("/hollow_out_config/fillPattern").toString();
		fill_pattern_combo->setCurrentIndex(fillPattern.toInt());

		QString wallThickness = readini->value("/hollow_out_config/wallThickness").toString();
		wall_thickness_spin->setValue(wallThickness.toDouble());

		QString Density = readini->value("/hollow_out_config/fillDensity").toString();
		density_spin->setValue(Density.toDouble());


		QString thickness = readini->value("/other/thickness").toString();
		thicknessCombo->setCurrentIndex(thickness.toInt());

		QString raft = readini->value("/other/raft").toString();
		raftSpin->setValue(raft.toInt());

		QString raftOffset = readini->value("/other/raftOffset").toString();
		raftOffsetSpin->setValue(raftOffset.toInt());

		QString arrangeSpace = readini->value("/other/arrangeSpace").toString();
		arrange_space_spin->setValue(arrangeSpace.toDouble());

		QString thread = readini->value("/other/thread").toString();
		threadSpin->setValue(thread.toInt());

		delete readini;
	}
	else {
		setDefultValue();
	}
}

AboutDialog::AboutDialog()
{
	icoBtn = new QPushButton(QIcon(":/icon/images/SM.png"), "");
	icoBtn->setFixedSize(60, 70);
	icoBtn->setIconSize(QSize(60, 70));
	icoBtn->setStyleSheet("background-color: rgba(225,225,225,0);");

	QFont font("ZYSong18030", 10);
	nameLabel = new QLabel("S-Maker_DLP1.6.7  64bit");
	nameLabel->setFont(font);

	QString str(QStringLiteral("速美科技版权所有"));
	str.append("\n@2017-2018");
	copyrightLabel = new QLabel(str);

	versionLabel = new QLabel(QStringLiteral("版本更新:"));

	textEdit = new QPlainTextEdit();
	textEdit->setReadOnly(true);
	textEdit->setPlainText(QStringLiteral(
		"版本--1.6.7\n"
		"1.增加机型选择功能。\n"
		"2.增加Dental_DLP机型。\n"
		"版本--1.6.6\n"
		"1.提高手动支撑效率。\n"
		"2.拖动功能的优化。\n"
		"3.增加obj文件格式。\n"
		"4.更新图标。\n"
		"版本--1.6.5\n"
		"1.内存优化。\n"
		"2.删除支撑改为左键单击。\n"
		"3.添加底板补偿参数。\n"
		"4.针对悬吊面按形状均匀支撑。\n"
		"5.修复复杂模型支撑容易穿过的bug。\n"
		"6.支撑底端与模型接触成角度。\n"
		"7.其他修改。\n"
		"版本--1.6.4\n"
		"1.优化进度条显示。\n"
		"2.增加模型拖拽功能。\n"
		"3.优化自动支撑。\n"
		"4.增加蜂窝填充。\n"
		"5.其他修改。\n"
		"版本--1.6.3\n"
		"1.添加“撤销重做”功能。\n"
		"2.支撑头与模型成角度。\n"
		"3.模型转换操作实时显示。\n"
		"4.增加透视投影与正交投影。\n"
		"5.排除支撑直接穿过模型的bug。\n"
		"6.支撑树枝部分加强。\n"
		"7.其他修改。\n"
		"版本--1.6.2\n"
		"1.树状支撑。\n"
		"2.可单独添加支撑。\n"
		"3.改变支撑编辑的方式。\n"
		"版本--1.6.1\n"
		"1.添加自动排列功能。\n"
		"2.添加模型复制功能。\n"
		"3.添加坐标轴信息。\n"
		"4.修复支撑渲染效果不真实的bug。\n"
		"5.优化手动添加支撑。\n"
		"6.排除添加支撑时悬吊面对悬吊点的影响。\n"
		"7.优化切片时出现断层的影响。\n"
		"8.调整界面的逻辑结构。\n"
		"9.添加可直接对模型进行手动支撑的操作。\n"
		"10.优化图形渲染时内存消耗。\n"
		"版本--1.6.0\n"
		"1.调整模型视图渲染效果。\n"
		"2.渲染实体支撑。\n"
		"3.支持模型平移，缩放，旋转三个方向的操作。\n"
		"4.增加支撑编辑功能，实现手动支撑，删除支撑的操作。\n"
		"5.多线程生成图片。\n"
		"6.切片生成的文件为.sm的自定义的文件。\n"
		"7.丰富设置选项。\n"
		"8.增加浏览切片文件的功能。\n"
		"9.视图内的模型支撑可另存为stl模型。\n"
		"10.应用程序支持64bit。\n"
		"11.调整内部支撑的结构。\n"
		"12.界面重新调整。"));

	layout = new QGridLayout();
	layout->addWidget(icoBtn, 0, 0, 2, 2);
	layout->addWidget(nameLabel, 0, 2, 1, 3);
	layout->addWidget(copyrightLabel, 1, 2, 1, 3);
	layout->addWidget(versionLabel, 2, 0, 1, 2);
	layout->addWidget(textEdit, 3, 0, 8, 5);
	setLayout(layout);
	setFixedSize(400, 300);
}

AboutDialog::~AboutDialog()
{
	delete icoBtn;
	delete nameLabel;
	delete copyrightLabel;
	delete versionLabel;
	delete textEdit;
	delete layout;
}

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	initDlprinter();
	interface1 = new Interface(dlprinter);
	initUndo();
	initAction();
	initSetupDialog();
	initCentralWindow();//要先于中央界面上button初始化
	initButton();
	initOffsetWidget();
	initRotateWidget();
	initScaleWidget();
	initProgress();
	init_rotate();

	setAcceptDrops(true);
	setWindowTitle("DLPSlicer");
	setAttribute(Qt::WA_QuitOnClose, true);//设置主窗口关闭即程序退出
	resize(1200, 700);
}

MainWindow::~MainWindow()
{

}

QString MainWindow::readStlTxt()
{
	QString file = getFileLocation();
	file.append("/modelPath.ini");
	QFile dir(file);
	if (dir.exists()) {
		QSettings* readini = new QSettings(file, QSettings::IniFormat);
		//读取打印设置
		QString path = readini->value("/modelPath/path").toString();
		delete readini;
		return path;
	}
	else
		return "";
}

void MainWindow::stroyStlTxt(QString stl)
{
	QString file = getFileLocation();
	file.append("/modelPath.ini");
	QSettings* writeini = new QSettings(file, QSettings::IniFormat);
	writeini->clear();
	writeini->setValue("/modelPath/path", stl);
	delete writeini;
}

QString MainWindow::get_file_location()
{
	QString name = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
	name.append("/.S_Maker");
	QDir* file = new QDir(name);
	bool ret = file->exists();
	if (ret) {
		return name;
	}
	else {
		bool ok = file->mkdir(name);
		if (!ok)
			exit(2);
		return file->dirName();
	}
}

void MainWindow::openStl()
{
	//-------------进入---------------
	clickedButtonState(OPENBTN);
	openButton->setStyleSheet(OffButton);
	OnOff_open = true;
	//-----------------------------

	QString temp, path;
	path = readStlTxt();
	int a = path.lastIndexOf("/");
	QString s = path.left(a);
	temp = QFileDialog::getOpenFileName(this, QStringLiteral("选择需要打开的文件"), s, "*.stl *.sm *.obj");

	int format = -1;
	if(temp.right(4).indexOf(".stl", 0, Qt::CaseInsensitive) >= 0)
		format = stl;
	else if (temp.right(4).indexOf(".obj", 0, Qt::CaseInsensitive) >= 0)
		format = obj;
	else if (temp.right(4).indexOf(".amf", 0, Qt::CaseInsensitive) >= 0)
		format = amf;

	if(format>=0){
		stroyStlTxt(temp);

		showProgress(OPENBTN);
		qDebug() << temp;
		std::string file = temp.toStdString();
		P(20);
		size_t id = interface1->load_model(file, format);
		P(60);
		glwidget->save_valume(id);
		P(80);
		MainUndoStack->push(new AddModelCommand(id, this));
		P(100);
		hideProgress();
		glwidget->updateConfine();
		glwidget->updateTranslationID();
	}
	else if (temp.right(3).indexOf(".sm", 0, Qt::CaseInsensitive) >= 0) {
		showPreviewWidget1(temp);
	}

	//--------------退出------------------
	openButton->setStyleSheet(OnButton);
	OnOff_open = false;
	//-----------------------------------
}

void MainWindow::deleteStl()
{
	clickedButtonState(SETUPBTN);
	if (glwidget->selectID >= 0) {
		deleteSupport();
		MainUndoStack->push(new DeleteModelCommand(glwidget->selectID, this));

		glwidget->updateConfine();
		glwidget->selectID = -1;
	}
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Delete) {
		deleteStl();
	}
	else if (event->key() == Qt::Key_Return) {
		glwidget->addOneSupport();
	}
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->urls()[0].fileName().right(4).indexOf(".stl", 0, Qt::CaseInsensitive) >= 0 ||
		event->mimeData()->urls()[0].fileName().right(4).indexOf(".obj", 0, Qt::CaseInsensitive) >= 0 ||
		event->mimeData()->urls()[0].fileName().right(4).indexOf(".amf", 0, Qt::CaseInsensitive) >= 0 ||
		event->mimeData()->urls()[0].fileName().right(3).indexOf(".sm", 0, Qt::CaseInsensitive) >= 0)
		event->acceptProposedAction();
	else
		event->ignore();//否则不接受鼠标事件  
}

void MainWindow::dropEvent(QDropEvent *event)
{
	QList<QUrl> urls = event->mimeData()->urls();
	if (urls.isEmpty())
		return;

	//打开stl文件
	QString fileName = urls.first().toLocalFile();
	int format = -1;
	if (fileName.right(4).indexOf(".stl", 0, Qt::CaseInsensitive) >= 0)
		format = stl;
	else if (fileName.right(4).indexOf(".obj", 0, Qt::CaseInsensitive) >= 0)
		format = obj;
	else if (fileName.right(4).indexOf(".amf", 0, Qt::CaseInsensitive) >= 0)
		format = amf;

	if (format >= 0) {
		stroyStlTxt(fileName);

		showProgress(OPENBTN);
		std::string file = fileName.toStdString();
		P(20);
		size_t id = interface1->load_model(file, format);
		P(60);
		glwidget->save_valume(id);
		P(80);
		MainUndoStack->push(new AddModelCommand(id, this));
		P(100);
		hideProgress();
		glwidget->updateConfine();

		glwidget->updateTranslationID();
	}
	else if (fileName.right(3).indexOf(".sm", 0, Qt::CaseInsensitive) >= 0) {
		showPreviewWidget1(fileName);
	}
}

void MainWindow::_exit()
{
	exit(0);
}

void MainWindow::initButton()
{
	//机型选择部分
	SelectPrinterLabel = new QLabel(QStringLiteral("选择机型"),this);
	SelectPrinterLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	SelectPrinterLabel->setFixedSize(90, 20);
	SelectPrinterLabel->show();

	SelectPrinterCombo = new QComboBox(this);
	SelectPrinterCombo->insertItem(0, "S288");
	SelectPrinterCombo->insertItem(1, "S250");
	SelectPrinterCombo->setFixedSize(100, 20);
	if (dlprinter->printer == S288)
		SelectPrinterCombo->setCurrentIndex(0);
	else if (dlprinter->printer == S250)
		SelectPrinterCombo->setCurrentIndex(1);
	SelectPrinterCombo->show();
	connect(SelectPrinterCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(dlprinterChange(QString)));

	//left
	openButton = new QPushButton(QIcon(":/icon/images/load_b.png"), "",this);
	openButton->setIconSize(QSize(120, 140));
	openButton->setStyleSheet(OnButton);
	openButton->setFixedSize(60, 70);
	openButton->setToolTip(QStringLiteral("打开"));
	openButton->show();
	OnOff_open = false;
	connect(openButton, SIGNAL(clicked()), this, SLOT(openStl()));

	offsetButton = new QPushButton(QIcon(":/icon/images/offset_b.PNG"), "", this);
	offsetButton->setIconSize(QSize(90, 115));
	offsetButton->setStyleSheet(OnButton);
	offsetButton->setEnabled(true);
	offsetButton->setFixedSize(60, 70);
	offsetButton->setToolTip(QStringLiteral("移动"));
	offsetButton->show();
	OnOff_offset = false;
	connect(offsetButton, SIGNAL(clicked()), this, SLOT(showOffsetWidget()));

	scaleButton = new QPushButton(QIcon(":/icon/images/scale_b.png"), "", this);
	scaleButton->setIconSize(QSize(90, 115));
	scaleButton->setStyleSheet(OnButton);
	scaleButton->setEnabled(true);
	scaleButton->setFixedSize(60, 70);
	scaleButton->setToolTip(QStringLiteral("缩放"));
	scaleButton->show();
	OnOff_scale = false;
	connect(scaleButton, SIGNAL(clicked()), this, SLOT(showScaleWidget()));

	rotateButton = new QPushButton(QIcon(":/icon/images/rotate_b.png"), "", this);
	rotateButton->setIconSize(QSize(90, 115));
	rotateButton->setStyleSheet(OnButton);
	rotateButton->setEnabled(true);
	rotateButton->setFixedSize(60, 70);
	rotateButton->setToolTip(QStringLiteral("旋转"));
	rotateButton->show();
	OnOff_rotate = false;
	connect(rotateButton, SIGNAL(clicked()), this, SLOT(showRotateWidget()));

	//right
	supportButton = new QPushButton(QIcon(":/icon/images/support_b.png"), "", this);
	supportButton->setIconSize(QSize(90, 115));
	supportButton->setStyleSheet(OnButton);
	supportButton->setFixedSize(60, 70);
	supportButton->setToolTip(QStringLiteral("支撑"));
	supportButton->show();
	OnOff_support = false;
	connect(supportButton, SIGNAL(clicked()), this, SLOT(generateSupport()));

	supportEditButton = new QPushButton(QIcon(":/icon/images/supportEdit_b.png"), "", this);
	supportEditButton->setIconSize(QSize(90, 115));
	supportEditButton->setStyleSheet(OnButton);
	supportEditButton->setFixedSize(60, 70);
	supportEditButton->setToolTip(QStringLiteral("支撑编辑"));
	supportEditButton->show();
	OnOff_supportEdit = false;
	connect(supportEditButton, SIGNAL(clicked()), this, SLOT(SupportEdit()));

	sliceButton = new QPushButton(QIcon(":/icon/images/slice_b.png"), "", this);
	sliceButton->setIconSize(QSize(90, 115));
	sliceButton->setStyleSheet(OnButton);
	sliceButton->setFixedSize(60, 70);
	sliceButton->setToolTip(QStringLiteral("切片"));
	sliceButton->show();
	OnOff_slice = false;
	connect(sliceButton, SIGNAL(clicked()), this, SLOT(slice()));

	setupButton = new QPushButton(QIcon(":/icon/images/setup_b.png"), "", this);
	setupButton->setIconSize(QSize(90, 115));
	setupButton->setStyleSheet(OnButton);
	setupButton->setFixedSize(60, 70);
	setupButton->setToolTip(QStringLiteral("设置"));
	setupButton->show();
	OnOff_setup = false;
	connect(setupButton, SIGNAL(clicked()), this, SLOT(showSetupDialog()));
}

void MainWindow::clickedButtonState(int btn)
{
	if (OnOff_offset && btn != OFFSETBTN) {
		offsetWidget->hide();
		OnOff_offset = false;
		offsetButton->setStyleSheet(OnButton);
	}
	else if (OnOff_rotate && btn!=ROTATEBTN) {
		init_rotate();
		rotateWidget->hide();
		OnOff_rotate = false;
		rotateButton->setStyleSheet(OnButton);
	}
	else if (OnOff_scale && btn != SCALEBTN) {
		scaleWidget->hide();
		OnOff_scale = false;
		scaleButton->setStyleSheet(OnButton);
	}
	else if (OnOff_open && btn != OPENBTN) {
		OnOff_open = false;
		openButton->setStyleSheet(OnButton);
	}
	else if (OnOff_support && btn != SUPPORTBTN) {
		OnOff_support = false;
		supportButton->setStyleSheet(OnButton);
	}
	else if (OnOff_supportEdit && btn != SUPPORTEDITBTN) {
		SupportEdit();
		OnOff_supportEdit = false;
		supportEditButton->setStyleSheet(OnButton);
	}
	else if (OnOff_setup && btn != SETUPBTN) {
		OnOff_setup = false;
		setupButton->setStyleSheet(OnButton);
	}
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
	QSize size = event->size();
	int w = size.width();
	int h = size.height();

	SelectPrinterLabel->move(w / 2 - 110, 30);
	SelectPrinterCombo->move(w / 2 - 20, 30);

	//left
	openButton->move(10, h/2 - 240);
	offsetButton->move(10, h/2 - 160);
	scaleButton->move(10, h/2 - 80);
	rotateButton->move(10, h/2);

	//rigft
	supportButton->move(w - 70, h / 2 - 240);
	supportEditButton->move(w - 70, h / 2 - 160);
	sliceButton->move(w - 70, h / 2 - 80);
	setupButton->move(w - 70, h / 2);

	offsetWidget->move(75, h / 2 - 160-10);
	rotateWidget->move(75, h / 2 -10);
	scaleWidget->move(75, h / 2 - 80 - 30);
	progressWidget->move(w / 2 - 150, h / 2 - 50);
}

void MainWindow::initAction()
{
	//“文件”主菜单
	QMenu* fileMenu = menuBar()->addMenu(QStringLiteral("文件(F)"));

	QAction* newAct = new QAction(QStringLiteral("新建项目"), this);
	fileMenu->addAction(newAct);
	connect(newAct, SIGNAL(triggered()), this, SLOT(newJob()));

	QAction* openAct = new QAction(QStringLiteral("打开文件"), this);
	fileMenu->addAction(openAct);
	connect(openAct, SIGNAL(triggered()), this, SLOT(openStl()));

	QAction* deleteAct = new QAction(QStringLiteral("删除模型"), this);
	fileMenu->addAction(deleteAct);
	connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteStl()));

	fileMenu->addSeparator();

	QAction* exitAct = new QAction(QStringLiteral("退出"), this);
	fileMenu->addAction(exitAct);
	connect(exitAct, SIGNAL(triggered()), this, SLOT(_exit()));

	//“编辑”主菜单
	QMenu* editMenu = menuBar()->addMenu(QStringLiteral("编辑(E)"));

	QAction *undoAction = MainUndoGroup->createUndoAction(this, QStringLiteral("撤销"));
	undoAction->setIcon(QIcon(":/icon/images/undo.png"));
	undoAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z));

	QAction *redoAction = MainUndoGroup->createRedoAction(this, QStringLiteral("重做"));
	redoAction->setIcon(QIcon(":/icon/images/redo.png"));
	redoAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Y));

	editMenu->addAction(undoAction);
	editMenu->addAction(redoAction);

	QAction* clearUndoAction = new QAction(QStringLiteral("清空撤销栈"));
	editMenu->addAction(clearUndoAction);
	connect(clearUndoAction, SIGNAL(triggered()), this, SLOT(clearUndoStack()));

	editMenu->addSeparator();

	QAction* repairAct = new QAction(QStringLiteral("模型修复"), this);
	editMenu->addAction(repairAct);
	repairAct->setDisabled(true);

	QAction* autoAct = new QAction(QStringLiteral("自动排列"), this);
	editMenu->addAction(autoAct);
	connect(autoAct, SIGNAL(triggered()), this, SLOT(autoArrange()));

	QAction*saveViewAct = new QAction(QStringLiteral("截图"), this);
	editMenu->addAction(saveViewAct);
	connect(saveViewAct, SIGNAL(triggered()), this, SLOT(saveView()));

	QAction* duplicateAct = new QAction(QStringLiteral("复制"), this);
	editMenu->addAction(duplicateAct);
	connect(duplicateAct, SIGNAL(triggered()), this, SLOT(_duplicate()));

	//“支撑”主菜单
	QMenu* supportMenu = menuBar()->addMenu(QStringLiteral("支撑(S)"));

	QAction* supportAct = new QAction(QStringLiteral("支撑"), this);
	supportMenu->addAction(supportAct);
	connect(supportAct, SIGNAL(triggered()), this, SLOT(generateSupport()));

	QAction* deleteSupportAct = new QAction(QStringLiteral("删除支撑"));
	supportMenu->addAction(deleteSupportAct);
	connect(deleteSupportAct, SIGNAL(triggered()), this, SLOT(deleteSupport()));

	QAction* supportAllAct = new QAction(QStringLiteral("支撑所有模型"));
	supportMenu->addAction(supportAllAct);
	connect(supportAllAct, SIGNAL(triggered()), this, SLOT(generateAllSupport()));

	QAction* deleteAllSupportAct = new QAction(QStringLiteral("删除所有支撑"));
	supportMenu->addAction(deleteAllSupportAct);
	connect(deleteAllSupportAct, SIGNAL(triggered()), this, SLOT(deleteAllSupport()));

    //“视图”主菜单
	QMenu* viewMenu = menuBar()->addMenu(QStringLiteral("视图(V)"));

	QAction* defaultAct = new QAction(QIcon(":/icon/images/iso.png"), QStringLiteral("默认视图"), this);
	viewMenu->addAction(defaultAct);
	connect(defaultAct, SIGNAL(triggered()), this, SLOT(defaultView()));

	QAction* overlookAct = new QAction(QIcon(":/icon/images/top.png"),QStringLiteral("俯视图"), this);
	viewMenu->addAction(overlookAct);
	connect(overlookAct, SIGNAL(triggered()), this, SLOT(overlookView()));

	QAction* leftAct = new QAction(QIcon(":/icon/images/left.png"), QStringLiteral("左视图"), this);
	viewMenu->addAction(leftAct);
	connect(leftAct, SIGNAL(triggered()), this, SLOT(leftView()));

	QAction* rightAct = new QAction(QIcon(":/icon/images/right.png"), QStringLiteral("右视图"), this);
	viewMenu->addAction(rightAct);
	connect(rightAct, SIGNAL(triggered()), this, SLOT(rightView()));

	QAction* frontAct = new QAction(QIcon(":/icon/images/front.png"), QStringLiteral("前视图"), this);
	viewMenu->addAction(frontAct);
	connect(frontAct, SIGNAL(triggered()), this, SLOT(frontView()));

	QAction* behindAct = new QAction(QIcon(":/icon/images/back.png"), QStringLiteral("后视图"), this);
	viewMenu->addAction(behindAct);
	connect(behindAct, SIGNAL(triggered()), this, SLOT(behindView()));

	viewMenu->addSeparator();

	QAction* persperctiveAct = new QAction(QStringLiteral("透视投影"), this);
	viewMenu->addAction(persperctiveAct);
	connect(persperctiveAct, SIGNAL(triggered()), this, SLOT(setPersperctive()));

	QAction* orthogonalityAct = new QAction(QStringLiteral("正交投影"), this);
	viewMenu->addAction(orthogonalityAct);
	connect(orthogonalityAct, SIGNAL(triggered()), this, SLOT(setOrthogonality()));

	//“帮助”主菜单
	QMenu* helpMenu = menuBar()->addMenu(QStringLiteral("帮助(H)"));

	QAction* updateAct = new QAction(QStringLiteral("检查更新"), this);
	helpMenu->addAction(updateAct);
	connect(updateAct, SIGNAL(triggered()), this, SLOT(checkUpdate()));

	QAction* messageAct = new QAction(QStringLiteral("版本信息"), this);
	helpMenu->addAction(messageAct);
	connect(messageAct, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
}

void MainWindow::initCentralWindow()
{
	glwidget = new GlWidget(this, dlprinter,interface1->model,interface1->dlprint);
	setCentralWidget(glwidget);
}

void MainWindow::initOffsetWidget()
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
	connect(x_offset_spin, SIGNAL(valueChanged(double)), this, SLOT(xoffsetValueChange(double)));

	y_offset_spin = new QDoubleSpinBox();
	y_offset_spin->setRange(-1000, 1000);
	y_offset_spin->setSingleStep(1);
	y_offset_spin->setStyleSheet(spinStyle);
	y_offset_spin->setSuffix("mm");
	connect(y_offset_spin, SIGNAL(valueChanged(double)), this, SLOT(yoffsetValueChange(double)));

	z_offset_spin = new QDoubleSpinBox();
	z_offset_spin->setRange(-1000, 1000);
	z_offset_spin->setSingleStep(1);
	z_offset_spin->setStyleSheet(spinStyle);
	z_offset_spin->setSuffix("mm");
	connect(z_offset_spin, SIGNAL(valueChanged(double)), this, SLOT(zoffsetValueChange(double)));

	QPushButton* ZPosZeroBtn = new QPushButton(QStringLiteral("贴底"));
	connect(ZPosZeroBtn, SIGNAL(clicked()), this, SLOT(ZPosZero()));

	QGridLayout* mainlayout = new QGridLayout(this);
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
	offsetWidget = new QStackedWidget(this);
	offsetWidget->setStyleSheet("background-color: rgba(225,225,225,100);");
	//offsetWidget->setMask(QPolygon(anomalyRect1));
	offsetWidget->setMinimumSize(180, 150);
	offsetWidget->addWidget(w);
	offsetWidget->hide();
}

void MainWindow::ZPosZero()
{
	if (glwidget->selectID >= 0) {
		ModelInstance* instance = IdToInstance();
		TriangleMesh mesh = instance->get_object()->volumes[0]->mesh;
		instance->transform_mesh(&mesh);
		BoundingBoxf3 box = mesh.bounding_box();

		TreeSupport* s1 = NULL;
		if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
			MainUndoStack->beginMacro(tr(""));
			MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
			MainUndoStack->push(new offsetValueChangeCommand(glwidget->selectID, 0, 0, -box.min.z, this));
			MainUndoStack->endMacro();
		}
		else
			MainUndoStack->push(new offsetValueChangeCommand(glwidget->selectID, 0, 0, -box.min.z, this));
	}
}

void MainWindow::setOffsetValue(ModelInstance* instance)
{
	x_offset_spin->setValue(instance->offset.x);
	y_offset_spin->setValue(instance->offset.y);
	z_offset_spin->setValue(instance->z_translation);
}

void MainWindow::AddOffsetValue(double x, double y, double z)
{
	if (x != 0)
		x_offset_spin->setValue(x_offset_spin->value() + x);
	if (y != 0)
		y_offset_spin->setValue(y_offset_spin->value() + y);
	if (z != 0)
		z_offset_spin->setValue(z_offset_spin->value() + z);
}

void MainWindow::xoffsetValueChange(double value)
{
	if (glwidget->selectID >= 0) {
		ModelInstance* instance = IdToInstance();
		if (instance->offset.x != value) {
			double x = value - instance->offset.x;
			if (fabs(x) >= 0.01) {//防止细微误差
				TreeSupport* s1 = NULL;
				if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
					MainUndoStack->beginMacro(tr(""));
					MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
					MainUndoStack->push(new offsetValueChangeCommand(glwidget->selectID, x, 0, 0, this));
					MainUndoStack->endMacro();
				}
				else
					MainUndoStack->push(new offsetValueChangeCommand(glwidget->selectID, x, 0, 0, this));
			}
		}
	}
}

void MainWindow::yoffsetValueChange(double value)
{
	if(glwidget->selectID >= 0) {
		ModelInstance* instance = IdToInstance();
		if (instance->offset.y != value) {
			double y = value - instance->offset.y;
			if (fabs(y) >= 0.01) {
				TreeSupport* s1 = NULL;
				if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
					MainUndoStack->beginMacro(tr(""));
					MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
					MainUndoStack->push(new offsetValueChangeCommand(glwidget->selectID, y, 0, 0, this));
					MainUndoStack->endMacro();
				}
				else
					MainUndoStack->push(new offsetValueChangeCommand(glwidget->selectID, 0, y, 0, this));
			}
		}
	}
}

void MainWindow::zoffsetValueChange(double value)
{
	if(glwidget->selectID >= 0) {
		ModelInstance* instance = IdToInstance();
		if (instance->z_translation != value) {
			double z = value - instance->z_translation;
			if (fabs(z) >= 0.01) {
				TreeSupport* s1 = NULL;
				if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
					MainUndoStack->beginMacro(tr(""));
					MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
					MainUndoStack->push(new offsetValueChangeCommand(glwidget->selectID, z, 0, 0, this));
					MainUndoStack->endMacro();
				}
				else
					MainUndoStack->push(new offsetValueChangeCommand(glwidget->selectID, 0, 0, z, this));
			}
		}
	}
}

void MainWindow::showOffsetWidget()
{
	clickedButtonState(OFFSETBTN);
	if (OnOff_offset) {
		offsetButton->setStyleSheet(OnButton);
		offsetWidget->hide(); 
		OnOff_offset = false;
	}
	else {
		if (glwidget->selectID >= 0)
			setOffsetValue(interface1->find_instance(glwidget->selectID));
		offsetButton->setStyleSheet(OffButton);
		offsetWidget->show();
		OnOff_offset = true;
	}
}

void MainWindow::AddRotateValue(double angle, int x, int y, int z)
{
	//只支持一个方向的角度值，且比例值为1
	TreeSupport* s1 = NULL;
	if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
		MainUndoStack->beginMacro(tr("rotate delete supports"));
		MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
		MainUndoStack->push(new rotateValueChangeCommand(glwidget->selectID, angle, x, y, z, this));
		MainUndoStack->endMacro();
	}
	else
		MainUndoStack->push(new rotateValueChangeCommand(glwidget->selectID, angle, x, y, z, this));
}


void MainWindow::initRotateWidget()
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
	connect(x_rotate_spin, SIGNAL(valueChanged(double)), this, SLOT(xRotateValueChange(double)));

	y_rotate_spin = new QDoubleSpinBox();
	y_rotate_spin->setRange(-360.0, 360.0);
	y_rotate_spin->setSingleStep(5.0);
	y_rotate_spin->setSuffix(QStringLiteral("°"));
	y_rotate_spin->setStyleSheet(spinStyle);
	connect(y_rotate_spin, SIGNAL(valueChanged(double)), this, SLOT(yRotateValueChange(double)));

	z_rotate_spin = new QDoubleSpinBox();
	z_rotate_spin->setRange(-360.0, 360.0);
	z_rotate_spin->setSingleStep(5.0);
	z_rotate_spin->setSuffix(QStringLiteral("°"));
	z_rotate_spin->setStyleSheet(spinStyle);
	connect(z_rotate_spin, SIGNAL(valueChanged(double)), this, SLOT(zRotateValueChange(double)));

	QGridLayout* mainlayout = new QGridLayout(this);
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
	rotateWidget = new QStackedWidget(this);
	rotateWidget->setStyleSheet("background-color: rgba(225,225,225,100);");
	rotateWidget->setMinimumSize(130, 100);
	//rotateWidget->setMask(QPolygon(anomalyRect1));
	rotateWidget->addWidget(w);
	rotateWidget->hide();
}


void MainWindow::setRotateValue(ModelInstance* instance)
{
	x_rotate_spin->setValue(x_rotate);
	y_rotate_spin->setValue(y_rotate);
	z_rotate_spin->setValue(z_rotate);
}

void MainWindow::xRotateValueChange(double angle)
{
	if (glwidget->selectID >= 0) {
		double x = angle - x_rotate;
		
		if (angle == 360 || angle == -360)
		{
			x_rotate = 0;
			x_rotate_spin->setValue(0);
		}
		else
			x_rotate = angle;

		if (fabs(x) > 0.01) {
		
			TreeSupport* s1 = NULL;
			if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
				MainUndoStack->beginMacro(tr(""));
				MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
				MainUndoStack->push(new rotateValueChangeCommand(glwidget->selectID, x, 1, 0, 0, this));
				MainUndoStack->endMacro();
			}
			else
				MainUndoStack->push(new rotateValueChangeCommand(glwidget->selectID, x, 1, 0, 0, this));

		}
	}
}

void MainWindow::yRotateValueChange(double angle)
{
	if (glwidget->selectID >= 0) {
		double y = angle - y_rotate;
		
		if (angle == 360 || angle == -360)
		{
			y_rotate = 0;
			y_rotate_spin->setValue(0);
		}
		else
			y_rotate = angle;

		if (fabs(y) > 0.01) {
			TreeSupport* s1 = NULL;
			if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
				MainUndoStack->beginMacro(tr(""));
				MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
				MainUndoStack->push(new rotateValueChangeCommand(glwidget->selectID, y, 0, 1, 0, this));
				MainUndoStack->endMacro();
			}
			else
				MainUndoStack->push(new rotateValueChangeCommand(glwidget->selectID, y, 0, 1, 0, this));
	
		}
	}
}

void MainWindow::zRotateValueChange(double angle)
{
	if (glwidget->selectID >= 0) {
		double z = angle - z_rotate;
		
		if (angle == 360 || angle == -360)
		{
			z_rotate = 0;
			z_rotate_spin->setValue(0);
		}
		else
			z_rotate = angle;

		if (fabs(z) > 0.01) {
			TreeSupport* s1 = NULL;
			if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
				MainUndoStack->beginMacro(tr(""));
				MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
				MainUndoStack->push(new rotateValueChangeCommand(glwidget->selectID, z, 0, 0, 1, this));
				MainUndoStack->endMacro();
			}
			else
				MainUndoStack->push(new rotateValueChangeCommand(glwidget->selectID, z, 0, 0, 1, this));

		}
	}
}

void MainWindow::showRotateWidget()
{
	clickedButtonState(ROTATEBTN);
	if (OnOff_rotate) {
		rotateButton->setStyleSheet(OnButton);
		rotateWidget->hide();
		OnOff_rotate = false;
	}
	else {
		init_rotate();
		rotateButton->setStyleSheet(OffButton);
		rotateWidget->show();
		OnOff_rotate = true;
	}
}

void MainWindow::AddScaleValue(double x, double y, double z)
{
	if (x != 0)
		x_scale_spin->setValue(x_scale_spin->value() + x);
	if (y != 0)
		y_scale_spin->setValue(y_scale_spin->value() + y);
	if (z != 0)
		z_scale_spin->setValue(z_scale_spin->value() + z);
}

void MainWindow::initScaleWidget()
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
	x_scale_spin->setRange(1,100000);
	x_scale_spin->setSingleStep(5);
	x_scale_spin->setStyleSheet(spinStyle);
	x_scale_spin->setSuffix("%");
	connect(x_scale_spin, SIGNAL(valueChanged(double)), this, SLOT(xScaleValueChange(double)));

	y_scale_spin = new QDoubleSpinBox();
	y_scale_spin->setRange(1, 100000);
	y_scale_spin->setSingleStep(5);
	y_scale_spin->setSuffix("%");
	y_scale_spin->setDisabled(true);
	y_scale_spin->setStyleSheet(spinStyle);
	connect(y_scale_spin, SIGNAL(valueChanged(double)), this, SLOT(yScaleValueChange(double)));

	z_scale_spin = new QDoubleSpinBox();
	z_scale_spin->setRange(1, 100000);
	z_scale_spin->setSingleStep(5);
	z_scale_spin->setSuffix("%");
	z_scale_spin->setDisabled(true);
	z_scale_spin->setStyleSheet(spinStyle);
	connect(z_scale_spin, SIGNAL(valueChanged(double)), this, SLOT(zScaleValueChange(double)));

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
 
	QGridLayout* mainlayout = new QGridLayout(this);
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
	scaleWidget = new QStackedWidget(this);
	scaleWidget->setStyleSheet("background-color: rgba(225,225,225,100);");
	scaleWidget->setMinimumSize(200, 140);
	//scaleWidget->setMask(QPolygon(anomalyRect2));
	scaleWidget->addWidget(w);
	scaleWidget->hide();
}

void MainWindow::setScaleValue(ModelInstance* instance)
{
	x_scale_spin->setValue(instance->scaling_vector.x*100);
	y_scale_spin->setValue(instance->scaling_vector.y*100);
	z_scale_spin->setValue(instance->scaling_vector.z*100);
}

void MainWindow::setSizeValue(ModelInstance* instance)
{
	//得到实例的包围盒
	TriangleMesh mesh = instance->get_object()->volumes[0]->mesh;
	instance->transform_mesh(&mesh);
	BoundingBoxf3 box = mesh.bounding_box();

	x_size_label->setValue(box.size().x);
	y_size_label->setValue(box.size().y);
	z_size_label->setValue(box.size().z);
}

void MainWindow::xScaleValueChange(double value)
{
	if (glwidget->selectID >= 0) {
		ModelInstance* instance = IdToInstance();
		double xScale = value / 100;
		double x = xScale - instance->scaling_vector.x;
		double y = instance->scaling_vector.y*(x / instance->scaling_vector.x);
		double z = instance->scaling_vector.z*(x / instance->scaling_vector.x);
		if (fabs(x) > 0.01) {
			if (unify_scale->checkState() == Qt::Unchecked) {
				TreeSupport* s1 = NULL;
				if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
					MainUndoStack->beginMacro(tr("scale delete supports"));
					MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, x, 0, 0, this));
					MainUndoStack->endMacro();
				}
				else
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, x, 0, 0, this));

			}
			else {
				TreeSupport* s1 = NULL;
				if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
					MainUndoStack->beginMacro(tr("scale delete supports"));
					MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, x, y, z, this));
					MainUndoStack->endMacro();
				}
				else
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, x, y, z, this));

			}
		}
	}
}

void MainWindow::yScaleValueChange(double value)
{
	if (glwidget->selectID >= 0) {
		ModelInstance* instance = IdToInstance();
		double yScale = value / 100;
		double y = yScale - instance->scaling_vector.y;
		double x = instance->scaling_vector.x*(y / instance->scaling_vector.y);
		double z = instance->scaling_vector.z*(y / instance->scaling_vector.y);
		if (fabs(y) > 0.01) {
			if (unify_scale->checkState() == Qt::Unchecked) {
				TreeSupport* s1 = NULL;
				if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
					MainUndoStack->beginMacro(tr(""));
					MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, 0, y, 0, this));
					MainUndoStack->endMacro();
				}
				else
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, 0, y, 0, this));
			}
			else {
				TreeSupport* s1 = NULL;
				if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
					MainUndoStack->beginMacro(tr("scale delete supports"));
					MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, x, y, z, this));
					MainUndoStack->endMacro();
				}
				else
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, x, y, z, this));
			}
		}
	}
}

void MainWindow::zScaleValueChange(double value)
{
	if (glwidget->selectID >= 0) {
		ModelInstance* instance = IdToInstance();
		double zScale = value / 100;
		double z = zScale - instance->scaling_vector.z;
		double x = instance->scaling_vector.x*(z / instance->scaling_vector.z);
		double y = instance->scaling_vector.y*(z / instance->scaling_vector.z);
		if (fabs(z) > 0.01) {
			if (unify_scale->checkState() == Qt::Unchecked) {
				TreeSupport* s1 = NULL;
				if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
					MainUndoStack->beginMacro(tr(""));
					MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, 0, 0, z, this));
					MainUndoStack->endMacro();
				}
				else
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, 0, 0, z, this));
			}
			else {
				TreeSupport* s1 = NULL;
				if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
					MainUndoStack->beginMacro(tr("scale delete supports"));
					MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, x, y, z, this));
					MainUndoStack->endMacro();
				}
				else
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, x, y, z, this));
			}
		}
	}
}

void MainWindow::setUnifyScale(int state)
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

void MainWindow::showScaleWidget()
{
	clickedButtonState(SCALEBTN);
	if (OnOff_scale) {
		scaleButton->setStyleSheet(OnButton);
		scaleWidget->hide();
		OnOff_scale = false;
	}
	else {
		if (glwidget->selectID >= 0) {
			ModelInstance* i = interface1->find_instance(glwidget->selectID);
			setScaleValue(i);
			setSizeValue(i);
		}
		scaleButton->setStyleSheet(OffButton);
		scaleWidget->show();
		OnOff_scale = true;
	}
}

void MainWindow::showSetupDialog()
{
	if (!OnOff_setup) {
		//-------------------进入-----------
		clickedButtonState(SETUPBTN);
		OnOff_setup = true;
		setupButton->setStyleSheet(OffButton);
		//--------------------------------------
		setupDialog = new SetupDialog(this);
		setupDialog->exec();
		delete setupDialog;
	}
	else {
		//---------------退出------------
		OnOff_setup = false;
		setupButton->setStyleSheet(OnButton);
		//-----------------------------
	}
}

void MainWindow::defaultView()
{
	glwidget->ChangeView(glwidget->DEFAULT);
}

void MainWindow::overlookView()
{
	glwidget->ChangeView(glwidget->OVERLOOK);
}

void MainWindow::leftView()
{
	glwidget->ChangeView(glwidget->LEFT);
}

void MainWindow::rightView()
{
	glwidget->ChangeView(glwidget->RIGHT);
}

void MainWindow::frontView()
{
	glwidget->ChangeView(glwidget->FRONT);
}

void MainWindow::behindView()
{
	glwidget->ChangeView(glwidget->BEHIND);
}

void MainWindow::newJob()//新建项目
{
	interface1->clear_model();
	interface1->delete_all_support();
	glwidget->clearModelBuffer();
	glwidget->clear_volumes();
	glwidget->clearSupportBuffer();
	clearUndoStack();
}

void MainWindow::deleteSupport()
{
	TreeSupport* s1 = NULL;
	if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
		MainUndoStack->push(new offsetValueChangeCommand(glwidget->selectID, 0, 0, -interface1->dlprint->config.model_lift, this));
		MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
	}
}

void MainWindow::deleteAllSupport()
{
	for (ModelObjectPtrs::const_iterator o = interface1->model->objects.begin(); o != interface1->model->objects.end(); ++o) {
		for (ModelInstancePtrs::const_iterator i = (*o)->instances.begin(); i != (*o)->instances.end(); ++i) {
			if ((*i)->exist) {
				size_t id = interface1->find_id(*i);

				TreeSupport* s1 = NULL;
				if (interface1->dlprint->chilck_tree_support(id, s1)) {
					MainUndoStack->push(new offsetValueChangeCommand(id, 0, 0, -interface1->dlprint->config.model_lift, this));
					MainUndoStack->push(new deleteSupportsCommand(id, s1, this));
				}
			}
		}
	}
}

void MainWindow::modelSelect()
{
	ModelInstance* instance = IdToInstance();
	setOffsetValue(instance);
	setRotateValue(instance);
	setScaleValue(instance);
	setSizeValue(instance);
}


ModelInstance* MainWindow::IdToInstance()
{
	return interface1->find_instance(glwidget->selectID);
}


void MainWindow::generateSupport()
{
	//----------------进入-----------------
	clickedButtonState(SUPPORTBTN);
	supportButton->setStyleSheet(OffButton);
	OnOff_support = true;
	//-------------------------------

	if (glwidget->selectID >= 0) {
		showProgress(SUPPORTBTN);
		P(10);
		TreeSupport* s = new TreeSupport();
		TreeSupport* s1 = new TreeSupport();
		if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
			interface1->generate_id_support(glwidget->selectID, s, progress);
			MainUndoStack->beginMacro(tr(""));
			MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
			MainUndoStack->push(new addSupportsCommand(glwidget->selectID, s, this, progress));
			MainUndoStack->endMacro();
		}
		else {
			MainUndoStack->push(new offsetValueChangeCommand(glwidget->selectID, 0, 0, interface1->dlprint->config.model_lift, this));
			interface1->generate_id_support(glwidget->selectID, s, progress);
			MainUndoStack->push(new addSupportsCommand(glwidget->selectID, s, this, progress));
		}
		P(100);
		hideProgress();

	}
	else
	{
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("请选中一个模型。"));
	}

	//----------------退出-----------------
	supportButton->setStyleSheet(OnButton);
	OnOff_support = false;
	//-------------------------------------
}

void MainWindow::generateAllSupport()
{
	for (ModelObjectPtrs::const_iterator o = interface1->model->objects.begin(); o != interface1->model->objects.end(); ++o) {
		for (ModelInstancePtrs::const_iterator i = (*o)->instances.begin(); i != (*o)->instances.end(); ++i) {
			if ((*i)->exist) {
				size_t id = interface1->find_id(*i);

				showProgress(SUPPORTBTN);
				P(10);
				TreeSupport* s = new TreeSupport();
				TreeSupport* s1 = new TreeSupport();
				if (interface1->dlprint->chilck_tree_support(id, s1)) {
					interface1->generate_id_support(id, s, progress);
					MainUndoStack->beginMacro(tr(""));
					MainUndoStack->push(new deleteSupportsCommand(id, s1, this));
					MainUndoStack->push(new addSupportsCommand(id, s, this, progress));
					MainUndoStack->endMacro();
				}
				else {
					MainUndoStack->push(new offsetValueChangeCommand(id, 0, 0, interface1->dlprint->config.model_lift, this));
					interface1->generate_id_support(id, s, progress);
					MainUndoStack->push(new addSupportsCommand(id, s, this, progress));
				}
				P(100);
				hideProgress();
			}
		}
	}
}

void MainWindow::DlpPrintLoadSetup()
{
	DLPrintConfig* config = &interface1->dlprint->config;
	config->normIlluTime.setInt(setupDialog->normIlluSpin->value());
	config->norm_inttersity.setInt(setupDialog->norm_inttersity_spin->value());
	config->overIlluTime = setupDialog->overIlluSpin->value();
	config->overLayer.setInt(setupDialog->overLayerSpin->value());
	config->over_inttersity.setInt(setupDialog->over_inttersity_spin->value());

	config->support_top_height = setupDialog->top_height_spin->value();
	config->support_top_radius = setupDialog->top_radius_spin->value();
	config->support_radius = setupDialog->support_radius_spin->value();
	config->support_bottom_radius = setupDialog->bottom_radius_spin->value();
	config->space.setInt(setupDialog->support_space_spin->value());
	config->angle.setInt(setupDialog->support_angle_spin->value());
	config->leaf_num = setupDialog->leaf_num_spin->value();
	config->model_lift = setupDialog->model_lift_spin->value();

	config->hollow_out = int(setupDialog->hollow_out_box->isChecked());

	if (setupDialog->fill_pattern_combo->currentIndex() == 0)
		config->fill_pattern.value = ipHoneycomb;
	else if (setupDialog->fill_pattern_combo->currentIndex() == 1)
		config->fill_pattern.value = ip3DSupport;

	config->wall_thickness = setupDialog->wall_thickness_spin->value();
	config->fill_density = setupDialog->density_spin->value();

	if (setupDialog->thicknessCombo->currentText() == "0.05mm")
		config->layer_height = 0.05;
	else if (setupDialog->thicknessCombo->currentText() == "0.1mm")
		config->layer_height = 0.1;
	else if (setupDialog->thicknessCombo->currentText() == "0.025mm")
		config->layer_height = 0.025;

	config->raft_layers.setInt(setupDialog->raftSpin->value());
	config->raft_offset.setInt(setupDialog->raftOffsetSpin->value());
	config->arrange_space = setupDialog->arrange_space_spin->value();
	config->threads.setInt(setupDialog->threadSpin->value());
}

void MainWindow::saveView()
{
	QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	QString path = QFileDialog::getSaveFileName(this, QStringLiteral("保存切片文件"), desktop, "histogram file(*.bmp)");
	if (path.size() > 0) {
		QByteArray a = path.toLocal8Bit();
		char* _path = a.data();
		glwidget->saveOneView(_path);
	}
}

void MainWindow::SupportEdit()
{
	clickedButtonState(SUPPORTEDITBTN);

	if (!OnOff_supportEdit)
	{
		//---------------------进入-------------
		OnOff_supportEdit = true;
		supportEditButton->setStyleSheet(OffButton);
		//-------------------------------------

		if (glwidget->selectID >= 0) {

			//生成支撑编辑“撤销重做”栈并激活
			SupportEditUndoStack = new QUndoStack(this);
			MainUndoGroup->addStack(SupportEditUndoStack);
			MainUndoGroup->setActiveStack(SupportEditUndoStack);

			glwidget->supportEditChange();
		}
		else
		{
			QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("请选中一个已生成支撑的模型。"));
			//----------------退出----------------
			OnOff_supportEdit = false;
			supportEditButton->setStyleSheet(OnButton);
			//-----------------------------------
			return;
		}
	}
	else {
		//-----------退出支撑编辑模式，更新支撑点------------
		showProgress(SUPPORTEDITBTN);
		progress->setValue(10);

		auto p = interface1->dlprint->treeSupports.find(glwidget->selectID);
		TreeSupport* t = new TreeSupport();

		stl_vertex v;
		for (auto i = treeSupportsExist.begin(); i != treeSupportsExist.end(); ++i) {
			if ((*i).exist) {
				v.x = (*i).p.x;
				v.y = (*i).p.y;
				v.z = (*i).p.z;

				if (p != interface1->dlprint->treeSupports.end()) {
					if ((*p).second->support_point.size() != 0) {
						for (auto i1 = (*p).second->support_point.begin(); i1 != (*p).second->support_point.end(); ++i1) {
							if (equal_vertex(*i1, v)) {
								t->support_point.push_back(v);
								break;
							}
							if (i1 == (*p).second->support_point.end() - 1)
								t->support_point_face.push_back(v);
						}
					}
					else
						t->support_point_face.push_back(v);
				}
				else
					t->support_point_face.push_back(v);
			}
		}

		t->generate_tree_support(glwidget->find_model(glwidget->selectID),interface1->dlprint->config.leaf_num,
			interface1->dlprint->config.threads,progress,interface1->dlprint->config.support_top_height);
		
		if (p != interface1->dlprint->treeSupports.end()) {
			MainUndoStack->beginMacro(tr(""));
			MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID,(*p).second,this));
			MainUndoStack->push(new addSupportsCommand(glwidget->selectID, t, this,progress));
			MainUndoStack->endMacro();
		}
		else {
			MainUndoStack->push(new addSupportsCommand(glwidget->selectID, t, this,progress));
		}
		hideProgress();
		//------------------------------------------------------------

		//删除支撑编辑“撤销重做”栈,激活主栈
		delete SupportEditUndoStack;
		MainUndoGroup->setActiveStack(MainUndoStack);

		glwidget->supportEditChange();
		//清空
		treeSupportsExist.clear();

		//----------------退出----------------
		OnOff_supportEdit = false;
		supportEditButton->setStyleSheet(OnButton);
		//-----------------------------------
	}
}

void MainWindow::slice()
{
	//---------------进入---------------
	clickedButtonState(SLICEBTN);
	OnOff_slice = true;
	sliceButton->setStyleSheet(OffButton);
	//-------------------------------------

	if (interface1->model->objects.size() == 0) {
		//----------------退出--------------
		OnOff_slice = false;
		sliceButton->setStyleSheet(OnButton);
		//-----------------------------------
		return;
	}

	if (!glwidget->checkConfine()) {
		QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
		QString path = QFileDialog::getSaveFileName(this, QStringLiteral("保存切片文件"), desktop, "histogram file(*.sm)");
		if (path.size() > 0) {
			//在用户目录下建立一个保存切片的文件夹
			QString name = getFileLocation();
			name.append("/images");
			clearDir(name);

			QDir* file = new QDir(name);
			bool ret = file->exists();
			if (!ret) {
				bool ok = file->mkdir(name);
				if (!ok)
					exit(2);
			}

			//保存一张截图
			QString view = name;
			view.append("/A_Front.bmp");
			QByteArray a = view.toLocal8Bit();
			char* _path = a.data();
			glwidget->saveOneView(_path);

			showProgress(SLICEBTN);

			P(10);
			interface1->generate_all_inside_support();
			P(20);
			interface1->dlprint->slice(glwidget->return_support_model(),progress);
			P(98);
			interface1->dlprint->savePNG(name);
			P(99);
			JlCompress::compressDir(path, name);
			P(100);

			//清空切片文件夹
			clearDir(name);
			delete file;

			hideProgress();

			switch (QMessageBox::question(this, QStringLiteral("提示"), QStringLiteral("切片完成，是否预览切片文件？"), QMessageBox::Cancel, QMessageBox::Ok))
			{
			case QMessageBox::Ok:
				showPreviewWidget1(path);
				break;
			case QMessageBox::Cancel:
				break;
			default:
				break;
			}
		
		}
	}
	else {
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("模型超过边界，无法切片。"));
	}

	//----------------退出--------------
	OnOff_slice = false;
	sliceButton->setStyleSheet(OnButton);
	//-----------------------------------
}

void MainWindow::initProgress()
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
	progressWidget = new QStackedWidget(this);
	progressWidget->setStyleSheet("background-color: rgba(100,100,100,225);");
	progressWidget->setMask(QPolygon(progressRect));
	progressWidget->setMinimumSize(QSize(300, 80));
	progressWidget->addWidget(widget);
	progressWidget->hide();

}

void MainWindow::showProgress(int btn)
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

void MainWindow::hideProgress()
{
	progressWidget->hide();
}

void MainWindow::P(size_t i)
{
	progress->setValue(i);
	QCoreApplication::processEvents();
}

bool MainWindow::clearDir( QString &path)
{
	if (path.isEmpty()) {
		return false;
	}
	QDir dir(path);
	if (!dir.exists()) {
		return true;
	}
	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); //设置过滤  
	QFileInfoList fileList = dir.entryInfoList(); // 获取所有的文件信息  
	foreach(QFileInfo file, fileList) { //遍历文件信息  
		if (file.isFile()) { // 是文件，删除  
			file.dir().remove(file.fileName());
		}
		else if(file.isDir()){ // 递归删除  
			clearDir(file.absoluteFilePath());
		}
	}
	return dir.rmpath(dir.absolutePath()); // 删除文件夹  
}

void MainWindow::showPreviewWidget1(QString zipPath)
{
	//解压zip包到用户目录下
	QString name = getFileLocation();
	name.append("/images");
	clearDir(name);

	QDir file(name);
	bool ret = file.exists();
	if (!ret) {
		bool ok = file.mkdir(name);
		if (!ok)
			exit(2);
	}

	if (extractZipPath(zipPath)) {
		JlCompress::extractDir(zipPath, name);

		previewDialog = new PreviewDialog();
		if (previewDialog->readIni(name)) {
			previewDialog->exec();
			delete previewDialog;
		}
	}
	else {
		QString text(QStringLiteral("打开文件"));
		text.append(zipPath);
		text.append(QStringLiteral("失败！"));
		QMessageBox::about(this, QStringLiteral("警告"), text);
	}
}

bool MainWindow::extractZipPath(QString zipPath)
{
	bool bmpBool = false;
	bool pngBool = false;
	bool iniBool = false;
	QStringList list = JlCompress::getFileList(zipPath);
	for (auto l = list.begin(); l != list.end(); ++l) {
		if ((*l).right(4).compare(".bmp") && (*l).right(4).compare(".png") && (*l).right(4).compare(".ini"))
			return false;
		if (!(*l).right(4).compare(".bmp") && !bmpBool)
			bmpBool = true;
		if (!(*l).right(4).compare(".png") && !pngBool)
			pngBool = true;
		if (!(*l).right(4).compare(".ini") && !iniBool)
			iniBool = true;
	}
	return bmpBool && pngBool && iniBool;
}

void MainWindow::saveModelSupport()
{
	if (interface1->model->objects.size() > 0) {
		QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
		QString path = QFileDialog::getSaveFileName(this, QStringLiteral("保存切片文件"), desktop, "histogram file(*.stl)");
		if (!path.isEmpty()) {
			TriangleMesh mesh = glwidget->saveSupport();
			interface1->wirteStlBinary(path.toStdString(), mesh);
		}
	}
}

void MainWindow::showAboutDialog()
{
	aboutDialog = new AboutDialog();
	aboutDialog->exec();
	delete aboutDialog;
}

void MainWindow::autoArrange()
{
	//未删除支撑？？？
	Pointfs positions = interface1->autoArrange(interface1->dlprint->config.arrange_space);

	glwidget->clearSupportBuffer();

	MainUndoStack->beginMacro(tr(""));
	for (ModelObjectPtrs::const_iterator o = interface1->model->objects.begin(); o != interface1->model->objects.end(); ++o) {
	    for (ModelInstancePtrs::const_iterator i = (*o)->instances.begin(); i != (*o)->instances.end(); ++i) {
			if ((*i)->exist) {
				size_t id = interface1->find_id(*i);
				
				//将所有实例在xy方向上重新排列
				Pointf p = positions.back();
				positions.pop_back();
				double x = p.x - (*i)->offset.x;
				double y = p.y - (*i)->offset.y;
				MainUndoStack->push(new offsetValueChangeCommand(id, x, y, 0, this));
				TreeSupport* s1 = NULL;
				if (interface1->dlprint->chilck_tree_support(id, s1)) {
					s1->support_offset(x, y);
					glwidget->initTreeSupport_id(id, s1, NULL);
				}
			}
	    }
	    (*o)->invalidate_bounding_box();
	}
	MainUndoStack->endMacro();
}

void MainWindow::duplicate(size_t num, bool arrange)
{
	MainUndoStack->beginMacro(tr(""));
	int distance = 0;

	TreeSupport* s1 = NULL;
	bool ret = interface1->dlprint->chilck_tree_support(glwidget->selectID, s1);

	for (int i = 0; i < num; ++i) {
		ModelInstance* s = interface1->addInstance(glwidget->selectID);
		size_t id = interface1->find_id(s);
		MainUndoStack->push(new AddModelCommand(id, this));
		if (ret) {
			TreeSupport* ss = new TreeSupport();
			ss->support_point = s1->support_point;
			ss->support_point_face = s1->support_point_face;
			ss->tree_support_bole = s1->tree_support_bole;
			ss->tree_support_bottom = s1->tree_support_bottom;
			ss->tree_support_branch = s1->tree_support_branch;
			ss->tree_support_leaf = s1->tree_support_leaf;
			ss->tree_support_node = s1->tree_support_node;
			MainUndoStack->push(new addSupportsCommand(id, ss, this, NULL));
		}

		//if (!arrange) {
		//	distance = distance + 5;
		//	s->offset.x += distance;
		//	s->offset.y += distance;
		//}
	}
	//if(arrange)
		autoArrange();
	MainUndoStack->endMacro();
}


void MainWindow::_duplicate()
{
	if (glwidget->selectID >= 0) {
		bool ret;
		int num = QInputDialog::getInt(this, QStringLiteral("复制"), QStringLiteral("请输入复制模型的个数："), 1, 1, 50, 1, &ret);
		if (ret) {
			//switch (QMessageBox::question(this, QStringLiteral("提示"), QStringLiteral("复制后进行自动排列？"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes))
			//{
			//case QMessageBox::Yes:
				duplicate(num,true);
			//	break;
			//case QMessageBox::No:
			//	duplicate(num,false);
			//	break;
			//}
		}
	}
	else
	{
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("请选中一个模型进行复制。"));
		return;
	}
}

void MainWindow::initUndo()
{
	MainUndoGroup = new QUndoGroup(this);
	MainUndoStack = new QUndoStack(this);
	MainUndoGroup->addStack(MainUndoStack);
	MainUndoGroup->setActiveStack(MainUndoStack);
}

void MainWindow::addModelBuffer(size_t id)
{
	ModelInstance* instance = interface1->find_instance(id);
	glwidget->addModelBuffer(instance);
	instance->setExist();
}

void MainWindow::deleteModelBuffer(size_t id)
{
	glwidget->delModelBuffer(id);
	interface1->find_instance(id)->setExist(false);
}

//传入移动的距离数值
void MainWindow::offsetValueChange(size_t id,double x, double y, double z)
{
	ModelInstance* i = interface1->find_instance(id);
	i->offset.x += x;
	i->offset.y += y;
	i->z_translation += z;

	if (glwidget->selectID == id)
		setOffsetValue(i);

	i->update_attribute();
	glwidget->updateConfine();
}

//传入缩放的比例值
void MainWindow::scaleValueChange(size_t id, double x, double y, double z)
{
	ModelInstance* i = interface1->find_instance(id);
	i->scaling_vector.x += x;
	i->scaling_vector.y += y;
	i->scaling_vector.z += z;

	if (glwidget->selectID == id) {
		setScaleValue(i);
		setSizeValue(i);
	}

	i->update_attribute();
	glwidget->updateConfine();
}

//传入旋转的角度的数值
void MainWindow::rotateValueChange(size_t id, double angle, int x, int y, int z)
{
	ModelInstance* i = interface1->find_instance(id);
	QMatrix4x4 m;
	m.rotate(angle, x, y, z);
	i->rotation_M = m*i->rotation_M;
	
	i->update_attribute();
	glwidget->updateConfine();
}

void MainWindow::addSupports(size_t id, TreeSupport* s, QProgressBar* progress)
{
	//支撑数值存入treeSupports
	interface1->dlprint->insertSupports(id, s);
	//渲染支撑
	glwidget->initTreeSupport_id(id, s, progress);
}

void MainWindow::deleteSupports(size_t id)
{
	//删除treeSupports中的指定支撑
	interface1->dlprint->delete_tree_support(id);
	//删除支撑缓存区
	glwidget->delSupportBuffer(id);
}

void MainWindow::clearUndoStack()
{
	MainUndoGroup->activeStack()->clear();
}

//支撑编辑可以100个点为一个区间，每次删减操作最多只对100个点进行初始化，区间点过少会消耗过多内存??????
void MainWindow::addOneSupport(Pointf3 p)
{
	Pointf3Exist temp = { true,p };
	treeSupportsExist.push_back(temp);
	SupportEditUndoStack->push(new addOneSupportCommand(treeSupportsExist.size(), this));
}

void MainWindow::deleteOneSupport(size_t id)
{
	SupportEditUndoStack->push(new deleteOneSupportCommand(id, this));
}

void MainWindow::addOneSupportUndo(size_t id)
{
	auto p = treeSupportsExist.begin() + id;
	(*p).exist = true;
	glwidget->initSupportEditBuffer(id / 100);
	glwidget->update();
}

void MainWindow::deleteOneSupportUndo(size_t id)
{
	auto p = treeSupportsExist.begin() + id;
	(*p).exist = false;
	glwidget->initSupportEditBuffer(id / 100);
	glwidget->update();
}

void MainWindow::setPersperctive()
{
	glwidget->setPresprective();
}

void MainWindow::setOrthogonality()
{
	glwidget->setOrthogonality();
}

void MainWindow::init_rotate()
{
	x_rotate = 0;
	y_rotate = 0;
	z_rotate = 0;
	x_rotate_spin->setValue(x_rotate);
	y_rotate_spin->setValue(y_rotate);
	z_rotate_spin->setValue(z_rotate);
}

void MainWindow::initDlprinter()
{
	QString file = getFileLocation();
	file.append("/dlprinter.ini");
	QFile dir(file);
	if (dir.exists()) {
		QSettings* readini = new QSettings(file, QSettings::IniFormat);
		//读取打印设置
		QString name = readini->value("/dlprinter/name").toString();
		delete readini;
		if (name == "S288")
			dlprinter = new DLPrinter(S288);
		else if (name == "S250")
			dlprinter = new DLPrinter(S250);
		else
			dlprinter = new DLPrinter(S288);
	}
	else
		dlprinter = new DLPrinter(S288);
	dlprinter->readPrinter();
}

void MainWindow::initSetupDialog()
{
	setupDialog = new SetupDialog(this);
	DlpPrintLoadSetup();
	delete setupDialog;
}

void MainWindow::dlprinterChange(QString name)
{
	QString file = getFileLocation();
	file.append("/dlprinter.ini");
	QSettings* writeini = new QSettings(file, QSettings::IniFormat);
	writeini->clear();
	
	if (name == "S288") {
		writeini->setValue("/dlprinter/name", "S288");
		dlprinter->printer = S288;
		dlprinter->readPrinter();
	}
	else if (name == "S250") {
		writeini->setValue("/dlprinter/name", "S250");
		dlprinter->printer = S250;
		dlprinter->readPrinter();
	}
	delete writeini;

	glwidget->dlprinterChange();
	glwidget->updateConfine();
	glwidget->update();
}

void MainWindow::checkUpdate()
{
	process = new QProcess();
	//connect(process, SIGNAL(started()), SLOT(started()));
	connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(UpdateFinished(int)));
	//connect(process, SIGNAL(stateChanged(QProcess::ProcessState)), SLOT(stateChanged()));

	QString path = QCoreApplication::applicationDirPath();
	qDebug() << path;
	process->start(path + "/updater.exe");
	//process->start("cmd");

	if (!process->waitForStarted())
		qDebug() << "failure!";
	else
		qDebug() << "success!";
}

void MainWindow::UpdateFinished(int a)
{
	qDebug() << "updateFinised: " << a;
	delete process;
	//正常更新
	//if (a == 1)
	//	_exit();
}


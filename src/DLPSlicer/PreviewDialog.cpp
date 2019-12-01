#include "PreviewDialog.h"

#include <qevent.h>
#include <qicon.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qtextstream.h>

//切片生成的2D图片名字前缀
static QString ImageName("slice");

//切片生成的配置文件的文件名
static QString buildsciptName("buildscipt");

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
	mainLayout->addLayout(layout1, 0, 0);
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

	QDir* dir = new QDir(path);
	QStringList filter;
	//得到文件名列表
	QList<QFileInfo>* fileInfo = new QList<QFileInfo>(dir->entryInfoList(filter));

	int normIlluminationTime, numberSlices;

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
	slider->setRange(0, 0);
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

void PreviewView::wheelEvent(QWheelEvent* event)
{
	int numDegrees = event->delta() / 8;//滚动的角度，*8就是鼠标滚动的距离
	int numSteps = numDegrees / 15;//滚动的步数，*15就是鼠标滚动的角度
	_scale = _scale + numSteps * 0.1;
	if (_scale > 3)
		_scale = 3;
	if (_scale < 0.1)
		_scale = 0.1;
	image->setScale(_scale);
	update();
}

void PreviewView::mouseMoveEvent(QMouseEvent* event)
{
	image->moveBy(event->x() - x, event->y() - y);
	x = event->x();
	y = event->y();
	update();
}

void PreviewView::mousePressEvent(QMouseEvent* event)
{
	x = event->x();
	y = event->y();
}
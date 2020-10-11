#include "PreviewWidget.h"
#include "mainwindow.h"

#include <qpainter.h>

//InfoWidget

InfoWidget::InfoWidget(QWidget* parent)
	:QFrame(parent)
{
	QLabel* lab1 = new QLabel(QStringLiteral("总层数："));
	m_layersLab = new QLabel("");

	QLabel* lab2 = new QLabel(QStringLiteral("长宽高："));
	m_sizeLab = new QLabel("");

	QLabel* lab3 = new QLabel(QStringLiteral("体积："));
	m_volumeLab = new QLabel("");

	QGridLayout* layout = new QGridLayout(this);
	layout->addWidget(lab1, 0, 0);
	layout->addWidget(m_layersLab, 0, 1);
	layout->addWidget(lab2, 1, 0);
	layout->addWidget(m_sizeLab, 1, 1);
	layout->addWidget(lab3, 2, 0);
	layout->addWidget(m_volumeLab, 2, 1);

	this->setLayout(layout);
}


//PreviewTopWidget

PreviewTopWidget::PreviewTopWidget(MainWindow* parent)
	:m_parent(parent)
{
	m_slider = new QSlider(m_parent);
	m_slider->setRange(0, 0);
	(void)connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(slot_layerChanged(int)));

	m_saveBtn = new PushButton(QIcon(":/icon/images/load_b.png"), "", m_parent);
	m_saveBtn->setIconSize(QSize(120, 140));
	m_saveBtn->setStyleSheet(OnButton);
	m_saveBtn->setFixedSize(60, 70);
	m_saveBtn->setToolTip(QStringLiteral("保存"));
	(void)connect(m_saveBtn, SIGNAL(clicked(bool)), m_parent, SLOT(slot_saveSlice()));

	m_curLayerLab = new QLabel(m_parent);
	m_curLayerLab->setText(QString::number(m_slider->maximum()));
	QFontMetrics fontMetric(m_curLayerLab->font());
	m_curLayerLab->setFixedSize(50, fontMetric.height());
	m_curLayerLab->setAlignment(Qt::AlignCenter);

	m_infoWidget = new InfoWidget(m_parent);
	m_infoWidget->setFixedSize(200, 100);

	this->setEnable(false);
	this->HideWidget();
}

PreviewTopWidget::~PreviewTopWidget()
{
	delete m_slider;
	delete m_curLayerLab;
	delete m_saveBtn;
	delete m_infoWidget;
}

void PreviewTopWidget::resize(const QRect& rect)
{
	int r = rect.right();
	int l = rect.left();
	int t = rect.top();
	int b = rect.bottom();

	int yh = rect.center().y();
	int xh = rect.center().x();

	m_saveBtn->move(l + 10, t + 10);
	m_curLayerLab->move(r - 60, t + 10);
	m_slider->setFixedSize(50, rect.height() - m_curLayerLab->height() - 30);
	m_slider->move(r - 60, t + 30);
	m_infoWidget->move(l + 10, b - m_infoWidget->height() - 10);
}

void PreviewTopWidget::reload(size_t max)
{
	m_slider->setRange(0, max - 1);
	this->setEnable(true);
	m_slider->setValue(0);
}

void PreviewTopWidget::setEnable(bool val)
{
	m_slider->setEnabled(val);
	m_saveBtn->setEnabled(val);
	if (!val)
		m_curLayerLab->setText(QString());

	m_infoWidget->m_layersLab->setText("");
	m_infoWidget->m_sizeLab->setText("");
	m_infoWidget->m_volumeLab->setText("");
}

void PreviewTopWidget::slot_layerChanged(int num)
{
	m_curLayerLab->setText(QString::number(num));
}

void PreviewTopWidget::HideWidget()
{
	m_slider->hide();
	m_saveBtn->hide();
	m_curLayerLab->hide();
	m_infoWidget->hide();
}

void PreviewTopWidget::ShowWidget()
{
	m_slider->show();
	m_saveBtn->show();
	m_curLayerLab->show();
	m_infoWidget->show();
}

//PreviewWidget

PreviewWidget::PreviewWidget(DLPrint* print)
	:m_pdlprint(print)
{
	//设置画布颜色
	//QPalette palette;
	//palette.setColor(QPalette::Background, Qt::black);
	//this->setAutoFillBackground(true);
	//this->setPalette(palette);
}

PreviewWidget::~PreviewWidget()
{

}

void PreviewWidget::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);

	QPainter painter(this);

	if (m_dirty) {
		painter.setPen(Qt::black);
		painter.setFont(QFont("Arial", 30));
		painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("预览"));
	}
	else {
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
		painter.setBrush(QBrush(Qt::black));
		painter.setPen(QPen(Qt::Dense7Pattern, 1));

		auto p = m_pdlprint->layer_qt_path.find(m_cur_layer);
		if (p == m_pdlprint->layer_qt_path.end()) {
			painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("预览"));
			return;
		}
		
		painter.scale(m_scale, m_scale);
		painter.drawPath(*(p->second));
	}

}

void PreviewWidget::reload()
{
	m_dirty = false;
}

void PreviewWidget::slot_layerChanged(int num)
{
	m_cur_layer = num;
	update();
}

void PreviewWidget::resizeEvent(QResizeEvent* event)
{
	Q_UNUSED(event);

	float s1 = (float)this->width() / (float)1920;
	float s2 = (float)this->height() / (float)1080;
	m_scale = s1 > s2 ? s2 : s1;

	emit sig_sizeChange();
}

//bool PreviewDialog::readIni(QString path)
//{
//	this->previewWidget->path = path;
//	this->previewWidget->path.append("/");
//	this->previewWidget->path.append(ImageName);
//
//	QDir* dir = new QDir(path);
//	QStringList filter;
//	//得到文件名列表
//	QList<QFileInfo>* fileInfo = new QList<QFileInfo>(dir->entryInfoList(filter));
//
//	int normIlluminationTime, numberSlices;
//
//	for (auto f = fileInfo->begin(); f != fileInfo->end(); ++f) {
//		if ((*f).baseName() == buildsciptName) {
//			QFile file((*f).filePath());
//			if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
//				return false;
//			QTextStream in(&file);
//			QString line = in.readLine();
//			while (!line.isNull())
//			{
//				if (line.contains("number of slices")) {
//					int a = line.lastIndexOf("=");
//					QString num = line.mid(a + 2);//得到总层数
//					previewWidget->slider->setRange(0, num.toInt() - 1);
//					numberSlices = num.toInt();
//
//					num.append(QStringLiteral("层"));
//					numberSlicesLabel->setText(num);
//					//初始化
//					previewWidget->slider->setValue(0);
//					previewWidget->image->setScale(1);
//					previewWidget->image->setPos(0, 0);
//					previewWidget->_scale = 1;
//					previewWidget->x = 0;
//					previewWidget->y = 0;
//
//					//读取第一张图片
//					QString s = path;
//					s.append("/");
//					s.append(ImageName);
//					s.append("0.png");
//					previewWidget->image->updateItem(s);
//
//				}
//				else if (line.contains("model volume")) {
//					int a = line.lastIndexOf("=");
//					QString num = line.mid(a + 2);
//					num.append("ml");
//					modelVolumeLabel->setText(num);
//				}
//				else if (line.contains("norm illumination time")) {
//					int a = line.lastIndexOf("=");
//					normIlluminationTime = line.mid(a + 2).toInt();
//				}
//				line = in.readLine();
//			}
//		}
//	}
//
//	//计算打印时间
//	//int printTime= normIlluminationTime*
//
//	return true;
//}
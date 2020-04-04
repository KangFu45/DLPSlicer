#include "PreviewWidget.h"

#include "mainwindow.h"

#include <qpainter.h>

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
	m_saveBtn->setToolTip(QStringLiteral("±£´æ"));
	(void)connect(m_saveBtn, SIGNAL(clicked(bool)), m_parent, SLOT(slot_saveSlice()));

	m_curLayerLab = new QLabel(m_parent);
	m_curLayerLab->setText(QString::number(m_slider->maximum()));
	QFontMetrics fontMetric(m_curLayerLab->font());
	m_curLayerLab->setFixedSize(50, fontMetric.height());
	m_curLayerLab->setAlignment(Qt::AlignCenter);

	this->setEnable(false);
	this->HideWidget();
}

PreviewTopWidget::~PreviewTopWidget()
{
	delete m_slider;
	delete m_curLayerLab;
	delete m_saveBtn;
}

void PreviewTopWidget::resize(const QRect& rect)
{
	int r = rect.right();
	int l = rect.left();
	int t = rect.top();

	int yh = rect.center().y();
	int xh = rect.center().x();

	m_saveBtn->move(l + 10, t + 10);
	m_curLayerLab->move(r - 60, t + 10);
	m_slider->setFixedSize(50, rect.height() - m_curLayerLab->height() - 30);
	m_slider->move(r - 60, t + 30);
}

void PreviewTopWidget::reload(size_t max)
{
	m_slider->setRange(0, max);
	this->setEnable(true);
	m_slider->setValue(0);
}

void PreviewTopWidget::setEnable(bool val)
{
	m_slider->setEnabled(val);
	m_saveBtn->setEnabled(val);
	if (!val)
		m_curLayerLab->setText(QString());
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
}

void PreviewTopWidget::ShowWidget()
{
	m_slider->show();
	m_saveBtn->show();
	m_curLayerLab->show();
}

//PreviewWidget

PreviewWidget::PreviewWidget(DLPrint* print)
	:m_pdlprint(print)
{
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
		painter.drawText(rect(), Qt::AlignCenter, "helloQt");
	}
	else {
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
		painter.setBrush(QBrush(QColor(150, 150, 150)));
		painter.setPen(QPen(Qt::Dense7Pattern, 1));

		auto p = m_pdlprint->layer_qt_path.find(m_cur_layer);
		if (p == m_pdlprint->layer_qt_path.end()) {
			painter.drawText(rect(), Qt::AlignCenter, "helloQt");
			return;
		}
		
		painter.scale(m_scale, m_scale);
		painter.drawPath(p->second);
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
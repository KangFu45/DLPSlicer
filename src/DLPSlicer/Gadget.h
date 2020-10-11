#pragma once

#include <qpushbutton.h>
#include <qgridlayout.h>
#include <qcoreapplication.h>
#include <qprogressbar.h>
#include <qdir.h>

//���Զ���Ի�����вü��Ķ���εĵ�
static QVector<QPoint> progressRect = { QPoint(3,0)
,QPoint(297,0),QPoint(300,3),QPoint(300,77)
,QPoint(297,80),QPoint(3,80),QPoint(0,77),QPoint(0,3) };

//���Ա������ť��QSS
static QString OnButton("border: 1px outset white; border-radius: 5px;  background-color: rgba(100,225,50,225);");
//���ܱ������ť��QSS
static QString OffButton("border: 1px outset white; border-radius: 5px; background-color: rgba(200,200,200,150);");

inline bool clearDir(QString path)
{
	if (path.isEmpty())
		return false;

	QDir dir(path);
	if (!dir.exists())
		return true;

	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); //���ù���  
	foreach(QFileInfo file, dir.entryInfoList()) { //�����ļ���Ϣ  
		if (file.isFile()) // ���ļ���ɾ��  
			file.dir().remove(file.fileName());
		else if (file.isDir()) // �ݹ�ɾ��  
			clearDir(file.absoluteFilePath());
	}
	return dir.rmpath(dir.absolutePath()); // ɾ���ļ���  
}

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
		this->checked = !this->checked;
	};
	bool c_isChecked() { return this->checked; };

private:
	//����ť�����Ƿ��µ�����
	//��������Ƿ��»���Ĳ���
	bool checked = { false };
};

class ProgressWidget : public QWidget
{
private:
	QLabel* m_progressLabel;

public:
	QProgressBar* m_progressBar;

	ProgressWidget(QWidget* parent = nullptr)
		:QWidget(parent)
	{
		m_progressLabel = new QLabel;
		QFont font("ZYSong18030", 15);
		m_progressLabel->setFont(font);
		m_progressLabel->setStyleSheet("color: white");
		m_progressLabel->setText("waiting ......");
		m_progressBar = new QProgressBar;
		m_progressBar->setOrientation(Qt::Horizontal);

		QString strStyle = "QProgressBar {border-radius: 5px ; text-align: center; background: rgb(200, 200, 200);}"
			"QProgressBar::chunk {border-radius:5px;border:1px solid black;background: rgb(200,50,0)}";
		m_progressBar->setStyleSheet(strStyle);
		m_progressBar->setRange(0, 100);
		QGridLayout* layout = new QGridLayout();
		layout->setMargin(10);
		layout->setSpacing(2);
		layout->addWidget(m_progressLabel, 0, 2);
		layout->addWidget(m_progressBar, 1, 0, 2, 5);

		this->setLayout(layout);
		this->setStyleSheet("background-color: rgba(100,100,100,225);");
		this->setMask(QPolygon(progressRect));
		this->setMinimumSize(QSize(300, 80));
		this->hide();
	};

	~ProgressWidget() {
		delete m_progressLabel;
		delete m_progressBar;
	}

	void P(size_t i) {
		m_progressBar->setValue(i);
		QCoreApplication::processEvents();
	}

	void ShowProgress(QRect rect, QString text) {
		this->move(rect.center().x() / 2 + this->width() / 2
			, rect.center().y() / 2 + this->height() / 2);
		this->m_progressLabel->setText(text);
		this->show();
		m_progressBar->reset();
		P(0);
	}
};
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qmainwindow.h>
#include <qmenu.h>
#include <qaction.h>
#include <qdialog.h>
#include <qcoreapplication.h>
#include <qmessagebox.h>  
#include <qmimedata.h>  
#include <qurl.h>
#include <qlistwidget.h>
#include <qgroupbox.h>
#include <qgridlayout.h>
#include <qpixmapcache.h>
#include <qplaintextedit.h>
#include <qdir.h>
#include <qfileinfo.h>

#include <iostream>
#include <fstream>

#include "DLPrinter.h"
#include "glwidget.h"
#include "CenterTopWidget.h"
#include "SetupDialog.h"
#include "PreviewDialog.h"
#include "Setting.h"
#include "Model.hpp"

//���Զ���Ի�����вü��Ķ���εĵ�
static QVector<QPoint> anomalyRect1 = {QPoint(10,0),QPoint(130,0),QPoint(130,100),QPoint(10,100),QPoint(10,60),QPoint(0,50),QPoint(10,40)};
static QVector<QPoint> anomalyRect2 = { QPoint(10,0),QPoint(210,0),QPoint(210,140),QPoint(10,140),QPoint(10,80),QPoint(0,70),QPoint(10,60) };
static QVector<QPoint> progressRect = { QPoint(3,0),QPoint(297,0),QPoint(300,3),QPoint(300,77),QPoint(297,80),QPoint(3,80),QPoint(0,77),QPoint(0,3) };

//�����е�QSS
static QString spinStyle("background-color: rgba(225,225,225,0);border: 1px outset black;");

//��ǩ��QSS
static QString labelStyle("background-color: rgba(225,225,225,0)");

class GlWidget;

//��
typedef struct {
	Pointf3 centre;
	double radius;
}Ball;

class AboutDialog : public QDialog
{
public:
	AboutDialog();
	~AboutDialog();

private:
	QPushButton*  icoBtn;
	QLabel*  nameLabel;
	QLabel*  copyrightLabel;
	QLabel* versionLabel;
	QPlainTextEdit* textEdit;
	QGridLayout* layout;
};

inline bool clearDir(QString path)
{
	if (path.isEmpty()) {
		return false;
	}
	QDir dir(path);
	if (!dir.exists()) {
		return true;
	}
	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); //���ù���  
	QFileInfoList fileList = dir.entryInfoList(); // ��ȡ���е��ļ���Ϣ  
	foreach(QFileInfo file, fileList) { //�����ļ���Ϣ  
		if (file.isFile()) { // ���ļ���ɾ��  
			file.dir().remove(file.fileName());
		}
		else if (file.isDir()) { // �ݹ�ɾ��  
			clearDir(file.absoluteFilePath());
		}
	}
	return dir.rmpath(dir.absolutePath()); // ɾ���ļ���  
}

class MainWindow : public QMainWindow
{
	enum fileFormat
	{
		stl,
		obj,
		amf
	};

	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

private:
	std::unique_ptr<CenterTopWidget> m_centerTopWidget;
	Model* m_model = { new Model };
	DLPrint* m_dlprint;
	GlWidget* glwidget;										//��ά��ͼ����
	DLPrinter* dlprinter;									//��ӡ��
	//֧�ű༭ģʽʱģ�͵�֧�ŵ�(100��Ϊһ�����䣩
	std::vector<Pointf3Exist> treeSupportsExist;

	AboutDialog* aboutDialog;							//���ڶԻ���
	SetupDialog* setupDialog;							//���öԻ���
	PreviewDialog* previewDialog;						//Ԥ������

private:
	//------���������룬���ڡ���������������-------
	void addModelBuffer(size_t id);
	void deleteModelBuffer(size_t id);

	void addSupports(size_t id, TreeSupport* s, QProgressBar* progress = NULL);
	void deleteSupports(size_t id);

	void addOneSupport(Pointf3 p);
	void deleteOneSupport(size_t id);
	//_____________________________________________________________

	void AddOffsetValue(double x,double y,double z);
	void AddScaleValue(double x, double y, double z);
	void AddRotateValue(double angle, int x, int y, int z);

	void setOffsetValue(ModelInstance* instance);
	void setRotateValue(ModelInstance* instance);
	void setScaleValue(ModelInstance* instance);
	void setSizeValue(ModelInstance* instance);

	void initAction();
	void initDlprinter();
	void initSetupDialog();

	QString readStlTxt();
	void stroyStlTxt(QString stl);
	void showPreviewWidget1(QString zipPath);
	bool extractZipPath(QString zipPath);
	void duplicate(size_t num, bool arrange);

public slots:
	void slot_modelSelect();
	void slot_offsetChange();
	void slot_scaleChange();
	void slot_rotateChange();

	void DlpPrintLoadSetup();
	void showSetupDialog();

	void xoffsetValueChange(double value);
	void yoffsetValueChange(double value);
	void zoffsetValueChange(double value);

	void xRotateValueChange(double angle);
	void yRotateValueChange(double angle);
	void zRotateValueChange(double angle);

	void xScaleValueChange(double value);
	void yScaleValueChange(double value);
	void zScaleValueChange(double value);

	void showOffsetWidget();
	void showRotateWidget();
	void showScaleWidget();

private	slots:
	void ZPosZero();
	void setPersperctive() { glwidget->setPresprective(); };
	void setOrthogonality() { glwidget->setOrthogonality(); };
	void openStl();
	void deleteStl();
	void _exit();		

	//���ܣ������ͼ����
	void defaultView() { glwidget->ChangeView(glwidget->DEFAULT); };
	void overlookView() { glwidget->ChangeView(glwidget->OVERLOOK); };
	void leftView() { glwidget->ChangeView(glwidget->LEFT); };
	void rightView() { glwidget->ChangeView(glwidget->RIGHT); };
	void frontView() { glwidget->ChangeView(glwidget->FRONT); };
	void behindView() { glwidget->ChangeView(glwidget->BEHIND); };

	void newJob();
	void deleteSupport();
	void deleteAllSupport();
	void generateSupport();
	void generateAllSupport();
	void SupportEdit();
	void saveView();
	void slice();
	void saveModelSupport();
	void showAboutDialog();
	void autoArrange();
	void _duplicate();
	void dlprinterChange(QString name);

protected:
	void resizeEvent(QResizeEvent* event) ;
	void keyPressEvent(QKeyEvent* event);
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);

private:
	void generate_id_support(size_t id, TreeSupport*& s, QProgressBar* progress);
	void generate_all_inside_support();
};
#endif // MAINWINDOW_H

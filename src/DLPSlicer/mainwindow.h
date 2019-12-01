#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <glwidget.h>
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QDialog>
#include <QLabel>
#include <QSpinBox>
#include <QDockWidget>
#include <QEvent>
#include <QStackedWidget>
#include <QCheckBox>
#include <iostream>
#include <fstream>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QTabWidget>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QKeyEvent>
#include <QProgressBar>
#include <QCoreApplication>
#include "Interface.h"
#include <QMessageBox>
#include <QDragEnterEvent>  
#include <QMimeData>  
#include <QDropEvent>  
#include <QUrl>  
#include <QUndoGroup>
#include <QUndoStack>
#include "commands.h"
#include "qlistwidget.h"
#include "qgroupbox.h"
#include "qgridlayout.h"
#include "qpixmapcache.h"
#include <qplaintextedit.h>
#include "DLPrinter.h"

#include "CenterTopWidget.h"
#include "SetupDialog.h"
#include "PreviewDialog.h"
#include "Setting.h"

//֧�ű༭ʱ��֧�ŵ�ṹ��
typedef struct {
	bool exist;		//֧�ŵ��Ƿ���ڣ�true--���ڣ�false--�����ڣ�
	Pointf3 p;		//֧�ŵ�
}Pointf3Exist;

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

class MainWindow;

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

//inline QString getFileLocation()
//{
//	QString name = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
//	name.append("/.S_Maker");
//	QDir* file = new QDir(name);
//	bool ret = file->exists();
//	if (ret) {
//		return name;
//	}
//	else {
//		bool ok = file->mkdir(name);
//		if (!ok)
//			exit(2);
//		return file->dirName();
//	}
//}

//���ߣ�����
//���ڣ�2017
//���ܣ������ڣ�������Ⱦ���ڣ����ô��ڣ��˵����ȣ����������ܣ����֡�
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

	std::unique_ptr<CenterTopWidget> m_centerTopWidget;

	Interface* interface1;						//�ӿ�
	GlWidget* glwidget;							//��ά��ͼ����

	DLPrinter* dlprinter;						//��ӡ��

	//���ڣ�2017
	//���ܣ���ģ�ͱ�ѡ��ʱ��״̬��
	void modelSelect();

	//���ڣ�2018.5.29
	//------���������룬���ڡ���������������-------
	void addModelBuffer(size_t id);
	void deleteModelBuffer(size_t id);

	void offsetValueChange(size_t id,double x, double y, double z);
	void scaleValueChange(size_t id, double x, double y, double z);
	void rotateValueChange(size_t id, double angle, int x,int y, int z);

	void addSupports(size_t id, TreeSupport* s, QProgressBar* progress = NULL);
	void deleteSupports(size_t id);

	void addOneSupport(Pointf3 p);
	void addOneSupportUndo(size_t id);

	void deleteOneSupport(size_t id);
	void deleteOneSupportUndo(size_t id);
	//_____________________________________________________________

	void AddOffsetValue(double x,double y,double z);
	void AddScaleValue(double x, double y, double z);
	void AddRotateValue(double angle, int x, int y, int z);

	//֧�ű༭ģʽʱģ�͵�֧�ŵ�(100��Ϊһ�����䣩
	std::vector<Pointf3Exist> treeSupportsExist;

public slots:
	//���ڣ�2017
	//���ܣ�dlpprint��ȡ���á�
	void DlpPrintLoadSetup();

	//���ڣ�2017
	//���ܣ���ʾ���öԻ���
	void showSetupDialog();

private	slots:
	void ZPosZero();

	void setPersperctive();

	void setOrthogonality();

	//���ڣ�2018.5.29
	//����:�����ġ�����������ջ
	void clearUndoStack();

	//���ڣ�2017
	//���ܣ���stl�ļ���
	void openStl();

	//���ڣ�2017
	//���ܣ�ɾ��ģ���ļ���
	void deleteStl();

	//���ڣ�2017
	//���ܣ������˳���
	void _exit();		

	//********************
	//���ڣ�2017
	//���ܣ������ͼ����
	void defaultView();
	void overlookView();
	void leftView();
	void rightView();
	void frontView();
	void behindView();
	//*******************

	//���ڣ�2017
	//���ܣ��½���Ŀ��
	void newJob();

	//���ڣ�2017
	//���ܣ�ɾ��֧�š�
	void deleteSupport();

	void deleteAllSupport();


	//���ڣ�2017
	//���ܣ�����֧�š�
	void generateSupport();

	//���ڣ�2018.5
	//���ܣ���������֧�š�
	void generateAllSupport();

	//���ڣ�2017
	//���ܣ�֧�ű༭��
	void SupportEdit();

	//���ڣ�2017
	//���ܣ���ͼ��
	void saveView();

	//���ڣ�2017
	//���ܣ���Ƭ��
	void slice();

	//���ڣ�2017
	//���ܣ�������ͼ�ڵ�ģ��֧�š�
	void saveModelSupport();

	//���ڣ�2017
	//���ܣ���ʾabout�Ի���
	void showAboutDialog();

	//���ڣ�2018.3.7
	//���ܣ��Զ�����1
	void autoArrange();

	//���ڣ�2018.3.12
	//���ܣ�����
	void _duplicate();

	void dlprinterChange(QString name);

protected:
	//****************************************
	//���ڣ�2017
	//���ܣ���д���¼�������
	void resizeEvent(QResizeEvent* event) ;
	void keyPressEvent(QKeyEvent* event);
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);
	//*****************************************

private:
	//���ڣ�2017
	//���ܣ���ʼ��������
	void initAction();

	//���ڣ�2017
	//���ܣ���ʼ�����Ĵ��ڡ�
	void initCentralWindow();

	//���ڣ�2017
	//���ܣ���ȡstl�ļ���ʷ������
	//���أ��ļ�·��
	QString readStlTxt();

	//���ڣ�2017
	//���ܣ��洢stl�ļ�������
	//����1���ļ�·��
	void stroyStlTxt(QString stl);

	//���ڣ�2017
	//���ܣ��õ��ļ�λ�á�
	//���أ��ļ�·��
	//QString get_file_location();

	//���ڣ�2017
	//���ܣ�����ļ��С�
	//����1���ļ�·��
	//���أ��Ƿ�ɹ�
	//bool clearDir( QString &path);

	//���ڣ�2017
	//���ܣ��ڶ��ַ�ʽ��ȡzip����
	//����1��zip��·��
	void showPreviewWidget1(QString zipPath);

	//���ڣ�2017
	//���ܣ��ж��ܷ��ѹzip����
	//����1��zip����·��
	//���أ�true���ܽ�ѹ����false�����ܽ�ѹ��
	bool extractZipPath(QString zipPath);


	//���ڣ�2018.3.12
	//���ܣ�����
	//����1��ģ�͸�������
	void duplicate(size_t num,bool arrange);

	SetupDialog* setupDialog;					//���öԻ���
	PreviewDialog* previewDialog;					//Ԥ������

	//���ڣ�2018.10
	//���ܣ���ʼ�����öԻ���
	void initSetupDialog();

	//���ڣ�2017
	//���ܣ��õ�ѡ��ģ��ʵ����
	//���أ�ģ��ʵ��
	ModelInstance* IdToInstance();

	//���ڣ�2018.10
	//���ܣ���ʼ����ӡ��
	void initDlprinter();

	AboutDialog* aboutDialog;							//���ڶԻ���

	QUndoGroup *MainUndoGroup;						//��������������

	QUndoStack *MainUndoStack;						//��������������ջ

	QUndoStack *SupportEditUndoStack;				//֧�ű༭������������ջ

	//���ڣ�2018.5.10
	//���ܣ���ʼ����������
	void initUndo();

	enum fileFormat
	{
		stl,
		obj,
		amf
	};


//--------------------center top widget--------------
public slots:
	void xoffsetValueChange(double value);
	void yoffsetValueChange(double value);
	void zoffsetValueChange(double value);

	void xRotateValueChange(double angle);
	void yRotateValueChange(double angle);
	void zRotateValueChange(double angle);

	void xScaleValueChange(double value);
	void yScaleValueChange(double value);
	void zScaleValueChange(double value);

private:
	void setOffsetValue(ModelInstance* instance);
	void setRotateValue(ModelInstance* instance);
	void setScaleValue(ModelInstance* instance);
	void setSizeValue(ModelInstance* instance);

};
#endif // MAINWINDOW_H

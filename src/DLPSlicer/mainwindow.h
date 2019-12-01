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

//支撑编辑时的支撑点结构体
typedef struct {
	bool exist;		//支撑点是否存在（true--存在，false--不存在）
	Pointf3 p;		//支撑点
}Pointf3Exist;

//对自定义对话框进行裁剪的多边形的点
static QVector<QPoint> anomalyRect1 = {QPoint(10,0),QPoint(130,0),QPoint(130,100),QPoint(10,100),QPoint(10,60),QPoint(0,50),QPoint(10,40)};
static QVector<QPoint> anomalyRect2 = { QPoint(10,0),QPoint(210,0),QPoint(210,140),QPoint(10,140),QPoint(10,80),QPoint(0,70),QPoint(10,60) };
static QVector<QPoint> progressRect = { QPoint(3,0),QPoint(297,0),QPoint(300,3),QPoint(300,77),QPoint(297,80),QPoint(3,80),QPoint(0,77),QPoint(0,3) };

//自旋盒的QSS
static QString spinStyle("background-color: rgba(225,225,225,0);border: 1px outset black;");

//标签的QSS
static QString labelStyle("background-color: rgba(225,225,225,0)");

class GlWidget;

//球
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
	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); //设置过滤  
	QFileInfoList fileList = dir.entryInfoList(); // 获取所有的文件信息  
	foreach(QFileInfo file, fileList) { //遍历文件信息  
		if (file.isFile()) { // 是文件，删除  
			file.dir().remove(file.fileName());
		}
		else if (file.isDir()) { // 递归删除  
			clearDir(file.absoluteFilePath());
		}
	}
	return dir.rmpath(dir.absolutePath()); // 删除文件夹  
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

//作者：付康
//日期：2017
//功能：主窗口，包含渲染窗口，设置窗口，菜单栏等，控制整体框架，布局。
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

	std::unique_ptr<CenterTopWidget> m_centerTopWidget;

	Interface* interface1;						//接口
	GlWidget* glwidget;							//三维视图部件

	DLPrinter* dlprinter;						//打印机

	//日期：2017
	//功能：有模型被选中时的状态。
	void modelSelect();

	//日期：2018.5.29
	//------将操作分离，用于“撤销重做”操作-------
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

	//支撑编辑模式时模型的支撑点(100个为一个区间）
	std::vector<Pointf3Exist> treeSupportsExist;

public slots:
	//日期：2017
	//功能：dlpprint读取配置。
	void DlpPrintLoadSetup();

	//日期：2017
	//功能：显示设置对话框。
	void showSetupDialog();

private	slots:
	void ZPosZero();

	void setPersperctive();

	void setOrthogonality();

	//日期：2018.5.29
	//功能:清除活动的“撤销重做”栈
	void clearUndoStack();

	//日期：2017
	//功能：打开stl文件。
	void openStl();

	//日期：2017
	//功能：删除模型文件。
	void deleteStl();

	//日期：2017
	//功能：程序退出。
	void _exit();		

	//********************
	//日期：2017
	//功能：多个视图方向。
	void defaultView();
	void overlookView();
	void leftView();
	void rightView();
	void frontView();
	void behindView();
	//*******************

	//日期：2017
	//功能：新建项目。
	void newJob();

	//日期：2017
	//功能：删除支撑。
	void deleteSupport();

	void deleteAllSupport();


	//日期：2017
	//功能：生成支撑。
	void generateSupport();

	//日期：2018.5
	//功能：生成所有支撑。
	void generateAllSupport();

	//日期：2017
	//功能：支撑编辑。
	void SupportEdit();

	//日期：2017
	//功能：截图。
	void saveView();

	//日期：2017
	//功能：切片。
	void slice();

	//日期：2017
	//功能：保存视图内的模型支撑。
	void saveModelSupport();

	//日期：2017
	//功能：显示about对话框。
	void showAboutDialog();

	//日期：2018.3.7
	//功能：自动排列1
	void autoArrange();

	//日期：2018.3.12
	//功能：复制
	void _duplicate();

	void dlprinterChange(QString name);

protected:
	//****************************************
	//日期：2017
	//功能：重写的事件函数。
	void resizeEvent(QResizeEvent* event) ;
	void keyPressEvent(QKeyEvent* event);
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);
	//*****************************************

private:
	//日期：2017
	//功能：初始化动作。
	void initAction();

	//日期：2017
	//功能：初始化中心窗口。
	void initCentralWindow();

	//日期：2017
	//功能：读取stl文件历史环境。
	//返回：文件路径
	QString readStlTxt();

	//日期：2017
	//功能：存储stl文件环境。
	//参数1：文件路径
	void stroyStlTxt(QString stl);

	//日期：2017
	//功能：得到文件位置。
	//返回：文件路径
	//QString get_file_location();

	//日期：2017
	//功能：清空文件夹。
	//参数1：文件路径
	//返回：是否成功
	//bool clearDir( QString &path);

	//日期：2017
	//功能：第二种方式读取zip包。
	//参数1：zip包路径
	void showPreviewWidget1(QString zipPath);

	//日期：2017
	//功能：判断能否解压zip包。
	//参数1：zip包的路径
	//返回：true（能解压），false（不能解压）
	bool extractZipPath(QString zipPath);


	//日期：2018.3.12
	//功能：复制
	//参数1：模型复制数量
	void duplicate(size_t num,bool arrange);

	SetupDialog* setupDialog;					//设置对话框
	PreviewDialog* previewDialog;					//预览部件

	//日期：2018.10
	//功能：初始化设置对话框
	void initSetupDialog();

	//日期：2017
	//功能：得到选中模型实例。
	//返回：模型实例
	ModelInstance* IdToInstance();

	//日期：2018.10
	//功能：初始化打印机
	void initDlprinter();

	AboutDialog* aboutDialog;							//关于对话框

	QUndoGroup *MainUndoGroup;						//“撤销重做”组

	QUndoStack *MainUndoStack;						//主“撤销重做”栈

	QUndoStack *SupportEditUndoStack;				//支撑编辑“撤销重做”栈

	//日期：2018.5.10
	//功能：初始化撤销重做
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

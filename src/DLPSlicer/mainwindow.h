#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <glwidget.h>
#include <QWidget>
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
#include "qprocess.h"

//支撑编辑时的支撑点结构体
typedef struct {
	bool exist;		//支撑点是否存在（true--存在，false--不存在）
	Pointf3 p;		//支撑点
}Pointf3Exist;

//对自定义对话框进行裁剪的多边形的点
static QVector<QPoint> anomalyRect1 = {QPoint(10,0),QPoint(130,0),QPoint(130,100),QPoint(10,100),QPoint(10,60),QPoint(0,50),QPoint(10,40)};
static QVector<QPoint> anomalyRect2 = { QPoint(10,0),QPoint(210,0),QPoint(210,140),QPoint(10,140),QPoint(10,80),QPoint(0,70),QPoint(10,60) };
static QVector<QPoint> progressRect = { QPoint(3,0),QPoint(297,0),QPoint(300,3),QPoint(300,77),QPoint(297,80),QPoint(3,80),QPoint(0,77),QPoint(0,3) };

//可以被点击按钮的QSS
static QString OnButton("border: 1px outset white; border-radius: 5px;  background-color: rgba(100,225,50,225);");

//不能被点击按钮的QSS
static QString OffButton("border: 1px outset white; border-radius: 5px; background-color: rgba(200,200,200,150);");

//自旋盒的QSS
static QString spinStyle("background-color: rgba(225,225,225,0);border: 1px outset black;");

//标签的QSS
static QString labelStyle("background-color: rgba(225,225,225,0)");

//切片生成的2D图片名字前缀
static QString ImageName("slice");

//切片生成的配置文件的文件名
static QString buildsciptName("buildscipt");

class GlWidget;

//球
typedef struct {
	Pointf3 centre;
	double radius;
}Ball;

inline QString getFileLocation()
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

class MainWindow;

//********************
//作者：付康
//日期：2017
//功能：显示一张图片
//********************
class PixItem : public QGraphicsItem
{
public:
	PixItem(QPixmap* pixmap) ;
	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
	void updateItem(QString path);

private:
	QPixmap pix;//图元显示的图片
};

//***********************************************************************
//作者：付康
//日期：2017
//功能：用作预览切片后的文件，逐层显示图片，可对图片执行缩放，移动观察。
//***********************************************************************
class PreviewView : public QGraphicsView
{
	Q_OBJECT
public:
	PreviewView();
	~PreviewView();
	
	float _scale;						//缩放值
	int x;
	int y;

	PixItem* image;
	QString path;
	QSlider* slider;					//选择层的滑块
	QLabel* label;						//显示层数的标签

private:
	QVBoxLayout* layerLayout;
	QGraphicsScene* scene;
	QPixmap* pixmap;

private slots:

	//*******************
	//日期：2017
	//功能：层数改变。
	//参数1：改变的层数
	//*******************
	void valueChange(int num);

protected:
	//*******************************************************
	//日期：2017
	//功能：事件重写。
	void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	//*******************************************************
};

class PreviewDialog : public QDialog
{
public:
	PreviewDialog();
	~PreviewDialog();

	PreviewView* previewWidget;

	bool readIni(QString path);
private:
	//QLabel* printTimeName;
	//QLabel* printTimeLabel;
	QLabel* modelVolumeName;
	QLabel* modelVolumeLabel;
	QLabel* numberSlicesName;
	QLabel* numberSlicesLabel;
	QHBoxLayout* layout1;
	QGridLayout* mainLayout;
};

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

//**************************************************
//作者：付康
//日期：2017
//功能：设置对话框，提供光照，支撑，抽空和其他设置。
//*************************************************
class SetupDialog : public QDialog
{ 
	Q_OBJECT
public:
	SetupDialog(MainWindow* _mainwindow);
	~SetupDialog();

	//*******************
	//日期：2017
	//功能：初始化布局。
	//******************
	void initLayout();

	//光照设置
	QSpinBox* normIlluSpin;                       //正常曝光时间
	QLabel* normIlluLabel;
	QSpinBox* norm_inttersity_spin;               //正常曝光强度
	QLabel* norm_inttersity_label;
	QSpinBox* overIlluSpin;                       //长曝光时间
	QLabel* overIlluLabel;
	QSpinBox* over_inttersity_spin;               //长曝光强度
	QLabel* over_inttersity_label;
	QSpinBox* overLayerSpin;                      //长曝光层数
	QLabel* overLayerLabel;

	QGridLayout* illuLayout;
	QWidget* illuWidget;

	//支撑设置
	QDoubleSpinBox* top_height_spin;			  //顶端高度
	QLabel* top_height_label;
	QDoubleSpinBox* top_radius_spin;              //顶端半径
	QLabel* top_radius_label;
	QDoubleSpinBox* support_radius_spin;          //中间半径
	QLabel* support_radius_label;
	QDoubleSpinBox* bottom_radius_spin;			  //底部半径
	QLabel* bottom_radius_label;
	QSpinBox* support_space_spin;                 //支撑间距
	QLabel* support_space_label;
	QSpinBox* support_angle_spin;                 //支撑角度
	QLabel* support_angle_label;
	QSpinBox* leaf_num_spin;					  //树枝长度
	QLabel* leaf_num_label;
	QDoubleSpinBox* model_lift_spin;			  //模型提升
	QLabel* model_lift_label;

	QGridLayout* supportLayout;
	QWidget* supportWidget;

	//抽空设置
	QCheckBox* hollow_out_box;                    //是否抽空
	QLabel* hollow_out_label;
	QComboBox* fill_pattern_combo;					  //填充图案
	QLabel* fillPatternLabel;
	QDoubleSpinBox* wall_thickness_spin;          //壁厚
	QLabel* wall_thickness_label;
	QSpinBox* density_spin;						  //内部支撑密度
	QLabel* density_label;

	QGridLayout* hollowOutLayout;
	QWidget* hollowOutWidget;

	//其他设置
	QComboBox* thicknessCombo;                    //层厚
	QLabel* thicknessLabel;
	QSpinBox* raftSpin;                           //底筏层数
	QLabel* raftLabel;
	QSpinBox* raftOffsetSpin;					  //底板补偿
	QLabel* raftOffsetLabel;
	QDoubleSpinBox* arrange_space_spin;			  //自动排列间距
	QLabel* arrangeSpaceLabel;
	QSpinBox* threadSpin;						  //线程数
	QLabel* threadLabel;

	QGridLayout* otherLayourt;
	QWidget* otherWidget;

	QTabWidget* setupTab;

	QPushButton* defultBtn;
	QPushButton* cancalBtn;
	QPushButton* okBtn;
	QGridLayout* mainLayout;

private slots:
	//日期：2017
	//功能：写配置。
	void writeConfig();

	//日期：2017
	//功能：恢复默认。
	void recoverDefult();

	//日期：2017
	//功能：关闭对话框操作。
	void BtnChange();
private:
	//日期：2017
	//功能：设置默认值。
	void setDefultValue();

	//日期：2017
	//功能：读取配置。
	void readConfig();

	MainWindow* mainwindow;						  //父窗口
};

//作者：付康
//日期：2017
//功能：主窗口，包含渲染窗口，设置窗口，菜单栏等，控制整体框架，布局。
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

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

	//日期：2017
	//功能：显示平移对话框。
	void showOffsetWidget();

	//日期：2017
	//功能：显示旋转对话框。
	void showRotateWidget();

	//日期：2017
	//功能：显示缩放对话框。
	void showScaleWidget();

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

	//************************
	//日期：2017
	//功能：平移值的改变。
	void xoffsetValueChange(double value);
	void yoffsetValueChange(double value);
	void zoffsetValueChange(double value);
	//************************

	//************************
	//日期：2017
	//功能：旋转值的改变。
	void xRotateValueChange(double angle);
	void yRotateValueChange(double angle);
	void zRotateValueChange(double angle);
	//************************

	//日期：2017
	//功能：统一缩放。
	void setUnifyScale(int state);

	//************************
	//日期：2017
	//功能：比例值的改变。
	void xScaleValueChange(double value);
	void yScaleValueChange(double value);
	void zScaleValueChange(double value);
	//***********************

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

	//日期：2018.10
	//功能：检测更新
	void checkUpdate();

	void UpdateFinished(int a);
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
	//功能：为进度条设值。
	void P(size_t i);

	//日期：2017
	//功能：初始化中心窗口。
	void initCentralWindow();

	//日期：2017
	//功能：初始化按钮。
	void initButton();

	//日期：2017
	//功能：初始化平移对话框。
	void initOffsetWidget();

	//日期：2017
	//功能：初始化旋转对话框。
	void initRotateWidget();

	//日期：2017
	//功能：初始化缩放对话框。
	void initScaleWidget();

	//日期：2017
	//功能：初始化进度条。
	void initProgress();

	//日期：2017
	//功能：显示进度条。
	//参数1：执行操作的枚举值
	void showProgress(int btn);

	//日期：2017
	//功能：隐藏进度条。
	void hideProgress();

	//日期：2017
	//功能：点击按钮时的状态。
	//参数1：执行操作的枚举值
	void clickedButtonState(int btn);

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
	QString get_file_location();

	//日期：2017
	//功能：清空文件夹。
	//参数1：文件路径
	//返回：是否成功
	bool clearDir( QString &path);

	//日期：2017
	//功能：第二种方式读取zip包。
	//参数1：zip包路径
	void showPreviewWidget1(QString zipPath);

	//日期：2017
	//功能：判断能否解压zip包。
	//参数1：zip包的路径
	//返回：true（能解压），false（不能解压）
	bool extractZipPath(QString zipPath);

	//日期：2017
	//功能：设置选中模型的移动值。
	//参数1：模型实例
	void setOffsetValue(ModelInstance* instance);

	//日期：2017
	//功能：设置选中模型的旋转值。
	//参数1：模型实例
	void setRotateValue(ModelInstance* instance);

	//日期：2017
	//功能：设置选中模型的缩放值。
	//参数1：模型实例
	void setScaleValue(ModelInstance* instance);

	//日期：2017
	//功能：设置选中模型的大小值。
	//参数1：模型实例
	void setSizeValue(ModelInstance* instance);

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

	QLabel* SelectPrinterLabel;
	QComboBox* SelectPrinterCombo;

	//日期：2018.10
	//功能：初始化打印机
	void initDlprinter();
	
	//点击的按钮枚举
	enum BTN{
		OPENBTN = 5,
		OFFSETBTN,
		SCALEBTN,
		ROTATEBTN,
		MODELBTN,
		SLICEBTN,
		SETUPBTN,
		SUPPORTBTN,
		SUPPORTEDITBTN,
		PREVIEWBTN
	};

	//按钮
	QPushButton* openButton;
	bool OnOff_open;

	QPushButton* supportButton;
	bool OnOff_support;

	QPushButton* supportEditButton;
	bool OnOff_supportEdit;

	QPushButton* sliceButton;
	bool OnOff_slice;

	QPushButton* setupButton;
	bool OnOff_setup;

	//进度条对话框
	QLabel* progressLabel;
	QProgressBar* progress;
	QStackedWidget* progressWidget;
	
	//平移对话框
	QPushButton* offsetButton;
	QDoubleSpinBox* x_offset_spin;
	QDoubleSpinBox* y_offset_spin;
	QDoubleSpinBox* z_offset_spin;
	bool OnOff_offset;								//开关
	QStackedWidget* offsetWidget;

	//旋转对话框
	double x_rotate;
	double y_rotate;
	double z_rotate;
	void init_rotate();

	QPushButton* rotateButton;
	QDoubleSpinBox* x_rotate_spin;
	QDoubleSpinBox* y_rotate_spin;
	QDoubleSpinBox* z_rotate_spin;
	bool OnOff_rotate;
	QStackedWidget* rotateWidget;

	//缩放对话框
	QPushButton* scaleButton;
	QDoubleSpinBox* x_scale_spin;
	QDoubleSpinBox* y_scale_spin;
	QDoubleSpinBox* z_scale_spin;
	QDoubleSpinBox* x_size_label;
	QDoubleSpinBox* y_size_label;
	QDoubleSpinBox* z_size_label;
	QCheckBox* unify_scale;							//统一缩放
	bool OnOff_scale;
	QStackedWidget* scaleWidget;

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

	QProcess* process;
};
#endif // MAINWINDOW_H

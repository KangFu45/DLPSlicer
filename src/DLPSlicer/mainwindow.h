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

//֧�ű༭ʱ��֧�ŵ�ṹ��
typedef struct {
	bool exist;		//֧�ŵ��Ƿ���ڣ�true--���ڣ�false--�����ڣ�
	Pointf3 p;		//֧�ŵ�
}Pointf3Exist;

//���Զ���Ի�����вü��Ķ���εĵ�
static QVector<QPoint> anomalyRect1 = {QPoint(10,0),QPoint(130,0),QPoint(130,100),QPoint(10,100),QPoint(10,60),QPoint(0,50),QPoint(10,40)};
static QVector<QPoint> anomalyRect2 = { QPoint(10,0),QPoint(210,0),QPoint(210,140),QPoint(10,140),QPoint(10,80),QPoint(0,70),QPoint(10,60) };
static QVector<QPoint> progressRect = { QPoint(3,0),QPoint(297,0),QPoint(300,3),QPoint(300,77),QPoint(297,80),QPoint(3,80),QPoint(0,77),QPoint(0,3) };

//���Ա������ť��QSS
static QString OnButton("border: 1px outset white; border-radius: 5px;  background-color: rgba(100,225,50,225);");

//���ܱ������ť��QSS
static QString OffButton("border: 1px outset white; border-radius: 5px; background-color: rgba(200,200,200,150);");

//�����е�QSS
static QString spinStyle("background-color: rgba(225,225,225,0);border: 1px outset black;");

//��ǩ��QSS
static QString labelStyle("background-color: rgba(225,225,225,0)");

//��Ƭ���ɵ�2DͼƬ����ǰ׺
static QString ImageName("slice");

//��Ƭ���ɵ������ļ����ļ���
static QString buildsciptName("buildscipt");

class GlWidget;

//��
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
//���ߣ�����
//���ڣ�2017
//���ܣ���ʾһ��ͼƬ
//********************
class PixItem : public QGraphicsItem
{
public:
	PixItem(QPixmap* pixmap) ;
	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
	void updateItem(QString path);

private:
	QPixmap pix;//ͼԪ��ʾ��ͼƬ
};

//***********************************************************************
//���ߣ�����
//���ڣ�2017
//���ܣ�����Ԥ����Ƭ����ļ��������ʾͼƬ���ɶ�ͼƬִ�����ţ��ƶ��۲졣
//***********************************************************************
class PreviewView : public QGraphicsView
{
	Q_OBJECT
public:
	PreviewView();
	~PreviewView();
	
	float _scale;						//����ֵ
	int x;
	int y;

	PixItem* image;
	QString path;
	QSlider* slider;					//ѡ���Ļ���
	QLabel* label;						//��ʾ�����ı�ǩ

private:
	QVBoxLayout* layerLayout;
	QGraphicsScene* scene;
	QPixmap* pixmap;

private slots:

	//*******************
	//���ڣ�2017
	//���ܣ������ı䡣
	//����1���ı�Ĳ���
	//*******************
	void valueChange(int num);

protected:
	//*******************************************************
	//���ڣ�2017
	//���ܣ��¼���д��
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
//���ߣ�����
//���ڣ�2017
//���ܣ����öԻ����ṩ���գ�֧�ţ���պ��������á�
//*************************************************
class SetupDialog : public QDialog
{ 
	Q_OBJECT
public:
	SetupDialog(MainWindow* _mainwindow);
	~SetupDialog();

	//*******************
	//���ڣ�2017
	//���ܣ���ʼ�����֡�
	//******************
	void initLayout();

	//��������
	QSpinBox* normIlluSpin;                       //�����ع�ʱ��
	QLabel* normIlluLabel;
	QSpinBox* norm_inttersity_spin;               //�����ع�ǿ��
	QLabel* norm_inttersity_label;
	QSpinBox* overIlluSpin;                       //���ع�ʱ��
	QLabel* overIlluLabel;
	QSpinBox* over_inttersity_spin;               //���ع�ǿ��
	QLabel* over_inttersity_label;
	QSpinBox* overLayerSpin;                      //���ع����
	QLabel* overLayerLabel;

	QGridLayout* illuLayout;
	QWidget* illuWidget;

	//֧������
	QDoubleSpinBox* top_height_spin;			  //���˸߶�
	QLabel* top_height_label;
	QDoubleSpinBox* top_radius_spin;              //���˰뾶
	QLabel* top_radius_label;
	QDoubleSpinBox* support_radius_spin;          //�м�뾶
	QLabel* support_radius_label;
	QDoubleSpinBox* bottom_radius_spin;			  //�ײ��뾶
	QLabel* bottom_radius_label;
	QSpinBox* support_space_spin;                 //֧�ż��
	QLabel* support_space_label;
	QSpinBox* support_angle_spin;                 //֧�ŽǶ�
	QLabel* support_angle_label;
	QSpinBox* leaf_num_spin;					  //��֦����
	QLabel* leaf_num_label;
	QDoubleSpinBox* model_lift_spin;			  //ģ������
	QLabel* model_lift_label;

	QGridLayout* supportLayout;
	QWidget* supportWidget;

	//�������
	QCheckBox* hollow_out_box;                    //�Ƿ���
	QLabel* hollow_out_label;
	QComboBox* fill_pattern_combo;					  //���ͼ��
	QLabel* fillPatternLabel;
	QDoubleSpinBox* wall_thickness_spin;          //�ں�
	QLabel* wall_thickness_label;
	QSpinBox* density_spin;						  //�ڲ�֧���ܶ�
	QLabel* density_label;

	QGridLayout* hollowOutLayout;
	QWidget* hollowOutWidget;

	//��������
	QComboBox* thicknessCombo;                    //���
	QLabel* thicknessLabel;
	QSpinBox* raftSpin;                           //�׷�����
	QLabel* raftLabel;
	QSpinBox* raftOffsetSpin;					  //�װ岹��
	QLabel* raftOffsetLabel;
	QDoubleSpinBox* arrange_space_spin;			  //�Զ����м��
	QLabel* arrangeSpaceLabel;
	QSpinBox* threadSpin;						  //�߳���
	QLabel* threadLabel;

	QGridLayout* otherLayourt;
	QWidget* otherWidget;

	QTabWidget* setupTab;

	QPushButton* defultBtn;
	QPushButton* cancalBtn;
	QPushButton* okBtn;
	QGridLayout* mainLayout;

private slots:
	//���ڣ�2017
	//���ܣ�д���á�
	void writeConfig();

	//���ڣ�2017
	//���ܣ��ָ�Ĭ�ϡ�
	void recoverDefult();

	//���ڣ�2017
	//���ܣ��رնԻ��������
	void BtnChange();
private:
	//���ڣ�2017
	//���ܣ�����Ĭ��ֵ��
	void setDefultValue();

	//���ڣ�2017
	//���ܣ���ȡ���á�
	void readConfig();

	MainWindow* mainwindow;						  //������
};

//���ߣ�����
//���ڣ�2017
//���ܣ������ڣ�������Ⱦ���ڣ����ô��ڣ��˵����ȣ����������ܣ����֡�
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

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

	//���ڣ�2017
	//���ܣ���ʾƽ�ƶԻ���
	void showOffsetWidget();

	//���ڣ�2017
	//���ܣ���ʾ��ת�Ի���
	void showRotateWidget();

	//���ڣ�2017
	//���ܣ���ʾ���ŶԻ���
	void showScaleWidget();

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

	//************************
	//���ڣ�2017
	//���ܣ�ƽ��ֵ�ĸı䡣
	void xoffsetValueChange(double value);
	void yoffsetValueChange(double value);
	void zoffsetValueChange(double value);
	//************************

	//************************
	//���ڣ�2017
	//���ܣ���תֵ�ĸı䡣
	void xRotateValueChange(double angle);
	void yRotateValueChange(double angle);
	void zRotateValueChange(double angle);
	//************************

	//���ڣ�2017
	//���ܣ�ͳһ���š�
	void setUnifyScale(int state);

	//************************
	//���ڣ�2017
	//���ܣ�����ֵ�ĸı䡣
	void xScaleValueChange(double value);
	void yScaleValueChange(double value);
	void zScaleValueChange(double value);
	//***********************

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

	//���ڣ�2018.10
	//���ܣ�������
	void checkUpdate();

	void UpdateFinished(int a);
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
	//���ܣ�Ϊ��������ֵ��
	void P(size_t i);

	//���ڣ�2017
	//���ܣ���ʼ�����Ĵ��ڡ�
	void initCentralWindow();

	//���ڣ�2017
	//���ܣ���ʼ����ť��
	void initButton();

	//���ڣ�2017
	//���ܣ���ʼ��ƽ�ƶԻ���
	void initOffsetWidget();

	//���ڣ�2017
	//���ܣ���ʼ����ת�Ի���
	void initRotateWidget();

	//���ڣ�2017
	//���ܣ���ʼ�����ŶԻ���
	void initScaleWidget();

	//���ڣ�2017
	//���ܣ���ʼ����������
	void initProgress();

	//���ڣ�2017
	//���ܣ���ʾ��������
	//����1��ִ�в�����ö��ֵ
	void showProgress(int btn);

	//���ڣ�2017
	//���ܣ����ؽ�������
	void hideProgress();

	//���ڣ�2017
	//���ܣ������ťʱ��״̬��
	//����1��ִ�в�����ö��ֵ
	void clickedButtonState(int btn);

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
	QString get_file_location();

	//���ڣ�2017
	//���ܣ�����ļ��С�
	//����1���ļ�·��
	//���أ��Ƿ�ɹ�
	bool clearDir( QString &path);

	//���ڣ�2017
	//���ܣ��ڶ��ַ�ʽ��ȡzip����
	//����1��zip��·��
	void showPreviewWidget1(QString zipPath);

	//���ڣ�2017
	//���ܣ��ж��ܷ��ѹzip����
	//����1��zip����·��
	//���أ�true���ܽ�ѹ����false�����ܽ�ѹ��
	bool extractZipPath(QString zipPath);

	//���ڣ�2017
	//���ܣ�����ѡ��ģ�͵��ƶ�ֵ��
	//����1��ģ��ʵ��
	void setOffsetValue(ModelInstance* instance);

	//���ڣ�2017
	//���ܣ�����ѡ��ģ�͵���תֵ��
	//����1��ģ��ʵ��
	void setRotateValue(ModelInstance* instance);

	//���ڣ�2017
	//���ܣ�����ѡ��ģ�͵�����ֵ��
	//����1��ģ��ʵ��
	void setScaleValue(ModelInstance* instance);

	//���ڣ�2017
	//���ܣ�����ѡ��ģ�͵Ĵ�Сֵ��
	//����1��ģ��ʵ��
	void setSizeValue(ModelInstance* instance);

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

	QLabel* SelectPrinterLabel;
	QComboBox* SelectPrinterCombo;

	//���ڣ�2018.10
	//���ܣ���ʼ����ӡ��
	void initDlprinter();
	
	//����İ�ťö��
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

	//��ť
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

	//�������Ի���
	QLabel* progressLabel;
	QProgressBar* progress;
	QStackedWidget* progressWidget;
	
	//ƽ�ƶԻ���
	QPushButton* offsetButton;
	QDoubleSpinBox* x_offset_spin;
	QDoubleSpinBox* y_offset_spin;
	QDoubleSpinBox* z_offset_spin;
	bool OnOff_offset;								//����
	QStackedWidget* offsetWidget;

	//��ת�Ի���
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

	//���ŶԻ���
	QPushButton* scaleButton;
	QDoubleSpinBox* x_scale_spin;
	QDoubleSpinBox* y_scale_spin;
	QDoubleSpinBox* z_scale_spin;
	QDoubleSpinBox* x_size_label;
	QDoubleSpinBox* y_size_label;
	QDoubleSpinBox* z_size_label;
	QCheckBox* unify_scale;							//ͳһ����
	bool OnOff_scale;
	QStackedWidget* scaleWidget;

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

	QProcess* process;
};
#endif // MAINWINDOW_H

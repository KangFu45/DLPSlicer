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

#include "glwidget.h"
#include "CenterTopWidget.h"
#include "SetupDialog.h"
#include "PreviewDialog.h"
#include "Setting.h"
#include "Model.hpp"

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
	MainWindow(QWidget* parent = 0);
	~MainWindow();

private:
	std::unique_ptr<CenterTopWidget> m_centerTopWidget;
	Model* m_model = { new Model };
	DLPrint* m_dlprint;
	GlWidget* glwidget;										//三维视图部件

	AboutDialog* aboutDialog;							//关于对话框
	SetupDialog* setupDialog;							//设置对话框
	PreviewDialog* previewDialog;						//预览部件

private:
	void AddOffsetValue(double x, double y, double z);

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
	void _exit();

	//功能：多个视图方向。
	void defaultView() { glwidget->ChangeView(glwidget->DEFAULT); };
	void overlookView() { glwidget->ChangeView(glwidget->OVERLOOK); };
	void leftView() { glwidget->ChangeView(glwidget->LEFT); };
	void rightView() { glwidget->ChangeView(glwidget->RIGHT); };
	void frontView() { glwidget->ChangeView(glwidget->FRONT); };
	void behindView() { glwidget->ChangeView(glwidget->BEHIND); };

	void newJob();
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

protected:
	void resizeEvent(QResizeEvent* event);
	void keyPressEvent(QKeyEvent* event);
	void dragEnterEvent(QDragEnterEvent* event);
	void dropEvent(QDropEvent* event);

private:
	void generate_all_inside_support();
};
#endif // MAINWINDOW_H

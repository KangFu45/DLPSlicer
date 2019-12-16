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

class Config;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget* parent = 0);
	~MainWindow();

private:
	std::unique_ptr<CenterTopWidget> m_centerTopWidget;
	Model* m_model = { new Model };
	DLPrint* m_dlprint;
	GlWidget* m_glwidget;	//三维视图部件
	Config* m_config;

	void SetOffsetValue(ModelInstance* instance);
	void SetRotateValue(ModelInstance* instance);
	void SetScaleValue(ModelInstance* instance);
	void SetSizeValue(ModelInstance* instance);

	void InitAction();
	void InitDlprinter();

	QString ReadStlTxt();
	void StroyStlTxt(QString stl);
	void ShowPreviewWidget(QString zipPath);
	bool ExtractZipPath(QString zipPath);
	void Duplicate(size_t num, bool arrange);
	void GenAllInsideSupport();

public slots:
	void slot_modelSelect();
	void slot_offsetChange();
	void slot_scaleChange();
	void slot_rotateChange();

	void slot_showSetupDialog();

	void slot_xoffsetValueChange(double value);
	void slot_yoffsetValueChange(double value);
	void slot_zoffsetValueChange(double value);

	void slot_xRotateValueChange(double angle);
	void slot_yRotateValueChange(double angle);
	void slot_zRotateValueChange(double angle);

	void slot_xScaleValueChange(double value);
	void slot_yScaleValueChange(double value);
	void slot_zScaleValueChange(double value);

	void slot_showOffsetWidget();
	void slot_showRotateWidget();
	void slot_showScaleWidget();

private	slots:
	void slot_ZPosZero();
	void slot_setPersperctive() { m_glwidget->SetViewPort(m_glwidget->PRESPRECTIVE); };
	void slot_setOrthogonality() { m_glwidget->SetViewPort(m_glwidget->ORTHOGONALITY); };
	void slot_openStl();
	void slot_exit() { exit(0); };

	//功能：多个视图方向。
	void slot_defaultView() { m_glwidget->ChangeView(m_glwidget->DEFAULT); };
	void slot_overlookView() { m_glwidget->ChangeView(m_glwidget->OVERLOOK); };
	void slot_leftView() { m_glwidget->ChangeView(m_glwidget->LEFT); };
	void slot_rightView() { m_glwidget->ChangeView(m_glwidget->RIGHT); };
	void slot_frontView() { m_glwidget->ChangeView(m_glwidget->FRONT); };
	void slot_behindView() { m_glwidget->ChangeView(m_glwidget->BEHIND); };

	void slot_newJob();
	void slot_generateSupport();
	void slot_generateAllSupport();
	void slot_supportEdit();
	void slot_saveView();
	void slot_slice();
	void slot_showAboutDialog();
	void slot_autoArrange();
	void slot_duplicate();

protected:
	void resizeEvent(QResizeEvent* event);
	void keyPressEvent(QKeyEvent* event);
	void dragEnterEvent(QDragEnterEvent* event);
	void dropEvent(QDropEvent* event);
};
#endif // MAINWINDOW_H

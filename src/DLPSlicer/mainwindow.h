#pragma once

#include <qmainwindow.h>
#include <qmenu.h>
#include <qaction.h>
#include <qdialog.h>
#include <qmessagebox.h>  
#include <qmimedata.h>  
#include <qurl.h>
#include <qlistwidget.h>
#include <qgroupbox.h>
#include <qgridlayout.h>
#include <qpixmapcache.h>
#include <qplaintextedit.h>
#include <qtabwidget.h>

#include <iostream>
#include <fstream>

#include "glwidget.h"
#include "CenterTopWidget.h"
#include "Model.hpp"
#include "SetupDialog.h"
#include "PreviewWidget.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget* parent = 0);
	~MainWindow();

private:
	std::unique_ptr<CenterTopWidget> m_centerTopWidget;
	std::unique_ptr<PreviewTopWidget> m_previewTopWidget;

	Model* m_model = { new Model };
	DLPrint* m_dlprint;
	GlWidget* m_glwidget;	//三维视图部件
	Config* m_config = { new Config };
	PreviewWidget* m_previewWidget;
	QTabWidget* m_tabWidget = {new QTabWidget};
	ProgressWidget* m_progressWidget;

	void SetOffsetValue(ModelInstance* instance);
	void SetRotateValue(ModelInstance* instance);
	void SetScaleValue(ModelInstance* instance);
	void SetSizeValue(ModelInstance* instance);

	void InitAction();
	void InitDlprinter();

	void LoadStl(QString name);
	QString ReadStlTxt();
	void StroyStlTxt(QString stl);
	bool ExtractZipPath(QString zipPath);
	void Duplicate(size_t num, bool arrange);
	void GenAllInsideSupport();
	void SetDLPrintDirty();

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

	//功能：多个视图方向。
	void slot_defaultView() { m_glwidget->ChangeView(m_glwidget->DEFAULT); };
	void slot_overlookView() { m_glwidget->ChangeView(m_glwidget->OVERLOOK); };
	void slot_leftView() { m_glwidget->ChangeView(m_glwidget->LEFT); };
	void slot_rightView() { m_glwidget->ChangeView(m_glwidget->RIGHT); };
	void slot_frontView() { m_glwidget->ChangeView(m_glwidget->FRONT); };
	void slot_behindView() { m_glwidget->ChangeView(m_glwidget->BEHIND); };

private	slots:
	void slot_ZPosZero();
	void slot_setPersperctive() { m_glwidget->SetViewPort(m_glwidget->PRESPRECTIVE); };
	void slot_setOrthogonality() { m_glwidget->SetViewPort(m_glwidget->ORTHOGONALITY); };
	void slot_openStl();
	void slot_exit() { exit(0); };

	void slot_newJob();
	void slot_delSelSupport();
	void slot_generateSupport();
	void slot_generateAllSupport();
	void slot_supportEdit();
	void slot_saveView();
	void slot_slice();
	void slot_saveSlice();
	void slot_autoArrange();
	void slot_duplicate();

	void slot_glwidgetSizeChange();
	void slot_previewSizeChange();
	void slot_tabWidgetChange(int index);

protected:
	void keyPressEvent(QKeyEvent* event);
	void dragEnterEvent(QDragEnterEvent* event);
	void dropEvent(QDropEvent* event);
};

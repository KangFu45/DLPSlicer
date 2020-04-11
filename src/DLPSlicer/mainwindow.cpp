#include "mainwindow.h"

#include <qmenubar.h>
#include <qevent.h>
#include <qregion.h>
#include <qsettings.h>
#include <qplaintextedit.h>
#include <qinputdialog.h>
#include <qtimer.h>
#include <qfiledialog.h>
#include <qstandardpaths.h>
#include <qdir.h>
#include <qfileinfo.h>

#include <quazip/JlCompress.h>

#include "IO.hpp"

//ERROR:setting.h放在.cpp文件夹下会报错???
#include "Setting.h"
extern Setting e_setting;

#ifdef DLPSlicer_DEBUG
#include "qdebug.h"
#endif

inline bool clearDir(QString path)
{
	if (path.isEmpty())
		return false;

	QDir dir(path);
	if (!dir.exists())
		return true;

	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); //设置过滤  
	foreach(QFileInfo file, dir.entryInfoList()) { //遍历文件信息  
		if (file.isFile()) // 是文件，删除  
			file.dir().remove(file.fileName());
		else if (file.isDir()) // 递归删除  
			clearDir(file.absoluteFilePath());
	}
	return dir.rmpath(dir.absolutePath()); // 删除文件夹  
}

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	InitDlprinter();

	SetupDialog setup(m_config);
	setup.slot_writeConfig();

	m_dlprint = new DLPrint(m_model, m_config);
	m_glwidget = new GlWidget(m_model, m_dlprint);
	m_previewWidget = new PreviewWidget(m_dlprint);

	m_tabWidget->addTab(m_glwidget, QStringLiteral("视图"));
	m_tabWidget->addTab(m_previewWidget, QStringLiteral("预览"));
	m_tabWidget->setTabPosition(QTabWidget::South);
	setCentralWidget(m_tabWidget);//要先于中央界面上button初始化

	m_progressWidget = new ProgressWidget(m_tabWidget);

	InitAction();
	m_centerTopWidget = std::unique_ptr<CenterTopWidget>(new CenterTopWidget(this));
	m_previewTopWidget = std::unique_ptr<PreviewTopWidget>(new PreviewTopWidget(this));

	(void)connect(m_glwidget, &GlWidget::sig_modelSelect, this, &MainWindow::slot_modelSelect);
	(void)connect(m_glwidget, &GlWidget::sig_offsetChange, this, &MainWindow::slot_offsetChange);
	(void)connect(m_glwidget, &GlWidget::sig_rotateChange, this, &MainWindow::slot_rotateChange);
	(void)connect(m_glwidget, &GlWidget::sig_scaleChange, this, &MainWindow::slot_scaleChange);
	(void)connect(m_glwidget, &GlWidget::sig_sizeChange, this, &MainWindow::slot_glwidgetSizeChange);
	(void)connect(m_previewWidget, &PreviewWidget::sig_sizeChange, this, &MainWindow::slot_previewSizeChange);
	(void)connect(m_centerTopWidget->m_printerCombo, SIGNAL(currentIndexChanged(QString)), m_glwidget, SLOT(slot_dlprinterChange(QString)));
	(void)connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(slot_tabWidgetChange(int)));
	(void)connect(m_previewTopWidget->m_slider, SIGNAL(valueChanged(int)), m_previewWidget, SLOT(slot_layerChanged(int)));

	setAcceptDrops(true);
	setWindowTitle("DLPSlicer");
	setWindowIcon(QIcon(":/icon/images/printer.ico"));
	setAttribute(Qt::WA_QuitOnClose, true);//设置主窗口关闭即程序退出
	resize(1200, 700);
}

MainWindow::~MainWindow()
{
	//关闭窗口时，此信号槽还会执行一次
	(void)disconnect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(slot_tabWidgetChange(int)));

	delete m_centerTopWidget.release();
	delete m_config;
	delete m_model;
	delete m_dlprint;
	delete m_glwidget;
	delete m_progressWidget;
	delete m_tabWidget;
}

QString MainWindow::ReadStlTxt()
{
	if (QFile(e_setting.ModelFile.c_str()).exists())
		return QSettings(e_setting.ModelFile.c_str(), QSettings::IniFormat).value("/modelPath/path").toString();
	else
		return QString();
}

void MainWindow::StroyStlTxt(QString stl)
{
	QSettings writeini(e_setting.ModelFile.c_str(), QSettings::IniFormat);
	writeini.clear();
	writeini.setValue("/modelPath/path", stl);
}

void MainWindow::slot_openStl()
{
	m_centerTopWidget->CenterButtonPush(CenterTopWidget::OPENBTN);
	QString path(ReadStlTxt());
	LoadStl(QFileDialog::getOpenFileName(this, QStringLiteral("选择需要打开的文件"), 
		path.left(path.lastIndexOf("/")), "*.stl *.sm *.obj"));
	m_centerTopWidget->CenterButtonPush(CenterTopWidget::OPENBTN);
}

void MainWindow::LoadStl(QString name)
{
	if (name.right(4).indexOf(".stl", 0, Qt::CaseInsensitive) >= 0) {
		StroyStlTxt(name);
		
		m_progressWidget->ShowProgress(m_tabWidget->rect(), QStringLiteral("加载模型"));
		qDebug() << name;
		m_progressWidget->P(20);
		m_glwidget->AddModelInstance(m_model->load_model(name.toStdString()));
		m_progressWidget->P(100);
		m_progressWidget->hide();
		m_glwidget->UpdateConfine();

		this->SetDLPrintDirty();
	}
	//else if (name.right(3).indexOf(".sm", 0, Qt::CaseInsensitive) >= 0)
	//	ShowPreviewWidget(name);
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Delete)
		m_glwidget->slot_delSelectIntance();
	else if (event->key() == Qt::Key_Return)
		m_glwidget->AddNewSupportPoint();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->urls()[0].fileName().right(4).indexOf(".stl", 0, Qt::CaseInsensitive) >= 0 ||
		event->mimeData()->urls()[0].fileName().right(4).indexOf(".obj", 0, Qt::CaseInsensitive) >= 0 ||
		event->mimeData()->urls()[0].fileName().right(4).indexOf(".amf", 0, Qt::CaseInsensitive) >= 0 ||
		event->mimeData()->urls()[0].fileName().right(3).indexOf(".sm", 0, Qt::CaseInsensitive) >= 0)
		event->acceptProposedAction();
	else
		event->ignore();//否则不接受鼠标事件  
}

void MainWindow::dropEvent(QDropEvent *event)
{
	QList<QUrl> urls = event->mimeData()->urls();
	if (urls.isEmpty())
		return;

	LoadStl(urls.first().toLocalFile());
}

void MainWindow::slot_glwidgetSizeChange()
{
	//glwidget相对主窗口的位置=tabWidget相对主窗口的位置+glwidget相对tabWidget的位置
	m_centerTopWidget->resize(QRect(m_tabWidget->pos(), m_glwidget->size()));
}

void MainWindow::slot_previewSizeChange()
{
	m_previewTopWidget->resize(QRect(m_tabWidget->pos(), m_previewWidget->size()));
}

void MainWindow::InitAction()
{
	//“文件”主菜单
	QMenu* fileMenu = menuBar()->addMenu(QStringLiteral("文件(F)"));

	QAction* newAct = new QAction(QStringLiteral("新建项目"), this);
	fileMenu->addAction(newAct);
	(void)connect(newAct, SIGNAL(triggered()), this, SLOT(slot_newJob()));

	QAction* openAct = new QAction(QStringLiteral("打开文件"), this);
	fileMenu->addAction(openAct);
	(void)connect(openAct, SIGNAL(triggered()), this, SLOT(slot_openStl()));

	QAction* deleteAct = new QAction(QStringLiteral("删除模型"), this);
	fileMenu->addAction(deleteAct);
	(void)connect(deleteAct, SIGNAL(triggered()), m_glwidget, SLOT(slot_delSelectIntance()));

	fileMenu->addSeparator();

	QAction* exitAct = new QAction(QStringLiteral("退出"), this);
	fileMenu->addAction(exitAct);
	(void)connect(exitAct, SIGNAL(triggered()), this, SLOT(slot_exit()));

	//“编辑”主菜单
	QMenu* editMenu = menuBar()->addMenu(QStringLiteral("编辑(E)"));

	editMenu->addSeparator();

	QAction* autoAct = new QAction(QStringLiteral("自动排列"), this);
	editMenu->addAction(autoAct);
	(void)connect(autoAct, SIGNAL(triggered()), this, SLOT(slot_autoArrange()));

	QAction*saveViewAct = new QAction(QStringLiteral("截图"), this);
	editMenu->addAction(saveViewAct);
	(void)connect(saveViewAct, SIGNAL(triggered()), this, SLOT(slot_saveView()));

	QAction* duplicateAct = new QAction(QStringLiteral("复制"), this);
	editMenu->addAction(duplicateAct);
	(void)connect(duplicateAct, SIGNAL(triggered()), this, SLOT(slot_duplicate()));

	//“支撑”主菜单
	QMenu* supportMenu = menuBar()->addMenu(QStringLiteral("支撑(S)"));

	QAction* supportAct = new QAction(QStringLiteral("支撑"), this);
	supportMenu->addAction(supportAct);
	(void)connect(supportAct, SIGNAL(triggered()), this, SLOT(slot_generateSupport()));

	QAction* deleteSupportAct = new QAction(QStringLiteral("删除支撑"));
	supportMenu->addAction(deleteSupportAct);
	(void)connect(deleteSupportAct, SIGNAL(triggered()), this, SLOT(slot_delSelSupport()));

	QAction* supportAllAct = new QAction(QStringLiteral("支撑所有模型"));
	supportMenu->addAction(supportAllAct);
	(void)connect(supportAllAct, SIGNAL(triggered()), this, SLOT(slot_generateAllSupport()));

    //“视图”主菜单
	QMenu* viewMenu = menuBar()->addMenu(QStringLiteral("视图(V)"));

	QAction* defaultAct = new QAction(QIcon(":/icon/images/iso.png"), QStringLiteral("默认视图"), this);
	viewMenu->addAction(defaultAct);
	(void)connect(defaultAct, SIGNAL(triggered()), this, SLOT(slot_defaultView()));

	QAction* overlookAct = new QAction(QIcon(":/icon/images/top.png"),QStringLiteral("俯视图"), this);
	viewMenu->addAction(overlookAct);
	(void)connect(overlookAct, SIGNAL(triggered()), this, SLOT(slot_overlookView()));

	QAction* leftAct = new QAction(QIcon(":/icon/images/left.png"), QStringLiteral("左视图"), this);
	viewMenu->addAction(leftAct);
	(void)connect(leftAct, SIGNAL(triggered()), this, SLOT(slot_leftView()));

	QAction* rightAct = new QAction(QIcon(":/icon/images/right.png"), QStringLiteral("右视图"), this);
	viewMenu->addAction(rightAct);
	(void)connect(rightAct, SIGNAL(triggered()), this, SLOT(slot_rightView()));

	QAction* frontAct = new QAction(QIcon(":/icon/images/front.png"), QStringLiteral("前视图"), this);
	viewMenu->addAction(frontAct);
	(void)connect(frontAct, SIGNAL(triggered()), this, SLOT(slot_frontView()));

	QAction* behindAct = new QAction(QIcon(":/icon/images/back.png"), QStringLiteral("后视图"), this);
	viewMenu->addAction(behindAct);
	(void)connect(behindAct, SIGNAL(triggered()), this, SLOT(slot_behindView()));

	viewMenu->addSeparator();

	QAction* persperctiveAct = new QAction(QStringLiteral("透视投影"), this);
	viewMenu->addAction(persperctiveAct);
	(void)connect(persperctiveAct, SIGNAL(triggered()), this, SLOT(slot_setPersperctive()));

	QAction* orthogonalityAct = new QAction(QStringLiteral("正交投影"), this);
	viewMenu->addAction(orthogonalityAct);
	(void)connect(orthogonalityAct, SIGNAL(triggered()), this, SLOT(slot_setOrthogonality()));
}

void MainWindow::slot_ZPosZero()
{
	if (m_glwidget->m_selInstance == nullptr)
		return;

	TriangleMesh mesh = m_glwidget->m_selInstance->get_object()->volumes[0]->mesh;
	m_glwidget->m_selInstance->transform_mesh(&mesh);
	BoundingBoxf3 box = mesh.bounding_box();

	m_glwidget->DelSelectSupport();
	m_glwidget->OffsetValueChange(0, 0, -box.min.z);

	this->SetDLPrintDirty();
}

void MainWindow::SetOffsetValue(ModelInstance* instance)
{
	m_centerTopWidget->x_offset_spin->setValue(instance->offset.x);
	m_centerTopWidget->y_offset_spin->setValue(instance->offset.y);
	m_centerTopWidget->z_offset_spin->setValue(instance->z_translation);
}

void MainWindow::slot_xoffsetValueChange(double value)
{
	if (m_glwidget->m_selInstance == nullptr)
		return;

	double x = value - m_glwidget->m_selInstance->offset.x;
	if (fabs(x) >= 0.1) {
		m_glwidget->DelSelectSupport();
		m_glwidget->OffsetValueChange(x, 0, 0);
		this->SetDLPrintDirty();
	}
}

void MainWindow::slot_yoffsetValueChange(double value)
{
	if (m_glwidget->m_selInstance == nullptr)
		return;

	double y = value - m_glwidget->m_selInstance->offset.y;
	if (fabs(y) >= 0.1) {
		m_glwidget->DelSelectSupport();
		m_glwidget->OffsetValueChange(0, y, 0);
		this->SetDLPrintDirty();
	}
}

void MainWindow::slot_zoffsetValueChange(double value)
{
	if (m_glwidget->m_selInstance == nullptr)
		return;

	double z = value - m_glwidget->m_selInstance->z_translation;
	if (fabs(z) >= 0.1) {
		m_glwidget->DelSelectSupport();
		m_glwidget->OffsetValueChange(0, 0, z);
		this->SetDLPrintDirty();
	}
}

void MainWindow::SetRotateValue(ModelInstance* instance)
{
	//m_centerTopWidget->x_rotate_spin->setValue(m_centerTopWidget->x_rotate);
	//m_centerTopWidget->y_rotate_spin->setValue(m_centerTopWidget->y_rotate);
	//m_centerTopWidget->z_rotate_spin->setValue(m_centerTopWidget->z_rotate);
}

void MainWindow::slot_xRotateValueChange(double angle)
{
	if (m_glwidget->m_selInstance == nullptr)
		return;

	double x = angle - m_centerTopWidget->x_rotate;

	if (angle == 360 || angle == -360) {
		m_centerTopWidget->x_rotate = 0;
		m_centerTopWidget->x_rotate_spin->setValue(0);
	}
	else
		m_centerTopWidget->x_rotate = angle;

	if (fabs(x) > 0.01) {
		m_glwidget->DelSelectSupport();
		m_glwidget->RotateValueChange(x, 1, 0, 0);
		this->SetDLPrintDirty();
	}
}

void MainWindow::slot_yRotateValueChange(double angle)
{
	if (m_glwidget->m_selInstance == nullptr)
		return;

	double y = angle - m_centerTopWidget->y_rotate;

	if (angle == 360 || angle == -360) {
		m_centerTopWidget->y_rotate = 0;
		m_centerTopWidget->y_rotate_spin->setValue(0);
	}
	else
		m_centerTopWidget->y_rotate = angle;

	if (fabs(y) > 0.01) {
		m_glwidget->DelSelectSupport();
		m_glwidget->RotateValueChange(y, 0, 1, 0);
		this->SetDLPrintDirty();
	}
}

void MainWindow::slot_zRotateValueChange(double angle)
{
	if (m_glwidget->m_selInstance == nullptr)
		return;

	double z = angle - m_centerTopWidget->z_rotate;

	if (angle == 360 || angle == -360) {
		m_centerTopWidget->z_rotate = 0;
		m_centerTopWidget->z_rotate_spin->setValue(0);
	}
	else
		m_centerTopWidget->z_rotate = angle;

	if (fabs(z) > 0.01) {
		m_glwidget->DelSelectSupport();
		m_glwidget->RotateValueChange(z, 0, 0, 1);
		this->SetDLPrintDirty();
	}
}

void MainWindow::SetScaleValue(ModelInstance* instance)
{
	m_centerTopWidget->x_scale_spin->setValue(instance->scaling_vector.x * 100);
	m_centerTopWidget->y_scale_spin->setValue(instance->scaling_vector.y * 100);
	m_centerTopWidget->z_scale_spin->setValue(instance->scaling_vector.z * 100);
}

void MainWindow::SetSizeValue(ModelInstance* instance)
{
	//得到实例的包围盒
	TriangleMesh mesh = instance->get_object()->volumes[0]->mesh;
	instance->transform_mesh(&mesh);
	BoundingBoxf3 box = mesh.bounding_box();

	m_centerTopWidget->x_size_label->setValue(box.size().x);
	m_centerTopWidget->y_size_label->setValue(box.size().y);
	m_centerTopWidget->z_size_label->setValue(box.size().z);
}

void MainWindow::slot_xScaleValueChange(double value)
{
	if (m_glwidget->m_selInstance == nullptr)
		return;

	double x = value / 100 - m_glwidget->m_selInstance->scaling_vector.x;
	if (fabs(x) > 0.01) {
		m_glwidget->DelSelectSupport();
		if (m_centerTopWidget->isUnityScale())
			m_glwidget->ScaleValueChange(x, 0, 0);
		else {
			double y = m_glwidget->m_selInstance->scaling_vector.y * (x / m_glwidget->m_selInstance->scaling_vector.x);
			double z = m_glwidget->m_selInstance->scaling_vector.z * (x / m_glwidget->m_selInstance->scaling_vector.x);
			m_glwidget->ScaleValueChange(x, y, z);
		}
		this->SetDLPrintDirty();
	}
}

void MainWindow::slot_yScaleValueChange(double value)
{
	if (m_glwidget->m_selInstance == nullptr)
		return;

	double y = value / 100 - m_glwidget->m_selInstance->scaling_vector.y;
	if (fabs(y) > 0.01) {
		m_glwidget->DelSelectSupport();
		if (m_centerTopWidget->isUnityScale())
			m_glwidget->ScaleValueChange(0, y, 0);
		else {
			double x = m_glwidget->m_selInstance->scaling_vector.x * (y / m_glwidget->m_selInstance->scaling_vector.y);
			double z = m_glwidget->m_selInstance->scaling_vector.z * (y / m_glwidget->m_selInstance->scaling_vector.y);
			m_glwidget->ScaleValueChange(x, y, z);
		}
		this->SetDLPrintDirty();
	}
}

void MainWindow::slot_zScaleValueChange(double value)
{
	if (m_glwidget->m_selInstance == nullptr)
		return;

	double z = value / 100 - m_glwidget->m_selInstance->scaling_vector.z;
	if (fabs(z) > 0.01) {
		m_glwidget->DelSelectSupport();
		if (m_centerTopWidget->isUnityScale())
			m_glwidget->ScaleValueChange(0, 0, z);
		else {
			double x = m_glwidget->m_selInstance->scaling_vector.x * (z / m_glwidget->m_selInstance->scaling_vector.z);
			double y = m_glwidget->m_selInstance->scaling_vector.y * (z / m_glwidget->m_selInstance->scaling_vector.z);
			m_glwidget->ScaleValueChange(x, y, z);
		}
		this->SetDLPrintDirty();
	}
}

void MainWindow::slot_newJob()//新建项目
{
	m_model->clear_materials();
	m_model->clear_objects();
	m_dlprint->DelAllSupport();
	m_glwidget->ClearModelBuffer();
	m_glwidget->ClearSupportBuffer();
	this->SetDLPrintDirty();
}

void MainWindow::slot_modelSelect()
{
	//error:选中模型会将支撑删除
	//slot_offsetChange();
	//slot_rotateChange();
	//slot_scaleChange();
}

void MainWindow::slot_generateSupport()
{
	m_centerTopWidget->CenterButtonPush(CenterTopWidget::SUPPORTBTN);

	if (m_glwidget->m_selInstance == nullptr) {
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("请选中一个模型。"));
		m_centerTopWidget->CenterButtonPush(CenterTopWidget::SUPPORTBTN);
		return;
	}

	m_progressWidget->ShowProgress(m_tabWidget->rect(), QStringLiteral("支撑"));
	m_progressWidget->P(10);

	//删除选中支撑，提升一定高度
	if (!m_glwidget->DelSelectSupport())
		m_glwidget->OffsetValueChange(0, 0, m_config->model_lift);

	m_glwidget->GenSelInstanceSupport(m_progressWidget->m_progressBar);
	m_progressWidget->P(100);
	m_progressWidget->hide();

	m_centerTopWidget->CenterButtonPush(CenterTopWidget::SUPPORTBTN);

	this->SetDLPrintDirty();
}

void MainWindow::slot_delSelSupport()
{
	if(m_glwidget->DelSelectSupport())
		this->SetDLPrintDirty();
}

void MainWindow::slot_generateAllSupport()
{
	for (ModelObjectPtrs::const_iterator o = m_model->objects.begin(); o != m_model->objects.end(); ++o) {
		for (ModelInstancePtrs::const_iterator i = (*o)->instances.begin(); i != (*o)->instances.end(); ++i) {
			size_t id = m_model->find_id(*i);

			m_progressWidget->ShowProgress(this->rect(), QStringLiteral("支撑"));
			m_progressWidget->P(10);

			m_glwidget->DelSupport(id);//不提升
			//if (!m_glwidget->DelSupport(id))
			//	m_glwidget->OffsetValueChange(0, 0, m_config->model_lift);

			m_glwidget->GenInstanceSupport(id, m_progressWidget->m_progressBar);
			m_progressWidget->P(100);
			m_progressWidget->hide();

			this->SetDLPrintDirty();
		}
	}
}

void MainWindow::slot_saveView()
{
	QString path = QFileDialog::getSaveFileName(this, QStringLiteral("保存切片文件"), 
		QStandardPaths::writableLocation(QStandardPaths::DesktopLocation), "histogram file(*.bmp)");
	if (path.size() > 0)
		m_glwidget->SaveOneView(path.toLocal8Bit().data());
}

void MainWindow::slot_supportEdit()
{
	if (!m_centerTopWidget->m_supportEditBtn->c_isChecked())
	{
		m_centerTopWidget->CenterButtonPush(CenterTopWidget::SUPPORTEDITBTN);
		m_glwidget->SupportEditChange();
	}
	else {
		//-----------退出支撑编辑模式，更新支撑点------------
		m_progressWidget->ShowProgress(m_tabWidget->rect(), QStringLiteral("更新支撑"));
		m_glwidget->SupportEditChange(m_progressWidget->m_progressBar);
		m_progressWidget->hide();

		m_centerTopWidget->CenterButtonPush(CenterTopWidget::SUPPORTEDITBTN);
	}
}

void MainWindow::slot_slice()
{
	m_centerTopWidget->CenterButtonPush(CenterTopWidget::SLICEBTN);

	if (m_model->objects.size() == 0) {
		m_centerTopWidget->CenterButtonPush(CenterTopWidget::SLICEBTN);
		return;
	}

	if (!m_glwidget->CheckConfine()) {
		//在用户目录下建立一个保存切片的文件夹
		clearDir(e_setting.ZipTempPath.c_str());

		QDir file(e_setting.ZipTempPath.c_str());
		if (!file.exists()) {
			if (!file.mkdir(e_setting.ZipTempPath.c_str()))
				exit(2);
		}

		//保存一张截图
		QString view(e_setting.ZipTempPath.c_str());
		view.append("/A_Front.bmp");
		char* _path = view.toLocal8Bit().data();
		m_glwidget->SaveOneView(_path);

		m_progressWidget->ShowProgress(m_tabWidget->rect(), QStringLiteral("切片"));

		m_progressWidget->P(10);
		GenAllInsideSupport();
		m_progressWidget->P(20);
		m_dlprint->Slice(m_glwidget->GetSupportModel(), m_progressWidget->m_progressBar);
		m_progressWidget->P(100);
		m_progressWidget->hide();
		m_previewWidget->reload();
		m_previewTopWidget->reload(m_dlprint->layer_qt_path.size() + 1);
	}
	else
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("模型超过边界，无法切片。"));

	m_centerTopWidget->CenterButtonPush(CenterTopWidget::SLICEBTN);
	m_tabWidget->setCurrentIndex(1);
}

void MainWindow::slot_saveSlice()
{
	QString path = QFileDialog::getSaveFileName(this, QStringLiteral("保存切片文件"),
		QStandardPaths::writableLocation(QStandardPaths::DesktopLocation), QString("histogram file(*.%1)").arg(e_setting.SuffixName.c_str()));
	if (path.size() > 0) {
		m_progressWidget->ShowProgress(m_tabWidget->rect(), QStringLiteral("保存"));
		m_progressWidget->P(35);
		m_dlprint->SaveSlice();
		m_progressWidget->P(70);
		JlCompress::compressDir(path, e_setting.ZipTempPath.c_str());
		clearDir(e_setting.ZipTempPath.c_str());
		m_progressWidget->hide();
	}
}

//void MainWindow::ShowPreviewWidget(QString zipPath)
//{
//	//解压zip包到用户目录下
//	clearDir(e_setting.ZipTempPath.c_str());
//
//	QDir file(e_setting.ZipTempPath.c_str());
//	if (!file.exists()) {
//		if (!file.mkdir(e_setting.ZipTempPath.c_str()))
//			exit(2);
//	}
//
//	if (ExtractZipPath(zipPath)) {
//		(void)JlCompress::extractDir(zipPath, e_setting.ZipTempPath.c_str());
//
//		PreviewDialog previewDialog;
//		if (previewDialog.readIni(e_setting.ZipTempPath.c_str()))
//			previewDialog.exec();
//	}
//	else
//		QMessageBox::about(this, QStringLiteral("警告"), QString("%1%2%3").arg(QStringLiteral("打开文件")).arg(zipPath).arg(QStringLiteral("失败！")));
//}

bool MainWindow::ExtractZipPath(QString zipPath)
{
	bool bmpBool = false;
	bool pngBool = false;
	bool iniBool = false;
	QStringList list = JlCompress::getFileList(zipPath);
	for (auto l = list.begin(); l != list.end(); ++l) {
		if ((*l).right(4).compare(".bmp") && (*l).right(4).compare(".png") && (*l).right(4).compare(".ini"))
			return false;
		if (!(*l).right(4).compare(".bmp") && !bmpBool)
			bmpBool = true;
		if (!(*l).right(4).compare(".png") && !pngBool)
			pngBool = true;
		if (!(*l).right(4).compare(".ini") && !iniBool)
			iniBool = true;
	}
	return bmpBool && pngBool && iniBool;
}

void MainWindow::slot_autoArrange()
{
	double L = double(e_setting.m_printers.begin()->length / 2);
	double W = double(e_setting.m_printers.begin()->width / 2);

	BoundingBoxf box;
	box.min.x = -L;
	box.min.y = -W;
	box.max.x = L;
	box.max.y = W;
	box.defined = true;
	//未删除支撑？？？
	Pointfs positions = m_model->arrange_objects(m_config->arrange_space, &box);

	m_glwidget->ClearSupportBuffer();

	for (ModelObjectPtrs::const_iterator o = m_model->objects.begin(); o != m_model->objects.end(); ++o) {
	    for (ModelInstancePtrs::const_iterator i = (*o)->instances.begin(); i != (*o)->instances.end(); ++i) {
				size_t id = m_model->find_id(*i);
				
				//将所有实例在xy方向上重新排列
				Pointf p = positions.back();
				positions.pop_back();
				double x = p.x - (*i)->offset.x;
				double y = p.y - (*i)->offset.y;

				(*i)->offset.x += x;
				(*i)->offset.y += y;
				(*i)->update_attribute();

				TreeSupport* ts1 = m_dlprint->GetTreeSupport(id);
				if (ts1!=nullptr) {
					TreeSupport* ts2 = new TreeSupport(ts1);
					ts2->translate_(x, y, 0);
					m_dlprint->InsertSupport(id, ts2);
					m_glwidget->InitTreeSupportID(id, nullptr);
				}
			}
	    (*o)->invalidate_bounding_box();
	}
}

void MainWindow::Duplicate(size_t num, bool arrange)
{
	//返回选中模型支撑
	TreeSupport* ts1 = m_dlprint->GetTreeSupport(m_model->find_id(m_glwidget->m_selInstance));

	for (int i = 0; i < num; ++i) {
		size_t id = m_model->find_id(
			m_model->addInstance(m_model->find_id(m_glwidget->m_selInstance)));

		qDebug() << "duolicate: " << id;

		m_glwidget->AddModelInstance(id);
		if (ts1 != nullptr)
			m_dlprint->InsertSupport(id, new TreeSupport(ts1));
	}
	slot_autoArrange();
}


void MainWindow::slot_duplicate()
{
	if (m_glwidget->m_selInstance == nullptr) {
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("请选中一个模型进行复制。"));
		return;
	}

	bool ret;
	int num = QInputDialog::getInt(this, QStringLiteral("复制"), QStringLiteral("请输入复制模型的个数："), 1, 1, 50, 1, &ret);
	if (ret) 
		Duplicate(num, true);
}

void MainWindow::InitDlprinter()
{
	if (QFile(e_setting.DlprinterFile.c_str()).exists())
		e_setting.setSelMachine(QSettings(e_setting.DlprinterFile.c_str(), QSettings::IniFormat).value("/dlprinter/name").toString().toStdString());
}

void MainWindow::slot_showSetupDialog()
{
	m_centerTopWidget->CenterButtonPush(CenterTopWidget::SETUPBTN);

	if (m_centerTopWidget->m_setupBtn->c_isChecked()) {
		SetupDialog setupDialog(m_config);
		setupDialog.exec();
	}

	m_centerTopWidget->CenterButtonPush(CenterTopWidget::SETUPBTN);
}

void MainWindow::slot_showOffsetWidget()
{
	m_centerTopWidget->CenterButtonPush(CenterTopWidget::OFFSETBTN);

	if (m_centerTopWidget->m_offsetBtn->c_isChecked() && m_glwidget->m_selInstance != nullptr)
		SetOffsetValue(m_glwidget->m_selInstance);
}

void MainWindow::slot_showRotateWidget()
{
	m_centerTopWidget->CenterButtonPush(CenterTopWidget::ROTATEBTN);
}

void MainWindow::slot_showScaleWidget()
{
	m_centerTopWidget->CenterButtonPush(CenterTopWidget::SCALEBTN);

	if (m_centerTopWidget->m_scaleBtn->c_isChecked() && m_glwidget->m_selInstance != nullptr) {
		SetScaleValue(m_glwidget->m_selInstance);
		SetSizeValue(m_glwidget->m_selInstance);
	}
}

void MainWindow::GenAllInsideSupport()
{
	m_dlprint->DelAllInsideSup();
	if (m_config->hollow_out && m_config->fill_pattern == Config::ip3DSupport) {
		for (auto o = m_model->objects.begin(); o != m_model->objects.end(); ++o) {
			for (auto i = (*o)->instances.begin(); i != (*o)->instances.end(); ++i) {
				TriangleMesh mesh((*o)->volumes[0]->mesh);
				(*i)->transform_mesh(&mesh);
				m_dlprint->GenInsideSupport(m_model->find_id(*i), &mesh);
			}
		}
		this->SetDLPrintDirty();
	}
}

void MainWindow::slot_offsetChange()
{
	m_glwidget->DelSelectSupport();
	if (!m_centerTopWidget->m_offsetWidget->isHidden())
		SetOffsetValue(m_glwidget->m_selInstance);
	this->SetDLPrintDirty();
}

void MainWindow::slot_scaleChange()
{
	m_glwidget->DelSelectSupport();
	if (!m_centerTopWidget->m_scaleWidget->isHidden())
		SetScaleValue(m_glwidget->m_selInstance);
	this->SetDLPrintDirty();
}

void MainWindow::slot_rotateChange()
{
	m_glwidget->DelSelectSupport();
	if (!m_centerTopWidget->m_rotateWidget->isHidden())
		SetRotateValue(m_glwidget->m_selInstance);
	this->SetDLPrintDirty();
}

void MainWindow::slot_tabWidgetChange(int index)
{
	if (index == 0) {
		m_previewTopWidget->HideWidget();
		m_centerTopWidget->ShowWidget();
	}
	else if (index == 1) {
		m_centerTopWidget->HideWidget();
		m_previewTopWidget->ShowWidget();
	}
}

void MainWindow::SetDLPrintDirty()
{
	if (!m_previewWidget->m_dirty) {
		m_previewTopWidget->setEnable(false);
		m_previewWidget->m_dirty = true;
	}
}
#include "mainwindow.h"

#include <qmenubar.h>
#include <qevent.h>
#include <qdebug.h>
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

#include "PreviewDialog.h"
#include "IO.hpp"

//ERROR:setting.h放在.cpp文件夹下会报错
//#include "Setting.h"

extern Setting e_setting;

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
	setCentralWidget(m_glwidget);//要先于中央界面上button初始化
	InitAction();
	m_centerTopWidget = std::unique_ptr<CenterTopWidget>(new CenterTopWidget(this));

	connect(m_glwidget, &GlWidget::sig_modelSelect, this, &MainWindow::slot_modelSelect);
	connect(m_glwidget, &GlWidget::sig_offsetChange, this, &MainWindow::slot_offsetChange);
	connect(m_glwidget, &GlWidget::sig_rotateChange, this, &MainWindow::slot_rotateChange);
	connect(m_glwidget, &GlWidget::sig_scaleChange, this, &MainWindow::slot_scaleChange);

	connect(m_centerTopWidget->m_printerCombo, SIGNAL(currentIndexChanged(QString)), m_glwidget, SLOT(slot_dlprinterChange(QString)));

	setAcceptDrops(true);
	setWindowTitle("DLPSlicer");
	setWindowIcon(QIcon(":/icon/images/printer.ico"));
	setAttribute(Qt::WA_QuitOnClose, true);//设置主窗口关闭即程序退出
	resize(1200, 700);
}

MainWindow::~MainWindow()
{
	delete m_centerTopWidget.release();
	delete m_config;
	delete m_model;
	delete m_dlprint;
	//delete m_glwidget;
}

QString MainWindow::ReadStlTxt()
{
	QFile dir(e_setting.ModelFile.c_str());
	if (dir.exists()) {
		QSettings readini(e_setting.ModelFile.c_str(), QSettings::IniFormat);
		//读取打印设置
		return readini.value("/modelPath/path").toString();
	}
	else
		return "";
}

void MainWindow::StroyStlTxt(QString stl)
{
	QSettings writeini(e_setting.ModelFile.c_str(), QSettings::IniFormat);
	writeini.clear();
	writeini.setValue("/modelPath/path", stl);
}

void MainWindow::slot_openStl()
{
 	m_centerTopWidget->CenterButtonPush(OPENBTN);

	QString temp, path;
	path = ReadStlTxt();
	int a = path.lastIndexOf("/");
	QString s = path.left(a);
	temp = QFileDialog::getOpenFileName(this, QStringLiteral("选择需要打开的文件"), s, "*.stl *.sm *.obj");

	if(temp.right(4).indexOf(".stl", 0, Qt::CaseInsensitive) >= 0){
		StroyStlTxt(temp);

		m_centerTopWidget->showProgress(OPENBTN);
		qDebug() << temp;
		std::string file = temp.toStdString();
		m_centerTopWidget->P(20);
		size_t id = m_model->load_model(file);
		m_centerTopWidget->P(60);
		m_glwidget->AddModelInstance(id);
		m_centerTopWidget->P(100);
		m_centerTopWidget->hideProgress();
		m_glwidget->UpdateConfine();
	}
	else if (temp.right(3).indexOf(".sm", 0, Qt::CaseInsensitive) >= 0) 
		ShowPreviewWidget(temp);

	m_centerTopWidget->CenterButtonPush(OPENBTN);
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Delete) {
		m_glwidget->slot_delSelectIntance();
	}
	else if (event->key() == Qt::Key_Return) {
		m_glwidget->AddNewSupportPoint();
	}
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

	//打开stl文件
	QString fileName = urls.first().toLocalFile();
	if (fileName.right(4).indexOf(".stl", 0, Qt::CaseInsensitive) >= 0){
		StroyStlTxt(fileName);

		m_centerTopWidget->showProgress(OPENBTN);
		std::string file = fileName.toStdString();
		m_centerTopWidget->P(20);
		size_t id = m_model->load_model(file);
		m_centerTopWidget->P(60);
		m_glwidget->AddModelInstance(id);
		m_centerTopWidget->P(100);
		m_centerTopWidget->hideProgress();
		m_glwidget->UpdateConfine();
	}
	else if (fileName.right(3).indexOf(".sm", 0, Qt::CaseInsensitive) >= 0)
		ShowPreviewWidget(fileName);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
	m_centerTopWidget->resize(event->size());
}

void MainWindow::InitAction()
{
	//“文件”主菜单
	QMenu* fileMenu = menuBar()->addMenu(QStringLiteral("文件(F)"));

	QAction* newAct = new QAction(QStringLiteral("新建项目"), this);
	fileMenu->addAction(newAct);
	connect(newAct, SIGNAL(triggered()), this, SLOT(slot_newJob()));

	QAction* openAct = new QAction(QStringLiteral("打开文件"), this);
	fileMenu->addAction(openAct);
	connect(openAct, SIGNAL(triggered()), this, SLOT(slot_openStl()));

	QAction* deleteAct = new QAction(QStringLiteral("删除模型"), this);
	fileMenu->addAction(deleteAct);
	connect(deleteAct, SIGNAL(triggered()), m_glwidget, SLOT(slot_delSelectIntance()));

	fileMenu->addSeparator();

	QAction* exitAct = new QAction(QStringLiteral("退出"), this);
	fileMenu->addAction(exitAct);
	connect(exitAct, SIGNAL(triggered()), this, SLOT(slot_exit()));

	//“编辑”主菜单
	QMenu* editMenu = menuBar()->addMenu(QStringLiteral("编辑(E)"));

	editMenu->addSeparator();

	QAction* repairAct = new QAction(QStringLiteral("模型修复"), this);
	editMenu->addAction(repairAct);
	repairAct->setDisabled(true);

	QAction* autoAct = new QAction(QStringLiteral("自动排列"), this);
	editMenu->addAction(autoAct);
	connect(autoAct, SIGNAL(triggered()), this, SLOT(slot_autoArrange()));

	QAction*saveViewAct = new QAction(QStringLiteral("截图"), this);
	editMenu->addAction(saveViewAct);
	connect(saveViewAct, SIGNAL(triggered()), this, SLOT(slot_saveView()));

	QAction* duplicateAct = new QAction(QStringLiteral("复制"), this);
	editMenu->addAction(duplicateAct);
	connect(duplicateAct, SIGNAL(triggered()), this, SLOT(slot_duplicate()));

	//“支撑”主菜单
	QMenu* supportMenu = menuBar()->addMenu(QStringLiteral("支撑(S)"));

	QAction* supportAct = new QAction(QStringLiteral("支撑"), this);
	supportMenu->addAction(supportAct);
	connect(supportAct, SIGNAL(triggered()), this, SLOT(slot_generateSupport()));

	QAction* deleteSupportAct = new QAction(QStringLiteral("删除支撑"));
	supportMenu->addAction(deleteSupportAct);
	deleteSupportAct->setEnabled(false);
	//connect(deleteSupportAct, SIGNAL(triggered()), glwidget, SLOT(DelSelectSupport()));

	QAction* supportAllAct = new QAction(QStringLiteral("支撑所有模型"));
	supportMenu->addAction(supportAllAct);
	connect(supportAllAct, SIGNAL(triggered()), this, SLOT(slot_generateAllSupport()));

    //“视图”主菜单
	QMenu* viewMenu = menuBar()->addMenu(QStringLiteral("视图(V)"));

	QAction* defaultAct = new QAction(QIcon(":/icon/images/iso.png"), QStringLiteral("默认视图"), this);
	viewMenu->addAction(defaultAct);
	connect(defaultAct, SIGNAL(triggered()), this, SLOT(slot_defaultView()));

	QAction* overlookAct = new QAction(QIcon(":/icon/images/top.png"),QStringLiteral("俯视图"), this);
	viewMenu->addAction(overlookAct);
	connect(overlookAct, SIGNAL(triggered()), this, SLOT(slot_overlookView()));

	QAction* leftAct = new QAction(QIcon(":/icon/images/left.png"), QStringLiteral("左视图"), this);
	viewMenu->addAction(leftAct);
	connect(leftAct, SIGNAL(triggered()), this, SLOT(slot_leftView()));

	QAction* rightAct = new QAction(QIcon(":/icon/images/right.png"), QStringLiteral("右视图"), this);
	viewMenu->addAction(rightAct);
	connect(rightAct, SIGNAL(triggered()), this, SLOT(slot_rightView()));

	QAction* frontAct = new QAction(QIcon(":/icon/images/front.png"), QStringLiteral("前视图"), this);
	viewMenu->addAction(frontAct);
	connect(frontAct, SIGNAL(triggered()), this, SLOT(slot_frontView()));

	QAction* behindAct = new QAction(QIcon(":/icon/images/back.png"), QStringLiteral("后视图"), this);
	viewMenu->addAction(behindAct);
	connect(behindAct, SIGNAL(triggered()), this, SLOT(slot_behindView()));

	viewMenu->addSeparator();

	QAction* persperctiveAct = new QAction(QStringLiteral("透视投影"), this);
	viewMenu->addAction(persperctiveAct);
	connect(persperctiveAct, SIGNAL(triggered()), this, SLOT(slot_setPersperctive()));

	QAction* orthogonalityAct = new QAction(QStringLiteral("正交投影"), this);
	viewMenu->addAction(orthogonalityAct);
	connect(orthogonalityAct, SIGNAL(triggered()), this, SLOT(slot_setOrthogonality()));
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
	}
}

void MainWindow::slot_zoffsetValueChange(double value)
{
	if (m_glwidget->m_selInstance == nullptr)
		return;

	double z = value - m_glwidget->m_selInstance->z_translation;
	if (fabs(z) >= 0.1) {
		m_glwidget->DelSelectSupport();
		m_glwidget->OffsetValueChange(z, 0, 0);
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
	}
}

void MainWindow::slot_newJob()//新建项目
{
	m_model->clear_materials();
	m_model->clear_objects();
	m_dlprint->DelAllSupport();
	m_glwidget->ClearModelBuffer();
	m_glwidget->ClearSupportBuffer();
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
	m_centerTopWidget->CenterButtonPush(SUPPORTBTN);

	if (m_glwidget->m_selInstance == nullptr) {
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("请选中一个模型。"));
		m_centerTopWidget->CenterButtonPush(SUPPORTBTN);
		return;
	}

	m_centerTopWidget->showProgress(SUPPORTBTN);
	m_centerTopWidget->P(10);

	//删除选中支撑，提升一定高度
	if (!m_glwidget->DelSelectSupport())
		m_glwidget->OffsetValueChange(0, 0, m_config->model_lift);

	m_glwidget->GenSelInstanceSupport(m_centerTopWidget->m_progressBar);
	m_centerTopWidget->P(100);
	m_centerTopWidget->hideProgress();

	m_centerTopWidget->CenterButtonPush(SUPPORTBTN);
}

void MainWindow::slot_generateAllSupport()
{
	for (ModelObjectPtrs::const_iterator o = m_model->objects.begin(); o != m_model->objects.end(); ++o) {
		for (ModelInstancePtrs::const_iterator i = (*o)->instances.begin(); i != (*o)->instances.end(); ++i) {
			size_t id = m_model->find_id(*i);

			m_centerTopWidget->showProgress(SUPPORTBTN);
			m_centerTopWidget->P(10);

			if (!m_glwidget->DelSelectSupport())
				m_glwidget->OffsetValueChange(0, 0, m_config->model_lift);

			m_glwidget->GenSelInstanceSupport(m_centerTopWidget->m_progressBar);
			m_centerTopWidget->P(100);
			m_centerTopWidget->hideProgress();

		}
	}
}

void MainWindow::slot_saveView()
{
	QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	QString path = QFileDialog::getSaveFileName(this, QStringLiteral("保存切片文件"), desktop, "histogram file(*.bmp)");
	if (path.size() > 0) {
		QByteArray a = path.toLocal8Bit();
		char* file = a.data();
		m_glwidget->SaveOneView(file);
	}
}

void MainWindow::slot_supportEdit()
{
	if (!m_centerTopWidget->m_supportEditBtn->c_isChecked())
	{
		m_centerTopWidget->CenterButtonPush(SUPPORTEDITBTN);
		m_glwidget->SupportEditChange();
	}
	else {
		//-----------退出支撑编辑模式，更新支撑点------------
		m_centerTopWidget->showProgress(SUPPORTEDITBTN);
		m_glwidget->SupportEditChange(m_centerTopWidget->m_progressBar);
		m_centerTopWidget->hideProgress();

		m_centerTopWidget->CenterButtonPush(SUPPORTEDITBTN);
	}
}

void MainWindow::slot_slice()
{
	m_centerTopWidget->CenterButtonPush(SLICEBTN);

	if (m_model->objects.size() == 0) {
		m_centerTopWidget->CenterButtonPush(SLICEBTN);
		return;
	}

	if (!m_glwidget->CheckConfine()) {
		QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
		QString path = QFileDialog::getSaveFileName(this, QStringLiteral("保存切片文件"), desktop, "histogram file(*.sm)");
		if (path.size() > 0) {
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
			
			m_centerTopWidget->showProgress(SLICEBTN);
			
			m_centerTopWidget->P(10);
			GenAllInsideSupport();
			m_centerTopWidget->P(20);
			m_dlprint->Slice(m_glwidget->GetSupportModel(), m_centerTopWidget->m_progressBar);
			m_centerTopWidget->P(99);
			JlCompress::compressDir(path, e_setting.ZipTempPath.c_str());
			m_centerTopWidget->P(100);

			//清空切片文件夹
			clearDir(e_setting.ZipTempPath.c_str());

			m_centerTopWidget->hideProgress();

			switch (QMessageBox::question(this, QStringLiteral("提示"), QStringLiteral("切片完成，是否预览切片文件？"), QMessageBox::Cancel, QMessageBox::Ok))
			{
			case QMessageBox::Ok:
				ShowPreviewWidget(path);
				break;
			case QMessageBox::Cancel:
				break;
			default:
				break;
			}
		
		}
	}
	else {
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("模型超过边界，无法切片。"));
	}

	m_centerTopWidget->CenterButtonPush(SLICEBTN);
}

void MainWindow::ShowPreviewWidget(QString zipPath)
{
	//解压zip包到用户目录下
	clearDir(e_setting.ZipTempPath.c_str());

	QDir file(e_setting.ZipTempPath.c_str());
	if (!file.exists()) {
		if (!file.mkdir(e_setting.ZipTempPath.c_str()))
			exit(2);
	}

	if (ExtractZipPath(zipPath)) {
		JlCompress::extractDir(zipPath, e_setting.ZipTempPath.c_str());

		PreviewDialog previewDialog;
		if (previewDialog.readIni(e_setting.ZipTempPath.c_str()))
			previewDialog.exec();
	}
	else {
		QString text(QStringLiteral("打开文件"));
		text.append(zipPath);
		text.append(QStringLiteral("失败！"));
		QMessageBox::about(this, QStringLiteral("警告"), text);
	}
}

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
		ModelInstance* s = m_model->addInstance(m_model->find_id(m_glwidget->m_selInstance));
		size_t id = m_model->find_id(s);
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
	if (QFile(e_setting.DlprinterFile.c_str()).exists()) {
		QSettings readini(e_setting.DlprinterFile.c_str(), QSettings::IniFormat);
		//读取打印设置
		QString name = readini.value("/dlprinter/name").toString();
		e_setting.setSelMachine(name.toStdString());
	}
}

void MainWindow::slot_showSetupDialog()
{
	m_centerTopWidget->CenterButtonPush(SETUPBTN);

	if (m_centerTopWidget->m_setupBtn->c_isChecked()) {
		SetupDialog setupDialog(m_config);
		setupDialog.exec();
	}

	m_centerTopWidget->CenterButtonPush(SETUPBTN);
}

void MainWindow::slot_showOffsetWidget()
{
	m_centerTopWidget->CenterButtonPush(OFFSETBTN);

	if (m_centerTopWidget->m_offsetBtn->c_isChecked() && m_glwidget->m_selInstance != nullptr)
		SetOffsetValue(m_glwidget->m_selInstance);
}

void MainWindow::slot_showRotateWidget()
{
	m_centerTopWidget->CenterButtonPush(ROTATEBTN);
}

void MainWindow::slot_showScaleWidget()
{
	m_centerTopWidget->CenterButtonPush(SCALEBTN);

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
				m_dlprint->GenInsideSupport(std::distance(m_model->objects.begin(), o) * InstanceNum + std::distance((*o)->instances.begin(), i), &mesh);
			}
		}
	}
}

void MainWindow::slot_offsetChange()
{
	m_glwidget->DelSelectSupport();
	if (!m_centerTopWidget->m_offsetWidget->isHidden())
		SetOffsetValue(m_glwidget->m_selInstance);
}

void MainWindow::slot_scaleChange()
{
	m_glwidget->DelSelectSupport();
	if (!m_centerTopWidget->m_scaleWidget->isHidden())
		SetScaleValue(m_glwidget->m_selInstance);
}

void MainWindow::slot_rotateChange()
{
	m_glwidget->DelSelectSupport();
	if (!m_centerTopWidget->m_rotateWidget->isHidden())
		SetRotateValue(m_glwidget->m_selInstance);
}

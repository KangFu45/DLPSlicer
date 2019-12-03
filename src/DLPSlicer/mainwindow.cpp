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

#include <quazip/JlCompress.h>

#include "IO.hpp"
#include "tool.h"

//ERROR:setting.h放在.cpp文件夹下会报错
//#include "Setting.h"

extern Setting e_setting;

AboutDialog::AboutDialog()
{
	icoBtn = new QPushButton(QIcon(":/icon/images/SM.png"), "");
	icoBtn->setFixedSize(60, 70);
	icoBtn->setIconSize(QSize(60, 70));
	icoBtn->setStyleSheet("background-color: rgba(225,225,225,0);");

	QFont font("ZYSong18030", 10);
	nameLabel = new QLabel("S-Maker_DLP1.6.7  64bit");
	nameLabel->setFont(font);

	QString str(QStringLiteral("速美科技版权所有"));
	str.append("\n@2017-2018");
	copyrightLabel = new QLabel(str);

	versionLabel = new QLabel(QStringLiteral("版本更新:"));

	textEdit = new QPlainTextEdit();
	textEdit->setReadOnly(true);
	textEdit->setPlainText(QStringLiteral(
		"版本--1.6.7\n"
		"1.增加机型选择功能。\n"
		"2.增加Dental_DLP机型。\n"
		"版本--1.6.6\n"
		"1.提高手动支撑效率。\n"
		"2.拖动功能的优化。\n"
		"3.增加obj文件格式。\n"
		"4.更新图标。\n"
		"版本--1.6.5\n"
		"1.内存优化。\n"
		"2.删除支撑改为左键单击。\n"
		"3.添加底板补偿参数。\n"
		"4.针对悬吊面按形状均匀支撑。\n"
		"5.修复复杂模型支撑容易穿过的bug。\n"
		"6.支撑底端与模型接触成角度。\n"
		"7.其他修改。\n"
		"版本--1.6.4\n"
		"1.优化进度条显示。\n"
		"2.增加模型拖拽功能。\n"
		"3.优化自动支撑。\n"
		"4.增加蜂窝填充。\n"
		"5.其他修改。\n"
		"版本--1.6.3\n"
		"1.添加“撤销重做”功能。\n"
		"2.支撑头与模型成角度。\n"
		"3.模型转换操作实时显示。\n"
		"4.增加透视投影与正交投影。\n"
		"5.排除支撑直接穿过模型的bug。\n"
		"6.支撑树枝部分加强。\n"
		"7.其他修改。\n"
		"版本--1.6.2\n"
		"1.树状支撑。\n"
		"2.可单独添加支撑。\n"
		"3.改变支撑编辑的方式。\n"
		"版本--1.6.1\n"
		"1.添加自动排列功能。\n"
		"2.添加模型复制功能。\n"
		"3.添加坐标轴信息。\n"
		"4.修复支撑渲染效果不真实的bug。\n"
		"5.优化手动添加支撑。\n"
		"6.排除添加支撑时悬吊面对悬吊点的影响。\n"
		"7.优化切片时出现断层的影响。\n"
		"8.调整界面的逻辑结构。\n"
		"9.添加可直接对模型进行手动支撑的操作。\n"
		"10.优化图形渲染时内存消耗。\n"
		"版本--1.6.0\n"
		"1.调整模型视图渲染效果。\n"
		"2.渲染实体支撑。\n"
		"3.支持模型平移，缩放，旋转三个方向的操作。\n"
		"4.增加支撑编辑功能，实现手动支撑，删除支撑的操作。\n"
		"5.多线程生成图片。\n"
		"6.切片生成的文件为.sm的自定义的文件。\n"
		"7.丰富设置选项。\n"
		"8.增加浏览切片文件的功能。\n"
		"9.视图内的模型支撑可另存为stl模型。\n"
		"10.应用程序支持64bit。\n"
		"11.调整内部支撑的结构。\n"
		"12.界面重新调整。"));

	layout = new QGridLayout();
	layout->addWidget(icoBtn, 0, 0, 2, 2);
	layout->addWidget(nameLabel, 0, 2, 1, 3);
	layout->addWidget(copyrightLabel, 1, 2, 1, 3);
	layout->addWidget(versionLabel, 2, 0, 1, 2);
	layout->addWidget(textEdit, 3, 0, 8, 5);
	setLayout(layout);
	setFixedSize(400, 300);
}

AboutDialog::~AboutDialog()
{
	delete icoBtn;
	delete nameLabel;
	delete copyrightLabel;
	delete versionLabel;
	delete textEdit;
	delete layout;
}

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	initDlprinter();
	initAction();
	m_dlprint = new DLPrint(m_model, dlprinter);
	initSetupDialog();
	glwidget = new GlWidget(dlprinter, m_model, m_dlprint, treeSupportsExist);
	setCentralWidget(glwidget);//要先于中央界面上button初始化
	m_centerTopWidget = std::unique_ptr<CenterTopWidget>(new CenterTopWidget(this));

	connect(glwidget, &GlWidget::signal_modelSelect, this, &MainWindow::slot_modelSelect);
	connect(glwidget, &GlWidget::signal_offsetChange, this, &MainWindow::slot_offsetChange);
	connect(glwidget, &GlWidget::signal_rotateChange, this, &MainWindow::slot_rotateChange);
	connect(glwidget, &GlWidget::signal_scaleChange, this, &MainWindow::slot_scaleChange);

	setAcceptDrops(true);
	setWindowTitle("DLPSlicer");
	setAttribute(Qt::WA_QuitOnClose, true);//设置主窗口关闭即程序退出
	resize(1200, 700);
}

MainWindow::~MainWindow()
{

}

QString MainWindow::readStlTxt()
{
	QFile dir(e_setting.ModelFile.c_str());
	if (dir.exists()) {
		QSettings* readini = new QSettings(e_setting.ModelFile.c_str(), QSettings::IniFormat);
		//读取打印设置
		QString path = readini->value("/modelPath/path").toString();
		delete readini;
		return path;
	}
	else
		return "";
}

void MainWindow::stroyStlTxt(QString stl)
{
	QSettings* writeini = new QSettings(e_setting.ModelFile.c_str(), QSettings::IniFormat);
	writeini->clear();
	writeini->setValue("/modelPath/path", stl);
	delete writeini;
}

void MainWindow::openStl()
{
 	m_centerTopWidget->CenterButtonPush(OPENBTN);

	QString temp, path;
	path = readStlTxt();
	int a = path.lastIndexOf("/");
	QString s = path.left(a);
	temp = QFileDialog::getOpenFileName(this, QStringLiteral("选择需要打开的文件"), s, "*.stl *.sm *.obj");

	int format = -1;
	if(temp.right(4).indexOf(".stl", 0, Qt::CaseInsensitive) >= 0)
		format = stl;
	else if (temp.right(4).indexOf(".obj", 0, Qt::CaseInsensitive) >= 0)
		format = obj;
	else if (temp.right(4).indexOf(".amf", 0, Qt::CaseInsensitive) >= 0)
		format = amf;

	if(format>=0){
		stroyStlTxt(temp);

		m_centerTopWidget->showProgress(OPENBTN);
		qDebug() << temp;
		std::string file = temp.toStdString();
		m_centerTopWidget->P(20);
		size_t id = m_model->load_model(file, format);
		m_centerTopWidget->P(60);
		glwidget->save_valume(id);
		m_centerTopWidget->P(80);
		addModelBuffer(id);
		m_centerTopWidget->P(100);
		m_centerTopWidget->hideProgress();
		glwidget->updateConfine();
		glwidget->updateTranslationID();
	}
	else if (temp.right(3).indexOf(".sm", 0, Qt::CaseInsensitive) >= 0) {
		showPreviewWidget1(temp);
	}

	m_centerTopWidget->CenterButtonPush(OPENBTN);
}

void MainWindow::deleteStl()
{
	if (glwidget->selectID >= 0) {
		deleteSupport();
		deleteModelBuffer(glwidget->selectID);

		glwidget->updateConfine();
		glwidget->selectID = -1;
	}
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Delete) {
		deleteStl();
	}
	else if (event->key() == Qt::Key_Return) {
		glwidget->addOneSupport();
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
	int format = -1;
	if (fileName.right(4).indexOf(".stl", 0, Qt::CaseInsensitive) >= 0)
		format = stl;
	else if (fileName.right(4).indexOf(".obj", 0, Qt::CaseInsensitive) >= 0)
		format = obj;
	else if (fileName.right(4).indexOf(".amf", 0, Qt::CaseInsensitive) >= 0)
		format = amf;

	if (format >= 0) {
		stroyStlTxt(fileName);

		m_centerTopWidget->showProgress(OPENBTN);
		std::string file = fileName.toStdString();
		m_centerTopWidget->P(20);
		size_t id = m_model->load_model(file, format);
		m_centerTopWidget->P(60);
		glwidget->save_valume(id);
		m_centerTopWidget->P(80);
		//MainUndoStack->push(new AddModelCommand(id, this));
		m_centerTopWidget->P(100);
		m_centerTopWidget->hideProgress();
		glwidget->updateConfine();

		glwidget->updateTranslationID();
	}
	else if (fileName.right(3).indexOf(".sm", 0, Qt::CaseInsensitive) >= 0) {
		showPreviewWidget1(fileName);
	}
}

void MainWindow::_exit()
{
	exit(0);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
	m_centerTopWidget->resize(event->size());
}

void MainWindow::initAction()
{
	//“文件”主菜单
	QMenu* fileMenu = menuBar()->addMenu(QStringLiteral("文件(F)"));

	QAction* newAct = new QAction(QStringLiteral("新建项目"), this);
	fileMenu->addAction(newAct);
	connect(newAct, SIGNAL(triggered()), this, SLOT(newJob()));

	QAction* openAct = new QAction(QStringLiteral("打开文件"), this);
	fileMenu->addAction(openAct);
	connect(openAct, SIGNAL(triggered()), this, SLOT(openStl()));

	QAction* deleteAct = new QAction(QStringLiteral("删除模型"), this);
	fileMenu->addAction(deleteAct);
	connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteStl()));

	fileMenu->addSeparator();

	QAction* exitAct = new QAction(QStringLiteral("退出"), this);
	fileMenu->addAction(exitAct);
	connect(exitAct, SIGNAL(triggered()), this, SLOT(_exit()));

	//“编辑”主菜单
	QMenu* editMenu = menuBar()->addMenu(QStringLiteral("编辑(E)"));

	editMenu->addSeparator();

	QAction* repairAct = new QAction(QStringLiteral("模型修复"), this);
	editMenu->addAction(repairAct);
	repairAct->setDisabled(true);

	QAction* autoAct = new QAction(QStringLiteral("自动排列"), this);
	editMenu->addAction(autoAct);
	connect(autoAct, SIGNAL(triggered()), this, SLOT(autoArrange()));

	QAction*saveViewAct = new QAction(QStringLiteral("截图"), this);
	editMenu->addAction(saveViewAct);
	connect(saveViewAct, SIGNAL(triggered()), this, SLOT(saveView()));

	QAction* duplicateAct = new QAction(QStringLiteral("复制"), this);
	editMenu->addAction(duplicateAct);
	connect(duplicateAct, SIGNAL(triggered()), this, SLOT(_duplicate()));

	//“支撑”主菜单
	QMenu* supportMenu = menuBar()->addMenu(QStringLiteral("支撑(S)"));

	QAction* supportAct = new QAction(QStringLiteral("支撑"), this);
	supportMenu->addAction(supportAct);
	connect(supportAct, SIGNAL(triggered()), this, SLOT(generateSupport()));

	QAction* deleteSupportAct = new QAction(QStringLiteral("删除支撑"));
	supportMenu->addAction(deleteSupportAct);
	connect(deleteSupportAct, SIGNAL(triggered()), this, SLOT(deleteSupport()));

	QAction* supportAllAct = new QAction(QStringLiteral("支撑所有模型"));
	supportMenu->addAction(supportAllAct);
	connect(supportAllAct, SIGNAL(triggered()), this, SLOT(generateAllSupport()));

	QAction* deleteAllSupportAct = new QAction(QStringLiteral("删除所有支撑"));
	supportMenu->addAction(deleteAllSupportAct);
	connect(deleteAllSupportAct, SIGNAL(triggered()), this, SLOT(deleteAllSupport()));

    //“视图”主菜单
	QMenu* viewMenu = menuBar()->addMenu(QStringLiteral("视图(V)"));

	QAction* defaultAct = new QAction(QIcon(":/icon/images/iso.png"), QStringLiteral("默认视图"), this);
	viewMenu->addAction(defaultAct);
	connect(defaultAct, SIGNAL(triggered()), this, SLOT(defaultView()));

	QAction* overlookAct = new QAction(QIcon(":/icon/images/top.png"),QStringLiteral("俯视图"), this);
	viewMenu->addAction(overlookAct);
	connect(overlookAct, SIGNAL(triggered()), this, SLOT(overlookView()));

	QAction* leftAct = new QAction(QIcon(":/icon/images/left.png"), QStringLiteral("左视图"), this);
	viewMenu->addAction(leftAct);
	connect(leftAct, SIGNAL(triggered()), this, SLOT(leftView()));

	QAction* rightAct = new QAction(QIcon(":/icon/images/right.png"), QStringLiteral("右视图"), this);
	viewMenu->addAction(rightAct);
	connect(rightAct, SIGNAL(triggered()), this, SLOT(rightView()));

	QAction* frontAct = new QAction(QIcon(":/icon/images/front.png"), QStringLiteral("前视图"), this);
	viewMenu->addAction(frontAct);
	connect(frontAct, SIGNAL(triggered()), this, SLOT(frontView()));

	QAction* behindAct = new QAction(QIcon(":/icon/images/back.png"), QStringLiteral("后视图"), this);
	viewMenu->addAction(behindAct);
	connect(behindAct, SIGNAL(triggered()), this, SLOT(behindView()));

	viewMenu->addSeparator();

	QAction* persperctiveAct = new QAction(QStringLiteral("透视投影"), this);
	viewMenu->addAction(persperctiveAct);
	connect(persperctiveAct, SIGNAL(triggered()), this, SLOT(setPersperctive()));

	QAction* orthogonalityAct = new QAction(QStringLiteral("正交投影"), this);
	viewMenu->addAction(orthogonalityAct);
	connect(orthogonalityAct, SIGNAL(triggered()), this, SLOT(setOrthogonality()));

	//“帮助”主菜单
	QMenu* helpMenu = menuBar()->addMenu(QStringLiteral("帮助(H)"));

	QAction* messageAct = new QAction(QStringLiteral("版本信息"), this);
	helpMenu->addAction(messageAct);
	connect(messageAct, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
}

void MainWindow::ZPosZero()
{
	if (glwidget->selectID >= 0) {
		ModelInstance* instance = m_model->find_instance(glwidget->selectID);
		TriangleMesh mesh = instance->get_object()->volumes[0]->mesh;
		instance->transform_mesh(&mesh);
		BoundingBoxf3 box = mesh.bounding_box();

		m_dlprint->delete_tree_support(glwidget->selectID);
		glwidget->offsetValueChange(glwidget->selectID, 0, 0, -box.min.z);
	}
}

void MainWindow::setOffsetValue(ModelInstance* instance)
{
	m_centerTopWidget->x_offset_spin->setValue(instance->offset.x);
	m_centerTopWidget->y_offset_spin->setValue(instance->offset.y);
	m_centerTopWidget->z_offset_spin->setValue(instance->z_translation);
}

void MainWindow::AddOffsetValue(double x, double y, double z)
{
	m_centerTopWidget->x_offset_spin->setValue(m_centerTopWidget->x_offset_spin->value() + x);
	m_centerTopWidget->y_offset_spin->setValue(m_centerTopWidget->y_offset_spin->value() + y);
	m_centerTopWidget->z_offset_spin->setValue(m_centerTopWidget->z_offset_spin->value() + z);
}

void MainWindow::xoffsetValueChange(double value)
{
	if (glwidget->selectID >= 0) {
		ModelInstance* instance = m_model->find_instance(glwidget->selectID);
		if (instance->offset.x != value) {
			double x = value - instance->offset.x;
			if (fabs(x) >= 0.01) {//防止细微误差
				m_dlprint->delete_tree_support(glwidget->selectID);
				glwidget->offsetValueChange(glwidget->selectID, x, 0, 0);
			}
		}
	}
}

void MainWindow::yoffsetValueChange(double value)
{
	if(glwidget->selectID >= 0) {
		ModelInstance* instance = m_model->find_instance(glwidget->selectID);
		if (instance->offset.y != value) {
			double y = value - instance->offset.y;
			if (fabs(y) >= 0.01) {
				m_dlprint->delete_tree_support(glwidget->selectID);
				glwidget->offsetValueChange(glwidget->selectID, 0, y, 0);
			}
		}
	}
}

void MainWindow::zoffsetValueChange(double value)
{
	if(glwidget->selectID >= 0) {
		ModelInstance* instance = m_model->find_instance(glwidget->selectID);
		if (instance->z_translation != value) {
			double z = value - instance->z_translation;
			if (fabs(z) >= 0.01) {
				m_dlprint->delete_tree_support(glwidget->selectID);
				glwidget->offsetValueChange(glwidget->selectID, 0, 0, z);
			}
		}
	}
}

void MainWindow::AddRotateValue(double angle, int x, int y, int z)
{
	//只支持一个方向的角度值，且比例值为1
	TreeSupport* s1 = NULL;
	if (m_dlprint->chilck_tree_support(glwidget->selectID, s1))
		m_dlprint->delete_tree_support(glwidget->selectID);
	glwidget->rotateValueChange(glwidget->selectID, angle, x, y, z);
}

void MainWindow::setRotateValue(ModelInstance* instance)
{
	m_centerTopWidget->x_rotate_spin->setValue(m_centerTopWidget->x_rotate);
	m_centerTopWidget->y_rotate_spin->setValue(m_centerTopWidget->y_rotate);
	m_centerTopWidget->z_rotate_spin->setValue(m_centerTopWidget->z_rotate);
}

void MainWindow::xRotateValueChange(double angle)
{
	if (glwidget->selectID >= 0) {
		double x = angle - m_centerTopWidget->x_rotate;
		
		if (angle == 360 || angle == -360)
		{
			m_centerTopWidget->x_rotate = 0;
			m_centerTopWidget->x_rotate_spin->setValue(0);
		}
		else
			m_centerTopWidget->x_rotate = angle;

		if (fabs(x) > 0.01) {
			m_dlprint->delete_tree_support(glwidget->selectID);
			glwidget->rotateValueChange(glwidget->selectID, x, 1, 0, 0);
		}
	}
}

void MainWindow::yRotateValueChange(double angle)
{
	if (glwidget->selectID >= 0) {
		double y = angle - m_centerTopWidget->y_rotate;
		
		if (angle == 360 || angle == -360)
		{
			m_centerTopWidget->y_rotate = 0;
			m_centerTopWidget->y_rotate_spin->setValue(0);
		}
		else
			m_centerTopWidget->y_rotate = angle;

		if (fabs(y) > 0.01) {
			m_dlprint->delete_tree_support(glwidget->selectID);
			glwidget->rotateValueChange(glwidget->selectID, y, 0, 1, 0);
		}
	}
}

void MainWindow::zRotateValueChange(double angle)
{
	if (glwidget->selectID >= 0) {
		double z = angle - m_centerTopWidget->z_rotate;
		
		if (angle == 360 || angle == -360)
		{
			m_centerTopWidget->z_rotate = 0;
			m_centerTopWidget->z_rotate_spin->setValue(0);
		}
		else
			m_centerTopWidget->z_rotate = angle;

		if (fabs(z) > 0.01) {
			m_dlprint->delete_tree_support(glwidget->selectID);
			glwidget->rotateValueChange(glwidget->selectID, z, 0, 0, 1);
		}
	}
}

void MainWindow::AddScaleValue(double x, double y, double z)
{
	if (x != 0)
		m_centerTopWidget->x_scale_spin->setValue(m_centerTopWidget->x_scale_spin->value() + x);
	if (y != 0)
		m_centerTopWidget->y_scale_spin->setValue(m_centerTopWidget->y_scale_spin->value() + y);
	if (z != 0)
		m_centerTopWidget->z_scale_spin->setValue(m_centerTopWidget->z_scale_spin->value() + z);
}

void MainWindow::setScaleValue(ModelInstance* instance)
{
	m_centerTopWidget->x_scale_spin->setValue(instance->scaling_vector.x * 100);
	m_centerTopWidget->y_scale_spin->setValue(instance->scaling_vector.y * 100);
	m_centerTopWidget->z_scale_spin->setValue(instance->scaling_vector.z * 100);
}

void MainWindow::setSizeValue(ModelInstance* instance)
{
	//得到实例的包围盒
	TriangleMesh mesh = instance->get_object()->volumes[0]->mesh;
	instance->transform_mesh(&mesh);
	BoundingBoxf3 box = mesh.bounding_box();

	m_centerTopWidget->x_size_label->setValue(box.size().x);
	m_centerTopWidget->y_size_label->setValue(box.size().y);
	m_centerTopWidget->z_size_label->setValue(box.size().z);
}

void MainWindow::xScaleValueChange(double value)
{
	if (glwidget->selectID >= 0) {
		ModelInstance* instance = m_model->find_instance(glwidget->selectID);
		double xScale = value / 100;
		double x = xScale - instance->scaling_vector.x;
		double y = instance->scaling_vector.y*(x / instance->scaling_vector.x);
		double z = instance->scaling_vector.z*(x / instance->scaling_vector.x);
		if (fabs(x) > 0.01) {
			m_dlprint->delete_tree_support(glwidget->selectID);
			if (m_centerTopWidget->unify_scale->checkState() == Qt::Unchecked)
				glwidget->scaleValueChange(glwidget->selectID, x, 0, 0);
			else 
				glwidget->scaleValueChange(glwidget->selectID, x, y, z);
		}
	}
}

void MainWindow::yScaleValueChange(double value)
{
	if (glwidget->selectID >= 0) {
		ModelInstance* instance = m_model->find_instance(glwidget->selectID);
		double yScale = value / 100;
		double y = yScale - instance->scaling_vector.y;
		double x = instance->scaling_vector.x*(y / instance->scaling_vector.y);
		double z = instance->scaling_vector.z*(y / instance->scaling_vector.y);
		if (fabs(y) > 0.01) {
			m_dlprint->delete_tree_support(glwidget->selectID);
			if (m_centerTopWidget->unify_scale->checkState() == Qt::Unchecked) 
				glwidget->scaleValueChange(glwidget->selectID, 0, y, 0);
			else 
				glwidget->scaleValueChange(glwidget->selectID, x, y, z);
		}
	}
}

void MainWindow::zScaleValueChange(double value)
{
	if (glwidget->selectID >= 0) {
		ModelInstance* instance = m_model->find_instance(glwidget->selectID);
		double zScale = value / 100;
		double z = zScale - instance->scaling_vector.z;
		double x = instance->scaling_vector.x*(z / instance->scaling_vector.z);
		double y = instance->scaling_vector.y*(z / instance->scaling_vector.z);
		if (fabs(z) > 0.01) {
			m_dlprint->delete_tree_support(glwidget->selectID);
			if (m_centerTopWidget->unify_scale->checkState() == Qt::Unchecked)
				glwidget->scaleValueChange(glwidget->selectID, 0, 0, z);
			else 
				glwidget->scaleValueChange(glwidget->selectID, x, y, z);
		}
	}
}

void MainWindow::newJob()//新建项目
{
	m_model->clear_materials();
	m_model->clear_objects();
	m_dlprint->delete_all_support();
	glwidget->clearModelBuffer();
	glwidget->clear_volumes();
	glwidget->clearSupportBuffer();
}

void MainWindow::deleteSupport()
{
	m_dlprint->delete_tree_support(glwidget->selectID);
}

void MainWindow::deleteAllSupport()
{
	for (ModelObjectPtrs::const_iterator o = m_model->objects.begin(); o != m_model->objects.end(); ++o) {
		for (ModelInstancePtrs::const_iterator i = (*o)->instances.begin(); i != (*o)->instances.end(); ++i) {
			if ((*i)->exist)
				m_dlprint->delete_tree_support(m_model->find_id(*i));
		}
	}
}

void MainWindow::slot_modelSelect()
{
	slot_offsetChange();
	slot_rotateChange();
	slot_scaleChange();
}

void MainWindow::generateSupport()
{
	m_centerTopWidget->CenterButtonPush(SUPPORTBTN);

	if (glwidget->selectID >= 0) {
		m_centerTopWidget->showProgress(SUPPORTBTN);
		m_centerTopWidget->P(10);
		TreeSupport* s = new TreeSupport();
		TreeSupport* s1 = new TreeSupport();
		if (m_dlprint->chilck_tree_support(glwidget->selectID, s1))
			deleteSupports(glwidget->selectID);
		else
			glwidget->offsetValueChange(glwidget->selectID, 0, 0, m_dlprint->config.model_lift);

		generate_id_support(glwidget->selectID, s, m_centerTopWidget->m_progressBar);
		addSupports(glwidget->selectID, s, m_centerTopWidget->m_progressBar);
		m_centerTopWidget->P(100);
		m_centerTopWidget->hideProgress();

	}
	else
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("请选中一个模型。"));

	m_centerTopWidget->CenterButtonPush(SUPPORTBTN);
}

void MainWindow::generateAllSupport()
{
	for (ModelObjectPtrs::const_iterator o = m_model->objects.begin(); o != m_model->objects.end(); ++o) {
		for (ModelInstancePtrs::const_iterator i = (*o)->instances.begin(); i != (*o)->instances.end(); ++i) {
			if ((*i)->exist) {
				size_t id = m_model->find_id(*i);

				m_centerTopWidget->showProgress(SUPPORTBTN);
				m_centerTopWidget->P(10);
				TreeSupport* s = new TreeSupport();
				TreeSupport* s1 = new TreeSupport();
				if (m_dlprint->chilck_tree_support(id, s1))
					deleteSupports(id);
				else
					glwidget->offsetValueChange(id, 0, 0, m_dlprint->config.model_lift);

				generate_id_support(id, s, m_centerTopWidget->m_progressBar);
				addSupports(id, s, m_centerTopWidget->m_progressBar);
				m_centerTopWidget->P(100);
				m_centerTopWidget->hideProgress();
			}
		}
	}
}

void MainWindow::DlpPrintLoadSetup()
{
	DLPrintConfig* config = &m_dlprint->config;
	config->normIlluTime.setInt(setupDialog->normIlluSpin->value());
	config->norm_inttersity.setInt(setupDialog->norm_inttersity_spin->value());
	config->overIlluTime = setupDialog->overIlluSpin->value();
	config->overLayer.setInt(setupDialog->overLayerSpin->value());
	config->over_inttersity.setInt(setupDialog->over_inttersity_spin->value());

	config->support_top_height = setupDialog->top_height_spin->value();
	config->support_top_radius = setupDialog->top_radius_spin->value();
	config->support_radius = setupDialog->support_radius_spin->value();
	config->support_bottom_radius = setupDialog->bottom_radius_spin->value();
	config->space.setInt(setupDialog->support_space_spin->value());
	config->angle.setInt(setupDialog->support_angle_spin->value());
	config->leaf_num = setupDialog->leaf_num_spin->value();
	config->model_lift = setupDialog->model_lift_spin->value();

	config->hollow_out = int(setupDialog->hollow_out_box->isChecked());

	if (setupDialog->fill_pattern_combo->currentIndex() == 0)
		config->fill_pattern.value = ipHoneycomb;
	else if (setupDialog->fill_pattern_combo->currentIndex() == 1)
		config->fill_pattern.value = ip3DSupport;

	config->wall_thickness = setupDialog->wall_thickness_spin->value();
	config->fill_density = setupDialog->density_spin->value();

	if (setupDialog->thicknessCombo->currentText() == "0.05mm")
		config->layer_height = 0.05;
	else if (setupDialog->thicknessCombo->currentText() == "0.1mm")
		config->layer_height = 0.1;
	else if (setupDialog->thicknessCombo->currentText() == "0.025mm")
		config->layer_height = 0.025;

	config->raft_layers.setInt(setupDialog->raftSpin->value());
	config->raft_offset.setInt(setupDialog->raftOffsetSpin->value());
	config->arrange_space = setupDialog->arrange_space_spin->value();
	config->threads.setInt(setupDialog->threadSpin->value());
}

void MainWindow::saveView()
{
	QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	QString path = QFileDialog::getSaveFileName(this, QStringLiteral("保存切片文件"), desktop, "histogram file(*.bmp)");
	if (path.size() > 0) {
		QByteArray a = path.toLocal8Bit();
		char* _path = a.data();
		glwidget->saveOneView(_path);
	}
}

void MainWindow::SupportEdit()
{
	if (!m_centerTopWidget->m_supportEditBtn->c_isChecked())
	{
		m_centerTopWidget->CenterButtonPush(SUPPORTEDITBTN);

		if (glwidget->selectID >= 0)
			glwidget->supportEditChange();
		else
		{
			QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("请选中一个已生成支撑的模型。"));
			m_centerTopWidget->CenterButtonPush(SUPPORTEDITBTN);
			return;
		}
	}
	else {
		//-----------退出支撑编辑模式，更新支撑点------------
		m_centerTopWidget->showProgress(SUPPORTEDITBTN);
		m_centerTopWidget->m_progressBar->setValue(10);

		auto p = m_dlprint->treeSupports.find(glwidget->selectID);
		TreeSupport* t = new TreeSupport();

		stl_vertex v;
		for (auto i = treeSupportsExist.begin(); i != treeSupportsExist.end(); ++i) {
			if ((*i).exist) {
				v.x = (*i).p.x;
				v.y = (*i).p.y;
				v.z = (*i).p.z;

				if (p != m_dlprint->treeSupports.end()) {
					if ((*p).second->support_point.size() != 0) {
						for (auto i1 = (*p).second->support_point.begin(); i1 != (*p).second->support_point.end(); ++i1) {
							if (equal_vertex(*i1, v)) {
								t->support_point.push_back(v);
								break;
							}
							if (i1 == (*p).second->support_point.end() - 1)
								t->support_point_face.push_back(v);
						}
					}
					else
						t->support_point_face.push_back(v);
				}
				else
					t->support_point_face.push_back(v);
			}
		}

		t->generate_tree_support(glwidget->find_model(glwidget->selectID),m_dlprint->config.leaf_num,
			m_dlprint->config.threads, m_centerTopWidget->m_progressBar,m_dlprint->config.support_top_height);
		
		if (p != m_dlprint->treeSupports.end()) 
			deleteSupports(glwidget->selectID);
		else
			glwidget->offsetValueChange(glwidget->selectID, 0, 0, m_dlprint->config.model_lift);

		generate_id_support(glwidget->selectID, t, m_centerTopWidget->m_progressBar);
		addSupports(glwidget->selectID, t, m_centerTopWidget->m_progressBar);
		m_centerTopWidget->hideProgress();
		//------------------------------------------------------------

		glwidget->supportEditChange();
		//清空
		treeSupportsExist.clear();
		m_centerTopWidget->CenterButtonPush(SUPPORTEDITBTN);
	}
}

void MainWindow::slice()
{
	m_centerTopWidget->CenterButtonPush(SLICEBTN);

	if (m_model->objects.size() == 0) {
		m_centerTopWidget->CenterButtonPush(SLICEBTN);
		return;
	}

	if (!glwidget->checkConfine()) {
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
			glwidget->saveOneView(_path);
			
			m_centerTopWidget->showProgress(SLICEBTN);
			
			m_centerTopWidget->P(10);
			generate_all_inside_support();
			m_centerTopWidget->P(20);
			m_dlprint->slice(glwidget->return_support_model(), m_centerTopWidget->m_progressBar);
			m_centerTopWidget->P(98);
			m_dlprint->savePNG(e_setting.ZipTempPath.c_str());
			m_centerTopWidget->P(99);
			JlCompress::compressDir(path, e_setting.ZipTempPath.c_str());
			m_centerTopWidget->P(100);

			//清空切片文件夹
			clearDir(e_setting.ZipTempPath.c_str());

			m_centerTopWidget->hideProgress();

			switch (QMessageBox::question(this, QStringLiteral("提示"), QStringLiteral("切片完成，是否预览切片文件？"), QMessageBox::Cancel, QMessageBox::Ok))
			{
			case QMessageBox::Ok:
				showPreviewWidget1(path);
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

void MainWindow::showPreviewWidget1(QString zipPath)
{
	//解压zip包到用户目录下
	clearDir(e_setting.ZipTempPath.c_str());

	QDir file(e_setting.ZipTempPath.c_str());
	if (!file.exists()) {
		if (!file.mkdir(e_setting.ZipTempPath.c_str()))
			exit(2);
	}

	if (extractZipPath(zipPath)) {
		JlCompress::extractDir(zipPath, e_setting.ZipTempPath.c_str());
	
		previewDialog = new PreviewDialog();
		if (previewDialog->readIni(e_setting.ZipTempPath.c_str())) {
			previewDialog->exec();
			delete previewDialog;
		}
	}
	else {
		QString text(QStringLiteral("打开文件"));
		text.append(zipPath);
		text.append(QStringLiteral("失败！"));
		QMessageBox::about(this, QStringLiteral("警告"), text);
	}
}

bool MainWindow::extractZipPath(QString zipPath)
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

void MainWindow::saveModelSupport()
{
	if (m_model->objects.size() > 0) {
		QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
		QString path = QFileDialog::getSaveFileName(this, QStringLiteral("保存切片文件"), desktop, "histogram file(*.stl)");
		if (!path.isEmpty()) {
			TriangleMesh mesh = glwidget->saveSupport();
			m_model->wirteStlBinary(path.toStdString(), mesh);
		}
	}
}

void MainWindow::showAboutDialog()
{
	aboutDialog = new AboutDialog();
	aboutDialog->exec();
	delete aboutDialog;
}

void MainWindow::autoArrange()
{
	double L = double(dlprinter->length / 2);
	double W = double(dlprinter->width / 2);

	BoundingBoxf box;
	box.min.x = -L;
	box.min.y = -W;
	box.max.x = L;
	box.max.y = W;
	box.defined = true;
	//未删除支撑？？？
	Pointfs positions = m_model->arrange_objects(m_dlprint->config.arrange_space, &box);

	glwidget->clearSupportBuffer();

	for (ModelObjectPtrs::const_iterator o = m_model->objects.begin(); o != m_model->objects.end(); ++o) {
	    for (ModelInstancePtrs::const_iterator i = (*o)->instances.begin(); i != (*o)->instances.end(); ++i) {
			if ((*i)->exist) {
				size_t id = m_model->find_id(*i);
				
				//将所有实例在xy方向上重新排列
				Pointf p = positions.back();
				positions.pop_back();
				double x = p.x - (*i)->offset.x;
				double y = p.y - (*i)->offset.y;
				glwidget->offsetValueChange(id, x, y, 0);
				TreeSupport* s1 = NULL;
				if (m_dlprint->chilck_tree_support(id, s1)) {
					s1->support_offset(x, y);
					glwidget->initTreeSupport_id(id, s1, NULL);
				}
			}
	    }
	    (*o)->invalidate_bounding_box();
	}
}

void MainWindow::duplicate(size_t num, bool arrange)
{
	int distance = 0;

	TreeSupport* s1 = NULL;
	bool ret = m_dlprint->chilck_tree_support(glwidget->selectID, s1);

	for (int i = 0; i < num; ++i) {
		ModelInstance* s = m_model->addInstance(glwidget->selectID);
		size_t id = m_model->find_id(s);
		addModelBuffer(id);
		if (ret) {
			TreeSupport* ss = new TreeSupport();
			ss->support_point = s1->support_point;
			ss->support_point_face = s1->support_point_face;
			ss->tree_support_bole = s1->tree_support_bole;
			ss->tree_support_bottom = s1->tree_support_bottom;
			ss->tree_support_branch = s1->tree_support_branch;
			ss->tree_support_leaf = s1->tree_support_leaf;
			ss->tree_support_node = s1->tree_support_node;
			addSupports(id, ss, NULL);
		}

		//if (!arrange) {
		//	distance = distance + 5;
		//	s->offset.x += distance;
		//	s->offset.y += distance;
		//}
	}
	autoArrange();
}


void MainWindow::_duplicate()
{
	if (glwidget->selectID >= 0) {
		bool ret;
		int num = QInputDialog::getInt(this, QStringLiteral("复制"), QStringLiteral("请输入复制模型的个数："), 1, 1, 50, 1, &ret);
		if (ret) {
			//switch (QMessageBox::question(this, QStringLiteral("提示"), QStringLiteral("复制后进行自动排列？"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes))
			//{
			//case QMessageBox::Yes:
				duplicate(num,true);
			//	break;
			//case QMessageBox::No:
			//	duplicate(num,false);
			//	break;
			//}
		}
	}
	else
	{
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("请选中一个模型进行复制。"));
		return;
	}
}

void MainWindow::addModelBuffer(size_t id)
{
	ModelInstance* instance = m_model->find_instance(id);
	glwidget->addModelBuffer(instance);
	instance->setExist();
}

void MainWindow::deleteModelBuffer(size_t id)
{
	glwidget->delModelBuffer(id);
	m_model->find_instance(id)->setExist(false);
}

void MainWindow::addSupports(size_t id, TreeSupport* s, QProgressBar* progress)
{
	//支撑数值存入treeSupports
	m_dlprint->insertSupports(id, s);
	//渲染支撑
	glwidget->initTreeSupport_id(id, s, progress);
}

void MainWindow::deleteSupports(size_t id)
{
	//删除treeSupports中的指定支撑
	m_dlprint->delete_tree_support(id);
	//删除支撑缓存区
	glwidget->delSupportBuffer(id);
}

//支撑编辑可以100个点为一个区间，每次删减操作最多只对100个点进行初始化，区间点过少会消耗过多内存??????
void MainWindow::addOneSupport(Pointf3 p)
{
	Pointf3Exist temp = { true,p };
	treeSupportsExist.push_back(temp);

	size_t id = treeSupportsExist.size();
	auto i = treeSupportsExist.begin() + id;
	(*i).exist = true;
	glwidget->initSupportEditBuffer(id / 100);
	glwidget->update();
}

void MainWindow::deleteOneSupport(size_t id)
{
	auto p = treeSupportsExist.begin() + id;
	(*p).exist = false;
	glwidget->initSupportEditBuffer(id / 100);
	glwidget->update();
}

void MainWindow::initDlprinter()
{
	if (QFile(e_setting.DlprinterFile.c_str()).exists()) {
		QSettings* readini = new QSettings(e_setting.DlprinterFile.c_str(), QSettings::IniFormat);
		//读取打印设置
		QString name = readini->value("/dlprinter/name").toString();
		delete readini;
		if (name == "S288")
			dlprinter = new DLPrinter(S288);
		else if (name == "S250")
			dlprinter = new DLPrinter(S250);
		else
			dlprinter = new DLPrinter(S288);
	}
	else
		dlprinter = new DLPrinter(S288);
	dlprinter->readPrinter();
}

void MainWindow::initSetupDialog()
{
	setupDialog = new SetupDialog(this);
	DlpPrintLoadSetup();
	delete setupDialog;
}

void MainWindow::dlprinterChange(QString name)
{
	QSettings* writeini = new QSettings(e_setting.DlprinterFile.c_str(), QSettings::IniFormat);
	writeini->clear();
	
	if (name == "S288") {
		writeini->setValue("/dlprinter/name", "S288");
		dlprinter->printer = S288;
		dlprinter->readPrinter();
	}
	else if (name == "S250") {
		writeini->setValue("/dlprinter/name", "S250");
		dlprinter->printer = S250;
		dlprinter->readPrinter();
	}
	delete writeini;
	
	glwidget->dlprinterChange();
	glwidget->updateConfine();
	glwidget->update();
}

void MainWindow::showSetupDialog()
{
	m_centerTopWidget->CenterButtonPush(SETUPBTN);

	if (m_centerTopWidget->m_setupBtn->c_isChecked()) {
		setupDialog = new SetupDialog(this);
		setupDialog->exec();
		delete setupDialog;
	}

	m_centerTopWidget->CenterButtonPush(SETUPBTN);
}

void MainWindow::showOffsetWidget()
{
	m_centerTopWidget->CenterButtonPush(OFFSETBTN);

	if (m_centerTopWidget->m_offsetBtn->c_isChecked() && glwidget->selectID >= 0)
		setOffsetValue(m_model->find_instance(glwidget->selectID));
}

void MainWindow::showRotateWidget()
{
	m_centerTopWidget->CenterButtonPush(ROTATEBTN);
}

void MainWindow::showScaleWidget()
{
	m_centerTopWidget->CenterButtonPush(SCALEBTN);

	if (m_centerTopWidget->m_scaleBtn->c_isChecked() && glwidget->selectID >= 0) {
		ModelInstance* i = m_model->find_instance(glwidget->selectID);
		setScaleValue(i);
		setSizeValue(i);
	}
}

//----------------------------------------------------------------------------

void MainWindow::generate_id_support(size_t id, TreeSupport*& s, QProgressBar* progress)
{
	ModelInstance* instance = m_model->find_instance(id);
	ModelObject* object = instance->get_object();
	TriangleMesh mesh(object->volumes[0]->mesh);
	instance->transform_mesh(&mesh);
	progress->setValue(15);
	m_dlprint->generate_support(id, s, &mesh, progress);
}

void MainWindow::generate_all_inside_support()
{
	m_dlprint->delete_all_inside_support();
	if (m_dlprint->config.hollow_out && m_dlprint->config.fill_pattern == ip3DSupport) {
		for (auto o = m_model->objects.begin(); o != m_model->objects.end(); ++o) {
			size_t a = std::distance(m_model->objects.begin(), o);
			for (auto i = (*o)->instances.begin(); i != (*o)->instances.end(); ++i) {
				if ((*i)->exist) {
					size_t b = std::distance((*o)->instances.begin(), i);
					size_t id = a * InstanceNum + b;
					TriangleMesh mesh((*o)->volumes[0]->mesh);
					(*i)->transform_mesh(&mesh);
					m_dlprint->generate_inside_support(id, &mesh);
				}
			}
		}
	}
}

void MainWindow::slot_offsetChange()
{
	if (!m_centerTopWidget->m_offsetWidget->isHidden())
		setOffsetValue(m_model->find_instance(glwidget->selectID));
}

void MainWindow::slot_scaleChange()
{
	if (!m_centerTopWidget->m_scaleWidget->isHidden())
		setScaleValue(m_model->find_instance(glwidget->selectID));
}

void MainWindow::slot_rotateChange()
{
	if (!m_centerTopWidget->m_rotateWidget->isHidden())
		setRotateValue(m_model->find_instance(glwidget->selectID));
}

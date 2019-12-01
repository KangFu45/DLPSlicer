#include "mainwindow.h"
#include "qmenubar.h"
#include "qevent.h"
#include "qdebug.h"
#include "qregion.h"
#include "qsettings.h"
#include "quazip/JlCompress.h"
#include <qplaintextedit.h>
#include <QInputDialog>
#include "qtimer.h"
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
	interface1 = new Interface(dlprinter);
	initUndo();
	initAction();
	initSetupDialog();
	initCentralWindow();//要先于中央界面上button初始化
	m_centerTopWidget = std::unique_ptr<CenterTopWidget>(new CenterTopWidget(this));

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
		size_t id = interface1->load_model(file, format);
		m_centerTopWidget->P(60);
		glwidget->save_valume(id);
		m_centerTopWidget->P(80);
		MainUndoStack->push(new AddModelCommand(id, this));
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
		MainUndoStack->push(new DeleteModelCommand(glwidget->selectID, this));

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
		size_t id = interface1->load_model(file, format);
		m_centerTopWidget->P(60);
		glwidget->save_valume(id);
		m_centerTopWidget->P(80);
		MainUndoStack->push(new AddModelCommand(id, this));
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
	QSize size = event->size();
	m_centerTopWidget->resize(size);
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

	QAction *undoAction = MainUndoGroup->createUndoAction(this, QStringLiteral("撤销"));
	undoAction->setIcon(QIcon(":/icon/images/undo.png"));
	undoAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z));

	QAction *redoAction = MainUndoGroup->createRedoAction(this, QStringLiteral("重做"));
	redoAction->setIcon(QIcon(":/icon/images/redo.png"));
	redoAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Y));

	editMenu->addAction(undoAction);
	editMenu->addAction(redoAction);

	QAction* clearUndoAction = new QAction(QStringLiteral("清空撤销栈"));
	editMenu->addAction(clearUndoAction);
	connect(clearUndoAction, SIGNAL(triggered()), this, SLOT(clearUndoStack()));

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

	QAction* updateAct = new QAction(QStringLiteral("检查更新"), this);
	helpMenu->addAction(updateAct);
	connect(updateAct, SIGNAL(triggered()), this, SLOT(checkUpdate()));

	QAction* messageAct = new QAction(QStringLiteral("版本信息"), this);
	helpMenu->addAction(messageAct);
	connect(messageAct, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
}

void MainWindow::initCentralWindow()
{
	glwidget = new GlWidget(this, dlprinter,interface1->model,interface1->dlprint);
	setCentralWidget(glwidget);
}

void MainWindow::ZPosZero()
{
	if (glwidget->selectID >= 0) {
		ModelInstance* instance = IdToInstance();
		TriangleMesh mesh = instance->get_object()->volumes[0]->mesh;
		instance->transform_mesh(&mesh);
		BoundingBoxf3 box = mesh.bounding_box();

		TreeSupport* s1 = NULL;
		if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
			MainUndoStack->beginMacro(tr(""));
			MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
			MainUndoStack->push(new offsetValueChangeCommand(glwidget->selectID, 0, 0, -box.min.z, this));
			MainUndoStack->endMacro();
		}
		else
			MainUndoStack->push(new offsetValueChangeCommand(glwidget->selectID, 0, 0, -box.min.z, this));
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
	if (x != 0)
		m_centerTopWidget->x_offset_spin->setValue(m_centerTopWidget->x_offset_spin->value() + x);
	if (y != 0)
		m_centerTopWidget->y_offset_spin->setValue(m_centerTopWidget->y_offset_spin->value() + y);
	if (z != 0)
		m_centerTopWidget->z_offset_spin->setValue(m_centerTopWidget->z_offset_spin->value() + z);
}

void MainWindow::xoffsetValueChange(double value)
{
	if (glwidget->selectID >= 0) {
		ModelInstance* instance = IdToInstance();
		if (instance->offset.x != value) {
			double x = value - instance->offset.x;
			if (fabs(x) >= 0.01) {//防止细微误差
				TreeSupport* s1 = NULL;
				if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
					MainUndoStack->beginMacro(tr(""));
					MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
					MainUndoStack->push(new offsetValueChangeCommand(glwidget->selectID, x, 0, 0, this));
					MainUndoStack->endMacro();
				}
				else
					MainUndoStack->push(new offsetValueChangeCommand(glwidget->selectID, x, 0, 0, this));
			}
		}
	}
}

void MainWindow::yoffsetValueChange(double value)
{
	if(glwidget->selectID >= 0) {
		ModelInstance* instance = IdToInstance();
		if (instance->offset.y != value) {
			double y = value - instance->offset.y;
			if (fabs(y) >= 0.01) {
				TreeSupport* s1 = NULL;
				if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
					MainUndoStack->beginMacro(tr(""));
					MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
					MainUndoStack->push(new offsetValueChangeCommand(glwidget->selectID, y, 0, 0, this));
					MainUndoStack->endMacro();
				}
				else
					MainUndoStack->push(new offsetValueChangeCommand(glwidget->selectID, 0, y, 0, this));
			}
		}
	}
}

void MainWindow::zoffsetValueChange(double value)
{
	if(glwidget->selectID >= 0) {
		ModelInstance* instance = IdToInstance();
		if (instance->z_translation != value) {
			double z = value - instance->z_translation;
			if (fabs(z) >= 0.01) {
				TreeSupport* s1 = NULL;
				if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
					MainUndoStack->beginMacro(tr(""));
					MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
					MainUndoStack->push(new offsetValueChangeCommand(glwidget->selectID, z, 0, 0, this));
					MainUndoStack->endMacro();
				}
				else
					MainUndoStack->push(new offsetValueChangeCommand(glwidget->selectID, 0, 0, z, this));
			}
		}
	}
}

void MainWindow::AddRotateValue(double angle, int x, int y, int z)
{
	//只支持一个方向的角度值，且比例值为1
	TreeSupport* s1 = NULL;
	if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
		MainUndoStack->beginMacro(tr("rotate delete supports"));
		MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
		MainUndoStack->push(new rotateValueChangeCommand(glwidget->selectID, angle, x, y, z, this));
		MainUndoStack->endMacro();
	}
	else
		MainUndoStack->push(new rotateValueChangeCommand(glwidget->selectID, angle, x, y, z, this));
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
		
			TreeSupport* s1 = NULL;
			if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
				MainUndoStack->beginMacro(tr(""));
				MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
				MainUndoStack->push(new rotateValueChangeCommand(glwidget->selectID, x, 1, 0, 0, this));
				MainUndoStack->endMacro();
			}
			else
				MainUndoStack->push(new rotateValueChangeCommand(glwidget->selectID, x, 1, 0, 0, this));

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
			TreeSupport* s1 = NULL;
			if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
				MainUndoStack->beginMacro(tr(""));
				MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
				MainUndoStack->push(new rotateValueChangeCommand(glwidget->selectID, y, 0, 1, 0, this));
				MainUndoStack->endMacro();
			}
			else
				MainUndoStack->push(new rotateValueChangeCommand(glwidget->selectID, y, 0, 1, 0, this));
	
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
			TreeSupport* s1 = NULL;
			if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
				MainUndoStack->beginMacro(tr(""));
				MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
				MainUndoStack->push(new rotateValueChangeCommand(glwidget->selectID, z, 0, 0, 1, this));
				MainUndoStack->endMacro();
			}
			else
				MainUndoStack->push(new rotateValueChangeCommand(glwidget->selectID, z, 0, 0, 1, this));

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
		ModelInstance* instance = IdToInstance();
		double xScale = value / 100;
		double x = xScale - instance->scaling_vector.x;
		double y = instance->scaling_vector.y*(x / instance->scaling_vector.x);
		double z = instance->scaling_vector.z*(x / instance->scaling_vector.x);
		if (fabs(x) > 0.01) {
			if (m_centerTopWidget->unify_scale->checkState() == Qt::Unchecked) {
				TreeSupport* s1 = NULL;
				if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
					MainUndoStack->beginMacro(tr("scale delete supports"));
					MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, x, 0, 0, this));
					MainUndoStack->endMacro();
				}
				else
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, x, 0, 0, this));

			}
			else {
				TreeSupport* s1 = NULL;
				if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
					MainUndoStack->beginMacro(tr("scale delete supports"));
					MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, x, y, z, this));
					MainUndoStack->endMacro();
				}
				else
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, x, y, z, this));

			}
		}
	}
}

void MainWindow::yScaleValueChange(double value)
{
	if (glwidget->selectID >= 0) {
		ModelInstance* instance = IdToInstance();
		double yScale = value / 100;
		double y = yScale - instance->scaling_vector.y;
		double x = instance->scaling_vector.x*(y / instance->scaling_vector.y);
		double z = instance->scaling_vector.z*(y / instance->scaling_vector.y);
		if (fabs(y) > 0.01) {
			if (m_centerTopWidget->unify_scale->checkState() == Qt::Unchecked) {
				TreeSupport* s1 = NULL;
				if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
					MainUndoStack->beginMacro(tr(""));
					MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, 0, y, 0, this));
					MainUndoStack->endMacro();
				}
				else
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, 0, y, 0, this));
			}
			else {
				TreeSupport* s1 = NULL;
				if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
					MainUndoStack->beginMacro(tr("scale delete supports"));
					MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, x, y, z, this));
					MainUndoStack->endMacro();
				}
				else
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, x, y, z, this));
			}
		}
	}
}

void MainWindow::zScaleValueChange(double value)
{
	if (glwidget->selectID >= 0) {
		ModelInstance* instance = IdToInstance();
		double zScale = value / 100;
		double z = zScale - instance->scaling_vector.z;
		double x = instance->scaling_vector.x*(z / instance->scaling_vector.z);
		double y = instance->scaling_vector.y*(z / instance->scaling_vector.z);
		if (fabs(z) > 0.01) {
			if (m_centerTopWidget->unify_scale->checkState() == Qt::Unchecked) {
				TreeSupport* s1 = NULL;
				if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
					MainUndoStack->beginMacro(tr(""));
					MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, 0, 0, z, this));
					MainUndoStack->endMacro();
				}
				else
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, 0, 0, z, this));
			}
			else {
				TreeSupport* s1 = NULL;
				if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
					MainUndoStack->beginMacro(tr("scale delete supports"));
					MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, x, y, z, this));
					MainUndoStack->endMacro();
				}
				else
					MainUndoStack->push(new scaleValueChangeCommand(glwidget->selectID, x, y, z, this));
			}
		}
	}
}

void MainWindow::defaultView()
{
	glwidget->ChangeView(glwidget->DEFAULT);
}

void MainWindow::overlookView()
{
	glwidget->ChangeView(glwidget->OVERLOOK);
}

void MainWindow::leftView()
{
	glwidget->ChangeView(glwidget->LEFT);
}

void MainWindow::rightView()
{
	glwidget->ChangeView(glwidget->RIGHT);
}

void MainWindow::frontView()
{
	glwidget->ChangeView(glwidget->FRONT);
}

void MainWindow::behindView()
{
	glwidget->ChangeView(glwidget->BEHIND);
}

void MainWindow::newJob()//新建项目
{
	interface1->clear_model();
	interface1->delete_all_support();
	glwidget->clearModelBuffer();
	glwidget->clear_volumes();
	glwidget->clearSupportBuffer();
	clearUndoStack();
}

void MainWindow::deleteSupport()
{
	TreeSupport* s1 = NULL;
	if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
		MainUndoStack->push(new offsetValueChangeCommand(glwidget->selectID, 0, 0, -interface1->dlprint->config.model_lift, this));
		MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
	}
}

void MainWindow::deleteAllSupport()
{
	for (ModelObjectPtrs::const_iterator o = interface1->model->objects.begin(); o != interface1->model->objects.end(); ++o) {
		for (ModelInstancePtrs::const_iterator i = (*o)->instances.begin(); i != (*o)->instances.end(); ++i) {
			if ((*i)->exist) {
				size_t id = interface1->find_id(*i);

				TreeSupport* s1 = NULL;
				if (interface1->dlprint->chilck_tree_support(id, s1)) {
					MainUndoStack->push(new offsetValueChangeCommand(id, 0, 0, -interface1->dlprint->config.model_lift, this));
					MainUndoStack->push(new deleteSupportsCommand(id, s1, this));
				}
			}
		}
	}
}

void MainWindow::modelSelect()
{
	ModelInstance* instance = IdToInstance();
	setOffsetValue(instance);
	setRotateValue(instance);
	setScaleValue(instance);
	setSizeValue(instance);
}


ModelInstance* MainWindow::IdToInstance()
{
	return interface1->find_instance(glwidget->selectID);
}


void MainWindow::generateSupport()
{
	m_centerTopWidget->CenterButtonPush(SUPPORTBTN);

	if (glwidget->selectID >= 0) {
		m_centerTopWidget->showProgress(SUPPORTBTN);
		m_centerTopWidget->P(10);
		TreeSupport* s = new TreeSupport();
		TreeSupport* s1 = new TreeSupport();
		if (interface1->dlprint->chilck_tree_support(glwidget->selectID, s1)) {
			interface1->generate_id_support(glwidget->selectID, s, m_centerTopWidget->progress);
			MainUndoStack->beginMacro(tr(""));
			MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID, s1, this));
			MainUndoStack->push(new addSupportsCommand(glwidget->selectID, s, this, m_centerTopWidget->progress));
			MainUndoStack->endMacro();
		}
		else {
			MainUndoStack->push(new offsetValueChangeCommand(glwidget->selectID, 0, 0, interface1->dlprint->config.model_lift, this));
			interface1->generate_id_support(glwidget->selectID, s, m_centerTopWidget->progress);
			MainUndoStack->push(new addSupportsCommand(glwidget->selectID, s, this, m_centerTopWidget->progress));
		}
		m_centerTopWidget->P(100);
		m_centerTopWidget->hideProgress();

	}
	else
	{
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("请选中一个模型。"));
	}

	m_centerTopWidget->CenterButtonPush(SUPPORTBTN);
}

void MainWindow::generateAllSupport()
{
	for (ModelObjectPtrs::const_iterator o = interface1->model->objects.begin(); o != interface1->model->objects.end(); ++o) {
		for (ModelInstancePtrs::const_iterator i = (*o)->instances.begin(); i != (*o)->instances.end(); ++i) {
			if ((*i)->exist) {
				size_t id = interface1->find_id(*i);

				m_centerTopWidget->showProgress(SUPPORTBTN);
				m_centerTopWidget->P(10);
				TreeSupport* s = new TreeSupport();
				TreeSupport* s1 = new TreeSupport();
				if (interface1->dlprint->chilck_tree_support(id, s1)) {
					interface1->generate_id_support(id, s, m_centerTopWidget->progress);
					MainUndoStack->beginMacro(tr(""));
					MainUndoStack->push(new deleteSupportsCommand(id, s1, this));
					MainUndoStack->push(new addSupportsCommand(id, s, this, m_centerTopWidget->progress));
					MainUndoStack->endMacro();
				}
				else {
					MainUndoStack->push(new offsetValueChangeCommand(id, 0, 0, interface1->dlprint->config.model_lift, this));
					interface1->generate_id_support(id, s, m_centerTopWidget->progress);
					MainUndoStack->push(new addSupportsCommand(id, s, this, m_centerTopWidget->progress));
				}
				m_centerTopWidget->P(100);
				m_centerTopWidget->hideProgress();
			}
		}
	}
}

void MainWindow::DlpPrintLoadSetup()
{
	DLPrintConfig* config = &interface1->dlprint->config;
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

		if (glwidget->selectID >= 0) {

			//生成支撑编辑“撤销重做”栈并激活
			SupportEditUndoStack = new QUndoStack(this);
			MainUndoGroup->addStack(SupportEditUndoStack);
			MainUndoGroup->setActiveStack(SupportEditUndoStack);

			glwidget->supportEditChange();
		}
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
		m_centerTopWidget->progress->setValue(10);

		auto p = interface1->dlprint->treeSupports.find(glwidget->selectID);
		TreeSupport* t = new TreeSupport();

		stl_vertex v;
		for (auto i = treeSupportsExist.begin(); i != treeSupportsExist.end(); ++i) {
			if ((*i).exist) {
				v.x = (*i).p.x;
				v.y = (*i).p.y;
				v.z = (*i).p.z;

				if (p != interface1->dlprint->treeSupports.end()) {
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

		t->generate_tree_support(glwidget->find_model(glwidget->selectID),interface1->dlprint->config.leaf_num,
			interface1->dlprint->config.threads, m_centerTopWidget->progress,interface1->dlprint->config.support_top_height);
		
		if (p != interface1->dlprint->treeSupports.end()) {
			MainUndoStack->beginMacro(tr(""));
			MainUndoStack->push(new deleteSupportsCommand(glwidget->selectID,(*p).second,this));
			MainUndoStack->push(new addSupportsCommand(glwidget->selectID, t, this, m_centerTopWidget->progress));
			MainUndoStack->endMacro();
		}
		else {
			MainUndoStack->push(new addSupportsCommand(glwidget->selectID, t, this, m_centerTopWidget->progress));
		}
		m_centerTopWidget->hideProgress();
		//------------------------------------------------------------

		//删除支撑编辑“撤销重做”栈,激活主栈
		delete SupportEditUndoStack;
		MainUndoGroup->setActiveStack(MainUndoStack);

		glwidget->supportEditChange();
		//清空
		treeSupportsExist.clear();

		m_centerTopWidget->CenterButtonPush(SUPPORTEDITBTN);
	}
}

void MainWindow::slice()
{
	m_centerTopWidget->CenterButtonPush(SLICEBTN);
	//-------------------------------------

	if (interface1->model->objects.size() == 0) {
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
			interface1->generate_all_inside_support();
			m_centerTopWidget->P(20);
			interface1->dlprint->slice(glwidget->return_support_model(), m_centerTopWidget->progress);
			m_centerTopWidget->P(98);
			interface1->dlprint->savePNG(e_setting.ZipTempPath.c_str());
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
	if (interface1->model->objects.size() > 0) {
		QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
		QString path = QFileDialog::getSaveFileName(this, QStringLiteral("保存切片文件"), desktop, "histogram file(*.stl)");
		if (!path.isEmpty()) {
			TriangleMesh mesh = glwidget->saveSupport();
			interface1->wirteStlBinary(path.toStdString(), mesh);
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
	//未删除支撑？？？
	Pointfs positions = interface1->autoArrange(interface1->dlprint->config.arrange_space);

	glwidget->clearSupportBuffer();

	MainUndoStack->beginMacro(tr(""));
	for (ModelObjectPtrs::const_iterator o = interface1->model->objects.begin(); o != interface1->model->objects.end(); ++o) {
	    for (ModelInstancePtrs::const_iterator i = (*o)->instances.begin(); i != (*o)->instances.end(); ++i) {
			if ((*i)->exist) {
				size_t id = interface1->find_id(*i);
				
				//将所有实例在xy方向上重新排列
				Pointf p = positions.back();
				positions.pop_back();
				double x = p.x - (*i)->offset.x;
				double y = p.y - (*i)->offset.y;
				MainUndoStack->push(new offsetValueChangeCommand(id, x, y, 0, this));
				TreeSupport* s1 = NULL;
				if (interface1->dlprint->chilck_tree_support(id, s1)) {
					s1->support_offset(x, y);
					glwidget->initTreeSupport_id(id, s1, NULL);
				}
			}
	    }
	    (*o)->invalidate_bounding_box();
	}
	MainUndoStack->endMacro();
}

void MainWindow::duplicate(size_t num, bool arrange)
{
	MainUndoStack->beginMacro(tr(""));
	int distance = 0;

	TreeSupport* s1 = NULL;
	bool ret = interface1->dlprint->chilck_tree_support(glwidget->selectID, s1);

	for (int i = 0; i < num; ++i) {
		ModelInstance* s = interface1->addInstance(glwidget->selectID);
		size_t id = interface1->find_id(s);
		MainUndoStack->push(new AddModelCommand(id, this));
		if (ret) {
			TreeSupport* ss = new TreeSupport();
			ss->support_point = s1->support_point;
			ss->support_point_face = s1->support_point_face;
			ss->tree_support_bole = s1->tree_support_bole;
			ss->tree_support_bottom = s1->tree_support_bottom;
			ss->tree_support_branch = s1->tree_support_branch;
			ss->tree_support_leaf = s1->tree_support_leaf;
			ss->tree_support_node = s1->tree_support_node;
			MainUndoStack->push(new addSupportsCommand(id, ss, this, NULL));
		}

		//if (!arrange) {
		//	distance = distance + 5;
		//	s->offset.x += distance;
		//	s->offset.y += distance;
		//}
	}
	//if(arrange)
		autoArrange();
	MainUndoStack->endMacro();
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

void MainWindow::initUndo()
{
	MainUndoGroup = new QUndoGroup(this);
	MainUndoStack = new QUndoStack(this);
	MainUndoGroup->addStack(MainUndoStack);
	MainUndoGroup->setActiveStack(MainUndoStack);
}

void MainWindow::addModelBuffer(size_t id)
{
	ModelInstance* instance = interface1->find_instance(id);
	glwidget->addModelBuffer(instance);
	instance->setExist();
}

void MainWindow::deleteModelBuffer(size_t id)
{
	glwidget->delModelBuffer(id);
	interface1->find_instance(id)->setExist(false);
}

//传入移动的距离数值
void MainWindow::offsetValueChange(size_t id,double x, double y, double z)
{
	ModelInstance* i = interface1->find_instance(id);
	i->offset.x += x;
	i->offset.y += y;
	i->z_translation += z;

	if (glwidget->selectID == id)
		setOffsetValue(i);

	i->update_attribute();
	glwidget->updateConfine();
}

//传入缩放的比例值
void MainWindow::scaleValueChange(size_t id, double x, double y, double z)
{
	ModelInstance* i = interface1->find_instance(id);
	i->scaling_vector.x += x;
	i->scaling_vector.y += y;
	i->scaling_vector.z += z;

	if (glwidget->selectID == id) {
		setScaleValue(i);
		setSizeValue(i);
	}

	i->update_attribute();
	glwidget->updateConfine();
}

//传入旋转的角度的数值
void MainWindow::rotateValueChange(size_t id, double angle, int x, int y, int z)
{
	ModelInstance* i = interface1->find_instance(id);
	QMatrix4x4 m;
	m.rotate(angle, x, y, z);
	i->rotation_M = m*i->rotation_M;
	
	i->update_attribute();
	glwidget->updateConfine();
}

void MainWindow::addSupports(size_t id, TreeSupport* s, QProgressBar* progress)
{
	//支撑数值存入treeSupports
	interface1->dlprint->insertSupports(id, s);
	//渲染支撑
	glwidget->initTreeSupport_id(id, s, progress);
}

void MainWindow::deleteSupports(size_t id)
{
	//删除treeSupports中的指定支撑
	interface1->dlprint->delete_tree_support(id);
	//删除支撑缓存区
	glwidget->delSupportBuffer(id);
}

void MainWindow::clearUndoStack()
{
	MainUndoGroup->activeStack()->clear();
}

//支撑编辑可以100个点为一个区间，每次删减操作最多只对100个点进行初始化，区间点过少会消耗过多内存??????
void MainWindow::addOneSupport(Pointf3 p)
{
	Pointf3Exist temp = { true,p };
	treeSupportsExist.push_back(temp);
	SupportEditUndoStack->push(new addOneSupportCommand(treeSupportsExist.size(), this));
}

void MainWindow::deleteOneSupport(size_t id)
{
	SupportEditUndoStack->push(new deleteOneSupportCommand(id, this));
}

void MainWindow::addOneSupportUndo(size_t id)
{
	auto p = treeSupportsExist.begin() + id;
	(*p).exist = true;
	glwidget->initSupportEditBuffer(id / 100);
	glwidget->update();
}

void MainWindow::deleteOneSupportUndo(size_t id)
{
	auto p = treeSupportsExist.begin() + id;
	(*p).exist = false;
	glwidget->initSupportEditBuffer(id / 100);
	glwidget->update();
}

void MainWindow::setPersperctive()
{
	glwidget->setPresprective();
}

void MainWindow::setOrthogonality()
{
	glwidget->setOrthogonality();
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


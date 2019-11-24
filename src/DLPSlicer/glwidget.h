#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <gl\GL.h>
#include <gl\GLU.h>
#include "Model.hpp"
#include <QEvent>
#include "mainwindow.h"
#include "DLPrint.h"

class MainWindow;
class DLPrinter;

//日期：2017
//功能：模型缓冲区结构体。
//属性1：模型或支撑的编号
//属性2：三角面片的数量
//属性3：三角面片的数组
//属性4：OpenGL缓冲区
typedef struct {
	int id;
	int size;
	GLfloat* stl;
	QOpenGLBuffer* buffer;
}ModelBuffer;

typedef ModelBuffer SupportBuffer;

//日期：2018.5.29
//功能：用来保存每个modelObjectd的渲染数据
//属性1：三角面的个数
//属性2：模型数据的数组指针，3*float（一个点）+3*floart（点的法向量）
typedef struct {
	int size;
	GLfloat* stl;
}stl_render;

//作者：付康
//日期：2017
//功能：渲染窗口
class GlWidget : public  QOpenGLWidget, protected QOpenGLFunctions
{
public:
	GlWidget(MainWindow* parent, DLPrinter* _dlprinter, Model* _model, DLPrint* _dlprint);
	~GlWidget() {};

	int selectID;								//选中模型的ID ,等于-1为未选中
	int translationID;							//选中的转换动作标签ID，等于-1为未选中


	//各个方向视图的枚举
	enum VIEW
	{
		DEFAULT,
		OVERLOOK,
		LEFT,
		RIGHT,
		FRONT,
		BEHIND
	};

	void updateTranslationID();

	void setPresprective();

	void setOrthogonality();

	//日期：2018.4.10
	//功能：查找模型。
	//参数1：模型id
	//返回：模型
	TriangleMesh find_model(size_t id);

	//日期：2018.4.10
	//功能：初始化支撑编辑渲染缓冲区。
	void initSupportEditBuffer(size_t i);

	void initSupportEditBufferAll();

	//日期：2017
	//功能：视图改变。
	//参数1：需要改变视图的枚举值
	void ChangeView(int view);

	//日期：2017
	//功能：添加模型缓冲区。
	//参数1：新加入模型实例的模型实例
	void addModelBuffer(ModelInstance* instance);

	//日期：2017
	//功能：保存视图（截图）。
	//参数1：保存路径
	void saveOneView(char* _name);

	//日期：2017
	//功能：支撑编辑状态改变。
	void supportEditChange();

	//日期：2017
	//功能：添加一根支撑。
	void addOneSupport();

	//日期：2017
	//功能：更新边界信息。
	void updateConfine();

	//日期：2017
	//功能：检查边界。
	bool checkConfine();

	//日期：2017
	//功能：保存支撑。
	//返回：三角面片实体
	TriangleMesh saveSupport();

	//日期：2017
	//功能：删除一个模型缓冲区。
	//参数1：选定的模型id
	void delModelBuffer(size_t id);

	//日期：2017
	//功能：删除一个支撑缓冲区。
	//参数1：选定的支撑id
	void delSupportBuffer(size_t id);

	//日期：2017
	//功能：清理模型缓冲区。
	void clearModelBuffer();

	//日期：2017
	//功能：清理支撑缓冲区。
	void clearSupportBuffer();

	//日期：2018.4.9
	//功能：初始化选中模型的树状支撑。
	//参数1：生成支撑模型的id即生成支撑的id
	//参数2：支撑数据
	void initTreeSupport_id(size_t id,TreeSupport* s,QProgressBar* progress);

	//日期：2018.4.12
	//功能：返回支撑模型。
	//返回：支撑模型
	std::vector<TriangleMesh> return_support_model();

	//日期：2018.5.29
	//功能：保存每个ModelObject的valume（只有一个原始实体）
	//参数1：添加实例的id
	void save_valume(size_t id);

	//日期：2018.5.29
	//功能：清空渲染数据
	void clear_volumes();

	//日期：2018.10.10
	//功能：打印机改变
	void dlprinterChange();

protected:
	void resizeGL(int w, int h) override;
	void paintGL() override;
	void initializeGL() override;

protected:
	//****************************
	//日期：2017
	//功能：重写事件函数。
	void wheelEvent(QWheelEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	//*************************************
private:
	//日期：2017
	//功能：一根支撑清零。
	void zeroOneSupport();

private:
	Model* model;								//模型
	DLPrint* dlprint;							//dlp打印

	//投影方式
	enum VIEWPORT
	{
		PRESPRECTIVE = 10,
		ORTHOGONALITY
	};

	int viewport;

	bool supportEdit;												//支撑编辑判断

	MainWindow* mainWindow;											//父窗口

	//渲染模型
	std::vector<ModelBuffer*> modelBuffers;							//模型渲染缓冲区

	//每个模型对象的渲染数据
	std::map<size_t, stl_render> volumes;

	//日期：2017
	//功能：画模型。
	void drawModel();

	std::vector<SupportBuffer*> treeSupportBuffers;					//树状支撑缓冲区

	//日期：2018.4.9
	//功能：渲染树状支撑
	void bindTreeSupport();

	//渲染手动支撑
	Pointf3 oneSupport;												//手动支撑点
	SupportBuffer* oneBallBuffer;									//手动支撑的缓冲区
	std::vector<SupportBuffer*> supportEditBuffer;					//支撑编辑时渲染缓冲区(100个支撑点为一个渲染缓存区）


	//选中模型的支撑区域---暂时只提供球形区域选择
	std::vector<ModelBuffer*> region_box;


	//日期：2018.4.10
	//功能：渲染支撑编辑的点。
	void bindSupportEditBuffer();

	//日期：2017
	//功能：初始化手动支撑。
	void initOneSupport();

	//日期：2017
	//功能：画手动支撑。
	void bindOneSupport();

	void initModelMatrix();

	//渲染平台
	QVector<GLfloat> platform;										//渲染平台的点
	QOpenGLBuffer platform_vbo;										//平台顶点缓存对象
	unsigned short lines_num{ 0 };									//平台的线段的数量

	//日期：2017
	//功能：初始化平台。
	void initPlatform();

	//日期：2017
	//功能：画平台。
	void bindPlatform();

	QOpenGLShaderProgram *m_program;								//OpenGL着色器程序
	int m_projMatrixLoc;
	int m_viewMatrixLoc;

	int m_rotateMatrixLoc;
	int m_scaleMatrixLoc;
	int m_translateMatrixLoc;

	int m_lightPosLoc;
	int func;

	QMatrix4x4 m_proj;												//投影矩阵
	QMatrix4x4 m_camera;											//视图矩阵
	QMatrix4x4 m_world;												//世界坐标矩阵

	void setDefaultView();

	//视图旋转，平移，缩放
	GLfloat _scale;
	GLfloat _verticalAngle, _horizonAngle;							//视图旋转的水平，垂直值
	QVector3D eye;													//照相机的位置
	QVector3D center;												//世界坐标平移的矢量
	int xLastPos, yLastPos;											//鼠标的坐标值

	//日期：2017
	//功能：设置照相机的位置。
	void setEye();

	//前
	bool front;	
	QVector<GLfloat> frontVector;
	QOpenGLBuffer front_vbo;

	//后
	bool back;
	QVector<GLfloat> backVector;
	QOpenGLBuffer back_vbo;

	//左
	bool left;
	QVector<GLfloat> leftVector;
	QOpenGLBuffer left_vbo;

	//右
	bool right;
	QVector<GLfloat> rightVector;
	QOpenGLBuffer right_vbo;

	//上
	bool up;
	QVector<GLfloat> upVector;
	QOpenGLBuffer up_vbo;

	//下
	bool down;
	QVector<GLfloat> downVector;
	QOpenGLBuffer down_vbo;

	//日期：2017
	//功能：初始化边界。
	void initConfine();

	//日期：2017
	//功能：画边界。
	void bindConfine();

	QPoint pos;										//保存屏幕坐标值
	bool _Depth;									//判断是否获取深度值的开关
	TriangleMesh* autoSupportMesh;					//手动支撑时的模型实例

	//日期：2018.3.21
	//功能：读取深度值和得到手动支撑点.
	void ReadDepth();

	//坐标轴
	QVector<GLfloat> coordVector;
	QOpenGLBuffer coord_vbo;

	//日期：2018.3.26
	//功能：初始化坐标轴.
	void initCoord();

	//日期：2018.3.26
	//功能：画坐标轴.
	void bindCoord();

	//圆环的半径为10mm,圆，圆锥，正方体为1mm单位
	TriangleMesh cube, ring, cone, ball;		//转换标签的基础模型

	void read_basics_mesh();					//读取基础模型

	ModelBuffer *translateMesh_X, *translateMesh_Y, *translateMesh_Z;
	ModelBuffer *scaleMesh_X, *scaleMesh_Y, *scaleMesh_Z;
	ModelBuffer *rotateMesh_X, *rotateMesh_Y, *rotateMesh_Z;

	void initTransformMesh();					//初始化转化模型

	void initTransformMesh_1(ModelBuffer* mb,TriangleMesh mesh,Vectorf3 color,Vectorf3 direction);		

	void drawTranslationMesh();

	void drawTranslationMesh_1(ModelBuffer* mb, QMatrix4x4 scaleM, QMatrix4x4 translateM);

	void renderTranslationMesh_1(TriangleMesh mesh,int id, Vectorf3 direction, QMatrix4x4 scaleM, QMatrix4x4 translateM);

	DLPrinter* dlprinter;				//打印机

};	
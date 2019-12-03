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
#include "DLPrint.h"

//支撑编辑时的支撑点结构体
typedef struct {
	bool exist;		//支撑点是否存在（true--存在，false--不存在）
	Pointf3 p;		//支撑点
}Pointf3Exist;

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

class GlWidget : public  QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	GlWidget(DLPrinter* _dlprinter, Model* _model, DLPrint* _dlprint, std::vector<Pointf3Exist>& ps);
	~GlWidget() {};

	//投影方式
	enum VIEWPORT
	{
		PRESPRECTIVE = 10,
		ORTHOGONALITY
	};

	//渲染平台
	QVector<GLfloat> platform;										//渲染平台的点
	QOpenGLBuffer platform_vbo;										//平台顶点缓存对象
	unsigned short lines_num{ 0 };									//平台的线段的数量
	int viewport;
	bool supportEdit;												//支撑编辑判断
	std::vector<ModelBuffer*> modelBuffers;							//模型渲染缓冲区
	//每个模型对象的渲染数据
	std::map<size_t, stl_render> volumes;
	void drawModel();
	std::vector<SupportBuffer*> treeSupportBuffers;					//树状支撑缓冲区
	int selectID{-1};											//选中模型的ID ,等于-1为未选中
	int translationID;										//选中的转换动作标签ID，等于-1为未选中
	Model* m_model;											//模型
	DLPrint* dlprint;										//dlp打印
	Pointf3 oneSupport;												//手动支撑点
	SupportBuffer* oneBallBuffer;									//手动支撑的缓冲区
	std::vector<SupportBuffer*> supportEditBuffer;					//支撑编辑时渲染缓冲区(100个支撑点为一个渲染缓存区）
	//选中模型的支撑区域---暂时只提供球形区域选择
	std::vector<ModelBuffer*> region_box;

	//支撑编辑模式时模型的支撑点(100个为一个区间）
	std::vector<Pointf3Exist>* treeSupportsExist;

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

	TriangleMesh find_model(size_t id);
	void initSupportEditBuffer(size_t i);
	void initSupportEditBufferAll();
	void ChangeView(int view);
	void addModelBuffer(ModelInstance* instance);
	void saveOneView(char* _name);
	void supportEditChange();
	void addOneSupport();
	void updateConfine();
	bool checkConfine();
	TriangleMesh saveSupport();
	void delModelBuffer(size_t id);
	void delSupportBuffer(size_t id);
	void clearModelBuffer();
	void clearSupportBuffer();
	void bindTreeSupport();
	void bindSupportEditBuffer();
	void initOneSupport();
	void bindOneSupport();
	void initModelMatrix();
	void initPlatform();
	void bindPlatform();
	void setEye();
	void initConfine();
	void bindConfine();
	void ReadDepth();
	void initCoord();
	void bindCoord();
	void read_basics_mesh();					//读取基础模型

	//功能：初始化选中模型的树状支撑。
	//参数1：生成支撑模型的id即生成支撑的id
	//参数2：支撑数据
	void initTreeSupport_id(size_t id,TreeSupport* s,QProgressBar* progress);

	std::vector<TriangleMesh> return_support_model();
	void save_valume(size_t id);
	void clear_volumes();
	void dlprinterChange();
	void setDefaultView();

	void initTransformMesh();					//初始化转化模型
	void initTransformMesh_1(ModelBuffer* mb, TriangleMesh mesh, Vectorf3 color, Vectorf3 direction);
	void drawTranslationMesh();
	void drawTranslationMesh_1(ModelBuffer* mb, QMatrix4x4 scaleM, QMatrix4x4 translateM);
	void renderTranslationMesh_1(TriangleMesh mesh, int id, Vectorf3 direction, QMatrix4x4 scaleM, QMatrix4x4 translateM);

	void addOneSupport(Pointf3 p);
	void deleteOneSupport(size_t id);

	void offsetValueChange(double x, double y, double z,bool back=false);
	void scaleValueChange(double x, double y, double z, bool back = false);
	void rotateValueChange(double angle, int x, int y, int z, bool back = false);
protected:
	void resizeGL(int w, int h) override;
	void paintGL() override;
	void initializeGL() override;

	//功能：重写事件函数。
	void wheelEvent(QWheelEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

signals:
	void signal_modelSelect();
	void signal_offsetChange();
	void signal_scaleChange();
	void signal_rotateChange();

private:

	QOpenGLShaderProgram* m_program{ nullptr };								//OpenGL着色器程序
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

	//视图旋转，平移，缩放
	GLfloat _scale;
	GLfloat _verticalAngle, _horizonAngle;							//视图旋转的水平，垂直值
	QVector3D eye;													//照相机的位置
	QVector3D center;												//世界坐标平移的矢量
	int xLastPos, yLastPos;											//鼠标的坐标值

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

	QPoint pos;										//保存屏幕坐标值
	bool _Depth;									//判断是否获取深度值的开关
	TriangleMesh* autoSupportMesh;					//手动支撑时的模型实例

	//坐标轴
	QVector<GLfloat> coordVector;
	QOpenGLBuffer coord_vbo;

	//圆环的半径为10mm,圆，圆锥，正方体为1mm单位
	TriangleMesh cube, ring, cone, ball;		//转换标签的基础模型

	ModelBuffer *translateMesh_X, *translateMesh_Y, *translateMesh_Z;
	ModelBuffer *scaleMesh_X, *scaleMesh_Y, *scaleMesh_Z;
	ModelBuffer *rotateMesh_X, *rotateMesh_Y, *rotateMesh_Z;

	DLPrinter* dlprinter;				//打印机

};	
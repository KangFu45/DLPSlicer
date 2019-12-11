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

//日期：2017
//功能：模型缓冲区结构体。
//属性1：模型或支撑的编号
//属性2：三角面片的数量
//属性3：三角面片的数组
//属性4：OpenGL缓冲区
struct ModelBuffer {
	int id;
	int size;
	GLfloat* stl;
	QOpenGLBuffer* buffer;

	Pointf3 origin = Pointf3(0.0, 0.0, 0.0);
	Pointf3 new_origin = Pointf3(0.0, 0.0, 0.0);
	//根据三角面片的渲染方式，简单地通过平移原始数据达到模型平移渲染效果
	//TODO:在着色器里进行转换
	void translation() {
		coordf_t delta_x = new_origin.x - origin.x;
		coordf_t delta_y = new_origin.y - origin.y;
		coordf_t delta_z = new_origin.z - origin.z;

		for (int i = 0; i < size; ++i) {
			if (delta_x != 0) {
				stl[i * 18] += delta_x;
				stl[i * 18 + 6] += delta_x;
				stl[i * 18 + 12] += delta_x;
			}

			if (delta_y != 0) {
				stl[i * 18 + 1] += delta_y;
				stl[i * 18 + 7] += delta_y;
				stl[i * 18 + 13] += delta_y;
			}

			if (delta_z != 0) {
				stl[i * 18 + 2] += delta_z;
				stl[i * 18 + 8] += delta_z;
				stl[i * 18 + 14] += delta_z;
			}
		}
		origin = new_origin;
	}

	~ModelBuffer() {
		delete [] stl;
		delete buffer;
	}

};
typedef ModelBuffer SupportBuffer;
typedef std::vector<ModelBuffer> ModelBuffers;
typedef std::vector<ModelBuffer*> ModelBufferPtrs;
typedef std::vector<SupportBuffer> SupportBuffers;
typedef std::vector<SupportBuffer*> SupportBufferPtrs;

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
	GlWidget(Model* _model, DLPrint* _dlprint);
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
	ModelBufferPtrs modelBuffers;							//模型渲染缓冲区
	//每个模型对象的渲染数据
	std::map<size_t, stl_render> volumes;
	void drawModel();
	SupportBufferPtrs treeSupportBuffers;					//树状支撑缓冲区

	//选中的模型实例，一次只能选择一个模型
	//取代选中id编号，未选中时未空
	ModelInstance* m_selInstance{ nullptr };
	int translationID;												//选中的转换动作标签ID，等于-1为未选中
	Model* m_model;													//模型
	DLPrint* dlprint;												//dlp打印

	//支撑编辑控制器，进入支撑编辑模式生成
	//退出支撑模式则删除
	struct SupEditControl
	{
		SupportBuffer* m_curPoint = { nullptr };		//手动支撑的缓冲区
		//TODO:将buffer与points整合
		SupportBufferPtrs supportEditBuffer;			//支撑编辑时渲染缓冲区(100个支撑点为一个渲染缓存区）
		//支撑编辑模式时模型的支撑点(100个为一个区间）
		Pointf3s m_supportEditPoints;
		TriangleMesh* mesh;					//手动支撑时的模型实例

		~SupEditControl() {
			delete m_curPoint;
			delete mesh;
			while (!supportEditBuffer.empty()) {
				auto s = supportEditBuffer.begin();
				delete *s;
				supportEditBuffer.erase(s);
			}
		}
	};
	SupEditControl* m_supEditControl = { nullptr };

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
	void addOneSupport();
	void updateConfine();
	bool checkConfine();
	TriangleMesh saveSupport();
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

	std::vector<TriangleMesh> return_support_model();
	void save_valume(size_t id);
	void clear_volumes();
	void setDefaultView();

	void initTransformMesh();					//初始化转化模型
	void initTransformMesh_1(ModelBuffer* mb, TriangleMesh mesh, Vectorf3 color, Vectorf3 direction);
	void drawTranslationMesh();
	void drawTranslationMesh_1(ModelBuffer* mb, QMatrix4x4 scaleM, QMatrix4x4 translateM);
	void renderTranslationMesh_1(TriangleMesh mesh, int id, Vectorf3 direction, QMatrix4x4 scaleM, QMatrix4x4 translateM);

	void addOneSupport(Pointf3 p);
	void deleteOneSupport(size_t id);

	void offsetValueChange(double x, double y, double z, bool back = false);
	void scaleValueChange(double x, double y, double z, bool back = false);
	void rotateValueChange(double angle, int x, int y, int z, bool back = false);

	bool DelSelectSupport();
	void GenSelInstanceSupport(QProgressBar* progress);
	TreeSupport* GetSelSupport() { return dlprint->chilck_tree_support(selectID); };
	void UpdateTreeSupport(TreeSupport* new_sup, QProgressBar* progress);
	void SupportEditChange(QProgressBar* progress = nullptr);
private:
	void DelModelBuffer(size_t id);

	//功能：初始化选中模型的树状支撑。
	//参数1：生成支撑模型的id即生成支撑的id
	//参数2：支撑数据
	void initTreeSupport_id(size_t id, QProgressBar* progress);
protected:
	void resizeGL(int w, int h) override;
	void paintGL() override;
	void initializeGL() override;

	//功能：重写事件函数。
	void wheelEvent(QWheelEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseDoubleClickEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);

signals:
	void sig_modelSelect();
	void sig_offsetChange();
	void sig_scaleChange();
	void sig_rotateChange();

public slots:
	void slot_delSelectIntance();
	void slot_dlprinterChange(QString name);

private:
	//选中模型的ID ,等于-1为未选中
	int selectID{ -1 };

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

	//坐标轴
	QVector<GLfloat> coordVector;
	QOpenGLBuffer coord_vbo;

	//圆环的半径为10mm,圆，圆锥，正方体为1mm单位
	TriangleMesh cube, ring, cone, ball;		//转换标签的基础模型

	ModelBuffer* translateMesh_X, * translateMesh_Y, * translateMesh_Z;
	ModelBuffer* scaleMesh_X, * scaleMesh_Y, * scaleMesh_Z;
	ModelBuffer* rotateMesh_X, * rotateMesh_Y, * rotateMesh_Z;
};
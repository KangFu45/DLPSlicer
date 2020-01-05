#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QEvent>

#include <gl\GL.h>
#include <gl\GLU.h>

#include "Model.hpp"
#include "DLPrint.h"

struct BasicBuffer {
	int size;	//渲染点的坐标数
	GLfloat* stl = nullptr;
	QOpenGLBuffer* buffer = nullptr;
};

typedef BasicBuffer LineBuffer;

struct RectLimitBuffer : public BasicBuffer{
	bool visible = false;	//超限标记

	void InitStl(int _size) {
		if (stl == nullptr || this->size != _size) {
			this->size = _size;
			stl = new GLfloat[this->size * 3];
		}
	}
	void BindBuffer() {
		if (buffer == nullptr)
			buffer = new QOpenGLBuffer();

		if (!buffer->isCreated())
			buffer->create();
		buffer->bind();
		buffer->allocate(stl, this->size * 3 * sizeof(GLfloat));
		buffer->release();
	}

	~RectLimitBuffer() {
		delete[] stl;
		delete buffer;
	}
};

//日期：2017
//功能：模型缓冲区结构体。
//属性1：模型或支撑的编号
//属性2：三角面片的数量
//属性3：三角面片的数组
//属性4：OpenGL缓冲区
struct ModelBuffer : public BasicBuffer{
	int id;
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


class GlWidget : public  QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	GlWidget(Model* _model, DLPrint* _dlprint);
	~GlWidget();

private:
	//支撑编辑控制器，进入支撑编辑模式生成
	//退出支撑模式则删除
	struct SupEditControl
	{
		SupportBuffer* m_curPoint = { nullptr };	//手动支撑的缓冲区
		SupportBufferPtrs supportPoints;	//支撑编辑时渲染缓冲区
		TriangleMesh* m_mesh = nullptr;			//手动支撑时的模型实例
		//TODO:只是引进ball后面不需要parent
		GlWidget* m_parent;

		void AddSupportPoint(const stl_vertex& ver);
		void DelSupportPoint(size_t id);
		void AddNewSupportPoint();
		void InitOneSupport();
		SupEditControl(GlWidget* parent) :m_parent(parent) {};
		~SupEditControl() {
			delete m_curPoint;
			delete m_mesh;
			while (!supportPoints.empty()) {
				auto s = supportPoints.begin();
				delete* s;
				supportPoints.erase(s);
			}
		}
	};
	SupEditControl* m_supEditControl = { nullptr };

	RectLimitBuffer frontBuffer;
	RectLimitBuffer backBuffer;
	RectLimitBuffer leftBuffer;
	RectLimitBuffer rightBuffer;
	RectLimitBuffer upBuffer;
	RectLimitBuffer downBuffer;

	int translationID;	//选中的转换动作标签ID，等于-1为未选中
	Model* m_model;		//模型
	DLPrint* m_dlprint;	//dlp打印

	LineBuffer platformBuffer;	//平台渲染缓冲区
	LineBuffer coordBuffer;	//坐标轴缓冲区
	ModelBufferPtrs modelBuffers;	//模型渲染缓冲区
	SupportBufferPtrs treeSupportBuffers;	//树状支撑缓冲区

	//选中模型的ID ,等于-1为未选中
	int selectID;

	QOpenGLShaderProgram* m_program{ nullptr };			//OpenGL着色器程序
	int m_projMatrixLoc;
	int m_viewMatrixLoc;

	int m_rotateMatrixLoc;
	int m_scaleMatrixLoc;
	int m_translateMatrixLoc;

	int m_lightPosLoc;
	int func;

	QMatrix4x4 m_proj;								//投影矩阵
	QMatrix4x4 m_camera;							//视图矩阵
	QMatrix4x4 m_world;								//世界坐标矩阵

	//视图旋转，平移，缩放
	GLfloat _scale;
	GLfloat _verticalAngle, _horizonAngle;			//视图旋转的水平，垂直值
	QVector3D eye;									//照相机的位置
	QVector3D center;								//世界坐标平移的矢量
	int xLastPos, yLastPos;							//鼠标的坐标值

	QPoint pos;										//保存屏幕坐标值
	bool _Depth{ false };							//判断是否获取深度值的开关

	//圆环的半径为10mm,圆，圆锥，正方体为1mm单位
	TriangleMesh cube, ring, cone, ball;		//转换标签的基础模型

	ModelBuffer translateMesh_X, translateMesh_Y, translateMesh_Z;
	ModelBuffer scaleMesh_X, scaleMesh_Y, scaleMesh_Z;
	ModelBuffer rotateMesh_X, rotateMesh_Y, rotateMesh_Z;

public:
	//投影方式
	enum VIEWPORT 
	{
		PRESPRECTIVE = 0x01,
		ORTHOGONALITY
	}viewport;

	//各个方向视图的枚举
	enum VIEW
	{
		DEFAULT = 0x03,
		OVERLOOK,
		LEFT,
		RIGHT,
		FRONT,
		BEHIND
	};

	//选中的模型实例，一次只能选择一个模型
	//取代选中id编号，未选中时未空
	ModelInstance* m_selInstance{ nullptr };

	void SetViewPort(VIEWPORT view);
	void ChangeView(VIEW view);
	void SaveOneView(char* file);
	TriangleMesh SaveSupport();
	void ClearModelBuffer();
	void ClearSupportBuffer();
	TriangleMeshs GetSupportModel();
	void UpdateConfine();
	bool CheckConfine();

	void OffsetValueChange(double x, double y, double z, bool back = false);
	void ScaleValueChange(double x, double y, double z, bool back = false);
	void rotateValueChange(double angle, int x, int y, int z, bool back = false);

	bool DelSelectSupport();
	void GenSelInstanceSupport(QProgressBar* progress);
	void SupportEditChange(QProgressBar* progress = nullptr);
	void AddNewSupportPoint() { m_supEditControl->AddNewSupportPoint(); };
	void AddModelInstance(size_t id);

private:
	void DrawModel();
	TriangleMesh FindModel(size_t id);
	void DelModelBuffer(size_t id);
	void BindTreeSupport();
	void BindSupportEditBuffer();
	void BindOneSupport();
	void InitModelMatrix();
	void SetEye();
	void ReadDepth();
	void ReadBasicMesh();//读取基础模型
	void SetDefaultView();
	void DrawTranMesh();
	void DrawTranMeshCase(ModelBuffer* mb, QMatrix4x4 scaleM, QMatrix4x4 translateM);
	void RenderTranMeshCase(TriangleMesh mesh, int id, Vectorf3 direction, QMatrix4x4 scaleM, QMatrix4x4 translateM);
	void InitCoord();
	void BindCoord();
	void InitPlatform();
	void BindPlatform();
	void InitConfine();
	void BindConfine();
	void UpdateTreeSupport(TreeSupport* new_sup, QProgressBar* progress);
	void InitTreeSupportID(size_t id, QProgressBar* progress);

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
};
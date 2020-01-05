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
	int size;	//��Ⱦ���������
	GLfloat* stl = nullptr;
	QOpenGLBuffer* buffer = nullptr;
};

typedef BasicBuffer LineBuffer;

struct RectLimitBuffer : public BasicBuffer{
	bool visible = false;	//���ޱ��

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

//���ڣ�2017
//���ܣ�ģ�ͻ������ṹ�塣
//����1��ģ�ͻ�֧�ŵı��
//����2��������Ƭ������
//����3��������Ƭ������
//����4��OpenGL������
struct ModelBuffer : public BasicBuffer{
	int id;
	Pointf3 origin = Pointf3(0.0, 0.0, 0.0);
	Pointf3 new_origin = Pointf3(0.0, 0.0, 0.0);
	//����������Ƭ����Ⱦ��ʽ���򵥵�ͨ��ƽ��ԭʼ���ݴﵽģ��ƽ����ȾЧ��
	//TODO:����ɫ�������ת��
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
	//֧�ű༭������������֧�ű༭ģʽ����
	//�˳�֧��ģʽ��ɾ��
	struct SupEditControl
	{
		SupportBuffer* m_curPoint = { nullptr };	//�ֶ�֧�ŵĻ�����
		SupportBufferPtrs supportPoints;	//֧�ű༭ʱ��Ⱦ������
		TriangleMesh* m_mesh = nullptr;			//�ֶ�֧��ʱ��ģ��ʵ��
		//TODO:ֻ������ball���治��Ҫparent
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

	int translationID;	//ѡ�е�ת��������ǩID������-1Ϊδѡ��
	Model* m_model;		//ģ��
	DLPrint* m_dlprint;	//dlp��ӡ

	LineBuffer platformBuffer;	//ƽ̨��Ⱦ������
	LineBuffer coordBuffer;	//�����Ỻ����
	ModelBufferPtrs modelBuffers;	//ģ����Ⱦ������
	SupportBufferPtrs treeSupportBuffers;	//��״֧�Ż�����

	//ѡ��ģ�͵�ID ,����-1Ϊδѡ��
	int selectID;

	QOpenGLShaderProgram* m_program{ nullptr };			//OpenGL��ɫ������
	int m_projMatrixLoc;
	int m_viewMatrixLoc;

	int m_rotateMatrixLoc;
	int m_scaleMatrixLoc;
	int m_translateMatrixLoc;

	int m_lightPosLoc;
	int func;

	QMatrix4x4 m_proj;								//ͶӰ����
	QMatrix4x4 m_camera;							//��ͼ����
	QMatrix4x4 m_world;								//�����������

	//��ͼ��ת��ƽ�ƣ�����
	GLfloat _scale;
	GLfloat _verticalAngle, _horizonAngle;			//��ͼ��ת��ˮƽ����ֱֵ
	QVector3D eye;									//�������λ��
	QVector3D center;								//��������ƽ�Ƶ�ʸ��
	int xLastPos, yLastPos;							//��������ֵ

	QPoint pos;										//������Ļ����ֵ
	bool _Depth{ false };							//�ж��Ƿ��ȡ���ֵ�Ŀ���

	//Բ���İ뾶Ϊ10mm,Բ��Բ׶��������Ϊ1mm��λ
	TriangleMesh cube, ring, cone, ball;		//ת����ǩ�Ļ���ģ��

	ModelBuffer translateMesh_X, translateMesh_Y, translateMesh_Z;
	ModelBuffer scaleMesh_X, scaleMesh_Y, scaleMesh_Z;
	ModelBuffer rotateMesh_X, rotateMesh_Y, rotateMesh_Z;

public:
	//ͶӰ��ʽ
	enum VIEWPORT 
	{
		PRESPRECTIVE = 0x01,
		ORTHOGONALITY
	}viewport;

	//����������ͼ��ö��
	enum VIEW
	{
		DEFAULT = 0x03,
		OVERLOOK,
		LEFT,
		RIGHT,
		FRONT,
		BEHIND
	};

	//ѡ�е�ģ��ʵ����һ��ֻ��ѡ��һ��ģ��
	//ȡ��ѡ��id��ţ�δѡ��ʱδ��
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
	void ReadBasicMesh();//��ȡ����ģ��
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

	//���ܣ���д�¼�������
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
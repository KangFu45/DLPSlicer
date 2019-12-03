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

//֧�ű༭ʱ��֧�ŵ�ṹ��
typedef struct {
	bool exist;		//֧�ŵ��Ƿ���ڣ�true--���ڣ�false--�����ڣ�
	Pointf3 p;		//֧�ŵ�
}Pointf3Exist;

//���ڣ�2017
//���ܣ�ģ�ͻ������ṹ�塣
//����1��ģ�ͻ�֧�ŵı��
//����2��������Ƭ������
//����3��������Ƭ������
//����4��OpenGL������
typedef struct {
	int id;
	int size;
	GLfloat* stl;
	QOpenGLBuffer* buffer;
}ModelBuffer;

typedef ModelBuffer SupportBuffer;

//���ڣ�2018.5.29
//���ܣ���������ÿ��modelObjectd����Ⱦ����
//����1��������ĸ���
//����2��ģ�����ݵ�����ָ�룬3*float��һ���㣩+3*floart����ķ�������
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

	//ͶӰ��ʽ
	enum VIEWPORT
	{
		PRESPRECTIVE = 10,
		ORTHOGONALITY
	};

	//��Ⱦƽ̨
	QVector<GLfloat> platform;										//��Ⱦƽ̨�ĵ�
	QOpenGLBuffer platform_vbo;										//ƽ̨���㻺�����
	unsigned short lines_num{ 0 };									//ƽ̨���߶ε�����
	int viewport;
	bool supportEdit;												//֧�ű༭�ж�
	std::vector<ModelBuffer*> modelBuffers;							//ģ����Ⱦ������
	//ÿ��ģ�Ͷ������Ⱦ����
	std::map<size_t, stl_render> volumes;
	void drawModel();
	std::vector<SupportBuffer*> treeSupportBuffers;					//��״֧�Ż�����
	int selectID{-1};											//ѡ��ģ�͵�ID ,����-1Ϊδѡ��
	int translationID;										//ѡ�е�ת��������ǩID������-1Ϊδѡ��
	Model* m_model;											//ģ��
	DLPrint* dlprint;										//dlp��ӡ
	Pointf3 oneSupport;												//�ֶ�֧�ŵ�
	SupportBuffer* oneBallBuffer;									//�ֶ�֧�ŵĻ�����
	std::vector<SupportBuffer*> supportEditBuffer;					//֧�ű༭ʱ��Ⱦ������(100��֧�ŵ�Ϊһ����Ⱦ��������
	//ѡ��ģ�͵�֧������---��ʱֻ�ṩ��������ѡ��
	std::vector<ModelBuffer*> region_box;

	//֧�ű༭ģʽʱģ�͵�֧�ŵ�(100��Ϊһ�����䣩
	std::vector<Pointf3Exist>* treeSupportsExist;

	//����������ͼ��ö��
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
	void read_basics_mesh();					//��ȡ����ģ��

	//���ܣ���ʼ��ѡ��ģ�͵���״֧�š�
	//����1������֧��ģ�͵�id������֧�ŵ�id
	//����2��֧������
	void initTreeSupport_id(size_t id,TreeSupport* s,QProgressBar* progress);

	std::vector<TriangleMesh> return_support_model();
	void save_valume(size_t id);
	void clear_volumes();
	void dlprinterChange();
	void setDefaultView();

	void initTransformMesh();					//��ʼ��ת��ģ��
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

	//���ܣ���д�¼�������
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

	QOpenGLShaderProgram* m_program{ nullptr };								//OpenGL��ɫ������
	int m_projMatrixLoc;
	int m_viewMatrixLoc;

	int m_rotateMatrixLoc;
	int m_scaleMatrixLoc;
	int m_translateMatrixLoc;

	int m_lightPosLoc;
	int func;

	QMatrix4x4 m_proj;												//ͶӰ����
	QMatrix4x4 m_camera;											//��ͼ����
	QMatrix4x4 m_world;												//�����������

	//��ͼ��ת��ƽ�ƣ�����
	GLfloat _scale;
	GLfloat _verticalAngle, _horizonAngle;							//��ͼ��ת��ˮƽ����ֱֵ
	QVector3D eye;													//�������λ��
	QVector3D center;												//��������ƽ�Ƶ�ʸ��
	int xLastPos, yLastPos;											//��������ֵ

	//ǰ
	bool front;	
	QVector<GLfloat> frontVector;
	QOpenGLBuffer front_vbo;

	//��
	bool back;
	QVector<GLfloat> backVector;
	QOpenGLBuffer back_vbo;

	//��
	bool left;
	QVector<GLfloat> leftVector;
	QOpenGLBuffer left_vbo;

	//��
	bool right;
	QVector<GLfloat> rightVector;
	QOpenGLBuffer right_vbo;

	//��
	bool up;
	QVector<GLfloat> upVector;
	QOpenGLBuffer up_vbo;

	//��
	bool down;
	QVector<GLfloat> downVector;
	QOpenGLBuffer down_vbo;

	QPoint pos;										//������Ļ����ֵ
	bool _Depth;									//�ж��Ƿ��ȡ���ֵ�Ŀ���
	TriangleMesh* autoSupportMesh;					//�ֶ�֧��ʱ��ģ��ʵ��

	//������
	QVector<GLfloat> coordVector;
	QOpenGLBuffer coord_vbo;

	//Բ���İ뾶Ϊ10mm,Բ��Բ׶��������Ϊ1mm��λ
	TriangleMesh cube, ring, cone, ball;		//ת����ǩ�Ļ���ģ��

	ModelBuffer *translateMesh_X, *translateMesh_Y, *translateMesh_Z;
	ModelBuffer *scaleMesh_X, *scaleMesh_Y, *scaleMesh_Z;
	ModelBuffer *rotateMesh_X, *rotateMesh_Y, *rotateMesh_Z;

	DLPrinter* dlprinter;				//��ӡ��

};	
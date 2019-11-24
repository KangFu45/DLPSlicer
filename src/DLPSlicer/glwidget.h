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

//���ߣ�����
//���ڣ�2017
//���ܣ���Ⱦ����
class GlWidget : public  QOpenGLWidget, protected QOpenGLFunctions
{
public:
	GlWidget(MainWindow* parent, DLPrinter* _dlprinter, Model* _model, DLPrint* _dlprint);
	~GlWidget() {};

	int selectID;								//ѡ��ģ�͵�ID ,����-1Ϊδѡ��
	int translationID;							//ѡ�е�ת��������ǩID������-1Ϊδѡ��


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

	//���ڣ�2018.4.10
	//���ܣ�����ģ�͡�
	//����1��ģ��id
	//���أ�ģ��
	TriangleMesh find_model(size_t id);

	//���ڣ�2018.4.10
	//���ܣ���ʼ��֧�ű༭��Ⱦ��������
	void initSupportEditBuffer(size_t i);

	void initSupportEditBufferAll();

	//���ڣ�2017
	//���ܣ���ͼ�ı䡣
	//����1����Ҫ�ı���ͼ��ö��ֵ
	void ChangeView(int view);

	//���ڣ�2017
	//���ܣ����ģ�ͻ�������
	//����1���¼���ģ��ʵ����ģ��ʵ��
	void addModelBuffer(ModelInstance* instance);

	//���ڣ�2017
	//���ܣ�������ͼ����ͼ����
	//����1������·��
	void saveOneView(char* _name);

	//���ڣ�2017
	//���ܣ�֧�ű༭״̬�ı䡣
	void supportEditChange();

	//���ڣ�2017
	//���ܣ����һ��֧�š�
	void addOneSupport();

	//���ڣ�2017
	//���ܣ����±߽���Ϣ��
	void updateConfine();

	//���ڣ�2017
	//���ܣ����߽硣
	bool checkConfine();

	//���ڣ�2017
	//���ܣ�����֧�š�
	//���أ�������Ƭʵ��
	TriangleMesh saveSupport();

	//���ڣ�2017
	//���ܣ�ɾ��һ��ģ�ͻ�������
	//����1��ѡ����ģ��id
	void delModelBuffer(size_t id);

	//���ڣ�2017
	//���ܣ�ɾ��һ��֧�Ż�������
	//����1��ѡ����֧��id
	void delSupportBuffer(size_t id);

	//���ڣ�2017
	//���ܣ�����ģ�ͻ�������
	void clearModelBuffer();

	//���ڣ�2017
	//���ܣ�����֧�Ż�������
	void clearSupportBuffer();

	//���ڣ�2018.4.9
	//���ܣ���ʼ��ѡ��ģ�͵���״֧�š�
	//����1������֧��ģ�͵�id������֧�ŵ�id
	//����2��֧������
	void initTreeSupport_id(size_t id,TreeSupport* s,QProgressBar* progress);

	//���ڣ�2018.4.12
	//���ܣ�����֧��ģ�͡�
	//���أ�֧��ģ��
	std::vector<TriangleMesh> return_support_model();

	//���ڣ�2018.5.29
	//���ܣ�����ÿ��ModelObject��valume��ֻ��һ��ԭʼʵ�壩
	//����1�����ʵ����id
	void save_valume(size_t id);

	//���ڣ�2018.5.29
	//���ܣ������Ⱦ����
	void clear_volumes();

	//���ڣ�2018.10.10
	//���ܣ���ӡ���ı�
	void dlprinterChange();

protected:
	void resizeGL(int w, int h) override;
	void paintGL() override;
	void initializeGL() override;

protected:
	//****************************
	//���ڣ�2017
	//���ܣ���д�¼�������
	void wheelEvent(QWheelEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	//*************************************
private:
	//���ڣ�2017
	//���ܣ�һ��֧�����㡣
	void zeroOneSupport();

private:
	Model* model;								//ģ��
	DLPrint* dlprint;							//dlp��ӡ

	//ͶӰ��ʽ
	enum VIEWPORT
	{
		PRESPRECTIVE = 10,
		ORTHOGONALITY
	};

	int viewport;

	bool supportEdit;												//֧�ű༭�ж�

	MainWindow* mainWindow;											//������

	//��Ⱦģ��
	std::vector<ModelBuffer*> modelBuffers;							//ģ����Ⱦ������

	//ÿ��ģ�Ͷ������Ⱦ����
	std::map<size_t, stl_render> volumes;

	//���ڣ�2017
	//���ܣ���ģ�͡�
	void drawModel();

	std::vector<SupportBuffer*> treeSupportBuffers;					//��״֧�Ż�����

	//���ڣ�2018.4.9
	//���ܣ���Ⱦ��״֧��
	void bindTreeSupport();

	//��Ⱦ�ֶ�֧��
	Pointf3 oneSupport;												//�ֶ�֧�ŵ�
	SupportBuffer* oneBallBuffer;									//�ֶ�֧�ŵĻ�����
	std::vector<SupportBuffer*> supportEditBuffer;					//֧�ű༭ʱ��Ⱦ������(100��֧�ŵ�Ϊһ����Ⱦ��������


	//ѡ��ģ�͵�֧������---��ʱֻ�ṩ��������ѡ��
	std::vector<ModelBuffer*> region_box;


	//���ڣ�2018.4.10
	//���ܣ���Ⱦ֧�ű༭�ĵ㡣
	void bindSupportEditBuffer();

	//���ڣ�2017
	//���ܣ���ʼ���ֶ�֧�š�
	void initOneSupport();

	//���ڣ�2017
	//���ܣ����ֶ�֧�š�
	void bindOneSupport();

	void initModelMatrix();

	//��Ⱦƽ̨
	QVector<GLfloat> platform;										//��Ⱦƽ̨�ĵ�
	QOpenGLBuffer platform_vbo;										//ƽ̨���㻺�����
	unsigned short lines_num{ 0 };									//ƽ̨���߶ε�����

	//���ڣ�2017
	//���ܣ���ʼ��ƽ̨��
	void initPlatform();

	//���ڣ�2017
	//���ܣ���ƽ̨��
	void bindPlatform();

	QOpenGLShaderProgram *m_program;								//OpenGL��ɫ������
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

	void setDefaultView();

	//��ͼ��ת��ƽ�ƣ�����
	GLfloat _scale;
	GLfloat _verticalAngle, _horizonAngle;							//��ͼ��ת��ˮƽ����ֱֵ
	QVector3D eye;													//�������λ��
	QVector3D center;												//��������ƽ�Ƶ�ʸ��
	int xLastPos, yLastPos;											//��������ֵ

	//���ڣ�2017
	//���ܣ������������λ�á�
	void setEye();

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

	//���ڣ�2017
	//���ܣ���ʼ���߽硣
	void initConfine();

	//���ڣ�2017
	//���ܣ����߽硣
	void bindConfine();

	QPoint pos;										//������Ļ����ֵ
	bool _Depth;									//�ж��Ƿ��ȡ���ֵ�Ŀ���
	TriangleMesh* autoSupportMesh;					//�ֶ�֧��ʱ��ģ��ʵ��

	//���ڣ�2018.3.21
	//���ܣ���ȡ���ֵ�͵õ��ֶ�֧�ŵ�.
	void ReadDepth();

	//������
	QVector<GLfloat> coordVector;
	QOpenGLBuffer coord_vbo;

	//���ڣ�2018.3.26
	//���ܣ���ʼ��������.
	void initCoord();

	//���ڣ�2018.3.26
	//���ܣ���������.
	void bindCoord();

	//Բ���İ뾶Ϊ10mm,Բ��Բ׶��������Ϊ1mm��λ
	TriangleMesh cube, ring, cone, ball;		//ת����ǩ�Ļ���ģ��

	void read_basics_mesh();					//��ȡ����ģ��

	ModelBuffer *translateMesh_X, *translateMesh_Y, *translateMesh_Z;
	ModelBuffer *scaleMesh_X, *scaleMesh_Y, *scaleMesh_Z;
	ModelBuffer *rotateMesh_X, *rotateMesh_Y, *rotateMesh_Z;

	void initTransformMesh();					//��ʼ��ת��ģ��

	void initTransformMesh_1(ModelBuffer* mb,TriangleMesh mesh,Vectorf3 color,Vectorf3 direction);		

	void drawTranslationMesh();

	void drawTranslationMesh_1(ModelBuffer* mb, QMatrix4x4 scaleM, QMatrix4x4 translateM);

	void renderTranslationMesh_1(TriangleMesh mesh,int id, Vectorf3 direction, QMatrix4x4 scaleM, QMatrix4x4 translateM);

	DLPrinter* dlprinter;				//��ӡ��

};	
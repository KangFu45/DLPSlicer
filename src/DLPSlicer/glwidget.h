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

//���ڣ�2017
//���ܣ�ģ�ͻ������ṹ�塣
//����1��ģ�ͻ�֧�ŵı��
//����2��������Ƭ������
//����3��������Ƭ������
//����4��OpenGL������
struct ModelBuffer {
	int id;
	int size;
	GLfloat* stl;
	QOpenGLBuffer* buffer;

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
	GlWidget(Model* _model, DLPrint* _dlprint);
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
	ModelBufferPtrs modelBuffers;							//ģ����Ⱦ������
	//ÿ��ģ�Ͷ������Ⱦ����
	std::map<size_t, stl_render> volumes;
	void drawModel();
	SupportBufferPtrs treeSupportBuffers;					//��״֧�Ż�����

	//ѡ�е�ģ��ʵ����һ��ֻ��ѡ��һ��ģ��
	//ȡ��ѡ��id��ţ�δѡ��ʱδ��
	ModelInstance* m_selInstance{ nullptr };
	int translationID;												//ѡ�е�ת��������ǩID������-1Ϊδѡ��
	Model* m_model;													//ģ��
	DLPrint* dlprint;												//dlp��ӡ

	//֧�ű༭������������֧�ű༭ģʽ����
	//�˳�֧��ģʽ��ɾ��
	struct SupEditControl
	{
		SupportBuffer* m_curPoint = { nullptr };		//�ֶ�֧�ŵĻ�����
		//TODO:��buffer��points����
		SupportBufferPtrs supportEditBuffer;			//֧�ű༭ʱ��Ⱦ������(100��֧�ŵ�Ϊһ����Ⱦ��������
		//֧�ű༭ģʽʱģ�͵�֧�ŵ�(100��Ϊһ�����䣩
		Pointf3s m_supportEditPoints;
		TriangleMesh* mesh;					//�ֶ�֧��ʱ��ģ��ʵ��

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
	void read_basics_mesh();					//��ȡ����ģ��

	std::vector<TriangleMesh> return_support_model();
	void save_valume(size_t id);
	void clear_volumes();
	void setDefaultView();

	void initTransformMesh();					//��ʼ��ת��ģ��
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

	//���ܣ���ʼ��ѡ��ģ�͵���״֧�š�
	//����1������֧��ģ�͵�id������֧�ŵ�id
	//����2��֧������
	void initTreeSupport_id(size_t id, QProgressBar* progress);
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

private:
	//ѡ��ģ�͵�ID ,����-1Ϊδѡ��
	int selectID{ -1 };

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

	//������
	QVector<GLfloat> coordVector;
	QOpenGLBuffer coord_vbo;

	//Բ���İ뾶Ϊ10mm,Բ��Բ׶��������Ϊ1mm��λ
	TriangleMesh cube, ring, cone, ball;		//ת����ǩ�Ļ���ģ��

	ModelBuffer* translateMesh_X, * translateMesh_Y, * translateMesh_Z;
	ModelBuffer* scaleMesh_X, * scaleMesh_Y, * scaleMesh_Z;
	ModelBuffer* rotateMesh_X, * rotateMesh_Y, * rotateMesh_Z;
};
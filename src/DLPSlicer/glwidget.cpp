#include "glwidget.h"
#include "qevent.h"
#include <vector>

#include <qdir.h>
#include <qfileinfo.h>
#include <qsettings.h>

#include "Setting.h"
extern Setting e_setting;
static int TranInvalidID = -10;
static int SelInvalidID = -1;

//功能：bmp的头文件。
static char head[54] = {0x42,0x4d,0x66,0x75,0x00,0x00,0x00,0x00,0x00,0x00,0x36,0x00,0x00,0x00,0x28,0x00,0x00
,0x00,0x64,0x00,0x00,0x00,0x64,0x00,0x00,0x00,0x01,0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x30,0x75};

GlWidget::GlWidget(Model* _model, DLPrint* _dlprint)
	: translationID(TranInvalidID)
	, selectID(SelInvalidID)
	, viewport(ORTHOGONALITY)
	, m_model(_model)
	, m_dlprint(_dlprint)
{
	translateMesh_X.id = -1;
	translateMesh_Y.id = -2;
	translateMesh_Z.id = -3;
	scaleMesh_X.id = -4;
	scaleMesh_Y.id = -5;
	scaleMesh_Z.id = -6;
	rotateMesh_X.id = -7;
	rotateMesh_Y.id = -8;
	rotateMesh_Z.id = -9;

	SetDefaultView();
	ReadBasicMesh();
	setMouseTracking(true);//设置鼠标跟踪
}

GlWidget::~GlWidget()
{
	if (m_supEditControl != nullptr)
		delete m_supEditControl;
	delete m_program;
}

//顶点着色器
//着色器阶段，顶点着色器输出的变量会输入到下一个着色器，如没有其他着色器便会直接输入到片元着色器。
//将模型的三种转化放在顶点着色器阶段进行？？？
static const char *vertexShaderSourceCore =
"#version 330\n"												//opengl的版本
"layout(location = 0) in vec4 vertex;\n"						//输入的顶点坐标值（使用四元数计算）
"layout(location = 1) in vec3 normal;\n"						//输入的顶点法向量
"layout(location = 2) in vec4 color;\n"							//输入的颜色信息
"out vec3 vert;\n"												//输出的顶点坐标值
"out vec3 vertNormal;\n"										//输出的顶点法向量
"out vec3 vertColor;\n"											//输出的顶点颜色信息
"uniform mat4 projMatrix;\n"									//投影矩阵
"uniform mat4 viewMatrix;\n"									//视图矩阵
"uniform mat4 rotateMatrix;\n"
"uniform mat4 scaleMatrix;\n"
"uniform mat4 translateMatrix;\n"
"void main() {\n"												//主函数
"	vec4 mv_vertex=viewMatrix * translateMatrix  * scaleMatrix *rotateMatrix * vertex;\n"//得到转化后的顶点坐标
"   vert = mv_vertex.xyz;\n"									//计算输出的顶点坐标值
"   vertNormal =mat3(rotateMatrix) * normal;\n"					//计算输出的顶点法向量值
"	vertColor=color.xyz;\n"										//将顶点颜色信息传入下个坐标
"   gl_Position = projMatrix * mv_vertex;\n"					//计算顶点的位置（直接用这个值渲染点）
"}\n";

//片元着色器
static const char *fragmentShaderSourceCore =
"#version 330\n"
"in highp vec3 vert;\n"											//从上个着色器输入的顶点坐标值
"in highp vec3 vertNormal;\n"									//从上个着色器输入的顶点法向量
"in highp vec3 vertColor;\n"									//从上个着色器输入的顶点颜色值
"out highp vec4 fragColor;\n"									//输出的顶点片元颜色
"uniform highp vec3 lightPos;\n"								//光源的位置
"uniform int func;\n"											//不同的处理函数
"void main() {\n"
"   if(func==1){\n"//渲染模型
"       highp vec3 L = normalize(lightPos - vert);\n"
"       highp float NL = max(dot(normalize(vertNormal), L), 0.0);\n"
"		highp vec3 color = vec3(0.39, 1.0, 0.0);\n"
"		highp vec3 col = clamp(color * 0.2 + color * 0.8 * NL, 0.0, 1.0);\n"
"       fragColor = vec4(col, 1.0);\n"
"   }\n"
"   else if(func==2){\n"//渲染平台
"       fragColor = vec4(0.0 ,0.0, 0.0, 1.0);\n"
"   }\n"
"	else if(func==3){\n"//渲染选中模型
"       highp vec3 L = normalize(lightPos - vert);\n"
"       highp float NL = max(dot(normalize(vertNormal), L), 0.0);\n"
"		highp vec3 color = vec3(1.0, 0.39, 0.0);\n"
"		highp vec3 col = clamp(color * 0.2 + color * 0.8 * NL, 0.0, 1.0);\n"
"       fragColor = vec4(col, 1.0);\n"
"	}\n"
"   else if(func==4){\n"//渲染支撑			 
"       highp vec3 L = normalize(lightPos - vert);\n"
"       highp float NL = max(dot(normalize(vertNormal), L), 0.0);\n"
"		highp vec3 color = vec3(0.2, 0.5, 0.6);\n"
"		highp vec3 col = clamp(color * 0.2 + color * 0.8 * NL, 0.0, 1.0);\n"
"       fragColor = vec4(col, 1.0);\n"
"   }\n"
"   else if(func==5){\n"//渲染边界
"       fragColor = vec4(0.5 ,0.0, 0.0, 0.3);\n"
"   }\n"
"   else if(func==6){\n"//支撑渲染模式下渲染单个支撑
"       highp vec3 L = normalize(lightPos - vert);\n"
"       highp float NL = max(dot(normalize(vertNormal), L), 0.0);\n"
"		highp vec3 color = vec3(0.0, 0.1, 0.9);\n"
"		highp vec3 col = clamp(color * 0.2 + color * 0.8 * NL, 0.0, 1.0);\n"
"       fragColor = vec4(col, 1.0);\n"
"   }\n"
"	else if(func==7){\n"//渲染坐标轴，直接使用输入的顶点的颜色信息
"		fragColor=vec4(vertColor,1.0);\n"
"	}"
"   else if(func==8){\n"		 
"       highp vec3 L = normalize(lightPos - vert);\n"
"       highp float NL = max(dot(normalize(vertNormal), L), 0.0);\n"
"		highp vec3 color = vertColor;\n"
"		highp vec3 col = clamp(color * 0.8 + color * 0.2 * NL, 0.0, 1.0);\n"
"       fragColor = vec4(col, 0.8);\n"
"   }\n"
"}\n";

void InitTransformMesh(ModelBuffer* mb, TriangleMesh mesh, Vectorf3 color, Vectorf3 direction)
{
	mesh.rotate_x((direction.x / 180) * PI);
	mesh.rotate_y((direction.y) / 180 * PI);
	mesh.rotate_z((direction.z) / 180 * PI);

	mb->size = mesh.stl.stats.number_of_facets;
	mb->stl = new GLfloat[mb->size * 3 * 3 * 3];  //包含  坐标值  法向量  颜色

	for (int i = 0; i < mb->size; ++i) {
		mb->stl[i * 27] = mesh.stl.facet_start[i].vertex[0].x;
		mb->stl[i * 27 + 1] = mesh.stl.facet_start[i].vertex[0].y;
		mb->stl[i * 27 + 2] = mesh.stl.facet_start[i].vertex[0].z;
		mb->stl[i * 27 + 3] = mesh.stl.facet_start[i].normal.x;
		mb->stl[i * 27 + 4] = mesh.stl.facet_start[i].normal.y;
		mb->stl[i * 27 + 5] = mesh.stl.facet_start[i].normal.z;
		mb->stl[i * 27 + 6] = color.x;
		mb->stl[i * 27 + 7] = color.y;
		mb->stl[i * 27 + 8] = color.z;

		mb->stl[i * 27 + 9] = mesh.stl.facet_start[i].vertex[1].x;
		mb->stl[i * 27 + 10] = mesh.stl.facet_start[i].vertex[1].y;
		mb->stl[i * 27 + 11] = mesh.stl.facet_start[i].vertex[1].z;
		mb->stl[i * 27 + 12] = mesh.stl.facet_start[i].normal.x;
		mb->stl[i * 27 + 13] = mesh.stl.facet_start[i].normal.y;
		mb->stl[i * 27 + 14] = mesh.stl.facet_start[i].normal.z;
		mb->stl[i * 27 + 15] = color.x;
		mb->stl[i * 27 + 16] = color.y;
		mb->stl[i * 27 + 17] = color.z;

		mb->stl[i * 27 + 18] = mesh.stl.facet_start[i].vertex[2].x;
		mb->stl[i * 27 + 19] = mesh.stl.facet_start[i].vertex[2].y;
		mb->stl[i * 27 + 20] = mesh.stl.facet_start[i].vertex[2].z;
		mb->stl[i * 27 + 21] = mesh.stl.facet_start[i].normal.x;
		mb->stl[i * 27 + 22] = mesh.stl.facet_start[i].normal.y;
		mb->stl[i * 27 + 23] = mesh.stl.facet_start[i].normal.z;
		mb->stl[i * 27 + 24] = color.x;
		mb->stl[i * 27 + 25] = color.y;
		mb->stl[i * 27 + 26] = color.z;
	}

	mb->buffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	mb->buffer->create();
	mb->buffer->bind();
	mb->buffer->allocate(mb->stl, mb->size * 3 * 3 * 3 * sizeof(GLfloat));
	mb->buffer->release();
}

void GlWidget::initializeGL()
{
	initializeOpenGLFunctions();
	glClearColor(0.9, 0.9, 0.9, 0.9);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);//裁减背面

	//点，线反走样
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	
	glPolygonMode(GL_BACK, GL_LINE);
	glPolygonMode(GL_FRONT, GL_FILL);

	m_program = new QOpenGLShaderProgram;
	m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSourceCore);//从源代码中添加顶点着色器
	m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSourceCore);//从源代码中添加片元着色器
	//m_program->bindAttributeLocation("vertex", 0);
	//m_program->bindAttributeLocation("normal", 1);
	m_program->link();

	m_program->bind();

	m_projMatrixLoc = m_program->uniformLocation("projMatrix");//透视投影矩阵
	m_viewMatrixLoc = m_program->uniformLocation("viewMatrix");//视图矩阵
	m_lightPosLoc = m_program->uniformLocation("lightPos");//光照矩阵
	func = m_program->uniformLocation("func");

	m_rotateMatrixLoc = m_program->uniformLocation("rotateMatrix");
	m_scaleMatrixLoc = m_program->uniformLocation("scaleMatrix");
	m_translateMatrixLoc = m_program->uniformLocation("translateMatrix");

	//glClipPlane(GL_CLIP_PLANE0, 50.0);

	//translation
	InitTransformMesh(&translateMesh_X, cone, Vectorf3(0.8, 0.0, 0.0), Vectorf3(0, 90, 0));
	InitTransformMesh(&translateMesh_Y, cone, Vectorf3(0.0, 0.8, 0.0), Vectorf3(90, 0, 0));
	InitTransformMesh(&translateMesh_Z, cone, Vectorf3(0.0, 0.0, 0.8), Vectorf3(0, 0, 0));
	//scale
	InitTransformMesh(&scaleMesh_X, cube, Vectorf3(0.8, 0.0, 0.0), Vectorf3(0, -90, 0));
	InitTransformMesh(&scaleMesh_Y, cube, Vectorf3(0.0, 0.8, 0.0), Vectorf3(-90, 0, 0));
	InitTransformMesh(&scaleMesh_Z, cube, Vectorf3(0.0, 0.0, 0.8), Vectorf3(180, 0, 0));
	//rotate
	InitTransformMesh(&rotateMesh_X, ring, Vectorf3(0.8, 0.0, 0.0), Vectorf3(90, 0, 0));
	InitTransformMesh(&rotateMesh_Y, ring, Vectorf3(0.0, 0.8, 0.0), Vectorf3(0, 90, 0));
	InitTransformMesh(&rotateMesh_Z, ring, Vectorf3(0.0, 0.0, 0.8), Vectorf3(0, 0, 0));

	InitPlatform();
	InitConfine();
	InitCoord();
	SetEye();
	m_program->release();
}

void GlWidget::paintGL()
{
	// 清除颜色和深度缓存
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//GLdouble eqn[4] = { 0.0,0.0,1.0,0.0 };
	//glClipPlane(GL_CLIP_PLANE0, eqn);  // 经过此句裁剪后，球体只剩下上半部分
	//glEnable(GL_CLIP_PLANE0);

	//视图矩阵
	m_camera.setToIdentity();
	m_camera.lookAt(eye, QVector3D(0, 0, 0), QVector3D(0, 0, 1));
	m_camera.scale(_scale);

	//世界坐标矩阵
	m_world.setToIdentity();
	m_world.translate(0, 0, -130);
	m_world.translate(center);

	m_program->bind();
	m_program->setUniformValue(m_projMatrixLoc, m_proj);//只能在应用程序里才能改变以uniform为限制符的值
	m_program->setUniformValue(m_viewMatrixLoc, m_camera*m_world);
	m_program->setUniformValue(m_lightPosLoc,eye);
	
	//渲染模型
	DrawModel();
	DrawTranMesh();
	glDisable(GL_CULL_FACE);

	//渲染树状支撑
	if (!m_dlprint->TreeSupportsEmpty() && m_supEditControl == nullptr)
		BindTreeSupport();

	//添加支撑时的标识渲染
	if (m_supEditControl != nullptr) {
		BindOneSupport();
		if (!m_supEditControl->supportPoints.empty())
			BindSupportEditBuffer();
	}
	else {
		//渲染边界
		glPolygonMode(GL_BACK, GL_FILL);
		BindConfine();
		glPolygonMode(GL_BACK, GL_LINE);

		//渲染坐标轴
		glLineWidth(3);
		BindCoord();
		glLineWidth(1);

		//渲染平台
		BindPlatform();
	}

	glEnable(GL_CULL_FACE);

	//在结束渲染时读取深度值
	ReadDepth();

	m_program->release();

}

void GlWidget::resizeGL(int w, int h)
{
	m_proj.setToIdentity();

	if(viewport==PRESPRECTIVE)
		m_proj.perspective(45, GLfloat(w) / h, 0.1f, 10000.0f);
	else
		m_proj.ortho(-w / 2, w / 2, -h / 2, h / 2, -10000.0f, 10000.0f);

	emit sig_sizeChange();
}

//函数：m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
//功能：读取缓冲区的规则。
//参数1：属性的id
//参数2：属性的数据类型
//参数3：读取数组的起始位置
//参数4：读取一个顶点的一个属性的距离（一小步）
//参数5：读取一个顶点的所有属性的距离（一大步）
void GlWidget::DrawModel()//渲染模型
{
	for (auto m = modelBuffers.begin(); m != modelBuffers.end(); ++m) {
		ModelBuffer* mb = *m;
		mb->buffer->bind();
		m_program->enableAttributeArray(0);//顶点属性数组使能
		m_program->enableAttributeArray(1);
		m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));//将顶点着色器的变量与存储在缓存对象中数据关联（即着色管线装配的过程）
		m_program->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

		//-------------设置模型矩阵------------
		ModelInstance* instance = m_model->find_instance(mb->id);
		//得到原始模型中心
		BoundingBoxf3 box = instance->get_object()->volumes[0]->mesh.bounding_box();
		Pointf3 origin((box.max.x + box.min.x) / 2, (box.max.y + box.min.y) / 2, (box.max.z + box.min.z) / 2);

		QMatrix4x4 translateM,scaleM;
		translateM.setToIdentity();
		scaleM.setToIdentity();

		QVector4D v(origin.x, origin.y, origin.z, 1);
		QVector4D v1 = instance->rotation_M*v;

		scaleM.scale(instance->scaling_vector.x, instance->scaling_vector.y, instance->scaling_vector.z);
		translateM.translate(instance->offset.x, instance->offset.y, instance->z_translation);

		translateM.translate(v.x() - v1.x(), v.y() - v1.y(), v.z() - v1.z());

		m_program->setUniformValue(m_rotateMatrixLoc, instance->rotation_M);
		m_program->setUniformValue(m_scaleMatrixLoc, scaleM);
		m_program->setUniformValue(m_translateMatrixLoc, translateM);
		//-------------------------------------

		if (mb->id == selectID) {
			m_program->setUniformValue(func, 3);
			glDrawArrays(GL_TRIANGLES, 0, mb->size * 3); //第三个参数是顶点的数量
		}
		else if (m_supEditControl == nullptr) {
			m_program->setUniformValue(func, 1);
			glDrawArrays(GL_TRIANGLES, 0, mb->size * 3);
		}

		mb->buffer->release();
		m_program->disableAttributeArray(0);
		m_program->disableAttributeArray(1);
	}
}

void GlWidget::DelModelBuffer(size_t id)//删除一个模型缓冲区
{
	for (auto m = modelBuffers.begin(); m != modelBuffers.end(); ++m) {
		if ((*m)->id == id) {
			delete* m;
			modelBuffers.erase(m);
			update();
			break;
		}
	}

	for (auto m = modelBuffers.begin(); m != modelBuffers.end(); ++m) {
		if ((*m)->id / InstanceNum == id / InstanceNum && (*m)->id > id)
			(*m)->id = (*m)->id - 1;
	}
}

void GlWidget::ClearModelBuffer()
{
	while (!modelBuffers.empty()) {
		delete* (modelBuffers.begin());
		modelBuffers.erase(modelBuffers.begin());
	}
	modelBuffers.swap(ModelBufferPtrs());

	selectID = SelInvalidID;
	m_selInstance = nullptr;
	translationID = TranInvalidID;

	UpdateConfine();
	update();
}

void GlWidget::ClearSupportBuffer()
{
	while (!treeSupportBuffers.empty()) {
		delete* (treeSupportBuffers.begin());
		treeSupportBuffers.erase(treeSupportBuffers.begin());
	}
	treeSupportBuffers.swap(SupportBufferPtrs());
	update();
}

void GlWidget::InitPlatform()
{
	unsigned int L = e_setting.m_printers.begin()->length;
	unsigned int W = e_setting.m_printers.begin()->width;
	unsigned int H = e_setting.m_printers.begin()->height;

	unsigned int a = L / 10;
	unsigned int b = L % 10;

	unsigned int c = W / 10;
	unsigned int d = W % 10;

	platformBuffer.size = a + c + 12 - 2;
	if (platformBuffer.stl != nullptr)
		delete[] platformBuffer.stl;
	platformBuffer.stl = new GLfloat[platformBuffer.size * 2 * 3];
	GLfloat* p = platformBuffer.stl;

	//x方向中间a-1根线
	for (int i = 1; i < a; i++) {
		*p++ = GLfloat(i * 10 - GLfloat((L - b) / 2)); *p++ = GLfloat(W / 2); *p++ = 0;

		*p++ = GLfloat(i * 10 - GLfloat((L - b) / 2)); *p++ = -GLfloat(W / 2); *p++ = 0;
	}
	//x方向最右边一根
	*p++ = GLfloat(L / 2); *p++ = GLfloat(W / 2); *p++ = 0;
	*p++ = GLfloat(L / 2); *p++ = -GLfloat(W / 2); *p++ = 0;

	//y方向中间c-1根
	for (int j = 1; j < c; j++) {
		*p++ = -GLfloat(L / 2); *p++ = GLfloat(GLfloat((W - d) / 2) - j * 10); *p++ = 0;

		*p++ = GLfloat(L / 2); *p++ = GLfloat(GLfloat((W - d) / 2) - j * 10); *p++ = 0;
	}
	//y方向最上面一根
	*p++ = -GLfloat(L / 2); *p++ = GLfloat(W / 2); *p++ = 0;
	*p++ = GLfloat(L / 2); *p++ = GLfloat(W / 2); *p++ = 0;

	*p++ = -GLfloat(L / 2); *p++ = -GLfloat(W / 2); *p++ = H;
	*p++ = GLfloat(L / 2);  *p++ = -GLfloat(W / 2); *p++ = H;

	*p++ = GLfloat(L / 2); *p++ = -GLfloat(W / 2); *p++ = H;
	*p++ = GLfloat(L / 2); *p++ = -GLfloat(W / 2); *p++ = 0;

	*p++ = GLfloat(L / 2); *p++ = -GLfloat(W / 2); *p++ = H;
	*p++ = GLfloat(L / 2); *p++ = GLfloat(W / 2); *p++ = H;

	*p++ = GLfloat(L / 2); *p++ = GLfloat(W / 2); *p++ = H;
	*p++ = GLfloat(L / 2); *p++ = GLfloat(W / 2); *p++ = 0;

	*p++ = GLfloat(L / 2); *p++ = GLfloat(W / 2); *p++ = H;
	*p++ = -GLfloat(L / 2); *p++ = GLfloat(W / 2); *p++ = H;

	*p++ = -GLfloat(L / 2); *p++ = GLfloat(W / 2); *p++ = H;
	*p++ = -GLfloat(L / 2); *p++ = GLfloat(W / 2); *p++ = 0;

	*p++ = -GLfloat(L / 2); *p++ = GLfloat(W / 2); *p++ = H;
	*p++ = -GLfloat(L / 2); *p++ = -GLfloat(W / 2); *p++ = H;

	//----------缩短30mm--------------
	*p++ = GLfloat(L / 2); *p++ = -GLfloat(W / 2); *p++ = 0;
	*p++ = -GLfloat(GLfloat(L / 2) - 30); *p++ = -GLfloat(W / 2); *p++ = 0;

	*p++ = -GLfloat(L / 2); *p++ = -GLfloat(W / 2); *p++ = 30;
	*p++ = -GLfloat(L / 2); *p++ = -GLfloat(W / 2); *p++ = H;

	*p++ = -GLfloat(L / 2); *p++ = GLfloat(W / 2); *p++ = 0;
	*p++ = -GLfloat(L / 2); *p++ = -GLfloat(GLfloat(W / 2) - 30); *p++ = 0;
	//---------------------------------

	if (platformBuffer.buffer == nullptr)
		platformBuffer.buffer = new QOpenGLBuffer();

	if (!platformBuffer.buffer->isCreated())
		platformBuffer.buffer->create();
	platformBuffer.buffer->bind();
	platformBuffer.buffer->allocate(platformBuffer.stl, platformBuffer.size * 2 * 3 * sizeof(GLfloat));
	platformBuffer.buffer->release();
}

void GlWidget::BindPlatform()
{
	InitModelMatrix();

	m_program->setUniformValue(func, 2);
	m_program->enableAttributeArray(0);
	
	platformBuffer.buffer->bind();
	m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));
	platformBuffer.buffer->release();

	glDrawArrays(GL_LINES, 0, platformBuffer.size * 2);

	m_program->disableAttributeArray(0);
}

void GlWidget::wheelEvent(QWheelEvent *event)
{
	int numDegrees = event->delta() / 8;//滚动的角度，*8就是鼠标滚动的距离
	int numSteps = numDegrees / 15;//滚动的步数，*15就是鼠标滚动的角度

	if (_scale > 1) {
		if (_scale > 10)
			_scale = _scale + numSteps * 3;
		else
			_scale = _scale + numSteps * 0.8;
	}
	else {
		_scale = _scale + numSteps*0.08;
	}
	
	if (_scale > 500)
		_scale = 500;
	if (_scale < 0.1)
		_scale = 0.1;

	update();
}

void GlWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::RightButton)
	{
		int x = event->pos().x() - xLastPos;
		int y = event->pos().y() - yLastPos;

		_horizonAngle += x / 5;
		_verticalAngle += y / 5;

		if (_horizonAngle > 360)
			_horizonAngle = 0;
		if (_horizonAngle < -360)
			_horizonAngle = 0;

		if (_verticalAngle > 89.99)
			_verticalAngle = 89.99;
		else if (_verticalAngle < -89.99)
			_verticalAngle = -89.99;

		xLastPos = event->pos().x();
		yLastPos = event->pos().y();

		SetEye();
		update();
	}
	else if (event->buttons() & Qt::MiddleButton)
	{
		Pointf3 p1(xLastPos, yLastPos, 0);
		Pointf3 p2(event->pos().x(), event->pos().y(), 0);
		Pointf3 p3, p4;

		GLint viewport[4];
		GLfloat* mvmatrix;
		GLfloat* projmatrix;

		GLdouble mv[16];
		GLdouble pro[16];

		glGetIntegerv(GL_VIEWPORT, viewport);   /* 获取三个矩阵 */

		QMatrix4x4 m = m_camera * m_world;
		mvmatrix = m.data();
		projmatrix = m_proj.data();

		for (int i = 0; i < 16; ++i) {
			mv[i] = (double)mvmatrix[i];
			pro[i] = (double)projmatrix[i];
		}
		gluUnProject(p1.x, viewport[3] - p1.y, p1.z, mv, pro, viewport, &p3.x, &p3.y, &p3.z); /* 获取三维坐标 */
		gluUnProject(p2.x, viewport[3] - p2.y, p2.z, mv, pro, viewport, &p4.x, &p4.y, &p4.z);

		//只确定了世界坐标平移的三个方向  正交投影时3000比例因子不需要
		if (this->viewport == PRESPRECTIVE)
		{
			center.setX(center.x() + (p4.x - p3.x) * 3000);
			center.setY(center.y() + (p4.y - p3.y) * 3000);
			center.setZ(center.z() + (p4.z - p3.z) * 3000);
		}
		else
		{
			center.setX(center.x() + (p4.x - p3.x));
			center.setY(center.y() + (p4.y - p3.y));
			center.setZ(center.z() + (p4.z - p3.z));
		}

		update();

		xLastPos = event->pos().x();
		yLastPos = event->pos().y();
	}

	if (m_supEditControl != nullptr && event->buttons() == Qt::LeftButton) {
		_pos.setX(event->pos().x());
		_pos.setY(event->pos().y());
		_Depth = true;
		update();
	}

	if (m_supEditControl == nullptr && event->buttons() == Qt::LeftButton && translationID != TranInvalidID) {

		int x = event->pos().x() - xLastPos;
		int y = yLastPos - event->pos().y();

		float _h, x_ratio, y_ratio;
		if (translationID == translateMesh_X.id) {
			_h = _horizonAngle - 90;
			x_ratio = cosf((_h / 180.0) * PI);
			y_ratio = sinf((_h / 180.0) * PI);
			OffsetValueChange((x * x_ratio + y * y_ratio) / _scale, 0, 0, true);
		}
		else if (translationID == translateMesh_Y.id) {
			_h = _horizonAngle - 180;
			x_ratio = cosf((_h / 180.0) * PI);
			y_ratio = sinf((_h / 180.0) * PI);
			OffsetValueChange(0, -(x * x_ratio + y * y_ratio) / _scale, 0, true);
		}
		else if (translationID == translateMesh_Z.id)
			OffsetValueChange(0, 0, y / _scale, true);
		else if (translationID == scaleMesh_X.id) {
			_h = _horizonAngle + 90;
			x_ratio = cosf((_h / 180.0) * PI);
			y_ratio = sinf((_h / 180.0) * PI);
			ScaleValueChange((x * x_ratio + y * y_ratio) / (_scale * 100), 0, 0, true);
		}
		else if (translationID == scaleMesh_Y.id) {
			_h = _horizonAngle + 180;
			x_ratio = cosf((_h / 180.0) * PI);
			y_ratio = sinf((_h / 180.0) * PI);
			ScaleValueChange(0, -(x * x_ratio + y * y_ratio) / (_scale * 100), 0, true);
		}
		else if (translationID == scaleMesh_Z.id) {
			ScaleValueChange(0, 0, -y / (_scale * 100), true);
		}
		else if (translationID == rotateMesh_Z.id || translationID == rotateMesh_X.id || translationID == rotateMesh_Y.id)
		{
			Pointf3 p;

			GLint viewport[4];
			GLfloat* mvmatrix;
			GLfloat* projmatrix;

			GLdouble mv[16];
			GLdouble pro[16];

			glGetIntegerv(GL_VIEWPORT, viewport);   /* 获取三个矩阵 */

			QMatrix4x4 m = m_camera * m_world;
			mvmatrix = m.data();
			projmatrix = m_proj.data();

			for (int i = 0; i < 16; ++i) {
				mv[i] = (double)mvmatrix[i];
				pro[i] = (double)projmatrix[i];
			}
			gluProject(m_selInstance->origin.x
				, m_selInstance->origin.y
				, m_selInstance->origin.z
				, mv, pro, viewport
				, &p.x, &p.y, &p.z);

			Vectorf v1(event->pos().x() - p.x, event->pos().y() - p.y);
			Vectorf v2(xLastPos - p.x, yLastPos - p.y);
			double angle1 = vector_angle_2(v1, Vectorf(1, 0)) / PI * 180;
			double angle2 = vector_angle_2(Vectorf(1, 0), v2) / PI * 180;

			if (v1.y < 0)
				angle1 = 360 - angle1;
			if (v2.y < 0)
				angle2 = 360 - angle2;

			if (translationID == rotateMesh_Z.id) {
				if (_verticalAngle > 0)
					RotateValueChange(angle2 - angle1, 0, 0, 1, true);
				else
					RotateValueChange(angle1 - angle2, 0, 0, 1, true);
			}
			else if (translationID == rotateMesh_X.id) {
				if ((_horizonAngle >= 0 && _horizonAngle < 180) || (_horizonAngle <= -180 && _horizonAngle > -360))
					RotateValueChange(angle1 - angle2, 0, 1, 0, true);
				else
					RotateValueChange(angle2 - angle1, 0, 1, 0, true);
			}
			else if (translationID == rotateMesh_Y.id) {
				if ((_horizonAngle >= 90 && _horizonAngle < 270) || (_horizonAngle <= -90 && _horizonAngle > -270))
					RotateValueChange(angle1 - angle2, 1, 0, 0, true);
				else
					RotateValueChange(angle2 - angle1, 1, 0, 0, true);
			}
		}

		xLastPos = event->pos().x();
		yLastPos = event->pos().y();

		update();
	}
}

void GlWidget::ReadDepth()
{
	if (_Depth) {
		GLint viewport[4];
		GLfloat winx, winy, winz = 0;
		glGetIntegerv(GL_VIEWPORT, viewport);

		winx = _pos.x();
		winy = viewport[3] - _pos.y();

		GLdouble mvmatrix[16], projmatrix[16];
		GLdouble prox = 0, proy = 0, proz = 0;
		glPushMatrix();

		if (this->viewport == PRESPRECTIVE)
			gluPerspective(45.0, (GLfloat)viewport[2] / (GLfloat)viewport[3], 0.1f, 10000.0f);
		else
			glOrtho(-(GLfloat)viewport[2] / 2, (GLfloat)viewport[2] / 2, -(GLfloat)viewport[3] / 2, (GLfloat)viewport[3] / 2, -10000.0f, 10000.0f);

		gluLookAt(eye[0], eye[1], eye[2], 0, 0, 0, 0, 0, 1);
		glScalef(_scale, _scale, _scale);
		glTranslatef(0, 0, -130);
		glTranslatef(center.x(), center.y(), center.z());

		/* 获取三个矩阵 */
		glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
		glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);

		glPopMatrix();
		glReadPixels((int)winx, (int)winy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winz);
		gluUnProject((GLdouble)winx, (GLdouble)winy, (GLdouble)winz, mvmatrix, projmatrix, viewport, &prox, &proy, &proz);

		if (winz != 1) {
			stl_facet f, face;
			float v[3];
			float dis = -1;

			for (int i = 0; i < m_supEditControl->m_mesh->stl.stats.number_of_facets; ++i) {
				f = m_supEditControl->m_mesh->stl.facet_start[i];

				//得到射线
				v[0] = prox - eye.x();
				v[1] = proy - eye.y();
				v[2] = proz - eye.z();
				stl_normalize_vector(v);

				Pointf3 a;
				if (Ray2TrianglePoint(f, Vectorf3(v[0], v[1], v[2]), Pointf3(eye.x(), eye.y(), eye.z()), a)) {
					float temp = DisPoint3(a, Pointf3(eye.x(), eye.y(), eye.z()));
					if (dis >= 0) {
						if (temp < dis) {
							m_supEditControl->m_curPoint->new_origin = a;
							face = f;
							dis = temp;
						}
					}
					else {
						m_supEditControl->m_curPoint->new_origin = a;
						face = f;
						dis = temp;
					}
				}
			}
			if (aormal_xy_angle(face.normal) < 0)
				m_supEditControl->m_curPoint->new_origin = Pointf3{ 0,0,0 };
		}
		else
			m_supEditControl->m_curPoint->new_origin = Pointf3{ 0,0,0 };

		m_supEditControl->InitOneSupport();

		_Depth = false;
		update();
	}
}

void GlWidget::mousePressEvent(QMouseEvent* event)
{
	xLastPos = event->pos().x();
	yLastPos = event->pos().y();

	if (event->buttons() == Qt::LeftButton && selectID > SelInvalidID && m_supEditControl == nullptr) {
		GLint x = event->x();
		GLint y = event->y();

		const int BUFSIZE = 1024;
		GLuint selectBuf[BUFSIZE] = { 0,0,0,0 };
		GLint hits;
		GLint viewport[4];

		glGetIntegerv(GL_VIEWPORT, viewport);
		glSelectBuffer(BUFSIZE, selectBuf);

		glRenderMode(GL_SELECT);
		glInitNames();
		glPushName(-1);

		glPushMatrix();
		glMatrixMode(GL_PROJECTION);  // 1: GL_SELECT下投影变换
		glLoadIdentity();
		gluPickMatrix(x, viewport[3] - y, 3, 3, viewport);

		if (this->viewport == PRESPRECTIVE)
			gluPerspective(45.0, (GLfloat)viewport[2] / (GLfloat)viewport[3], 0.1f, 10000.0f);
		else
			glOrtho(-(GLfloat)viewport[2] / 2, (GLfloat)viewport[2] / 2, -(GLfloat)viewport[3] / 2, (GLfloat)viewport[3] / 2, -10000.0f, 10000.0f);

		glMatrixMode(GL_MODELVIEW);    // 2: GL_SELECT下模型视图变换
		glLoadIdentity();

		gluLookAt(eye[0], eye[1], eye[2], 0, 0, 0, 0, 0, 1);
		glScalef(_scale, _scale, _scale);
		glTranslatef(0, 0, -130);
		glTranslatef(center.x(), center.y(), center.z());

		//得到模型实例
		ModelInstance* i = m_selInstance;

		//半径
		double radius = sqrtf(((i->box.max.x - i->box.min.x) / 2) * ((i->box.max.x - i->box.min.x) / 2) +
			((i->box.max.y - i->box.min.y) / 2) * ((i->box.max.y - i->box.min.y) / 2) +
			((i->box.max.z - i->box.min.z) / 2) * ((i->box.max.z - i->box.min.z) / 2));

		QMatrix4x4 scaleM, translateM;
		scaleM.setToIdentity();
		translateM.setToIdentity();

		double s = radius / 10 * 0.8;

		//rotate
		scaleM.scale(s);
		translateM.translate(i->origin.x, i->origin.y, i->origin.z);
		RenderTranMeshCase(ring, rotateMesh_X.id, Vectorf3(90, 0, 0), scaleM, translateM);

		RenderTranMeshCase(ring, rotateMesh_Y.id, Vectorf3(0, 90, 0), scaleM, translateM);

		RenderTranMeshCase(ring, rotateMesh_Z.id, Vectorf3(0, 0, 0), scaleM, translateM);

		TriangleMesh m(ring);
		m.scale(s);
		double x1 = (m.stl.stats.max.x - m.stl.stats.min.x) / 2 * 1.01;

		s *= 2;
		//scale
		scaleM.setToIdentity();
		scaleM.scale(s);
		translateM.setToIdentity();
		translateM.translate(i->origin.x - x1, i->origin.y, i->origin.z);
		RenderTranMeshCase(cube, scaleMesh_X.id, Vectorf3(0, -90, 0), scaleM, translateM);

		scaleM.setToIdentity();
		scaleM.scale(s);
		translateM.setToIdentity();
		translateM.translate(i->origin.x, i->origin.y + x1, i->origin.z);
		RenderTranMeshCase(cube, scaleMesh_Y.id, Vectorf3(-90, 0, 0), scaleM, translateM);

		scaleM.setToIdentity();
		scaleM.scale(s);
		translateM.setToIdentity();
		translateM.translate(i->origin.x, i->origin.y, i->origin.z - x1);
		RenderTranMeshCase(cube, scaleMesh_Z.id, Vectorf3(180, 0, 0), scaleM, translateM);

		//translate
		scaleM.setToIdentity();
		scaleM.scale(s);
		translateM.setToIdentity();
		translateM.translate(i->origin.x + x1, i->origin.y, i->origin.z);
		RenderTranMeshCase(cone, translateMesh_X.id, Vectorf3(0, 90, 0), scaleM, translateM);

		scaleM.setToIdentity();
		scaleM.scale(s);
		translateM.setToIdentity();
		translateM.translate(i->origin.x, i->origin.y - x1, i->origin.z);
		RenderTranMeshCase(cone, translateMesh_Y.id, Vectorf3(90, 0, 0), scaleM, translateM);

		scaleM.setToIdentity();
		scaleM.scale(s);
		translateM.setToIdentity();
		translateM.translate(i->origin.x, i->origin.y, i->origin.z + x1);
		RenderTranMeshCase(cone, translateMesh_Z.id, Vectorf3(0, 0, 0), scaleM, translateM);


		glPopMatrix();

		// 切换回GL_RENDER模式
		hits = glRenderMode(GL_RENDER); // 返回hit为选取的个数

		if (hits) {
			int id = selectBuf[3];
			GLuint z = selectBuf[1];
			for (int i = 0; i < hits; ++i) {
				if (z > selectBuf[i * 4 + 1]) {
					z = selectBuf[i * 4 + 1];
					id = selectBuf[i * 4 + 3];
				}
			}
			if (translationID != id) {
				translationID = id;
				//mainWindow->modelSelect();
			}
		}
		update();
	}

	if (m_supEditControl != nullptr && event->buttons() == Qt::LeftButton) {
		GLint x = event->x();
		GLint y = event->y();

		const int BUFSIZE = 1024;
		GLuint selectBuf[BUFSIZE] = { 0,0,0,0 };
		GLint hits;
		GLint viewport[4];

		glGetIntegerv(GL_VIEWPORT, viewport);
		glSelectBuffer(BUFSIZE, selectBuf);

		glRenderMode(GL_SELECT);
		glInitNames();
		glPushName(-1);

		glPushMatrix();
		glMatrixMode(GL_PROJECTION);  // 1: GL_SELECT下投影变换
		glLoadIdentity();
		gluPickMatrix(x, viewport[3] - y, 5, 5, viewport);

		if (this->viewport == PRESPRECTIVE)
			gluPerspective(45.0, (GLfloat)viewport[2] / (GLfloat)viewport[3], 0.1f, 10000.0f);
		else
			glOrtho(-(GLfloat)viewport[2] / 2, (GLfloat)viewport[2] / 2, -(GLfloat)viewport[3] / 2, (GLfloat)viewport[3] / 2, -10000.0f, 10000.0f);

		glMatrixMode(GL_MODELVIEW);    // 2: GL_SELECT下模型视图变换
		glLoadIdentity();

		gluLookAt(eye[0], eye[1], eye[2], 0, 0, 0, 0, 0, 1);
		glScalef(_scale, _scale, _scale);
		glTranslatef(0, 0, -130);
		glTranslatef(center.x(), center.y(), center.z());

		TriangleMesh mesh;
		stl_facet f;

		//?????????
		for (auto p = m_supEditControl->supportPoints.begin(); p != m_supEditControl->supportPoints.end(); ++p) {
			mesh = ball;
			mesh.translate((*p)->origin.x, (*p)->origin.y, (*p)->origin.z);

			//采用流水线方式在选中模式中渲染支撑点
			glLoadName(std::distance(m_supEditControl->supportPoints.begin(), p));

			glBegin(GL_TRIANGLES);

			for (int i = 0; i < mesh.stl.stats.number_of_facets; ++i) {
				f = mesh.stl.facet_start[i];
				glVertex3f(f.vertex[0].x, f.vertex[0].y, f.vertex[0].z);
				glVertex3f(f.vertex[1].x, f.vertex[1].y, f.vertex[1].z);
				glVertex3f(f.vertex[2].x, f.vertex[2].y, f.vertex[2].z);
			}
			glEnd();
		}

		glPopMatrix();

		// 切换回GL_RENDER模式
		hits = glRenderMode(GL_RENDER); // 返回hit为选取的个数

		if (hits) {
			int id = selectBuf[3];
			GLuint z = selectBuf[1];
			for (int i = 0; i < hits; ++i) {
				if (z > selectBuf[i * 4 + 1]) {
					z = selectBuf[i * 4 + 1];
					id = selectBuf[i * 4 + 3];
				}
			}

			if (id >= 0) {
				qDebug() << "delete oneSupport: " << id;
				//删除一个支撑点（支撑点的位置关系生成id）并初始化渲染
				m_supEditControl->DelSupportPoint(id);
			}
		}
		update();
	}
}

void GlWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (translationID != TranInvalidID) {
		translationID = TranInvalidID;
		update();
	}
}

void GlWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (m_supEditControl == nullptr && event->buttons() == Qt::LeftButton) {
		GLint x = event->x();
		GLint y = event->y();

		const int BUFSIZE = 1024;
		GLuint selectBuf[BUFSIZE] = { 0,0,0,0 };
		GLint hits;
		GLint viewport[4];

		glGetIntegerv(GL_VIEWPORT, viewport);
		glSelectBuffer(BUFSIZE, selectBuf);

		glRenderMode(GL_SELECT);
		glInitNames();
		glPushName(-1);

		glPushMatrix();
		glMatrixMode(GL_PROJECTION);  // 1: GL_SELECT下投影变换
		glLoadIdentity();
		gluPickMatrix(x, viewport[3] - y, 3, 3, viewport);

		if (this->viewport == PRESPRECTIVE)
			gluPerspective(45.0, (GLfloat)viewport[2] / (GLfloat)viewport[3], 0.1f, 10000.0f);
		else
			glOrtho(-(GLfloat)viewport[2] / 2, (GLfloat)viewport[2] / 2, -(GLfloat)viewport[3] / 2, (GLfloat)viewport[3] / 2, -10000.0f, 10000.0f);

		glMatrixMode(GL_MODELVIEW);    // 2: GL_SELECT下模型视图变换
		glLoadIdentity();

		gluLookAt(eye[0], eye[1], eye[2], 0, 0, 0, 0, 0, 1);
		glScalef(_scale, _scale, _scale);
		glTranslatef(0, 0, -130);
		glTranslatef(center.x(), center.y(), center.z());

		stl_facet f;
		for (auto o = m_model->objects.begin(); o != m_model->objects.end(); ++o) {
			for (auto i = (*o)->instances.begin(); i != (*o)->instances.end(); ++i) {
				glLoadName(m_model->find_id(*i));
				TriangleMesh mesh = (*o)->volumes[0]->mesh;
				(*i)->transform_mesh(&mesh);
				glBegin(GL_TRIANGLES);
				for (int i = 0; i < mesh.stl.stats.number_of_facets; ++i) {
					f = mesh.stl.facet_start[i];
					glVertex3f(f.vertex[0].x, f.vertex[0].y, f.vertex[0].z);
					glVertex3f(f.vertex[1].x, f.vertex[1].y, f.vertex[1].z);
					glVertex3f(f.vertex[2].x, f.vertex[2].y, f.vertex[2].z);
				}
				glEnd();

			}
		}

		glPopMatrix();

		// 切换回GL_RENDER模式
		hits = glRenderMode(GL_RENDER); // 返回hit为选取的个数

		if (hits) {
			int id = selectBuf[3];
			GLuint z = selectBuf[1];
			for (int i = 0; i < hits; ++i) {
				if (z > selectBuf[i * 4 + 1]) {
					z = selectBuf[i * 4 + 1];
					id = selectBuf[i * 4 + 3];
				}
			}
			if (selectID != id) {
				selectID = id;
				m_selInstance = m_model->find_instance(selectID);
				emit sig_modelSelect();
			}
		}
		else {
			selectID = SelInvalidID;
			m_selInstance = nullptr;
		}
		update();
	}
}

void GlWidget::SetEye()
{
	eye.setX(500.0 * cos(PI * _verticalAngle / 180.0) * cos(PI * _horizonAngle / 180.0));
	eye.setY(-500.0 * cos(PI * _verticalAngle / 180.0) * sin(PI * _horizonAngle / 180.0));
	eye.setZ(500.0 * sin(PI * _verticalAngle / 180.0));
}

void GlWidget::ChangeView(VIEW view)
{
	switch (view)
	{
	case DEFAULT:
		SetDefaultView();
		SetEye();
		update();
		break;

	case OVERLOOK:
		_scale = 1.2;
		_horizonAngle = 90; 
		_verticalAngle = 89.99;
		SetEye();
		update();
		break;

	case LEFT:
		_scale = 1.2;
		_horizonAngle = 180;
		_verticalAngle = 0;
		SetEye();
		update();
		break;

	case RIGHT:
		_scale = 1.2;
		_horizonAngle = 0;
		_verticalAngle = 0;
		SetEye();
		update();
		break;

	case FRONT:
		_scale = 1.2;
		_horizonAngle = 90; 
		_verticalAngle = 0;
		SetEye();
		update();
		break;

	case BEHIND:
		_scale = 1.2;
		_horizonAngle = -90; 
		_verticalAngle = 0;
		SetEye();
		update();
		break;

	default:
		break;
	}
}

void GlWidget::InitTreeSupportID(size_t id, QProgressBar* progress)
{
	TreeSupport* s = m_dlprint->GetTreeSupport(id);
	if (s == nullptr)
		return;

	double height = m_dlprint->m_config->support_top_height;//写死的顶端与底端的高度

	Pointfs circle = CirclePoints(15);

	Pointf3s _topCircle, _supportCircle, branchCircle, _bottomCircle;//树枝的圆坐标

	Pointf3 c4, c5, c6, c7;
	for (auto c = circle.begin(); c != circle.end(); ++c) {
		c4.x = (*c).x; c4.y = (*c).y; c4.z = 0;
		c5.x = (*c).x; c5.y = (*c).y; c5.z = 0;
		c6.x = (*c).x; c6.y = (*c).y; c6.z = 0;
		c7.x = (*c).x; c7.y = (*c).y; c7.z = 0;

		c4.scale(m_dlprint->m_config->support_radius);
		c5.scale(m_dlprint->m_config->support_top_radius);
		c6.scale(m_dlprint->m_config->support_radius);
		c7.scale(m_dlprint->m_config->support_bottom_radius);

		//向x轴正坐标方向（1,0,0）旋转45度
		c4.rotate_y(45);

		_supportCircle.push_back(c6);
		_topCircle.push_back(c5);
		branchCircle.push_back(c4);
		_bottomCircle.push_back(c7);
	}

	TriangleMesh mesh;
	Pointf3s topCircle3, supportCircle3A, supportCircle3B, bottomCircle3;
	Pointf3 p4, p5, p6, p7;
	Linef3 line, line1;
	size_t len2;


	SupportBuffer* sb = new SupportBuffer;
	sb->id = id;
	//树干 76->80
	for (auto i = s->tree_support_bole.begin(); i != s->tree_support_bole.end(); ++i) {
		if (progress != nullptr) {
			size_t id = std::distance(s->tree_support_bole.begin(), i);
			int unit = (id + 1) / (s->tree_support_bole.size() / (80 - 76) + 1);
			if (progress->value() < 76 + unit)
				progress->setValue(76 + unit);
		}

		line = *i;

		//------------------圆坐标旋转-----------------------
		float m[3];
		m[0] = line.b.x - line.a.x;
		m[1] = line.b.y - line.a.y;
		m[2] = line.b.z - line.a.z;
		stl_normalize_vector(m);

		//与z轴正方向所成的角度,在y轴方向上旋转
		double z_angle = vector_angle_3(Vectorf3(m[0], m[1], m[2]), Vectorf3(0, 0, 1));

		//与x轴正方向所成的角度，在z轴方向上旋转
		double x_angle = atan2(m[1], m[0]) - atan2(0, -1);
		//qDebug() << "z:   " << acos(z_angle) / PI * 180 << "x:   " << x_angle / PI * 180;
		supportCircle3A = _supportCircle;
		supportCircle3B = _supportCircle;

		for (auto t = supportCircle3A.begin(); t != supportCircle3A.end(); ++t) {
			(*t).rotate_y_radian(z_angle);
			(*t).rotate_z_radian(x_angle);
			(*t).translate(line.b.x, line.b.y, line.b.z);
		}

		for (auto s = supportCircle3B.begin(); s != supportCircle3B.end(); ++s) {
			(*s).rotate_y_radian(z_angle);
			(*s).rotate_z_radian(x_angle);
			(*s).translate(line.a.x, line.a.y, line.a.z);
		}
		//---------------------------------------------------

		for (auto l = supportCircle3A.begin(); l != supportCircle3A.end() - 1; ++l) {
			len2 = std::distance(supportCircle3A.begin(), l);//第几个面单元
			p4 = *l;
			p5 = *(l + 1);

			p6 = *(supportCircle3B.begin() + len2);
			p7 = *(supportCircle3B.begin() + len2 + 1);

			//低端三角面片
			stl_add_face(p6.x, p6.y, p6.z, p7.x, p7.y, p7.z, line.a.x, line.a.y, line.a.z, mesh);

			//顶端三角面片
			stl_add_face(p5.x, p5.y, p5.z, p4.x, p4.y, p4.z, line.b.x, line.b.y, line.b.z, mesh);

			//中间三角面片
			stl_add_face(p7.x, p7.y, p7.z, p6.x, p6.y, p6.z, p4.x, p4.y, p4.z, mesh);

			stl_add_face(p7.x, p7.y, p7.z, p4.x, p4.y, p4.z, p5.x, p5.y, p5.z, mesh);
		}
	}

	//树叶
	for (auto i = s->tree_support_leaf.begin(); i != s->tree_support_leaf.end(); ++i) {
		//81-85
		if (progress != nullptr) {
			size_t id = std::distance(s->tree_support_leaf.begin(), i);
			int unit = (id + 1) / (s->tree_support_leaf.size() / (85 - 81) + 1);
			if (progress->value() < 81 + unit)
				progress->setValue(81 + unit);
		}

		line = *i;
		//------------------圆坐标旋转-----------------------
		float m[3];
		m[0] = line.b.x - line.a.x;
		m[1] = line.b.y - line.a.y;
		m[2] = line.b.z - line.a.z;
		stl_normalize_vector(m);

		//与z轴正方向所成的角度,在y轴方向上旋转
		double z_angle = vector_angle_3(Vectorf3(m[0], m[1], m[2]), Vectorf3(0, 0, 1));

		//与x轴正方向所成的角度，在z轴方向上旋转
		double x_angle = atan2(m[1], m[0]) - atan2(0, -1);
		//qDebug() << "z:   " << acos(z_angle) / PI * 180 << "x:   " << x_angle / PI * 180;
		topCircle3 = _topCircle;
		supportCircle3A = _supportCircle;

		for (auto t = topCircle3.begin(); t != topCircle3.end(); ++t) {
			(*t).rotate_y_radian(z_angle);
			(*t).rotate_z_radian(x_angle);
			(*t).translate(line.b.x, line.b.y, line.b.z);
		}

		for (auto s = supportCircle3A.begin(); s != supportCircle3A.end(); ++s) {
			(*s).rotate_y_radian(z_angle);
			(*s).rotate_z_radian(x_angle);
			(*s).translate(line.a.x, line.a.y, line.a.z);
		}
		//---------------------------------------------------

		for (auto l = topCircle3.begin(); l != topCircle3.end() - 1; ++l) {
			len2 = std::distance(topCircle3.begin(), l);//第几个面单元
			p4 = *l;
			p5 = *(l + 1);

			p6 = *(supportCircle3A.begin() + len2);
			p7 = *(supportCircle3A.begin() + len2 + 1);

			//低端三角面片
			stl_add_face(p6.x, p6.y, p6.z, p7.x, p7.y, p7.z, line.a.x, line.a.y, line.a.z, mesh);

			//顶端三角面片
			stl_add_face(p5.x, p5.y, p5.z, p4.x, p4.y, p4.z, line.b.x, line.b.y, line.b.z, mesh);

			//中间三角面片
			stl_add_face(p7.x, p7.y, p7.z, p6.x, p6.y, p6.z, p4.x, p4.y, p4.z, mesh);

			stl_add_face(p7.x, p7.y, p7.z, p4.x, p4.y, p4.z, p5.x, p5.y, p5.z, mesh);
		}

	}

	//树枝
	for (auto i = s->tree_support_branch.begin(); i != s->tree_support_branch.end(); ++i) {
		//86-90
		if (progress != nullptr) {
			size_t id = std::distance(s->tree_support_branch.begin(), i);
			int unit = (id + 1) / (s->tree_support_branch.size() / (86 - 90) + 1);
			if (progress->value() < 86 + unit)
				progress->setValue(86 + unit);
		}

		line = *i;

		//-----------求树枝在z轴方向旋转的角度值并旋转---------
		float m[3];
		m[0] = line.b.x - line.a.x;
		m[1] = line.b.y - line.a.y;
		m[2] = 0;
		stl_normalize_vector(m);

		//将圆坐标绕z轴旋转至支撑线的方向
		double angle = atan2(m[1], m[0]) - atan2(0, -1);

		Pointf3s _branchCircle = branchCircle;
		for (auto b = _branchCircle.begin(); b != _branchCircle.end(); ++b)
			(*b).rotate_z_radian(angle);
		//--------------------------------------------------------

		for (auto a = _branchCircle.begin(); a != _branchCircle.end() - 1; ++a) {
			p4 = *a;
			p5 = *(a + 1);

			p6 = *a;
			p7 = *(a + 1);

			p4.translate(line.a.x, line.a.y, line.a.z);
			p5.translate(line.a.x, line.a.y, line.a.z);

			p6.translate(line.b.x, line.b.y, line.b.z);
			p7.translate(line.b.x, line.b.y, line.b.z);

			//低端三角面片
			stl_add_face(p4.x, p4.y, p4.z, p5.x, p5.y, p5.z, line.a.x, line.a.y, line.a.z, mesh);

			//顶端三角面片
			stl_add_face(p7.x, p7.y, p7.z, p6.x, p6.y, p6.z, line.b.x, line.b.y, line.b.z, mesh);

			//中间三角面片
			stl_add_face(p5.x, p5.y, p5.z, p4.x, p4.y, p4.z, p6.x, p6.y, p6.z, mesh);

			stl_add_face(p6.x, p6.y, p6.z, p7.x, p7.y, p7.z, p5.x, p5.y, p5.z, mesh);
		}

	}

	//树根
	for (auto i = s->tree_support_bottom.begin(); i != s->tree_support_bottom.end(); ++i) {
		//91-95
		if (progress != nullptr) {
			size_t id = std::distance(s->tree_support_bottom.begin(), i);
			int unit = (id + 1) / (s->tree_support_bottom.size() / (95 - 91) + 1);
			if (progress->value() < 91 + unit)
				progress->setValue(91 + unit);
		}

		line = *i;
		if (line.a.z == 0) {
			//------------------圆坐标旋转-----------------------
			supportCircle3A = _supportCircle;
			bottomCircle3 = _bottomCircle;

			for (auto t = bottomCircle3.begin(); t != bottomCircle3.end(); ++t)
				(*t).translate(line.a.x, line.a.y, line.a.z);

			for (auto s = supportCircle3A.begin(); s != supportCircle3A.end(); ++s)
				(*s).translate(line.b.x, line.b.y, line.b.z);
			//---------------------------------------------------

			for (auto l = supportCircle3A.begin(); l != supportCircle3A.end() - 1; ++l) {
				len2 = std::distance(supportCircle3A.begin(), l);//第几个面单元
				p4 = *l;
				p5 = *(l + 1);

				p6 = *(bottomCircle3.begin() + len2);
				p7 = *(bottomCircle3.begin() + len2 + 1);

				//低端三角面片
				stl_add_face(p6.x, p6.y, p6.z, p7.x, p7.y, p7.z, line.a.x, line.a.y, line.a.z, mesh);

				//顶端三角面片
				stl_add_face(p5.x, p5.y, p5.z, p4.x, p4.y, p4.z, line.b.x, line.b.y, line.b.z, mesh);

				//中间三角面片
				stl_add_face(p7.x, p7.y, p7.z, p6.x, p6.y, p6.z, p4.x, p4.y, p4.z, mesh);

				stl_add_face(p7.x, p7.y, p7.z, p4.x, p4.y, p4.z, p5.x, p5.y, p5.z, mesh);
			}
		}
		else {
			line = *i;
			//------------------圆坐标旋转-----------------------
			float m[3];
			m[0] = line.b.x - line.a.x;
			m[1] = line.b.y - line.a.y;
			m[2] = line.b.z - line.a.z;
			stl_normalize_vector(m);

			//与z轴正方向所成的角度,在y轴方向上旋转
			double z_angle = vector_angle_3(Vectorf3(m[0], m[1], m[2]), Vectorf3(0, 0, 1));

			//与x轴正方向所成的角度，在z轴方向上旋转
			double x_angle = atan2(m[1], m[0]) - atan2(0, -1);
			//qDebug() << "z:   " << acos(z_angle) / PI * 180 << "x:   " << x_angle / PI * 180;
			topCircle3 = _topCircle;
			supportCircle3A = _supportCircle;

			for (auto t = topCircle3.begin(); t != topCircle3.end(); ++t) {
				(*t).rotate_y_radian(z_angle);
				(*t).rotate_z_radian(x_angle);
				(*t).translate(line.a.x, line.a.y, line.a.z);
			}

			for (auto s = supportCircle3A.begin(); s != supportCircle3A.end(); ++s) {
				(*s).rotate_y_radian(z_angle);
				(*s).rotate_z_radian(x_angle);
				(*s).translate(line.b.x, line.b.y, line.b.z);
			}
			//---------------------------------------------------

			for (auto l = supportCircle3A.begin(); l != supportCircle3A.end() - 1; ++l) {
				len2 = std::distance(supportCircle3A.begin(), l);//第几个面单元
				p4 = *l;
				p5 = *(l + 1);

				p6 = *(topCircle3.begin() + len2);
				p7 = *(topCircle3.begin() + len2 + 1);

				//低端三角面片
				stl_add_face(p6.x, p6.y, p6.z, p7.x, p7.y, p7.z, line.a.x, line.a.y, line.a.z, mesh);

				//顶端三角面片
				stl_add_face(p5.x, p5.y, p5.z, p4.x, p4.y, p4.z, line.b.x, line.b.y, line.b.z, mesh);

				//中间三角面片
				stl_add_face(p7.x, p7.y, p7.z, p6.x, p6.y, p6.z, p4.x, p4.y, p4.z, mesh);

				stl_add_face(p7.x, p7.y, p7.z, p4.x, p4.y, p4.z, p5.x, p5.y, p5.z, mesh);
			}
		}

	}

	//加强节点
	for (auto p = s->tree_support_node.begin(); p != s->tree_support_node.end(); ++p) {
		if (progress != nullptr) {
			size_t id = std::distance(s->tree_support_node.begin(), p);
			int unit = (id + 1) / (s->tree_support_node.size() / (100 - 95) + 1);
			if (progress->value() < 95 + unit)
				progress->setValue(95 + unit);
		}

		TriangleMesh mesh1 = ball;
		mesh1.scale(m_dlprint->m_config->support_radius);
		mesh1.translate((*p).x, (*p).y, (*p).z);
		mesh.merge(mesh1);
	}


	sb->size = mesh.stl.stats.number_of_facets;
	sb->stl = new GLfloat[sb->size * 3 * 2 * 3];

	float normal[3];

	for (int i = 0; i < sb->size; ++i) {
		stl_calculate_normal(normal, &mesh.stl.facet_start[i]);
		stl_normalize_vector(normal);

		sb->stl[i * 18] = mesh.stl.facet_start[i].vertex[0].x;
		sb->stl[i * 18 + 1] = mesh.stl.facet_start[i].vertex[0].y;
		sb->stl[i * 18 + 2] = mesh.stl.facet_start[i].vertex[0].z;
		sb->stl[i * 18 + 3] = normal[0];
		sb->stl[i * 18 + 4] = normal[1];
		sb->stl[i * 18 + 5] = normal[2];

		sb->stl[i * 18 + 6] = mesh.stl.facet_start[i].vertex[1].x;
		sb->stl[i * 18 + 7] = mesh.stl.facet_start[i].vertex[1].y;
		sb->stl[i * 18 + 8] = mesh.stl.facet_start[i].vertex[1].z;
		sb->stl[i * 18 + 9] = normal[0];
		sb->stl[i * 18 + 10] = normal[1];
		sb->stl[i * 18 + 11] = normal[2];

		sb->stl[i * 18 + 12] = mesh.stl.facet_start[i].vertex[2].x;
		sb->stl[i * 18 + 13] = mesh.stl.facet_start[i].vertex[2].y;
		sb->stl[i * 18 + 14] = mesh.stl.facet_start[i].vertex[2].z;
		sb->stl[i * 18 + 15] = normal[0];
		sb->stl[i * 18 + 16] = normal[1];
		sb->stl[i * 18 + 17] = normal[2];
	}

	sb->buffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	sb->buffer->create();
	sb->buffer->bind();
	sb->buffer->allocate(sb->stl, sb->size * 3 * 2 * 3 * sizeof(GLfloat));
	sb->buffer->release();

	if (!treeSupportBuffers.empty()) {
		for (auto sb = treeSupportBuffers.begin(); sb != treeSupportBuffers.end(); ++sb) {
			if ((*sb)->id == id) {
				delete* sb;
				treeSupportBuffers.erase(sb);
				break;
			}
		}
	}
	treeSupportBuffers.emplace_back(sb);
	update();
}

TriangleMeshs GlWidget::GetSupportModel()
{
	TriangleMeshs support_meshs;
	stl_facet face;
	for (auto s = treeSupportBuffers.begin(); s != treeSupportBuffers.end(); ++s) {
		TriangleMesh mesh;
		for (int i = 0; i < (*s)->size; ++i) {
			face.vertex[0].x = (*s)->stl[i * 18];
			face.vertex[0].y = (*s)->stl[i * 18 + 1];
			face.vertex[0].z = (*s)->stl[i * 18 + 2];

			face.vertex[1].x = (*s)->stl[i * 18 + 6];
			face.vertex[1].y = (*s)->stl[i * 18 + 7];
			face.vertex[1].z = (*s)->stl[i * 18 + 8];

			face.vertex[2].x = (*s)->stl[i * 18 + 12];
			face.vertex[2].y = (*s)->stl[i * 18 + 13];
			face.vertex[2].z = (*s)->stl[i * 18 + 14];

			stl_add_facet(&mesh.stl, &face);
		}
		support_meshs.push_back(mesh);
	}
	return support_meshs;
}

TriangleMesh GlWidget::SaveSupport()
{
	TriangleMesh mesh;
	stl_facet face;
	for (auto s = treeSupportBuffers.begin(); s != treeSupportBuffers.end(); ++s) {
		for (int i = 0; i < (*s)->size; ++i) {
			
			face.vertex[0].x = (*s)->stl[i * 18];
			face.vertex[0].y = (*s)->stl[i * 18 + 1];
			face.vertex[0].z = (*s)->stl[i * 18 + 2];
			
			face.vertex[1].x = (*s)->stl[i * 18 + 6];
			face.vertex[1].y = (*s)->stl[i * 18 + 7];
			face.vertex[1].z = (*s)->stl[i * 18 + 8];
	
			face.vertex[2].x = (*s)->stl[i * 18 + 12];
			face.vertex[2].y = (*s)->stl[i * 18 + 13];
			face.vertex[2].z = (*s)->stl[i * 18 + 14];
	
			stl_add_facet(&mesh.stl, &face);
		}
	}
	return mesh;
}

void GlWidget::BindOneSupport()
{
	InitModelMatrix();

	m_program->setUniformValue(func, 1);
	m_supEditControl->m_curPoint->buffer->bind();
	m_program->enableAttributeArray(0);
	m_program->enableAttributeArray(1);
	m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
	m_program->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

	glDrawArrays(GL_TRIANGLES, 0, m_supEditControl->m_curPoint->size * 3); //第三个参数是顶点的数量

	m_supEditControl->m_curPoint->buffer->release();
	m_program->disableAttributeArray(0);
	m_program->disableAttributeArray(1);
}


void GlWidget::SaveOneView(char* file)
{
	GLint viewPort[4] = { 0 };
	glGetIntegerv(GL_VIEWPORT, viewPort);
	GLint width = viewPort[2];
	GLint height = viewPort[3];

	GLint pixelLength;
	GLubyte* pixelDate;
	FILE* wfile;
	//打开文件
	wfile = fopen(file, "wb");
	fwrite(head, 54, 1, wfile);
	//更改grab.bmp的头文件中的高度和宽度
	fseek(wfile, 0x0012, SEEK_SET);
	fwrite(&width, sizeof(width), 1, wfile);
	fwrite(&height, sizeof(height), 1, wfile);
	//为像素分配内存
	pixelLength = width * 3;
	if (pixelLength % 4 != 0)
	{
		pixelLength += 4 - pixelLength % 4;
	}
	pixelLength *= height;
	pixelDate = (GLubyte*)malloc(pixelLength);
	if (pixelDate == 0)
	{
		printf("/a/n分配内存失败!");
	}
	//读取窗口像素并存储
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glReadPixels(0, 0, width, height, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixelDate);
	//写入像素数据
	fseek(wfile, 0, SEEK_END);
	fwrite(pixelDate, pixelLength, 1, wfile);
	fclose(wfile);
	free(pixelDate);
}

void GlWidget::SupportEditChange(QProgressBar* progress)
{
	if (m_selInstance == nullptr)
		return;

	if (m_supEditControl == nullptr) {
		m_supEditControl = new SupEditControl(this);
		m_supEditControl->InitOneSupport();

		//保存待支撑模型
		m_supEditControl->m_mesh = new TriangleMesh(m_selInstance->get_object()->volumes[0]->mesh);
		m_selInstance->transform_mesh(m_supEditControl->m_mesh);

		TreeSupport* p = m_dlprint->GetTreeSupport(selectID);
		if (p != nullptr) {
			//有支撑点,将支撑点传入m_supportEditPoints并初始化渲染
			Pointf3 temp(0, 0, 0);
			stl_vertexs ps = p->support_point;
			ps.insert(ps.end(), p->support_point_face.begin(), p->support_point_face.end());
			for each (const stl_vertex & i in ps)
				m_supEditControl->AddSupportPoint(i);
		}
	}
	else {
		TreeSupport* sup = m_dlprint->GetTreeSupport(selectID);
		TreeSupport* sup_temp = new TreeSupport;

		//支撑不存在，只进行手动支撑
		if (sup == nullptr) {
			for each (const SupportBuffer * sb in m_supEditControl->supportPoints)
				sup_temp->support_point_face.emplace_back(stl_vertex{ (float)sb->origin.x,(float)sb->origin.y,(float)sb->origin.z });
		}
		else {
			if (sup->support_point.empty()) {
				for each (const SupportBuffer * sb in m_supEditControl->supportPoints)
					sup_temp->support_point_face.emplace_back(stl_vertex{ (float)sb->origin.x,(float)sb->origin.y,(float)sb->origin.z });
			}
			else {
				stl_vertex v;
				for each (const SupportBuffer * sb in m_supEditControl->supportPoints) {
					v = { (float)sb->origin.x,(float)sb->origin.y,(float)sb->origin.z };
					for (auto v1 = sup->support_point.begin(); v1 != sup->support_point.end(); ++v1) {
						if (equal_vertex(v, *v1)) {
							sup_temp->support_point.emplace_back(v);
							break;
						}

						if (v1 == sup->support_point.end() - 1)
							sup_temp->support_point_face.emplace_back(v);
					}
				}
			}
		}

		UpdateTreeSupport(sup_temp, progress);
		delete m_supEditControl;
		m_supEditControl = nullptr;
	}
	update();
}

void GlWidget::InitConfine()
{
	unsigned int L = e_setting.m_printers.begin()->length;
	unsigned int W = e_setting.m_printers.begin()->width;
	unsigned int H = e_setting.m_printers.begin()->height;

	GLfloat LF = GLfloat(L / 2);
	GLfloat WF = GLfloat(W / 2);

	frontBuffer.InitStl(6);
	backBuffer.InitStl(6);
	leftBuffer.InitStl(6);
	rightBuffer.InitStl(6);
	upBuffer.InitStl(6);
	downBuffer.InitStl(6);

	GLfloat* p1 = frontBuffer.stl;
	GLfloat* p2 = backBuffer.stl;
	GLfloat* p3 = leftBuffer.stl;
	GLfloat* p4 = rightBuffer.stl;
	GLfloat* p5 = upBuffer.stl;
	GLfloat* p6 = downBuffer.stl;

	//底面下降1mm
	*p1++ = -LF; *p1++ = -WF; *p1++ = -1;
	*p1++ = LF; *p1++ = -WF; *p1++ = -1;
	*p1++ = -LF; *p1++ = -WF; *p1++ = H;

	*p1++ = -LF; *p1++ = -WF; *p1++ = H;
	*p1++ = LF; *p1++ = -WF; *p1++ = -1;
	*p1++ = LF; *p1++ = -WF; *p1++ = H;

	*p2++ = -LF; *p2++ = WF; *p2++ = -1;
	*p2++ = LF; *p2++ = WF; *p2++ = -1;
	*p2++ = -LF; *p2++ = WF; *p2++ = H;

	*p2++ = -LF; *p2++ = WF; *p2++ = H;
	*p2++ = LF; *p2++ = WF; *p2++ = -1;
	*p2++ = LF; *p2++ = WF; *p2++ = H;

	*p3++ = -LF; *p3++ = -WF; *p3++ = -1;
	*p3++ = -LF; *p3++ = WF; *p3++ = -1;
	*p3++ = -LF; *p3++ = -WF; *p3++ = H;

	*p3++ = -LF; *p3++ = -WF; *p3++ = H;
	*p3++ = -LF; *p3++ = WF; *p3++ = -1;
	*p3++ = -LF; *p3++ = WF; *p3++ = H;

	*p4++ = LF; *p4++ = -WF; *p4++ = -1;
	*p4++ = LF; *p4++ = WF; *p4++ = -1;
	*p4++ = LF; *p4++ = -WF; *p4++ = H;

	*p4++ = LF; *p4++ = -WF; *p4++ = H;
	*p4++ = LF; *p4++ = WF; *p4++ = -1;
	*p4++ = LF; *p4++ = WF; *p4++ = H;

	*p5++ = -LF; *p5++ = -WF; *p5++ = H;
	*p5++ = LF; *p5++ = -WF; *p5++ = H;
	*p5++ = -LF; *p5++ = WF; *p5++ = H;

	*p5++ = -LF; *p5++ = WF; *p5++ = H;
	*p5++ = LF; *p5++ = -WF; *p5++ = H;
	*p5++ = LF; *p5++ = WF; *p5++ = H;

	*p6++ = -LF; *p6++ = -WF; *p6++ = -1;
	*p6++ = LF; *p6++ = -WF; *p6++ = -1;
	*p6++ = -LF; *p6++ = WF; *p6++ = -1;

	*p6++ = -LF; *p6++ = WF; *p6++ = -1;
	*p6++ = LF; *p6++ = -WF; *p6++ = -1;
	*p6++ = LF; *p6++ = WF; *p6++ = -1;

	frontBuffer.BindBuffer();
	backBuffer.BindBuffer();
	leftBuffer.BindBuffer();
	rightBuffer.BindBuffer();
	upBuffer.BindBuffer();
	downBuffer.BindBuffer();
}

void GlWidget::BindConfine()
{
	InitModelMatrix();

	m_program->setUniformValue(func, 5);
	m_program->enableAttributeArray(0);

	if (frontBuffer.visible) {
		frontBuffer.buffer->bind();
		m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));
		glDrawArrays(GL_TRIANGLES, 0, frontBuffer.size);
		frontBuffer.buffer->release();
	}

	if (backBuffer.visible) {
		backBuffer.buffer->bind();
		m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));
		backBuffer.buffer->release();
		glDrawArrays(GL_TRIANGLES, 0, backBuffer.size);
	}

	if (leftBuffer.visible) {
		leftBuffer.buffer->bind();
		m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));
		leftBuffer.buffer->release();
		glDrawArrays(GL_TRIANGLES, 0, leftBuffer.size);
	}

	if (rightBuffer.visible) {
		rightBuffer.buffer->bind();
		m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));
		rightBuffer.buffer->release();
		glDrawArrays(GL_TRIANGLES, 0, rightBuffer.size);
	}

	if (upBuffer.visible) {
		upBuffer.buffer->bind();
		m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));
		upBuffer.buffer->release();
		glDrawArrays(GL_TRIANGLES, 0, upBuffer.size);
	}

	if (downBuffer.visible) {
		downBuffer.buffer->bind();
		m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));
		downBuffer.buffer->release();
		glDrawArrays(GL_TRIANGLES, 0, downBuffer.size);
	}

	m_program->disableAttributeArray(0);
}

void GlWidget::UpdateConfine()
{
	unsigned int L = e_setting.m_printers.begin()->length;
	unsigned int W = e_setting.m_printers.begin()->width;
	unsigned int H = e_setting.m_printers.begin()->height;

	GLfloat LF = GLfloat(L / 2);
	GLfloat WF = GLfloat(W / 2);

	frontBuffer.visible = false;
	backBuffer.visible = false;
	leftBuffer.visible = false;
	rightBuffer.visible = false;
	upBuffer.visible = false;
	downBuffer.visible = false;

	for (auto o = m_model->objects.begin(); o != m_model->objects.end(); ++o) {
		for (auto i = (*o)->instances.begin(); i != (*o)->instances.end(); ++i) {
			TriangleMesh mesh = (*o)->volumes[0]->mesh;
			(*i)->transform_mesh(&mesh);
			BoundingBoxf3 box = mesh.bounding_box();

			if (!frontBuffer.visible) {
				if (box.min.y < -WF)
					frontBuffer.visible = true;
			}

			if (!backBuffer.visible) {
				if (box.max.y > WF)
					backBuffer.visible = true;
			}

			if (!leftBuffer.visible) {
				if (box.min.x < -LF)
					leftBuffer.visible = true;
			}

			if (!rightBuffer.visible) {
				if (box.max.x > LF)
					rightBuffer.visible = true;
			}

			if (!upBuffer.visible) {
				if (box.max.z > H)
					upBuffer.visible = true;
			}

			if (!downBuffer.visible) {
				if (box.min.z < 0)
					downBuffer.visible = true;
			}
		}
	}
	update();
}

bool GlWidget::CheckConfine()
{
	return frontBuffer.visible 
		|| backBuffer.visible 
		|| leftBuffer.visible 
		|| rightBuffer.visible 
		|| upBuffer.visible 
		|| downBuffer.visible;
}

void GlWidget::InitCoord()
{
	unsigned int L = e_setting.m_printers.begin()->length;
	unsigned int W = e_setting.m_printers.begin()->width;

	coordBuffer.size = 3;
	if (coordBuffer.stl != nullptr)
		delete[] coordBuffer.stl;
	coordBuffer.stl = new GLfloat[coordBuffer.size * 2 * 6];
	GLfloat* p = coordBuffer.stl;

	//x轴,红色
	*p++ = -GLfloat(L / 2); *p++ = -GLfloat(W / 2); *p++ = 0.0f; *p++ = 1.0f; *p++ = 0.0f; *p++ = 0.0f;
	*p++ = -(GLfloat(L / 2) - 30); *p++ = -GLfloat(W / 2); *p++ = 0.0f; *p++ = 1.0f; *p++ = 0.0f; *p++ = 0.0f;

	//y轴,绿色
	*p++ = -GLfloat(L / 2); *p++ = -GLfloat(W / 2); *p++ = 0.0f; *p++ = 0.0f; *p++ = 1.0f; *p++ = 0.0f;
	*p++ = -GLfloat(L / 2); *p++ = -(GLfloat(W / 2) - 30); *p++ = 0.0f; *p++ = 0.0f; *p++ = 1.0f; *p++ = 0.0f;

	//z轴，蓝色
	*p++ = -GLfloat(L / 2); *p++ = -GLfloat(W / 2); *p++ = 0.0f; *p++ = 0.0f; *p++ = 0.0f; *p++ = 1.0f;
	*p++ = -GLfloat(L / 2); *p++ = -GLfloat(W / 2); *p++ = 30.0f; *p++ = 0.0f; *p++ = 0.0f; *p++ = 1.0f;

	if (coordBuffer.buffer == nullptr)
		coordBuffer.buffer = new QOpenGLBuffer();

	if (!coordBuffer.buffer->isCreated())
		coordBuffer.buffer->create();
	coordBuffer.buffer->bind();
	coordBuffer.buffer->allocate(coordBuffer.stl, coordBuffer.size * 2 * 6 * sizeof(GLfloat));
	coordBuffer.buffer->release();
}

void GlWidget::BindCoord()
{
	InitModelMatrix();

	m_program->setUniformValue(func, 7);
	m_program->enableAttributeArray(0);
	m_program->enableAttributeArray(2);

	coordBuffer.buffer->bind();
	m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
	m_program->setAttributeBuffer(2, GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));
	coordBuffer.buffer->release();
	glDrawArrays(GL_LINES, 0, coordBuffer.size * 2);

	m_program->disableAttributeArray(0);
	m_program->disableAttributeArray(2);
}

void GlWidget::BindTreeSupport()
{
	InitModelMatrix();

	m_program->setUniformValue(func, 4);
	for (auto s = treeSupportBuffers.begin(); s != treeSupportBuffers.end(); ++s) {
		ModelBuffer* sb = *s;
		sb->buffer->bind();
		m_program->enableAttributeArray(0);
		m_program->enableAttributeArray(1);
		m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
		m_program->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

		glDrawArrays(GL_TRIANGLES, 0, sb->size * 3); //第三个参数是顶点的数量

		sb->buffer->release();
		m_program->disableAttributeArray(0);
		m_program->disableAttributeArray(1);
	}
}

void GlWidget::BindSupportEditBuffer()
{
	InitModelMatrix();

	for (auto SB = m_supEditControl->supportPoints.begin(); SB != m_supEditControl->supportPoints.end(); ++SB) {
		m_program->setUniformValue(func, 6);
		(*SB)->buffer->bind();
		m_program->enableAttributeArray(0);
		m_program->enableAttributeArray(1);
		m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
		m_program->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

		glDrawArrays(GL_TRIANGLES, 0, (*SB)->size * 3); //第三个参数是顶点的数量

		(*SB)->buffer->release();
		m_program->disableAttributeArray(0);
		m_program->disableAttributeArray(1);
	}
}

TriangleMesh GlWidget::FindModel(size_t id)
{
	ModelInstance* i = m_model->find_instance(id);
	ModelObject* o = i->get_object();
	TriangleMesh mesh = o->volumes[0]->mesh;
	i->transform_mesh(&mesh);
	return mesh;
}

void GlWidget::InitModelMatrix()
{
	QMatrix4x4 rotateM, scaleM, translateM;
	rotateM.setToIdentity();
	scaleM.setToIdentity();
	translateM.setToIdentity();

	m_program->setUniformValue(m_rotateMatrixLoc, rotateM);
	m_program->setUniformValue(m_scaleMatrixLoc, scaleM);
	m_program->setUniformValue(m_translateMatrixLoc, translateM);
}

void GlWidget::SetViewPort(VIEWPORT view)
{
	if (this->viewport != view) {
		this->viewport = view;

		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		resizeGL(viewport[2], viewport[3]);
		update();
	}
}

void GlWidget::SetDefaultView()
{
	if (viewport == PRESPRECTIVE) {
		_scale = 0.8;
		center.setX(0);
		center.setY(0);
		center.setZ(0);
		_horizonAngle = 80;
		_verticalAngle = 5;
	}
	else {
		_scale = 6;
		center.setX(0);
		center.setY(0);
		center.setZ(100);
		_horizonAngle = 78;
		_verticalAngle = 10;
	}
}

void GlWidget::ReadBasicMesh()
{
	QString ball_path(QString(e_setting.modelPath.c_str()) + "/ball.stl");
	QString cube_path(QString(e_setting.modelPath.c_str()) + "/cube.stl");
	QString ring_path(QString(e_setting.modelPath.c_str()) + "/ring.stl");
	QString cone_path(QString(e_setting.modelPath.c_str()) + "/cone.stl");

	QFileInfo f(cube_path);
	if (f.isFile())
		cube.ReadSTLFile(cube_path.toStdString());
	else
		qDebug() << "read cube error!";

	f.setFile(ball_path);
	if (f.isFile())
		ball.ReadSTLFile(ball_path.toStdString());
	else
		qDebug() << "read ball error!";

	f.setFile(ring_path);
	if (f.isFile())
		ring.ReadSTLFile(ring_path.toStdString());
	else
		qDebug() << "read ring error!";

	f.setFile(cone_path);
	if (f.isFile())
		cone.ReadSTLFile(cone_path.toStdString());
	else
		qDebug() << "read cone error!";
}

void GlWidget::DrawTranMesh()
{
	if (selectID > SelInvalidID && m_supEditControl == nullptr) {
		//得到模型实例
		ModelInstance* i = m_selInstance;

		//半径
		double radius = sqrtf(((i->box.max.x - i->box.min.x) / 2) * ((i->box.max.x - i->box.min.x) / 2) +
			((i->box.max.y - i->box.min.y) / 2) * ((i->box.max.y - i->box.min.y) / 2) +
			((i->box.max.z - i->box.min.z) / 2) * ((i->box.max.z - i->box.min.z) / 2));

		QMatrix4x4 scaleM, translateM;
		scaleM.setToIdentity();
		translateM.setToIdentity();

		m_program->setUniformValue(func, 8);//!!!

		double s = radius / 10 * 0.8;

		//rotate
		scaleM.scale(s);
		translateM.translate(i->origin.x, i->origin.y, i->origin.z);

		if (translationID == rotateMesh_X.id || translationID == TranInvalidID)
			DrawTranMeshCase(&rotateMesh_X, scaleM, translateM);

		if (translationID == rotateMesh_Y.id || translationID == TranInvalidID)
			DrawTranMeshCase(&rotateMesh_Y, scaleM, translateM);

		if (translationID == rotateMesh_Z.id || translationID == TranInvalidID)
			DrawTranMeshCase(&rotateMesh_Z, scaleM, translateM);

		TriangleMesh m(ring);
		m.scale(s);
		double x = (m.stl.stats.max.x - m.stl.stats.min.x) / 2 * 1.01;

		s *= 2;
		//scale
		if (translationID == scaleMesh_X.id || translationID == TranInvalidID) {
			scaleM.setToIdentity();
			scaleM.scale(s);
			translateM.setToIdentity();
			translateM.translate(i->origin.x - x, i->origin.y, i->origin.z);
			DrawTranMeshCase(&scaleMesh_X, scaleM, translateM);
		}

		if (translationID == scaleMesh_Y.id || translationID == TranInvalidID) {
			scaleM.setToIdentity();
			scaleM.scale(s);
			translateM.setToIdentity();
			translateM.translate(i->origin.x, i->origin.y + x, i->origin.z);
			DrawTranMeshCase(&scaleMesh_Y, scaleM, translateM);
		}

		if (translationID == scaleMesh_Z.id || translationID == TranInvalidID) {
			scaleM.setToIdentity();
			scaleM.scale(s);
			translateM.setToIdentity();
			translateM.translate(i->origin.x, i->origin.y, i->origin.z - x);
			DrawTranMeshCase(&scaleMesh_Z, scaleM, translateM);
		}

		//translate
		if (translationID == translateMesh_X.id || translationID == TranInvalidID) {
			scaleM.setToIdentity();
			scaleM.scale(s);
			translateM.setToIdentity();
			translateM.translate(i->origin.x + x, i->origin.y, i->origin.z);
			DrawTranMeshCase(&translateMesh_X, scaleM, translateM);
		}

		if (translationID == translateMesh_Y.id || translationID == TranInvalidID) {
			scaleM.setToIdentity();
			scaleM.scale(s);
			translateM.setToIdentity();
			translateM.translate(i->origin.x, i->origin.y - x, i->origin.z);
			DrawTranMeshCase(&translateMesh_Y, scaleM, translateM);
		}

		if (translationID == translateMesh_Z.id || translationID == TranInvalidID) {
			scaleM.setToIdentity();
			scaleM.scale(s);
			translateM.setToIdentity();
			translateM.translate(i->origin.x, i->origin.y, i->origin.z + x);
			DrawTranMeshCase(&translateMesh_Z, scaleM, translateM);
		}
	}
}

void GlWidget::DrawTranMeshCase(ModelBuffer* mb, QMatrix4x4 scaleM, QMatrix4x4 translateM)
{
	mb->buffer->bind();
	m_program->enableAttributeArray(0);
	m_program->enableAttributeArray(1);
	m_program->enableAttributeArray(2);
	m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 9 * sizeof(GLfloat));
	m_program->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 3, 9 * sizeof(GLfloat));
	m_program->setAttributeBuffer(2, GL_FLOAT, 6 * sizeof(GLfloat), 3, 9 * sizeof(GLfloat));

	QMatrix4x4 rotateM;
	rotateM.setToIdentity();
	
	m_program->setUniformValue(m_rotateMatrixLoc, rotateM);
	m_program->setUniformValue(m_scaleMatrixLoc, scaleM);
	m_program->setUniformValue(m_translateMatrixLoc, translateM);
	
	glDrawArrays(GL_TRIANGLES, 0, mb->size * 3);
	
	mb->buffer->release();
	m_program->disableAttributeArray(0);
	m_program->disableAttributeArray(1);
	m_program->disableAttributeArray(2);
}

void GlWidget::RenderTranMeshCase(TriangleMesh mesh, int id, Vectorf3 direction, QMatrix4x4 scaleM, QMatrix4x4 translateM)
{
	stl_facet f;
	glLoadName(id);

	mesh.rotate_x((direction.x / 180)*PI);
	mesh.rotate_y((direction.y) / 180 * PI);
	mesh.rotate_z((direction.z) / 180 * PI);
	mesh.transform_matrix(scaleM);
	mesh.transform_matrix(translateM);
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < mesh.stl.stats.number_of_facets; ++i) {
		f = mesh.stl.facet_start[i];
		glVertex3f(f.vertex[0].x, f.vertex[0].y, f.vertex[0].z);
		glVertex3f(f.vertex[1].x, f.vertex[1].y, f.vertex[1].z);
		glVertex3f(f.vertex[2].x, f.vertex[2].y, f.vertex[2].z);
	}
	glEnd();
}

//传入缩放的比例值
void GlWidget::ScaleValueChange(double x, double y, double z, bool back)
{
	if (x != 0) m_selInstance->scaling_vector.x += x;
	if (y != 0) m_selInstance->scaling_vector.y += y;
	if (z != 0) m_selInstance->scaling_vector.z += z;

	m_selInstance->update_attribute();
	UpdateConfine();

	if (back)
		emit sig_scaleChange();
}

//传入旋转的角度的数值
void GlWidget::RotateValueChange(double angle, int x, int y, int z, bool back)
{
	QMatrix4x4 m;
	m.rotate(angle, x, y, z);
	m_selInstance->rotation_M = m * m_selInstance->rotation_M;

	m_selInstance->update_attribute();
	UpdateConfine();

	if (back)
		emit sig_rotateChange();
}

//传入移动的距离数值
void GlWidget::OffsetValueChange(double x, double y, double z, bool back)
{
	if (x != 0)m_selInstance->offset.x += x;
	if (y != 0)m_selInstance->offset.y += y;
	if (z != 0)m_selInstance->z_translation += z;

	m_selInstance->update_attribute();
	UpdateConfine();

	if (back)
		emit sig_offsetChange();
}

void GlWidget::slot_delSelectIntance()
{
	if (m_selInstance == nullptr)
		return;

	DelSelectSupport(true);//删除模型时，删除支撑需重新排序
	DelModelBuffer(selectID);
	m_model->delete_modelInstance(selectID);
	UpdateConfine();

	m_selInstance = nullptr;
	selectID = SelInvalidID;
}

bool GlWidget::DelSelectSupport(bool resort)
{
	if (m_selInstance == nullptr)
		return false;
	return DelSupport(selectID, true);
}

bool GlWidget::DelSupport(size_t id, bool resort)
{
	bool ret = false;
	if (m_dlprint->DelTreeSupport(id, resort)) {
		//支撑数据删除成功同时删除支撑缓存区
		for (auto sb = treeSupportBuffers.begin(); sb != treeSupportBuffers.end(); ++sb) {
			if ((*sb)->id == id) {
				delete* sb;
				treeSupportBuffers.erase(sb);
				ret = true;
				break;
			}
		}

		if (resort) {
			for (auto sb = treeSupportBuffers.begin(); sb != treeSupportBuffers.end(); ++sb) {
				if ((*sb)->id / InstanceNum == id / InstanceNum && (*sb)->id > id)
					(*sb)->id = (*sb)->id - 1;
			}
		}
	}
	if (ret)
		update();
	return ret;
}

void GlWidget::GenSelInstanceSupport(QProgressBar* progress)
{
	if (m_selInstance == nullptr)
		return;
	GenInstanceSupport(selectID, progress);
}

void GlWidget::GenInstanceSupport(size_t id, QProgressBar* progress)
{
	//支撑数值存入treeSupports
	m_dlprint->InsertSupport(id, m_dlprint->GenSupport(id, &FindModel(id), progress));
	//渲染支撑
	InitTreeSupportID(id, progress);
}

void GlWidget::slot_dlprinterChange(QString name)
{
	if (e_setting.setSelMachine(name.toStdString())) {
		QSettings writeini(e_setting.DlprinterFile.c_str(), QSettings::IniFormat);
		writeini.setValue("/dlprinter/name", name);

		InitPlatform();
		InitConfine();
		InitCoord();

		UpdateConfine();
	}
}

void GlWidget::UpdateTreeSupport(TreeSupport* new_sup, QProgressBar* progress)
{
	//从支撑编辑里得到，只有支撑点
	new_sup->GenTreeSupport(FindModel(selectID), TreeSupport::Paras{ m_dlprint->m_config->leaf_num
		,m_dlprint->m_config->threads ,m_dlprint->m_config->support_top_height }, progress);

	m_dlprint->InsertSupport(selectID, new_sup);
	InitTreeSupportID(selectID, progress);
}

void GlWidget::AddModelInstance(size_t id)
{
	ModelBuffer* mb = new ModelBuffer;
	//读取对应modelObject里的渲染数据
	TriangleMesh mesh = m_model->find_object(id)->volumes[0]->mesh;
	mb->size = mesh.stl.stats.number_of_facets;

	GLfloat* v = new GLfloat[mb->size * 3 * 2 * 3];
	for (int i = 0; i < mb->size; ++i) {
		v[i * 18] = mesh.stl.facet_start[i].vertex[0].x;
		v[i * 18 + 1] = mesh.stl.facet_start[i].vertex[0].y;
		v[i * 18 + 2] = mesh.stl.facet_start[i].vertex[0].z;
		v[i * 18 + 3] = mesh.stl.facet_start[i].normal.x;
		v[i * 18 + 4] = mesh.stl.facet_start[i].normal.y;
		v[i * 18 + 5] = mesh.stl.facet_start[i].normal.z;

		v[i * 18 + 6] = mesh.stl.facet_start[i].vertex[1].x;
		v[i * 18 + 7] = mesh.stl.facet_start[i].vertex[1].y;
		v[i * 18 + 8] = mesh.stl.facet_start[i].vertex[1].z;
		v[i * 18 + 9] = mesh.stl.facet_start[i].normal.x;
		v[i * 18 + 10] = mesh.stl.facet_start[i].normal.y;
		v[i * 18 + 11] = mesh.stl.facet_start[i].normal.z;

		v[i * 18 + 12] = mesh.stl.facet_start[i].vertex[2].x;
		v[i * 18 + 13] = mesh.stl.facet_start[i].vertex[2].y;
		v[i * 18 + 14] = mesh.stl.facet_start[i].vertex[2].z;
		v[i * 18 + 15] = mesh.stl.facet_start[i].normal.x;
		v[i * 18 + 16] = mesh.stl.facet_start[i].normal.y;
		v[i * 18 + 17] = mesh.stl.facet_start[i].normal.z;
	}
	mb->stl = v;
	mb->id = id;
	mb->buffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	mb->buffer->create();
	mb->buffer->bind();
	mb->buffer->allocate(mb->stl, mb->size * sizeof(GLfloat) * 3 * 2 * 3);
	mb->buffer->release();
	modelBuffers.emplace_back(mb);
	update();
}

//--------------GlWidget::SupEditControl---------------

void GlWidget::SupEditControl::AddSupportPoint(const stl_vertex& ver)
{
	SupportBuffer* SB = new SupportBuffer();
	SB->origin = Pointf3(ver.x, ver.y, ver.z);
	TriangleMesh mesh = m_parent->ball;
	mesh.translate(SB->origin.x, SB->origin.y, SB->origin.z);

	SB->id = -1;
	SB->size = mesh.stl.stats.number_of_facets;
	SB->stl = new GLfloat[SB->size * 3 * 2 * 3];

	float normal[3];
	for (int i = 0; i < SB->size; ++i) {
		stl_calculate_normal(normal, &mesh.stl.facet_start[i]);
		stl_normalize_vector(normal);

		SB->stl[i * 18] = mesh.stl.facet_start[i].vertex[0].x;
		SB->stl[i * 18 + 1] = mesh.stl.facet_start[i].vertex[0].y;
		SB->stl[i * 18 + 2] = mesh.stl.facet_start[i].vertex[0].z;
		SB->stl[i * 18 + 3] = normal[0];
		SB->stl[i * 18 + 4] = normal[1];
		SB->stl[i * 18 + 5] = normal[2];

		SB->stl[i * 18 + 6] = mesh.stl.facet_start[i].vertex[1].x;
		SB->stl[i * 18 + 7] = mesh.stl.facet_start[i].vertex[1].y;
		SB->stl[i * 18 + 8] = mesh.stl.facet_start[i].vertex[1].z;
		SB->stl[i * 18 + 9] = normal[0];
		SB->stl[i * 18 + 10] = normal[1];
		SB->stl[i * 18 + 11] = normal[2];

		SB->stl[i * 18 + 12] = mesh.stl.facet_start[i].vertex[2].x;
		SB->stl[i * 18 + 13] = mesh.stl.facet_start[i].vertex[2].y;
		SB->stl[i * 18 + 14] = mesh.stl.facet_start[i].vertex[2].z;
		SB->stl[i * 18 + 15] = normal[0];
		SB->stl[i * 18 + 16] = normal[1];
		SB->stl[i * 18 + 17] = normal[2];
	}
	SB->buffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	SB->buffer->create();
	SB->buffer->bind();
	SB->buffer->allocate(SB->stl, SB->size * 3 * 2 * 3 * sizeof(GLfloat));
	SB->buffer->release();
	supportPoints.emplace_back(SB);
}

void GlWidget::SupEditControl::DelSupportPoint(size_t id)
{
	auto sb = supportPoints.begin() + id;
	if (sb != supportPoints.end()) {
		delete* sb;
		supportPoints.erase(sb);
	}
}

void GlWidget::SupEditControl::AddNewSupportPoint()
{
	if (m_curPoint->new_origin.x != 0
		&& m_curPoint->new_origin.y != 0
		&& m_curPoint->new_origin.y != 0) {
		AddSupportPoint(stl_vertex{ (float)m_curPoint->new_origin.x, (float)m_curPoint->new_origin.y,(float)m_curPoint->new_origin.z });
		m_curPoint->new_origin = Pointf3(0.0, 0.0, 0.0);
		InitOneSupport();
	}
}

void GlWidget::SupEditControl::InitOneSupport()
{
	if (m_curPoint == nullptr) {
		//ball就是在原点0,0,0
		m_curPoint = new SupportBuffer();
		m_curPoint->id = -1;
		m_curPoint->size = m_parent->ball.stl.stats.number_of_facets;
		m_curPoint->stl = new GLfloat[m_curPoint->size * 3 * 2 * 3];

		float normal[3];
		for (int i = 0; i < m_curPoint->size; ++i) {
			stl_calculate_normal(normal, &m_parent->ball.stl.facet_start[i]);
			stl_normalize_vector(normal);

			m_curPoint->stl[i * 18] = m_parent->ball.stl.facet_start[i].vertex[0].x;
			m_curPoint->stl[i * 18 + 1] = m_parent->ball.stl.facet_start[i].vertex[0].y;
			m_curPoint->stl[i * 18 + 2] = m_parent->ball.stl.facet_start[i].vertex[0].z;
			m_curPoint->stl[i * 18 + 3] = normal[0];
			m_curPoint->stl[i * 18 + 4] = normal[1];
			m_curPoint->stl[i * 18 + 5] = normal[2];

			m_curPoint->stl[i * 18 + 6] = m_parent->ball.stl.facet_start[i].vertex[1].x;
			m_curPoint->stl[i * 18 + 7] = m_parent->ball.stl.facet_start[i].vertex[1].y;
			m_curPoint->stl[i * 18 + 8] = m_parent->ball.stl.facet_start[i].vertex[1].z;
			m_curPoint->stl[i * 18 + 9] = normal[0];
			m_curPoint->stl[i * 18 + 10] = normal[1];
			m_curPoint->stl[i * 18 + 11] = normal[2];

			m_curPoint->stl[i * 18 + 12] = m_parent->ball.stl.facet_start[i].vertex[2].x;
			m_curPoint->stl[i * 18 + 13] = m_parent->ball.stl.facet_start[i].vertex[2].y;
			m_curPoint->stl[i * 18 + 14] = m_parent->ball.stl.facet_start[i].vertex[2].z;
			m_curPoint->stl[i * 18 + 15] = normal[0];
			m_curPoint->stl[i * 18 + 16] = normal[1];
			m_curPoint->stl[i * 18 + 17] = normal[2];
		}

		m_curPoint->buffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		m_curPoint->buffer->create();
	}
	else
		m_curPoint->translation();

	m_curPoint->buffer->bind();
	m_curPoint->buffer->allocate(m_curPoint->stl, m_curPoint->size * 3 * 2 * 3 * sizeof(GLfloat));
	m_curPoint->buffer->release();
}
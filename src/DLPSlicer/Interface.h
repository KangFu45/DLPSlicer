#ifndef slic3r_Interface_hpp_
#define slic3r_Interface_hpp_

#include"libslic3r.h"
#include"Model.hpp"
#include"DLPrint.h"
#include"qprogressbar.h"
#include"DLPrinter.h"

namespace Slic3r{

	//���ߣ�����
	//���ڣ�2017
	//���ܣ��ӿڣ����ӽ�����ײ����ݴ���Ч�������룩��
	class Interface
	{
	public:
		Interface(DLPrinter* _dlprinter);

		/*----------------------ģ�͹���----------------------*/

		//���ڣ�2017
		//���ܣ����ģ�͡�
		//����1����ȡģ�͵�·��
		//���أ����ģ��id
		size_t load_model(std::string file,int format);

		//���ڣ�2018.3.12
		//���ܣ����ģ��ʵ��
		//����1��ģ��id
		ModelInstance* addInstance(size_t id);

		//���ڣ�2017
		//���ܣ�ɾ��ģ�ͣ�ɾ��һ��ģ�Ͷ��󣬻Ὣ������ʵ��ȫ��ɾ������
		//����1����ɾ��ģ�Ͷ����id
		void delete_model(size_t id);

		//���ڣ�2017
		//���ܣ�ɾ������ģ�͡�
		void clear_model();

		//���ڣ�2017
		//���ܣ�����ģ��Z�᷽���ƶ����롣
		//����1���ƶ�����ֵ
		void model_lift(double distance);

		//���ڣ�2017
		//���ܣ�������ͼ�ڵ�ģ��֧�š�
		//����1������·��
		//����2��֧��ģ��
		void wirteStlBinary(const std::string& outFile,TriangleMesh& supportMesh);

		//���ڣ�2018.3.7
		//���ܣ��Զ����С�
		//����1�����о���
		Pointfs autoArrange(double dis);

		//���ڣ�2017
		//���ܣ�����id�ҵ���Ӧ��ģ��ʵ����
		//����1��ģ��id
		//���أ�ģ��ʵ��
		ModelInstance* find_instance(size_t id);

		//���ڣ�2018.5.10
		//���ܣ�����ʵ������id��
		size_t find_id(ModelInstance* instance);

		//���ڣ�2017
		//���ܣ�����id�ҵ���Ӧ��ģ�Ͷ���
		//����1��ģ��id
		//���أ�ģ��ʵ��
		ModelObject* find_object(size_t id);

		/*---------------------DLP��ӡ------------------------*/

		//���ڣ�2018.4.8
		//���ܣ�����ѡ��ģ�͡�
		//����1��ѡ��ģ��id
		void generate_id_support(size_t id,TreeSupport*& s,QProgressBar* progress);
		
		//���ڣ�2017
		//���ܣ���������ģ�͵�֧�š�
		//void generate_all_support();

		//���ڣ�2017
		//���ܣ���������ģ�͵��ڲ�֧�š�
		void generate_all_inside_support();

		//���ڣ�2017
		//���ܣ�ɾ������ģ�͵�֧�š�
		void delete_all_support();		

		//���ڣ�2017
		//���ܣ�ɾ������ģ�͵��ڲ�֧�š�
		void delete_all_inside_support();

		//���ڣ�2017
		//���ܣ�ɾ��ѡ��֧�š�
		//����1��ѡ��֧�ŵ�id
		void delete_support(size_t id);

	private:
		//���ڣ�2017
		//���ܣ�ģ�;��С�
		//����1��������Ƭģ��
		void model_center(TriangleMesh& mesh);
		
		DLPrinter* dlprinter;

	public:
		Model* model;										//ģ�͵Ĺ���
		DLPrint* dlprint;									//DLP��ӡ
	};

}

#endif
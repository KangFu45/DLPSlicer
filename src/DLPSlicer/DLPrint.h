#pragma once
#ifndef slic3r_DLPPrint_hpp_
#define slic3r_DLPPrint_hpp_

#include "libslic3r.h"
#include "ExPolygon.hpp"
#include "ExPolygonCollection.hpp"
#include "Fill/Fill.hpp"
#include "Model.hpp"
#include "Point.hpp"
#include "PrintConfig.hpp"
#include <qrect.h>
#include <map>
#include "qpolygon.h"
#include "TreeSupport.h"
#include "qprogressbar.h"
#include "DLPrinter.h"

namespace Slic3r {

	//���ߣ�����
	//���ڣ�2017
	//���ܣ�dlp��ӡ������ÿ�����Ϣ��ִ����Ƭ������ͼƬ������
	class DLPrint
	{
	public:
		DLPrintConfig config;											//dlp��ӡ����
																		
		class Layer {													
		public:															
			ExPolygonCollection slices;									//��Ƭ��
			ExPolygonCollection perimeters;								//��Ȧ�߽���
			ExtrusionEntityCollection infill;							//�ڲ������
			ExPolygonCollection solid_infill;							//��̬�����
			float slice_z, print_z;										//��Ƭ�ߣ���ӡ��
			bool solid;													//�Ƿ�Ϊ��̬��

			Layer(float _slice_z, float _print_z)
				: slice_z(_slice_z), print_z(_print_z), solid(true) {};
		};
		size_t layer_num;												//ģ�Ͳ���
		std::vector<std::vector<Layer>> layers;							//ÿ��ʵ�����󵥶���Ƭ�ֲ�
		std::vector<double> areas;										//ÿ��������������ģ�͵����
		std::vector<Layer> _layers;										//����ÿ��ģ��Layer			
		std::vector<Layer> r_layers;									//�װ��
		std::map <size_t, linef3s*> inside_supports;					//ÿ��ģ�Ͷ�����Ե��ڲ�֧��


		//ÿ��ģ�Ͷ����֧��
		//����1��ģ�Ͷ����id
		//����2��ģ�Ͷ����֧�����ݽṹ(������)
		std::map<int, TreeSupport*> treeSupports;

		std::vector<Pointf> circle;										//��λԲ
		DLPrint(Model* _model,DLPrinter* _dlprinter) ;										//���캯��

		//���ڣ�2017
		//���ܣ���ģ�ͷֲ㣬��ա�
		//����1��֧��ģ��
		void slice(const std::vector<TriangleMesh>& support_meshs,QProgressBar* progress);

		//���ڣ�2017
		//���ܣ������ڲ�֧�ţ�֧�������εġ�
		//����1������id
		//����2��ģ��
		void generate_inside_support(size_t id, TriangleMesh* mesh);

		void insertSupports(size_t id, TreeSupport* s);

		bool chilck_tree_support(size_t id, TreeSupport*& s);

		//���ڣ�2017
		//���ܣ�����ָ��ģ�͵�֧�š�
		//����1������id
		//����2��ģ��
		void generate_support(size_t id, TreeSupport*& s,TriangleMesh* mesh,QProgressBar* progress);

		//���ڣ�2018.4.9
		//���ܣ�ɾ��ѡ����״֧�š�
		void delete_tree_support(size_t id);

		//���ڣ�2017
		//���ܣ�����PNGͼƬ��
		//����1������ͼƬ·��
		void savePNG(QString ini_file);

		//���ڣ�2017
		//���ܣ�����һ��PNGͼƬ��
		//����1������
		void saveOnePNG(size_t num);

	private:
		Model* model;													//ģ��
		BoundingBoxf3 bb;												//��Χ��
		QString ini_file;												//��Ƭ�ļ�·��
		QProgressBar* m_progress;
		DLPrinter* dlprinter;

		//�ĸ�����֧��ö��
		enum xyz
		{
			XZ_45 = 10,
			XZ_135,
			YZ_45,
			YZ_135
		};

		//���ڣ�2017
		//���ܣ�ģ��ʵ����id��
		//����1��ģ��ʵ��
		//���أ�id
		int instanceToId(ModelInstance* i);

		//���ڣ�2017
		//���ܣ�Polygon==>QPolygon����ʽת������
		//����1����ת���Ķ����
		//���أ�ת����Ķ����
		QPolygonF polTpQpol(Polygon& p);

		//���ڣ�2017
		//���ܣ����㡣
		//����2������
		void _infill_layer(size_t i, const Fill* _fill, ExPolygons pattern);

		//���ڣ�2017
		//���ܣ���ȡ��Χ�������ϵ㼯�ϡ�
		//����1����Χ��
		//����2�����ص������ϵĵ�
		//����3����ľ��루֧���ܶȣ�
		//����4��ָ����άƽ��
		void radiate_point(BoundingBoxf3 bb, Pointf3s& ps,float space,int xyz);

		//���ڣ�2017
		//���ܣ������󽻡�
		//����1:���ڲ�֧�ŵ�ģ��
		//����2�������ϵ�һ���㼯��
		//����4��ָ����άƽ��
		void radiate_intersection(TriangleMesh* mesh, Pointf3s& ps ,linef3s& lines, int xyz);

		//���ڣ�2018.4.12
		//���ܣ���ά���ɸߵ�������
		//����1����Ҫ���������
		void pointf3_sort_z(std::vector<Pointf3>& ps);

		ExPolygon generate_pattern(BoundingBox box);//�����ڲ����ͼ��

		void generate_honeycomb(BoundingBoxf box, std::vector<Pointf>& ps,double radius, Pointf p,double angle,bool again);

		ExPolygon generate_honeycomb_pattern(BoundingBox box, double wall, double radius);
	};
}

#endif

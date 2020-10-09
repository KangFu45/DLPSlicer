#pragma once

#include "libslic3r.h"
#include "ExPolygon.hpp"
#include "ExPolygonCollection.hpp"
#include "Model.hpp"
#include "Point.hpp"

#include <map>

#include "TreeSupport.h"
#include "SetupDialog.h"

#include <qpolygon.h>
#include <qrect.h>
#include <qprogressbar.h>
#include <qpainter.h>

namespace DLPSlicer {

//<<<<<<< HEAD
//	class CanceledException : public std::exception {
//	public:
//		const char* what() const throw() { return "Background processing has been canceled"; }
//	};
//
//	typedef std::vector<TreeSupport*> TreeSupportPtrs;
//
//=======
//>>>>>>> parent of f6126a0... 修复bug，更新至1.6.1版本
	class DLPrint
	{
	public:
		DLPrint(Model* _model, Config* config);			//���캯��
		~DLPrint();

	private:
		class Layer {
		public:
			ExPolygonCollection slices;					//��Ƭ��
			ExPolygonCollection perimeters;				//��Ȧ�߽���
			//ExtrusionEntityCollection infill;			//�ڲ������
			ExPolygonCollection solid_infill;			//��̬�����
			float slice_z, print_z;						//��Ƭ�ߣ���ӡ��
			bool solid;									//�Ƿ�Ϊ��̬��

			Layer(float _slice_z, float _print_z)
				: slice_z(_slice_z), print_z(_print_z), solid(true) {};
		};
		typedef std::vector<Layer> Layers;

		std::vector<Layers> layerss;						//ÿ��ʵ�����󵥶���Ƭ�ֲ�
		double* m_areas = nullptr;							//ÿ��������������ģ�͵����
		Layers t_layers;//��ʱ��
		Layers r_layers;									//�װ��
		std::map <size_t, linef3s*> inside_supports;		//ÿ��ģ�Ͷ�����Ե��ڲ�֧��

		Model* model;										//ģ��
		std::map<int, TreeSupport*> treeSupports;

		//�ĸ�����֧��ö��
		enum xyz
		{
			XZ_45 = 0x01,
			XZ_135,
			YZ_45,
			YZ_135
		};

	public:
		Config* m_config;
		std::map<size_t, QPainterPath*> layer_qt_path;			//ʹ��qt����ͼ�εķ�ʽ�洢·��

		void Slice(const TriangleMeshs& supMeshs, QProgressBar* progress);
		void GenInsideSupport(size_t id, TriangleMesh* mesh);
		void InsertSupport(size_t id, TreeSupport* s);
		TreeSupport* GetTreeSupport(size_t id);
		bool DelTreeSupport(size_t id);
		bool TreeSupportsEmpty() { return treeSupports.empty(); };
		TreeSupport* GenSupport(size_t id, TriangleMesh* mesh, QProgressBar* progress);

		void DelAllInsideSup();
		void DelAllSupport();

		void SaveSlice();

		bool Finished() { return true; };
		bool Canceled() { return true; };
		
	private:
		void SaveOneSlice(size_t num, QString path);

		void SavePng(const BoundingBoxf3& box,size_t layerNum);
		void SaveOnePng(size_t num);

		void InfillLayer(size_t i, ExPolygons pattern);
		void RadiatePoint(const BoundingBoxf3& bb, Pointf3s& ps,float space,int xyz);
		void RadiateInter(const TriangleMesh* mesh, const Pointf3s& ps, linef3s& lines, int xyz);

		ExPolygon GenPattern(const BoundingBox& box);//�����ڲ����ͼ��
		ExPolygon GenHoneycombPattern(BoundingBox box, double wall, double radius);
	};
}

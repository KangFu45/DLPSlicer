#pragma once
#ifndef slic3r_DLPPrint_hpp_
#define slic3r_DLPPrint_hpp_

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

namespace Slic3r {

	class DLPrint
	{
	public:
		DLPrint(Model* _model, Config* config);			//构造函数
		~DLPrint() {};

	private:
		class Layer {
		public:
			ExPolygonCollection slices;					//切片层
			ExPolygonCollection perimeters;				//外圈边界线
			//ExtrusionEntityCollection infill;			//内部填充线
			ExPolygonCollection solid_infill;			//固态填充线
			float slice_z, print_z;						//切片高，打印高
			bool solid;									//是否为固态层

			Layer(float _slice_z, float _print_z)
				: slice_z(_slice_z), print_z(_print_z), solid(true) {};
		};
		typedef std::vector<Layer> Layers;

		std::vector<Layers*> layerPtrs;						//每个实例对象单独切片分层
		double* m_areas = nullptr;							//每层的面积，用作求模型的体积
		Layers r_layers;									//底板层
		std::map <size_t, linef3s*> inside_supports;		//每个模型对象各自的内部支撑

		Model* model;										//模型
		std::map<int, TreeSupport*> treeSupports;

		//四个方向支撑枚举
		enum xyz
		{
			XZ_45 = 0x01,
			XZ_135,
			YZ_45,
			YZ_135
		};

	public:
		Config* m_config;															

		void Slice(const TriangleMeshs& supMeshs, QProgressBar* progress);
		void GenInsideSupport(size_t id, TriangleMesh* mesh);
		void InsertSupport(size_t id, TreeSupport* s);
		TreeSupport* GetTreeSupport(size_t id);
		bool DelTreeSupport(size_t id);
		bool TreeSupportsEmpty() { return treeSupports.empty(); };
		TreeSupport* GenSupport(size_t id, TriangleMesh* mesh, QProgressBar* progress);

		void DelAllInsideSup();
		void DelAllSupport();

	private:
		void SavePng(const BoundingBoxf3& box,size_t layerNum);
		void SaveOnePng(size_t num, const std::string& path);

		void InfillLayer(size_t i, ExPolygons pattern, Layers t_layers);
		void RadiatePoint(const BoundingBoxf3& bb, Pointf3s& ps,float space,int xyz);
		void RadiateInter(const TriangleMesh* mesh, const Pointf3s& ps, linef3s& lines, int xyz);

		ExPolygon GenPattern(const BoundingBox& box);//生成内部填充图案
		ExPolygon GenHoneycombPattern(BoundingBox box, double wall, double radius);
	};
}

#endif

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

	//作者：付康
	//日期：2017
	//功能：dlp打印，保存每层的信息，执行切片，保存图片操作。
	class DLPrint
	{
	public:
		DLPrintConfig config;											//dlp打印设置
																		
		class Layer {													
		public:															
			ExPolygonCollection slices;									//切片层
			ExPolygonCollection perimeters;								//外圈边界线
			ExtrusionEntityCollection infill;							//内部填充线
			ExPolygonCollection solid_infill;							//固态填充线
			float slice_z, print_z;										//切片高，打印高
			bool solid;													//是否为固态层

			Layer(float _slice_z, float _print_z)
				: slice_z(_slice_z), print_z(_print_z), solid(true) {};
		};
		size_t layer_num;												//模型层数
		std::vector<std::vector<Layer>> layers;							//每个实例对象单独切片分层
		std::vector<double> areas;										//每层的面积，用作求模型的体积
		std::vector<Layer> _layers;										//传递每个模型Layer			
		std::vector<Layer> r_layers;									//底板层
		std::map <size_t, linef3s*> inside_supports;					//每个模型对象各自的内部支撑


		//每个模型对象的支撑
		//参数1：模型对象的id
		//参数2：模型对象的支撑数据结构(分区域)
		std::map<int, TreeSupport*> treeSupports;

		std::vector<Pointf> circle;										//单位圆
		DLPrint(Model* _model,DLPrinter* _dlprinter) ;										//构造函数

		//日期：2017
		//功能：对模型分层，抽空。
		//参数1：支撑模型
		void slice(const std::vector<TriangleMesh>& support_meshs,QProgressBar* progress);

		//日期：2017
		//功能：生成内部支撑，支撑是线形的。
		//参数1：对象id
		//参数2：模型
		void generate_inside_support(size_t id, TriangleMesh* mesh);

		void insertSupports(size_t id, TreeSupport* s);

		bool chilck_tree_support(size_t id, TreeSupport*& s);

		//日期：2017
		//功能：生成指定模型的支撑。
		//参数1：对象id
		//参数2：模型
		void generate_support(size_t id, TreeSupport*& s,TriangleMesh* mesh,QProgressBar* progress);

		//日期：2018.4.9
		//功能：删除选中树状支撑。
		void delete_tree_support(size_t id);

		//日期：2017
		//功能：保存PNG图片。
		//参数1：保存图片路径
		void savePNG(QString ini_file);

		//日期：2017
		//功能：保存一张PNG图片。
		//参数1：层数
		void saveOnePNG(size_t num);

	private:
		Model* model;													//模型
		BoundingBoxf3 bb;												//包围盒
		QString ini_file;												//切片文件路径
		QProgressBar* m_progress;
		DLPrinter* dlprinter;

		//四个方向支撑枚举
		enum xyz
		{
			XZ_45 = 10,
			XZ_135,
			YZ_45,
			YZ_135
		};

		//日期：2017
		//功能：模型实例到id。
		//参数1：模型实例
		//返回：id
		int instanceToId(ModelInstance* i);

		//日期：2017
		//功能：Polygon==>QPolygon（格式转换）。
		//参数1：待转换的多边形
		//返回：转换后的多边形
		QPolygonF polTpQpol(Polygon& p);

		//日期：2017
		//功能：填充层。
		//参数2：层数
		void _infill_layer(size_t i, const Fill* _fill, ExPolygons pattern);

		//日期：2017
		//功能：求取包围盒射线上点集合。
		//参数1：包围盒
		//参数2：返回的射线上的点
		//参数3：点的距离（支撑密度）
		//参数4：指定二维平面
		void radiate_point(BoundingBoxf3 bb, Pointf3s& ps,float space,int xyz);

		//日期：2017
		//功能：射线求交。
		//参数1:加内部支撑的模型
		//参数2：射线上的一个点集合
		//参数4：指定二维平面
		void radiate_intersection(TriangleMesh* mesh, Pointf3s& ps ,linef3s& lines, int xyz);

		//日期：2018.4.12
		//功能：三维点由高到底排序。
		//参数1：需要排序的数组
		void pointf3_sort_z(std::vector<Pointf3>& ps);

		ExPolygon generate_pattern(BoundingBox box);//生成内部填充图案

		void generate_honeycomb(BoundingBoxf box, std::vector<Pointf>& ps,double radius, Pointf p,double angle,bool again);

		ExPolygon generate_honeycomb_pattern(BoundingBox box, double wall, double radius);
	};
}

#endif

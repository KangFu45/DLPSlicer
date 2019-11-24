#pragma once

#include<Point.hpp>
#include<Line.hpp>
#include<admesh\stl.h>
#include<TriangleMesh.hpp>
#include"qprogressbar.h"

namespace Slic3r {

	//********************************
	//日期：2018.4.20
	//功能：树状支撑中树的节点结构体。
	//********************************
	typedef struct {
		size_t num;			//当前节点上面的树枝树
		Pointf3 p;			//节点的坐标位置
	}treeNode;

	typedef std::vector<Linef3> linef3s;
	typedef std::vector<std::pair<int, Linef3>> int_linef3;

	//球 --- 基础几何形状
	class Sphere
	{
	public:
		Pointf3 origin;			//球心
		double radius;			//半径
	};


	//************************
	//作者：付康
	//日期：2018.4.2
	//功能：管理树状支撑结构。
	//************************
	class TreeSupport
	{
	public:
		TreeSupport() {};
		~TreeSupport() {};

		std::vector<stl_vertex> support_point;		//待支撑悬吊点集
		std::vector<stl_vertex> support_point_face;	//悬吊面上的待支撑点集
		linef3s tree_support_branch;	//树枝
		linef3s tree_support_bole;		//树杆
		linef3s tree_support_leaf;		//树叶
		linef3s tree_support_bottom;	//树根
		Pointf3s tree_support_node;		//需要加强的节点

		void support_offset(double x, double y);

		//**************************
		//日期：2018.4.2
		//功能：生成树状支撑模型。
		//参数1：被支撑的模型
		//参数2：一颗树上树枝的数量
		//参数3：线程数
		//*************************
		void generate_tree_support(TriangleMesh& mesh, size_t leaf_num, size_t thread, QProgressBar* progress,double TDHeight);

		//*********************
		//日期：2018.4.9
		//功能：生成支撑横梁。
		//参数1：被支撑的模型
		//*********************
		void generate_support_beam(TriangleMesh& mesh);

		//***********************
		//日期：2018.4.9
		//功能：清空
		//参数1：是否清空点
		//********************
		void clear(bool point = true);

	private:

		//**********************
		//日期：2018.4.16
		//功能：生成树状结构。
		//参数1：树的节点
		//参数2：被支撑模型
		//参数3：一颗树树枝最大数量
		//**********************
		void generate_tree_support_area(size_t i, std::vector<std::vector<treeNode>> nodes, TriangleMesh& mesh, size_t leaf_num,double TDHeight);

		//*********************
		//日期：2018.4.4
		//功能：删除指定节点。
		//参数1：节点集合
		//参数2：待删除的节点
		//********************
		void delete_node(std::vector<treeNode>& node, treeNode& p);

		//*************************
		//日期：2018.4.4
		//功能：节点由高到低排序。
		//参数1：节点集合
		//*************************
		void rank_node(std::vector<treeNode>& node);

		//******************************
		//日期：2018.4.9
		//功能：在两条单位线中生成横梁。
		//参数1：直线a
		//参数2：直线b
		//参数3：生成横梁的高度
		//返回：横梁线
		//*******************************
		std::vector<Pointf3> two_line_beam(Pointf a, Pointf b, int height);
	};

}


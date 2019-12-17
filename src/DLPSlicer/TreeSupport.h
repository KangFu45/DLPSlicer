#pragma once

#include "Point.hpp"
#include "Line.hpp"
#include "admesh/stl.h"
#include "TriangleMesh.hpp"

#include <qprogressbar.h>

namespace Slic3r {

	typedef std::vector<std::pair<int, Linef3>> int_linef3;

	struct TreeNode {
		size_t num;		//当前节点上面的树枝树
		Pointf3 p;		//节点的坐标位置
	};
	typedef std::vector<TreeNode> TreeNodes;

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

		struct Paras
		{
			int leaf_num;
			int thread;
			double td_height;
		};

		//功能：生成树状支撑模型。
		//参数1：被支撑的模型
		//参数2：一颗树上树枝的数量
		//参数3：线程数
		void generate_tree_support(TriangleMesh& mesh,const Paras& paras, QProgressBar* progress);

	private:
		//功能：生成支撑横梁。
		//参数1：被支撑的模型
		void generate_support_beam(TriangleMesh& mesh);

		//功能：生成树状结构。
		//参数1：树的节点
		//参数2：被支撑模型
		//参数3：一颗树树枝最大数量
		void GenTreeSupArea(size_t i, std::vector<TreeNodes>* nodes,TriangleMesh& mesh, const Paras& paras);

		//支撑向下延伸与模型相交
		void ModelInterSupport(TriangleMesh mesh,const Pointf3& p, const Paras& paras);
	};

}


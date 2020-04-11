#pragma once

#include "Point.hpp"
#include "Line.hpp"
#include "admesh/stl.h"
#include "TriangleMesh.hpp"

#include <qprogressbar.h>

namespace DLPSlicer {

	inline void translate(stl_vertexs& vts, double x, double y, double z)
	{
		for (auto vt = vts.begin(); vt != vts.end(); ++vt) {
			vt->x += x;
			vt->y += y;
			vt->z += z;
		}
	}

	inline void translate(linef3s& lfs, double x, double y, double z)
	{
		for (auto lf = lfs.begin(); lf != lfs.end(); ++lf)
			lf->translate(x, y, z);
	}

	inline void translate(Pointf3s& pfs, double x, double y, double z)
	{
		for (auto& pf = pfs.begin(); pf != pfs.end(); ++pf)
			pf->translate(x, y, z);
	}

	struct TreeNode {
		size_t num;		//当前节点上面的树枝树
		Pointf3 p;		//节点的坐标位置
	};
	typedef std::vector<TreeNode> TreeNodes;

	class TreeSupport
	{
	public:
		TreeSupport() {};
		TreeSupport(const TreeSupport* ts);
		~TreeSupport() {};

		int id;
		stl_vertexs support_point;		//待支撑悬吊点集
		stl_vertexs support_point_face;	//悬吊面上的待支撑点集
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

		//功能：生成树状支撑模型
		//参数1：被支撑的模型
		//参数2：支撑参数
		//参数3：进度条
		void GenTreeSupport(TriangleMesh& mesh, const Paras& paras, QProgressBar* progress);

		void translate_(double x, double y,double z);

		inline void operator=(const TreeSupport* ts) {
			this->support_point = ts->support_point;
			this->support_point_face = ts->support_point_face;
			this->tree_support_bole = ts->tree_support_bole;
			this->tree_support_bottom = ts->tree_support_bottom;
			this->tree_support_branch = ts->tree_support_branch;
			this->tree_support_leaf = ts->tree_support_leaf;
			this->tree_support_node = ts->tree_support_node;
		}

	private:
		//功能：生成支撑横梁。
		//参数1：被支撑的模型
		void GenSupportBeam(const TriangleMesh& mesh);

		//功能：生成树状结构
		//参数1：区域标记
		//参数2：划分区域的一级支撑结点
		//参数3：被支撑模型
		//参数4：支撑参数
		void GenTreeSupArea(size_t i, std::vector<TreeNodes>* nodes, TriangleMesh& mesh, const Paras& paras);

		//功能：支撑点直接向下延伸
		//参数1：被支撑模型
		//参数2：直接向下生成顶端的结点
		//参数3：支撑结点集合
		//参数4：支撑参数
		void SupportPointDown(TriangleMesh mesh, const Pointf3& p1, TreeNodes& nodes, const Paras& paras);

		//功能：支撑向下延伸求与模型相交，相交给出交点，不相交即给出到平台的点
		//参数1：基础模型
		//参数2：支撑点
		//参数3：支撑参数
		void ModelInterSupport(TriangleMesh mesh, const Pointf3& p, const Paras& paras);
	};
}


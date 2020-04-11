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
		size_t num;		//��ǰ�ڵ��������֦��
		Pointf3 p;		//�ڵ������λ��
	};
	typedef std::vector<TreeNode> TreeNodes;

	class TreeSupport
	{
	public:
		TreeSupport() {};
		TreeSupport(const TreeSupport* ts);
		~TreeSupport() {};

		int id;
		stl_vertexs support_point;		//��֧�������㼯
		stl_vertexs support_point_face;	//�������ϵĴ�֧�ŵ㼯
		linef3s tree_support_branch;	//��֦
		linef3s tree_support_bole;		//����
		linef3s tree_support_leaf;		//��Ҷ
		linef3s tree_support_bottom;	//����
		Pointf3s tree_support_node;		//��Ҫ��ǿ�Ľڵ�

		struct Paras
		{
			int leaf_num;
			int thread;
			double td_height;
		};

		//���ܣ�������״֧��ģ��
		//����1����֧�ŵ�ģ��
		//����2��֧�Ų���
		//����3��������
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
		//���ܣ�����֧�ź�����
		//����1����֧�ŵ�ģ��
		void GenSupportBeam(const TriangleMesh& mesh);

		//���ܣ�������״�ṹ
		//����1��������
		//����2�����������һ��֧�Ž��
		//����3����֧��ģ��
		//����4��֧�Ų���
		void GenTreeSupArea(size_t i, std::vector<TreeNodes>* nodes, TriangleMesh& mesh, const Paras& paras);

		//���ܣ�֧�ŵ�ֱ����������
		//����1����֧��ģ��
		//����2��ֱ���������ɶ��˵Ľ��
		//����3��֧�Ž�㼯��
		//����4��֧�Ų���
		void SupportPointDown(TriangleMesh mesh, const Pointf3& p1, TreeNodes& nodes, const Paras& paras);

		//���ܣ�֧��������������ģ���ཻ���ཻ�������㣬���ཻ��������ƽ̨�ĵ�
		//����1������ģ��
		//����2��֧�ŵ�
		//����3��֧�Ų���
		void ModelInterSupport(TriangleMesh mesh, const Pointf3& p, const Paras& paras);
	};
}


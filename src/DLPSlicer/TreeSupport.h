#pragma once

#include "Point.hpp"
#include "Line.hpp"
#include "admesh/stl.h"
#include "TriangleMesh.hpp"

#include <qprogressbar.h>

namespace Slic3r {

	typedef std::vector<std::pair<int, Linef3>> int_linef3;

	struct TreeNode {
		size_t num;		//��ǰ�ڵ��������֦��
		Pointf3 p;		//�ڵ������λ��
	};
	typedef std::vector<TreeNode> TreeNodes;

	class TreeSupport
	{
	public:
		TreeSupport() {};
		~TreeSupport() {};

		std::vector<stl_vertex> support_point;		//��֧�������㼯
		std::vector<stl_vertex> support_point_face;	//�������ϵĴ�֧�ŵ㼯
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

		//���ܣ�������״֧��ģ�͡�
		//����1����֧�ŵ�ģ��
		//����2��һ��������֦������
		//����3���߳���
		void generate_tree_support(TriangleMesh& mesh,const Paras& paras, QProgressBar* progress);

	private:
		//���ܣ�����֧�ź�����
		//����1����֧�ŵ�ģ��
		void generate_support_beam(TriangleMesh& mesh);

		//���ܣ�������״�ṹ��
		//����1�����Ľڵ�
		//����2����֧��ģ��
		//����3��һ������֦�������
		void GenTreeSupArea(size_t i, std::vector<TreeNodes>* nodes,TriangleMesh& mesh, const Paras& paras);

		//֧������������ģ���ཻ
		void ModelInterSupport(TriangleMesh mesh,const Pointf3& p, const Paras& paras);
	};

}


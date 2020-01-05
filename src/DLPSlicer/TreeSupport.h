#pragma once

#include "Point.hpp"
#include "Line.hpp"
#include "admesh/stl.h"
#include "TriangleMesh.hpp"

#include <qprogressbar.h>

namespace Slic3r {

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


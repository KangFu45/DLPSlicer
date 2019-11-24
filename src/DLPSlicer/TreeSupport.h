#pragma once

#include<Point.hpp>
#include<Line.hpp>
#include<admesh\stl.h>
#include<TriangleMesh.hpp>
#include"qprogressbar.h"

namespace Slic3r {

	//********************************
	//���ڣ�2018.4.20
	//���ܣ���״֧�������Ľڵ�ṹ�塣
	//********************************
	typedef struct {
		size_t num;			//��ǰ�ڵ��������֦��
		Pointf3 p;			//�ڵ������λ��
	}treeNode;

	typedef std::vector<Linef3> linef3s;
	typedef std::vector<std::pair<int, Linef3>> int_linef3;

	//�� --- ����������״
	class Sphere
	{
	public:
		Pointf3 origin;			//����
		double radius;			//�뾶
	};


	//************************
	//���ߣ�����
	//���ڣ�2018.4.2
	//���ܣ�������״֧�Žṹ��
	//************************
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

		void support_offset(double x, double y);

		//**************************
		//���ڣ�2018.4.2
		//���ܣ�������״֧��ģ�͡�
		//����1����֧�ŵ�ģ��
		//����2��һ��������֦������
		//����3���߳���
		//*************************
		void generate_tree_support(TriangleMesh& mesh, size_t leaf_num, size_t thread, QProgressBar* progress,double TDHeight);

		//*********************
		//���ڣ�2018.4.9
		//���ܣ�����֧�ź�����
		//����1����֧�ŵ�ģ��
		//*********************
		void generate_support_beam(TriangleMesh& mesh);

		//***********************
		//���ڣ�2018.4.9
		//���ܣ����
		//����1���Ƿ���յ�
		//********************
		void clear(bool point = true);

	private:

		//**********************
		//���ڣ�2018.4.16
		//���ܣ�������״�ṹ��
		//����1�����Ľڵ�
		//����2����֧��ģ��
		//����3��һ������֦�������
		//**********************
		void generate_tree_support_area(size_t i, std::vector<std::vector<treeNode>> nodes, TriangleMesh& mesh, size_t leaf_num,double TDHeight);

		//*********************
		//���ڣ�2018.4.4
		//���ܣ�ɾ��ָ���ڵ㡣
		//����1���ڵ㼯��
		//����2����ɾ���Ľڵ�
		//********************
		void delete_node(std::vector<treeNode>& node, treeNode& p);

		//*************************
		//���ڣ�2018.4.4
		//���ܣ��ڵ��ɸߵ�������
		//����1���ڵ㼯��
		//*************************
		void rank_node(std::vector<treeNode>& node);

		//******************************
		//���ڣ�2018.4.9
		//���ܣ���������λ�������ɺ�����
		//����1��ֱ��a
		//����2��ֱ��b
		//����3�����ɺ����ĸ߶�
		//���أ�������
		//*******************************
		std::vector<Pointf3> two_line_beam(Pointf a, Pointf b, int height);
	};

}


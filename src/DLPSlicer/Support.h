#pragma once
#ifndef slic3r_Support_hpp_
#define slic3r_Support_hpp_

#include "libslic3r.h"
#include "Line.hpp"

namespace Slic3r {

	typedef std::vector<Linef3> linef3s;
	typedef std::vector<std::pair<int, Linef3>> int_linef3;

	//**************************
	//���ߣ�����
	//���ڣ�2017
	//���ܣ�����֧�ţ����ɺ�����
	//**************************
	class Support{
	public:
		linef3s support_lines;								//֧����
		linef3s support_beam;								//֧�ż����

		Support();											//Ĭ�Ϲ��캯��

		//*********************
		//���ڣ�2017
		//���ܣ�����֧�ź�����
		//*********************
		void generate_support_beam(const linef3s& line, linef3s& beam);

		//*********************
		//���ڣ�2017
		//���ܣ����һ��֧�š�
		//����1��֧����
		//********************
		void addOneSupport(Linef3 l);

		//********************
		//���ڣ�2017
		//���ܣ�ɾ��һ��֧�š�
		//����1��֧��id
		//********************
		void delOneSupport(int id);

		//*********************************
		//���ڣ�2017
		//���ܣ�����֧�ŵ㡣
		//����1��֧�ŵ�
		//���أ�true�����ڣ���false�������ڣ�
		//*********************************
		bool checkSupportPoint(Pointf3 p);

	private:
		//******************************
		//���ڣ�2017
		//���ܣ���������λ�������ɺ�����
		//����1��ֱ��a
		//����2��ֱ��b
		//����3�����ɺ����ĸ߶�
		//���أ�������
		//*******************************
		std::vector<Pointf3> two_line_beam(Pointf a, Pointf b, int height);
	};

}
#endif
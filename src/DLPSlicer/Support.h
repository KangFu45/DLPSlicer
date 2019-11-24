#pragma once
#ifndef slic3r_Support_hpp_
#define slic3r_Support_hpp_

#include "libslic3r.h"
#include "Line.hpp"

namespace Slic3r {

	typedef std::vector<Linef3> linef3s;
	typedef std::vector<std::pair<int, Linef3>> int_linef3;

	//**************************
	//作者：付康
	//日期：2017
	//功能：管理支撑，生成横梁。
	//**************************
	class Support{
	public:
		linef3s support_lines;								//支撑线
		linef3s support_beam;								//支撑间横梁

		Support();											//默认构造函数

		//*********************
		//日期：2017
		//功能：生成支撑横梁。
		//*********************
		void generate_support_beam(const linef3s& line, linef3s& beam);

		//*********************
		//日期：2017
		//功能：添加一根支撑。
		//参数1：支撑线
		//********************
		void addOneSupport(Linef3 l);

		//********************
		//日期：2017
		//功能：删除一根支撑。
		//参数1：支撑id
		//********************
		void delOneSupport(int id);

		//*********************************
		//日期：2017
		//功能：查找支撑点。
		//参数1：支撑点
		//返回：true（存在），false（不存在）
		//*********************************
		bool checkSupportPoint(Pointf3 p);

	private:
		//******************************
		//日期：2017
		//功能：在两条单位线中生成横梁。
		//参数1：直线a
		//参数2：直线b
		//参数3：生成横梁的高度
		//返回：横梁线
		//*******************************
		std::vector<Pointf3> two_line_beam(Pointf a, Pointf b, int height);
	};

}
#endif
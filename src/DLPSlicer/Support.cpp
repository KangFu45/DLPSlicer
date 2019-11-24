#include"Support.h"
#include"tool.h"

namespace Slic3r{

	Support::Support()
	{
	}

	void Support::generate_support_beam(const linef3s& line, linef3s& beam)//生成支撑横梁
	{
		int_linef3 lines_i;//对支撑线的横梁计数
		for (auto l = line.begin(); l != line.end(); ++l){
			float dis = (*l).b.z - (*l).a.z;
			//大于30mm的支撑不需要横梁
			if (dis>30)
				lines_i.push_back(std::make_pair(0, *l));
		}
		for (int_linef3::iterator _l = lines_i.begin(); _l != lines_i.end(); ++_l) {
			std::pair<int, Linef3> l = *_l;
			std::map<float, int> lines_0;//可以连接的支撑线
			if (l.first == 0) {//没有连接横梁的支撑去加横梁
				for (int_linef3::iterator _l1 = lines_i.begin(); _l1 != lines_i.end(), _l1 != _l; ++_l1) {
					float dis = distance_point(Pointf(l.second.a.x, l.second.a.y), Pointf((*_l1).second.a.x, (*_l1).second.a.y));
					if (dis < 30) { //控制可生成支撑柱之间的距离范围
						int len = std::distance(lines_i.begin(), _l1);
						if (_l1->first == 0 || _l1->first == 1) {
							lines_0.insert(std::make_pair(dis, len));
						}
					}
				}

				if (lines_0.size() > 0) {
					std::map<float, int>::iterator m = lines_0.begin();
					int_linef3::iterator _l2 = lines_i.begin() + (*m).second;
					++_l2->first;//横梁数加一
					std::pair<int, Linef3> l2 = *_l2;
					Linef3 line1 = l.second;
					Linef3 line2 = l2.second;

					//排除挨在一起的支撑
				//	if (distance_point(Pointf(line1.a.x, line1.a.y), Pointf(line2.b.x, line2.b.y))<0.4)
				//		continue;

					//生成单位横梁
					std::vector<Pointf3> ps = two_line_beam(Pointf(line1.a.x, line1.a.y), Pointf(line2.a.x, line2.a.y), 200);
					std::vector<Pointf3> ps1;

					//挑选横梁
					for (std::vector<Pointf3>::iterator _p3 = ps.begin(); _p3 != ps.end(); ++_p3) {
						Pointf3 p3 = *_p3;
						if (line1.a.x == p3.x&&line1.a.y == p3.y&&line1.a.z >= 0 && line1.b.z < 300) {
							if (p3.z > line1.a.z&&p3.z < line1.b.z)
								ps1.push_back(p3);
						}
						if (line2.a.x == p3.x&&line2.a.y == p3.y&&line2.a.z >= 0 && line2.b.z < 300) {
							if (p3.z > line2.a.z&&p3.z < line2.b.z)
								ps1.push_back(p3);
						}
					}

					if (ps1.size() > 1) {
						int a = ps1.size() / 5 + 2;//控制横梁数量
						for (std::vector<Pointf3>::iterator p = ps1.begin(); p != ps1.end() - 1; ++p) {
							int b = std::distance(ps1.begin(), p);
							if (b % a == 0)
								beam.push_back(Linef3(*p, *(p + 1)));
						}
					}
				}
			}
		}
	}
	
	std::vector<Pointf3> Support::two_line_beam(Pointf a, Pointf b, int height)//生成单位横梁
	{
		std::vector<Pointf3> ps;
		float dis = distance_point(a, b);
		if (dis == 0)
			return ps;
		int k = height / dis;
		for (int i = 0; i<k; ++i){
			if (i % 2 == 0){
				ps.push_back(Pointf3(a.x, a.y, dis*i));
			}
			else{
				ps.push_back(Pointf3(b.x, b.y, dis*i));
			}
		}
		return ps;
	}

	void Support::addOneSupport(Linef3 l)
	{
		support_lines.push_back(l);
	}

	void Support::delOneSupport(int id)
	{
		auto s = support_lines.begin() + id;
		support_lines.erase(s);
	}

	bool Support::checkSupportPoint(Pointf3 p)
	{
		for (auto s = support_lines.begin(); s != support_lines.end(); ++s) {
			if ((*s).b.x == p.x && (*s).b.y == p.y && (*s).b.z == p.z)
				return true;
		}
		return false;
	}
}
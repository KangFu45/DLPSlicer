#include "TreeSupport.h"
#include "tool.h"

#include <qdebug.h>

namespace Slic3r {

	void TreeSupport::generate_tree_support(TriangleMesh& mesh, const Paras& paras, QProgressBar* progress)
	{
		//进度条范围35->81
		double MinHeight = 1.0;
		std::vector<TreeNode> node;//树的节点（不会存储树的结构关系）

		//-----------------悬吊面上待支撑点生成顶端与一级树节点--------------------
		std::vector<stl_vertex> deleteVector;
		for (auto p = support_point_face.begin(); p != support_point_face.end(); ++p) {
			size_t id = std::distance(support_point_face.begin(), p);
			int unit = (id + 1) / (support_point_face.size() / (60 - 35) + 1);
			if (progress->value() < 35 + unit)
				progress->setValue(35 + unit);

			stl_facet f, face;
			int i1;
			int count = 0;

			Pointf3 p1((*p).x, (*p).y, (*p).z);

			//-----------------求包含支撑点的三角面片-------------------------
			//支撑点数据直接带所在三角面片即可省去一段求所在三角面片的代码！！！
			bool ret = false;
			double dis = 1;
			for (int i = 0; i < mesh.stl.stats.number_of_facets; ++i) {
				f = mesh.stl.facet_start[i];
				//判断是否是三角面的顶点或在三角面上
				if ((f.vertex[0].x == p1.x && f.vertex[0].y == p1.y && f.vertex[0].z == p1.z) ||
					(f.vertex[1].x == p1.x && f.vertex[1].y == p1.y && f.vertex[1].z == p1.z) ||
					(f.vertex[2].x == p1.x && f.vertex[2].y == p1.y && f.vertex[2].z == p1.z))
				{
					ret = true;//当前点为悬吊点
					i1 = i;
					face = f;
				}

				if (PointinTriangle1(Pointf3(f.vertex[0].x, f.vertex[0].y, f.vertex[0].z),
					Pointf3(f.vertex[1].x, f.vertex[1].y, f.vertex[1].z),
					Pointf3(f.vertex[2].x, f.vertex[2].y, f.vertex[2].z), p1)) {

					Pointf3 t = CalPlaneLineIntersectPoint(Vectorf3(f.normal.x, f.normal.y, f.normal.z), Pointf3(f.vertex[0].x, f.vertex[0].y, f.vertex[0].z),
						Vectorf3(-f.normal.x, -f.normal.y, -f.normal.z), p1);
					double dis1 = distance_point_3(t, p1);
					if (!ret && dis1 < 0.1) {
						if (dis1 == 0) {
							i1 = i;
							face = f;
							break;
						}
						else if (dis1 < dis) {
							dis = dis1;
							i1 = i;
							face = f;
						}
					}
				}

			}
			//-------------------------------------
			Pointf3 p2;
			p2.x = face.normal.x * paras.td_height + p1.x;
			p2.y = face.normal.y * paras.td_height + p1.y;
			p2.z = face.normal.z * paras.td_height + p1.z;

			bool chlik = true;//判断是否检查延伸与模型相交
			bool down = false;//支撑点是否向下延伸
			//检查（如果发生碰撞，则向Z轴负方向延伸）
			if (chlik) {
				Pointf3 a;
				for (int i = 0; i < mesh.stl.stats.number_of_facets; ++i) {
					f = mesh.stl.facet_start[i];
					if (i1 != i) {
						if (line_to_triangle_bool(f, Linef3(p1, p2))) {
							down = true;
							break;
						}
					}
				}
			}

			if (!down) {//向其法向量方向延伸
				double ccc = face.normal.x + face.normal.y + face.normal.z;
				if (p2.z >= 0 && ccc < 2) {
					Pointf3 p3(-face.normal.x * 0.1 + p1.x, -face.normal.y * 0.1 + p1.y, -face.normal.z * 0.1 + p1.z);//延伸0.1mm
					tree_support_leaf.push_back(Linef3(p2, p3));
					//存储树的节点
					TreeNode n = { 1,p2 };
					node.push_back(n);

					if (p2.z > 0.5)//防止加强求穿过底板
						tree_support_node.push_back(p2);
				}
				else
					down = true;
			}

			if (down) {//向下延伸
				p2 = mesh.point_model_intersection_Z(p1);
				double dis = p1.z - p2.z;

				if (dis < MinHeight) {//删除过短的线段
					deleteVector.push_back(*p);
				}
				else if (dis < paras.td_height * 2 && dis >= MinHeight) {//小于顶端距离直接分两段处理
					dis /= 2.0;
					Pointf3 p3(p1.x, p1.y, p1.z - dis);
					tree_support_leaf.push_back(Linef3(p3, p1));
					tree_support_bottom.push_back(Linef3(p2, p3));
				}
				//生成顶端并添加树节点
				else {
					p2.x = p1.x;
					p2.y = p1.y;
					p2.z = p1.z - paras.td_height * 2;

					tree_support_leaf.push_back(Linef3(p2, p1));
					//存储树的节点
					TreeNode n = { 1,p2 };
					node.push_back(n);

					if (p2.z > 0.5)//防止加强求穿过底板
						tree_support_node.push_back(p2);
				}
			}
		}

		//删除过短线段的支撑点
		for (auto d = deleteVector.begin(); d != deleteVector.end(); ++d) {
			int dis = -1;
			for (auto t = support_point_face.begin(); t != support_point_face.end(); ++t) {
				if (equal_vertex(*t, *d))
					dis = std::distance(support_point_face.begin(), t);
			}
			support_point_face.erase(support_point_face.begin() + dis);
		}
		deleteVector.clear();

		//-----------------悬吊点生成顶端与一级树节点--------------------
		for (auto p = support_point.begin(); p != support_point.end(); ++p) {
			size_t id = std::distance(support_point.begin(), p);
			int unit = (id + 1) / (support_point.size() / (75 - 61) + 1);
			if (progress->value() < 61 + unit)
				progress->setValue(61 + unit);

			Pointf3 p1((*p).x, (*p).y, (*p).z);
			Pointf3 p2 = mesh.point_model_intersection_Z(p1);
			double dis = p1.z - p2.z;
			if (dis < MinHeight) {//删除过短的线段
				deleteVector.push_back(*p);
			}
			else if (dis <= paras.td_height * 2 && dis >= MinHeight) {//小于顶端距离直接分两段处理
				dis /= 2.0;
				Pointf3 p3(p1.x, p1.y, p1.z - dis);
				tree_support_leaf.push_back(Linef3(p3, p1));
				tree_support_bottom.push_back(Linef3(p2, p3));
			}
			//生成顶端并添加树节点
			else {
				p2.x = p1.x;
				p2.y = p1.y;
				p2.z = p1.z - paras.td_height * 2;

				tree_support_leaf.push_back(Linef3(p2, p1));
				//存储树的节点
				TreeNode n = { 1,p2 };
				node.push_back(n);

				if (p2.z > 0.5)//防止加强求穿过底板
					tree_support_node.push_back(p2);
			}
		}

		//删除过短线段的支撑点
		for (auto d = deleteVector.begin(); d != deleteVector.end(); ++d) {
			int dis = -1;
			for (auto t = support_point.begin(); t != support_point.end(); ++t) {
				if (equal_vertex(*t, *d))
					dis = std::distance(support_point.begin(), t);
			}
			support_point.erase(support_point.begin() + dis);
		}
		deleteVector.clear();

		//划分区域
		BoundingBoxf3 box = mesh.bounding_box();
		box.min.x -= paras.td_height;
		box.min.y -= paras.td_height;
		box.max.x += paras.td_height;
		box.max.y += paras.td_height;

		size_t a = 2;
		std::vector<float> x, y;
		float eachx = (box.max.x - box.min.x) / a;
		float eachy = (box.max.y - box.min.y) / a;
		for (int j = 0; j < a; ++j) {
			x.push_back(box.min.x + j * eachx);
			y.push_back(box.min.y + j * eachy);
		}
		x.push_back(box.max.x + 1);
		y.push_back(box.max.y + 1);

		std::vector<Linef> areas;
		for (auto i = y.begin(); i != y.end() - 1; ++i) {
			for (auto j = x.begin(); j != x.end() - 1; ++j) {
				areas.push_back(Linef(Pointf(*j, *i), Pointf(*(j + 1), *(i + 1))));
			}
		}

		//对节点进行分区域
		std::vector<std::vector<TreeNode>> nodes;
		for (auto a = areas.begin(); a != areas.end(); ++a) {
			std::vector<TreeNode> ps;
			for (auto n = node.begin(); n != node.end(); ++n) {
				if ((*n).p.x >= (*a).a.x && (*n).p.x < (*a).b.x &&
					(*n).p.y >= (*a).a.y && (*n).p.y < (*a).b.y) {
					ps.push_back(*n);
				}
			}
			nodes.push_back(ps);
		}

		progress->setValue(75);

		size_t thread = paras.thread > 1 ? paras.thread : 1;

		//for (int i = 0; i < nodes.size(); ++i) {
		//	GenTreeSupArea(i, nodes, mesh, leaf_num, TDHeight);
		//}

		//if (nodes.size() < 100) {
		parallelize<size_t>(
			0,
			nodes.size() - 1,
			boost::bind(&TreeSupport::GenTreeSupArea, this, _1, &nodes, mesh, paras),
			thread
			);
		//}

		generate_support_beam(mesh);
	}

	//将节点按z轴方向按降序排列
	inline void RankNode(TreeNodes& nodes)
	{
		if (nodes.empty())
			return;

		TreeNode t_node;
		for (int i = 0; i < nodes.size() - 1; ++i) {
			for (auto p2 = nodes.begin() + 1; p2 != nodes.end(); ++p2) {
				if ((*p2).p.z > (*(p2 - 1)).p.z) {
					t_node = *p2;
					*p2 = *(p2 - 1);
					*(p2 - 1) = t_node;
				}
			}
		}
	}

	//从节点集里删除指定节点
	void DeleteNode(TreeNodes& nodes, const TreeNode& node)
	{
		for (auto i = nodes.begin(); i != nodes.end(); ++i) {
			if (node.p == (*i).p) {
				nodes.erase(i);
				return;
			}
		}
	}

	inline stl_facet PointFindTrangle(const TriangleMesh* mesh, Pointf3 p)
	{
		stl_facet face;
		bool ret = false;
		double dis2 = 1;
		//求包含支撑点的三角面片
		for (int i = 0; i < mesh->stl.stats.number_of_facets; ++i) {
			stl_facet f = mesh->stl.facet_start[i];
			//判断是否是三角面的顶点或在三角面上
			if (p == f.vertex[0] || p == f.vertex[1] || p == f.vertex[2]) {
				ret = true;//当前点为悬吊点
				face = f;
			}

			if (PointinTriangle1(f, p)) {
				double dis1 = distance_point_3(CalPlaneLineIntersectPoint(Nor2Vt3(f.normal), Vec2Pf3(f.vertex[0])
					, Nor2Vt3(f.normal).Reverse(), p), p);
				if (!ret && dis1 < 0.1) {
					if (dis1 == 0) {
						face = f;
						break;
					}
					else if (dis1 < dis2) {
						dis2 = dis1;
						face = f;
					}
				}
			}
		}
		return face;
	}

	void TreeSupport::ModelInterSupport(TriangleMesh mesh, const Pointf3& p, const Paras& paras)
	{
		//-------------向下延伸时判断底端是否与模型成角度----------------
		Pointf3 bottom = mesh.point_model_intersection_Z(p);
		if (bottom.z == 0) {//直接延伸至平台
			//删除不需要的增强的节点
			//if (MaxNode.num == 1) {
			//	int dis = -1;
			//	for (auto t = tree_support_node.begin(); t != tree_support_node.end(); ++t) {
			//		if ((*t).x == MaxNode.p.x && (*t).y == MaxNode.p.y && (*t).z == MaxNode.p.z)
			//			dis = std::distance(tree_support_node.begin(), t);
			//	}
			//	tree_support_node.erase(tree_support_node.begin() + dis);
			//}
			if (p.z > paras.td_height) {
				Pointf3 p1(bottom.x, bottom.y, paras.td_height);
				tree_support_bottom.emplace_back(Linef3(bottom, p1));
				tree_support_bole.emplace_back(Linef3(p1, p));
			}
			else
				tree_support_bottom.emplace_back(Linef3(bottom, p));
		}
		else {
			//与模型相交

			Pointf3 p1;
			double angle;
			bool down = false;
			double hei = paras.td_height;

			stl_facet face = PointFindTrangle(&mesh, bottom);
			//通过底端长度来降低树干的角度
			if (180 / PI * vector_angle_3D(Vectorf3(0, 0, -1), Nor2Vt3(face.normal)) >= 90) {
				do
				{
					p1.x = face.normal.x * hei + bottom.x;
					p1.y = face.normal.y * hei + bottom.y;
					p1.z = face.normal.z * hei + bottom.z;

					float* v = (p1 - p).data();
					stl_normalize_vector(v);

					if (v[0] != 0 && v[1] != 0 && v[2] != 0) {
						angle = 180 / PI * vector_angle_3D(Vectorf3(0, 0, -1), Vectorf3(v));

						if (hei >= 0.5)
							hei -= 0.5;
						else
							down = true;
					}
					else
						down = true;

					delete v;
				} while (angle > 30 && !down);
			}
			else
				down = true;

			if (!down) {
				tree_support_bole.emplace_back(Linef3(p1, p));
				tree_support_bottom.emplace_back(Linef3(bottom, p1));
				tree_support_node.emplace_back(p);
				tree_support_node.emplace_back(p1);
			}
			else
			{
				if (p.z - bottom.z > paras.td_height) {
					Pointf3 p2(bottom.x, bottom.y, paras.td_height + bottom.z);
					tree_support_bottom.emplace_back(Linef3(bottom, p2));
					tree_support_bole.emplace_back(Linef3(p2, p));
				}
				else {
					tree_support_bottom.emplace_back(Linef3(bottom, p));
				}
			}
		}
	}

	void TreeSupport::GenTreeSupArea(size_t i, std::vector<TreeNodes>* nodess, TriangleMesh& mesh, const Paras& paras)
	{
		//------------生成树状结构-------------
		TreeNodes nodes = *(nodess->begin() + i);

		//由高到底排序
		RankNode(nodes);

		while (!nodes.empty())
		{
			//选择节点最高点
			TreeNode MaxNode = nodes.front();
			nodes.erase(nodes.begin());

			//选择两点之间距离小于lenght的点
			std::map<float, TreeNode> dis_nodes;
			for each (const TreeNode & p3 in nodes) {
				//限制树枝数量
				if (p3.num + MaxNode.num <= paras.leaf_num) {
					float dis = distance_point_3(MaxNode.p, p3.p);
					if (dis <= 30)
						dis_nodes.insert(std::make_pair(dis, p3));
				}
			}

			//最高节点与距离最近的节点进行配对。
			//以两个节点为圆锥顶点，以约束角度(45度)为最大角度的圆锥，求两点与圆锥交点最短距离的交点。
			bool ok = false;//判断配对是否成功
			for (auto p4 = dis_nodes.begin(); p4 != dis_nodes.end(); ++p4) {
				TreeNode SecondNode = (*p4).second;
				//求射线的法向量,先得到xy平面上的法向量，再在z轴方向上旋转约束角。
				float MN[3], SN[3];//两个射线的法向量
				MN[0] = SecondNode.p.x - MaxNode.p.x; MN[1] = SecondNode.p.y - MaxNode.p.y; MN[2] = 0.0;
				stl_normalize_vector(MN);

				SN[0] = MaxNode.p.x - SecondNode.p.x; SN[1] = MaxNode.p.y - SecondNode.p.y; SN[2] = 0.0;
				stl_normalize_vector(SN);

				MN[2] = -0.7;
				SN[2] = -0.7;

				stl_normalize_vector(MN);
				stl_normalize_vector(SN);

				//z方向旋转45度
				//XY_normal_sacle_Z(MN, 45);
				//XY_normal_sacle_Z(SN, 45);


				//是否判断两射线共面？？？
				//求过第二条射线平面的法向量
				Vectorf3 MNV(MN[0], MN[1], MN[2]);
				Vectorf3 SNV(SN[0], SN[1], SN[2]);

				Vectorf3 SNNV1 = SNV.Cross(MNV);
				Vectorf3 SNNV = SNV.Cross(SNNV1);

				//求交点
				Pointf3 NextNode = CalPlaneLineIntersectPoint(SNNV, SecondNode.p, MNV, MaxNode.p);

				//射线求交发生错误(跳过)
				if (NextNode.z > MaxNode.p.z || NextNode.z > SecondNode.p.z || NextNode.z <= 0)
					continue;

				//判断两个树枝是否穿过模型
				bool across = false;
				if (NextNode.z > 1) {//节点高度小于1会穿过底板
					for (int i = 0; i < mesh.stl.stats.number_of_facets; ++i) {
						stl_facet f = mesh.stl.facet_start[i];
						//--------------两个树枝--------------
						if (line_to_triangle_bool(f, Linef3(NextNode, MaxNode.p)) ||
							line_to_triangle_bool(f, Linef3(NextNode, SecondNode.p))) {
							across = true;
							break;
						}
					}
				}
				else
					across = true;

				if (!across) {
					//------未穿过------
					//保存两个树枝
					tree_support_branch.emplace_back(Linef3(NextNode, MaxNode.p));
					tree_support_branch.emplace_back(Linef3(NextNode, SecondNode.p));

					//增加一个新的节点
					TreeNode n{ MaxNode.num + SecondNode.num ,NextNode };

					tree_support_node.emplace_back(NextNode);

					//控制树枝的数量
					if (n.num >= paras.leaf_num)
						ModelInterSupport(mesh, n.p, paras);
					else {
						nodes.emplace_back(n);
						RankNode(nodes);
					}
					DeleteNode(nodes, SecondNode);
					//配对成功
					ok = true;
					break;
				}
			}

			//配对未成功
			if (!ok)
				ModelInterSupport(mesh, MaxNode.p, paras);
		}
	}

	//a,b点作xy平面垂线，height为横梁高度,生成单位横梁
	inline Pointf3s TwoLineBeam(Pointf a, Pointf b, int height)
	{
		float dis = distance_point(a, b);
		if (dis == 0)
			return Pointf3s();

		Pointf3s ps;
		int k = height / dis;
		for (int i = 0; i < k; ++i) {
			if (i % 2 == 0)
				ps.emplace_back(Pointf3(a.x, a.y, dis * i));
			else
				ps.emplace_back(Pointf3(b.x, b.y, dis * i));
		}
		return ps;
	}

	void TreeSupport::generate_support_beam(TriangleMesh& mesh)//生成支撑横梁
	{
		int_linef3 lines_i;//对支撑线的横梁计数
		for (auto l = tree_support_bole.begin(); l != tree_support_bole.end(); ++l) {
			if ((*l).a.x == (*l).b.x && (*l).a.y == (*l).b.y) {
				float dis = (*l).b.z - (*l).a.z;
				//大于30mm的支撑不需要横梁
				if (dis > 30)
					lines_i.push_back(std::make_pair(0, *l));
			}
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

					//生成单位横梁
					Pointf3s ps = TwoLineBeam(Pointf(line1.a.x, line1.a.y), Pointf(line2.a.x, line2.a.y), 300);
					Pointf3s ps1;

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
						//size_t a = ps1.size() / 5 + 1;//控制横梁数量
						for (std::vector<Pointf3>::iterator p = ps1.begin(); p != ps1.end() - 1; ++p) {
							int b = std::distance(ps1.begin(), p);
							//if (b % a == 0) {
								//判断横梁是否穿过模型
								Linef3 line(*p, *(p + 1));
								bool across = false;

								if (distance_point_3(line.a, line.b) > 10) {
									for (int i = 0; i < mesh.stl.stats.number_of_facets; ++i) {
										if (line_to_triangle_bool(mesh.stl.facet_start[i], line)) {
											across = true;
											break;
										}
									}
								}
								
								if (!across) {
									tree_support_branch.push_back(line);
								}
							//}
						}
					}
				}
			}
		}
	}
}


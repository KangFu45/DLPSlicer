#include "TreeSupport.h"
#include "tool.h"
#include "qdebug.h"

namespace Slic3r {

	void TreeSupport::support_offset(double x, double y)
	{
		for (auto t = tree_support_branch.begin(); t != tree_support_branch.end(); ++t) {
			(*t).a.translate(x, y, 0);
			(*t).b.translate(x, y, 0);
		}

		for (auto t = tree_support_bole.begin(); t != tree_support_bole.end(); ++t) {
			(*t).a.translate(x, y, 0);
			(*t).b.translate(x, y, 0);
		}

		for (auto t = tree_support_leaf.begin(); t != tree_support_leaf.end(); ++t) {
			(*t).a.translate(x, y, 0);
			(*t).b.translate(x, y, 0);
		}

		for (auto t = tree_support_bottom.begin(); t != tree_support_bottom.end(); ++t) {
			(*t).a.translate(x, y, 0);
			(*t).b.translate(x, y, 0);
		}

		for (auto t = tree_support_node.begin(); t != tree_support_node.end(); ++t) {
			(*t).translate(x, y, 0);
		}
	}

	void TreeSupport::generate_tree_support(TriangleMesh& mesh, size_t leaf_num, size_t thread, QProgressBar* progress, double TDHeight)
	{
		//��������Χ35->81
		double MinHeight = 1.0;
		std::vector<treeNode> node;//���Ľڵ㣨����洢���Ľṹ��ϵ��

		//-----------------�������ϴ�֧�ŵ����ɶ�����һ�����ڵ�--------------------
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

			//-----------------�����֧�ŵ��������Ƭ-------------------------
			//֧�ŵ�����ֱ�Ӵ�����������Ƭ����ʡȥһ��������������Ƭ�Ĵ��룡����
			bool ret = false;
			double dis = 1;
			for (int i = 0; i < mesh.stl.stats.number_of_facets; ++i) {
				f = mesh.stl.facet_start[i];
				//�ж��Ƿ���������Ķ��������������
				if ((f.vertex[0].x == p1.x&&f.vertex[0].y == p1.y&&f.vertex[0].z == p1.z) ||
					(f.vertex[1].x == p1.x&&f.vertex[1].y == p1.y&&f.vertex[1].z == p1.z) ||
					(f.vertex[2].x == p1.x&&f.vertex[2].y == p1.y&&f.vertex[2].z == p1.z))
				{
					ret = true;//��ǰ��Ϊ������
					i1 = i;
					face = f;
				}
			
				if (PointinTriangle1(Pointf3(f.vertex[0].x, f.vertex[0].y, f.vertex[0].z),
					Pointf3(f.vertex[1].x, f.vertex[1].y, f.vertex[1].z),
					Pointf3(f.vertex[2].x, f.vertex[2].y, f.vertex[2].z), p1)) {
					
					Pointf3 t = CalPlaneLineIntersectPoint(Vectorf3(f.normal.x, f.normal.y, f.normal.z), Pointf3(f.vertex[0].x, f.vertex[0].y, f.vertex[0].z),
						Vectorf3(-f.normal.x, -f.normal.y, -f.normal.z), p1);
					double dis1= distance_point_3(t, p1);
					if (!ret&&dis1 < 0.1) {
						if (dis1 == 0) {
							i1 = i;
							face = f;
							break;
						}
						else if(dis1<dis){
							dis = dis1;
							i1 = i;
							face = f;
						}
					}
				}
			
			}
			//-------------------------------------
			Pointf3 p2;
			p2.x = face.normal.x*TDHeight + p1.x;
			p2.y = face.normal.y*TDHeight + p1.y;
			p2.z = face.normal.z*TDHeight + p1.z;

			bool chlik = true;//�ж��Ƿ���������ģ���ཻ
			bool down = false;//֧�ŵ��Ƿ���������
			//��飨���������ײ������Z�Ḻ�������죩
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

			if (!down) {//���䷨������������
				double ccc = face.normal.x + face.normal.y + face.normal.z;
				if (p2.z >= 0 && ccc < 2) {
					Pointf3 p3(-face.normal.x*0.1 + p1.x, -face.normal.y*0.1 + p1.y, -face.normal.z*0.1 + p1.z);//����0.1mm
					tree_support_leaf.push_back(Linef3(p2, p3));
					//�洢���Ľڵ�
					treeNode n = { 1,p2 };
					node.push_back(n);

					if (p2.z > 0.5)//��ֹ��ǿ�󴩹��װ�
						tree_support_node.push_back(p2);
				}
				else
					down = true;
			}

			if(down) {//��������
				p2 = mesh.point_model_intersection_Z(p1);
				double dis = p1.z - p2.z;

				if (dis < MinHeight) {//ɾ�����̵��߶�
					deleteVector.push_back(*p);
				}
				else if (dis < TDHeight * 2&&dis>= MinHeight) {//С�ڶ��˾���ֱ�ӷ����δ���
					dis /= 2.0;
					Pointf3 p3(p1.x, p1.y, p1.z - dis);
					tree_support_leaf.push_back(Linef3(p3, p1));
					tree_support_bottom.push_back(Linef3(p2, p3));
				}
				//���ɶ��˲�������ڵ�
				else {
					p2.x = p1.x;
					p2.y = p1.y;
					p2.z = p1.z - TDHeight * 2;

					tree_support_leaf.push_back(Linef3(p2, p1));
					//�洢���Ľڵ�
					treeNode n = { 1,p2 };
					node.push_back(n);

					if (p2.z > 0.5)//��ֹ��ǿ�󴩹��װ�
						tree_support_node.push_back(p2);
				}
			}
		}

		//ɾ�������߶ε�֧�ŵ�
		for (auto d = deleteVector.begin(); d != deleteVector.end(); ++d) {
			int dis = -1;
			for (auto t = support_point_face.begin(); t != support_point_face.end(); ++t) {
				if (equal_vertex(*t,*d))
					dis = std::distance(support_point_face.begin(), t);
			}
			support_point_face.erase(support_point_face.begin() + dis);
		}
		deleteVector.clear();

		//-----------------���������ɶ�����һ�����ڵ�--------------------
		for (auto p = support_point.begin(); p != support_point.end(); ++p) {
			size_t id = std::distance(support_point.begin(), p);
			int unit = (id + 1) / (support_point.size() / (75 - 61) + 1);
			if (progress->value() < 61 + unit)
				progress->setValue(61 + unit);

			Pointf3 p1((*p).x, (*p).y, (*p).z);
			Pointf3 p2 = mesh.point_model_intersection_Z(p1);
			double dis = p1.z - p2.z;
			if (dis < MinHeight) {//ɾ�����̵��߶�
				deleteVector.push_back(*p);
			}
			else if (dis <= TDHeight * 2&&dis>=MinHeight) {//С�ڶ��˾���ֱ�ӷ����δ���
				dis /= 2.0;
				Pointf3 p3(p1.x, p1.y, p1.z - dis);
				tree_support_leaf.push_back(Linef3(p3, p1));
				tree_support_bottom.push_back(Linef3(p2, p3));
			}
			//���ɶ��˲�������ڵ�
			else {
				p2.x = p1.x;
				p2.y = p1.y;
				p2.z = p1.z - TDHeight * 2;

				tree_support_leaf.push_back(Linef3(p2, p1));
				//�洢���Ľڵ�
				treeNode n = { 1,p2 };
				node.push_back(n);

				if (p2.z > 0.5)//��ֹ��ǿ�󴩹��װ�
					tree_support_node.push_back(p2);
			}
		}

		//ɾ�������߶ε�֧�ŵ�
		for (auto d = deleteVector.begin(); d != deleteVector.end(); ++d) {
			int dis = -1;
			for (auto t = support_point.begin(); t != support_point.end(); ++t) {
				if (equal_vertex(*t, *d))
					dis = std::distance(support_point.begin(), t);
			}
			support_point.erase(support_point.begin() + dis);
		}
		deleteVector.clear();

		//��������
		BoundingBoxf3 box = mesh.bounding_box();
		box.min.x -= TDHeight;
		box.min.y -= TDHeight;
		box.max.x += TDHeight;
		box.max.y += TDHeight;

		size_t a = 2;
		std::vector<float> x, y;
		float eachx = (box.max.x - box.min.x) / a;
		float eachy = (box.max.y - box.min.y) / a;
		for (int j = 0; j < a; ++j) {
			x.push_back(box.min.x + j*eachx);
			y.push_back(box.min.y + j*eachy);
		}
		x.push_back(box.max.x + 1);
		y.push_back(box.max.y + 1);

		std::vector<Linef> areas;
		for (auto i = y.begin(); i != y.end() - 1; ++i) {
			for (auto j = x.begin(); j != x.end() - 1; ++j) {
				areas.push_back(Linef(Pointf(*j, *i), Pointf(*(j + 1), *(i + 1))));
			}
		}

		//�Խڵ���з�����
		std::vector<std::vector<treeNode>> nodes;
		for (auto a = areas.begin(); a != areas.end(); ++a) {
			std::vector<treeNode> ps;
			for (auto n = node.begin(); n != node.end(); ++n) {
				if ((*n).p.x >= (*a).a.x && (*n).p.x < (*a).b.x &&
					(*n).p.y >= (*a).a.y && (*n).p.y < (*a).b.y) {
					ps.push_back(*n);
				}
			}
			nodes.push_back(ps);
		}

		progress->setValue(75);

		thread = thread > 1 ? thread : 1;

		//for (int i = 0; i < nodes.size(); ++i) {
		//	generate_tree_support_area(i, nodes, mesh, leaf_num, TDHeight);
		//}

		//if (nodes.size() < 100) {
			parallelize<size_t>(
				0,
				nodes.size()-1,
				boost::bind(&TreeSupport::generate_tree_support_area, this, _1, nodes, mesh, leaf_num,TDHeight),
				thread
				);
		//}

		generate_support_beam(mesh);
	}

	void TreeSupport::generate_tree_support_area(size_t i, std::vector<std::vector<treeNode>> nodes, TriangleMesh& mesh, size_t leaf_num, double TDHeight)
	{
		//------------������״�ṹ-------------
		std::vector<treeNode> node = *(nodes.begin() + i);
		treeNode n;

		//�ɸߵ�������
		rank_node(node);

		while (!node.empty())
		{
			//ѡ��ڵ���ߵ�
			treeNode MaxNode = *(node.begin());

			//ѡ������֮�����С��lenght�ĵ�
			std::map<float, treeNode> DisNode;
			float dis;
			for (auto p3 = node.begin(); p3 != node.end(); ++p3) {
				if ((*p3).p != MaxNode.p) {
					//������֦����
					if ((*p3).num + MaxNode.num <= leaf_num) {
						dis = distance_point_3(MaxNode.p, (*p3).p);
						if (dis <= 30)
							DisNode.insert(std::make_pair(dis, *p3));
					}
				}
			}

			//��߽ڵ����������Ľڵ������ԡ�
			//�������ڵ�ΪԲ׶���㣬��Լ���Ƕ�(45��)Ϊ���Ƕȵ�Բ׶����������Բ׶������̾���Ľ��㡣
			bool ok = false;//�ж�����Ƿ�ɹ�
			for (auto p4 = DisNode.begin(); p4 != DisNode.end(); ++p4) {
				treeNode SecondNode = (*p4).second;
				//�����ߵķ�����,�ȵõ�xyƽ���ϵķ�����������z�᷽������תԼ���ǡ�
				float MN[3], SN[3];//�������ߵķ�����
				MN[0] = SecondNode.p.x - MaxNode.p.x; MN[1] = SecondNode.p.y - MaxNode.p.y; MN[2] = 0.0;
				stl_normalize_vector(MN);

				SN[0] = MaxNode.p.x - SecondNode.p.x; SN[1] = MaxNode.p.y - SecondNode.p.y; SN[2] = 0.0;
				stl_normalize_vector(SN);

				MN[2] = -0.7;
				SN[2] = -0.7;

				stl_normalize_vector(MN);
				stl_normalize_vector(SN);

				//z������ת45��
				//XY_normal_sacle_Z(MN, 45);
				//XY_normal_sacle_Z(SN, 45);


				//�Ƿ��ж������߹��棿����
				//����ڶ�������ƽ��ķ�����
				Vectorf3 MNV(MN[0], MN[1], MN[2]);
				Vectorf3 SNV(SN[0], SN[1], SN[2]);

				Vectorf3 SNNV1 = SNV.Cross(MNV);
				Vectorf3 SNNV = SNV.Cross(SNNV1);

				//�󽻵�
				Pointf3 NextNode = CalPlaneLineIntersectPoint(SNNV, SecondNode.p, MNV, MaxNode.p);

				//�����󽻷�������(����)
				if (NextNode.z > MaxNode.p.z || NextNode.z > SecondNode.p.z || NextNode.z <= 0) {
					continue;
				}

				//�ж�������֦�Ƿ񴩹�ģ��
				bool across = false;
				if (NextNode.z > 1) {//�ڵ�߶�С��1�ᴩ���װ�
					stl_facet f;
					for (int i = 0; i < mesh.stl.stats.number_of_facets; ++i) {
						f = mesh.stl.facet_start[i];
						//--------------������֦--------------
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
					//------δ����------
					//����������֦
					tree_support_branch.push_back(Linef3(NextNode, MaxNode.p));
					tree_support_branch.push_back(Linef3(NextNode, SecondNode.p));

					//����һ���µĽڵ�
					n.num = MaxNode.num + SecondNode.num;
					n.p = NextNode;

					tree_support_node.push_back(NextNode);

					//������֦������
					if (n.num >= leaf_num) {
						//-------------��������ʱ�жϵ׶��Ƿ���ģ�ͳɽǶ�----------------
						Pointf3 bottom = mesh.point_model_intersection_Z(n.p);
						if (bottom.z == 0) {//ֱ��������ƽ̨
							double dis = n.p.z;
							if (dis > TDHeight) {
								Pointf3 p1(bottom.x, bottom.y, TDHeight);
								tree_support_bottom.push_back(Linef3(bottom, p1));
								tree_support_bole.push_back(Linef3(p1, n.p));
							}
							else {
								tree_support_bottom.push_back(Linef3(bottom, n.p));
							}
						}
						else {//��ģ���ཻ
							  //--------�����֧�ŵ��������Ƭ---------
							stl_facet face, f;
							bool ret = false;
							double dis2 = 1;
							//�����֧�ŵ��������Ƭ
							for (int i = 0; i < mesh.stl.stats.number_of_facets; ++i) {
								f = mesh.stl.facet_start[i];
								//�ж��Ƿ���������Ķ��������������
								if ((f.vertex[0].x == bottom.x&&f.vertex[0].y == bottom.y&&f.vertex[0].z == bottom.z) ||
									(f.vertex[1].x == bottom.x&&f.vertex[1].y == bottom.y&&f.vertex[1].z == bottom.z) ||
									(f.vertex[2].x == bottom.x&&f.vertex[2].y == bottom.y&&f.vertex[2].z == bottom.z))
								{
									ret = true;//��ǰ��Ϊ������
									face = f;
								}

								if (PointinTriangle1(Pointf3(f.vertex[0].x, f.vertex[0].y, f.vertex[0].z),
									Pointf3(f.vertex[1].x, f.vertex[1].y, f.vertex[1].z),
									Pointf3(f.vertex[2].x, f.vertex[2].y, f.vertex[2].z), bottom)) {

									Pointf3 t = CalPlaneLineIntersectPoint(Vectorf3(f.normal.x, f.normal.y, f.normal.z), Pointf3(f.vertex[0].x, f.vertex[0].y, f.vertex[0].z),
										Vectorf3(-f.normal.x, -f.normal.y, -f.normal.z), bottom);
									double dis1 = distance_point_3(t, bottom);
									if (!ret&&dis1 < 0.1) {
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

							Pointf3 p1;
							double angle;
							bool down = false;
							double height = TDHeight;

							//ͨ���׶˳������������ɵĽǶ�
							if (180/PI*vector_angle_3D(Vectorf3(0, 0, -1), Vectorf3(face.normal.x, face.normal.y, face.normal.z)) >=90) {
								do
								{
									p1.x = face.normal.x*height + bottom.x;
									p1.y = face.normal.y*height + bottom.y;
									p1.z = face.normal.z*height + bottom.z;

									float v[3];
									v[0] = p1.x - n.p.x;
									v[1] = p1.y - n.p.y;
									v[2] = p1.z - n.p.z;
									stl_normalize_vector(v);

									if (!(v[0] == 0 && v[1] == 0 && v[2] == 0)) {
										angle = 180 / PI*vector_angle_3D(Vectorf3(0, 0, -1), Vectorf3(v[0], v[1], v[2]));

										if (height >= 0.5)
											height -= 0.5;
										else
											down = true;
									}
									else
										down = true;
								} while (angle > 30 && !down);
							}
							else
								down = true;

							if (!down) {
								tree_support_bole.push_back(Linef3(p1, n.p));
								tree_support_bottom.push_back(Linef3(bottom, p1));
								tree_support_node.push_back(n.p);
								tree_support_node.push_back(p1);
							}
							else
							{
								if (n.p.z - bottom.z > TDHeight) {
									Pointf3 p2(bottom.x, bottom.y, TDHeight + bottom.z);
									tree_support_bottom.push_back(Linef3(bottom, p2));
									tree_support_bole.push_back(Linef3(p2, n.p));
								}
								else {
									tree_support_bottom.push_back(Linef3(bottom, n.p));
								}
							}
						}
						//--------------------------------------
					}
					else {
						node.push_back(n);
						rank_node(node);
					}

					//ɾ�������ɵĽڵ�
					delete_node(node, MaxNode);
					delete_node(node, SecondNode);

					//��Գɹ�
					ok = true;

					break;
				}
			}

			//���δ�ɹ�
			if (!ok) {
				//��������ʱ�жϵ׶��Ƿ���ģ�ͳɽǶ�
				Pointf3 bottom = mesh.point_model_intersection_Z(MaxNode.p);
				if (bottom.z == 0) {//ֱ��������ƽ̨
					//ɾ������Ҫ����ǿ�Ľڵ�
					//if (MaxNode.num == 1) {
					//	int dis = -1;
					//	for (auto t = tree_support_node.begin(); t != tree_support_node.end(); ++t) {
					//		if ((*t).x == MaxNode.p.x && (*t).y == MaxNode.p.y && (*t).z == MaxNode.p.z)
					//			dis = std::distance(tree_support_node.begin(), t);
					//	}
					//	tree_support_node.erase(tree_support_node.begin() + dis);
					//}

					if (MaxNode.p.z > TDHeight) {
						Pointf3 p1(bottom.x, bottom.y, TDHeight);
						tree_support_bottom.push_back(Linef3(bottom, p1));
						tree_support_bole.push_back(Linef3(p1, MaxNode.p));
					}
					else {
						tree_support_bottom.push_back(Linef3(bottom, MaxNode.p));
					}
				}
				else {//��ģ���ཻ
					//--------�����֧�ŵ��������Ƭ---------
					double dis2 = 1;
					stl_facet face, f;
					bool ret = false;
					//�����֧�ŵ��������Ƭ
					for (int i = 0; i < mesh.stl.stats.number_of_facets; ++i) {
						f = mesh.stl.facet_start[i];
						//�ж��Ƿ���������Ķ��������������
						if ((f.vertex[0].x == bottom.x&&f.vertex[0].y == bottom.y&&f.vertex[0].z == bottom.z) ||
							(f.vertex[1].x == bottom.x&&f.vertex[1].y == bottom.y&&f.vertex[1].z == bottom.z) ||
							(f.vertex[2].x == bottom.x&&f.vertex[2].y == bottom.y&&f.vertex[2].z == bottom.z))
						{
							ret = true;//��ǰ��Ϊ������
							face = f;
						}

						if (PointinTriangle1(Pointf3(f.vertex[0].x, f.vertex[0].y, f.vertex[0].z),
							Pointf3(f.vertex[1].x, f.vertex[1].y, f.vertex[1].z),
							Pointf3(f.vertex[2].x, f.vertex[2].y, f.vertex[2].z), bottom)) {

							Pointf3 t = CalPlaneLineIntersectPoint(Vectorf3(f.normal.x, f.normal.y, f.normal.z), Pointf3(f.vertex[0].x, f.vertex[0].y, f.vertex[0].z),
								Vectorf3(-f.normal.x, -f.normal.y, -f.normal.z), bottom);
							double dis1 = distance_point_3(t, bottom);
							if (!ret&&dis1 < 0.1) {
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
					//------------------------------

					Pointf3 p1;
					double angle;
					bool down = false;
					double height = TDHeight;

					//ͨ���׶˳������������ɵĽǶ�
					if (180 / PI*vector_angle_3D(Vectorf3(0, 0, -1), Vectorf3(face.normal.x, face.normal.y, face.normal.z)) >= 90) {
						do
						{
							p1.x = face.normal.x*height + bottom.x;
							p1.y = face.normal.y*height + bottom.y;
							p1.z = face.normal.z*height + bottom.z;

							float v[3];
							v[0] = p1.x - MaxNode.p.x;
							v[1] = p1.y - MaxNode.p.y;
							v[2] = p1.z - MaxNode.p.z;
							stl_normalize_vector(v);

							if (!(v[0] == 0 && v[1] == 0 && v[2] == 0)) {
								angle = 180 / PI*vector_angle_3D(Vectorf3(0, 0, -1), Vectorf3(v[0], v[1], v[2]));

								if (height >= 0.5)
									height -= 0.5;
								else
									down = true;

							}
							else
								down = true;
						} while (angle > 30 && !down);
					}
					else
						down = true;

					if (!down) {
						tree_support_bole.push_back(Linef3(p1, MaxNode.p));
						tree_support_bottom.push_back(Linef3(bottom, p1));
						tree_support_node.push_back(MaxNode.p);
						tree_support_node.push_back(p1);
					}
					else
					{
						if (MaxNode.p.z - bottom.z > TDHeight) {
							Pointf3 p2(bottom.x, bottom.y, TDHeight + bottom.z);
							tree_support_bottom.push_back(Linef3(bottom, p2));
							tree_support_bole.push_back(Linef3(p2, MaxNode.p));
						}
						else {
							tree_support_bottom.push_back(Linef3(bottom, MaxNode.p));
						}
					}
				}

				//ɾ��һ���ڵ�
				delete_node(node, MaxNode);
			}
		}
	}

	void TreeSupport::generate_support_beam(TriangleMesh& mesh)//����֧�ź���
	{
		int_linef3 lines_i;//��֧���ߵĺ�������
		for (auto l = tree_support_bole.begin(); l != tree_support_bole.end(); ++l) {
			if ((*l).a.x == (*l).b.x && (*l).a.y == (*l).b.y) {
				float dis = (*l).b.z - (*l).a.z;
				//����30mm��֧�Ų���Ҫ����
				if (dis > 30)
					lines_i.push_back(std::make_pair(0, *l));
			}
		}
		for (int_linef3::iterator _l = lines_i.begin(); _l != lines_i.end(); ++_l) {
			std::pair<int, Linef3> l = *_l;
			std::map<float, int> lines_0;//�������ӵ�֧����
			if (l.first == 0) {//û�����Ӻ�����֧��ȥ�Ӻ���
				for (int_linef3::iterator _l1 = lines_i.begin(); _l1 != lines_i.end(), _l1 != _l; ++_l1) {
					float dis = distance_point(Pointf(l.second.a.x, l.second.a.y), Pointf((*_l1).second.a.x, (*_l1).second.a.y));
					if (dis < 30) { //���ƿ�����֧����֮��ľ��뷶Χ
						int len = std::distance(lines_i.begin(), _l1);
						if (_l1->first == 0 || _l1->first == 1) {
							lines_0.insert(std::make_pair(dis, len));
						}
					}
				}

				if (lines_0.size() > 0) {
					std::map<float, int>::iterator m = lines_0.begin();
					int_linef3::iterator _l2 = lines_i.begin() + (*m).second;
					++_l2->first;//��������һ
					std::pair<int, Linef3> l2 = *_l2;
					Linef3 line1 = l.second;
					Linef3 line2 = l2.second;

					//���ɵ�λ����
					std::vector<Pointf3> ps = two_line_beam(Pointf(line1.a.x, line1.a.y), Pointf(line2.a.x, line2.a.y), 300);
					std::vector<Pointf3> ps1;

					//��ѡ����
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
						//size_t a = ps1.size() / 5 + 1;//���ƺ�������
						for (std::vector<Pointf3>::iterator p = ps1.begin(); p != ps1.end() - 1; ++p) {
							int b = std::distance(ps1.begin(), p);
							//if (b % a == 0) {
								//�жϺ����Ƿ񴩹�ģ��
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

	void TreeSupport::clear(bool point)
	{
		if (point) {
			support_point.clear();
			support_point_face.clear();
		}
		tree_support_leaf.clear();
		tree_support_bole.clear();
		tree_support_branch.clear();
		tree_support_bottom.clear();
	}


	void TreeSupport::delete_node(std::vector<treeNode>& node, treeNode& p)
	{
		for (auto i = node.begin(); i != node.end(); ++i) {
			if (p.p == (*i).p) {
				node.erase(i);
				return;
			}
		}
	}

	void TreeSupport::rank_node(std::vector<treeNode>& node)
	{
		if (node.size() > 1) {
			treeNode temp;
			for (int i = 0; i < node.size() - 1; ++i) {
				for (auto p2 = node.begin() + 1; p2 != node.end(); ++p2) {
					if ((*p2).p.z > (*(p2 - 1)).p.z) {
						temp = *p2;
						*p2 = *(p2 - 1);
						*(p2 - 1) = temp;
					}
				}
			}
		}
	}

	std::vector<Pointf3> TreeSupport::two_line_beam(Pointf a, Pointf b, int height)//���ɵ�λ����
	{
		std::vector<Pointf3> ps;
		float dis = distance_point(a, b);
		if (dis == 0)
			return ps;
		int k = height / dis;
		for (int i = 0; i<k; ++i) {
			if (i % 2 == 0) {
				ps.push_back(Pointf3(a.x, a.y, dis*i));
			}
			else {
				ps.push_back(Pointf3(b.x, b.y, dis*i));
			}
		}
		return ps;
	}
}


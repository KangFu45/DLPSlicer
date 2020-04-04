#include "TreeSupport.h"

namespace DLPSlicer {

	TreeSupport::TreeSupport(const TreeSupport* ts) {
		this->support_point = ts->support_point;
		this->support_point_face = ts->support_point_face;
		this->tree_support_bole = ts->tree_support_bole;
		this->tree_support_bottom = ts->tree_support_bottom;
		this->tree_support_branch = ts->tree_support_branch;
		this->tree_support_leaf = ts->tree_support_leaf;
		this->tree_support_node = ts->tree_support_node;
	}

	void TreeSupport::translate_(double x, double y, double z)
	{
		translate(this->support_point, x, y, z);
		translate(this->support_point_face, x, y, z);
		translate(this->tree_support_bole, x, y, z);
		translate(this->tree_support_bottom, x, y, z);
		translate(this->tree_support_branch, x, y, z);
		translate(this->tree_support_leaf, x, y, z);
		translate(this->tree_support_node, x, y, z);
	}

	//���ܣ���ģ�����ҵ�����ָ�����������
	//����1������ģ��
	//����2��ָ����
	//����3���������������
	//���أ��Ƿ��ҵ�
	inline bool PointFindTrangle(const TriangleMesh* mesh, Pointf3 p, stl_facet& face)
	{
		//�����֧�ŵ��������Ƭ
		for (int i = 0; i < mesh->stl.stats.number_of_facets; ++i) {
			face = mesh->stl.facet_start[i];
			//�ж��Ƿ���������Ķ��������������
			if (p == face.vertex[0] || p == face.vertex[1] || p == face.vertex[2])
				return true;

			//��������xyƽ��ͶӰ�ж��Ƿ������
			if (!PointInTriangle3(face, p))
				continue;

			//�жϵ��Ƿ��ڵ�ǰ���ϼ�������ľ���Ϊ��
			if (DisPoint3(CalPlaneLineIntersectPoint(Nor2Vt3(face.normal),
				Ver2Pf3(face.vertex[0]), Nor2Vt3(face.normal).Reverse(), p), p) <= 0.001)//0.001Ϊ����ľ������
				return true;
		}
		return false;
	}

	void TreeSupport::SupportPointDown(TriangleMesh mesh, const Pointf3& p1, TreeNodes& nodes, const Paras& paras)
	{
		//��������
		Pointf3 p2 = mesh.point_model_intersection_Z(p1);
		double dis = p1.z - p2.z;

		if (dis < paras.td_height * 2) {//С�ڶ��˾���ֱ�ӷ����δ���
			Pointf3 p3(p1.x, p1.y, p1.z - (dis /= 2.0));
			tree_support_leaf.emplace_back(Linef3(p3, p1));
			tree_support_bottom.emplace_back(Linef3(p2, p3));
		}
		else {//���ɶ��˲�������ڵ�
			p2.x = p1.x; p2.y = p1.y; p2.z = p1.z - paras.td_height * 2;
			tree_support_leaf.emplace_back(Linef3(p2, p1));
			//�洢���Ľڵ�
			nodes.emplace_back(TreeNode{ 1,p2 });
		}
		if (p2.z > 0.5)//��ֹ��ǿ�󴩹��װ�
			tree_support_node.emplace_back(p2);
	}

	void TreeSupport::GenTreeSupport(TriangleMesh& mesh, const Paras& paras, QProgressBar* progress)
	{
		//��������Χ35->81
		TreeNodes nodes;//���Ľڵ㣨����洢���Ľṹ��ϵ��

		//-----------------�������ϴ�֧�ŵ����ɶ�����һ�����ڵ�--------------------
		for each (const stl_vertex & v  in support_point_face)
		{
			//֧�ŵ�
			Pointf3 p1 = Ver2Pf3(v);
			//parent
			stl_facet face;
			bool ret = PointFindTrangle(&mesh, p1, face);

			//֧�ŵ����淨���������ӳ��Ķ��˵�
			Pointf3 p2(face.normal.x * paras.td_height + p1.x
				, face.normal.y * paras.td_height + p1.y
				, face.normal.z * paras.td_height + p1.z);

			bool down = false;//֧�ŵ��Ƿ���������
			if (p2.z >= 0) {
				//��飨���������ײ������Z�Ḻ�������죩
				for (int i = 0; i < mesh.stl.stats.number_of_facets; ++i) {
					if (mesh.stl.facet_start[i] == face)
						continue;

					if (isLineCrossTriangle(mesh.stl.facet_start[i], Linef3(p1, p2))) {
						down = true;
						break;
					}
				}
			}
			else
				down = true;

			if (!down) {//���䷨������������
				//����0.1mm
				tree_support_leaf.emplace_back(Linef3(p2, Pointf3(-face.normal.x * 0.1 + p1.x, -face.normal.y * 0.1 + p1.y, -face.normal.z * 0.1 + p1.z)));
				//�洢���Ľڵ�
				nodes.emplace_back(TreeNode{ 1,p2 });
				if (p2.z > 0.5)//��ֹ��ǿ�󴩹��װ�
					tree_support_node.emplace_back(p2);
			}
			else
				SupportPointDown(mesh, p1, nodes, paras);
		}

		//-----------------���������ɶ�����һ�����ڵ�--------------------
		for each (const stl_vertex & v in support_point)
			SupportPointDown(mesh, Ver2Pf3(v), nodes, paras);

		//TODO:�Խڵ���з����򣬸����߳�����������ʹ�ò��м���
		BoundingBoxf3 box = mesh.bounding_box();
		float space_z = (box.max.z - box.min.z) / 4;
		TreeNodes nodes1, nodes2, nodes3, nodes4;
		for each (const TreeNode& node in nodes)
		{
			if (node.p.z < space_z + box.min.z)
				nodes1.emplace_back(node);
			else if (node.p.z < space_z * 2 + box.min.z)
				nodes2.emplace_back(node);
			else if (node.p.z < space_z * 3 + box.min.z)
				nodes3.emplace_back(node);
			else if (node.p.z < space_z * 4 + box.min.z)
				nodes4.emplace_back(node);
		}

		std::vector<TreeNodes> nodess;
		nodess.emplace_back(nodes1);
		nodess.emplace_back(nodes2);
		nodess.emplace_back(nodes3);
		nodess.emplace_back(nodes4);

		progress->setValue(75);

		parallelize<size_t>(
			0,
			nodess.size() - 1,
			boost::bind(&TreeSupport::GenTreeSupArea, this, _1, &nodess, mesh, paras),
			paras.thread > 1 ? paras.thread : 1
			);

		progress->setValue(90);
		GenSupportBeam(mesh);
	}

	//���ڵ㰴z�᷽�򰴽�������
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

	//�ӽڵ㼯��ɾ��ָ���ڵ�
	void DeleteNode(TreeNodes& nodes, const TreeNode& node)
	{
		for (auto i = nodes.begin(); i != nodes.end(); ++i) {
			if (node.p == (*i).p) {
				nodes.erase(i);
				return;
			}
		}
	}

	void TreeSupport::ModelInterSupport(TriangleMesh mesh, const Pointf3& p, const Paras& paras)
	{
		//-------------��������ʱ�жϵ׶��Ƿ���ģ�ͳɽǶ�----------------
		Pointf3 bottom = mesh.point_model_intersection_Z(p);
		if (bottom.z == 0) {//ֱ��������ƽ̨
			if (p.z > paras.td_height) {
				Pointf3 p1(bottom.x, bottom.y, paras.td_height);
				tree_support_bottom.emplace_back(Linef3(bottom, p1));
				tree_support_bole.emplace_back(Linef3(p1, p));
			}
			else
				tree_support_bottom.emplace_back(Linef3(bottom, p));
		}
		else {
			//��ģ���ཻ
			Pointf3 p1;
			double angle;
			bool down = false;
			double hei = paras.td_height;

			stl_facet face;
			//TODO:δ�ҵ���Ĵ������ʼ�С��
			bool ret = PointFindTrangle(&mesh, bottom, face);
			//ͨ���׶˳������������ɵĽǶ�
			if (180 / PI * vector_angle_3(Vectorf3(0, 0, -1), Nor2Vt3(face.normal)) >= 90) {
				do
				{
					p1.x = face.normal.x * hei + bottom.x;
					p1.y = face.normal.y * hei + bottom.y;
					p1.z = face.normal.z * hei + bottom.z;

					float* v = (p1 - p).data();
					stl_normalize_vector(v);

					if (v[0] != 0 && v[1] != 0 && v[2] != 0) {
						angle = 180 / PI * vector_angle_3(Vectorf3(0, 0, -1), Vectorf3(v));

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
		//------------������״�ṹ-------------
		TreeNodes nodes = *(nodess->begin() + i);

		//�ɸߵ�������
		RankNode(nodes);

		while (!nodes.empty())
		{
			//ѡ��ڵ���ߵ�
			TreeNode MaxNode = nodes.front();
			nodes.erase(nodes.begin());

			//ѡ������֮�����С��lenght�ĵ�
			std::map<float, TreeNode> dis_nodes;
			for each (const TreeNode & p3 in nodes) {
				//������֦����
				if (p3.num + MaxNode.num <= paras.leaf_num) {
					float dis = DisPoint3(MaxNode.p, p3.p);
					if (dis <= 30)
						dis_nodes.insert(std::make_pair(dis, p3));
				}
			}

			//��߽ڵ����������Ľڵ������ԡ�
			//�������ڵ�ΪԲ׶���㣬��Լ���Ƕ�(45��)Ϊ���Ƕȵ�Բ׶����������Բ׶������̾���Ľ��㡣
			bool ok = false;//�ж�����Ƿ�ɹ�
			for (auto p4 = dis_nodes.begin(); p4 != dis_nodes.end(); ++p4) {
				TreeNode SecondNode = (*p4).second;
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
				if (NextNode.z > MaxNode.p.z || NextNode.z > SecondNode.p.z || NextNode.z <= 0)
					continue;

				//�ж�������֦�Ƿ񴩹�ģ��
				bool across = false;
				if (NextNode.z > 1) {//�ڵ�߶�С��1�ᴩ���װ�
					for (int i = 0; i < mesh.stl.stats.number_of_facets; ++i) {
						stl_facet f = mesh.stl.facet_start[i];
						//--------------������֦--------------
						if (isLineCrossTriangle(f, Linef3(NextNode, MaxNode.p)) ||
							isLineCrossTriangle(f, Linef3(NextNode, SecondNode.p))) {
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
					tree_support_branch.emplace_back(Linef3(NextNode, MaxNode.p));
					tree_support_branch.emplace_back(Linef3(NextNode, SecondNode.p));

					//����һ���µĽڵ�
					TreeNode n{ MaxNode.num + SecondNode.num ,NextNode };

					tree_support_node.emplace_back(NextNode);

					//������֦������
					if (n.num >= paras.leaf_num)
						ModelInterSupport(mesh, n.p, paras);
					else {
						nodes.emplace_back(n);
						RankNode(nodes);
					}
					DeleteNode(nodes, SecondNode);
					//��Գɹ�
					ok = true;
					break;
				}
			}

			//���δ�ɹ�
			if (!ok)
				ModelInterSupport(mesh, MaxNode.p, paras);
		}
	}

	//����l1��l2������ֱ�ߵĽ��������ps1Ϊl1�ϵĺ����㣬ps2Ϊl2�ϵĺ�����
	inline void TwoLineBeam(Linef3 l1, Linef3 l2, Pointf3s& ps1, Pointf3s& ps2)
	{
		float dis = DisPoint2(Pointf(l1.a.x, l1.a.y), Pointf(l2.a.x, l2.a.y));
		if (dis == 0)
			return;

		float low = l1.a.z > l2.a.z ? l2.a.z : l1.a.z;
		float high = l1.b.z < l2.b.z ? l1.b.z : l2.b.z;

		float pos, pos1;
		for (int i = 0; i < high / dis; ++i) {
			pos = low + dis * i;//��������͵㿪ʼ
			pos1 = low + dis * (i + 1);
			if (pos1 > high)
				break;


			if (i % 2 == 0) {
				ps1.emplace_back(Pointf3(l1.a.x, l1.a.y, pos));
				ps2.emplace_back(Pointf3(l2.b.x, l2.b.y, pos1));
			}
			else {
				ps1.emplace_back(Pointf3(l1.b.x, l1.b.y, pos1));
				ps2.emplace_back(Pointf3(l2.b.x, l2.b.y, pos));
			}
		}
	}

	void TreeSupport::GenSupportBeam(const TriangleMesh& mesh)//����֧�ź���
	{
		//���ɵ���ʱ���ݽṹ
		struct TreeBole {
			unsigned short num; //������ warning:δʹ��
			Linef3 bole;	//����
		};

		std::vector<TreeBole> boles;//��֧���ߵĺ�������
		for each (const Linef3 & l in tree_support_bole) {
			if (l.a.x == l.b.x && l.a.y == l.b.y) {
				//����30mm��֧����Ҫ����
				//if (l.b.z - l.a.z > 30)//TODO:���������Ʋ�����
				boles.emplace_back(TreeBole{ 0,l });
			}
		}

		//for (auto b = boles.begin(); b != boles.end(); ++b) {
		for each (const TreeBole & b in boles)
		{
			if (b.num > 0)
				continue;

			//������
			float len = 30;//�洢ƥ������֮��ĳ���
			TreeBole b2;
			//for (auto b3 = boles.begin(); b3 != boles.end(); ++b3) {
			for each (const TreeBole & b3 in boles) {
				float t_len = DisPoint2(Pointf(b.bole.a.x, b.bole.a.y), Pointf(b3.bole.a.x, b3.bole.a.y));
				if (t_len < 30 && t_len < len && t_len != 0) {
					len = t_len;
					b2 = b3;
				}
			}

			if (len == 30)
				continue;

			//���ɵ�λ����
			Pointf3s ps1, ps2;
			TwoLineBeam(b.bole, b2.bole, ps1, ps2);

			if (ps1.empty() || ps1.size() != ps2.size())
				continue;

			bool dir = true;
			for (auto p1 = ps1.begin(); p1 != ps1.end(); ++p1) {
				auto p2 = ps2.begin() + std::distance(ps1.begin(), p1);
				bool across = false;

				Linef3 line;
				if (dir) {
					line.a = *p1;
					line.b = *p2;
				}
				else {
					line.a = *p2;
					line.b = *p1;
				}
				dir = !dir;

				for (int i = 0; i < mesh.stl.stats.number_of_facets; ++i) {
					if (isLineCrossTriangle(mesh.stl.facet_start[i], line)) {
						across = true;
						break;
					}
				}

				if (!across)
					tree_support_branch.emplace_back(line);
			}
		}
	}
}


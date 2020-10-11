#include "DLPrint.h"
#include "Geometry.hpp"
#include "ClipperUtils.hpp"

#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qimage.h>
#include <qbuffer.h>
#include <qdir.h>

#include <tbb/parallel_for.h>

#include "Setting.h"
extern Setting e_setting;

#ifdef DLPSlicer_DEBUG
#include "qdebug.h"
#endif

namespace DLPSlicer {

	DLPrint::DLPrint(Model* _model, Config* config)
		:model(_model)
		, m_config(config)
	{
	}

	DLPrint::~DLPrint()
	{
		while (!treeSupports.empty()) {
			delete treeSupports.begin()->second;
			treeSupports.erase(treeSupports.begin());
		}

		if (m_areas != nullptr)
			delete m_areas;
	}

	void DLPrint::Slice(const TriangleMeshs& supMeshs, QProgressBar* progress)
	{
		BoundingBoxf3 box = model->bounding_box();

		const float lh = this->m_config->layer_height;
		Layers ls;
		ls.emplace_back(Layer(lh / 2, lh));
		while (ls.back().print_z + (double)lh / 2 <= box.max.z)
			ls.emplace_back(Layer(ls.back().print_z + lh / 2, ls.back().print_z + lh));

		this->layerss.swap(std::vector<Layers>());

		size_t layerNum = ls.size();
		std::vector<float> slice_z;
		//准备切片并得到切片层
		{
			for (size_t i = 0; i < ls.size(); ++i)
				slice_z.emplace_back(ls[i].slice_z);

			int aaa = 0;
			for (auto o = this->model->objects.begin(); o != this->model->objects.end(); ++o) {
				for (auto i = (*o)->instances.begin(); i != (*o)->instances.end(); ++i) {
					this->t_layers.swap(Layers());

					for (auto a = ls.begin(); a != ls.end(); ++a)
						this->t_layers.emplace_back(*a);

					//切片
					std::vector<ExPolygons> slices;
					TriangleMesh mesh = (*o)->raw_mesh();
					(*i)->transform_mesh(&mesh);

					TriangleMeshSlicer<Z>(&mesh).slice(slice_z, &slices, m_config->threads);//切片函数
					for (size_t i = 0; i < slices.size(); ++i)
						this->t_layers.at(i).slices.expolygons = slices[i];

					//切片进度条范围21->98
					if (progress != nullptr) {
						int unit = (aaa + 1) / (model->instances_num() / (98 - 21) + 1);
						if (progress->value() < 21 + unit)
							progress->setValue(21 + unit);
					}
					++aaa;

					//抽空
					if (m_config->hollow_out) {

						//std::unique_ptr<Fill> fill(Fill::new_from_type(ip3DHoneycomb));


						//fill->bounding_box.merge(Point::new_scale(bb.min.x, bb.min.y));
						//fill->bounding_box.merge(Point::new_scale(bb.max.x, bb.max.y));

						//fill->min_spacing = this->config.get_abs_value("infill_extrusion_width", this->config.layer_height.value);
						//fill->min_spacing = 3;
						//qDebug() << "min_spacing:  " << fill->min_spacing;

						//fill->angle = 45;
						//fill->angle = Geometry::deg2rad(this->config.fill_angle.value);

						//fill->density = this->config.fill_density.value / 100;
						//fill->density = 0.5;
						//qDebug() << "dendity:  " << fill->density;

						//计算模型体积，体积小于50ml的模型不抽空
						double areas = 0;
						for (auto s = slices.begin(); s != slices.end(); ++s) {
							Polygons poly = to_polygons(*s);
							for (auto ps = poly.begin(); ps != poly.end(); ++ps) {
								//求面积
								areas += (*ps).area() * SCALING_FACTOR * SCALING_FACTOR;
							}
						}

						if (areas > 50) {
							ExPolygons pattern;

							if (m_config->fill_pattern == Config::ipHoneycomb) {
								//计算填充密度
								double space = (1 - double(m_config->fill_density / 100)) * 20;
								space = 1 >= space ? 1 : space;

								//壁厚
								double wall = double(m_config->fill_density / 100) * 2;
								wall = wall <= 0.1 ? 0.1 : wall;

								BoundingBox box;
								box.merge(Point::new_scale((*i)->box.min.x, (*i)->box.min.y));
								box.merge(Point::new_scale((*i)->box.max.x, (*i)->box.max.y));
								pattern.emplace_back(GenHoneycombPattern(box, wall, space));
							}
							
							tbb::parallel_for(
								tbb::blocked_range<size_t>(0, t_layers.size() - 1),
								[this, pattern](const tbb::blocked_range<size_t>& range) {
								for (size_t line_idx = range.begin(); line_idx < range.end(); ++line_idx) {
									this->InfillLayer(line_idx, pattern);
								}
							}
							);
						}
					}
					layerss.emplace_back(t_layers);
				}
			}

			//切支撑
			if(!supMeshs.empty()){
				TriangleMesh SMesh;
				for (auto s = supMeshs.begin(); s != supMeshs.end(); ++s)
					SMesh.merge(*s);

				Layers t_layers1;
				for (auto a = ls.begin(); a != ls.end(); ++a)
					t_layers1.emplace_back(*a);

				std::vector<ExPolygons> slices;
				TriangleMeshSlicer<Z>(&SMesh).slice(slice_z, &slices, m_config->threads);//切片函数
				for (size_t i = 0; i < slices.size(); ++i)
					t_layers1.at(i).slices.expolygons = slices[i];
				layerss.emplace_back(t_layers1);
			}
		}

		//生成底板
		if (this->m_config->raft_layers > 0) {
			float r = m_config->layer_height * m_config->raft_layers;

			double L = double(e_setting.m_printers.begin()->length / 2);
			double W = double(e_setting.m_printers.begin()->width / 2);

			BoundingBoxf bb;
			bb.min.x = box.min.x - m_config->raft_offset > -L ? box.min.x - m_config->raft_offset : -L;
			bb.min.y = box.min.y - m_config->raft_offset > -W ? box.min.y - m_config->raft_offset : -W;
			bb.max.x = box.max.x + m_config->raft_offset < L ? box.max.x + m_config->raft_offset : L;
			bb.max.y = box.max.y + m_config->raft_offset < W ? box.max.y + m_config->raft_offset : W;

			ExPolygons raft;
			Polygon poly;
			poly.points.emplace_back(Point(scale_(bb.min.x), scale_(bb.min.y)));
			poly.points.emplace_back(Point(scale_(bb.min.x), scale_(bb.max.y)));
			poly.points.emplace_back(Point(scale_(bb.max.x), scale_(bb.max.y)));
			poly.points.emplace_back(Point(scale_(bb.max.x), scale_(bb.min.y)));
			raft.emplace_back(ExPolygon(poly));

			this->r_layers.swap(Layers());
			for (int i = this->m_config->raft_layers; i >= 1; --i) {
				this->r_layers.insert(this->r_layers.begin(), Layer(0, lh * i));
				this->r_layers.front().slices.expolygons = raft;
			}

			// prepend total raft height to all sliced layers
			for (auto l = layerss.begin(); l != layerss.end(); ++l)
				for (auto l1 = l->begin(); l1 != l->end(); ++l1)
					(*l1).print_z += lh * this->m_config->raft_layers;

			for (auto i = this->inside_supports.begin(); i != this->inside_supports.end(); ++i) {
				linef3s* l = (*i).second;
				for (auto l1 = l->begin(); l1 != l->end(); ++l1) {
					(*l1).a.z += r;
					(*l1).b.z += r;
				}
			}
		}

		//导出图片
		this->SavePng(box, layerNum);

		//更新切片信息
		info.layers_num = layer_qt_path.size();
		info.length = box.max.x - box.min.x;
		info.width = box.max.y - box.min.y;
		info.height = box.max.z + (double)m_config->layer_height * m_config->raft_layers;

		info.volume = 0;
		float laft_area = (box.max.x - box.min.x + m_config->raft_offset * 2) * (box.max.y - box.min.y + m_config->raft_offset * 2);
		for (int i = m_config->raft_layers; i < layer_qt_path.size(); ++i) {
			info.volume += m_areas[i] * m_config->layer_height;
		}
		info.volume += laft_area * m_config->raft_layers;
	}

	void DLPrint::InfillLayer(size_t i, ExPolygons pattern)
	{
		Layer& layer = this->t_layers[i];//在多线程下运行，传入的参数都进行拷贝操作

		// const float shell_thickness = this->config.get_abs_value("perimeter_extrusion_width", this->config.layer_height.value);
		const float shell_thickness = m_config->wall_thickness;
		// In order to detect what regions of this layer need to be solid,
		// perform an intersection with layers within the requested shell thickness.
		Polygons internal = layer.slices;
		for (size_t j = 0; j < this->t_layers.size(); ++j) {
			const Layer& other = this->t_layers[j];
			if (std::abs(other.print_z - layer.print_z) > shell_thickness) continue;

			if (j == 0 || j == this->t_layers.size() - 1) {
				internal.clear();
				break;
			}
			else if (i != j) {
				internal = intersection(internal, other.slices);
				if (internal.empty()) break;
			}
		}

		// If we have no internal infill, just print the whole layer as a solid slice.
		if (internal.empty()) return;
		layer.solid = false;

		const Polygons infill = offset(layer.slices, -scale_(shell_thickness));//内圈填充

		// Generate solid infill
		layer.solid_infill << diff_ex(infill, internal, true);

		// Generate internal infill
		if (m_config->fill_pattern == Config::ipHoneycomb)
		{
			//std::unique_ptr<Fill> fill(_fill->clone());
			//fill->layer_id = i;
			//fill->z = layer.print_z;

			//ExtrusionPath templ(erInternalInfill);
			//templ.width = fill->spacing();
			const ExPolygons internal_ex = intersection_ex(infill, internal);//需要内部填充的区域

			layer.solid_infill << union_ex(intersection_ex(internal_ex, pattern));
			//for (ExPolygons::const_iterator it = internal_ex.begin(); it != internal_ex.end(); ++it) {
			//	Polylines polylines = fill->fill_surface(Surface(stInternal, *it));
			//	layer.infill.append(polylines, templ);
			//}
		}
		else if (!inside_supports.empty() && m_config->fill_pattern == Config::ip3DSupport) {
			ExPolygons contour = layer.slices.expolygons;
			ExPolygons ss1;
			Pointfs circle = CirclePoints(15);
			float now = m_config->layer_height * i;

			for (auto i = inside_supports.begin(); i != inside_supports.end(); ++i) {
				for (auto l3 = (*i).second->begin(); l3 != (*i).second->end(); ++l3) {
					if (now > (*l3).a.z&& now < (*l3).b.z) {
						ExPolygon s1;
						//求横梁与当前面的交点
						Vectorf3 planeVector(0, 0, 1);
						Pointf3 planePoint(0, 0, now);
						Vectorf3 lineVector((*l3).b.x - (*l3).a.x, (*l3).b.y - (*l3).a.y, (*l3).b.z - (*l3).a.z);
						Pointf3 point = CalPlaneLineIntersectPoint(planeVector, planePoint, lineVector, (*l3).a);
						//将模型支撑在不同实例上画出

						for (std::vector<Pointf>::iterator p = circle.begin(); p != circle.end() - 1; ++p) {
							Pointf _p = *p;
							_p.scale(m_config->support_radius);
							_p.translate(point.x, point.y);
							s1.contour.points.emplace_back(Point(scale_(_p.x), scale_(_p.y)));
						}
						ss1.push_back(s1);
					}
				}
			}
			ss1 = union_ex(ss1);
			layer.solid_infill = diff_ex(layer.solid_infill, ss1) + intersection_ex(ss1, contour);//(a-b)+(a*b)
		}

		// Generate perimeter(s).
		layer.perimeters << diff_ex(
			layer.slices,
			offset(layer.slices, -scale_(shell_thickness))
		);
	}

	inline void GenHoneycomb(const BoundingBoxf& box, Pointfs& ps, double radius, Pointf p, double angle, bool again)
	{
		//超出边界退出
		if ((p.x + radius) < box.min.x || (p.x - radius) > box.max.x
			|| (p.y + radius) < box.min.y || (p.y - radius) > box.max.y)
			return;

		if (again)
			ps.push_back(p);

		double angle1 = angle / 180 * PI;

		Pointf p1;
		if (angle == 30)
		{
			p1.x = p.x + cos(angle1) * radius;
			p1.y = p.y + sin(angle1) * radius;

			GenHoneycomb(box, ps, radius, p1, 30, true);
			GenHoneycomb(box, ps, radius, p1, 90, false);
			GenHoneycomb(box, ps, radius, p1, 330, false);
		}
		else if (angle == 90)
		{
			p1.x = p.x;
			p1.y = p.y + radius;

			GenHoneycomb(box, ps, radius, p1, 90, true);
		}
		else if (angle == 330)
		{
			p1.x = p.x + cos(angle1) * radius;
			p1.y = p.y + sin(angle1) * radius;

			GenHoneycomb(box, ps, radius, p1, 330, true);
		}
	}

	ExPolygon DLPrint::GenHoneycombPattern(BoundingBox box, double wall, double radius)
	{
		box.offset(scale_(radius * 2));

		ExPolygon pattern;
		pattern.contour = box.polygon();

		BoundingBoxf box1;
		box1.max.x = unscale(box.max.x);
		box1.max.y = unscale(box.max.y);
		box1.min.x = unscale(box.min.x);
		box1.min.y = unscale(box.min.y);

		Pointfs ps;
		GenHoneycomb(box1, ps, radius + wall / 2, box1.min, 30, true);

		Pointfs circle= CirclePoints(30);
		Points ps1;
		for (auto s = circle.begin(); s != circle.end(); ++s)
			ps1.emplace_back(Point(scale_((*s).x), scale_((*s).y)));

		Polygon _hole(ps1);

		Polygon hole;
		for (auto p1 = ps.begin(); p1 != ps.end(); ++p1) {
			hole = _hole;
			hole.scale(radius * 1.1);
			hole.translate(scale_((*p1).x), scale_((*p1).y));
			pattern.holes.push_back(hole);
		}

		return pattern;
	}

	ExPolygon DLPrint::GenPattern(const BoundingBox& box)
	{
		ExPolygon pattern;
		pattern.contour = box.polygon();

		Pointfs circle = CirclePoints(15);
		Points ps;
		for (auto s = circle.begin(); s != circle.end(); ++s)
			ps.push_back(Point(scale_((*s).x), scale_((*s).y)));

		Polygon _hole(ps);

		//得到正方形洞的中心
		int space = 10;

		BoundingBoxf _box;
		_box.max.x = unscale(box.max.x);
		_box.max.y = unscale(box.max.y);
		_box.min.x = unscale(box.min.x);
		_box.min.y = unscale(box.min.y);

		int x = (_box.max.x - _box.min.x) / space;
		int y = (_box.max.y - _box.min.y) / space;

		Polygon hole;
		for (int i = 0; i < x; ++i) {
			for (int j = 0; j < y; ++j) {
				hole = _hole;
				hole.scale(space);
				hole.translate(scale_(_box.min.x + (INT64)i * space + space / 2), scale_(_box.min.y + j * space + space / 2));
				pattern.holes.push_back(hole);
			}
		}

		return pattern;
	}


	void DLPrint::GenInsideSupport(size_t id, TriangleMesh* mesh)
	{
		BoundingBoxf3 bb = mesh->bounding_box();
		linef3s* lines = new linef3s();
		Pointf3s pointf3_xz_45, pointf3_xz_135, pointf3_yz_45, pointf3_yz_135;

		//计算填充密度
		double space = (1 - double(m_config->fill_density / 100)) * 10;
		space = m_config->support_radius >= space ? m_config->support_radius + 1.0 : space;

		RadiatePoint(bb, pointf3_xz_45, space, XZ_45);
		RadiatePoint(bb, pointf3_xz_135, space, XZ_135);
		RadiatePoint(bb, pointf3_yz_45, space, YZ_45);
		RadiatePoint(bb, pointf3_yz_135, space, YZ_135);

		RadiateInter(mesh, pointf3_xz_45, *lines, XZ_45);
		RadiateInter(mesh, pointf3_xz_135, *lines, XZ_135);
		RadiateInter(mesh, pointf3_yz_45, *lines, YZ_45);
		RadiateInter(mesh, pointf3_yz_135, *lines, YZ_135);
		inside_supports.insert(std::make_pair(id, lines));
	}


	TreeSupport* DLPrint::GenSupport(size_t id, TriangleMesh* mesh, QProgressBar* progress)
	{
		mesh->require_shared_vertices();
		mesh->extract_feature_face(m_config->angle);

		progress->setValue(20);

		TreeSupport* s = new TreeSupport;
		s->support_point = mesh->feature_point(progress);
		s->support_point_face = mesh->feature_point_face(m_config->space, progress);

#ifdef DLPSlicer_DEBUG
		qDebug() << "support point: " << s->support_point.size();
		qDebug() << "support point face: " << s->support_point_face.size();
#endif

		s->GenTreeSupport(*mesh, TreeSupport::Paras{ m_config->leaf_num ,m_config->threads ,m_config->support_top_height }, progress);
		return s;
	}

	//删除支撑时需要更新id，但map的键值无法直接修改，所以效率比vector更新差许多
	bool DLPrint::DelTreeSupport(size_t id)
	{
		bool ret = false;
		size_t a = id / InstanceNum;
		std::map<int, TreeSupport*> t_tss;
		for (auto ts = treeSupports.begin(); ts != treeSupports.end(); ++ts) {
			if (ts->first != id) {
				size_t b = ts->first / InstanceNum;
				if (a == b && ts->first > id)
					t_tss.insert(*ts);
			}
		}

		auto s = treeSupports.find(id);
		if (s != treeSupports.end()) {
			delete (*s).second;
			treeSupports.erase(s);
			ret = true;
		}

		for (auto ts = t_tss.begin(); ts != t_tss.end(); ++ts) {
			treeSupports.erase(ts->first);
			treeSupports.insert(std::pair<int, TreeSupport*>(ts->first - 1, ts->second));
		}

		return ret;
	}

	void DLPrint::SaveSlice()
	{
		BoundingBoxf3 box = model->bounding_box();

		if (QDir(e_setting.ZipTempPath.c_str()).exists()) {

			QFile file(string(e_setting.ZipTempPath + "/buildscipt.ini").c_str());
			//每次切片都会删除上次的临时切片文件，当配置文件存在说明再次保存同一切片文件
			if (!file.exists()) {
				if (file.open(QFile::WriteOnly | QFile::Truncate))
				{
					QTextStream stream(&file);
					stream << "Slice thickness = " << m_config->layer_height << "\n";
					stream << "norm illumination time = " << m_config->normIlluTime << "\n";
					stream << "norm illumination inttersity = " << m_config->norm_inttersity << "\n";
					stream << "number of override slices = " << m_config->overLayer << "\n";
					stream << "override illumination time = " << m_config->overIlluTime << "\n";
					stream << "override illumination inttersity =" << m_config->over_inttersity << "\n";
					stream << "first layer_illumination_time = " << m_config->overIlluTime << "\n";
					stream << "first illumination inttersity = " << m_config->over_inttersity << "\n";
					stream << "number of slices = " << info.layers_num << "\n";
					stream << "length = " << info.length << "\n";
					stream << "width = " << info.width << "\n";
					stream << "height = " << info.height << "\n";
					stream << "model volume = " << info.volume / 1000.0 << "\n";

					float laft_area = (box.max.x - box.min.x + m_config->raft_offset * 2) * (box.max.y - box.min.y + m_config->raft_offset * 2);
					float t = 0;
					for (int j = 0; j < layer_qt_path.size(); ++j) {
						if (j >= m_config->raft_layers)
							t = m_areas[j];
						else
							t = laft_area;
						//if (j < m_config->overLayer) {
							//长曝光层
						stream << j * m_config->layer_height << " , " << "slice" << j << ".png , " << t << "\n";
						//}
						//else {
						//	//正常曝光层
						//	stream << j * m_config->layer_height << " , " << "slice" << j << ".png , " << aaa << "\n";
						//}
					}
				}
				file.close();
			}

			tbb::parallel_for(
				tbb::blocked_range<size_t>(0, layer_qt_path.size()),
				[this](const tbb::blocked_range<size_t>& range) {
				for (size_t line_idx = range.begin(); line_idx < range.end(); ++line_idx) {
					this->SaveOneSlice(line_idx, e_setting.ZipTempPath.c_str());
				}
			}
			);
		}
	}

	void DLPrint::SavePng(const BoundingBoxf3& box, size_t layerNum)
	{
		if (!layer_qt_path.empty()) {
			for (auto l = layer_qt_path.begin(); l != layer_qt_path.end(); ++l)
				delete l->second;
			layer_qt_path.swap(map<size_t, QPainterPath* >());
		}

		if (this->m_areas != nullptr)
			delete this->m_areas;

		this->m_areas = new double[layerNum + m_config->raft_layers];

		tbb::parallel_for(
			tbb::blocked_range<size_t>(0, layerNum + m_config->raft_layers - 1),
			[this](const tbb::blocked_range<size_t>& range) {
			for (size_t line_idx = range.begin(); line_idx < range.end(); ++line_idx) {
				this->SaveOnePng(line_idx);
			}
		}
		);
	}

	inline QPolygonF poly2Qpoly(const Polygon& p)//将Polygon装换为QPolygonF
	{
		QVector<QPointF> pfs;
		for each (Point p in p.points)
		{
			Pointf pf(unscale(p.x), unscale(p.y));
			pf.scale(e_setting.m_printers.begin()->factor);
			pfs.push_back(QPointF(pf.x + 960, pf.y + 540));
		}
		return QPolygonF(pfs);
	}

	void DLPrint::SaveOneSlice(size_t num, QString path)
	{
		QFile file(path + QObject::tr("/slice%1.png").arg(num));
		if (!file.exists()) {
			if (!file.open(QIODevice::ReadWrite))
				return;

			QImage image(1920, 1080, QImage::Format_RGB32);
			QPainter painter(&image);
			painter.setRenderHint(QPainter::Antialiasing, true);
			painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
			painter.setBrush(QBrush(QColor(255, 255, 255)));
			painter.setPen(QPen(Qt::Dense7Pattern, 1));

			painter.drawPath(*(layer_qt_path.find(num)->second));

			QByteArray ba;
			QBuffer buffer(&ba);
			buffer.open(QIODevice::WriteOnly);
			image.save(&buffer, "PNG");
			file.write(ba);
		}
	}

	void DLPrint::SaveOnePng(size_t num)
	{
		QPainterPath* painterPath =new QPainterPath;
		painterPath->setFillRule(Qt::WindingFill);

		//画底板
		if (m_config->raft_layers > num) {
			for (ExPolygons::iterator exps = r_layers[num].slices.expolygons.begin(); exps != r_layers[num].slices.expolygons.end(); ++exps) {
				painterPath->addPolygon(poly2Qpoly((*exps).contour));
				for (Polygons::iterator ps = (*exps).holes.begin(); ps != (*exps).holes.end(); ++ps)
					painterPath->addPolygon(poly2Qpoly(*ps));
			}
		}
		else {
			ExPolygons t_expoly;
			for (auto l = layerss.begin(); l != layerss.end(); ++l) {
				Layer _l = *(l->begin() + num - m_config->raft_layers);

				ExPolygons exp;
				if (_l.solid) {
					//整层为固态填充
					exp = _l.slices.expolygons;
				}
				else {
					//内部填充+固态填充
					exp = _l.perimeters.expolygons + _l.solid_infill.expolygons;
					//合并
					exp = union_ex(exp);

				}
				for (ExPolygons::iterator exps = exp.begin(); exps != exp.end(); ++exps)
					t_expoly.emplace_back(*exps);
			}

			Polygons poly = to_polygons(union_ex(t_expoly));
			double area = 0;
			for (Polygons::iterator ps = poly.begin(); ps != poly.end(); ++ps) {
				//求面积
				area += (*ps).area() * SCALING_FACTOR * SCALING_FACTOR;
				painterPath->addPolygon(poly2Qpoly(*ps));
			}
			this->m_areas[num] = area;
		}

		layer_qt_path.insert(std::pair<size_t, QPainterPath*>(num, painterPath));
	}

	void DLPrint::RadiatePoint(const BoundingBoxf3& bb, Pointf3s& ps, float space, int xyz)
	{
		Pointfs pfs;
		switch (xyz)
		{
		case XZ_45:
			if (1) {
				Linef l1(Pointf(bb.min.x, bb.max.z), Pointf(bb.max.x, bb.min.z));
				double a = fabs(l1.a.x - l1.b.x) > fabs(l1.a.y - l1.b.y) ? fabs(l1.a.x - l1.b.x) : fabs(l1.a.y - l1.b.y);//长边
				int b = a / space;//间隔次数
				double x = fabs(l1.a.x - l1.b.x) / b;//x边的间隔距离
				double z = fabs(l1.a.y - l1.b.y) / b;//z边的间隔距离
				for (int i = 1; i < b; ++i) {
					Pointf pf;
					if (l1.a.x < l1.b.x)
						pf.x = l1.a.x + i * x;
					else
						pf.x = l1.b.x + i * x;
					if (l1.a.y < l1.b.y)
						pf.y = l1.a.y + i * z;
					else
						pf.y = l1.b.y + i * z;
					pfs.push_back(pf);//得到二维方向的交点
				}

				int c = fabs(bb.max.y - bb.min.y) / space;//y边间隔次数,间隔距离为space
				for (int j = 1; j < c; ++j) {
					for (auto p = pfs.begin(); p != pfs.end(); ++p) {
						int i = std::distance(pfs.begin(), p);
						auto p1 = pfs.end() - i - 1;
						ps.push_back(Pointf3((*p).x, bb.min.y + j * space, (*p1).y));//y==z
					}
				}
			}
			break;

		case XZ_135:
			if (1) {
				Linef l2(Pointf(bb.min.x, bb.min.z), Pointf(bb.max.x, bb.max.z));
				double a = fabs(l2.a.x - l2.b.x) > fabs(l2.a.y - l2.b.y) ? fabs(l2.a.x - l2.b.x) : fabs(l2.a.y - l2.b.y);//长边
				int b = a / space;//间隔次数
				double x = fabs(l2.a.x - l2.b.x) / b;//x边的间隔距离
				double z = fabs(l2.a.y - l2.b.y) / b;//z边的间隔距离
				for (int i = 1; i < b; ++i) {
					Pointf pf;
					if (l2.a.x < l2.b.x)
						pf.x = l2.a.x + i * x;
					else
						pf.x = l2.b.x + i * x;
					if (l2.a.y < l2.b.y)
						pf.y = l2.a.y + i * z;
					else
						pf.y = l2.b.y + i * z;
					pfs.push_back(pf);//得到二维方向的交点
				}

				int c = fabs(bb.max.y - bb.min.y) / space;//y边间隔次数,间隔距离为space
				for (int j = 1; j < c; ++j) {
					for (auto p = pfs.begin(); p != pfs.end(); ++p) {
						ps.push_back(Pointf3((*p).x, bb.min.y + j * space, (*p).y));
					}
				}
			}
			break;

		case YZ_45:
			if (1) {
				Linef l3(Pointf(bb.min.y, bb.max.z), Pointf(bb.max.y, bb.min.z));
				double a = fabs(l3.a.x - l3.b.x) > fabs(l3.a.y - l3.b.y) ? fabs(l3.a.x - l3.b.x) : fabs(l3.a.y - l3.b.y);//长边
				int b = a / space;//间隔次数
				double y = fabs(l3.a.x - l3.b.x) / b;//y边的间隔距离
				double z = fabs(l3.a.y - l3.b.y) / b;//z边的间隔距离
				for (int i = 1; i < b; ++i) {
					Pointf pf;
					if (l3.a.x < l3.b.x)
						pf.x = l3.a.x + i * y;
					else
						pf.x = l3.b.x + i * y;
					if (l3.a.y < l3.b.y)
						pf.y = l3.a.y + i * z;
					else
						pf.y = l3.b.y + i * z;
					pfs.push_back(pf);//得到二维方向的交点
				}

				int c = fabs(bb.max.x - bb.min.x) / space;//x边间隔次数,间隔距离为space
				for (int j = 1; j < c; ++j) {
					for (auto p = pfs.begin(); p != pfs.end(); ++p) {
						int i = std::distance(pfs.begin(), p);
						auto p1 = pfs.end() - i - 1;
						ps.push_back(Pointf3(bb.min.x + j * space, (*p).x, (*p1).y));
					}
				}
			}
			break;

		case YZ_135:
			if (1) {
				Linef l4(Pointf(bb.min.y, bb.min.z), Pointf(bb.max.y, bb.max.z));
				double a = fabs(l4.a.x - l4.b.x) > fabs(l4.a.y - l4.b.y) ? fabs(l4.a.x - l4.b.x) : fabs(l4.a.y - l4.b.y);//长边
				int b = a / space;//间隔次数
				double y = fabs(l4.a.x - l4.b.x) / b;//y边的间隔距离
				double z = fabs(l4.a.y - l4.b.y) / b;//z边的间隔距离
				for (int i = 1; i < b; ++i) {
					Pointf pf;
					if (l4.a.x < l4.b.x)
						pf.x = l4.a.x + i * y;
					else
						pf.x = l4.b.x + i * y;
					if (l4.a.y < l4.b.y)
						pf.y = l4.a.y + i * z;
					else
						pf.y = l4.b.y + i * z;
					pfs.push_back(pf);//得到二维方向的交点
				}

				int c = fabs(bb.max.x - bb.min.x) / space;//x边间隔次数,间隔距离为space
				for (int j = 1; j < c; ++j) {
					for (auto p = pfs.begin(); p != pfs.end(); ++p) {
						ps.push_back(Pointf3(bb.min.x + j * space, (*p).x, (*p).y));
					}
				}
			}
			break;
		default:
			break;
		}
	}

	//将点按z轴高度按升序排列
	inline void pointf3sSortZ(Pointf3s& ps)
	{
		for (int i = 0; i < ps.size(); ++i) {
			for (auto b = ps.begin(); b != ps.end() - 1; ++b) {
				auto c = b + 1;
				if ((*b).z > (*c).z) {
					Pointf3 temp((*b).x, (*b).y, (*b).z);
					(*b).x = (*c).x; (*b).y = (*c).y; (*b).z = (*c).z;
					(*c).x = temp.x; (*c).y = temp.y; (*c).z = temp.z;
				}
			}
		}
	}

	void DLPrint::RadiateInter(const TriangleMesh* mesh,const Pointf3s& ps, linef3s& lines, int xyz)
	{
		Vectorf3 xz_45(1, 0, 1);
		Vectorf3 xz_135(-1, 0, 1);
		Vectorf3 yz_45(0, 1, 1);
		Vectorf3 yz_135(0, -1, 1);

		switch (xyz)
		{
		case XZ_45:
			for (auto p = ps.begin(); p != ps.end(); ++p) {
				Pointf3s pf3s;
				Pointf3 a;
				for (int i = 0; i < mesh->stl.stats.number_of_facets; ++i) {
					stl_facet f = mesh->stl.facet_start[i];
					//排除不与二维面相交的三角面
					if ((f.vertex[0].y > (*p).y&& f.vertex[1].y > (*p).y&& f.vertex[2].y > (*p).y) ||
						(f.vertex[0].y < (*p).y && f.vertex[1].y < (*p).y && f.vertex[2].y < (*p).y))
						continue;
					else {
						//得到射线与平面的交点
						if (Ray2TrianglePoint(f, xz_45, (*p), a))
							pf3s.push_back(a);
					}
				}
				//支撑点按z轴高度排序
				if (!pf3s.empty()) {
					pointf3sSortZ(pf3s);

					//得到支撑柱
					for (auto d = pf3s.begin(); d != pf3s.end() && d != pf3s.end() - 1; ++d) {
						lines.push_back(Linef3(*d, *(d + 1)));
						++d;
					}
				}
			}

			break;

		case XZ_135:
			for (auto p = ps.begin(); p != ps.end(); ++p) {
				Pointf3s pf3s;
				Pointf3 a;
				for (int i = 0; i < mesh->stl.stats.number_of_facets; ++i) {
					stl_facet f = mesh->stl.facet_start[i];
					//排除不与二维面相交的三角面
					if ((f.vertex[0].y > (*p).y&& f.vertex[1].y > (*p).y&& f.vertex[2].y > (*p).y) ||
						(f.vertex[0].y < (*p).y && f.vertex[1].y < (*p).y && f.vertex[2].y < (*p).y))
						continue;
					else {
						//得到射线与平面的交点
						if (Ray2TrianglePoint(f, xz_135, (*p), a))
							pf3s.push_back(a);
					}
				}
				//支撑点按z轴高度排序
				if (!pf3s.empty()) {
					pointf3sSortZ(pf3s);

					//得到支撑柱
					for (auto d = pf3s.begin(); d != pf3s.end() && d != pf3s.end() - 1; ++d) {
						lines.push_back(Linef3(*d, *(d + 1)));
						++d;
					}
				}
			}
			break;

		case YZ_45:
			for (auto p = ps.begin(); p != ps.end(); ++p) {
				Pointf3s pf3s;
				Pointf3 a;
				for (int i = 0; i < mesh->stl.stats.number_of_facets; ++i) {
					stl_facet f = mesh->stl.facet_start[i];
					//排除不与二维面相交的三角面
					if ((f.vertex[0].x > (*p).x&& f.vertex[1].x > (*p).x&& f.vertex[2].x > (*p).x) ||
						(f.vertex[0].x < (*p).x && f.vertex[1].x < (*p).y && f.vertex[2].x < (*p).x))
						continue;
					else {
						//得到射线与平面的交点
						if (Ray2TrianglePoint(f, yz_45, (*p), a))
							pf3s.push_back(a);
					}
				}
				//支撑点按z轴高度排序
				if (!pf3s.empty()) {
					pointf3sSortZ(pf3s);

					//得到支撑柱
					for (auto d = pf3s.begin(); d != pf3s.end() && d != pf3s.end() - 1; ++d) {
						lines.push_back(Linef3(*d, *(d + 1)));
						++d;
					}
				}
			}
			break;

		case YZ_135:
			for (auto p = ps.begin(); p != ps.end(); ++p) {
				Pointf3s pf3s;
				Pointf3 a;
				for (int i = 0; i < mesh->stl.stats.number_of_facets; ++i) {
					stl_facet f = mesh->stl.facet_start[i];
					//排除不与二维面相交的三角面
					if ((f.vertex[0].x > (*p).x&& f.vertex[1].x > (*p).x&& f.vertex[2].x > (*p).x) ||
						(f.vertex[0].x < (*p).x && f.vertex[1].x < (*p).y && f.vertex[2].x < (*p).x))
						continue;
					else {
						//得到射线与平面的交点
						if (Ray2TrianglePoint(f, yz_135, (*p), a))
							pf3s.push_back(a);
					}
				}
				//支撑点按z轴高度排序
				if (!pf3s.empty()) {
					pointf3sSortZ(pf3s);

					//得到支撑柱
					for (auto d = pf3s.begin(); d != pf3s.end() && d != pf3s.end() - 1; ++d) {
						lines.push_back(Linef3(*d, *(d + 1)));
						++d;
					}
				}
			}
			break;

		default:
			break;
		}
	}

	void DLPrint::InsertSupport(size_t id, TreeSupport* s)
	{
		DelTreeSupport(id);
		treeSupports.insert(std::make_pair(id, s));
	}

	//传出指针的引用
	TreeSupport* DLPrint::GetTreeSupport(size_t id)
	{
		auto s1 = treeSupports.find(id);
		if (s1 != treeSupports.end())
			return (*s1).second;

		return nullptr;
	}

	void DLPrint::DelAllInsideSup() {
		while (!inside_supports.empty()) {
			auto s = inside_supports.begin();
			delete (*s).second;
		}
		inside_supports.swap(std::map <size_t, linef3s*>());
	}

	void DLPrint::DelAllSupport()
	{
		while (!treeSupports.empty()) {
			auto s = treeSupports.begin();
			delete (*s).second;
		}
		treeSupports.swap(std::map<int, TreeSupport*>());
	}

}
#include"DLPrint.h"
#include"Geometry.hpp"
#include"ClipperUtils.hpp"
#include"qstring.h"
#include"qfile.h"
#include"qtextstream.h"
#include"qimage.h"
#include"qpainter.h"
#include"tool.h"
#include"qbuffer.h"
#include"qdebug.h"

#include "Setting.h"
extern Setting e_setting;

namespace Slic3r {

	DLPrint::DLPrint(Model* _model)
		:model(_model)
	{
		CirclePoints(circle, 15);
	}

	void DLPrint::slice(const std::vector<TriangleMesh>& support_meshs, QProgressBar* progress)
	{
		m_progress = progress;
		bool ret = true;
		int count = 0;//实例计数
		for (auto o = model->objects.begin(); o != model->objects.end(); ++o) {
			for (auto i = (*o)->instances.begin(); i != (*o)->instances.end(); ++i) {
				++count;
				TriangleMesh mesh = (*o)->volumes[0]->mesh;
				(*i)->transform_mesh(&mesh);
				if (ret) {
					this->bb = mesh.bounding_box();
					ret = false;
				}
				else {
					this->bb.defined = true;
					this->bb.merge(mesh.bounding_box());
				}
			}
		}

		const float lh = this->config.layer_height.value;
		std::vector<Layer> ls;
		ls.push_back(Layer(lh / 2, lh));
		while (ls.back().print_z + lh / 2 <= this->bb.max.z) {
			ls.push_back(Layer(ls.back().print_z + lh / 2, ls.back().print_z + lh));
		}

		layer_num = ls.size();
		std::vector<float> slice_z;
		//准备切片并得到切片层
		{
			for (size_t i = 0; i < ls.size(); ++i) {
				slice_z.push_back(ls[i].slice_z);
			}

			while (!layers.empty())
			{
				auto l = layers.begin();
				(*l).clear();
				layers.erase(l);
			}

			int aaa = 0;
			for (auto o = this->model->objects.begin(); o != this->model->objects.end(); ++o) {
				for (auto i = (*o)->instances.begin(); i != (*o)->instances.end(); ++i) {
					_layers.clear();

					for (auto a = ls.begin(); a != ls.end(); ++a)
						_layers.push_back(*a);

					//切片
					std::vector<ExPolygons> slices;
					TriangleMesh mesh = (*o)->raw_mesh();
					(*i)->transform_mesh(&mesh);

					TriangleMeshSlicer<Z>(&mesh).slice(slice_z, &slices, config.threads);//切片函数
					for (size_t i = 0; i < slices.size(); ++i)
						_layers.at(i).slices.expolygons = slices[i];

					//切片进度条范围21->98
					if (progress != NULL) {
						int unit = (aaa + 1) / (count / (98 - 21) + 1);
						if (progress->value() < 21 + unit)
							progress->setValue(21 + unit);
					}
					++aaa;

					//抽空
					if (config.hollow_out.value) {

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

							if (config.fill_pattern.value == ipHoneycomb) {
								//计算填充密度
								double space = (1 - double(config.fill_density / 100)) * 20;
								space = 1 >= space ? 1 : space;

								//壁厚
								double wall = double(config.fill_density / 100) * 2;
								wall = wall <= 0.1 ? 0.1 : wall;

								BoundingBox box;
								box.merge(Point::new_scale(bb.min.x, bb.min.y));
								box.merge(Point::new_scale(bb.max.x, bb.max.y));
								pattern.push_back(generate_honeycomb_pattern(box, wall, space));
							}

							parallelize<size_t>(
								0,
								_layers.size() - 1,
								boost::bind(&DLPrint::_infill_layer, this, _1, pattern),
								this->config.threads
								);
						}
					}
					layers.push_back(_layers);
				}

			}

			//切支撑
			{
				TriangleMesh SMesh;
				for (auto s = support_meshs.begin(); s != support_meshs.end(); ++s)
					SMesh.merge(*s);

				_layers.clear();
				for (auto a = ls.begin(); a != ls.end(); ++a)
					_layers.push_back(*a);

				std::vector<ExPolygons> slices;
				TriangleMeshSlicer<Z>(&SMesh).slice(slice_z, &slices, config.threads);//切片函数
				for (size_t i = 0; i < slices.size(); ++i)
					_layers.at(i).slices.expolygons = slices[i];
				layers.push_back(_layers);
			}
		}

		//生成底板
		if (this->config.raft_layers > 0) {
			float r = config.layer_height*config.raft_layers;

			double L = double(e_setting.m_printers.begin()->length / 2);
			double W = double(e_setting.m_printers.begin()->width / 2);

			BoundingBoxf bbb;
			bbb.min.x = this->bb.min.x - config.raft_offset > -L ? this->bb.min.x - config.raft_offset : -L;
			bbb.min.y = this->bb.min.y - config.raft_offset > -W ? this->bb.min.y - config.raft_offset : -W;
			bbb.max.x = this->bb.max.x + config.raft_offset < L ? this->bb.max.x + config.raft_offset : L;
			bbb.max.y = this->bb.max.y + config.raft_offset < W ? this->bb.max.y + config.raft_offset : W;

			ExPolygons raft;
			Polygon p;
			p.points.push_back(Point(scale_(bbb.min.x), scale_(bbb.min.y)));
			p.points.push_back(Point(scale_(bbb.min.x), scale_(bbb.max.y)));
			p.points.push_back(Point(scale_(bbb.max.x), scale_(bbb.max.y)));
			p.points.push_back(Point(scale_(bbb.max.x), scale_(bbb.min.y)));
			ExPolygon _raft;
			_raft.contour = p;
			raft.push_back(_raft);

			for (int i = this->config.raft_layers; i >= 1; --i) {
				this->r_layers.insert(this->r_layers.begin(), Layer(0, lh * i));
				this->r_layers.front().slices.expolygons = raft;
			}

			// prepend total raft height to all sliced layers
			for (auto l = this->layers.begin(); l != this->layers.end(); ++l) {
				for (auto l1 = (*l).begin(); l1 != (*l).end(); ++l1) {
					(*l1).print_z += lh*this->config.raft_layers;
				}
			}

			for (auto i = this->inside_supports.begin(); i != this->inside_supports.end(); ++i) {
				linef3s* l = (*i).second;
				for (auto l1 = l->begin(); l1 != l->end(); ++l1) {
					(*l1).a.z += r;
					(*l1).b.z += r;
				}
			}
		}
	}

	void DLPrint::_infill_layer(size_t i, ExPolygons pattern)
	{
		Layer &layer = this->_layers[i];

		// const float shell_thickness = this->config.get_abs_value("perimeter_extrusion_width", this->config.layer_height.value);
		const float shell_thickness = config.wall_thickness;
		// In order to detect what regions of this layer need to be solid,
		// perform an intersection with layers within the requested shell thickness.
		Polygons internal = layer.slices;
		for (size_t j = 0; j < this->_layers.size(); ++j) {
			const Layer &other = this->_layers[j];
			if (std::abs(other.print_z - layer.print_z) > shell_thickness) continue;

			if (j == 0 || j == this->_layers.size() - 1) {
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
		if (config.fill_pattern.value == ipHoneycomb)
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

		// Generate perimeter(s).
		layer.perimeters << diff_ex(
			layer.slices,
			offset(layer.slices, -scale_(shell_thickness))
		);
	}

	ExPolygon DLPrint::generate_honeycomb_pattern(BoundingBox box, double wall, double radius)
	{
		box.offset(scale_(radius*2));

		ExPolygon pattern;
		pattern.contour = box.polygon();
		
		BoundingBoxf _box;
		_box.max.x = unscale(box.max.x);
		_box.max.y = unscale(box.max.y);
		_box.min.x = unscale(box.min.x);
		_box.min.y = unscale(box.min.y);

		Pointf p = _box.min;

		std::vector<Pointf> ps;

		generate_honeycomb(_box, ps, radius + wall / 2, p, 30, true);

		std::vector<Pointf> circle1;
		CirclePoints(circle1, 30);
		Points ps1;
		for (auto s = circle1.begin(); s != circle1.end(); ++s)
			ps1.push_back(Point(scale_((*s).x), scale_((*s).y)));

		Polygon _hole(ps1);

		Polygon hole;
		for (auto p1 = ps.begin(); p1 != ps.end(); ++p1) {
			hole = _hole;
			hole.scale(radius*1.1);
			hole.translate(scale_((*p1).x), scale_((*p1).y));
			pattern.holes.push_back(hole);
		}

		return pattern;
	}

	void DLPrint::generate_honeycomb(BoundingBoxf box, std::vector<Pointf>& ps, double radius, Pointf p, double angle, bool again)
	{
		//超出边界退出
		if ((p.x + radius) < box.min.x || (p.x - radius) > box.max.x
			|| (p.y + radius) < box.min.y || (p.y - radius) > box.max.y)
			return;

		if (again)
			ps.push_back(p);

		double _angle;

		if (angle != 90)
			_angle = angle / 180 * PI;

		Pointf p1;

		if (angle == 30)
		{
			p1.x = p.x + cos(_angle)*radius;
			p1.y = p.y + sin(_angle)*radius;

			generate_honeycomb(box, ps, radius, p1, 30, true);
			generate_honeycomb(box, ps, radius, p1, 90, false);
			generate_honeycomb(box, ps, radius, p1, 330, false);
		}
		else if (angle == 90)
		{
			p1.x = p.x;
			p1.y = p.y + radius;

			generate_honeycomb(box, ps, radius, p1, 90, true);
		}
		else if (angle == 330)
		{
			p1.x = p.x + cos(_angle)*radius;
			p1.y = p.y + sin(_angle)*radius;

			generate_honeycomb(box, ps, radius, p1, 330, true);
		}
	}

	ExPolygon DLPrint::generate_pattern(BoundingBox box)
	{
		ExPolygon pattern;
		pattern.contour = box.polygon();

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
				hole.translate(scale_(_box.min.x + i * space + space / 2),scale_( _box.min.y + j * space + space / 2));
				pattern.holes.push_back(hole);
			}
		}

		return pattern;
	}


	void DLPrint::generate_inside_support(size_t id, TriangleMesh* mesh)
	{
		BoundingBoxf3 bb = mesh->bounding_box();
		linef3s* lines = new linef3s();
		Pointf3s pointf3_xz_45, pointf3_xz_135, pointf3_yz_45, pointf3_yz_135;

		//计算填充密度
		double space=(1-double(config.fill_density/100))*10;
		space = config.support_radius >= space ? config.support_radius + 1 : space;

		radiate_point(bb, pointf3_xz_45, space, XZ_45);
		radiate_point(bb, pointf3_xz_135, space, XZ_135);
		radiate_point(bb, pointf3_yz_45, space, YZ_45);
		radiate_point(bb, pointf3_yz_135, space, YZ_135);

		radiate_intersection(mesh, pointf3_xz_45, *lines, XZ_45);
		radiate_intersection(mesh, pointf3_xz_135, *lines, XZ_135);
		radiate_intersection(mesh, pointf3_yz_45, *lines, YZ_45);
		radiate_intersection(mesh, pointf3_yz_135, *lines, YZ_135);
		inside_supports.insert(std::make_pair(id, lines));
	}


	TreeSupport* DLPrint::generate_support(size_t id, TriangleMesh* mesh, QProgressBar* progress)
	{
		mesh->require_shared_vertices_faces();
		mesh->extract_feature_face(config.angle);

		progress->setValue(20);

		TreeSupport* s = new TreeSupport;
		s->support_point = mesh->feature_point(progress);
		s->support_point_face = mesh->feature_point_face(config.space, progress);
		s->generate_tree_support(*mesh, config.leaf_num, config.threads, progress, config.support_top_height);
		return s;
	}

	//更新键值只对删除模型时有效
	bool DLPrint::delete_tree_support(size_t id)
	{
		auto s = treeSupports.find(id);
		if (s != treeSupports.end()) {
			delete (*s).second;
			treeSupports.erase(s);
			return true;
		}

		return false;
	}

	void DLPrint::savePNG(QString ini_file1)
	{
		double _area = 0;

		QString file = ini_file1;
		file.append("/buildscipt.ini");
		QFile _file(file);

		if (_file.open(QFile::WriteOnly | QFile::Truncate))
		{
			QTextStream stream(&_file);
			stream << "Slice thickness = " << config.layer_height << "\n";
			stream << "norm illumination time = " << config.normIlluTime << "\n";
			stream << "norm illumination inttersity = " << config.norm_inttersity << "\n";
			stream << "number of override slices = " << config.overLayer << "\n";
			stream << "override illumination time = " << config.overIlluTime << "\n";
			stream << "override illumination inttersity =" << config.over_inttersity << "\n";
			stream << "first layer_illumination_time = " << config.overIlluTime << "\n";
			stream << "first illumination inttersity = " << config.over_inttersity << "\n";
			stream << "number of slices = " << layer_num + config.raft_layers << "\n";
			stream << "length = " << this->bb.max.x - this->bb.min.x << "\n";
			stream << "width = " << this->bb.max.y - this->bb.min.y << "\n";
			stream << "height = " << this->bb.max.z + config.layer_height*config.raft_layers << "\n";

			this->ini_file = ini_file1;



			parallelize<size_t>(
				0,
				layer_num + config.raft_layers - 1,
				boost::bind(&DLPrint::saveOnePNG, this, _1),
				this->config.threads
				);

			double area = 0;
			for (auto a = areas.begin(); a != areas.end(); ++a) {
				area += (*a)*config.layer_height;
			}
			areas.clear();

			_area = area / 1000;
			stream << "model volume = " << _area << "\n";

			float aaa = 0;
			for (int num = 0; num < layer_num + config.raft_layers; ++num) {
				if (num < config.raft_layers)
					aaa = (this->bb.max.x - this->bb.min.x)*(this->bb.max.y - this->bb.min.y);
				else
					aaa = *(areas.begin() + num - config.raft_layers);
				if (num < config.overLayer) {
					//长曝光层
					stream << num*config.layer_height << " , " << "slice" << num << ".png , " << aaa << "\n";
				}
				else {
					//正常曝光层
					stream << num*config.layer_height << " , " << "slice" << num << ".png , " << aaa << "\n";
				}
			}

		}
		_file.close();

		layers.clear();
		areas.clear();
		_layers.clear();
		r_layers.clear();
		if (!inside_supports.empty())
		{
			auto i = inside_supports.begin();
			delete (*i).second;
			inside_supports.erase(i);
		}
		inside_supports.clear();
	}

	void DLPrint::saveOnePNG(size_t num)
	{
		QImage image(1920, 1080, QImage::Format_RGB32);
		QPainter painter(&image);
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
		painter.setBrush(QBrush(QColor(255, 255, 255)));
		painter.setPen(QPen(Qt::Dense7Pattern, 1));

		QPainterPath _path;
		_path.setFillRule(Qt::WindingFill);

		float raft = config.layer_height*config.raft_layers;//底板高度
		float now = config.layer_height*num;

		//画底板
		if (config.raft_layers > num) {
			ExPolygons exp = r_layers[num].slices.expolygons;
			for (ExPolygons::iterator exps = exp.begin(); exps != exp.end(); ++exps) {
				ExPolygon _exps = *exps;
				QPolygonF qp = polTpQpol(_exps.contour);
				_path.addPolygon(qp);
				for (Polygons::iterator ps = _exps.holes.begin(); ps != _exps.holes.end(); ++ps) {
					QPolygonF qp = polTpQpol(*ps);
					_path.addPolygon(qp);
				}
			}
		}
		else {
			ExPolygons temp;
			ExPolygons contour;
			size_t num1 = num - config.raft_layers;
			for (auto l = layers.begin(); l != layers.end(); ++l) {
				std::vector<Layer>* _layers1 = &(*l);
				Layer _l = *(_layers1->begin() + num1);

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
				for (ExPolygons::iterator exps = exp.begin(); exps != exp.end(); ++exps) {
					//存储外部轮廓多边形
					ExPolygon t;
					t.contour = (*exps).contour;
					contour.push_back(t);

					temp.push_back(*exps);
				}
			}

		//画内部支撑
			if (config.hollow_out && !inside_supports.empty()&&config.fill_pattern.value==ip3DSupport) {
				ExPolygons ss1;
				for (auto i = inside_supports.begin(); i != inside_supports.end(); ++i) {
					std::vector<Linef3>* inside = (*i).second;

					for (auto l3 = inside->begin(); l3 != inside->end(); ++l3) {

						if (now > (*l3).a.z&&now < (*l3).b.z) {
							ExPolygon s1;
							//求横梁与当前面的交点
							Vectorf3 planeVector(0, 0, 1);
							Pointf3 planePoint(0, 0, now);
							Vectorf3 lineVector((*l3).b.x - (*l3).a.x, (*l3).b.y - (*l3).a.y, (*l3).b.z - (*l3).a.z);
							Pointf3 point = CalPlaneLineIntersectPoint(planeVector, planePoint, lineVector, (*l3).a);
							//将模型支撑在不同实例上画出

							for (std::vector<Pointf>::iterator p = circle.begin(); p != circle.end() - 1; ++p) {
								Pointf _p = *p;
								_p.scale(config.support_radius);
								_p.translate(point.x, point.y);
								s1.contour.points.push_back(Point(scale_(_p.x), scale_(_p.y)));
							}
							ss1.push_back(s1);

						}
					}

				}
				ss1 = union_ex(ss1);
				//去除多余线段
				contour = intersection_ex(ss1, contour);//a*b求交
				temp = diff_ex(temp, ss1);//a-b
				temp = temp + contour;
			}

			temp = union_ex(temp);

			Polygons poly = to_polygons(temp);
			double area = 0;
			for (Polygons::iterator ps = poly.begin(); ps != poly.end(); ++ps) {
				//求面积
				area += (*ps).area()*SCALING_FACTOR*SCALING_FACTOR;

				QPolygonF qp = polTpQpol(*ps);
				_path.addPolygon(qp);
			}
			areas.push_back(area);
		}

		painter.drawPath(_path);
		QString path(QObject::tr("/slice%1.png").arg(num));

		QString _pa;
		_pa.append(ini_file);
		_pa.append(path);
		QFile file(_pa);

		if (!file.open(QIODevice::ReadWrite))
			return;
		QByteArray ba;
		QBuffer buffer(&ba);
		buffer.open(QIODevice::WriteOnly);
		image.save(&buffer, "PNG");
		file.write(ba);
	}

	QPolygonF DLPrint::polTpQpol(Polygon& p)//将Polygon装换为QPolygonF
	{
		QVector<QPointF> f;
		for (Points::iterator ps = p.points.begin(); ps != p.points.end(); ++ps) {
			Pointf pf(unscale((*ps).x), unscale((*ps).y));
			pf.scale(e_setting.m_printers.begin()->factor);
			f.push_back(QPointF(pf.x + 960, pf.y + 540));
		}
		return QPolygonF(f);
	}

	void DLPrint::radiate_point(BoundingBoxf3 bb, Pointf3s& ps, float space, int xyz)
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
						pf.x = l1.a.x + i*x;
					else
						pf.x = l1.b.x + i*x;
					if (l1.a.y < l1.b.y)
						pf.y = l1.a.y + i*z;
					else
						pf.y = l1.b.y + i*z;
					pfs.push_back(pf);//得到二维方向的交点
				}

				int c = fabs(bb.max.y - bb.min.y) / space;//y边间隔次数,间隔距离为space
				for (int j = 1; j < c; ++j) {
					for (auto p = pfs.begin(); p != pfs.end(); ++p) {
						int i = std::distance(pfs.begin(), p);
						auto p1 = pfs.end() - i - 1;
						ps.push_back(Pointf3((*p).x, bb.min.y + j*space, (*p1).y));//y==z
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
						pf.x = l2.a.x + i*x;
					else
						pf.x = l2.b.x + i*x;
					if (l2.a.y < l2.b.y)
						pf.y = l2.a.y + i*z;
					else
						pf.y = l2.b.y + i*z;
					pfs.push_back(pf);//得到二维方向的交点
				}

				int c = fabs(bb.max.y - bb.min.y) / space;//y边间隔次数,间隔距离为space
				for (int j = 1; j < c; ++j) {
					for (auto p = pfs.begin(); p != pfs.end(); ++p) {
						ps.push_back(Pointf3((*p).x, bb.min.y + j*space, (*p).y));
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
						pf.x = l3.a.x + i*y;
					else
						pf.x = l3.b.x + i*y;
					if (l3.a.y < l3.b.y)
						pf.y = l3.a.y + i*z;
					else
						pf.y = l3.b.y + i*z;
					pfs.push_back(pf);//得到二维方向的交点
				}

				int c = fabs(bb.max.x - bb.min.x) / space;//x边间隔次数,间隔距离为space
				for (int j = 1; j < c; ++j) {
					for (auto p = pfs.begin(); p != pfs.end(); ++p) {
						int i = std::distance(pfs.begin(), p);
						auto p1 = pfs.end() - i - 1;
						ps.push_back(Pointf3(bb.min.x + j*space, (*p).x, (*p1).y));
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
						pf.x = l4.a.x + i*y;
					else
						pf.x = l4.b.x + i*y;
					if (l4.a.y < l4.b.y)
						pf.y = l4.a.y + i*z;
					else
						pf.y = l4.b.y + i*z;
					pfs.push_back(pf);//得到二维方向的交点
				}

				int c = fabs(bb.max.x - bb.min.x) / space;//x边间隔次数,间隔距离为space
				for (int j = 1; j < c; ++j) {
					for (auto p = pfs.begin(); p != pfs.end(); ++p) {
						ps.push_back(Pointf3(bb.min.x + j*space, (*p).x, (*p).y));
					}
				}
			}
			break;
		default:
			break;
		}
	}

	void DLPrint::radiate_intersection(TriangleMesh* mesh, Pointf3s& ps, linef3s& lines, int xyz)
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
					if ((f.vertex[0].y > (*p).y&&f.vertex[1].y > (*p).y&&f.vertex[2].y > (*p).y) ||
						(f.vertex[0].y < (*p).y&&f.vertex[1].y < (*p).y&&f.vertex[2].y < (*p).y))
						continue;
					else {
						//得到射线与平面的交点
						if (line_to_triangle_point(f, xz_45, (*p), a))
							pf3s.push_back(a);
					}
				}
				//支撑点按z轴高度排序
				if (!pf3s.empty()) {
					pointf3_sort_z(pf3s);

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
					if ((f.vertex[0].y > (*p).y&&f.vertex[1].y > (*p).y&&f.vertex[2].y > (*p).y) ||
						(f.vertex[0].y < (*p).y&&f.vertex[1].y < (*p).y&&f.vertex[2].y < (*p).y))
						continue;
					else {
						//得到射线与平面的交点
						if (line_to_triangle_point(f, xz_135, (*p), a))
							pf3s.push_back(a);
					}
				}
				//支撑点按z轴高度排序
				if (!pf3s.empty()) {
					pointf3_sort_z(pf3s);

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
					if ((f.vertex[0].x > (*p).x&&f.vertex[1].x > (*p).x&&f.vertex[2].x > (*p).x) ||
						(f.vertex[0].x < (*p).x&&f.vertex[1].x < (*p).y&&f.vertex[2].x < (*p).x))
						continue;
					else {
						//得到射线与平面的交点
						if (line_to_triangle_point(f, yz_45, (*p), a))
							pf3s.push_back(a);
					}
				}
				//支撑点按z轴高度排序
				if (!pf3s.empty()) {
					pointf3_sort_z(pf3s);

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
					if ((f.vertex[0].x > (*p).x&&f.vertex[1].x > (*p).x&&f.vertex[2].x > (*p).x) ||
						(f.vertex[0].x < (*p).x&&f.vertex[1].x < (*p).y&&f.vertex[2].x < (*p).x))
						continue;
					else {
						//得到射线与平面的交点
						if (line_to_triangle_point(f, yz_135, (*p), a))
							pf3s.push_back(a);
					}
				}
				//支撑点按z轴高度排序
				if (!pf3s.empty()) {
					pointf3_sort_z(pf3s);

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

	void DLPrint::pointf3_sort_z(std::vector<Pointf3>& ps)
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

	int DLPrint::instanceToId(ModelInstance* i)
	{
		for (auto o = model->objects.begin(); o != model->objects.end(); ++o) {
			int a = std::distance(model->objects.begin(), o);
			for (auto n = (*o)->instances.begin(); n != (*o)->instances.end(); ++n) {
				int b = std::distance((*o)->instances.begin(), n);
				if (i == *n) {
					return a * InstanceNum + b;
				}
			}
		}
		return -1;
	}

	void DLPrint::insertSupports(size_t id, TreeSupport* s)
	{
		delete_tree_support(id);
		treeSupports.insert(std::make_pair(id, s));
	}

	//传出指针的引用
	TreeSupport* DLPrint::chilck_tree_support(size_t id)
	{
		auto s1 = treeSupports.find(id);
		if (s1 != treeSupports.end())
			return (*s1).second;

		return nullptr;
	}


}
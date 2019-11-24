#include"Interface.h"
#include"IO.hpp"
#include <qdebug.h>


namespace Slic3r{

Interface::Interface(DLPrinter* _dlprinter)
	:dlprinter(_dlprinter)
{
	model = new Model();
	dlprint = new DLPrint(model,dlprinter);
}

size_t Interface::load_model(std::string file, int format)
{
	//添加一个已有模型对象的实例
	for (auto object = model->objects.begin(); object != model->objects.end(); ++object){
		if ((*object)->input_file == file){
			return find_id((*object)->add_instance());
		}
	}
	//添加一个模型对象
	ModelObject* temp=model->add_object();
	TriangleMesh* T = new TriangleMesh();

	if (format == 0)
		IO::STL::read(file, T);
	else if (format == 1) {
		IO::OBJ::read(file, T);
	}
	//else if (format == 2)
	//	IO::AMF::read(file, T);

	T->repair();

	temp->name = file;
	temp->input_file = file;
	temp->add_volume(*T);
	model_center(temp->volumes[0]->mesh);

	delete T;

	return find_id(temp->add_instance());
}

ModelInstance* Interface::addInstance(size_t id)
{
	size_t a = id / InstanceNum;
	ModelObject* o = find_object(id);
	ModelInstance* i = find_instance(id);
	return o->add_instance(*i);
}

void Interface::delete_model(size_t id)
{
	size_t a = id / InstanceNum;
	size_t i = id % InstanceNum;//模型对象实例的id
	ModelObject* o = find_object(id);
	if (o != NULL) {
		if (o->instances.size() == 1) {
			model->delete_object(a);
		}
		else
			o->delete_instance(i);
	}
}

void Interface::clear_model()
{
	model->clear_materials();
	model->clear_objects();
}

void Interface::generate_id_support(size_t id, TreeSupport*&s, QProgressBar* progress)
{
	ModelInstance* instance = find_instance(id);
	ModelObject* object = instance->get_object();
	TriangleMesh mesh(object->volumes[0]->mesh);
	instance->transform_mesh(&mesh);
	progress->setValue(15);
	dlprint->generate_support(id, s, &mesh, progress);
}

void Interface::delete_support(size_t id)
{
	dlprint->delete_tree_support(id);
}

void Interface::generate_all_inside_support()
{
	delete_all_inside_support();
	if (dlprint->config.hollow_out&&dlprint->config.fill_pattern == ip3DSupport) {
		for (auto o = model->objects.begin(); o != model->objects.end(); ++o) {
			size_t a = std::distance(model->objects.begin(), o);
			for (auto i = (*o)->instances.begin(); i != (*o)->instances.end(); ++i) {
				if ((*i)->exist) {
					size_t b = std::distance((*o)->instances.begin(), i);
					size_t id = a * InstanceNum + b;
					TriangleMesh mesh((*o)->volumes[0]->mesh);
					(*i)->transform_mesh(&mesh);
					dlprint->generate_inside_support(id, &mesh);
				}
			}
		}
	}
}

void Interface::delete_all_support()
{
	while (!dlprint->treeSupports.empty())
	{
		auto s = dlprint->treeSupports.begin();
		//delete (*s).second;//不要删除实际数据，等redo手动删除
		dlprint->treeSupports.erase(s);
	}
}

void Interface::delete_all_inside_support()
{
	while (!dlprint->inside_supports.empty())
	{
		auto s = dlprint->inside_supports.begin();
		delete (*s).second;
		dlprint->inside_supports.erase(s);
	}
}

void Interface::model_center(TriangleMesh& mesh)
{
	Pointf _origin((mesh.stl.stats.max.x - mesh.stl.stats.min.x) / 2 + mesh.stl.stats.min.x,
		(mesh.stl.stats.max.y - mesh.stl.stats.min.y) / 2 + mesh.stl.stats.min.y);

	mesh.translate(-_origin.x, -_origin.y, -mesh.stl.stats.min.z);
}

void Interface::model_lift(double distance)
{
	for (auto o = model->objects.begin(); o != model->objects.end(); ++o) {
		for (auto i = (*o)->instances.begin(); i != (*o)->instances.end(); ++i) {
			(*i)->z_translation += distance;
		}
	}
}

void Interface::wirteStlBinary(const std::string& outFile, TriangleMesh& supportMesh)
{
	TriangleMesh mesh = model->mesh();
	mesh.merge(supportMesh);

	//生成底板
	//if (h > 0) {
	//	mesh.translate(0, 0, h);
	//	BoundingBoxf3 box = model->bounding_box();
	//	Pointf3 a1(box.min.x, box.min.y, h);
	//	Pointf3 a2(box.min.x, box.max.y, h);
	//	Pointf3 a4(box.max.x, box.min.y, h);
	//	Pointf3 a3(box.max.x, box.max.y, h);
	//
	//	Pointf3 b1(box.min.x, box.min.y, 0);
	//	Pointf3 b2(box.min.x, box.max.y, 0);
	//	Pointf3 b4(box.max.x, box.min.y, 0);
	//	Pointf3 b3(box.max.x, box.max.y, 0);
	//
	//	std_add_face1(a1, a2, a3, mesh);
	//	std_add_face1(a4, a1, a3, mesh);
	//
	//	std_add_face1(a1, b1, a2, mesh);
	//	std_add_face1(b1, b2, a2, mesh);
	//
	//	std_add_face1(a2, b2, a3, mesh);
	//	std_add_face1(b2, b3, a3, mesh);
	//
	//	std_add_face1(a3, b3, a4, mesh);
	//	std_add_face1(b3, b4, a4, mesh);
	//
	//	std_add_face1(a4, b4, a1, mesh);
	//	std_add_face1(b4, b1, a1, mesh);
	//
	//	std_add_face1(b1, b2, b3, mesh);
	//	std_add_face1(b1, b3, b4, mesh);
	//}

	mesh.write_binary(outFile);
}

Pointfs Interface::autoArrange(double dis)
{
	double L = double(dlprinter->length / 2);
	double W = double(dlprinter->width / 2);

	BoundingBoxf box;
	box.min.x = -L;
	box.min.y = -W;
	box.max.x = L;
	box.max.y = W;
	box.defined = true;
	return model->arrange_objects(dis, &box);
}

ModelObject* Interface::find_object(size_t id)
{
	size_t i = id / InstanceNum;
	return model->objects[i];
}

ModelInstance* Interface::find_instance(size_t id)
{
	size_t i = id / InstanceNum;//对象id
	size_t j = id % InstanceNum;//实例id
	return model->objects[i]->instances[j];
}

size_t Interface::find_id(ModelInstance* instance)
{
	for (auto o = model->objects.begin(); o != model->objects.end(); ++o) {
		int a = std::distance(model->objects.begin(), o);
		for (auto n = (*o)->instances.begin(); n != (*o)->instances.end(); ++n) {
			int b = std::distance((*o)->instances.begin(), n);
			if (*n == instance)
				return a * InstanceNum + b;
		}
	}
	return -1;
}

}
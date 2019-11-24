#ifndef slic3r_Interface_hpp_
#define slic3r_Interface_hpp_

#include"libslic3r.h"
#include"Model.hpp"
#include"DLPrint.h"
#include"qprogressbar.h"
#include"DLPrinter.h"

namespace Slic3r{

	//作者：付康
	//日期：2017
	//功能：接口，连接界面与底层数据处理（效果不理想）。
	class Interface
	{
	public:
		Interface(DLPrinter* _dlprinter);

		/*----------------------模型管理----------------------*/

		//日期：2017
		//功能：添加模型。
		//参数1：读取模型的路径
		//返回：添加模型id
		size_t load_model(std::string file,int format);

		//日期：2018.3.12
		//功能：添加模型实例
		//参数1：模型id
		ModelInstance* addInstance(size_t id);

		//日期：2017
		//功能：删除模型（删除一个模型对象，会将其所有实例全部删除）。
		//参数1：待删除模型对象的id
		void delete_model(size_t id);

		//日期：2017
		//功能：删除所有模型。
		void clear_model();

		//日期：2017
		//功能：所有模型Z轴方向移动距离。
		//参数1：移动距离值
		void model_lift(double distance);

		//日期：2017
		//功能：保存视图内的模型支撑。
		//参数1：保存路径
		//参数2：支撑模型
		void wirteStlBinary(const std::string& outFile,TriangleMesh& supportMesh);

		//日期：2018.3.7
		//功能：自动排列。
		//参数1：排列距离
		Pointfs autoArrange(double dis);

		//日期：2017
		//功能：根据id找到对应的模型实例。
		//参数1：模型id
		//返回：模型实例
		ModelInstance* find_instance(size_t id);

		//日期：2018.5.10
		//功能：根据实例返回id。
		size_t find_id(ModelInstance* instance);

		//日期：2017
		//功能：根据id找到对应的模型对象。
		//参数1：模型id
		//返回：模型实例
		ModelObject* find_object(size_t id);

		/*---------------------DLP打印------------------------*/

		//日期：2018.4.8
		//功能：生成选中模型。
		//参数1：选中模型id
		void generate_id_support(size_t id,TreeSupport*& s,QProgressBar* progress);
		
		//日期：2017
		//功能：生成所有模型的支撑。
		//void generate_all_support();

		//日期：2017
		//功能：生成所有模型的内部支撑。
		void generate_all_inside_support();

		//日期：2017
		//功能：删除所有模型的支撑。
		void delete_all_support();		

		//日期：2017
		//功能：删除所有模型的内部支撑。
		void delete_all_inside_support();

		//日期：2017
		//功能：删除选定支撑。
		//参数1：选中支撑的id
		void delete_support(size_t id);

	private:
		//日期：2017
		//功能：模型居中。
		//参数1：三角面片模型
		void model_center(TriangleMesh& mesh);
		
		DLPrinter* dlprinter;

	public:
		Model* model;										//模型的管理
		DLPrint* dlprint;									//DLP打印
	};

}

#endif
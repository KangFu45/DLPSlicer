#include "Setting.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/typeof/typeof.hpp>

using namespace boost;
using namespace boost::property_tree;

inline void ifNotExistCreate(const filesystem::path& _path)
{
	if (!exists(_path))
		create_directory(_path);
}

Setting::Setting(string _appPath)
	:appPath(_appPath)
{
	this->resourcePath = this->appPath + "/resources";
	this->xmlFile = this->resourcePath + "/AppSet.xml";
	this->modelPath = this->resourcePath + "/model";

	//TODO:配置文件不存在处理
	ptree pt;
	read_xml(this->xmlFile, pt);

	string NodePath("Config.App");
	this->appName = pt.get<string>(NodePath + ".Name", "DLPSlicer");
	this->appVersion = pt.get<string>(NodePath + ".Version", "0.0.0v");

	this->UserPath = pt.get<string>("Config.UserFile.Path", "");
	if (this->UserPath.empty() || !exists(filesystem::path(this->UserPath))) {
		//未发现用户目录
		filesystem::path _path = filesystem::temp_directory_path().append("/" + appName).string();
		if (!exists(_path)) {
			if (!create_directory(_path))//在用户临时文件夹下创建软件目录
				;///TODO:软件配置文件夹创建失败，报错退出
		}
		this->UserPath = _path.string();
		pt.put("Config.UserFile.Path", UserPath);
		write_xml(this->xmlFile, pt);//覆盖原xml文件
	}

	ZipTempPath = UserPath + "ZipTemp";
	ifNotExistCreate(ZipTempPath);

	DlprinterFile = UserPath + "Dlprinter.ini";
	ModelFile = UserPath + "ModelPath.ini";
	ConfigFile = UserPath + "Config.ini";

	//读取机器参数
	BOOST_AUTO(child, pt.get_child("Config.Machines"));
	for (BOOST_AUTO(pos, child.begin()); pos != child.end(); ++pos)  //boost中的auto
	{
		Printer temp;
		temp.name = pos->second.get<string>("Name", "");
		temp.height = pos->second.get<unsigned short>("Height", 0);
		temp.length = pos->second.get<unsigned short>("Length", 0);
		temp.width = pos->second.get<unsigned short>("Width", 0);
		temp.factor = 1.0 / ((float)temp.length / 1920.0);
		if (!temp.name.empty())
			m_printers.emplace_back(temp);
	}

	if (m_printers.empty())
		m_printers.emplace_back(Printer() = { "default",100,100,100,float(0.13) });
}

Setting::~Setting()
{
}
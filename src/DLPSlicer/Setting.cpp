#include "Setting.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

using namespace boost::filesystem;

inline void ifNotExistCreate(const path& _path)
{
	if (!exists(_path))
		create_directory(_path);
}

Setting::Setting(string _appPath)
	:appPath(_appPath)
{
	resourcePath = this->appPath + "/resources";
	xmlFile = this->resourcePath + "/AppSet.xml";
	modelPath = this->resourcePath + "/model";

	m_XmlConfig = new XmlConfig(this->xmlFile);
	string NodePath("App");
	appName = m_XmlConfig->getString(NodePath + ":Name", "DLPSlicer");
	appVersion = m_XmlConfig->getString(NodePath + ":Version", "0.0.0v");

	UserPath = m_XmlConfig->getString("Path.UserFile:path", "");
	if (UserPath.empty() || !exists(path(UserPath))) {
		//未发现用户目录
		path _path = temp_directory_path().append("/" + appName).string();
		if (!exists(_path)) {
			if (!create_directory(_path))//在用户临时文件夹下创建软件目录
				;///TODO:软件配置文件夹创建失败，报错退出
		}
		UserPath = _path.string();
		m_XmlConfig->set("Path.UserFile:path", UserPath);
		m_XmlConfig->toXmlFile(this->xmlFile);//覆盖原xml文件
	}

	ZipTempPath = UserPath + "ZipTemp";
	ifNotExistCreate(ZipTempPath);

	DlprinterFile = UserPath + "Dlprinter.ini";
	ModelFile = UserPath + "ModelPath.ini";
	ConfigFile = UserPath + "Config.ini";

	//读取机器参数
	unsigned short id = 0;
	while (1) {
		id++;
		NodePath = "Machine.Number" + to_string(id);
		string name = m_XmlConfig->getString(NodePath+":name", "");
		if (!name.empty() && NamseNoRepetition(name)) {
			Printer temp;
			temp.name = name;
			temp.height = m_XmlConfig->getInt(NodePath + ":height", 0);
			temp.length = m_XmlConfig->getInt(NodePath + ":length", 0);
			temp.width = m_XmlConfig->getInt(NodePath + ":width", 0);
			temp.factor = 1.0 / ((float)temp.length / 1920.0);
			m_printers.emplace_back(temp);
		}
		else
			break;
	}

	if (m_printers.empty())
		m_printers.emplace_back(Printer() = { "default",100,100,100,0.13 });
}

Setting::~Setting()
{
	delete m_XmlConfig;
}
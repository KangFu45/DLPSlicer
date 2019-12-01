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

Setting::Setting(string filename)
	:m_XmlConfig(new XmlConfig(filename))
{
	string NodePath("App");
	appName = m_XmlConfig->getString(NodePath + ":Name", "DLPSlicer");
	appVersion = m_XmlConfig->getString(NodePath + ":Version", "0.0.0v");

	UserPath = m_XmlConfig->getString("Path.UserFile:path", "");
	if (UserPath.empty() || !exists(path(UserPath))) {
		//δ�����û�Ŀ¼
		path _path = temp_directory_path().append("/" + appName).string();
		if (!exists(_path)) {
			if (!create_directory(_path))//���û���ʱ�ļ����´�������Ŀ¼
				;///TODO:���������ļ��д���ʧ�ܣ������˳�
		}
		UserPath = _path.string();
		m_XmlConfig->set("Path.UserFile:path", UserPath);
		m_XmlConfig->toXmlFile(filename);//����ԭxml�ļ�
	}

	ZipTempPath = UserPath + "ZipTemp";
	ifNotExistCreate(ZipTempPath);

	DlprinterFile = UserPath + "Dlprinter.ini";
	ModelFile = UserPath + "ModelPath.ini";
	ConfigFile = UserPath + "Config.ini";
}

Setting::~Setting()
{
	delete m_XmlConfig;
}
#pragma once
#include "XmlConfig/XmlConfig.h"

class Setting
{
public:
	Setting() {};
	Setting(string filename);
	~Setting();

	//App
	string appName;
	string appVersion;

	string UserPath;
	string ModelFile;
	string ZipTempPath;
	string DlprinterFile;
	string ConfigFile;

private:
	XmlConfig* m_XmlConfig;
};

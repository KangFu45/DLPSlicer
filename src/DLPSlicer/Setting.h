#pragma once
#include <string>
#include <vector>

using namespace std;

class Setting
{
public:
	Setting() {};
	Setting(string _appPath);
	~Setting();
	
	string appPath;
	string resourcePath;
	string xmlFile;
	string modelPath;

	string appName;
	string appVersion;

	string UserPath;
	string ModelFile;
	string ZipTempPath;
	string DlprinterFile;
	string ConfigFile;

	struct Printer
	{
		string name;
		unsigned short length;
		unsigned short width;
		unsigned short height;
		float factor;
	};
	typedef vector<Printer> Printers;
	//设定第一个机器为选中的机器
	Printers m_printers;
	bool setSelMachine(string name) {
		for (auto per = m_printers.begin(); per != m_printers.end(); ++per) {
			if (per->name == name) {
				Printer temp = *per;
				m_printers.erase(per);
				if (m_printers.empty()) {
					m_printers.emplace_back(temp);
					return true;
				}
				else {
					m_printers.insert(m_printers.begin(), temp);
					return true;
				}
			}
		}
		return false;
	}

private:
	bool NamseNoRepetition(string name) {
		for each (Printer per in m_printers) {
			if (per.name == name)
				return false;
		}
		return true;
	}
};

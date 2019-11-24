#pragma once
#include "qstring.h"

enum Printer
{
	S288,
	S250
};

//功能：管理打印机
//作者：付康
//日期：2018.10
class DLPrinter
{
public:
	DLPrinter(Printer _printer);
	~DLPrinter();

	Printer printer;

	unsigned int length;
	unsigned int width;
	unsigned int height;

	//功能：读取打印机
	void readPrinter();
	
private:
	//功能：读取指定打印机设置
	bool readPrinterIni(QString path);

	//功能：添加默认打印机
	void addDefultPrinter();
};


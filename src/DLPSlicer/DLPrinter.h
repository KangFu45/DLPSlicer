#pragma once
#include "qstring.h"

enum Printer
{
	S288,
	S250
};

//���ܣ������ӡ��
//���ߣ�����
//���ڣ�2018.10
class DLPrinter
{
public:
	DLPrinter(Printer _printer);
	~DLPrinter();

	Printer printer;

	unsigned int length;
	unsigned int width;
	unsigned int height;

	//���ܣ���ȡ��ӡ��
	void readPrinter();
	
private:
	//���ܣ���ȡָ����ӡ������
	bool readPrinterIni(QString path);

	//���ܣ����Ĭ�ϴ�ӡ��
	void addDefultPrinter();
};


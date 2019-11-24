#include "DLPrinter.h"
#include "qsettings.h"
#include "qdebug.h"
#include "qdir.h"



DLPrinter::DLPrinter(Printer _printer) 
	: printer(_printer)
{
}


DLPrinter::~DLPrinter()
{
}

bool DLPrinter::readPrinterIni(QString path)
{
	QFile dir(path);
	if (dir.exists()) {
		QSettings* readini = new QSettings(path, QSettings::IniFormat);
		QString _lenght = readini->value("/3D/length").toString();
		if (_lenght != "")
			this->length = _lenght.toInt();
		else
			return false;

		QString _width = readini->value("/3D/width").toString();
		if (_width != "")
			this->width = _width.toInt();
		else
			return false;

		QString _height = readini->value("/3D/height").toString();
		if (_height != "")
			this->height = _height.toInt();
		else
			return false;

		delete readini;
		return true;
	}
	else
		return false;
}

void DLPrinter::readPrinter()
{
	if (printer == S288)
	{
		QString path = QDir::currentPath();
		path.append("/dlprinter/S288.ini");
		if (!readPrinterIni(path))
			addDefultPrinter();
	}
	else if (printer == S250)
	{
		QString path = QDir::currentPath();
		path.append("/dlprinter/S250.ini");
		if(!readPrinterIni(path))
			addDefultPrinter();
	}
	else
		addDefultPrinter();
}

void DLPrinter::addDefultPrinter()
{
}

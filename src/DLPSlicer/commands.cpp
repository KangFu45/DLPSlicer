#include "commands.h"

AddModelCommand::AddModelCommand(size_t _id, MainWindow *parent) : id(_id),_mainWindow(parent)
{

}

void AddModelCommand::undo()
{
	_mainWindow->deleteModelBuffer(this->id);
	//qDebug() << "add undo";
}

void AddModelCommand::redo()
{
	_mainWindow->addModelBuffer(this->id);
	//qDebug() << "add redo";
}

DeleteModelCommand::DeleteModelCommand(size_t _id, MainWindow *parent) : id(_id), _mainWindow(parent)
{

}

void DeleteModelCommand::undo()
{
	_mainWindow->addModelBuffer(this->id);
}

void DeleteModelCommand::redo()
{
	_mainWindow->deleteModelBuffer(this->id);
}

offsetValueChangeCommand::offsetValueChangeCommand(size_t _id, double _x, double _y, double _z, MainWindow* parent)
	:m_id(_id), x(_x), y(_y), z(_z), _mainWindow(parent)
{

}

void offsetValueChangeCommand::undo()
{
	_mainWindow->offsetValueChange(m_id,-x, -y, -z);
	//qDebug() << "offset undo";
}

void offsetValueChangeCommand::redo()
{
	_mainWindow->offsetValueChange(m_id, x, y, z);
	//qDebug() << "offset redo";
}

bool offsetValueChangeCommand::mergeWith(const QUndoCommand* command)
{
	const offsetValueChangeCommand* other = static_cast<const offsetValueChangeCommand*>(command);
	if (offsetValueCommandId != other->id() )
		return false;

	if (other->m_id != m_id)
		return false;

	x += other->x;
	y += other->y;
	z += other->z;
	return true;
}

int offsetValueChangeCommand::id() const
{
	return offsetValueCommandId;
}

scaleValueChangeCommand::scaleValueChangeCommand(size_t _id, double _x, double _y, double _z, MainWindow* parent)
	:m_id(_id), x(_x), y(_y), z(_z), _mainWindow(parent)
{

}

void scaleValueChangeCommand::undo()
{
	_mainWindow->scaleValueChange(m_id, -x, -y, -z);
	//qDebug() << "scale undo";
}

void scaleValueChangeCommand::redo()
{
	_mainWindow->scaleValueChange(m_id, x, y, z);
	//qDebug() << "scale redo";
}

bool scaleValueChangeCommand::mergeWith(const QUndoCommand* command)
{
	const scaleValueChangeCommand* other = static_cast<const scaleValueChangeCommand*>(command);
	if (scaleValueCommandId != other->id())
		return false;

	if (other->m_id != m_id)
		return false;

	x += other->x;
	y += other->y;
	z += other->z;
	return true;
}

int scaleValueChangeCommand::id() const
{
	return scaleValueCommandId;
}

rotateValueChangeCommand::rotateValueChangeCommand(size_t _id, double _angle,int _x, int _y, int _z, MainWindow* parent)
	:m_id(_id),angle(_angle), x(_x), y(_y), z(_z), _mainWindow(parent)
{

}

void rotateValueChangeCommand::undo()
{
	_mainWindow->rotateValueChange(m_id, -angle, x, y, z);
	//qDebug() << "rotate undo";
}

void rotateValueChangeCommand::redo()
{
	_mainWindow->rotateValueChange(m_id, angle, x, y, z);
	//qDebug() << "rotate redo";
}

bool rotateValueChangeCommand::mergeWith(const QUndoCommand* command)
{
	const rotateValueChangeCommand* other = static_cast<const rotateValueChangeCommand*>(command);
	if (rotateValueCommandId != other->id())
		return false;

	if (other->m_id != m_id)
		return false;

	if (x == other->x&&y == other->y&&z == other->z)
		angle += other->angle;
	else
		return false;

	//x += other->x;
	//y += other->y;
	//z += other->z;
	return true;
}

int rotateValueChangeCommand::id() const
{
	return rotateValueCommandId;
}

addSupportsCommand::addSupportsCommand(size_t _id, TreeSupport* _treeSupport, MainWindow* parent, QProgressBar* _progress)
	:m_id(_id), treeSupport(_treeSupport), _mainWindow(parent), progress(_progress)
{

}

addSupportsCommand::~addSupportsCommand()
{
	if (treeSupport)
		delete treeSupport;
}

void addSupportsCommand::undo()
{
	_mainWindow->deleteSupports(m_id);
	//qDebug() << "deleteSupport undo";
}

void addSupportsCommand::redo()
{
	_mainWindow->addSupports(m_id, treeSupport, progress);
	//qDebug() << "addSupport redo";
}

int addSupportsCommand::id() const
{
	return addSupportsCommandId;
}


deleteSupportsCommand::deleteSupportsCommand(size_t _id, TreeSupport* _treeSupport, MainWindow* parent)
	:m_id(_id),treeSupport(_treeSupport),_mainWindow(parent)
{

}

deleteSupportsCommand::~deleteSupportsCommand()
{
	//delete treeSupport;ÓÉaddSupportsCommandÎö¹¹É¾³ý
}

void deleteSupportsCommand::undo()
{
	_mainWindow->addSupports(m_id, treeSupport);
	//qDebug() << "addSupport redo";
}

void deleteSupportsCommand::redo()
{
	_mainWindow->deleteSupports(m_id);
	//qDebug() << "deleteSupport undo";
}

addOneSupportCommand::addOneSupportCommand(size_t _id, MainWindow* parent)
	:s_id(_id),_mainWindow(parent)
{

}

void addOneSupportCommand::undo()
{
	_mainWindow->deleteOneSupportUndo(s_id);
}

void addOneSupportCommand::redo()
{
	_mainWindow->addOneSupportUndo(s_id);
}

deleteOneSupportCommand::deleteOneSupportCommand(size_t _id, MainWindow* parent)
	:s_id(_id),_mainWindow(parent)
{

}

void deleteOneSupportCommand::undo()
{
	_mainWindow->addOneSupportUndo(s_id);
}

void deleteOneSupportCommand::redo()
{
	_mainWindow->deleteOneSupportUndo(s_id);
}
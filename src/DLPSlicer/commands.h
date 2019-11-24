#pragma once

#include <QUndoCommand>
#include "mainwindow.h"
#include "qprogressbar.h"

class MainWindow;

static const int offsetValueCommandId = 1;
static const int scaleValueCommandId = 2;
static const int rotateValueCommandId = 3;
static const int addSupportsCommandId = 4;


class AddModelCommand : public QUndoCommand
{
public:
	AddModelCommand(size_t _id,MainWindow *parent);
	void undo() override;
	void redo() override;

private:
	size_t id;					//模型id
	MainWindow* _mainWindow;
};

class DeleteModelCommand : public QUndoCommand
{
public:
	DeleteModelCommand(size_t _id, MainWindow *parent);
	void undo() override;
	void redo() override;

private:
	size_t id;					//模型id
	MainWindow* _mainWindow;
};

class offsetValueChangeCommand : public QUndoCommand
{
public:
	offsetValueChangeCommand(size_t _id, double _x, double _y, double _z, MainWindow* parent);
	void undo() override;
	void redo() override;
	bool mergeWith(const QUndoCommand* command) override;
	int id() const override; 

private:
	size_t m_id;
	MainWindow* _mainWindow;
	double x;
	double y;
	double z;
};

class scaleValueChangeCommand : public QUndoCommand
{
public:
	scaleValueChangeCommand(size_t _id, double _x, double _y, double _z, MainWindow* parent);
	void undo() override;
	void redo() override;
	bool mergeWith(const QUndoCommand* command) override;
	int id() const override;

private:
	size_t m_id;
	MainWindow* _mainWindow;
	double x;
	double y;
	double z;
};

class rotateValueChangeCommand : public QUndoCommand
{
public:
	rotateValueChangeCommand(size_t _id, double _angle, int _x, int _y, int _z, MainWindow* parent);
	void undo() override;
	void redo() override;
	bool mergeWith(const QUndoCommand* command) override;
	int id() const override;

private:
	size_t m_id;
	MainWindow* _mainWindow;
	double angle;
	int x;
	int y;
	int z;
};

class addSupportsCommand : public QUndoCommand
{
public:
	addSupportsCommand(size_t _id, TreeSupport* _treeSupport, MainWindow* parent,QProgressBar* _progress);
	~addSupportsCommand();
	void undo() override;
	void redo() override;
	int id() const override;

private:
	size_t m_id;
	MainWindow* _mainWindow;
	TreeSupport* treeSupport;
	QProgressBar* progress;
};

class deleteSupportsCommand : public QUndoCommand
{
public:
	deleteSupportsCommand(size_t _id, TreeSupport* _treeSupport, MainWindow* parent);
	~deleteSupportsCommand();
	void undo() override;
	void redo() override;

private:
	size_t m_id;
	MainWindow* _mainWindow;
	TreeSupport* treeSupport;
};

class addOneSupportCommand : public QUndoCommand
{
public:
	addOneSupportCommand(size_t _id, MainWindow* parent);
	void undo() override;
	void redo() override;

private:
	size_t s_id;
	MainWindow* _mainWindow;
};

class deleteOneSupportCommand : public QUndoCommand
{
public:
	deleteOneSupportCommand(size_t _id, MainWindow* parent);
	void undo() override;
	void redo() override;

private:
	size_t s_id;
	MainWindow* _mainWindow;
};




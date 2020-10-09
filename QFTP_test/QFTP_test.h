#pragma once

#include <QtWidgets/QWidget>
#include "ui_QFTP_test.h"

class QFTP_test : public QWidget
{
	Q_OBJECT

public:
	QFTP_test(QWidget *parent = Q_NULLPTR);

private:
	Ui::QFTP_testClass ui;
};

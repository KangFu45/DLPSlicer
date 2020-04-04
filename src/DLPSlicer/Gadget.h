#pragma once

#include <qpushbutton.h>

//可以被点击按钮的QSS
static QString OnButton("border: 1px outset white; border-radius: 5px;  background-color: rgba(100,225,50,225);");
//不能被点击按钮的QSS
static QString OffButton("border: 1px outset white; border-radius: 5px; background-color: rgba(200,200,200,150);");

//自定义的按钮
class PushButton : public QPushButton
{
	//Q_OBJECT
public:
	explicit PushButton(QWidget* parent = nullptr) : QPushButton(parent) {};
	explicit PushButton(const QString& text, QWidget* parent = nullptr) : QPushButton(text, parent) {};
	PushButton(const QIcon& icon, const QString& text, QWidget* parent = nullptr) : QPushButton(icon, text, parent) {};
	~PushButton() {};

	void push() {
		if (this->checked)
			setStyleSheet(OnButton);
		else
			setStyleSheet(OffButton);
		this->checked = !this->checked;
	};
	bool c_isChecked() { return this->checked; };

private:
	//给按钮增加是否按下的属性
	//需控制其是否按下或弹起的操作
	bool checked = { false };
};
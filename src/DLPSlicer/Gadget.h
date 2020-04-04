#pragma once

#include <qpushbutton.h>

//���Ա������ť��QSS
static QString OnButton("border: 1px outset white; border-radius: 5px;  background-color: rgba(100,225,50,225);");
//���ܱ������ť��QSS
static QString OffButton("border: 1px outset white; border-radius: 5px; background-color: rgba(200,200,200,150);");

//�Զ���İ�ť
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
	//����ť�����Ƿ��µ�����
	//��������Ƿ��»���Ĳ���
	bool checked = { false };
};
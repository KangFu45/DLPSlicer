#pragma once

#include <qdialog.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qgridlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>

class MainWindow;

class SetupDialog : public QDialog
{
	Q_OBJECT
public:
	SetupDialog(MainWindow* _mainwindow);
	~SetupDialog();

	void initLayout();

	//��������
	QSpinBox* normIlluSpin;                       //�����ع�ʱ��
	QLabel* normIlluLabel;
	QSpinBox* norm_inttersity_spin;               //�����ع�ǿ��
	QLabel* norm_inttersity_label;
	QSpinBox* overIlluSpin;                       //���ع�ʱ��
	QLabel* overIlluLabel;
	QSpinBox* over_inttersity_spin;               //���ع�ǿ��
	QLabel* over_inttersity_label;
	QSpinBox* overLayerSpin;                      //���ع����
	QLabel* overLayerLabel;

	QGridLayout* illuLayout;
	QWidget* illuWidget;

	//֧������
	QDoubleSpinBox* top_height_spin;			  //���˸߶�
	QLabel* top_height_label;
	QDoubleSpinBox* top_radius_spin;              //���˰뾶
	QLabel* top_radius_label;
	QDoubleSpinBox* support_radius_spin;          //�м�뾶
	QLabel* support_radius_label;
	QDoubleSpinBox* bottom_radius_spin;			  //�ײ��뾶
	QLabel* bottom_radius_label;
	QSpinBox* support_space_spin;                 //֧�ż��
	QLabel* support_space_label;
	QSpinBox* support_angle_spin;                 //֧�ŽǶ�
	QLabel* support_angle_label;
	QSpinBox* leaf_num_spin;					  //��֦����
	QLabel* leaf_num_label;
	QDoubleSpinBox* model_lift_spin;			  //ģ������
	QLabel* model_lift_label;

	QGridLayout* supportLayout;
	QWidget* supportWidget;

	//�������
	QCheckBox* hollow_out_box;                    //�Ƿ���
	QLabel* hollow_out_label;
	QComboBox* fill_pattern_combo;				 //���ͼ��
	QLabel* fillPatternLabel;
	QDoubleSpinBox* wall_thickness_spin;          //�ں�
	QLabel* wall_thickness_label;
	QSpinBox* density_spin;						  //�ڲ�֧���ܶ�
	QLabel* density_label;

	QGridLayout* hollowOutLayout;
	QWidget* hollowOutWidget;

	//��������
	QComboBox* thicknessCombo;                    //���
	QLabel* thicknessLabel;
	QSpinBox* raftSpin;                           //�׷�����
	QLabel* raftLabel;
	QSpinBox* raftOffsetSpin;					  //�װ岹��
	QLabel* raftOffsetLabel;
	QDoubleSpinBox* arrange_space_spin;			  //�Զ����м��
	QLabel* arrangeSpaceLabel;
	QSpinBox* threadSpin;						  //�߳���
	QLabel* threadLabel;

	QGridLayout* otherLayourt;
	QWidget* otherWidget;

	QTabWidget* setupTab;

	QPushButton* defultBtn;
	QPushButton* cancalBtn;
	QPushButton* okBtn;
	QGridLayout* mainLayout;

private slots:
	//���ڣ�2017
	//���ܣ�д���á�
	void writeConfig();

	//���ڣ�2017
	//���ܣ��ָ�Ĭ�ϡ�
	void recoverDefult();

	//���ڣ�2017
	//���ܣ��رնԻ��������
	void BtnChange();
private:
	//���ڣ�2017
	//���ܣ�����Ĭ��ֵ��
	void setDefultValue();

	//���ڣ�2017
	//���ܣ���ȡ���á�
	void readConfig();

	MainWindow* mainwindow;						  //������
};
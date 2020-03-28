#pragma once

#include <qdialog.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qgridlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>

struct Config
{
	float	fill_angle;
	float	fill_density;

	enum InfillPattern
	{
		ipHoneycomb = 0,
		ip3DSupport
	}fill_pattern;

	float	layer_height;
	int		raft_layers;
	int		raft_offset;

	float	support_top_height;
	float	support_radius;
	float	support_top_radius;
	float	support_bottom_radius;
	int		space;
	int		angle;
	int		leaf_num;
	float   model_lift;

	int		overLayer;
	int		overIlluTime;
	int		over_inttersity;
	int		norm_inttersity;
	int		normIlluTime;
	int		firstIllu;

	float	wall_thickness;
	bool	hollow_out;
	float	arrange_space;

	int		threads;

	void writeConfig();
};

class SetupDialog : public QDialog
{
	Q_OBJECT
public:
	SetupDialog(Config* config);
	~SetupDialog();

	void initLayout();

	Config* m_config;

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

public slots:
	void slot_writeConfig();
	void slot_setDefultValue();

private:
	void readConfig();
};
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

	//光照设置
	QSpinBox* normIlluSpin;                       //正常曝光时间
	QLabel* normIlluLabel;
	QSpinBox* norm_inttersity_spin;               //正常曝光强度
	QLabel* norm_inttersity_label;
	QSpinBox* overIlluSpin;                       //长曝光时间
	QLabel* overIlluLabel;
	QSpinBox* over_inttersity_spin;               //长曝光强度
	QLabel* over_inttersity_label;
	QSpinBox* overLayerSpin;                      //长曝光层数
	QLabel* overLayerLabel;

	QGridLayout* illuLayout;
	QWidget* illuWidget;

	//支撑设置
	QDoubleSpinBox* top_height_spin;			  //顶端高度
	QLabel* top_height_label;
	QDoubleSpinBox* top_radius_spin;              //顶端半径
	QLabel* top_radius_label;
	QDoubleSpinBox* support_radius_spin;          //中间半径
	QLabel* support_radius_label;
	QDoubleSpinBox* bottom_radius_spin;			  //底部半径
	QLabel* bottom_radius_label;
	QSpinBox* support_space_spin;                 //支撑间距
	QLabel* support_space_label;
	QSpinBox* support_angle_spin;                 //支撑角度
	QLabel* support_angle_label;
	QSpinBox* leaf_num_spin;					  //树枝长度
	QLabel* leaf_num_label;
	QDoubleSpinBox* model_lift_spin;			  //模型提升
	QLabel* model_lift_label;

	QGridLayout* supportLayout;
	QWidget* supportWidget;

	//抽空设置
	QCheckBox* hollow_out_box;                    //是否抽空
	QLabel* hollow_out_label;
	QComboBox* fill_pattern_combo;				 //填充图案
	QLabel* fillPatternLabel;
	QDoubleSpinBox* wall_thickness_spin;          //壁厚
	QLabel* wall_thickness_label;
	QSpinBox* density_spin;						  //内部支撑密度
	QLabel* density_label;

	QGridLayout* hollowOutLayout;
	QWidget* hollowOutWidget;

	//其他设置
	QComboBox* thicknessCombo;                    //层厚
	QLabel* thicknessLabel;
	QSpinBox* raftSpin;                           //底筏层数
	QLabel* raftLabel;
	QSpinBox* raftOffsetSpin;					  //底板补偿
	QLabel* raftOffsetLabel;
	QDoubleSpinBox* arrange_space_spin;			  //自动排列间距
	QLabel* arrangeSpaceLabel;
	QSpinBox* threadSpin;						  //线程数
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
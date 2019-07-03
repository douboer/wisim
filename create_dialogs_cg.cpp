/******************************************************************************************/
/**** PROGRAM: create_dialogs_cg.cpp                                                   ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#include "WiSim_gui.h"
#include "create_subnet_dia.h"
#include "subnet_traffic_dia.h"
#include "report_dia.h"
#include "pref_dia.h"
#include "prop_mod_mgr_dia.h"
#include "category_wid.h"
#include "rtd_threshold_dialog.h"
#include "image_registration.h"
// xxxxxxx #include "read_map_background_dialog.h"

#include "import_st_data_dialog.h"

#if HAS_MONTE_CARLO
#include "run_simulation_dia.h"
#endif

/******************************************************************************************/
/**** FUNCTION: FigureEditor:: Dialog create functions                                 ****/
/******************************************************************************************/

#if HAS_MONTE_CARLO
void FigureEditor::create_run_simulation_dialog()
{
    new RunSimulation(np, this);
}

void FigureEditor::create_import_st_data_dialog()
{
    new importStDataDia(np, this, 0);
}
#else
void FigureEditor::create_run_simulation_dialog()
{

}
void FigureEditor::create_import_st_data_dialog()
{

}
#endif

#if 0
    xxxxxxxxx 
void FigureEditor::create_read_map_background_dialog()
{
    new ReadMapBackgroundDialog(this, 0);
}
#endif


void FigureEditor::create_image_registration_dialog(QString imagefile)
{
    new ImageRegistration(imagefile, this, 0);
}

void FigureEditor::create_create_subnet_dialog()
{
    new CreSubnetDia(np, this);
}

void FigureEditor::create_subnet_traffic_dialog()
{
    new SubnetTraffic(np, this);
}

void FigureEditor::create_report_dialog(int type)
{
    new ReportDialog(np, type, this);
}

void FigureEditor::create_pref_dialog()
{
    new PrefDia(this, this);
}

void FigureEditor::create_prop_mgr_dialog()
{
    new PropModMgrDia(np, this);
}

void FigureEditor::create_set_param_dialog()
{
    new CategoryWid(np, this);
}

void FigureEditor::create_set_rtd_threshold_dialog()
{
    new RTDThresholdDialog(np, this);
}

/******************************************************************************************/

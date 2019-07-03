/******************************************************************************************/
/**** PROGRAM: create_dialogs_lt.cpp                                                     ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#include "WiSim_gui.h"
#include "pref.h"
#include "set_shape_dialog.h"
#include "convert_road_test_data_dialog.h"
#include "cvg_maindia.h"
#include "reg_dia.h"
#include "prop_analysis_dia.h"

/******************************************************************************************/
/**** FUNCTION: FigureEditor:: Dialog create functions                                 ****/
/******************************************************************************************/
void FigureEditor::create_set_group_shape_dialog()        { new SetGroupShapeDialog(this, this); }
void FigureEditor::create_convert_road_test_data_dialog() { new ConvertRoadTestDataDialog(this); }
void FigureEditor::create_cvg_mgr_dialog()                { new CvgAnaDia(this, this); }
void FigureEditor::create_prop_analysis_dialog()          { new PropAnalysisDia(np, 0); }
/******************************************************************************************/

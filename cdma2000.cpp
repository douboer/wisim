/******************************************************************************************/
/**** FILE: cdma2000.cpp                                                               ****/
/******************************************************************************************/

#include <math.h>

#include "cdma2000.h"
#include "cconst.h"

#if HAS_GUI
#include "gconst.h"
#endif

/******************************************************************************************/
/**** Get / Set functions                                                              ****/
/******************************************************************************************/
const int CDMA2000NetworkClass::technology() { return(CConst::CDMA2000); }
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: CDMA2000SectorClass::CDMA2000SectorClass                               ****/
/******************************************************************************************/
CDMA2000SectorClass::CDMA2000SectorClass(CellClass *cell) : SectorClass(cell)
{
    set_default_parameters();
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: CDMA2000SectorClass::~CDMA2000SectorClass                              ****/
/******************************************************************************************/
CDMA2000SectorClass::~CDMA2000SectorClass()
{
    return;
}
/******************************************************************************************/
/**** FUNCTION: CDMA2000SectorClass::set_default_parameters                            ****/
/******************************************************************************************/
void CDMA2000SectorClass::set_default_parameters()
{
    SectorClass::set_default_parameters();

}

/******************************************************************************************/
/**** FUNCTION: CDMA2000SectorClass::duplicate                                         ****/
/******************************************************************************************/
SectorClass *CDMA2000SectorClass::duplicate(int copy_csid)
{
    CDMA2000SectorClass *new_sector = new CDMA2000SectorClass((CellClass *) NULL);

    copy_sector_values((SectorClass *) new_sector);

    return((SectorClass *) new_sector);
}
/******************************************************************************************/
/**** FUNCTION: CDMA2000CellClass::CDMA2000CellClass                                   ****/
/******************************************************************************************/
CDMA2000CellClass::CDMA2000CellClass(int p_num_sector) : CellClass()
{
    int sector_idx;
    num_sector = p_num_sector;
    sector_list = (SectorClass **) malloc((num_sector)*sizeof(SectorClass *));
    for (sector_idx=0; sector_idx<=num_sector-1; sector_idx++) {
        sector_list[sector_idx] = (SectorClass *) new CDMA2000SectorClass(this);
    }
}
/******************************************************************************************/
/**** FUNCTION: WLANCellClass::view_name                                               ****/
/******************************************************************************************/
char *CDMA2000CellClass::view_name(int cell_idx, int cell_name_pref)
{
    static char str[50];

    switch(cell_name_pref) {
        case CConst::CellIdxRef:
            sprintf(str, "cell_%d", cell_idx);
            break;
#if 0
        case CConst::CellCSNumberRef:
            sprintf(str, "%.6d", ((WLANSectorClass *) sector_list[0])->gw_csc_cs);
            break;
        case GConst::CellHexCSIDRef:
            if (((WLANSectorClass *) sector_list[0])->csid_hex) {
                hex_to_hexstr(str, ((WLANSectorClass *) sector_list[0])->csid_hex, WLANSectorClass::csid_byte_length);
            } else {
                sprintf(str, "**UNASSIGNED**");
            }
            break;
#endif
        default:
            CORE_DUMP; break;
    }

    return(str);
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: CDMA2000CellClass::~CDMA2000CellClass                                  ****/
/******************************************************************************************/
CDMA2000CellClass::~CDMA2000CellClass()
{
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: CDMA2000CellClass::duplicate                                           ****/
/******************************************************************************************/
CellClass *CDMA2000CellClass::duplicate(const int x, const int y, int copy_csid)
{
    CellClass *new_cell = (CellClass *) new CDMA2000CellClass();

    copy_cell_values(new_cell, x, y, copy_csid);

    return(new_cell);
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: CDMA2000NetworkClass::CDMA2000NetworkClass                             ****/
/******************************************************************************************/
CDMA2000NetworkClass::CDMA2000NetworkClass() : NetworkClass()
{
    set_default_parameters();
}
/******************************************************************************************/
/**** FUNCTION: CDMA2000NetworkClass::~CDMA2000NetworkClass                            ****/
/******************************************************************************************/
CDMA2000NetworkClass::~CDMA2000NetworkClass()
{
    close_geometry();
}
/******************************************************************************************/
/**** FUNCTION: CDMA2000NetworkClass::set_default_parameters                           ****/
/******************************************************************************************/
void CDMA2000NetworkClass::set_default_parameters()
{
    NetworkClass::set_default_parameters();

}
/******************************************************************************************/
/**** FUNCTION: display_setting                                                        ****/
/**** Display parameter settings                                                       ****/
/******************************************************************************************/
void CDMA2000NetworkClass::display_settings(FILE *fp)
{
    char *chptr;

    NetworkClass::display_settings(fp);

    chptr = msg;
    chptr += sprintf(chptr, "\n");
    chptr += sprintf(chptr, "CDMA2000 PARAMETER SETTINGS:\n");
    PRMSG(fp, msg);

    return;
}
/******************************************************************************************/
/**** FUNCTION: CDMA2000NetworkClass::process_mode_change                                  ****/
/******************************************************************************************/
void CDMA2000NetworkClass::process_mode_change()
{
    // crash CG -190629
    //NetworkClass::process_mode_change();
}
/******************************************************************************************/
/**** FUNCTION: check_parameters                                                       ****/
/******************************************************************************************/
int CDMA2000NetworkClass::check_parameters()
{
    int cell_idx, sector_idx, unused_freq_idx;
    int num_error;
    CellClass *cell;
    CDMA2000SectorClass *sector;

    num_error = 0;

    if (num_error) {
        error_state = 1;
    }

    return(num_error);
}
/******************************************************************************************/
/**** FUNCTION: CDMA2000TrafficTypeClass::CDMA2000TrafficTypeClass                     ****/
/******************************************************************************************/
CDMA2000TrafficTypeClass::CDMA2000TrafficTypeClass(char *p_strid) : TrafficTypeClass(p_strid)
{
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: CDMA2000SectorClass::~CDMA2000SectorClass                              ****/
/******************************************************************************************/
CDMA2000TrafficTypeClass::~CDMA2000TrafficTypeClass()
{
}
/******************************************************************************************/


/******************************************************************************************/
/**** FILE: wcdma.cpp                                                                  ****/
/**** Michael Mandell 7/15/04                                                          ****/
/******************************************************************************************/

#include <math.h>

#include "wcdma.h"
#include "cconst.h"

#if HAS_GUI
#include "gconst.h"
#endif

/******************************************************************************************/
/**** Get / Set functions                                                              ****/
/******************************************************************************************/
const int WCDMANetworkClass::technology() { return(CConst::WCDMA); }
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: WCDMASectorClass::WCDMASectorClass                                     ****/
/******************************************************************************************/
WCDMASectorClass::WCDMASectorClass(CellClass *cell) : SectorClass(cell)
{
    set_default_parameters();
}
/******************************************************************************************/

#if HAS_GUI
/******************************************************************************************/
/**** FUNCTION: WCDMACellClass::name                                                   ****/
/******************************************************************************************/
char *WCDMACellClass::name(int cell_idx, int cell_name_pref)
{
    static char str[50];

    switch(cell_name_pref) {
        case CConst::CellIdxRef:
            sprintf(str, "cell_%d", cell_idx);
            break;
#if 0
        case CConst::CellCSNumberRef:
            sprintf(str, "%.6d", ((PHSSectorClass *) sector_list[0])->gw_csc_cs);
            break;
        case GConst::CellHexCSIDRef:
            if (((PHSSectorClass *) sector_list[0])->csid_hex) {
                hex_to_hexstr(str, ((PHSSectorClass *) sector_list[0])->csid_hex, PHSSectorClass::csid_byte_length);
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
#endif

/******************************************************************************************/
/**** FUNCTION: WCDMASectorClass::~WCDMASectorClass                                    ****/
/******************************************************************************************/
WCDMASectorClass::~WCDMASectorClass()
{
    return;
}
/******************************************************************************************/
/**** FUNCTION: WCDMASectorClass::set_default_parameters                               ****/
/******************************************************************************************/
void WCDMASectorClass::set_default_parameters()
{
    SectorClass::set_default_parameters();

}
/******************************************************************************************/
/**** FUNCTION: WCDMASectorClass::duplicate                                            ****/
/******************************************************************************************/
SectorClass *WCDMASectorClass::duplicate(int copy_csid)
{
    WCDMASectorClass *new_sector = new WCDMASectorClass((CellClass *) NULL);

    copy_sector_values((SectorClass *) new_sector);

    return((SectorClass *) new_sector);
}
/******************************************************************************************/
/**** FUNCTION: WCDMACellClass::WCDMACellClass                                         ****/
/******************************************************************************************/
WCDMACellClass::WCDMACellClass(int p_num_sector) : CellClass()
{
    int sector_idx;
    num_sector = p_num_sector;
    sector_list = (SectorClass **) malloc((num_sector)*sizeof(SectorClass *));
    for (sector_idx=0; sector_idx<=num_sector-1; sector_idx++) {
        sector_list[sector_idx] = (SectorClass *) new WCDMASectorClass(this);
    }
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: WCDMACellClass::~WCDMACellClass                                        ****/
/******************************************************************************************/
WCDMACellClass::~WCDMACellClass()
{
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: WCDMACellClass::duplicate                                              ****/
/******************************************************************************************/
CellClass *WCDMACellClass::duplicate(const int x, const int y, int copy_csid)
{
    CellClass *new_cell = (CellClass *) new WCDMACellClass();

    copy_cell_values(new_cell, x, y, copy_csid);

    return(new_cell);
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: WCDMANetworkClass::WCDMANetworkClass                                   ****/
/******************************************************************************************/
WCDMANetworkClass::WCDMANetworkClass() : NetworkClass()
{
    set_default_parameters();
}
/******************************************************************************************/
/**** FUNCTION: WCDMANetworkClass::~WCDMANetworkClass                                  ****/
/******************************************************************************************/
WCDMANetworkClass::~WCDMANetworkClass()
{
    close_geometry();
}
/******************************************************************************************/
/**** FUNCTION: WCDMANetworkClass::set_default_parameters                              ****/
/******************************************************************************************/
void WCDMANetworkClass::set_default_parameters()
{
    NetworkClass::set_default_parameters();

}
/******************************************************************************************/
/**** FUNCTION: display_setting                                                        ****/
/**** Display parameter settings                                                       ****/
/******************************************************************************************/
void WCDMANetworkClass::display_settings(FILE *fp)
{
    char *chptr;

    NetworkClass::display_settings(fp);

    chptr = msg;
    chptr += sprintf(chptr, "\n");
    chptr += sprintf(chptr, "WCDMA PARAMETER SETTINGS:\n");
    PRMSG(fp, msg);

    return;
}
/******************************************************************************************/
/**** FUNCTION: check_parameters                                                       ****/
/******************************************************************************************/
int WCDMANetworkClass::check_parameters()
{
    int cell_idx, sector_idx, unused_freq_idx;
    int num_error;
    CellClass *cell;
    WCDMASectorClass *sector;

    num_error = 0;

    if (num_error) {
        error_state = 1;
    }

    return(num_error);
}
/******************************************************************************************/
/**** FUNCTION: WCDMATrafficTypeClass::WCDMATrafficTypeClass                           ****/
/******************************************************************************************/
WCDMATrafficTypeClass::WCDMATrafficTypeClass(char *p_strid) : TrafficTypeClass(p_strid)
{
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: WCDMASectorClass::~WCDMASectorClass                                    ****/
/******************************************************************************************/
WCDMATrafficTypeClass::~WCDMATrafficTypeClass()
{
}
/******************************************************************************************/


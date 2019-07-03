/******************************************************************************************/
/**** PROGRAM: phs.cpp                                                                 ****/
/**** Michael Mandell 1/28/04                                                          ****/
/******************************************************************************************/

#include <math.h>
#include <string.h>

#include "cconst.h"
#include "hot_color.h"
#include "list.h"
#include "polygon.h"
#include "phs.h"
#include "strint.h"
#include "utm_conversion.h"
#include "st_param.h"

#if HAS_GUI
#include "gconst.h"
#endif

#define CGDEBUG 0

/******************************************************************************************/
/**** Global Strings                                                                   ****/
/******************************************************************************************/
char gstr_csid[]      = "CSID";
char gstr_gw_csc_cs[] = "GW_CSC_CS";
/******************************************************************************************/

/******************************************************************************************/
/**** Default parameter values                                                         ****/
/******************************************************************************************/
int    default_num_freq                           = 77;
int    default_num_slot                           = 4;
int    default_cntl_chan_freq                     = 25;
int    default_num_cntl_chan_slot                 = 80;
int    default_csid_format                        = CConst::CSID19NP;

int    default_num_attempt_assign_channel         = 4;
int    default_memoryless_ps                      = 0;
int    default_ac_hide_thr                        = 0;
int    default_ac_use_thr                         = 1;
double default_ac_hide_timer                      = 10.0;
double default_ac_use_timer                       = 10.0;
int    default_dca_alg                            = CConst::IntDCA;
int    default_max_sync_level                     = 4;

double default_sir_threshold_call_request_cs_db   = 6.0;
double default_sir_threshold_call_request_ps_db   = 6.0;
double default_sir_threshold_call_drop_cs_db      = 5.0;
double default_sir_threshold_call_drop_ps_db      = 5.0;

double default_int_threshold_call_request_cs_db   = 6.0;
double default_int_threshold_call_request_ps_db   = 6.0;
double default_int_threshold_call_drop_cs_db      = 5.0;
double default_int_threshold_call_drop_ps_db      = 5.0;

double default_cch_allocation_threshold_db        = 25.0;
double default_sync_level_allocation_threshold_db = 15.0;
/******************************************************************************************/

/******************************************************************************************/
/**** Static member declarations                                                       ****/
/******************************************************************************************/
unsigned int       PHSSectorClass :: csid_byte_length = 5;
int                PHSSectorClass :: st_data_period   = -1;
int                PHSNetworkClass:: csid_format      = CConst::CSID19NP;
int                PHSNetworkClass:: num_csid_format  = 2;


ListClass<STParamClass *> *PHSSectorClass::st_param_list = new ListClass<STParamClass *>(0);
/******************************************************************************************/

/******************************************************************************************/
/**** Get / Set functions                                                              ****/
/******************************************************************************************/
const int PHSNetworkClass::technology() { return(CConst::PHS); }
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: PHSSectorClass::PHSSectorClass                                         ****/
/******************************************************************************************/
PHSSectorClass::PHSSectorClass(CellClass *cell) : SectorClass(cell)
{
    set_default_parameters();
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: PHSSectorClass::~PHSSectorClass                                        ****/
/******************************************************************************************/
PHSSectorClass::~PHSSectorClass()
{
    if (num_unused_freq) {
        free(unused_freq);
    }

    if (csid_hex) { free(csid_hex); }

    if (st_data) { free(st_data); }

    return;
}
/******************************************************************************************/
/**** FUNCTION: PHSSectorClass::set_default_parameters                                 ****/
/******************************************************************************************/
void PHSSectorClass::set_default_parameters()
{
    SectorClass::set_default_parameters();

    csid_hex          = (unsigned char *) NULL;
    gw_csc_cs         = 0;
    cntl_chan_slot    = 0;
    num_unused_freq   = 0;
    unused_freq       = (int *) NULL;
    has_access_control = 0;
    sync_level        = -1;
    num_physical_tx   = 4;
    st_data           = (int *) NULL;

    cntl_chan_eff_tch_slot = -1;
    active = 1;
}
/******************************************************************************************/
/**** FUNCTION: PHSSectorClass::get_ival                                               ****/
/******************************************************************************************/
int PHSSectorClass::get_ival(int param_type)
{
    int ival;

    switch(param_type) {
        case CConst::SectorPagingArea:
            ival = extract_csid_data(CConst::CSIDPagingArea, PHSNetworkClass::csid_format);
            break;
        default:
            CORE_DUMP;
            break;
    }

    return(ival);
}
/******************************************************************************************/
/**** FUNCTION: PHSSectorClass::set_unused_freq                                        ****/
/******************************************************************************************/
void PHSSectorClass::set_unused_freq(ListClass<int> *freq_list)
{
    int i;

    if (num_unused_freq) {
        free(unused_freq);
    }

    num_unused_freq = freq_list->getSize();

    unused_freq = IVECTOR(num_unused_freq);

    for (i=0; i<=num_unused_freq-1; i++) {
        unused_freq[i] = (*freq_list)[i];
    }
}
/******************************************************************************************/
/**** FUNCTION: PHSSectorClass::st_comp_arrival_rate                                   ****/
/******************************************************************************************/
double PHSSectorClass::st_comp_arrival_rate(int traffic_idx)
{
    double arrival_rate;

    if (st_data) {
        if (st_data[11]+st_data[12]+st_data[14]+st_data[21] == 0) {
            if (traffic_idx == 0) {
                arrival_rate = (double) st_data[5]+st_data[6]+st_data[7];
            }
            if (traffic_idx == 1) {
                arrival_rate = 0.0;
            }
        } else {
            if (traffic_idx == 0) {
                arrival_rate = (double) (st_data[5]+st_data[6]+st_data[7])*(st_data[12]+st_data[14])/(st_data[11]+st_data[12]+st_data[14]+st_data[21]);
            }
            if (traffic_idx == 1) {
                arrival_rate = (double) (st_data[5]+st_data[6]+st_data[7])*(st_data[11]+st_data[21])/(st_data[11]+st_data[12]+st_data[14]+st_data[21]);
            }
        }
    } else {
        arrival_rate =0.0;
    }
    return (arrival_rate);
}
/******************************************************************************************/
/**** FUNCTION: PHSNetworkClass::Sum_st                                                 ****/
/******************************************************************************************/
int PHSNetworkClass::Sum_st(int st_idx)
{
    int cell_idx, sector_idx;
    CellClass *cell;
    PHSSectorClass *sector;
    int sum=0;

    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = (PHSSectorClass *) cell->sector_list[sector_idx];
            if (sector->st_data) {
                sum += sector->st_data[st_idx];
            }
        }
    }
    return sum;
}
/******************************************************************************************/
/**** FUNCTION: PHSSectorClass::duplicate                                              ****/
/******************************************************************************************/
SectorClass *PHSSectorClass::duplicate(int copy_csid)
{
    PHSSectorClass *new_sector = new PHSSectorClass((CellClass *) NULL);

    copy_sector_values((SectorClass *) new_sector);

    if (copy_csid) {
        if (csid_hex) {
            new_sector->csid_hex = (unsigned char *) malloc(csid_byte_length);
            memcpy(new_sector->csid_hex, csid_hex, csid_byte_length);
            new_sector->gw_csc_cs = gw_csc_cs;
        }
    }

    copy_sector_values((SectorClass *) new_sector);

    return((SectorClass *) new_sector);
}
/******************************************************************************************/
/**** FUNCTION: WLANSectorClass::copy_sector_values                                    ****/
/******************************************************************************************/
void PHSSectorClass::copy_sector_values(SectorClass *new_sector)
{
    SectorClass::copy_sector_values((SectorClass *) new_sector);

    PHSSectorClass *phs_sector = (PHSSectorClass *) new_sector;

    /**********************************************************************************/
    /**** Adjustable                                                               ****/
    /**********************************************************************************/
    phs_sector->cntl_chan_slot    = cntl_chan_slot;
    phs_sector->num_unused_freq   = num_unused_freq;
    phs_sector->unused_freq       = IVECTOR(num_unused_freq);
    for (int i=0; i<=num_unused_freq-1; i++) {
        phs_sector->unused_freq[i] = unused_freq[i];
    }
    phs_sector->has_access_control = has_access_control;
    phs_sector->mlc_priority_grp  = mlc_priority_grp;
    phs_sector->sync_level        = -1;
    phs_sector->num_physical_tx   = num_physical_tx;
    /**********************************************************************************/

    /**********************************************************************************/
    /**** Not Adjustable                                                           ****/
    /**********************************************************************************/
    phs_sector->cntl_chan_eff_tch_slot = -1;
    phs_sector->active = 1;
    /**********************************************************************************/

}
/******************************************************************************************/
/**** FUNCTION: PHSSectorClass::color_cells_by_pa                                      ****/
/******************************************************************************************/
void PHSNetworkClass::color_cells_by_pa()
{
    int cell_idx, idx, pa, color;
    int num_pa = 0;
    int pa_idx = 0;
    ListClass<int> *pa_list    = new ListClass<int>(0);
    ListClass<int> *color_list = new ListClass<int>(0);

    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        pa = ((PHSSectorClass *) (cell_list[cell_idx]->sector_list[0]))->extract_csid_data(CConst::CSIDPagingArea, csid_format);

        idx = pa_list->get_index(pa, 0);

        if (idx == -1) {
            idx = pa_list->getSize();
            pa_list->append(pa);

#if CGDEBUG // xxx
            color_list->append(hot_color->get_color(idx, pa_list->getSize()));
#endif
        }

#if !CGDEBUG
        if (cell_idx == num_cell-1)
        {
            num_pa = pa_list->getSize();

            for ( pa_idx=0; pa_idx<num_pa; pa_idx++ )
                color_list->append(hot_color->get_color(pa_idx, num_pa));

            for ( cell_idx=0; cell_idx<=num_cell-1; cell_idx++ ) {
                pa = ((PHSSectorClass *) (cell_list[cell_idx]->sector_list[0]))->extract_csid_data(CConst::CSIDPagingArea, csid_format);
                idx = pa_list->get_index(pa, 0);

                color = (*color_list)[idx];
                cell_list[cell_idx]->color = color;
            }

            break;
        }
#endif

#if CGDEBUG // xxx
            color = (*color_list)[idx];
            cell_list[cell_idx]->color = color;
#endif
    }
}
/******************************************************************************************/
/**** FUNCTION: PHSSectorClass::extract_csid_data                                      ****/
/******************************************************************************************/
int PHSSectorClass::extract_csid_data(int field, int csid_format)
{
    int n;

    if (csid_hex) {
        switch(field) {
            case CConst::CSIDPagingArea:
                if (csid_format == CConst::CSID19NP) {
                    n = ( ( 10*(csid_hex[2]>>4) + (csid_hex[2] & 0x0F) )<<4) | (csid_hex[3]>>4);
                } else if (csid_format == CConst::CSID16NP) {
                    n = ((csid_hex[1] & 0x7F)<<9) | (csid_hex[2]<<1) | (csid_hex[3] >>7);
                } else {
                    CORE_DUMP;
                }
                break;
            case CConst::CSIDGateway:
                if (csid_format == CConst::CSID19NP) {
                    n = ( 10*(csid_hex[2]>>4) + (csid_hex[2] & 0x0F) )<<4;
                } else if (csid_format == CConst::CSID16NP) {
                    n = ((csid_hex[2] & 0x0F)<<4);
                } else {
                    CORE_DUMP;
                }
                break;
            case CConst::CSIDCSC:
                if (csid_format == CConst::CSID19NP) {
                    n = (csid_hex[3]>>5);
                } else if (csid_format == CConst::CSID16NP) {
                    n = (csid_hex[3] & 0x0F);
                } else {
                    CORE_DUMP;
                }
                break;
            case CConst::CSIDCS:
                if (csid_format == CConst::CSID19NP) {
                    n = (csid_hex[4] & 0x1F);
                } else if (csid_format == CConst::CSID16NP) {
                    n = (csid_hex[3] & 0x7F);
                } else {
                    CORE_DUMP;
                }
                break;
            default:
                CORE_DUMP;
                break;
        }
    } else {
        n = -1;
    }

    return(n);
}
/******************************************************************************************/
/**** FUNCTION: PHSCellClass::PHSCellClass                                             ****/
/******************************************************************************************/
PHSCellClass::PHSCellClass(int p_num_sector) : CellClass()
{
    int sector_idx;
    num_sector = p_num_sector;

    if (num_sector) {
        sector_list = (SectorClass **) malloc((num_sector)*sizeof(SectorClass *));
    } else {
        sector_list = (SectorClass **) NULL;
    }
    for (sector_idx=0; sector_idx<=num_sector-1; sector_idx++) {
        sector_list[sector_idx] = (SectorClass *) new PHSSectorClass(this);
    }
}
/******************************************************************************************/
/**** FUNCTION: PHSCellClass::view_name                                                ****/
/******************************************************************************************/
char *PHSCellClass::view_name(int cell_idx, int cell_name_pref)
{
    int pa;
    static char str[50];

    switch(cell_name_pref) {
        case CConst::CellIdxRef:
            sprintf(str, "cell_%d", cell_idx);
            break;
        case CConst::CellCSNumberRef:
            sprintf(str, "%.6d", ((PHSSectorClass *) sector_list[0])->gw_csc_cs);
            break;
        case CConst::CellHexCSIDRef:
            if (((PHSSectorClass *) sector_list[0])->csid_hex) {
                hex_to_hexstr(str, ((PHSSectorClass *) sector_list[0])->csid_hex, PHSSectorClass::csid_byte_length);
            } else {
                sprintf(str, "**UNASSIGNED**");
            }
            break;
        case CConst::CellPagingArea:
            pa = ((PHSSectorClass *) sector_list[0])->extract_csid_data(CConst::CSIDPagingArea, PHSNetworkClass::csid_format);
            if (pa == -1) {
                sprintf(str, "UU_UU");
            } else {
                if (PHSNetworkClass::csid_format == CConst::CSID19NP) {
                    sprintf(str, "%d_%d", pa>>4, pa&0x0F);
                } else if (PHSNetworkClass::csid_format == CConst::CSID16NP) {
                    sprintf(str, "%d_%d_%d", pa>>9, (pa>>3)&0x3f, pa&0x07);
                }
            }

            break;
        default:
            CORE_DUMP; break;
    }

    return(str);
}
/******************************************************************************************/
/**** FUNCTION: PHSCellClass::~PHSCellClass                                            ****/
/******************************************************************************************/
PHSCellClass::~PHSCellClass()
{
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: PHSCellClass::duplicate                                                ****/
/******************************************************************************************/
CellClass *PHSCellClass::duplicate(const int x, const int y, int copy_csid)
{
    CellClass *new_cell = (CellClass *) new PHSCellClass();

    copy_cell_values(new_cell, x, y, copy_csid);

    return(new_cell);
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: PHSNetworkClass::PHSNetworkClass                                       ****/
/******************************************************************************************/
PHSNetworkClass::PHSNetworkClass() : NetworkClass()
{
    set_default_parameters();

    report_cell_name_opt_list->append( StrIntClass(gstr_gw_csc_cs, CConst::CellCSNumberRef) );
    report_cell_name_opt_list->append( StrIntClass(gstr_csid,      CConst::CellHexCSIDRef)  );
    report_cell_name_opt_list->resize();

    PHSSectorClass::st_param_list->append(new STParamClass( 1, 35, "<C-channel> Number of CRC error occurrence"));
    PHSSectorClass::st_param_list->append(new STParamClass( 2, 36, "<T-channel> Number of UW error occurrence"));
    PHSSectorClass::st_param_list->append(new STParamClass( 3, 37, "<T-channel> Number of CRC error occurrence"));
    PHSSectorClass::st_param_list->append(new STParamClass( 6,  3, "Number of link channel establishment request"));
    PHSSectorClass::st_param_list->append(new STParamClass( 7,  6, "Number of link channel establishment re-request"));
    PHSSectorClass::st_param_list->append(new STParamClass( 8,  7, "Number of link channel assignment rejects"));
    PHSSectorClass::st_param_list->append(new STParamClass( 9,  9, "Number of link channel assignment rejects(Frequency busy)"));
    PHSSectorClass::st_param_list->append(new STParamClass(10, 13, "Number of link channel establishment success"));
    PHSSectorClass::st_param_list->append(new STParamClass(11, 14, "Number of slots used as P-channel"));
    PHSSectorClass::st_param_list->append(new STParamClass(12, 15, "Number of slots used as up-link SCCH"));
    PHSSectorClass::st_param_list->append(new STParamClass(13, 16, "Number of slots used as down-link SCCH"));
    PHSSectorClass::st_param_list->append(new STParamClass(14, 17, "Number of location registration request"));
    PHSSectorClass::st_param_list->append(new STParamClass(17, 19, "The total number of calls PS originated"));
    PHSSectorClass::st_param_list->append(new STParamClass(18, 20, "Number of calls PS completed to originate"));
    PHSSectorClass::st_param_list->append(new STParamClass(19, 21, "Number of calls PS to receive"));
    PHSSectorClass::st_param_list->append(new STParamClass(20, 22, "Number of calls PS completed to receive"));
    PHSSectorClass::st_param_list->append(new STParamClass(21, 23, "The amount of seconds of calling from / to PS"));
    PHSSectorClass::st_param_list->append(new STParamClass(22, 25, "The amount of seconds of holding T-Channel"));
    PHSSectorClass::st_param_list->append(new STParamClass(24, 26, "Number of failures of switchback"));
    PHSSectorClass::st_param_list->append(new STParamClass(25, 27, "Number of requests of T-Channel switching from PS"));
    PHSSectorClass::st_param_list->append(new STParamClass(26, 28, "Number of indications of T-Channel switching"));
    PHSSectorClass::st_param_list->append(new STParamClass(28, 31, "Number of attempt of Hand-over"));
    PHSSectorClass::st_param_list->append(new STParamClass(29, 32, "Number of completions of Hand-over"));
    PHSSectorClass::st_param_list->append(new STParamClass(30, 33, "Number of hand-over"));
    PHSSectorClass::st_param_list->append(new STParamClass(31, 34, "Number of indications of hand-over"));
}
/******************************************************************************************/
/**** FUNCTION: PHSNetworkClass::~PHSNetworkClass                                      ****/
/******************************************************************************************/
PHSNetworkClass::~PHSNetworkClass()
{
    int i;
    for (i=0; i<=PHSSectorClass::st_param_list->getSize()-1; i++) {
        delete (*PHSSectorClass::st_param_list)[i];
    }
    delete PHSSectorClass::st_param_list;
    PHSSectorClass::st_param_list = (ListClass<STParamClass *> *) NULL;
    close_geometry();
}
/******************************************************************************************/
/**** FUNCTION: PHSNetworkClass::set_default_parameters                                ****/
/******************************************************************************************/
void PHSNetworkClass::set_default_parameters()
{
    NetworkClass::set_default_parameters();

    frequency                          = 1910.0E6;  /* PHS Frequency */
    num_freq                           = default_num_freq;
    num_slot                           = default_num_slot;
    cntl_chan_freq                     = default_cntl_chan_freq;
    num_cntl_chan_slot                 = default_num_cntl_chan_slot;
    csid_format                        = default_csid_format;
    num_csid_format                    = 2;

    bit_slot                           = -1;

    sir_threshold_call_request_cs_db   = default_sir_threshold_call_request_cs_db;
    sir_threshold_call_request_ps_db   = default_sir_threshold_call_request_ps_db;
    sir_threshold_call_drop_cs_db      = default_sir_threshold_call_drop_cs_db;
    sir_threshold_call_drop_ps_db      = default_sir_threshold_call_drop_ps_db;

    int_threshold_call_request_cs_db   = default_int_threshold_call_request_cs_db;
    int_threshold_call_request_ps_db   = default_int_threshold_call_request_ps_db;
    int_threshold_call_drop_cs_db      = default_int_threshold_call_drop_cs_db;
    int_threshold_call_drop_ps_db      = default_int_threshold_call_drop_ps_db;

    memoryless_ps                      = default_memoryless_ps;

    max_sync_level                     = default_max_sync_level;

    cch_allocation_threshold_db        = default_cch_allocation_threshold_db;
    sync_level_allocation_threshold_db = default_sync_level_allocation_threshold_db;

    cs_dca_alg                         = default_dca_alg;
    ps_dca_alg                         = default_dca_alg;

    ac_hide_thr                        = default_ac_hide_thr;
    ac_use_thr                         = default_ac_use_thr;
    ac_hide_timer                      = default_ac_hide_timer;
    ac_use_timer                       = default_ac_use_timer;

    sir_threshold_call_request_cs      = exp(sir_threshold_call_request_cs_db   * log(10.0)/10.0);
    sir_threshold_call_request_ps      = exp(sir_threshold_call_request_ps_db   * log(10.0)/10.0);
    sir_threshold_call_drop_cs         = exp(sir_threshold_call_drop_cs_db      * log(10.0)/10.0);
    sir_threshold_call_drop_ps         = exp(sir_threshold_call_drop_ps_db      * log(10.0)/10.0);

    int_threshold_call_request_cs      = exp(int_threshold_call_request_cs_db   * log(10.0)/10.0);
    int_threshold_call_request_ps      = exp(int_threshold_call_request_ps_db   * log(10.0)/10.0);
    int_threshold_call_drop_cs         = exp(int_threshold_call_drop_cs_db      * log(10.0)/10.0);
    int_threshold_call_drop_ps         = exp(int_threshold_call_drop_ps_db      * log(10.0)/10.0);

    cch_allocation_threshold           = exp(cch_allocation_threshold_db        * log(10.0)/10.0);
    sync_level_allocation_threshold    = exp(sync_level_allocation_threshold_db * log(10.0)/10.0);

    cch_rssi_table = (CchRssiTableClass *) NULL;

    subnet_naming_convention = CConst::CellHexCSIDRef;
}
/******************************************************************************************/
/**** FUNCTION: PHSNetworkClass::getSTParamIdx                                         ****/
/******************************************************************************************/
int PHSSectorClass::getSTParamIdx(int format, int stIdx)
{
    int paramIdx;
    int found = 0;
    int retval = -1;
    STParamClass *STParam;

    for (paramIdx=0; (paramIdx<=st_param_list->getSize()-1)&&(!found); paramIdx++) {
        STParam = (*st_param_list)[paramIdx];
        if (STParam->getSTIdx(format) == stIdx) {
            found = 1;
            retval = paramIdx;
        }
    }

    return(retval);
}
/******************************************************************************************/
/**** FUNCTION: PHSNetworkClass::getMaxSTIdx                                           ****/
/******************************************************************************************/
int PHSSectorClass::getMaxSTIdx(int format)
{
    int paramIdx;
    int maxSTIdx= -1;
    STParamClass *STParam;

    for (paramIdx=0; paramIdx<=st_param_list->getSize()-1; paramIdx++) {
        STParam = (*st_param_list)[paramIdx];
        if (STParam->getSTIdx(format) > maxSTIdx) {
            maxSTIdx = STParam->getSTIdx(format);
        }
    }

    return(maxSTIdx);
}
/******************************************************************************************/
/**** FUNCTION: PHSNetworkClass::checkSTData                                           ****/
/******************************************************************************************/
int PHSSectorClass::checkSTData(char *msg)
{
    int valid = 1;
    int paramIdx_22 = getSTParamIdx(0, 22);
    int paramIdx_21 = getSTParamIdx(0, 21);
    char *chptr;

    chptr = msg;
    if (st_data[paramIdx_22] < st_data[paramIdx_21]) {
        chptr += sprintf(chptr, "WARNING: Invalid ST data read for CS %.6d\n", gw_csc_cs);

        chptr += sprintf(chptr, "\"%s\" = %d\n", (*st_param_list)[paramIdx_22]->getDescription(), st_data[paramIdx_22]);

        chptr += sprintf(chptr, "\"%s\" = %d\n", (*st_param_list)[paramIdx_21]->getDescription(), st_data[paramIdx_21]);

        chptr += sprintf(chptr, "Must have \"%s\" >= \"%s\"\n", (*st_param_list)[paramIdx_22]->getDescription(),
                                                                (*st_param_list)[paramIdx_22]->getDescription());
        valid = 0;
    }

    return(valid);
}
/******************************************************************************************/
/**** FUNCTION: PHSNetworkClass::process_mode_change                                   ****/
/******************************************************************************************/
void PHSNetworkClass::process_mode_change()
{
    NetworkClass::process_mode_change();

    if ( (mode == CConst::noGeomMode) || (mode == CConst::editGeomMode) ) {
        bit_slot = -1;
    } else {
        BITWIDTH(bit_slot, num_slot-1);
    }
}
/******************************************************************************************/
/**** FUNCTION: display_setting                                                        ****/
/**** Display parameter settings                                                       ****/
/******************************************************************************************/
void PHSNetworkClass::display_settings(FILE *fp)
{
    char *chptr;

    NetworkClass::display_settings(fp);

    chptr = msg;
    chptr += sprintf(chptr, "\n");
    chptr += sprintf(chptr, "PHS PARAMETER SETTINGS:\n");
    chptr += sprintf(chptr, "NUM_FREQ                         = %d\n",                 num_freq);
    chptr += sprintf(chptr, "NUM_SLOT                         = %d\n",                 num_slot);
    PRMSG(fp, msg);

    chptr = msg;
    chptr += sprintf(chptr, "CS_DCA                           = %s\n",
        (cs_dca_alg==CConst::SIRDCA)    ? "SIR"     :
        (cs_dca_alg==CConst::IntDCA)    ? "INT"     :
        (cs_dca_alg==CConst::SIRIntDCA) ? "SIR_INT" :
        (cs_dca_alg==CConst::IntSIRDCA) ? "INT_SIR" : "MELCO");

    if ( (cs_dca_alg==CConst::SIRDCA) || (cs_dca_alg==CConst::SIRIntDCA) || (cs_dca_alg==CConst::IntSIRDCA) ) {
    chptr += sprintf(chptr, "SIR_THRESHOLD_CALL_REQUEST AT CS = %10.5e  (%5.3f dB)\n", sir_threshold_call_request_cs, sir_threshold_call_request_cs_db);
    chptr += sprintf(chptr, "SIR_THRESHOLD_CALL_DROP AT CS    = %10.5e  (%5.3f dB)\n", sir_threshold_call_drop_cs,    sir_threshold_call_drop_cs_db);
    }

    if ( (cs_dca_alg==CConst::IntDCA) || (cs_dca_alg==CConst::SIRIntDCA) || (cs_dca_alg==CConst::IntSIRDCA) ) {
    chptr += sprintf(chptr, "INT_THRESHOLD_CALL_REQUEST AT CS = %10.5e  (%5.3f dB)\n", int_threshold_call_request_cs, int_threshold_call_request_cs_db);
    chptr += sprintf(chptr, "INT_THRESHOLD_CALL_DROP AT CS    = %10.5e  (%5.3f dB)\n", int_threshold_call_drop_cs,    int_threshold_call_drop_cs_db);
    }
    PRMSG(fp, msg);

    chptr = msg;
    chptr += sprintf(chptr, "PS_DCA                           = %s\n",
        (ps_dca_alg==CConst::SIRDCA) ? "SIR" :
        (ps_dca_alg==CConst::IntDCA) ? "INT" : "INT_SIR");

    if ( (ps_dca_alg==CConst::SIRDCA) || (ps_dca_alg==CConst::IntSIRDCA) ) {
    chptr += sprintf(chptr, "SIR_THRESHOLD_CALL_REQUEST AT PS = %10.5e  (%5.3f dB)\n", sir_threshold_call_request_ps, sir_threshold_call_request_ps_db);
    chptr += sprintf(chptr, "SIR_THRESHOLD_CALL_DROP AT PS    = %10.5e  (%5.3f dB)\n", sir_threshold_call_drop_ps,    sir_threshold_call_drop_ps_db);
    }

    if ( (ps_dca_alg==CConst::IntDCA) || (ps_dca_alg==CConst::IntSIRDCA) ) {
    chptr += sprintf(chptr, "INT_THRESHOLD_CALL_REQUEST AT PS = %10.5e  (%5.3f dB)\n", int_threshold_call_request_ps, int_threshold_call_request_ps_db);
    chptr += sprintf(chptr, "INT_THRESHOLD_CALL_DROP AT PS    = %10.5e  (%5.3f dB)\n", int_threshold_call_drop_ps,    int_threshold_call_drop_ps_db);
    }
    PRMSG(fp, msg);

    chptr = msg;
    chptr += sprintf(chptr, "ACCESS_CONTROL_HIDE_THRESHOLD    = %d (phys chan avail)\n", ac_hide_thr);
    chptr += sprintf(chptr, "ACCESS_CONTROL_USE_THRESHOLD     = %d (phys chan avail)\n", ac_use_thr);
    chptr += sprintf(chptr, "ACCESS_CONTROL_HIDE_TIMER        = %10.5e (sec)\n",         ac_hide_timer);
    chptr += sprintf(chptr, "ACCESS_CONTROL_USE_TIMER         = %10.5e (sec)\n",         ac_use_timer);

    PRMSG(fp, msg);

    return;
}
/******************************************************************************************/
/**** FUNCTION: uid_to_sector                                                          ****/
/******************************************************************************************/
void PHSNetworkClass::uid_to_sector(char *uid, int &cell_idx, int &sector_idx)
{
    int c_idx, s_idx, found = 0;
    CellClass *cell;
    PHSSectorClass *sector;

    unsigned char *csid_hex = (unsigned char *) malloc(PHSSectorClass::csid_byte_length * sizeof(unsigned char));
    hexstr_to_hex(csid_hex, uid, PHSSectorClass::csid_byte_length);

    for (c_idx = 0; (c_idx<=num_cell-1)&&(!found); c_idx++) {
        cell = cell_list[c_idx];
        for (s_idx = 0; (s_idx<=cell->num_sector-1)&&(!found); s_idx++) {
            sector = (PHSSectorClass *) cell->sector_list[s_idx];
            if ( (sector->csid_hex) && (memcmp((void *) sector->csid_hex, (void *) csid_hex, PHSSectorClass::csid_byte_length)==0) ) {
                cell_idx   = c_idx;
                sector_idx = s_idx;
                found = 1;
            }
        }
    }

    free(csid_hex);

    if (!found) {
        cell_idx = -1;
        sector_idx = -1;
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: sector_to_uid                                                          ****/
/******************************************************************************************/
int PHSNetworkClass::sector_to_uid(char *uid, int cell_idx, int sector_idx)
{
    PHSSectorClass *sector;

    sector = (PHSSectorClass *) cell_list[cell_idx]->sector_list[sector_idx];
    hex_to_hexstr(uid, sector->csid_hex, PHSSectorClass::csid_byte_length);

    return(2*PHSSectorClass::csid_byte_length);
}
/******************************************************************************************/
/**** FUNCTION: uid_to_sector (Use GW_CSC_CS)                                          ****/
/******************************************************************************************/
void PHSNetworkClass::uid_to_sector(int   uid, int &cell_idx, int &sector_idx)
{
    int c_idx, s_idx, found = 0;
    CellClass *cell;
    PHSSectorClass *sector;

    for (c_idx = 0; (c_idx<=num_cell-1)&&(!found); c_idx++) {
        cell = cell_list[c_idx];
        for (s_idx = 0; (s_idx<=cell->num_sector-1)&&(!found); s_idx++) {
            sector = (PHSSectorClass *) cell->sector_list[s_idx];
            if ( sector->gw_csc_cs == uid ) {
                cell_idx   = c_idx;
                sector_idx = s_idx;
                found = 1;
            }
        }
    }

    if (!found) {
        cell_idx = -1;
        sector_idx = -1;
    }

    return;
}
/******************************************************************************************/
/**** FUNCTION: PHSNetworkClass::pa_to_str                                             ****/
/******************************************************************************************/
char *PHSNetworkClass::pa_to_str(int pa)
{
    static char str[50];

    if (pa == -1) {
        sprintf(str, "UU_UU");
    } else {
        if (csid_format == CConst::CSID19NP) {
            sprintf(str, "%d_%d", pa>>4, pa&0x0F);
        } else if (csid_format == CConst::CSID16NP) {
            sprintf(str, "%d_%d_%d", pa>>9, (pa>>3)&0x3f, pa&0x07);
        }
    }

    return(str);
}
/******************************************************************************************/
/**** FUNCTION: check_parameters                                                       ****/
/******************************************************************************************/
int PHSNetworkClass::check_parameters()
{
    int cell_idx, sector_idx, unused_freq_idx;
    int num_error;
    CellClass *cell;
    PHSSectorClass *sector;

    num_error = 0;
    if (num_freq <= 0) {
        sprintf(msg, "ERROR: num_freq = %d must be > 0\n", num_freq);
        PRMSG(stdout, msg);
        num_error++;
    }

    if (num_slot <= 0) {
        sprintf(msg, "ERROR: num_slot = %d must be > 0\n", num_freq);
        PRMSG(stdout, msg);
        num_error++;
    }

    if ( (num_cntl_chan_slot <= 0) || (num_cntl_chan_slot % num_slot != 0) ) {
        sprintf(msg, "ERROR: num_cntl_chan_slot = %d must be positive multiple of num_slot = %d\n",
            num_cntl_chan_slot, num_slot);
        PRMSG(stdout, msg);
        num_error++;
    }

    if ((cntl_chan_freq < 0)||(cntl_chan_freq > num_freq-1)) {
        sprintf(msg, "ERROR: cntl_chan_freq = %d must be between 0 and %d\n", cntl_chan_freq, num_freq-1);
        PRMSG(stdout, msg);
        num_error++;
    }

    for(cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        for(sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = (PHSSectorClass *) cell->sector_list[sector_idx];
            for(unused_freq_idx=0; unused_freq_idx<=sector->num_unused_freq-1; unused_freq_idx++) {
                if ((sector->unused_freq[unused_freq_idx] < 0) || (sector->unused_freq[unused_freq_idx] > num_freq-1)) {
                    sprintf(msg, "ERROR: CELL %d SECTOR %d UNUSED_FREQ[%d] = %d must be between 0 and %d\n",
                        cell_idx, sector_idx, unused_freq_idx, sector->unused_freq[unused_freq_idx], num_freq-1);
                    PRMSG(stdout, msg);
                    num_error++;
                }
            }
        }
    }

    if (num_error) {
        error_state = 1;
    }

    return(num_error);
}
/******************************************************************************************/
/**** FUNCTION: PHSNetworkClass::select_sectors_polygon                                ****/
/******************************************************************************************/
void PHSNetworkClass::select_sectors_polygon(ListClass<IntIntClass> *ii_list, double min_ext_dist, double max_ext_dist,
                                          char *filename, char *extended_filename, char *extended_bdy_filename)
{
    int i, found, flag;
    int cell_idx, cell_idx_x, sector_idx;
    int min_x, max_x, min_y, max_y;
    int min_ext_gpts;
    double posn_x, posn_y;
    double lon_min, lon_max, lat_min, lat_max;
    double distsq, max_ext_distsq;
    CellClass *cell, *cell_x;
    SectorClass *sector;
    PolygonClass *poly;
    ListClass<int> *sel_cell_list = new ListClass<int>(num_cell);
    FILE *fp;

    max_ext_distsq = max_ext_dist*max_ext_dist / (resolution*resolution);

    min_ext_gpts = (int) ceil(min_ext_dist / resolution);

    /**************************************************************************************/
    /**** Locate cells in polygon region                                               ****/
    /**************************************************************************************/
    poly = new PolygonClass(ii_list);

    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        if (poly->in_bdy_area(cell->posn_x, cell->posn_y)) {
            sel_cell_list->ins_elem(cell_idx, 1);
        }
    }

    delete poly;
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Write gw_csc_cs for CS's enclosed in polygon region                          ****/
    /**************************************************************************************/
    if ( (filename == NULL) || (strcmp(filename, "") == 0) ) {
        fp = stdout;
    } else if ( !(fp = fopen(filename, "w")) ) {
        sprintf(msg, "ERROR: Unable to write to file %s\n", filename);
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    sprintf( msg, "BigCSID\n");
    PRMSG(fp, msg);
    for (i=0; i<=sel_cell_list->getSize()-1; i++) {
        cell_idx = (*sel_cell_list)[i];
        cell = cell_list[cell_idx];
        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = cell->sector_list[sector_idx];
            sprintf( msg, "%.6d\n", ((PHSSectorClass *) sector)->gw_csc_cs);
            PRMSG(fp, msg);
        }
    }

    if (fp != stdout) {
        fclose(fp);
    }
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Find min_x, min_y, max_x, max_y for cells in selected region                 ****/
    /**** Expand by min_ext_dist                                                       ****/
    /**************************************************************************************/
    for (i=0; i<=sel_cell_list->getSize()-1; i++) {
        cell_idx = (*sel_cell_list)[i];
        cell = cell_list[cell_idx];
        if (i == 0) {
            min_x = max_x = cell->posn_x;
            min_y = max_y = cell->posn_y;
        } else {
            if (cell->posn_x < min_x) { min_x = cell->posn_x; }
            if (cell->posn_x > max_x) { max_x = cell->posn_x; }
            if (cell->posn_y < min_y) { min_y = cell->posn_y; }
            if (cell->posn_y > max_y) { max_y = cell->posn_y; }
        }
    }

    min_x -= min_ext_gpts;
    min_y -= min_ext_gpts;
    max_x += min_ext_gpts;
    max_y += min_ext_gpts;
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Write gw_csc_cs for CS's in extended polygon region                          ****/
    /**************************************************************************************/
    if ( (extended_filename == NULL) || (strcmp(extended_filename, "") == 0) ) {
        fp = stdout;
    } else if ( !(fp = fopen(extended_filename, "w")) ) {
        sprintf(msg, "ERROR: Unable to write to file %s\n", extended_filename);
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    sprintf( msg, "BigCSID MIN Distance = %12.10f\n", min_ext_dist);
    sprintf( msg, "BigCSID MAX Distance = %12.10f\n", max_ext_dist);
    PRMSG(fp, msg);
    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        found = 0;
        for (i=0; (i<=sel_cell_list->getSize()-1)&&(!found); i++) {
            cell_idx_x = (*sel_cell_list)[i];
            cell_x = cell_list[cell_idx_x];
            distsq = (double)  (cell_x->posn_x-cell->posn_x)*(cell_x->posn_x-cell->posn_x)
                             + (cell_x->posn_y-cell->posn_y)*(cell_x->posn_y-cell->posn_y);
            if (distsq <= max_ext_distsq) {
                found = 1;
            }
        }
        if (found) {
            if (cell->posn_x < min_x) { min_x = cell->posn_x; }
            if (cell->posn_x > max_x) { max_x = cell->posn_x; }
            if (cell->posn_y < min_y) { min_y = cell->posn_y; }
            if (cell->posn_y > max_y) { max_y = cell->posn_y; }
            for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                sector = cell->sector_list[sector_idx];
                sprintf( msg, "%.6d\n", ((PHSSectorClass *) sector)->gw_csc_cs);
                PRMSG(fp, msg);
                flag = 1;
            }
        }
    }
    if (fp != stdout) {
        fclose(fp);
    }
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Write gw_csc_cs for CS's in extended polygon region                          ****/
    /**************************************************************************************/
    if ( (flag) && (coordinate_system == CConst::CoordUTM) ) {

        if ( (extended_bdy_filename == NULL) || (strcmp(extended_bdy_filename, "") == 0) ) {
            fp = stdout;
        } else if ( !(fp = fopen(extended_bdy_filename, "w")) ) {
            sprintf(msg, "ERROR: Unable to write to file %s\n", extended_bdy_filename);
            PRMSG(stdout, msg); error_state = 1;
            return;
        }

        posn_x = idx_to_x(min_x-1);
        posn_y = idx_to_y(min_y-1);
        UTMtoLL( posn_x, posn_y, lon_min,  lat_min, utm_zone, utm_north, utm_equatorial_radius, utm_eccentricity_sq);

        posn_x = idx_to_x(max_x+1);
        posn_y = idx_to_y(max_y+1);
        UTMtoLL( posn_x, posn_y, lon_max,  lat_max, utm_zone, utm_north, utm_equatorial_radius, utm_eccentricity_sq);

        sprintf( msg, "NUM_PT: 4\n");                                 PRMSG(fp, msg);
        sprintf( msg, "    PT_0: %12.7f %12.7f\n", lon_min, lat_min); PRMSG(fp, msg);
        sprintf( msg, "    PT_1: %12.7f %12.7f\n", lon_max, lat_min); PRMSG(fp, msg);
        sprintf( msg, "    PT_2: %12.7f %12.7f\n", lon_max, lat_max); PRMSG(fp, msg);
        sprintf( msg, "    PT_3: %12.7f %12.7f\n", lon_min, lat_max); PRMSG(fp, msg);

        if (fp != stdout) {
            fclose(fp);
        }
    }
    /**************************************************************************************/

    delete sel_cell_list;

    return;
}

/******************************************************************************************/
/**** FUNCTION: PHSNetworkClass::select_sectors_polygon                                ****/
/******************************************************************************************/
void PHSNetworkClass::select_sectors_polygon(ListClass<IntIntClass> *ii_list, double min_ext_dist, double max_ext_dist,
                                          char *small_poly, char *ext_poly)
{
#if CGDBG
    char *small_poly = (char*) malloc ( 5000*sizeof(char) );
    char *ext_poly = (char*) malloc ( 5000*sizeof(char) );
#endif

    int i, found, flag;
    int cell_idx, cell_idx_x, sector_idx;
    int min_x, max_x, min_y, max_y;
    int min_ext_gpts;
    double posn_x, posn_y;
    double lon_min, lon_max, lat_min, lat_max;
    double distsq, max_ext_distsq;
    CellClass *cell, *cell_x;
    PolygonClass *poly;
    ListClass<int> *sel_cell_list = new ListClass<int>(num_cell);

    max_ext_distsq = max_ext_dist*max_ext_dist / (resolution*resolution);

    min_ext_gpts = (int) ceil(min_ext_dist / resolution);

    /**************************************************************************************/
    /**** Locate cells in polygon region                                               ****/
    /**************************************************************************************/
    poly = new PolygonClass(ii_list);

    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        if (poly->in_bdy_area(cell->posn_x, cell->posn_y)) {
            sel_cell_list->ins_elem(cell_idx, 1);
        }
    }

    delete poly;
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Write gw_csc_cs for CS's enclosed in polygon region                          ****/
    /**************************************************************************************/
    char *str = (char*) malloc ( 10*sizeof(char) );
    sprintf(small_poly, "'");
    for (i=0; i<=sel_cell_list->getSize()-1; i++) {
        cell_idx = (*sel_cell_list)[i];
        cell = cell_list[cell_idx];
        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++)
        {
#if 0
            // do not work, why? CG
            small_poly += sprintf(small_poly, "%d_%d ", cell_idx, sector_idx);
#endif
            sprintf(str, "%d_%d ", cell_idx, sector_idx);
            strcat ( small_poly, str);
        }

    }
    strcat ( small_poly, "'");
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Find min_x, min_y, max_x, max_y for cells in selected region                 ****/
    /**** Expand by min_ext_dist                                                       ****/
    /**************************************************************************************/
    for (i=0; i<=sel_cell_list->getSize()-1; i++) {
        cell_idx = (*sel_cell_list)[i];
        cell = cell_list[cell_idx];
        if (i == 0) {
            min_x = max_x = cell->posn_x;
            min_y = max_y = cell->posn_y;
        } else {
            if (cell->posn_x < min_x) { min_x = cell->posn_x; }
            if (cell->posn_x > max_x) { max_x = cell->posn_x; }
            if (cell->posn_y < min_y) { min_y = cell->posn_y; }
            if (cell->posn_y > max_y) { max_y = cell->posn_y; }
        }
    }

    min_x -= min_ext_gpts;
    min_y -= min_ext_gpts;
    max_x += min_ext_gpts;
    max_y += min_ext_gpts;
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Write gw_csc_cs for CS's in extended polygon region                          ****/
    /**************************************************************************************/
    sprintf(ext_poly, "'");
    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        found = 0;
        for (i=0; (i<=sel_cell_list->getSize()-1)&&(!found); i++) {
            cell_idx_x = (*sel_cell_list)[i];
            cell_x = cell_list[cell_idx_x];
            distsq = (double)  (cell_x->posn_x-cell->posn_x)*(cell_x->posn_x-cell->posn_x)
                             + (cell_x->posn_y-cell->posn_y)*(cell_x->posn_y-cell->posn_y);
            if (distsq <= max_ext_distsq) {
                found = 1;
            }
        }
        if (found) {
            if (cell->posn_x < min_x) { min_x = cell->posn_x; }
            if (cell->posn_x > max_x) { max_x = cell->posn_x; }
            if (cell->posn_y < min_y) { min_y = cell->posn_y; }
            if (cell->posn_y > max_y) { max_y = cell->posn_y; }
            for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                sprintf(str, "%d_%d ", cell_idx, sector_idx);
                strcat ( ext_poly, str);

                flag = 1;
            }
        }
    }
    strcat ( ext_poly, "'");

#if CGDBG
    printf( "small_poly : %s , ext_poly: %s\n", small_poly, ext_poly);
#endif

    delete sel_cell_list;

#if CGDBG
    free (small_poly);
    free (ext_poly);
    free (str);
#endif

    return;
}

/******************************************************************************************/
/**** FUNCTION: PHSTrafficTypeClass::PHSTrafficTypeClass                               ****/
/******************************************************************************************/
PHSTrafficTypeClass::PHSTrafficTypeClass(char *p_strid) : TrafficTypeClass(p_strid)
{
    num_attempt_request  = 4;
    num_attempt_handover = 4;
    ps_meas_best_channel = 0;
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: PHSTrafficTypeClass::~PHSTrafficTypeClass                              ****/
/******************************************************************************************/
PHSTrafficTypeClass::~PHSTrafficTypeClass()
{
}
/******************************************************************************************/
/**** FUNCTION: PHSTrafficTypeClass::get_num_attempt                                   ****/
/******************************************************************************************/
int PHSTrafficTypeClass::get_num_attempt(int type)
{
    int retval;

    switch (type) {
        case CConst::RequestEvent: retval = num_attempt_request;  break;
        case CConst::HandoverEvent: retval = num_attempt_handover; break;
        default: retval = -1; CORE_DUMP; break;
    }

    return(retval);
}
/******************************************************************************************/
/**** FUNCTION: PHSTrafficTypeClass::check                                             ****/
/******************************************************************************************/
int PHSTrafficTypeClass::check(NetworkClass *np)
{
    int num_error = 0;

    num_error += TrafficTypeClass::check(np);

    if (num_attempt_request > np->num_cell) {
        sprintf(np->msg, "ERROR: Traffic Type %s NUM_ATTEMPT_REQUEST = %d > NUM_CELL = %d\n",
            name(), num_attempt_request, np->num_cell);
        PRMSG(stdout, np->msg);
        num_error++;
    }

    if (num_attempt_handover > np->num_cell) {
        sprintf(np->msg, "ERROR: Traffic Type %s NUM_ATTEMPT_HANDOVER = %d > NUM_CELL = %d\n",
            name(), num_attempt_handover, np->num_cell);
        PRMSG(stdout, np->msg);
        num_error++;
    }

    return(num_error);
}
/******************************************************************************************/
/**** FUNCTION: reset_melco_params                                                     ****/
/**** OPTION:                                                                          ****/
/****     0: free memory                                                               ****/
/****     1: allocate memory                                                           ****/
/******************************************************************************************/
void PHSNetworkClass::reset_melco_params(int option)
{
    int i, index, grp_idx;
    int cell_idx, sector_idx, cs_idx;
    CellClass *cell;
    PHSSectorClass *sector;

    if (option == 0) {
        for (grp_idx=0; grp_idx<=mlc_ng-1; grp_idx++) {
            free(mlc_cmt[grp_idx]);
        }
        free(mlc_cmt);
        free(mlc_grpsz);
    }

    if (option == 1) {
        mlc_ng    = 6;
        mlc_grpsz = IVECTOR(mlc_ng);
        for (grp_idx=0; grp_idx<=mlc_ng-1; grp_idx++) {
            if (num_freq <= 82) {
                mlc_grpsz[grp_idx] = (6 + num_freq - 1 - grp_idx)/6;
            } else {
                mlc_grpsz[grp_idx] = (87 - grp_idx)/6 + ( 6 + (num_freq-82) - 1 - ((grp_idx+5)%6) )/6;
            }
        }

        mlc_cmt = (int **) malloc(mlc_ng*sizeof(int *));
        for (grp_idx=0; grp_idx<=mlc_ng-1; grp_idx++) {
            mlc_cmt[grp_idx] = IVECTOR(mlc_grpsz[grp_idx]);
        }

        for (i=0; i<=num_freq-1; i++) {
            if (i <= 81) {
                grp_idx = i % 6;
                index   = i / 6;
            } else {
                grp_idx = (i - 81) % 6;
                index   = (87 - grp_idx)/6 + ( 6 + (i+1-82) - 1 - ((grp_idx+5)%6) )/6 - 1;
            }
            mlc_cmt[grp_idx][index] = i;
        }


        for (cell_idx = 0; cell_idx<=num_cell-1; cell_idx++) {
            cell = cell_list[cell_idx];
            for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                sector = (PHSSectorClass *) cell->sector_list[sector_idx];
                cs_idx = (sector_idx << bit_cell) | cell_idx;
                sector->mlc_priority_grp = cs_idx % 6;
            }
        }
    }

#if 1
    printf("Carrier Management Table\n");
    for (grp_idx=0; grp_idx<=mlc_ng-1; grp_idx++) {
        printf("GROUP %d: ", grp_idx);
        for (index=0; index<=mlc_grpsz[grp_idx]-1; index++) {
            printf("%3d ", mlc_cmt[grp_idx][index]);
        }
        printf("\n");
    }
#endif

    return;
}
/******************************************************************************************/
/**** FUNCTION: coverage_pa_scan_fn                                                    ****/
/******************************************************************************************/
void coverage_pa_scan_fn(NetworkClass *np, int posn_x, int posn_y)
{
    int i, cell_idx, sector_idx, scan_idx;
    double pwr, max_pwr;
    CellClass *cell;
    SectorClass *sector;

    if (np->system_bdy->in_bdy_area(posn_x, posn_y)) {
        max_pwr = -1.0;
        for (i=0; i<=np->scan_cell_list->getSize()-1; i++) {
            cell_idx = (*np->scan_cell_list)[i];
            cell = np->cell_list[cell_idx];
            for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
                sector = cell->sector_list[sector_idx];
                if (((PHSSectorClass *) sector)->csid_hex) {
                    pwr = sector->tx_pwr*sector->comp_prop(np, posn_x, posn_y);
                    if (pwr > max_pwr) {
                        max_pwr = pwr;
                        scan_idx = ((PHSSectorClass *) sector)->extract_csid_data(CConst::CSIDPagingArea, ((PHSNetworkClass *) np)->csid_format);
                        np->scan_array[posn_x][posn_y] = scan_idx << 8;
                    }
                }
            }
        }
        if ( (np->scan_has_threshold) && (max_pwr < np->scan_threshold) ) {
            np->scan_array[posn_x][posn_y] = CConst::NullScan;
        }
    } else {
        np->scan_array[posn_x][posn_y] = CConst::NullScan;
    }
}
/******************************************************************************************/
/**** FUNCTION: group_by_csid_field                                                    ****/
/**** INPUTS:                                                                          ****/
/**** OUTPUTS:                                                                         ****/
void PHSNetworkClass::group_by_csid_field(int field)
{
    int found, i;
    int cell_idx, sector_idx, scan_idx, grp_idx, field_val;
    char *chptr, *namestr;
    CellClass *cell;
    SectorClass *sector;
    ListClass<int> *lc;
    ListClass<int> *field_val_list = new ListClass<int>(0);

    if (sector_group_list->getSize()) {
        sprintf(msg, "ERROR: Current design already contains groups");
        PRMSG(stdout, msg); error_state = 1;
        return;
    }

    for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        for (sector_idx=0; sector_idx<=cell->num_sector-1; sector_idx++) {
            sector = cell->sector_list[sector_idx];
            scan_idx = (sector_idx << bit_cell) | cell_idx;
            if (field == CConst::CSIDPagingArea) {
                field_val = ((PHSSectorClass *) sector)->extract_csid_data(CConst::CSIDPagingArea, csid_format);
            } else {
                field_val = ((PHSSectorClass *) sector)->gw_csc_cs / 100;
            }
            found = 0;
            for (grp_idx=0; (grp_idx<=sector_group_list->getSize()-1)&&(!found); grp_idx++) {
                if ((*field_val_list)[grp_idx] == field_val) {
                    ((ListClass<int> *) ((*sector_group_list)[grp_idx]))->append(scan_idx);;
                    found = 1;
                }
            }
            if (!found) {
                if (field == CConst::CSIDPagingArea) {
                    namestr = strdup(cell->view_name(cell_idx, CConst::CellPagingArea));
                } else {
                    namestr = CVECTOR(4);
                    sprintf(namestr, "%.4d", field_val);
                }
                lc = new ListClass<int>(1);
                lc->append(scan_idx);
                sector_group_list->append((void *) lc);
                sector_group_name_list->append(namestr);
                field_val_list->append(field_val);
            }
        }
    }

    delete field_val_list;

    for (grp_idx=0; grp_idx<=sector_group_list->getSize()-1; grp_idx++) {
        lc = (ListClass<int> *) ((*sector_group_list)[grp_idx]);
        lc->resize();
        lc->sort();
    }

#if 1
    chptr = msg;
    chptr += sprintf(chptr, "NUM SECTOR GROUPS: %d\n", sector_group_list->getSize());
    PRMSG(stdout, msg);
    int num_per_line = 100;
    int num;

    for (grp_idx=0; grp_idx<=sector_group_list->getSize()-1; grp_idx++) {
        chptr = msg;
        chptr += sprintf(chptr, "GROUP %d: \"%s\"", grp_idx, (*sector_group_name_list)[grp_idx]);
        num = 0;
        lc = (ListClass<int> *) ((*sector_group_list)[grp_idx]);
        for (i=0; i<=lc->getSize()-1; i++) {
            cell_idx   = (*lc)[i] & ((1<<bit_cell)-1);
            sector_idx = (*lc)[i] >> bit_cell;
            chptr += sprintf(chptr, " %d_%d", cell_idx, sector_idx);
            num++;
            if (num == num_per_line) {
                chptr += sprintf(chptr, "\n");
                PRMSG(stdout, msg);
                chptr = msg;
                num = 0;
            }
        }
        if (num) {
            chptr += sprintf(chptr, "\n");
            PRMSG(stdout, msg);
        }
    }
#endif
    return;
}

#undef CGDEBUG
/******************************************************************************************/

/******************************************************************************************/
/**** FILE: rd_prop_model.h                                                               ****/
/******************************************************************************************/

#ifndef RD_PROP_MODEL_H
#define RD_PROP_MODEL_H

#include "prop_model.h"

class RdPropModelClass : PropModelClass
{
public:
    RdPropModelClass(char *strid = (char *) NULL);
    const int type();
    double prop_power_loss(NetworkClass *np, SectorClass *sector, double delta_x, double delta_y);
    friend void NetworkClass::read_geometry(char *filename, char *WiSim_home, char *force_fmt);
    friend void NetworkClass::print_geometry(char *filename);
    friend int NetworkClass::process_command(char *line);
    friend class SelPropModDia;
    friend class PropModWidget;
private:
    // parameters of the model
    double exponent, coefficient;
};

#endif

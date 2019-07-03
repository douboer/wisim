/******************************************************************************************/
/**** FILE: license.h                                                                  ****/
/******************************************************************************************/

#ifndef LICENSE_H
#define LICENSE_H

#define LIC_FILE_NOT_FOUND 0      /**** License file not found (unable to read)       ****/
#define LIC_INVALID        1      /**** License file invalid                          ****/
#define LIC_EXPIRED        654    /**** License file expired                          ****/
#define LIC_NO_NET_CONN    67684  /**** Unable to get network time                    ****/
#define LIC_USB_INVALID    55665  /**** Usb key is not livalid                        ****/
#define LIC_VALID          472689 /**** Valid registration                            ****/
#define LIC_N              120    /**** Num times can run without getting net time    ****/
#define LIC_UPDATE_N       10     /**** Threshold to update N                         ****/
#define USE_MAC_INFO       0

int install_license_file(char *install_file, char *WiSim_home, unsigned char *reg_info, int ris);
int gen_reg_file(unsigned char *reg_info, int ris, char *name, char *email, char *company, char *&reg_file_rel, char *&reg_file_full);

#endif

/******************************************************************************************/
/**** FILE: pref.h                                                                     ****/
/**** Michael Mandell 1/15/02                                                          ****/
/******************************************************************************************/

/******************************************************************************************/
/****  CLASS: DatabasePrefClass                                                            ****/
/******************************************************************************************/
class DatabasePrefClass
{
public:
     DatabasePrefClass();
     ~DatabasePrefClass();
     char *connection;
     char *name;
     char *ipaddr;
     char *port;
     char *sid;
     char *user;
     char *password;
     friend class ConnectionNameDia;
     friend class DbStDia;
     friend class PrefDia;
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: PrefClass                                                                 ****/
/******************************************************************************************/
class PrefClass
{
public:
    PrefClass();
    ~PrefClass();
    int readFile(char *filename);
    int language;
    int report_cell_name_pref;
    int pwr_unit;
    double pwr_offset;
    char *pwr_str_long;
    char *pwr_str_short;
    char *filename;
    DatabasePrefClass **db_list;
    int num_db;
    int selected_db;

#if HAS_GUI
    int cell_size_idx;
    int vw_cell_name_pref;
    int rtd_view_pref;
#endif
/*
    char *user;
    char *password;
    char *name;
    int conn_flag;
*/
    char *lv_connection;
    int lv_dbnum;
    friend class ConnectionNameDia;
    friend class DbStDia;
    friend class PrefDia;
};
/******************************************************************************************/



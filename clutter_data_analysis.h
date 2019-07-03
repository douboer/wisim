
/*******************************************************************************************
**** PROGRAM: clutter_data_analysis.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

/*******************************************************************************************
**** PROGRAM: clutter_data_analysis.h 
**** AUTHOR:  Chengan 4/01/05
****          douboer@gmail.com
******************************************************************************************/

#ifndef CLUTTER_DATE_ANALYSIS_H 
#define CLUTTER_DATE_ANALYSIS_H

#include <math.h>
#include <cassert>
#include <string>
#include <fstream>
#include "randomc.h"
#include "prop_model.h"

#define CLUTTER_DEBUG       0
#define OLD_ALGORITHM       0
#define OUTPUT_TIME_INFOR   1  // print information of time cost  - 7/19/06 CG

const double DERROR = 1.0e-25;

class FigureEditor;
class NetworkClass;
class PolygonClass;
class CellClass;
class SectorClass;
class SparseMatrixClass;
struct divDistStruct;

template<class T> class ListClass;

//compare two double data
inline bool doubleEqual(double lhs, double rhs)
{
    return fabs(lhs - rhs) < DERROR ? bool(true) : bool(false); 
}

class GenericClutterPropModelClass : public PropModelClass
{
public:
    GenericClutterPropModelClass(char *strid = (char *) NULL);
    ~GenericClutterPropModelClass();

    virtual const int type();

    virtual double prop_power_loss(NetworkClass *, SectorClass *, int, int, int useheight=1, double angle_deg=0.0);
    virtual void get_prop_model_param_ptr(int, char *, int&, int *&, double *&);
    virtual int  comp_num_prop_param();
    virtual void print_params(FILE *fp, char *msg, int);
    virtual int is_clutter_model();
    virtual void split_clutter(NetworkClass *);
    virtual void clt_regulation( NetworkClass* np, ListClass<int> *scan_index_list, int iterate_flag);
    virtual void clt_fill( NetworkClass* np, ListClass<int> *scan_index_list);
    virtual int  get_color(int clutter_type, double min_neg, double max_pos);
    virtual void get_min_max_color(double &min_neg, double &max_pos);
    virtual void report_clutter(NetworkClass *np, FILE *fp, PolygonClass *p);

    void create_svd_matrices(NetworkClass *np, ListClass<int> *, PolygonClass *, int, int, double **&mx_a, double *&vec_b);
    void create_svd_matrices(NetworkClass *np, ListClass<int> *, PolygonClass *, int, int, SparseMatrixClass *&mx_a, double *&vec_b);
    double solve_clutter_coe(int num_eqn, int num_var, double **mx_a,double *vec_b);
    double compute_error(int num_eqn, int num_var, double **mx_a,double *vec_b);
    double compute_error(SparseMatrixClass *mx_a,double *vec_b);
    void clt_statistic( int& uu, int& zz, int& dd );
    void define_clutter(NetworkClass *np, int clutter_sim_res_ratio);
    PolygonClass *create_map_bdy();
    void refine_clutter(NetworkClass *np, int n, ListClass<int> *int_list);
    void save(NetworkClass *np, char *filename);

    friend class NetworkClass;
    friend class PropModMgrDia;
    friend class PixmapItem;
    friend class FigureEditor;

protected:
    int offset_x, offset_y, npts_x, npts_y;
    int clutter_sim_res_ratio;

    int num_clutter_type;           //The number of the clutter type
    int useheight;              
    double *mvec_x;                 //Clutter and height coefficients
};

class ClutterPropModelClass : public GenericClutterPropModelClass
{
public:
    ClutterPropModelClass(char *strid = (char *) NULL);
    ~ClutterPropModelClass();

    const int type();

    double prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int useheight=0, double angle_deg=0 );
    double compute_error(int num_road_test_pt,double **clutter_dis,double *power, int plot,char *flname);
    void get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr);
    int  comp_num_prop_param();
    void print_params(FILE *fp, char *msg, int);
    void clt_regulation( NetworkClass *np );            //Estimate the coefficients of clutters that is no road test data on it.
    void clt_statistic( int& uu, int& zz, int& dd );

    friend class NetworkClass;
    friend class PropModMgrDia;
    friend class GenericClutterPropModelClass;

private:
    double k, b;
};



class ClutterPropModelFullClass : public GenericClutterPropModelClass
{
public:
    ClutterPropModelFullClass(char *strid = (char *) NULL);
    ~ClutterPropModelFullClass();

    const int type();

    double prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int useheight=0, double angle_deg=0 );
    double compute_error(int num_road_test_pt,double **clutter_dis,double *power, int *a_posn, int plot,char *flname);
    void get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr);
    int  comp_num_prop_param();
    void print_params(FILE *fp, char *msg, int);
    void clt_regulation( NetworkClass *np );            //Estimate the coefficients of clutters that is no road test data on it.
    bool clt_fit_line( double& k, double& b );

    friend class NetworkClass;
    friend class PropModMgrDia;
    friend class FigureEditor;

private:
    double fit_k, fit_b;            //Fit the variables with serial values of clutter coefficient and constant.
};

class ClutterSymFullPropModelClass : public GenericClutterPropModelClass
{
public:
    ClutterSymFullPropModelClass(char *strid = (char *) NULL);
    ~ClutterSymFullPropModelClass();

    const int type();

    double prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int useheight=0, double angle_deg=0 );
    double compute_error(int num_road_test_pt,double **clutter_dis,double *power, int *a_posn, int plot,char *flname);
    void get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr);
    int  comp_num_prop_param();
    void print_params(FILE *fp, char *msg, int);
    void clt_regulation( NetworkClass *np );            //Estimate the coefficients of clutters that is no road test data on it.
    bool clt_fit_line( double& k, double& b );

    friend class NetworkClass;
    friend class PropModMgrDia;
    friend class FigureEditor;

private:
    double fit_k, fit_b;            //Fit the variables with serial values of clutter coefficient and constant.
};

class ClutterWtExpoPropModelClass : public GenericClutterPropModelClass
{
public:
    ClutterWtExpoPropModelClass(char *strid = (char *) NULL);
    ~ClutterWtExpoPropModelClass();

    const int type();

    double prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int useheight=0, double angle_deg=0 );
    void get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr);
    int  comp_num_prop_param();
    void print_params(FILE *fp, char *msg, int);
    void clt_regulation( NetworkClass* np, ListClass<int> *scan_index_list, int iterate_flag);
    bool clt_fit_line( double& k, double& b );
    void split_clutter( NetworkClass *np );

    friend class NetworkClass;
    friend class PropModMgrDia;
    friend class FigureEditor;

private:
    double fit_k, fit_b;            //Fit the variables with serial values of clutter coefficient and constant.
};



class ClutterWtExpoSlopePropModelClass : public GenericClutterPropModelClass
{
public:
    ClutterWtExpoSlopePropModelClass(char *strid = (char *) NULL);
    ~ClutterWtExpoSlopePropModelClass();

    const int type();

    double prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int useheight=0, double angle_deg=0 );
    void get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr);
    int  comp_num_prop_param();
    void print_params(FILE *fp, char *msg, int);
    void clt_regulation( NetworkClass* np, ListClass<int> *scan_index_list, int iterate_flag);
    void clt_fill( NetworkClass* np, ListClass<int> *scan_index_list);
    bool clt_fit_line( double& k, double& b );
    void split_clutter( NetworkClass *np );
    int  get_color(int clutter_type, double min_neg, double max_pos);
    void get_min_max_color(double &min_neg, double &max_pos);
    void report_clutter(NetworkClass *np, FILE *fp, PolygonClass *p);

    friend class NetworkClass;
    friend class PropModMgrDia;
    friend class FigureEditor;
    friend class GenericClutterPropModelClass;
    friend class ClutterInfoWid;

private:
    double r0;
};


class ClutterGlobalPropModelClass : public GenericClutterPropModelClass
{
public:
    ClutterGlobalPropModelClass(char *strid = (char *) NULL);
    ~ClutterGlobalPropModelClass();

    const int type();

    double prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int useheight=0, double angle_deg=0 );
    bool comp_global_model(NetworkClass *np, int num_scan_index, int *scan_index_list, double min_logd_threshold );
    void get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr);
    int  comp_num_prop_param();
    void print_params(FILE *fp, char *msg, int);
    void clt_regulation( NetworkClass* np, ListClass<int> *scan_index_list, int iterate_flag);
    void clt_fill( NetworkClass* np, ListClass<int> *scan_index_list);
    bool clt_fit_line( double& k, double& b );
    void split_clutter( NetworkClass *np );
    int  get_color(int clutter_type, double min_neg, double max_pos);
    void get_min_max_color(double &min_neg, double &max_pos);
    void report_clutter(NetworkClass *np, FILE *fp, PolygonClass *p);

    friend class NetworkClass;
    friend class PropModMgrDia;
    friend class FigureEditor;
    friend class GenericClutterPropModelClass;
    friend class ClutterInfoWid;

private:
    double r0;
    SegmentPropModelClass *globPm;
};

class ClutterExpoLinearPropModelClass : public GenericClutterPropModelClass
{
public:
    ClutterExpoLinearPropModelClass(char *strid = (char *) NULL);
    ~ClutterExpoLinearPropModelClass();

    const int type();

    double prop_power_loss(NetworkClass *np, SectorClass *sector, int delta_x, int delta_y, int useheight=0, double angle_deg=0 );
    void get_prop_model_param_ptr(int param_idx, char *str, int &type, int *&iptr, double *&dptr);
    int  comp_num_prop_param();
    void print_params(FILE *fp, char *msg, int);
    void clt_regulation( NetworkClass* np, ListClass<int> *scan_index_list, int iterate_flag);
    void clt_fill( NetworkClass* np, ListClass<int> *scan_index_list);
    bool clt_fit_line( double& k, double& b );
    void split_clutter( NetworkClass *np );
    int  get_color(int clutter_type, double min_neg, double max_pos);
    void get_min_max_color(double &min_neg, double &max_pos);
    void report_clutter(NetworkClass *np, FILE *fp, PolygonClass *p);

    friend class NetworkClass;
    friend class PropModMgrDia;
    friend class FigureEditor;
    friend class GenericClutterPropModelClass;
    friend class ClutterInfoWidClass;

private:
    double exponent, r0;
};

struct divDistStruct
{
    int    c_idx;      //Keep the corresponding clutter index that the line from CS to object point.
    double divD;       //Keep the value of D(i)/D(i-1), i is the passed clutter index, D is the cumulative sum distance of passed clutters.
};

struct distStruct
{
    int c_idx;
    double d;
};

// xxxxxxxxxx DELETE bool check_clt_bdy( NetworkClass* np, CellClass *cell );
int  get_clt_idx( NetworkClass* np, CellClass *cell );

//int matrixQR(matrix&, matrix& );
#if 0 
int matrixQR( double** r_mat, double** q_mat, int M, int N );
int le_houseHolder(double** a, double* b, int M, int N );
#endif

/************************************************************************************************/
#endif 

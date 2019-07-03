/******************************************************************************************/
/**** FILE: coverage.h                                                                 ****/
/**** Michael Mandell 2/20/04                                                          ****/
/******************************************************************************************/

#ifndef COVERAGE_H
#define COVERAGE_H

class NetworkClass;
class PolygonClass;
template<class T> class ListClass;

/******************************************************************************************/
/**** CLASS: CoverageClass                                                             ****/
/******************************************************************************************/
class CoverageClass
{
public:
    CoverageClass();
    CoverageClass(NetworkClass *np, char *param_strid, int param_type);
    ~CoverageClass();
    void shift(int x, int y);

    char *strid;
    double threshold, *level_list;
    int type, has_threshold;
    int *color_list;
    int init_sample_res;
    int num_digit;
    int use_gpm;
    double scan_fractional_area;

    PolygonClass **polygon_list;
    ListClass<int> *scan_list;

    int clipped_region;
    double dmax;
    ListClass<int> *cell_list;
    double eqv_num_sector;

    void read_coverage(  NetworkClass *np, char *filename, char *force_fmt);
    void write_coverage( NetworkClass *np, char *filename);
    void report_coverage(NetworkClass *np, char *filename, int pwr_unit);

    void read_coverage_1_1(NetworkClass *np, FILE *fp, char *line, char *filename, int linenum);
    void read_coverage_1_2(NetworkClass *np, FILE *fp, char *line, char *filename, int linenum);
    void scan_label(NetworkClass *np, char *label, double pwr_offset, int scan_type_idx);
    void set_num_digit(double pwr_offset);

#if HAS_ORACLE
    void write_coverage_db(NetworkClass *np);
    void read_coverage_db(NetworkClass *np);
#endif
};
/******************************************************************************************/

#endif

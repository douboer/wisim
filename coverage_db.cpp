/******************************************************************************************/
/**** PROGRAM: coverage.cpp                                                            ****/
/**** Michael Mandell 1/20/04                                                          ****/
/******************************************************************************************/
/**** Functions for reading/writing coverage analysis files.                           ****/
/******************************************************************************************/
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "antenna.h"
#include "WiSim.h"
#include "coverage.h"
#include "cconst.h"
#include "hot_color.h"
#include "polygon.h"
#include "list.h"
#include "pref.h"
#include "prop_model.h"

#include "database_fn.h"

/******************************************************************************************/
/**** FUNCTION: CoverageClass:write_coverage_db                                        ****/
/******************************************************************************************/

char *TrimStr(char *psz)
{
    int i = strlen(psz);
    while (--i >= 0) {
        if (psz[i] != ' ' && psz[i] != '\t')
            break;
    }
    psz[++i] = 0;
    return psz;
}




/* EXEC SQL BEGIN DECLARE SECTION; */

    char SQLSTATE[6];
/* EXEC SQL END DECLARE SECTION; */


#define CHECK_SQLSTATE if (CheckSqlState())  return;
//	printf("Database operation error.");

int CheckSqlState()
{
    int code = atoi(SQLSTATE);
    if (code != 0)
        return code;
    return 0;
}

void CoverageClass::write_coverage_db(NetworkClass *np)
{
#if 0
// xxxxxxx Update for modified CoverageClass structure
	DBClass WriteDB;
	char username[60], password[60], svcname[60];
	char* errmsg = (char*)malloc(255);

        printf("Login:");               fgets(username, 60, stdin);
        printf("Password:");            fgets(password, 60, stdin);
        printf("Server:");              fgets(svcname, 60, stdin);
        username[strlen(username)-1] = 0;
        password[strlen(password)-1] = 0;
        svcname[strlen(svcname)-1] = 0;

	if( !WriteDB.connectDB( svcname, username, password, errmsg ) ) {
		PRMSG(stdout, errmsg);
		np->error_state = 1;
		return;
	}

	char g_PROJNAME[32];
	char g_COORDINATE_SYSTEM[160];
	char g_TYPE[60];
	char g_COVERAGE_NAME[32];
	int g_INIT_SAMPLE_RES;
	double g_SCAN_FRACTIONAL_AREA;
	double g_THRESHOLD;
	int g_NUM_COLOR;
	int g_NUM_POLYGON_SCAN_TYPE;
	int g_NUM_POLYGON;

	int cvg_idx;
	int i;
	char str1[256];

	cvg_idx = np->num_coverage_analysis-1;	
	strcpy(g_PROJNAME, "chengdu001");
	
	if (np->coordinate_system == CConst::CoordGeneric) {
		sprintf(str1, "GENERIC");
	} else if (np->coordinate_system == CConst::CoordUTM) {
		sprintf(str1, "UTM:%.0f:%.9f:%d:%c\n", np->utm_equatorial_radius,
			np->utm_eccentricity_sq,np->utm_zone, (np->utm_north ? 'N' : 'S'));
	}	
	strcpy(g_COORDINATE_SYSTEM, str1);
	
	switch ( type ) {
		case 0: strcpy(g_TYPE, "Layer"); break;
		case 1: strcpy(g_TYPE, "Level"); break;
		case 2: strcpy(g_TYPE, "SirLayer"); break;
		default: break;
	}	

	strcpy(g_COVERAGE_NAME, strid);
	g_INIT_SAMPLE_RES = init_sample_res;
	g_SCAN_FRACTIONAL_AREA = scan_fractional_area;
	
	if ( type==CConst::levelCoverage )

		g_THRESHOLD = threshold;
	else
		g_THRESHOLD = 10.0*log(threshold)/log(10.0);
	g_NUM_COLOR = scan_list->getSize();
	g_NUM_POLYGON_SCAN_TYPE = num_polygon;
	g_NUM_POLYGON = num_polygon;
	
	WriteDB.beginSQL();
	WriteDB.processSQL("DELETE FROM ANAL_GENERAL");
	WriteDB.processSQL("DELETE FROM ANAL_COLOR");
	WriteDB.processSQL("DELETE FROM ANAL_POLYGON_SCAN_TYPE");
	WriteDB.processSQL("DELETE FROM ANAL_POL");
	WriteDB.processSQL("DELETE FROM ANAL_SEGMENT");
	WriteDB.processSQL("DELETE FROM ANAL_POL_SYS_BDY_PT");
	WriteDB.processSQL("DELETE FROM ANAL_LEVEL");
	WriteDB.endSQL();

	char* stmt = "insert into ANAL_GENERAL (PROJNAME,COORDINATE_SYSTEM,TYPE,COVERAGE_NAME,INIT_SAMPLE_RES,"
	"SCAN_FRACTIONAL_AREA,THRESHOLD,NUM_COLOR,NUM_POLYGON_SCAN_TYPE,NUM_POLYGON)"
        " values (:b0,:b1,:b2,:b3,:b4,:b5,:b6,:b7,:b8,:b9)";
	WriteDB.beginSQL();
	WriteDB.beginBind( stmt );
	WriteDB.bindString( ":b0", g_PROJNAME );
	WriteDB.bindString( ":b1", g_COORDINATE_SYSTEM );
	WriteDB.bindString( ":b2", g_TYPE );                    
	WriteDB.bindString( ":b3", g_COVERAGE_NAME );
	WriteDB.bindInt( ":b4", &g_INIT_SAMPLE_RES );
	WriteDB.bindDouble( ":b5", &g_SCAN_FRACTIONAL_AREA );
	WriteDB.bindDouble( ":b6", &g_THRESHOLD );
	WriteDB.bindInt( ":b7", &g_NUM_COLOR );
	WriteDB.bindInt( ":b8", &g_NUM_POLYGON_SCAN_TYPE );
	WriteDB.bindInt( ":b9", &g_NUM_POLYGON );
	WriteDB.execSQL();
	WriteDB.endBind();
	WriteDB.endSQL();
	
	printf("ANAL_GENERAL finished.\n");
	
	for (i = 1; i <= g_NUM_COLOR; i++)
	{
		int g_COLOR_NUM;
		int g_COLOR_VAL;

		stmt = "insert into ANAL_COLOR (PROJNAME,TYPE,COVERAGE_NAME,COLOR_NUM,COLOR_VAL)"
                       " values (:b0,:b1,:b2,:b3,:b4)";
		g_COLOR_NUM = i-1;
		g_COLOR_VAL = np->coverage_list[cvg_idx]->color_list[i-1];
		WriteDB.beginSQL();
		WriteDB.beginBind( stmt );
		WriteDB.bindString( ":b0", g_PROJNAME );
		WriteDB.bindString( ":b1", g_TYPE );                    
		WriteDB.bindString( ":b2", g_COVERAGE_NAME );
		WriteDB.bindInt( ":b3", &g_COLOR_NUM );
		WriteDB.bindInt( ":b4", &g_COLOR_VAL );
		WriteDB.execSQL();
		WriteDB.endBind();
		WriteDB.endSQL();
	}
	printf("ANAL_COLOR finished.\n");
	
	for (i = 1; i <= g_NUM_POLYGON_SCAN_TYPE; i++)
	{
		int g_POL_SCAN_TYPE_NUM;
		int g_POL_SCAN_TYPE_VAL;
		
		stmt = "insert into ANAL_POLYGON_SCAN_TYPE (PROJNAME,TYPE,COVERAGE_NAME,"
                       "POL_SCAN_TYPE_NUM,POL_SCAN_TYPE_VAL) values (:b0,:b1,:b2,:b3,:b4)";
		g_POL_SCAN_TYPE_NUM = i-1;
		g_POL_SCAN_TYPE_VAL = np->coverage_list[cvg_idx]->polygon_scan_type[i-1];
		WriteDB.beginSQL();
		WriteDB.beginBind( stmt );
		WriteDB.bindString( ":b0", g_PROJNAME );
		WriteDB.bindString( ":b1", g_TYPE );                    
		WriteDB.bindString( ":b2", g_COVERAGE_NAME );
		WriteDB.bindInt( ":b3", &g_POL_SCAN_TYPE_NUM );
		WriteDB.bindInt( ":b4", &g_POL_SCAN_TYPE_VAL );
		WriteDB.execSQL();
		WriteDB.endBind();
		WriteDB.endSQL();
	}
	printf("ANAL_POLYGON_SCAN_TYPE finished.\n");
	
	for (i = 1; i <= g_NUM_POLYGON; i++)
	{
		int g_POL_NUM;
		int g_NUM_SEGMENT;

		stmt = "insert into ANAL_POL (PROJNAME,TYPE,COVERAGE_NAME,POL_NUM,NUM_SEGMENT)"
                       " values (:b0,:b1,:b2,:b3,:b4)";
		g_POL_NUM = i-1;
		g_NUM_SEGMENT = np->coverage_list[cvg_idx]->polygon_list[i-1]->num_segment;
		WriteDB.beginSQL();
		WriteDB.beginBind( stmt );
		WriteDB.bindString( ":b0", g_PROJNAME );
		WriteDB.bindString( ":b1", g_TYPE );
		WriteDB.bindString( ":b2", g_COVERAGE_NAME );
		WriteDB.bindInt( ":b3", &g_POL_NUM );
		WriteDB.bindInt( ":b4", &g_NUM_SEGMENT );
		WriteDB.execSQL();
		WriteDB.endBind();
		WriteDB.endSQL();
		
		for (int j = 1; j <= g_NUM_SEGMENT; j++)
		{
			int g_SEGMENT_NUM;
			int g_NUM_BDY_PT;
			
			stmt = "insert into ANAL_SEGMENT (PROJNAME,TYPE,COVERAGE_NAME,"
                               "POL_NUM,SEGMENT_NUM,NUM_BDY_PT) values (:b0,:b1,:b2,:b3,:b4,:b5)";
			g_SEGMENT_NUM = j-1;
			g_NUM_BDY_PT = np->coverage_list[cvg_idx]->polygon_list[i-1]->num_bdy_pt[j-1];
			WriteDB.beginSQL();
			WriteDB.beginBind( stmt );
			WriteDB.bindString( ":b0", g_PROJNAME );
			WriteDB.bindString( ":b1", g_TYPE );
			WriteDB.bindString( ":b2", g_COVERAGE_NAME );
			WriteDB.bindInt( ":b3", &g_POL_NUM );
			WriteDB.bindInt( ":b4", &g_SEGMENT_NUM );
			WriteDB.bindInt( ":b5", &g_NUM_BDY_PT );
			WriteDB.execSQL();
			WriteDB.endBind();
			WriteDB.endSQL();

			polygon_list = np->coverage_list[cvg_idx]->polygon_list;
			for (int k = 1; k <= g_NUM_BDY_PT; k++)
			{
				int g_BDY_PT_NUM;
				double g_X_CRDNATE;
				double g_Y_CRDNATE;

				stmt = "insert into ANAL_POL_SYS_BDY_PT (PROJNAME,TYPE,COVERAGE_NAME,"
                                       "POL_NUM,SEGMENT_NUM,BDY_PT_NUM,X_CRDNATE,Y_CRDNATE)"
                                       " values (:b0,:b1,:b2,:b3,:b4,:b5,:b6,:b7)";
				g_BDY_PT_NUM = k-1;
				g_X_CRDNATE = np->idx_to_x(polygon_list[i-1]->bdy_pt_x[j-1][k-1]);
				g_Y_CRDNATE = np->idx_to_y(polygon_list[i-1]->bdy_pt_y[j-1][k-1]);
				WriteDB.beginSQL();
				WriteDB.beginBind( stmt );
				WriteDB.bindString( ":b0", g_PROJNAME );
				WriteDB.bindString( ":b1", g_TYPE );
				WriteDB.bindString( ":b2", g_COVERAGE_NAME );
				WriteDB.bindInt( ":b3", &g_POL_NUM );
				WriteDB.bindInt( ":b4", &g_SEGMENT_NUM );
				WriteDB.bindInt( ":b5", &g_BDY_PT_NUM );
				WriteDB.bindDouble( ":b6", &g_X_CRDNATE );
				WriteDB.bindDouble( ":b7", &g_Y_CRDNATE );
				WriteDB.execSQL();
				WriteDB.endBind();
				WriteDB.endSQL();
			}
		}
	}
	printf("ANAL_SEGMENT finished.\n");
	printf("ANAL_POL finished.\n");
	printf("ANAL_POL_SYS_BDY_PT finished.\n");
	
	if (np->coverage_list[cvg_idx]->type == CConst::levelCoverage)
	{
		for (i = 0; i < np->coverage_list[cvg_idx]->scan_list->getSize()-1; i++)
		{
			int g_LEVEL_NUM;
			double g_LEVEL_VAL;

			stmt = "insert into ANAL_LEVEL (PROJNAME,TYPE,COVERAGE_NAME,"
                               "LEVEL_NUM,LEVEL_VAL) values (:b0,:b1,:b2,:b3,:b4)";
			g_LEVEL_NUM = i;
			g_LEVEL_VAL = 10.0*log(np->coverage_list[cvg_idx]->level_list[i])/log(10.0);
			WriteDB.beginSQL();
			WriteDB.beginBind( stmt );
			WriteDB.bindString( ":b0", g_PROJNAME );
			WriteDB.bindString( ":b1", g_TYPE );
			WriteDB.bindString( ":b2", g_COVERAGE_NAME );
			WriteDB.bindInt( ":b3", &g_LEVEL_NUM );
			WriteDB.bindDouble( ":b4", &g_LEVEL_VAL );
			WriteDB.execSQL();
			WriteDB.endBind();
			WriteDB.endSQL();
		}
		printf("ANAL_LEVEL finished.\n");
	}
	WriteDB.disconnectDB();
	printf("Write DB finished.\n");
#endif
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: CoverageClass:read_coverage_db                                         ****/
/******************************************************************************************/
void CoverageClass::read_coverage_db(NetworkClass *np)
{
#if 0
// xxxxxxx Update for modified CoverageClass structure
    int i;
    DBClass ReadDB;
    char username[60], password[60], svcname[60];
    char* errmsg = (char*)malloc(255*sizeof(char));
/*
    printf("Login:");               fgets(username, 60, stdin);
    printf("Password:");            fgets(password, 60, stdin);
    printf("Server:");              fgets(svcname, 60, stdin);
    username[strlen(username)-1] = 0;
    password[strlen(password)-1] = 0;
    svcname[strlen(svcname)-1] = 0;
*/
   sprintf( username, "scott" );
   sprintf( password, "tiger" );
   sprintf( svcname, "WiSim" );

   if( !ReadDB.connectDB( svcname, username, password, errmsg ) ) {
	PRMSG(stdout, errmsg);
	np->error_state = 1;
	return;
    }
    
    char* str1;
    char* stmt1 = (char*)malloc(300*sizeof(char));
    printf("\n..........read_coverage_analysis_db begins..........\n\n");
//COORDINATE_SYSTEM

    char* stmt = "select PROJNAME,COORDINATE_SYSTEM,TYPE,COVERAGE_NAME,INIT_SAMPLE_RES,"
                 "SCAN_FRACTIONAL_AREA,THRESHOLD,NUM_COLOR,NUM_POLYGON_SCAN_TYPE,NUM_POLYGON from ANAL_GENERAL";
    
    char** v_coordinate_system = (char**)malloc(sizeof(char*));
    v_coordinate_system[0] = (char*)malloc(161*sizeof(char));
    ReadDB.readString( stmt, v_coordinate_system, 2, 160 );

    str1 = v_coordinate_system[0];
     
    if (str1) {
        if (strcmp(str1, "GENERIC")==0) {
            np->coordinate_system = CConst::CoordGeneric;
        }
        else if (strncmp(str1, "UTM:", 4)==0) {
            np->coordinate_system = CConst::CoordUTM;
            str1 = strtok(str1+4, ":");
            np->utm_equatorial_radius = atof(str1);
            str1 = strtok(NULL, ":");
            np->utm_eccentricity_sq = atof(str1);
            str1 = strtok(NULL, ":");
            np->utm_zone = atoi(str1);
            str1 = strtok(NULL, ":");
            if (strcmp(str1, "N")==0) {
                np->utm_north = 1;
            }
            else if (strcmp(str1, "S")==0) {
                np->utm_north = 0;
            }
            else {
                sprintf(np->msg, "ERROR: invalid database value in table ANAL_GENERAL column COORDINATE_SYSTEM");
                PRMSG(stdout, np->msg);
                np->error_state = 1;
                return;
            }
        }
    }
    else {
        sprintf(np->msg, "ERROR: invalid database value in table ANAL_GENERAL column COORDINATE_SYSTEM");
        PRMSG(stdout, np->msg);
        np->error_state = 1;
        return;
    }
    printf("coordinate_system: COORD_UTM\nutm_equatorial_radius :%f\nutm_eccentricity_sq: %f\nutm_zone: %d\n",
            np->utm_equatorial_radius,np->utm_eccentricity_sq,np->utm_zone );

    if( v_coordinate_system[0])  free( v_coordinate_system[0] );
    if (v_coordinate_system )  free( v_coordinate_system );


//Coverage_type

    char** v_type = (char**)malloc(sizeof(char*));
    v_type[0] = (char*)malloc(61*sizeof(char));
    ReadDB.readString( stmt, v_type, 3, 60 );

    str1 = v_type[0];
    if ( str1 ) {
        if (strcmp(str1, "Layer")==0) {
            type = CConst::layerCoverage;
        }
        else if (strcmp(str1, "Level")==0) {
            type = CConst::levelCoverage;
        } 
        else if (strcmp(str1, "SirLayer")==0) {
            type = CConst::sirLayerCoverage;
        } 
        else {
            sprintf(np->msg, "ERROR: type is out of range\n");
            PRMSG(stdout, np->msg);
            np->error_state = 1;
            return;
        }
    }
    printf("type: %s\n", str1);
    if( v_type[0] )  free( v_type[0] );
    if( v_type )  free( v_type );
    
//Coverage_name

    char** v_coverage_name = (char**)malloc(sizeof(char*));
    v_coverage_name[0] = (char*)malloc(21*sizeof(char));
    ReadDB.readString( stmt, v_coverage_name, 4, 20 );

    str1 = v_coverage_name[0];

    if (str1) {
        strid = strdup(str1);
    } 
    else {
        sprintf(np->msg, "ERROR: coverage name is empty\n");
        PRMSG(stdout, np->msg);
        np->error_state = 1;
        return;
    }
    printf("coverage_name: %s\n", str1 );
    if( v_coverage_name[0] )  free( v_coverage_name[0] );
    if( v_coverage_name )  free( v_coverage_name );

//Init_sample_res

    int* v_init_sample_res = (int*)malloc(sizeof(int));
    ReadDB.readInt( stmt, v_init_sample_res, 5 );
    init_sample_res = v_init_sample_res[0];
    printf( "init_sample_res: %d\n", init_sample_res );
    if( v_init_sample_res )  free( v_init_sample_res );

//Scan_fractional_area
    double* v_scan_fractional_area = (double*)malloc(sizeof(double));
    ReadDB.readDouble( stmt, v_scan_fractional_area, 6 );
    scan_fractional_area = v_scan_fractional_area[0];
    printf( "scan_fractional_area: %f\n", scan_fractional_area );
    if( v_scan_fractional_area )  free( v_scan_fractional_area );

//Threshold
    double* v_threshold = (double*)malloc(sizeof(double));
    ReadDB.readDouble( stmt, v_threshold, 7 );
    if ( type == CConst::levelCoverage )
        threshold = v_threshold[0];
    else 
        threshold = exp(log(10.0)*v_threshold[0]/10.0);
    printf( "threshold: %f\n", threshold );
    if( v_threshold )  free( v_threshold );


    if( type == CConst::levelCoverage ) {
           
    //Num_level
        strcpy( stmt1, "select count(*) from ANAL_LEVEL" );
        int* v_num_level = (int*)malloc(sizeof(int));
        ReadDB.readInt( stmt1, v_num_level, 1 );
        num_scan_type = v_num_level[0]+1;
        level_list = DVECTOR(num_scan_type-1);
        printf( "num_scan_type: %d\n", num_scan_type );
        if( v_num_level )  free( v_num_level );

    //Level
        strcpy( stmt1, "select PROJNAME,TYPE,COVERAGE_NAME,LEVEL_NUM,LEVEL_VAL from ANAL_LEVEL"
                     " order by LEVEL_NUM" );
        double* v_level_val = (double*)malloc( (num_scan_type-1)*sizeof(double));
        ReadDB.readDouble( stmt1, v_level_val, 5 );
        for( i=0; i<=num_scan_type-2; i++ ) {
            level_list[i] = exp(log(10.0)*v_level_val[i]/10.0);
            printf( "level_list[%d]: %f\n", i, level_list[i] );
        }
        if( v_level_val )  free( v_level_val );
    }                 

//Num_color
    int* v_num_color = (int*)malloc(sizeof(int));
    ReadDB.readInt( stmt, v_num_color, 8 ); 
    if ( type != CConst::levelCoverage ) 
        num_scan_type = v_num_color[0];
    color_list = IVECTOR(num_scan_type);
    printf( "num_color: %d\n", num_scan_type );
    if( v_num_color )  free( v_num_color );

//Color
    strcpy( stmt1, "select PROJNAME,TYPE,COVERAGE_NAME,COLOR_NUM,COLOR_VAL from ANAL_COLOR order by COLOR_NUM" );
    int* v_color_val = (int*)malloc( num_scan_type*sizeof(int) );
    ReadDB.readInt( stmt1, v_color_val, 5 );
    for( i=0; i<=num_scan_type-1; i++ ) {
        color_list[i]=v_color_val[i];
        printf( "color_list[%d]: %d\n", i, color_list[i] );
    }
    if( v_color_val )  free( v_color_val );
   
//Num_polygon_scan_type
    int* v_num_polygon_scan_type = (int*)malloc(sizeof(int));
    ReadDB.readInt( stmt, v_num_polygon_scan_type, 9 );
    num_polygon = v_num_polygon_scan_type[0];
    polygon_scan_type = IVECTOR(num_polygon);
    printf( "num_polygon_scan_type: %d\n", num_polygon );
    if( v_num_polygon_scan_type )  free( v_num_polygon_scan_type );

//Polygon_scan_type
    strcpy( stmt1, "select PROJNAME,TYPE,COVERAGE_NAME,POL_SCAN_TYPE_NUM,"
            "POL_SCAN_TYPE_VAL from ANAL_POLYGON_SCAN_TYPE order by POL_SCAN_TYPE_NUM" );
    int* v_pol_scan_type_val = (int*)malloc( num_polygon*sizeof(int) );
    ReadDB.readInt( stmt1, v_pol_scan_type_val, 5 );
    for( i=0; i<=num_polygon-1; i++ ) {
        polygon_scan_type[i]=v_pol_scan_type_val[i];
        printf( "polygon_scan_type[%d]: %d\n", i, polygon_scan_type[i] );
    }
    if( v_pol_scan_type_val )  free( v_pol_scan_type_val );

//Num_polygon
    int* v_num_polygon = (int*)malloc(sizeof(int));
    ReadDB.readInt( stmt, v_num_polygon, 10 );
    if( num_polygon != v_num_polygon[0] ) {
        sprintf(np->msg, "ERROR: invalid database value in table ANAL_GENERAL column NUM_POLYGON"
                         "-- not as same as NUM_POLYGON_SCAN_TYPE");
        PRMSG(stdout, np->msg);
        np->error_state = 1;
        return;
    }
    polygon_list = (PolygonClass **) malloc( num_polygon*sizeof(PolygonClass *) );
    printf( "num_polygon: %d\n", num_polygon );
    if( v_num_polygon )  free( v_num_polygon );

//Polygon
    for( i=0; i<=num_polygon-1; i++ ) 
        polygon_list[i] = new PolygonClass();
    
//Polygon_num_segment
    strcpy( stmt1, "select PROJNAME,TYPE,COVERAGE_NAME,POL_NUM,NUM_SEGMENT from ANAL_POL"
            " order by POL_NUM" );
    int* v_num_segment = (int*)malloc(num_polygon*sizeof(int));
    ReadDB.readInt( stmt1, v_num_segment, 5 );
    for( i=0; i<=num_polygon-1; i++ ) {
        polygon_list[i]->num_segment = v_num_segment[i];
        printf("num_segment[%d]: %d\n", i, polygon_list[i]->num_segment );
        if (polygon_list[i]->num_segment < 1) {
            sprintf(np->msg, "ERROR: invalid database value in table ANAL_POL column NUM_SEGMENT"
                             " -- must be >= 1 " );
                        PRMSG(stdout, np->msg);
                        np->error_state = 1;
                        return;
        }
        polygon_list[i]->num_bdy_pt = IVECTOR(polygon_list[i]->num_segment);
        polygon_list[i]->bdy_pt_x   = (int **) malloc(polygon_list[i]->num_segment*sizeof(int *));
        polygon_list[i]->bdy_pt_y   = (int **) malloc(polygon_list[i]->num_segment*sizeof(int *));
    }
    if( v_num_segment )  free( v_num_segment );

//Polygon_segment
    strcpy( stmt1, "select count(*) from ANAL_SEGMENT" );
    int* v_num_segment_all = (int*)malloc( sizeof(int) );
    ReadDB.readInt( stmt1, v_num_segment_all, 1 );
    int* v_num_bdy_pt = (int*)malloc( v_num_segment_all[0]*sizeof(int) );
    int* v_segment_num = (int*)malloc( v_num_segment_all[0]*sizeof(int) );
    int* v_pol_num = (int*)malloc( v_num_segment_all[0]*sizeof(int) );

    strcpy( stmt1, "select PROJNAME,TYPE,COVERAGE_NAME,POL_NUM,SEGMENT_NUM,NUM_BDY_PT"    
            " from ANAL_SEGMENT order by POL_NUM,SEGMENT_NUM" );
    ReadDB.readInt( stmt1, v_num_bdy_pt, 6 );
    ReadDB.readInt( stmt1, v_segment_num, 5 );
    ReadDB.readInt( stmt1, v_pol_num, 4 );

    printf("start reading segments...\n");
    for( i=0; i<=v_num_segment_all[0]-1; i++ ) {
        polygon_list[v_pol_num[i]]->num_bdy_pt[v_segment_num[i]] = v_num_bdy_pt[i];

        polygon_list[v_pol_num[i]]->bdy_pt_x[v_segment_num[i]]
             = IVECTOR(v_num_bdy_pt[i]);
        polygon_list[v_pol_num[i]]->bdy_pt_y[v_segment_num[i]]
             = IVECTOR(v_num_bdy_pt[i]);

//        printf("polygon_list[%d]->num_bdy_pt[%d]: %d\n",v_pol_num[i], v_segment_num[i], v_num_bdy_pt[i]);
        if ( v_num_bdy_pt[i]< 3 ) {
            sprintf(np->msg, "ERROR: invalid database value in table ANAL_SEGMENT column NUM_BDY_PT"
                             " -- must be > 3 ");
            PRMSG(stdout, np->msg);
            np->error_state = 1;
            return;
        }
    }
    printf("finished reading segments...\n");
    if( v_num_segment_all )  free( v_num_segment_all );
    if( v_num_bdy_pt )  free( v_num_bdy_pt );
    if( v_segment_num )  free( v_segment_num );
    if( v_pol_num )  free( v_pol_num );

//Polygon_bdy_pt
    strcpy( stmt1, "select count(*) from ANAL_POL_SYS_BDY_PT" );
    int* v_num_bdy_pt_all = (int*)malloc( sizeof(int) );
    ReadDB.readInt( stmt1, v_num_bdy_pt_all, 1 );
    v_pol_num = (int*)malloc( v_num_bdy_pt_all[0]*sizeof(int) );
    v_segment_num = (int*)malloc( v_num_bdy_pt_all[0]*sizeof(int) );
    int* v_bdy_pt_num = (int*)malloc( v_num_bdy_pt_all[0]*sizeof(int) );
    double* v_bdy_pt_x = (double*)malloc( v_num_bdy_pt_all[0]*sizeof(double) );
    double* v_bdy_pt_y = (double*)malloc( v_num_bdy_pt_all[0]*sizeof(double) );

    strcpy( stmt1, "select PROJNAME,TYPE,COVERAGE_NAME,POL_NUM,SEGMENT_NUM,BDY_PT_NUM,X_CRDNATE,"
            "Y_CRDNATE from ANAL_POL_SYS_BDY_PT order by POL_NUM,SEGMENT_NUM,BDY_PT_NUM" );

    ReadDB.readInt( stmt1, v_pol_num, 4 );
    ReadDB.readInt( stmt1, v_segment_num, 5 );
    ReadDB.readInt( stmt1, v_bdy_pt_num, 6 );
    ReadDB.readDouble( stmt1, v_bdy_pt_x, 7 );
    ReadDB.readDouble( stmt1, v_bdy_pt_y, 8 );

    printf("start reading bdy_pt...\n");
    for( i=0; i<=v_num_bdy_pt_all[0]-1; i++ ) {
        if( !check_grid_val( v_bdy_pt_x[i], np->resolution, np->system_startx,
            &(polygon_list[v_pol_num[i]]->bdy_pt_x[v_segment_num[i]][v_bdy_pt_num[i]]) ) ) {
            sprintf( np->msg, "ERROR: invalid database value in table ANAL_POL_SYS_BDY_PT\n"
                     "Boundary point x=%d, is not an integer multiple of"
                     "resolution=%15.10f\n", v_bdy_pt_num[i], np->resolution );
            PRMSG(stdout, np->msg);
            np->error_state = 1;
            return;
        }
        if( !check_grid_val( v_bdy_pt_y[i], np->resolution, np->system_starty,
            &(polygon_list[v_pol_num[i]]->bdy_pt_y[v_segment_num[i]][v_bdy_pt_num[i]]) ) ) {
            sprintf( np->msg, "ERROR: invalid database value in table ANAL_POL_SYS_BDY_PT\n"
                     "Boundary point y=%d, is not an integer multiple of"
                     "resolution=%15.10f\n", v_bdy_pt_num[i], np->resolution );
            PRMSG(stdout, np->msg);
            np->error_state = 1;
            return;
        }
/*        printf("polygon_list[%d]->bdy_pt_x[%d][%d]: %d\t", v_pol_num[i], v_segment_num[i], v_bdy_pt_num[i],
                polygon_list[v_pol_num[i]]->bdy_pt_x[v_segment_num[i]][v_bdy_pt_num[i]] );
        printf("y: %d\n",
                polygon_list[v_pol_num[i]]->bdy_pt_y[v_segment_num[i]][v_bdy_pt_num[i]] );*/
    }
    printf("finished reading bdy_pt...\n");
    if( v_num_bdy_pt_all )  free( v_num_bdy_pt_all );
    if( v_pol_num)  free( v_pol_num );
    if( v_segment_num )  free( v_segment_num );
    if( v_bdy_pt_num )  free( v_bdy_pt_num );
    if( v_bdy_pt_x )  free( v_bdy_pt_x );
    if( v_bdy_pt_y )  free( v_bdy_pt_y );

    if( stmt1 )  free( stmt1 );
   
    ReadDB.disconnectDB();
    printf("\n..........read_coverage_analysis_db finished..........\n\n");


#endif
    return;
}
/******************************************************************************************/

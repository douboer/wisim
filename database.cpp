#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "database_fn.h"
#include "polygon.h"
#include "phs.h"
#include "wisim.h"
#include "prop_model.h"
#include "antenna.h"
#include "pref.h"

void  PHSNetworkClass::read_geometry_db(char *wisim_home)
{
#if 0
// xxxxxxxxxxx Update for new Road Test Data structure
    char *antenna_filepath = CVECTOR( strlen(wisim_home) + strlen("/antenna/") );
    sprintf(antenna_filepath, "%s%cantenna%c", wisim_home, FPATH_SEPARATOR, FPATH_SEPARATOR);

  DBClass TempDB;
  char* errmsg = CVECTOR(255);

  if (preferences->selected_db == -1)
  {
      sprintf(msg, "ERROR: Not connected to database\n");
      PRMSG(stdout, msg);
      error_state = 1;
      return;
  } else 

  {
      if (!TempDB.connectDB(preferences->db_list[preferences->selected_db]->name,
                   preferences->db_list[preferences->selected_db]->user,
                   preferences->db_list[preferences->selected_db]->password,
                   errmsg))
           
      {
          sprintf(msg, errmsg);
          PRMSG(stdout, msg);
          error_state = 1;
          return;
      }
   }  

  /************read from table GEOM_GENERAL ***********/
  
  char * select_GEOM = (char *)"select * from GEOM_GENERAL";
  //char ** projname;
  char ** coordinate_sys,*str1;
  //projname = (char**)malloc(sizeof(char*));
  //projname[0] = CVECTOR(21);
  coordinate_sys = (char**)malloc(sizeof(char*));
  coordinate_sys[0] = CVECTOR(61);

  //readString( envhp, errhp, svchp, srvhp, select_GEOM, projname,           1, 21);
  TempDB.readString( select_GEOM, coordinate_sys,     4, 61);
  TempDB.readDouble( select_GEOM, &resolution,        5);
  TempDB.readInt   ( select_GEOM, &num_freq,          6);
  TempDB.readInt   ( select_GEOM, &num_slot,          7);
  TempDB.readInt   ( select_GEOM, &num_cntl_chan_slot,8);
  TempDB.readInt   ( select_GEOM, &cntl_chan_freq,    9); 
  TempDB.readInt   ( select_GEOM, &num_cell,          10);
  str1 = strdup(coordinate_sys[0]);
  res_digits = ( (resolution < 1.0) ? ((int) -floor(log(resolution)/log(10.0))) : 0 );

  if (strncmp(str1, "GENERIC",7)==0) {
       coordinate_system = CConst::CoordGeneric;
  } else if (strncmp(str1, "UTM:", 4)==0) {
       coordinate_system = CConst::CoordUTM;
       str1  = strtok((char*)(str1+4), ":");
       utm_equatorial_radius = atof(str1);
       str1 = strtok(NULL, ":");
       utm_eccentricity_sq = atof(str1);
       str1 = strtok(NULL, ":");
       utm_zone = atoi(str1);
       str1 = strtok(NULL, ":");
       if (strcmp(str1, "N")==0) {
           utm_north = 1;
       } else if (strcmp(str1, "S")==0) {
           utm_north = 0;
       } 
  }
  bit_cell = -1;
  select_GEOM = (char*)NULL;
  free(select_GEOM);
  /***************read from table GEOM_GENERAL *************/


  
  /***************read from table CELL *********************/

  char * select_CELL = (char *)"select * from CELL";
  //CellClass **cell_list;
  CellClass *cell;
  int *cell_num,*num_sector,*cell_color;
  double *posx,*posy;
  double *posn_x,*posn_y;
  int cell_idx,i;
  char ** cell_name;
  int index;
  cell_list  = (CellClass**)malloc(num_cell*sizeof(CellClass*));
  cell_num   = IVECTOR(num_cell);
  num_sector = IVECTOR(num_cell);
  cell_color = IVECTOR(num_cell);
  //cell_flag  = (int *)malloc(num_cell*sizeof(int));
  posx = DVECTOR(num_cell);
  posy = DVECTOR(num_cell);
  posn_x = DVECTOR(num_cell);
  posn_y = DVECTOR(num_cell);
  cell_name = (char**)malloc(num_cell*sizeof(char*));
  for ( cell_idx=0; cell_idx<num_cell; cell_idx++)
      cell_name[cell_idx] = CVECTOR(20); 
  TempDB.readInt   ( select_CELL, cell_num,          2);
  TempDB.readString( select_CELL, cell_name,         3,20);
  TempDB.readInt   ( select_CELL, cell_color,        4);
  TempDB.readDouble( select_CELL, posx,              5);
  TempDB.readDouble( select_CELL, posy,              6);
  TempDB.readInt   ( select_CELL, num_sector,        7);
//readInt   ( envhp, errhp, svchp, srvhp, select_CELL, cell_flag,         8);

  for (  cell_idx=0; cell_idx<num_cell; cell_idx++ )
  {
          i = 0;
	  cell_list[cell_num[cell_idx]] = new CellClass();
          cell = cell_list[cell_num[cell_idx]];
          if (strcmp(cell_name[cell_idx], "") == 0)
              cell->strid = NULL;      
          else
              cell->strid = strdup(cell_name[cell_idx]);
          cell->color = cell_color[cell_idx];
          cell->num_sector = num_sector[cell_idx];
	  posn_x[cell_num[cell_idx]] = posx[cell_idx];
	  posn_y[cell_num[cell_idx]] = posy[cell_idx];
          cell->sector_list= (SectorClass**)malloc(num_sector[cell_idx]*sizeof(SectorClass*));
  }
  select_CELL = (char*)NULL;
  free(select_CELL);
  free(cell_num);
 /***************read from table CELL **********************/



  /****************read from table CELL_SECTOR_GENERAL*******/

  char * select_CELL_SECTOR_GENERAL = (char *)"select * from CELL_SECTOR_GENERAL";
  int *sector_num,*cs_num,*antenna_typ,*num_physical_tx,sector_idx;
  double *antenna_deg,*antenna_height,*tx_power;
  char **sector_name, **comments;
  char **csid;
  int sector_total = 0;
 // double *comm_traffic, *lreg_traffic;
  PHSSectorClass *sector;
   
  total_num_sectors = 0;
  
  for ( cell_idx=0; cell_idx<num_cell; cell_idx++)
  {
    cell = cell_list[cell_idx];
    //cell->sector_list = (SectorClass **) malloc((cell->num_sector)*sizeof(SectorClass *));
    for (sector_idx=0; sector_idx<num_sector[cell_idx]; sector_idx++)
	  sector_total++; 
  }

  sector_name = (char**)malloc(sector_total*sizeof(char*));
  csid        = (char**)malloc(sector_total*sizeof(char*));
  comments    = (char**)malloc(sector_total*sizeof(char*));
  for ( index=0; index<sector_total; index++)
  {
	  sector_name[index] = CVECTOR(30);
	  sector_name[index][30] = (char) NULL;
          //csid[index]        = CVECTOR(2*csid_byte_length);
          //csid[index][2*csid_byte_length] = (char) NULL;
          csid[index]       =  CVECTOR(20);  
	  comments[index]    = CVECTOR(60); // (char *)malloc(15*sizeof(char));
	  comments[index][60] = (char) NULL;
  }
  antenna_deg    = DVECTOR(sector_total);
  antenna_height = DVECTOR(sector_total);
  tx_power       = DVECTOR(sector_total);

  cell_num       = IVECTOR(sector_total);
  sector_num     = IVECTOR(sector_total);
  cs_num         = IVECTOR(sector_total);
  antenna_typ    = IVECTOR(sector_total);  
  num_physical_tx= IVECTOR(sector_total);
  
//  comm_traffic   = DVECTOR(sector_total);
//  lreg_traffic   = DVECTOR(sector_total);

  TempDB.readInt   ( select_CELL_SECTOR_GENERAL, cell_num      , 2);
  TempDB.readInt   ( select_CELL_SECTOR_GENERAL, sector_num    , 3);
//TempDB.readString( select_CELL_SECTOR_GENERAL, sector_name   , 4, 30);
  TempDB.readString( select_CELL_SECTOR_GENERAL, comments   ,    5, 60);
  TempDB.readString( select_CELL_SECTOR_GENERAL, csid,           6, 20);
  TempDB.readInt   ( select_CELL_SECTOR_GENERAL, cs_num        , 7);
//TempDB.readDouble( select_CELL_SECTOR_GENERAL, comm_traffic  , 8);
//TempDB.readDouble( select_CELL_SECTOR_GENERAL, lreg_traffic  , 9);
  TempDB.readDouble( select_CELL_SECTOR_GENERAL, antenna_deg   , 8);
  TempDB.readInt   ( select_CELL_SECTOR_GENERAL, antenna_typ    ,9);
  TempDB.readDouble( select_CELL_SECTOR_GENERAL, antenna_height,10);
  TempDB.readDouble( select_CELL_SECTOR_GENERAL, tx_power      ,11);
  TempDB.readInt   ( select_CELL_SECTOR_GENERAL, num_physical_tx,12);

 
  index = 0;
  
  for ( cell_idx=0; cell_idx<num_cell; cell_idx++)
  {
      for (sector_idx=0; sector_idx<num_sector[cell_idx]; sector_idx++)
      {
          cell = cell_list[cell_num[index]];
          cell->sector_list[sector_num[index]] = (SectorClass *) new PHSSectorClass(cell);
	  sector          = (PHSSectorClass *) cell->sector_list[sector_num[index]];
          sector->comment = strdup(comments[index]);
          if (strcmp(csid[index],"") != 0 )
          {
             sector->csid_hex = (unsigned char *) malloc(20);
             hexstr_to_hex(sector->csid_hex, csid[index], sector->csid_byte_length);
          }
          else
             sector->csid_hex = NULL;
          sector->gw_csc_cs                           = cs_num[index];
	  sector->antenna_angle_rad                   = antenna_deg[index]*PI/180.0;
	  sector->antenna_type                        = antenna_typ[index];
	  sector->antenna_height                      = antenna_height[index];
	  sector->tx_pwr                              = tx_power[index];
	  sector->num_physical_tx                     = num_physical_tx[index];
          //sector->comm_traffic                        = comm_traffic[index];
	  //sector->lreg_traffic                        = lreg_traffic[index];
	  index++;
	}
  }    
  select_CELL_SECTOR_GENERAL = (char*)NULL;
  free(select_CELL_SECTOR_GENERAL);
  free(sector_num);
  free(cell_num);
  /******************read from table CELL_SECTOR_GENERAL************************/



  /*******************read from table GEOM_CSM **********************************/

  char * select_GEOM_CSM = (char *)"select * from GEOM_CSM";
  int num_sys_bdy_pt,num_subnet_0,num_subnet_1, num_traff;

  TempDB.readInt   ( select_GEOM_CSM,  &num_sys_bdy_pt,  2);
//TempDB.readInt   ( select_GEOM_CSM,  &num_comm_subnet, 3);
//TempDB.readInt   ( select_GEOM_CSM,  &num_lreg_subnet, 4);
  TempDB.readInt   ( select_GEOM_CSM,  &num_traffic_type,3);
  TempDB.readInt   ( select_GEOM_CSM,  &num_subnet_0,    4);
  TempDB.readInt   ( select_GEOM_CSM,  &num_subnet_1,    5);
  TempDB.readInt   ( select_GEOM_CSM,  &num_antenna_type,6);
  TempDB.readInt   ( select_GEOM_CSM,  &num_prop_model,  7);
  TempDB.readInt   ( select_GEOM_CSM,  &num_traff,       8);
  if ( num_traffic_type) {
      traffic_type_list = (TrafficTypeClass**)malloc((num_traffic_type)*sizeof(TrafficTypeClass*));
      num_subnet        = IVECTOR(num_traffic_type);
      subnet_list       = (SubnetClass ***)malloc((num_traffic_type)*sizeof(SubnetClass**));
  }
  if (num_traffic_type == 2)
  {
      num_subnet[0] = num_subnet_0;
      num_subnet[1] = num_subnet_1;
  }  else
  {
      num_subnet[0] = num_subnet_0;
  }     
  SectorClass::num_traffic = num_traff;
  if (SectorClass::num_traffic) {
      SectorClass::traffic_type_idx_list = IVECTOR(SectorClass::num_traffic);
  }
  for (i = 0; i<SectorClass::num_traffic; i++)
  {
      SectorClass::traffic_type_idx_list[i] = i;
  }
  select_GEOM_CSM = (char*)NULL;
  free(select_GEOM_CSM);

  /*******************read from table GEOM_CSM ***********************************/

  
  /***************read from MEAS_CTR_LIST *******************/

   char * select_MEAS_CTR_LIST  = (char *)"select * from MEAS_CTR_LIST";
   int  num_traffic_list = 0;
   int *traffic_num;
   double *traffic_list;
   for ( cell_idx=0; cell_idx<num_cell; cell_idx++)
   {
     cell = cell_list[cell_idx];
     for (sector_idx=0; sector_idx<num_sector[cell_idx]; sector_idx++)
     {
          sector = (PHSSectorClass*)cell->sector_list[sector_idx];
          num_traffic_list += SectorClass::num_traffic;
          sector->meas_ctr_list = (double*)malloc((SectorClass::num_traffic) * sizeof(double));
     }
   }

   cell_num    = IVECTOR(num_traffic_list);
   sector_num  = IVECTOR(num_traffic_list);
   traffic_num = IVECTOR(num_traffic_list);
   traffic_list= DVECTOR(num_traffic_list);

   TempDB.readInt   ( select_MEAS_CTR_LIST, cell_num,          2);
   TempDB.readInt   ( select_MEAS_CTR_LIST, sector_num,        3);
   TempDB.readInt   ( select_MEAS_CTR_LIST, traffic_num,       4);
   TempDB.readDouble( select_MEAS_CTR_LIST, traffic_list,      5);

   for (index =0; index < num_traffic_list; index++)
   {
        cell = cell_list[cell_num[index]];
        sector = (PHSSectorClass *) cell->sector_list[sector_num[index]];
        sector->meas_ctr_list[traffic_num[index]] = traffic_list[index];
   }
   select_MEAS_CTR_LIST = (char*)NULL;
   free(select_MEAS_CTR_LIST);
   free(cell_num); 
   free(sector_num);
   free(traffic_num); 
 /***************read from MEAS_CTR_LIST *******************/  



  /*******************read from table TRAFFIC_TYPE*******************************/

  char * select_TRAFFIC_TYPE = (char *)"select * from TRAFFIC_TYPE";
  int traffic_index;
  int *color ;
  double *mean_time,*min_time,*max_time;
  char **traffic_typ, **duration_dist;  
  TrafficTypeClass *traffic_type = (TrafficTypeClass*)NULL; 
 
  //arrival_rate = DVECTOR(num_traffic_type);
  color        = IVECTOR(num_traffic_type);
  traffic_num  = IVECTOR(num_traffic_type);
  mean_time    = DVECTOR(num_traffic_type);
  min_time     = DVECTOR(num_traffic_type);
  max_time     = DVECTOR(num_traffic_type);
  traffic_typ  = (char** )malloc(num_traffic_type*sizeof(char* ));
  duration_dist= (char** )malloc(num_traffic_type*sizeof(char* ));
  for (traffic_index=0; traffic_index<num_traffic_type; traffic_index++)
  {
      traffic_typ  [traffic_index] = CVECTOR(20);
      duration_dist[traffic_index] = CVECTOR(20);
  }   

  TempDB.readInt    ( select_TRAFFIC_TYPE, traffic_num,  2);
  TempDB.readString ( select_TRAFFIC_TYPE, traffic_typ,  3,20);
  TempDB.readInt    ( select_TRAFFIC_TYPE, color,        4);
  TempDB.readString ( select_TRAFFIC_TYPE, duration_dist,5,20);
  TempDB.readDouble ( select_TRAFFIC_TYPE, mean_time,    6);
  TempDB.readDouble ( select_TRAFFIC_TYPE, min_time,     7);
  TempDB.readDouble ( select_TRAFFIC_TYPE, max_time,     8);

  for (traffic_index=0; traffic_index<num_traffic_type; traffic_index++)
  {
      traffic_type_list[traffic_num[traffic_index]] = new TrafficTypeClass(traffic_typ[traffic_index]);
      //traffic_type_list[traffic_num[traffic_index]] = (TrafficTypeClass*)malloc(sizeof(TrafficTypeClass));
      traffic_type        = traffic_type_list[traffic_num[traffic_index]];
      traffic_type->strid = CVECTOR(20);
      strcpy(traffic_type->strid, traffic_typ[traffic_index]);
      //traffic_type->arrival_rate  = arrival_rate[traffic_index];
      traffic_type->color         = color[traffic_index];
      if ( strcmp(duration_dist[traffic_index],"EXPONENTIAL") == 0 )
      {
           traffic_type->duration_dist = CConst::ExpoDist;
           traffic_type->mean_time     = mean_time[traffic_index];
      } 
      else if ( strcmp(duration_dist[traffic_index],"UNIFORM") == 0 )
      {
           traffic_type->duration_dist = CConst::UnifDist;
           traffic_type->min_time      = min_time[traffic_index];
           traffic_type->max_time      = max_time[traffic_index];
      }
  }
  select_TRAFFIC_TYPE = (char*)NULL;
  free(select_TRAFFIC_TYPE);
  free(color);    
  /*******************read from table TRAFFIC_TYPE*******************************/




 /*******************read from table SUBNET ********************************/

  char * select_SUBNET = (char *)"select * from SUBNET";
  SubnetClass *subnet;
  int num_subnet_total = 0;
  int subnet_idx;

  for ( traffic_index = 0; traffic_index<num_traffic_type; traffic_index++)
  {
      subnet_list[traffic_index] = 
           (SubnetClass**)malloc((num_subnet[traffic_index])*sizeof(SubnetClass*));
      for (subnet_idx = 0; subnet_idx<num_subnet[traffic_index]; subnet_idx++)
      {
            subnet_list[traffic_index][subnet_idx] = new SubnetClass();
            subnet    = subnet_list[traffic_index][subnet_idx];
            subnet->p = new PolygonClass();
      }
      num_subnet_total += num_subnet[traffic_index];
  }

  int *num_segment;
  double *arrival_rate;
  int *sub_num;
  char **sub_name;
  int *sub_type;

  sub_type    = IVECTOR(num_subnet_total);
  sub_name    = (char**)malloc(num_subnet_total*sizeof(char*));
  sub_num     = IVECTOR(num_subnet_total);
  color       = IVECTOR(num_subnet_total);
  num_segment = IVECTOR(num_subnet_total);
  arrival_rate= DVECTOR(num_subnet_total);
  
  for (subnet_idx=0; subnet_idx<num_subnet_total; subnet_idx++)
           sub_name[subnet_idx] = CVECTOR(20);   

  TempDB.readInt    ( select_SUBNET, sub_type,   2);
  TempDB.readInt    ( select_SUBNET, sub_num,    3);
  TempDB.readString ( select_SUBNET, sub_name,   4,20);
  TempDB.readDouble ( select_SUBNET, arrival_rate,6);
  TempDB.readInt    ( select_SUBNET, color,      7);
  TempDB.readInt    ( select_SUBNET, num_segment,8);

  for (subnet_idx=0; subnet_idx<num_subnet_total; subnet_idx++)
  {
          i = 0;
          subnet                 = subnet_list[sub_type[subnet_idx]][sub_num[subnet_idx]];
          //while (sub_name[subnet_idx][i] != ' ')  i++;
          //subnet->strid          = CVECTOR(i);
          //for ( index =0; index<i; index++)
          //  subnet->strid[index] = sub_name[subnet_idx][index];
          subnet->strid          = strdup(sub_name[subnet_idx]);
          subnet->arrival_rate   = arrival_rate[subnet_idx];
          subnet->color          = color[subnet_idx];
          subnet->p->num_segment = num_segment[subnet_idx];
  }
  select_SUBNET = (char*)NULL;
  free(select_SUBNET);
  free(sub_num);
  free(sub_type);
  subnet = (SubnetClass*)NULL;
  free(subnet);
  /*******************read from table COMM_SUBNET ********************************/


/*******************read from table SEGMENT *******************************/

  char * select_SEGMENT = (char *)"select * from SEGMENT";
  int *num_bdy_pt,num_pt=0;
  int *segment_num;
  
  for ( traffic_index = 0; traffic_index<num_traffic_type; traffic_index++)
  {
      for (subnet_idx = 0; subnet_idx<num_subnet[traffic_index]; subnet_idx++)
      {
           subnet    = subnet_list[traffic_index][subnet_idx];
           subnet->p->num_bdy_pt = IVECTOR(subnet->p->num_segment);
           num_pt   += subnet->p->num_segment;
      }
  }

  num_bdy_pt = IVECTOR(num_pt);
  segment_num= IVECTOR(num_pt);
  sub_num    = IVECTOR(num_pt);
  sub_type   = IVECTOR(num_pt);

  TempDB.readInt  ( select_SEGMENT, sub_type,      2);
  TempDB.readInt  ( select_SEGMENT, sub_num,       3);
  TempDB.readInt  ( select_SEGMENT, segment_num,   4);
  TempDB.readInt  ( select_SEGMENT, num_bdy_pt,    6);

  for (i=0; i<num_pt; i++)
  {
          subnet  = subnet_list[sub_type[i]][sub_num[i]];
          subnet->p->num_bdy_pt[segment_num[i]] = num_bdy_pt[i];
  }
  select_SEGMENT = (char*)NULL;
  free(select_SEGMENT);
  free(sub_num);  
  free(sub_type);
  free(segment_num);
  /*******************read from table SEGMENT *******************************/


  /*******************read from table SYS_BDY_PT *********************************/

  char * select_SYS_BDY_PT = (char *)"select * from SYS_BDY_PT";
  double *x_crdnate, *y_crdnate;
  //PolygonClass *system_bdy;
  //int i;
  int maxx,maxy,bdy_pt_idx;
  int *pt_num;

  x_crdnate = DVECTOR(num_sys_bdy_pt); 
  y_crdnate = DVECTOR(num_sys_bdy_pt);

  system_bdy = new PolygonClass();
  system_bdy->num_segment = 1;
  system_bdy->num_bdy_pt = IVECTOR(system_bdy->num_segment);
  system_bdy->bdy_pt_x   = (int**)malloc(system_bdy->num_segment*sizeof(int*));
  system_bdy->bdy_pt_y   = (int**)malloc(system_bdy->num_segment*sizeof(int*)); 
  system_bdy->num_bdy_pt[0] = num_sys_bdy_pt;  

  system_bdy->bdy_pt_x[0] = IVECTOR(num_sys_bdy_pt);
  system_bdy->bdy_pt_y[0] = IVECTOR(num_sys_bdy_pt);
  pt_num                  = IVECTOR(num_sys_bdy_pt);
    
  TempDB.readInt    ( select_SYS_BDY_PT, pt_num   ,  2);
  TempDB.readDouble ( select_SYS_BDY_PT, x_crdnate,  3);
  TempDB.readDouble ( select_SYS_BDY_PT, y_crdnate,  4);

  for (i=0; i<num_sys_bdy_pt; i++)
  {
          check_grid_val(x_crdnate[i], resolution, 0, &system_bdy->bdy_pt_x[0][pt_num[i]]);
          check_grid_val(y_crdnate[i], resolution, 0, &system_bdy->bdy_pt_y[0][pt_num[i]]);
  }
  system_bdy->comp_bdy_min_max(system_startx, maxx, system_starty, maxy);
  npts_x = maxx - system_startx + 1;
  npts_y = maxy - system_starty + 1;
  for (bdy_pt_idx=0; bdy_pt_idx<=system_bdy->num_bdy_pt[0]-1; bdy_pt_idx++) {
       system_bdy->bdy_pt_x[0][bdy_pt_idx] -= system_startx;
       system_bdy->bdy_pt_y[0][bdy_pt_idx] -= system_starty;
  }
  
  double system_area = system_bdy->comp_bdy_area();
  if (num_cell) {
      avg_cell_radius = sqrt( system_area/( PI * (num_cell)) );
      rec_avg_cell_radius = 1.0 / avg_cell_radius;
  }

  for (cell_idx=0; cell_idx<num_cell; cell_idx++)
  {
       cell = cell_list[cell_idx];  
       check_grid_val(posn_x[cell_idx], resolution, system_startx, &cell->posn_x);
       check_grid_val(posn_y[cell_idx], resolution, system_starty, &cell->posn_y);
   }
  select_SYS_BDY_PT = (char*)NULL;
  free(select_SYS_BDY_PT);
  free(pt_num);
  free(x_crdnate);
  free(y_crdnate);
  /*******************read from table SYS_BDY_PT *********************************/




 /*******************read from table SEG_BDY_PT *********************************/

  char * select_SEG_BDY_PT = (char *)"select * from SEG_BDY_PT";
  //char **sub_type;
  //int subnet_idx;
  
  int num_xy = 0;
  //int segment_comm_idx=0, bdy_comm_pt_idx=0; 
  //int segment_lreg_idx=0, bdy_lreg_pt_idx=0;
  int segment_idx = 0;
  for (traffic_index=0; traffic_index<num_traffic_type; traffic_index++)
  {
          for (subnet_idx=0; subnet_idx<num_subnet[traffic_index];subnet_idx++)
          {
              subnet = subnet_list[traffic_index][subnet_idx];
              subnet->p->bdy_pt_x = (int **)malloc(subnet->p->num_segment*sizeof(int*));
              subnet->p->bdy_pt_y = (int **)malloc(subnet->p->num_segment*sizeof(int*));
              for (index=0; index<subnet->p->num_segment; index++)
              {
                   subnet->p->bdy_pt_x[index] = IVECTOR(subnet->p->num_bdy_pt[index]);
                   subnet->p->bdy_pt_y[index] = IVECTOR(subnet->p->num_bdy_pt[index]);
              }
              for ( int i=0; i<subnet->p->num_segment; i++ )
                  for (int j=0; j<subnet->p->num_bdy_pt[i]; j++)
                  {   
                     num_xy++;
                  }
          }
  }  
             
  sub_type  = IVECTOR(num_xy);
  x_crdnate = DVECTOR(num_xy);
  y_crdnate = DVECTOR(num_xy);
  sub_num   = IVECTOR(num_xy);
  segment_num=IVECTOR(num_xy);
  pt_num    = IVECTOR(num_xy);


  TempDB.readInt   ( select_SEG_BDY_PT, sub_type,    2);
  TempDB.readInt   ( select_SEG_BDY_PT, sub_num,     3);
  TempDB.readInt   ( select_SEG_BDY_PT, segment_num, 4);
  TempDB.readInt   ( select_SEG_BDY_PT, pt_num,      5);
  TempDB.readDouble( select_SEG_BDY_PT, x_crdnate,   6);
  TempDB.readDouble( select_SEG_BDY_PT, y_crdnate,   7);

 
  for (index=0; index<num_xy; index++)
  {
       subnet = subnet_list[sub_type[index]][sub_num[index]];
       check_grid_val(x_crdnate[index], resolution, system_startx,
                     &(subnet->p->bdy_pt_x[segment_num[index]][pt_num[index]]));
       check_grid_val(y_crdnate[index], resolution, system_starty, 
                     &(subnet->p->bdy_pt_y[segment_num[index]][pt_num[index]]));
   }

  for (traffic_index=0; traffic_index<num_traffic_type; traffic_index++) 
        for (subnet_idx=0; subnet_idx<num_subnet[traffic_index]; subnet_idx++)     
        {
            subnet = subnet_list[traffic_index][subnet_idx];
            for (segment_idx=0; segment_idx<=subnet->p->num_segment-1; segment_idx++) {
               for (bdy_pt_idx=0; bdy_pt_idx<=subnet->p->num_bdy_pt[segment_idx]-1; bdy_pt_idx++) {
                   system_bdy->in_bdy_area(subnet->p->bdy_pt_x[segment_idx][bdy_pt_idx], subnet->p->bdy_pt_y[segment_idx][bdy_pt_idx]);
               }
            }
         }
  

  for (traffic_index=0; traffic_index<num_traffic_type; traffic_index++) 
        for (subnet_idx=0; subnet_idx<num_subnet[traffic_index]; subnet_idx++)
        {
            subnet = subnet_list[traffic_index][subnet_idx];
            subnet->p->comp_bdy_min_max(subnet->minx, subnet->maxx, subnet->miny, subnet->maxy);
        }

  select_SEG_BDY_PT = (char*)NULL;
  free(select_SEG_BDY_PT);
   
  /************************read from table SEG_BDY_PT ***********************/


  /************************read from table CELL_SECTOR_CSM*******************/

  char * select_CELL_SECTOR_CSM = (char *)"select * from CELL_SECTOR_CSM";
  int *prop_model, *has_access_control, *cntl_chan_slot, *num_unused_freq;
  index = 0;
  for ( cell_idx=0; cell_idx<num_cell; cell_idx++ )
    for (sector_idx=0; sector_idx<num_sector[cell_idx]; sector_idx++)
	{
	  index++;
	}
  prop_model         = IVECTOR(index);
  has_access_control = IVECTOR(index);
  cntl_chan_slot     = IVECTOR(index);
  num_unused_freq    = IVECTOR(index);
  cell_num           = IVECTOR(index);
  sector_num         = IVECTOR(index);
  
  TempDB.readInt ( select_CELL_SECTOR_CSM, cell_num,             2);
  TempDB.readInt ( select_CELL_SECTOR_CSM, sector_num,           3);
  TempDB.readInt ( select_CELL_SECTOR_CSM, prop_model,           5);
  TempDB.readInt ( select_CELL_SECTOR_CSM, has_access_control,   6);
  TempDB.readInt ( select_CELL_SECTOR_CSM, cntl_chan_slot    ,   7);
  TempDB.readInt ( select_CELL_SECTOR_CSM, num_unused_freq,      8);
  
  for ( i=0; i<index; i++ )
  {
          sector = (PHSSectorClass*)cell_list[cell_num[i]]->sector_list[sector_num[i]];
          sector->prop_model         = prop_model[i];
          sector->has_access_control = has_access_control[i];
          sector->cntl_chan_slot     = cntl_chan_slot[i];
          sector->num_unused_freq    = num_unused_freq[i];
          sector->cntl_chan_eff_tch_slot = 
              ((sector->cntl_chan_slot == -1) ? -1 : sector->cntl_chan_slot % num_slot);
	  sector->stat_count         = (StatCountClass *) NULL;
          sector->num_road_test_pt   = 0;
          sector->sync_level         = -1;
          sector->num_call           = 0;
          sector->call_list          = (CallClass **) NULL;
          sector->active             = 1;
   }
  select_CELL_SECTOR_CSM = (char*)NULL;
  free(select_CELL_SECTOR_CSM);
  free(sector_num);
  free(cell_num);
  /************************read from table CELL_SECTOR_CSM*******************/



  /************************read from table CELL_UNUSED_FREQ******************/

  char * select_CELL_UNUSED_FREQ = (char *)"select * from CELL_UNUSED_FREQ";
  int *unused_freq_value, *unused_freq_num;
  index=0;

  for ( cell_idx=0; cell_idx<num_cell; cell_idx++ )
    for (sector_idx=0; sector_idx<num_sector[cell_idx]; sector_idx++)
	{
	   sector = (PHSSectorClass*)cell_list[cell_idx]->sector_list[sector_idx];
	   sector->unused_freq = (int*)malloc(sector->num_unused_freq * sizeof(int));
	   for ( int i=0; i<sector->num_unused_freq; i++ )
		   index++;
	}
  cell_num          = IVECTOR(index);
  sector_num        = IVECTOR(index);
  unused_freq_value = IVECTOR(index);
  unused_freq_num   = IVECTOR(index);

  TempDB.readInt ( select_CELL_UNUSED_FREQ, cell_num,           2);
  TempDB.readInt ( select_CELL_UNUSED_FREQ, sector_num,         3);
  TempDB.readInt ( select_CELL_UNUSED_FREQ, unused_freq_num,    4);
  TempDB.readInt ( select_CELL_UNUSED_FREQ, unused_freq_value,  6);

  for (i=0; i<index; i++)
  {
        sector = (PHSSectorClass*)cell_list[cell_num[i]]->sector_list[sector_num[i]];
        sector->unused_freq[unused_freq_num[i]] = unused_freq_value[i];
  }
     
  for (cell_idx=0; cell_idx<=num_cell-1; cell_idx++) {
        cell = cell_list[cell_idx];
        system_bdy->in_bdy_area(cell->posn_x, cell->posn_y);
  }
  select_CELL_UNUSED_FREQ = (char*)NULL;
  free(select_CELL_UNUSED_FREQ);
  /*******************************read from CELL_UNUSED_FREQ ***********************/



  /*******************************read from	PROP_MODEL *****************************/
  
  char * select_PROP_MODEL = (char *)"select * from PROP_MODEL";
  double *prop_y, *prop_py, *prop_s1, *prop_s2;
  double *prop_y0,*prop_ys, *prop_k0, *prop_k1;
  double *exponent,  *coefficient;
  int  *useheight, *num_clutter_type;
  int *prop_num;
  char **prop_type,**strid;
  //PropModelClass **prop_model_list;

  exponent    = DVECTOR(num_prop_model);  
  coefficient = DVECTOR(num_prop_model);
  prop_y0 = DVECTOR(num_prop_model);
  prop_ys = DVECTOR(num_prop_model);
  prop_k0 = DVECTOR(num_prop_model);   
  prop_k1 = DVECTOR(num_prop_model);
  prop_y  = DVECTOR(num_prop_model);
  prop_py = DVECTOR(num_prop_model);
  prop_s1 = DVECTOR(num_prop_model);
  prop_s2 = DVECTOR(num_prop_model);
  useheight       = IVECTOR(num_prop_model);
  num_clutter_type = IVECTOR(num_prop_model);
  prop_num         = IVECTOR(num_prop_model);
  prop_type = (char **)malloc(num_prop_model*sizeof(char*));
  strid     = (char **)malloc(num_prop_model*sizeof(char*));
  for (i=0; i<num_prop_model; i++)
  {
	  prop_type[i] = CVECTOR(20);
          strid[i]     = CVECTOR(20);
  }
  prop_model_list = (PropModelClass **) malloc((num_prop_model)*sizeof(PropModelClass *));
  //for (i=0; i<num_prop_model; i++)
  //	  prop_model_list[i] = (PropModelClass*)malloc(PropModelClass);

  TempDB.readInt     ( select_PROP_MODEL, prop_num,          2);
  TempDB.readString  ( select_PROP_MODEL, prop_type,         3, 20);
  TempDB.readString  ( select_PROP_MODEL, strid,             4, 20);
  TempDB.readDouble  ( select_PROP_MODEL, prop_y0,           5);
  TempDB.readDouble  ( select_PROP_MODEL, prop_ys,           6);
  TempDB.readDouble  ( select_PROP_MODEL, prop_k0,           7);
  TempDB.readDouble  ( select_PROP_MODEL, prop_k1,           8);
  TempDB.readDouble  ( select_PROP_MODEL, prop_y,            9);
  TempDB.readDouble  ( select_PROP_MODEL, prop_py,           10);
  TempDB.readDouble  ( select_PROP_MODEL, prop_s1,           11);
  TempDB.readDouble  ( select_PROP_MODEL, prop_s2,           12);
  TempDB.readInt     ( select_PROP_MODEL, useheight,        13);
  TempDB.readInt     ( select_PROP_MODEL, num_clutter_type,  14);
  TempDB.readDouble  ( select_PROP_MODEL, exponent,          15);
  TempDB.readDouble  ( select_PROP_MODEL, coefficient,       16);

  int num_trnprop_veck = 0;

  char *prop_model_strid;
  for (i=0; i<num_prop_model; i++)
  {
          if (strid[prop_num[i]]) {
              prop_model_strid = strdup(strid[prop_num[i]]);
          } else {
              prop_model_strid = (char *) NULL;
          }

	  if (strcmp(prop_type[prop_num[i]], "EXPONENTIAL")==0) {
           prop_model_list[prop_num[i]] = (PropModelClass *) new ExpoPropModelClass(prop_model_strid);
		   ExpoPropModelClass *expo_prop = ((ExpoPropModelClass *) prop_model_list[prop_num[i]]);
		   expo_prop->coefficient = coefficient[i];
		   expo_prop->exponent    = exponent[i];
                   expo_prop->strid       = strdup(strid[i]);
                   //expo_prop->type()      = CConst::PropExpo;
      } else if (strcmp(prop_type[prop_num[i]], "PW_LINEAR")==0) {
           prop_model_list[prop_num[i]] = (PropModelClass *) new PwLinPropModelClass(prop_model_strid);
		   PwLinPropModelClass *pwlin_prop = ((PwLinPropModelClass *) prop_model_list[prop_num[i]]);
		   pwlin_prop->py   = prop_py[i];
		   pwlin_prop->s1   = prop_s1[i];
		   pwlin_prop->y0   = prop_y0[i];
		   pwlin_prop->ys   = prop_ys[i];
		   pwlin_prop->k0   = prop_k0[i];
		   pwlin_prop->k1   = prop_k1[i];
                   pwlin_prop->strid= strdup(strid[i]);
                   //pwlin_prop->type()= CConst::PropPwLin;
      } else if (strcmp(prop_type[prop_num[i]], "TERRAIN")==0) {
           prop_model_list[prop_num[i]] = (PropModelClass *) new TerrainPropModelClass(map_clutter,prop_model_strid);
           TerrainPropModelClass *trn_prop = ((TerrainPropModelClass *) prop_model_list[prop_num[i]]);
                   trn_prop->val_y  = prop_y[i];
		   trn_prop->val_py = prop_py[i];
		   trn_prop->val_s1 = prop_s1[i];
		   trn_prop->val_s2 = prop_s2[i];
		   trn_prop->useheight        = useheight[i];
		   trn_prop->num_clutter_type = num_clutter_type[i];
                   trn_prop->strid  = strdup(strid[i]);
                   //trn_prop->type()           = CConst::PropTerrain;
           num_trnprop_veck += 2*useheight[i]+num_clutter_type[i];
	  } 
  }
  select_PROP_MODEL = (char*)NULL;
  free(select_PROP_MODEL);
  for (i=0; i<num_prop_model; i++)
     free(prop_type[i]);
  if (prop_type) free(prop_type);
  free(prop_num);
  /*****************************read from table PROP_MODEL*********************/



  /*****************************read from table TRN_PROP_VECK******************/
  
  char * select_TRN_PROP_VECK = (char *)"select * from TRN_PROP_VECK";

  int   j=0;
  double *prop_veck;
  int   *veck_num;
  
  prop_num  = IVECTOR(num_trnprop_veck);
  veck_num  = IVECTOR(num_trnprop_veck);
  prop_type = (char**)malloc(num_trnprop_veck*sizeof(char*));
  prop_veck = DVECTOR(num_trnprop_veck);
  for (i=0; i<num_trnprop_veck; i++)
	  prop_type[i] = CVECTOR(20);

  if (num_trnprop_veck != 0) {
      TempDB.readInt    ( select_TRN_PROP_VECK, prop_num,   2);
      TempDB.readInt    ( select_TRN_PROP_VECK, veck_num,   3);
      TempDB.readString ( select_TRN_PROP_VECK, prop_type,  4, 20);
      TempDB.readDouble ( select_TRN_PROP_VECK, prop_veck,  5);   
  }

  for (i=0; i<num_trnprop_veck; i++)
  {
      TerrainPropModelClass *trn_prop = ((TerrainPropModelClass *) prop_model_list[prop_num[i]]);
      int temp = 2*trn_prop->useheight+trn_prop->num_clutter_type;
      trn_prop->vec_k = DVECTOR(temp);
  } 

  i=0;
  if ( num_trnprop_veck >0 )
  {
     do {
        if (strcmp(prop_type[prop_num[i]], "TERRAIN")==0) 
        {
           TerrainPropModelClass *trn_prop = ((TerrainPropModelClass *) prop_model_list[prop_num[i]]);
           //int temp = 2*trn_prop->useheight+trn_prop->num_clutter_type;
           //trn_prop->vec_k = (double*)malloc(temp*sizeof(double));
           //for (j=0; j<temp; j++)
           //{
                trn_prop->vec_k[veck_num[i]] = prop_veck[i];  
                i++;
           //}	
     	 
        }
     }  while ( i < num_trnprop_veck );
  }
  select_TRN_PROP_VECK = (char*)NULL;
  free(select_TRN_PROP_VECK);
  /****************************read from table TRN_PROP_VECK**********************/

 
  
  /**********************************read from ANTENNA_TYPE*******************************/

  char * select_ANTENNA_TYPE = (char *)"select * from ANTENNA_TYPE";
  int *antenna_type_num;
  int antenna_type;
  char **antenna_type_str, **antenna_file;
  //AntennaClass **antenna_type_list;
  
  antenna_type_num = IVECTOR(num_antenna_type);
  antenna_type_str = (char**)malloc(num_antenna_type*sizeof(char *));
  antenna_file     = (char**)malloc(num_antenna_type*sizeof(char *));
  for (i=0; i<num_antenna_type; i++)
  {
	  antenna_type_str[i] = CVECTOR(10);
	  antenna_file[i]     = CVECTOR(101);
  }

  antenna_type_list = (AntennaClass **) malloc((num_antenna_type)*sizeof(AntennaClass *));
 // for (i=0; i<num_antenna_type; i++)
//	  antenna_type_list[i] = new AntennaClass();

  TempDB.readInt    ( select_ANTENNA_TYPE, antenna_type_num, 2);
  TempDB.readString ( select_ANTENNA_TYPE, antenna_type_str, 3, 10);
  TempDB.readString ( select_ANTENNA_TYPE, antenna_file    , 4, 101);

  for (i=0; i<num_antenna_type; i++) 
  {
      if (strcmp(antenna_type_str[i], "OMNI")==0) {
          strcpy(antenna_type_str[i],"OMNI");
          antenna_type = CConst::antennaOmni;
      } else if (strcmp(antenna_type_str[i], "LUT_H")==0) {
          antenna_type = CConst::antennaLUT_H;
      } else if (strcmp(antenna_type_str[i], "LUT_V")==0) {
          antenna_type = CConst::antennaLUT_V;
      } else if (strcmp(antenna_type_str[i], "LUT")==0) {
          strcpy(antenna_type_str[i],"LUT");
          antenna_type = CConst::antennaLUT;
      } 
      if (antenna_type == CConst::antennaOmni) {
          antenna_type_list[antenna_type_num[i]] = new AntennaClass(antenna_type, "OMNI");
      } else {
          antenna_type_list[antenna_type_num[i]] = new AntennaClass(antenna_type);
          if (strcmp(antenna_file[i],"") != 0) antenna_type_list[antenna_type_num[i]]->readFile(antenna_filepath, antenna_file[i]);
	  }
  }
  select_ANTENNA_TYPE = (char*)NULL;
  free(select_ANTENNA_TYPE);
  /*****************************read from table ANTENNA_TYPE***********************/

  /*****************************free memory***************************************/
  TempDB.disconnectDB();

  free(errmsg);
  //free(projname[0]);
  //if (projname) free(projname);
  //if (str1) free(str1);
  free(coordinate_sys[0]);
  if (coordinate_sys) free(coordinate_sys);
  
  for (i=0; i<num_cell; i++)
  {
     if (cell_name[i]) free(cell_name[i]);     
  }
  cell = (CellClass*)NULL;
  free(cell);
  if (cell_name) free(cell_name);
  free(cell_num);
  free(cell_color);
  free(num_sector);
  free(posx);
  free(posy);
  free(posn_x);
  free(posn_y);
  
  for (i=0; i<sector_total; i++)
  {
     free(sector_name[i]);
     free(csid[i]);
     free(comments[i]);
  }
  if (sector_name) free(sector_name);
  if (csid)        free(csid);
  if (comments)    free(comments);
  free(antenna_deg);
  free(antenna_height);
  free(tx_power);
  free(sector_num);
  free(cs_num);
  free(antenna_typ);
  free(num_physical_tx);
 
  free(traffic_num);
  free(traffic_list);
  
  free(color);
  free(arrival_rate);
  free(mean_time);
  free(min_time);
  free(max_time);
  for (i=0; i<num_traffic_type; i++)
  {
     free(traffic_typ[i]);
     free(duration_dist[i]);
  }  
  if (traffic_typ) free(traffic_typ);
  if (duration_dist) free(duration_dist);
  traffic_type = (TrafficTypeClass*)NULL;  
  free(traffic_type);
  subnet = (SubnetClass*)NULL;
  free(subnet);
  free(num_segment);
  //free(traffic);
  free(sub_num);

  free(sub_type);
  for (i=0; i<num_subnet_total; i++)
    free(sub_name[i]);
  if (sub_name) free(sub_name);

  free(num_bdy_pt);
  free(segment_num);

  free(x_crdnate);
  free(y_crdnate);
  free(pt_num);

  free(prop_model);
  free(has_access_control);
  free(cntl_chan_slot);
  free(num_unused_freq);

  free(unused_freq_value);
  free(unused_freq_num);

  free(prop_y);
  free(prop_py);
  free(prop_s1);
  free(prop_s2);
  free(prop_y0);
  free(prop_ys);
  free(prop_k0);
  free(prop_k1);
  free(exponent);
  free(coefficient);
  free(useheight);
  free(num_clutter_type);
  free(prop_num);
  free(prop_model_strid);
  free(prop_veck);
  free(veck_num);
  for (i=0; i<num_trnprop_veck; i++)
     free(prop_type[i]);
  for (i=0; i<num_prop_model; i++)
     free(strid[i]);
  if (prop_type) free(prop_type);
  if (strid)     free(strid);
  
  free(antenna_type_num);
  for (i=0; i<num_antenna_type; i++)
  {
    free(antenna_type_str[i]);
    free(antenna_file[i]);
  }
  if (antenna_type_str)  free(antenna_type_str);
  if (antenna_file)      free(antenna_file);  

  if (antenna_filepath)  free(antenna_filepath);

/***************************************free memory*****************************************/
 //free(antenna_filepath);
 
#endif
}

 
void PHSNetworkClass::print_geometry_db()
{
#if 0
// xxxxxxxxxxx Update for new Road Test Data structure

  char *csid_string;
  char *antenna_filename;

  csid_string = CVECTOR( 2*PHSSectorClass::csid_byte_length );

  DBClass TempDB;
  char *errmsg = CVECTOR(255);

  //PHSNetworkClass *np = this; // xxxxxxxxxx Temporary patch for IDX_TO_X()

  if (preferences->selected_db == -1)
  {
      sprintf(msg, "ERROR: Not connected to database\n");
      PRMSG(stdout, msg);
      error_state = 1;
      return;
  } else 
  {
     if (!TempDB.connectDB(preferences->db_list[preferences->selected_db]->name,
                   preferences->db_list[preferences->selected_db]->user,
                   preferences->db_list[preferences->selected_db]->password,
                   errmsg))

      {
          sprintf(msg, errmsg);
          PRMSG(stdout, msg);
          error_state = 1;
          return;
      }
  }

 /*******************************Clear Database************************/

  char * sqlstmt_GEOM_GENERAL = (char *) "delete from GEOM_GENERAL";
  TempDB.beginSQL();
  TempDB.processSQL(sqlstmt_GEOM_GENERAL);
  TempDB.endSQL();
  sqlstmt_GEOM_GENERAL = (char*)NULL;
  free(sqlstmt_GEOM_GENERAL);


  char *sqlstmt_CELL = (char*) "delete from CELL";
  TempDB.beginSQL();
  TempDB.processSQL(sqlstmt_CELL);
  TempDB.endSQL();
  sqlstmt_CELL = (char*)NULL;
  free(sqlstmt_CELL);

  char *sqlstmt_CELL_SECTOR_GENERAL = (char*) "delete from CELL_SECTOR_GENERAL";
  TempDB.beginSQL();
  TempDB.processSQL(sqlstmt_CELL_SECTOR_GENERAL);
  TempDB.endSQL();
  sqlstmt_CELL_SECTOR_GENERAL = (char*)NULL;
  free(sqlstmt_CELL_SECTOR_GENERAL);

  char *sqlstmt_GEOM_CSM = (char*) "delete from GEOM_CSM";
  TempDB.beginSQL();
  TempDB.processSQL(sqlstmt_GEOM_CSM);
  TempDB.endSQL();
  sqlstmt_GEOM_CSM = (char*)NULL;
  free(sqlstmt_GEOM_CSM);

  char *sqlstmt_MEAS_CTR_LIST = (char*) "delete from MEAS_CTR_LIST";
  TempDB.beginSQL();
  TempDB.processSQL(sqlstmt_MEAS_CTR_LIST);
  TempDB.endSQL();
  sqlstmt_MEAS_CTR_LIST = (char*)NULL;
  free(sqlstmt_MEAS_CTR_LIST);

  char *sqlstmt_TRAFFIC_TYPE = (char*) "delete from TRAFFIC_TYPE";
  TempDB.beginSQL();
  TempDB.processSQL(sqlstmt_TRAFFIC_TYPE);
  TempDB.endSQL();
  sqlstmt_TRAFFIC_TYPE = (char*)NULL;
  free(sqlstmt_TRAFFIC_TYPE);
  
  char *sqlstmt_SUBNET = (char*) "delete from SUBNET";
  TempDB.beginSQL();
  TempDB.processSQL(sqlstmt_SUBNET);
  TempDB.endSQL();
  sqlstmt_SUBNET = (char*)NULL;
  free(sqlstmt_SUBNET);

  char *sqlstmt_SEGMENT = (char*) "delete from SEGMENT";
  TempDB.beginSQL();
  TempDB.processSQL(sqlstmt_SEGMENT);
  TempDB.endSQL();
  sqlstmt_SEGMENT = (char*)NULL;
  free(sqlstmt_SEGMENT);

  char *sqlstmt_SYS_BDY_PT = (char*) "delete from SYS_BDY_PT";
  TempDB.beginSQL();
  TempDB.processSQL(sqlstmt_SYS_BDY_PT);
  TempDB.endSQL();
  sqlstmt_SYS_BDY_PT = (char*)NULL;
  free(sqlstmt_SYS_BDY_PT);
  
  char *sqlstmt_SEG_BDY_PT = (char*) "delete from SEG_BDY_PT";
  TempDB.beginSQL();
  TempDB.processSQL(sqlstmt_SEG_BDY_PT);
  TempDB.endSQL();
  sqlstmt_SEG_BDY_PT = (char *)NULL;
  free(sqlstmt_SEG_BDY_PT);

  char *sqlstmt_CELL_SECTOR_CSM = (char*) "delete from CELL_SECTOR_CSM";
  TempDB.beginSQL();
  TempDB.processSQL(sqlstmt_CELL_SECTOR_CSM);
  TempDB.endSQL();
  sqlstmt_CELL_SECTOR_CSM = (char*)NULL;
  free(sqlstmt_CELL_SECTOR_CSM);

  char *sqlstmt_CELL_UNUSED_FREQ = (char*) "delete from CELL_UNUSED_FREQ";
  TempDB.beginSQL();
  TempDB.processSQL(sqlstmt_CELL_UNUSED_FREQ);
  TempDB.endSQL();
  sqlstmt_CELL_UNUSED_FREQ = (char*)NULL;
  free(sqlstmt_CELL_UNUSED_FREQ);

  char *sqlstmt_PROP_MODEL = (char*) "delete from PROP_MODEL";
  TempDB.beginSQL();
  TempDB.processSQL(sqlstmt_PROP_MODEL);
  TempDB.endSQL();
  sqlstmt_PROP_MODEL = (char*)NULL;
  free(sqlstmt_PROP_MODEL);

  char *sqlstmt_TRN_PROP_VECK = (char*) "delete from TRN_PROP_VECK";
  TempDB.beginSQL();
  TempDB.processSQL(sqlstmt_TRN_PROP_VECK);
  TempDB.endSQL();
  sqlstmt_TRN_PROP_VECK = (char*)NULL;
  free(sqlstmt_TRN_PROP_VECK);

  char *sqlstmt_ANTENNA_TYPE  = (char*) "delete from ANTENNA_TYPE";
  TempDB.beginSQL();
  TempDB.processSQL(sqlstmt_ANTENNA_TYPE);
  TempDB.endSQL();
  sqlstmt_ANTENNA_TYPE = (char*)NULL;
  free(sqlstmt_ANTENNA_TYPE);

  /****************************Clear Database***************************/


  /*************************print table GEOM_GENERAL*********************/	

  char *insstmt_GEOM_GENERAL =
     (char *)"INSERT INTO GEOM_GENERAL (COORD_SYS, RESOLUTION, NUM_FREQ,"
	          "NUM_SLOT,NUM_CNTL_CHAN_SLOT,CNTL_CHAN_FREQ,NUM_CELL)" 
			  "values (:COORD_SYS,:RESOLUTION,:NUM_FREQ,:NUM_SLOT,:NUM_CNTL_CHAN_SLOT,"
			  ":CNTL_CHAN_FREQ,:NUM_CELL)";
  char *chptr;

  chptr = CVECTOR(60);

  if (coordinate_system == CConst::CoordGeneric) {
       sprintf(chptr,"GENERIC\n");
  } else if (coordinate_system == CConst::CoordUTM) {
       sprintf(chptr,"UTM:%.0f:%.9f:%d:%c:\n",
                utm_equatorial_radius,utm_eccentricity_sq,utm_zone,(utm_north?'N':'S'));
  }
  
  TempDB.beginSQL();
  TempDB.beginBind(insstmt_GEOM_GENERAL);
  TempDB.bindString(":COORD_SYS",chptr);
  TempDB.bindDouble(":RESOLUTION",&resolution);
  TempDB.bindInt   (":NUM_FREQ", &num_freq);
  TempDB.bindInt   (":NUM_SLOT", &num_slot);
  TempDB.bindInt   (":NUM_CNTL_CHAN_SLOT",&num_cntl_chan_slot);
  TempDB.bindInt   (":CNTL_CHAN_FREQ",&cntl_chan_freq);
  TempDB.bindInt   (":NUM_CELL", &num_cell);
  TempDB.execSQL();  
  TempDB.endBind();
  TempDB.endSQL();

   insstmt_GEOM_GENERAL = (char*)NULL;
   free(insstmt_GEOM_GENERAL);
   free(chptr);
  /******************************print table GEOM_GENERAL *************************/



  /******************************print table CELL *********************************/

  char *insstmt_CELL =
     (char *)"INSERT INTO CELL (CELL_NUM,CELL_NAME,COLOR,POSX,POSY,NUM_SECTOR)"
              "values (:CELL_NUM,:CELL_NAME,:COLOR,:POSX,:POSY,:NUM_SECTOR)";
 
  CellClass *cell;
  int index;
  int traffic_index, subnet_idx;
  double posx, posy;
  SubnetClass *subnet;

  for (traffic_index=0; traffic_index<num_traffic_type; traffic_index++)
    for (subnet_idx = 0; subnet_idx<num_subnet[traffic_index]; subnet_idx++)
    {
      subnet = subnet_list[traffic_index][subnet_idx];      
      for (index=0; index<num_cell; index++)
      {
          cell = cell_list[index];
          subnet->p->in_bdy_area(cell->posn_x,cell->posn_y);
      }
    }

  TempDB.beginSQL();
  TempDB.beginBind(insstmt_CELL);
  for (index=0; index<num_cell; index++)
  {
      cell = cell_list[index];
      posx = idx_to_x(cell->posn_x);
      posy = idx_to_y(cell->posn_y);

      TempDB.bindInt(":CELL_NUM",&index);
      if (cell->strid == NULL)
      {
          cell->strid=strdup("");
         //strcpy(cell->strid, "");
      }
      TempDB.bindString(":CELL_NAME", cell->strid);
      TempDB.bindInt   (":COLOR",    &cell->color);
      TempDB.bindDouble(":POSX",     &posx);
      TempDB.bindDouble(":POSY",     &posy);
      TempDB.bindInt   (":NUM_SECTOR",&cell->num_sector);
      TempDB.execSQL();
  }
  TempDB.endBind();
  TempDB.endSQL();

  insstmt_CELL = (char*)NULL;
  free(insstmt_CELL);

  /*******************************print table CELL*******************************/



  /*******************************print table CELL_SECOTR_GENERAL****************/


  char *insstmt_CELL_SECTOR_GENERAL =
     (char *)"INSERT INTO CELL_SECTOR_GENERAL (CELL_NUM,SECTOR_NUM,COMMENTS,CSID,CS_NUM,"
     "ANGLE_DEG,ANTENNA_TYPE,ANTENNA_HEIGHT,TX_POWER,NUM_PHYSICAL_TX)" 
    "values (:CELL_NUM,:SECTOR_NUM,:COMMENTS,:CSID,:CS_NUM,"
     ":ANGLE_DEG,:ANTENNA_TYPE,:ANTENNA_HEIGHT,:TX_POWER,:NUM_PHYSICAL_TX)";
  
  PHSSectorClass *sector;
  int cell_idx;
  int sector_idx;

  TempDB.beginSQL();
  TempDB.beginBind(insstmt_CELL_SECTOR_GENERAL);
  for ( cell_idx=0; cell_idx<num_cell; cell_idx++)
  {
    cell = cell_list[cell_idx];
    for (sector_idx=0; sector_idx<cell->num_sector; sector_idx++)
    {
        sector = (PHSSectorClass*)cell->sector_list[sector_idx];
        chptr = csid_string;
        if (sector->csid_hex) {
            for (unsigned int byte_num = 0; byte_num<=sector->csid_byte_length-1; byte_num++) {
                chptr += sprintf(chptr, "%.2X", sector->csid_hex[byte_num]);
            }
        }
        else  strcpy(csid_string,"");
        if (sector->comment == NULL)  sector->comment = strdup("");
        TempDB.bindInt   (":CELL_NUM",       &cell_idx);
        TempDB.bindInt   (":SECTOR_NUM",     &sector_idx);
        TempDB.bindString(":COMMENTS",       sector->comment);
        TempDB.bindString(":CSID",           csid_string);
        TempDB.bindInt   (":CS_NUM",         &sector->gw_csc_cs);
        double antenna_angle_deg = sector->antenna_angle_rad*180.0/PI;
        TempDB.bindDouble(":ANGLE_DEG",      &antenna_angle_deg);
        TempDB.bindInt   (":ANTENNA_TYPE",   &sector->antenna_type);
        TempDB.bindDouble(":ANTENNA_HEIGHT", &sector->antenna_height);
        TempDB.bindDouble(":TX_POWER",       &sector->tx_pwr);
        TempDB.bindInt   (":NUM_PHYSICAL_TX",&sector->num_physical_tx);
        TempDB.execSQL();
    }
  } 
  TempDB.endBind();
  TempDB.endSQL();

  insstmt_CELL_SECTOR_GENERAL = (char*)NULL;
  free(insstmt_CELL_SECTOR_GENERAL);

    /*****************************print table CELL_SECTOR_GENERAL**********************/



    /*****************************print table MEAS_CTR_LIST ****************************/

  char *insstmt_MEAS_CTR_LIST =
        (char *)"INSERT INTO MEAS_CTR_LIST (CELL_NUM,SECTOR_NUM,TRAFFIC_NUM,TRAFFIC)"
        " values (:CELL_NUM,:SECTOR_NUM,:TRAFFIC_NUM,:TRAFFIC)";
         
   int traffic_idx;

   TempDB.beginSQL();
   TempDB.beginBind(insstmt_MEAS_CTR_LIST);
   
   for ( cell_idx=0; cell_idx<num_cell; cell_idx++)
   {
        cell = cell_list[cell_idx];
        for (sector_idx=0; sector_idx<cell->num_sector; sector_idx++)
        {
           sector = (PHSSectorClass*)cell->sector_list[sector_idx];
           for (traffic_idx=0; traffic_idx < SectorClass::num_traffic; traffic_idx++)
           {
              TempDB.bindInt(":CELL_NUM",&cell_idx); 
              TempDB.bindInt(":SECTOR_NUM",&sector_idx);
              TempDB.bindInt(":TRAFFIC_NUM",&traffic_idx);
              TempDB.bindDouble(":TRAFFIC", &sector->meas_ctr_list[traffic_idx]);
              TempDB.execSQL();
           }
        }
  }
  TempDB.endBind();
  TempDB.endSQL();
  insstmt_MEAS_CTR_LIST = (char*)NULL;
  free(insstmt_MEAS_CTR_LIST);
  /*****************************print table MEAS_CTR_LIST ********************************/


  /*****************************print table GEOM_CSM ********************************/

  char *insstmt_GEOM_CSM =
     (char *)"INSERT INTO GEOM_CSM (NUM_SYS_BDY_PT,NUM_TRAFFIC_TYPE,NUM_SUBNET_0,NUM_SUBNET_1,"
       "NUM_ANTN_TYPE,NUM_PROP_MODEL,NUM_TRAFFIC) values (:NUM_SYS_BDY_PT,:NUM_TRAFFIC_TYPE,:NUM_SUBNET_0,"
       ":NUM_SUBNET_1,:NUM_ANTN_TYPE,:NUM_PROP_MODEL,:NUM_TRAFFIC)";

  TempDB.beginSQL();
  TempDB.beginBind(insstmt_GEOM_CSM);
  TempDB.bindInt(":NUM_SYS_BDY_PT",system_bdy->num_bdy_pt);
  TempDB.bindInt("NUM_TRAFFIC_TYPE",&num_traffic_type);
  TempDB.bindInt(":NUM_SUBNET_0", &num_subnet[0]);
  TempDB.bindInt(":NUM_SUBNET_1",  &num_subnet[1]);
  TempDB.bindInt(":NUM_ANTN_TYPE", &num_antenna_type);
  TempDB.bindInt(":NUM_PROP_MODEL",  &num_prop_model);
  TempDB.bindInt(":NUM_TRAFFIC",  &(SectorClass::num_traffic));
  TempDB.execSQL();
  TempDB.endBind();
  TempDB.endSQL();

   insstmt_GEOM_CSM = (char*)NULL;
   free(insstmt_GEOM_CSM);
  /******************************print table GEOM_CSM **************************/




  /*****************************print table TRAFFIC_TYPE ********************************/

  char *insstmt_TRAFFIC_TYPE =
     (char *)"INSERT INTO TRAFFIC_TYPE (TRAFFIC_NUM,TRAFFIC_TYPE,COLOR,"
       "DURATION_DIST,MEAN_TIME,MIN_TIME,MAX_TIME) values (:TRAFFIC_NUM,:TRAFFIC_TYPE,"
       ":COLOR,:DURATION_DIST,:MEAN_TIME,:MIN_TIME,:MAX_TIME)";
  
  char *duration_type;
  TrafficTypeClass *traffic_type= (TrafficTypeClass*)NULL;

  duration_type = (char*)malloc(15*sizeof(char));

  TempDB.beginSQL();
  TempDB.beginBind(insstmt_TRAFFIC_TYPE);
  
  for (traffic_index=0; traffic_index<num_traffic_type; traffic_index++)
  {
     traffic_type = traffic_type_list[traffic_index]; 
     TempDB.bindInt(":TRAFFIC_NUM",&traffic_index);
     TempDB.bindString(":TRAFFIC_TYPE",traffic_type->strid);
     //TempDB.bindDouble(":ARRIVAL_RATE", &traffic_type->arrival_rate);
     TempDB.bindInt(":COLOR",  &traffic_type->color);
     if (traffic_type->duration_dist == CConst::ExpoDist)
     {
        strcpy(duration_type, "EXPONENTIAL");
        traffic_type->min_time = 0;
        traffic_type->max_time = 0;
      }
      else if (traffic_type->duration_dist == CConst::UnifDist)
      {
         strcpy(duration_type, "UNIFORM");
         traffic_type->mean_time = 0;
      }
      TempDB.bindString(":DURATION_DIST",duration_type);
      TempDB.bindDouble(":MEAN_TIME",  &traffic_type->mean_time);
      TempDB.bindDouble(":MIN_TIME",  &traffic_type->min_time);
      TempDB.bindDouble(":MAX_TIME", &traffic_type->max_time);
      TempDB.execSQL(); 
  }
  TempDB.endBind();
  TempDB.endSQL();
  insstmt_TRAFFIC_TYPE = (char*)NULL;
  free(insstmt_TRAFFIC_TYPE);
  /******************************print table TRAFFIC_TYPE **************************/




  /******************************print table SUBNET ***********************/

  char *insstmt_SUBNET =
     (char *)"INSERT INTO SUBNET (SUB_TYPE,SUB_NUM,SUB_NAME,ARRIVAL_RATE,COLOR,NUM_SEGMENT)" 
                          "values (:SUB_TYPE,:SUB_NUM,:SUB_NAME,:ARRIVAL_RATE,:COLOR,:NUM_SEGMENT)";
  
  TempDB.beginSQL();
  TempDB.beginBind(insstmt_SUBNET);

  for (traffic_index = 0; traffic_index<num_traffic_type; traffic_index++)
    for (subnet_idx = 0; subnet_idx<num_subnet[traffic_index]; subnet_idx++)

  {
      subnet = subnet_list[traffic_index][subnet_idx] ;
      TempDB.bindInt(":SUB_TYPE",&traffic_index);
      TempDB.bindInt(":SUB_NUM",&subnet_idx);
      if (subnet->strid == NULL)
      {
          subnet->strid = CVECTOR(1);
          strcpy(subnet->strid,"");
      }
      TempDB.bindString(":SUB_NAME",subnet->strid);     
      TempDB.bindDouble(":ARRIVAL_RATE", &subnet->arrival_rate);
      TempDB.bindInt   (":COLOR",&subnet->color);
      TempDB.bindInt   (":NUM_SEGMENT",&subnet->p->num_segment);
      TempDB.execSQL();
  }
  TempDB.endBind();
  TempDB.endSQL();
  insstmt_SUBNET = (char*)NULL;
  free(insstmt_SUBNET);
  /********************************print table SUBNET ***************************/


/****************************print table SEGMENT **********************************/


  char *insstmt_SEGMENT =
     (char *)"INSERT INTO SEGMENT(SUB_TYPE,SUB_NUM,SEGMENT_NUM,NUM_BDY_PT)"
                          "values (:SUB_TYPE,:SUB_NUM,:SEGMENT_NUM,:NUM_BDY_PT)";

  int num_xy = 0,i;
  int  *segment_number;
  int segment_idx = 0;

  for (traffic_index=0; traffic_index<num_traffic_type; traffic_index++)
    for (index = 0; index < num_subnet[traffic_index]; index++)
   {
      subnet       = subnet_list[traffic_index][index];
      segment_idx += subnet->p->num_segment;
  }
  segment_number = (int*)malloc(segment_idx*sizeof(int));
  segment_idx = 0;
  for (traffic_index=0; traffic_index<num_traffic_type; traffic_index++)
    for (index = 0; index < num_subnet[traffic_index]; index++)
    {
          subnet = subnet_list[traffic_index][index];
          for (i=0; i<subnet->p->num_segment; i++)
          {
          segment_number[segment_idx++] = i;
          }
    }

  TempDB.beginSQL();
  TempDB.beginBind(insstmt_SEGMENT); 

  segment_idx = 0;
  for (traffic_index=0; traffic_index<num_traffic_type; traffic_index++)
    for (index = 0; index < num_subnet[traffic_index]; index++)
    {
      subnet = subnet_list[traffic_index][index];
      for (i =0; i<subnet->p->num_segment; i++)
          {
            TempDB.bindInt(":SUB_TYPE",&traffic_index);
            TempDB.bindInt(":SUB_NUM", &index);
            TempDB.bindInt(":SEGMENT_NUM", &segment_number[segment_idx++]);
            TempDB.bindInt(":NUM_BDY_PT",&subnet->p->num_bdy_pt[i]);
            TempDB.execSQL(); 
            num_xy += subnet->p->num_bdy_pt[i];
          }
  }
  TempDB.endBind();
  TempDB.endSQL();
  insstmt_SEGMENT= (char*)NULL;
  free(insstmt_SEGMENT);

  /******************************print table SEGMENT*************************************/


  /******************************print table SYS_BDY_PT***************************/

  char *insstmt_SYS_BDY_PT =
     (char *)"INSERT INTO SYS_BDY_PT (PT_NUM,X_CRDNATE,Y_CRDNATE)" 
			  "values (:PT_NUM,:X_CRDNATE,:Y_CRDNATE)";

  double bdyptx, bdypty;
 
  TempDB.beginSQL();
  TempDB.beginBind(insstmt_SYS_BDY_PT);

  for (index = 0; index<system_bdy->num_bdy_pt[0]; index++)
  {
       bdyptx = idx_to_x(system_bdy->bdy_pt_x[0][index]);
       bdypty = idx_to_y(system_bdy->bdy_pt_y[0][index]); 
       TempDB.bindInt(":PT_NUM",&index);
       TempDB.bindDouble(":X_CRDNATE",&bdyptx);
       TempDB.bindDouble(":Y_CRDNATE",&bdypty);
       TempDB.execSQL();
  }
  TempDB.endBind();
  TempDB.endSQL();
  insstmt_SYS_BDY_PT = (char*)NULL;
  free(insstmt_SYS_BDY_PT);
  /*********************************print table SYS_BDY_PT ******************************/



  /*********************************print table SEG_BDY_PT ******************************/

  char *insstmt_SEG_BDY_PT =
     (char *)"INSERT INTO SEG_BDY_PT (SUB_TYPE,SUB_NUM,SEGMENT_NUM,PT_NUM,X_CRDNATE,Y_CRDNATE)" 
			  "values (:SUB_TYPE,:SUB_NUM,:SEGMENT_NUM,:PT_NUM,:X_CRDNATE,:Y_CRDNATE)";

  int j, k = 0;
  int *segment_num, *pt_num;
  int bdy_pt_idx = 0;
  segment_idx = 0;
  
  segment_num = IVECTOR(num_xy);
  pt_num      = IVECTOR(num_xy);

  for (traffic_index = 0; traffic_index<num_traffic_type; traffic_index++)
    for (index=0; index<num_subnet[traffic_index]; index++)
  {
	  subnet = subnet_list[traffic_index][index];
	  for ( i=0; i<subnet->p->num_segment; i++ )
	  	  for (j=0; j<subnet->p->num_bdy_pt[i]; j++)
		  {   
	        	  segment_num[k] = i;
			  pt_num[k] = j;
			  k++;
		  }
  }

  TempDB.beginSQL();
  TempDB.beginBind(insstmt_SEG_BDY_PT);  
  for (traffic_index = 0; traffic_index<num_traffic_type; traffic_index++)
    for (index=0; index<num_subnet[traffic_index]; index++)
  {
          subnet = subnet_list[traffic_index][index];
          for ( segment_idx=0; segment_idx<subnet->p->num_segment; segment_idx++ )
                  for (bdy_pt_idx=0; bdy_pt_idx<subnet->p->num_bdy_pt[segment_idx]; bdy_pt_idx++)
                  {
                    bdyptx = idx_to_x(subnet->p->bdy_pt_x[segment_idx][bdy_pt_idx]);
                    bdypty = idx_to_y(subnet->p->bdy_pt_y[segment_idx][bdy_pt_idx]);
                    TempDB.bindInt(":SUB_TYPE",&traffic_index);
                    TempDB.bindInt(":SUB_NUM",&index);
                    TempDB.bindInt(":SEGMENT_NUM",&segment_idx);
                    TempDB.bindInt(":PT_NUM",&bdy_pt_idx);
                    TempDB.bindDouble(":X_CRDNATE",&bdyptx);
                    TempDB.bindDouble(":Y_CRDNATE",&bdypty);
                    TempDB.execSQL(); 
	           }
  }
  insstmt_SEG_BDY_PT = (char*)NULL;
  free(insstmt_SEG_BDY_PT);
  /***********************************print table SEG_BDY_PT ********************/



  /**********************************print table CELL_SECTOR_CSM******************/

  char *insstmt_CELL_SECTOR_CSM =
     (char *)"INSERT INTO CELL_SECTOR_CSM (CELL_NUM,SECTOR_NUM,ANGLE_DEG_CAL,PROP_MODEL,"
	         "HAS_ACCESS_CONTROL,CNTL_CHAN_SLOT,NUM_UNUSED_FREQ) values (:CELL_NUM,:SECTOR_NUM,"
			 ":ANGLE_DEG_CAL,:PROP_MODEL,:HAS_ACCESS_CONTROL,:CNTL_CHAN_SLOT,:NUM_UNUSED_FREQ)";

  TempDB.beginSQL();
  TempDB.beginBind(insstmt_CELL_SECTOR_CSM);
  for ( cell_idx=0; cell_idx<num_cell; cell_idx++ )
  {
    cell = cell_list[cell_idx];
    for (sector_idx=0; sector_idx<cell->num_sector; sector_idx++)
	{
	  sector = (PHSSectorClass*)cell_list[cell_idx]->sector_list[sector_idx];
          TempDB.bindInt(":CELL_NUM",&cell_idx);
          TempDB.bindInt(":SECTOR_NUM",&sector_idx);
          TempDB.bindDouble(":ANGLE_DEG_CAL",&sector->antenna_angle_rad);
          TempDB.bindInt(":PROP_MODEL",&sector->prop_model);
          TempDB.bindInt(":HAS_ACCESS_CONTROL",&sector->has_access_control);
          TempDB.bindInt(":CNTL_CHAN_SLOT",&sector->cntl_chan_slot);
          TempDB.bindInt(":NUM_UNUSED_FREQ",&sector->num_unused_freq);
          TempDB.execSQL(); 
	}
  }
  TempDB.endBind();
  TempDB.endSQL();
  insstmt_CELL_SECTOR_CSM = (char*)NULL;
  free(insstmt_CELL_SECTOR_CSM);
  /********************************print table CELL_SECTOR_CSM ************************/

  
  
  /********************************print table CELL_UNUSED_FREQ ***********************/

  char *insstmt_CELL_UNUSED_FREQ =
     (char *)"INSERT INTO CELL_UNUSED_FREQ (CELL_NUM,SECTOR_NUM,UNUSED_FREQ_NUM,UNUSED_FREQ_VALUE)"
	         "values (:CELL_NUM,:SECTOR_NUM,:UNUSED_FREQ_NUM,:UNUSED_FREQ_VALUE)";

  TempDB.beginSQL();
  TempDB.beginBind(insstmt_CELL_UNUSED_FREQ);
  for ( cell_idx=0; cell_idx<num_cell; cell_idx++ )
  {
    cell = cell_list[cell_idx];
    for (sector_idx=0; sector_idx<cell->num_sector; sector_idx++)
	{
	  sector = (PHSSectorClass*)cell_list[cell_idx]->sector_list[sector_idx];
      for ( i=0; i<sector->num_unused_freq; i++)
	  {
             TempDB.bindInt(":CELL_NUM",&cell_idx);
             TempDB.bindInt(":SECTOR_NUM",&sector_idx);
             TempDB.bindInt(":UNUSED_FREQ_NUM",&i);
             TempDB.bindInt(":UNUSED_FREQ_VALUE",&sector->unused_freq[i]);
             TempDB.execSQL();
	  }
	}
  }
  TempDB.endBind();
  TempDB.endSQL();
  insstmt_CELL_UNUSED_FREQ = (char*)NULL;
  free(insstmt_CELL_UNUSED_FREQ);

  /***************************print table CELL_UNUSED_FREQ *************************/



  /***************************print table PROP_MODEL *******************************/

   char *insstmt_PROP_MODEL;
  /* (char *)"INSERT INTO PROP_MODEL (PROP_NUM,PROP_TYPE,PROP_Y0,PROP_YS,PROP_K0,PROP_K1,"
	 "PROP_Y,PROP_PY,PROP_S1,PROP_S2,USE_HEIGHT,NUM_CLUTTER_TYPE,EXPONENT,COEFFICIENT) values (:PROP_NUM,:PROP_TYPE,"
	 ":PROP_Y0,:PROP_YS,:PROP_K0,:PROP_K1,:PROP_Y,:PROP_PY,:PROP_S1,:PROP_S2,:USE_HEIGHT,:NUM_CLUTTER_TYPE,"
	 ":EXPONENT,:COEFFICIENT)";
*/
  char *insstmt_PropExpo_Model = 
      (char *)"INSERT INTO PROP_MODEL (PROP_NUM,PROP_TYPE,STRID,EXPONENT,COEFFICIENT)"
       " values (:PROP_NUM,:PROP_TYPE,:STRID,:EXPONENT,:COEFFICIENT)";
  char *insstmt_PropPwLin_Model =
      (char *)"INSERT INTO PROP_MODEL (PROP_NUM,PROP_TYPE,STRID,PROP_Y0,PROP_YS,PROP_K0,PROP_K1,PROP_PY,PROP_S1)"
             " values (:PROP_NUM,:PROP_TYPE,:STRID,:PROP_Y0,:PROP_YS,:PROP_K0,:PROP_K1,:PROP_PY,:PROP_S1)";
  char *insstmt_PropTerrain_Model =
      (char *)"INSERT INTO PROP_MODEL (PROP_NUM,PROP_TYPE,STRID,PROP_Y,PROP_PY,PROP_S1,PROP_S2,USE_HEIGHT,NUM_CLUTTER_TYPE)"
             " values (:PROP_NUM,:PROP_TYPE,:STRID,:PROP_Y,:PROP_PY,:PROP_S1,:PROP_S2,:USE_HEIGHT,:NUM_CLUTTER_TYPE)"; 
  //TerrainPropModelClass *trn;
  int num_trnprop_veck = 0,num_veck;
  char *proptype;
  proptype = CVECTOR(15);

  for (i=0; i<num_prop_model; i++)
  {
     
     if (prop_model_list[i]->type() == CConst::PropExpo)
     {
            strcpy(proptype,"EXPONENTIAL");
            insstmt_PROP_MODEL= insstmt_PropExpo_Model;

      }
     else if (prop_model_list[i]->type() == CConst::PropPwLin)
     {
            strcpy(proptype,"PW_LINEAR");
            insstmt_PROP_MODEL= insstmt_PropPwLin_Model;
      }
     else if (prop_model_list[i]->type() == CConst::PropTerrain)
     {
             strcpy(proptype,"TERRAIN");
             insstmt_PROP_MODEL = insstmt_PropTerrain_Model;
      }
     else {
             strcpy(proptype,"RD");
      }

     TempDB.beginSQL();
     TempDB.beginBind(insstmt_PROP_MODEL);
     TempDB.bindInt(":PROP_NUM",&i);
     TempDB.bindString(":PROP_TYPE",proptype);
     TempDB.bindString(":STRID",prop_model_list[i]->get_strid());
     if (prop_model_list[i]->type() == CConst::PropExpo) {

           ExpoPropModelClass *expo_prop = ((ExpoPropModelClass *) prop_model_list[i]);
           double expo_prop_exponent    = expo_prop->exponent;
           double expo_prop_coefficient = expo_prop->coefficient;

	   TempDB.bindDouble(":EXPONENT",&expo_prop_exponent);
           TempDB.bindDouble(":COEFFICIENT",&expo_prop_coefficient); 

     }
     else if (prop_model_list[i]->type() == CConst::PropPwLin){

           PwLinPropModelClass *pwlin_prop = ((PwLinPropModelClass *) prop_model_list[i]);

           TempDB.bindDouble(":PROP_Y0",&pwlin_prop->y0);
           TempDB.bindDouble(":PROP_YS",&pwlin_prop->ys);
           TempDB.bindDouble(":PROP_K0",&pwlin_prop->k0);
           TempDB.bindDouble(":PROP_K1",&pwlin_prop->k1);
           TempDB.bindDouble(":PROP_PY",&pwlin_prop->py);
           TempDB.bindDouble(":PROP_S1",&pwlin_prop->s1);

     } 
     else if (prop_model_list[i]->type()==CConst::PropTerrain) {
 
           TerrainPropModelClass *trn_prop = ((TerrainPropModelClass *) prop_model_list[i]);
		   
           double val_y = trn_prop->val_y;
           double val_py= trn_prop->val_py;
           double val_s1= trn_prop->val_s1;
           double val_s2= trn_prop->val_s2;
           int    useheight = trn_prop->useheight;
           int    num_clttp = trn_prop->num_clutter_type;

           TempDB.bindDouble(":PROP_Y",&val_y);
           TempDB.bindDouble(":PROP_PY",&val_py);
           TempDB.bindDouble(":PROP_S1",&val_s1);
           TempDB.bindDouble(":PROP_S2",&val_s2);
           TempDB.bindInt(":USE_HEIGHT",&useheight);
           TempDB.bindInt(":NUM_CLUTTER_TYPE",&num_clttp);
	
           num_veck = 2*trn_prop->useheight + trn_prop->num_clutter_type;
           num_trnprop_veck += num_veck;
     }

     TempDB.execSQL();
     TempDB.endBind();
     TempDB.endSQL(); 
  }
  insstmt_PROP_MODEL = (char*)NULL;
  insstmt_PropExpo_Model = (char*)NULL;
  insstmt_PropPwLin_Model = (char*)NULL;
  insstmt_PropTerrain_Model = (char*)NULL;
  free(insstmt_PROP_MODEL);
  free(insstmt_PropExpo_Model);
  free(insstmt_PropPwLin_Model);
  free(insstmt_PropTerrain_Model);
  /*************************print table PROP_MODEL************************/



  /*************************print table TRN_PROP_VECK ********************/
   char *insstmt_TRN_PROP_VECK =
     (char *)"INSERT INTO TRN_PROP_VECK (PROP_NUM,VECK_NUM,PROP_TYPE,PROP_VECK)"
	         "values (:PROP_NUM,:VECK_NUM,:PROP_TYPE,:PROP_VECK)";

  double *prop_veck;
  char **prop_typ;
  int   *prop_num;
  int temp;
  int *veck_num;
  k=0;
  prop_num  = IVECTOR(num_trnprop_veck);
  veck_num  = IVECTOR(num_trnprop_veck);
  prop_veck = DVECTOR(num_trnprop_veck);
  prop_typ  = (char**)malloc(num_trnprop_veck*sizeof(char*));
  for (i=0; i<num_trnprop_veck; i++)
	  prop_typ[i] = CVECTOR(20);

  for (i=0; i<num_prop_model; i++)
  {
       if (prop_model_list[i]->type() == CConst::PropTerrain) {
          TerrainPropModelClass *trn_prop = ((TerrainPropModelClass *) prop_model_list[i]);
          num_veck = 2*trn_prop->useheight + trn_prop->num_clutter_type;
	  for (j=0; j<num_veck; j++)
          {
               prop_num[k] = i;
               veck_num[k] = j;
               k++;
          }
       }
  }

  i = 0;
  if ( num_trnprop_veck > 0 ) 
  {
    while (i < num_trnprop_veck )
    {
       TerrainPropModelClass *trn_prop = ((TerrainPropModelClass *) prop_model_list[prop_num[i]]);
       temp = 2*trn_prop->useheight+trn_prop->num_clutter_type;
       for (j=0; j<temp;j++)
       {
            strcpy(prop_typ[i], "TERRAIN");
            prop_veck[i] = (double)trn_prop->vec_k[j];
            i++;
       } 
     }
  }
  if ( num_trnprop_veck > 0)
  {
      TempDB.beginSQL();
      TempDB.beginBind(insstmt_TRN_PROP_VECK);

      for ( i=0; i<num_trnprop_veck; i++) 
      {
         TempDB.bindInt(":PROP_NUM",&prop_num[i]);
         TempDB.bindInt(":VECK_NUM",&veck_num[i]);
         TempDB.bindString(":PROP_TYPE",prop_typ[i]);
         TempDB.bindDouble(":PROP_VECK",&prop_veck[i]);
         TempDB.execSQL(); 
       }
      TempDB.endBind();
      TempDB.endSQL();   
  }
  insstmt_TRN_PROP_VECK = (char*)NULL;
  free(insstmt_TRN_PROP_VECK);
  /**************************print table TRN_PROP_VECK*****************************/



  /**************************print table ANTENNA_TYPE*****************************/
/*
  char * insstmt_ANTENNA_TYPE_NO_FILE  = 
            (char *)"INSERT INTO ANTENNA_TYPE(ANTENNA_TYPE_NUM,ANTENNA_TYPE)"
               "VALUES (:ANTENNA_TYPE_NUM,:ANTENNA_TYPE)";*/
  char * insstmt_ANTENNA_TYPE =
            (char *)"INSERT INTO ANTENNA_TYPE(ANTENNA_TYPE_NUM,ANTENNA_TYPE,ANTENNA_FILE)"
               "VALUES (:ANTENNA_TYPE_NUM,:ANTENNA_TYPE,:ANTENNA_FILE)";
  //char * insstmt_ANTENNA_TYPE;

  AntennaClass *antenna;
  char *ap;
  ap = CVECTOR(6);

  TempDB.beginSQL();
  TempDB.beginBind(insstmt_ANTENNA_TYPE);


  for (index = 0; index<num_antenna_type; index++)
  {
 
       antenna = antenna_type_list[index];      
       antenna_filename = antenna->get_filename();
     
       if (!antenna_filename)
            antenna_filename = strdup("");

       TempDB.bindInt(":ANTENNA_TYPE_NUM",&index);

       switch (antenna->get_type()){
           case CConst::antennaOmni : strcpy(ap,"OMNI"); break;
           case CConst::antennaLUT_H: strcpy(ap,"LUT_H");break;
           case CConst::antennaLUT_V: strcpy(ap,"LUT_V");break;
           case CConst::antennaLUT  : strcpy(ap,"LUT");  break;
       }

       TempDB.bindString(":ANTENNA_TYPE",ap);
       TempDB.bindString(":ANTENNA_FILE",antenna_filename);
       TempDB.execSQL();
  }
  insstmt_ANTENNA_TYPE = (char*)NULL;
  free(insstmt_ANTENNA_TYPE);
 /**********************************print table ANTENNA_TYPE********************************/



  //TempDB.disconnectDB();
  free(errmsg);

  cell = (CellClass*)NULL;
  free(cell);
  subnet = (SubnetClass*)NULL;
  free(subnet);
  sector = (PHSSectorClass *)NULL;
  free(sector);
  traffic_type = (TrafficTypeClass*)NULL;
  free(traffic_type);
  antenna = (AntennaClass*)NULL;
  free(antenna);
  
  free(duration_type);
  free(segment_number);
  free(segment_num);
  free(pt_num);
  free(proptype);
  free(prop_veck);
  free(prop_typ);
  free(prop_num);
  free(veck_num);
  free(ap);
  free(csid_string);
  free(antenna_filename);
  
  TempDB.disconnectDB();

#endif
}

void NetworkClass::quit_db()
{
  
  DBClass TempDB;
  char * errmsg = CVECTOR(255);
  if (preferences->selected_db == -1)
  {
      sprintf(msg, "ERROR: Not connected to database\n");
      PRMSG(stdout, msg);
      error_state = 1;
      return;
  } else 
  {
      if (!TempDB.connectDB(preferences->db_list[preferences->selected_db]->name,
                   preferences->db_list[preferences->selected_db]->user,
                   preferences->db_list[preferences->selected_db]->password,
                   errmsg))

      {
          sprintf(msg, errmsg);
          PRMSG(stdout, msg);
          error_state = 1;
          return;
      }
  }

  char *sqlstmt_PROGRESS = (char*) "delete from PROGRESS";

  TempDB.beginSQL();
  TempDB.processSQL(sqlstmt_PROGRESS);
  TempDB.endSQL();
    
  sqlstmt_PROGRESS = (char*)NULL; 
  free(sqlstmt_PROGRESS);

  char *insstmt_PROGRESS =
     (char *)"INSERT INTO PROGRESS(PROGRESS) values (:PROGRESS)";

  double      progress = 100.0;

  TempDB.beginSQL();
  TempDB.beginBind(insstmt_PROGRESS);
  TempDB.bindDouble(":PROGRESS",&progress);
  TempDB.execSQL();
  TempDB.endBind();
  TempDB.endSQL();

  insstmt_PROGRESS = (char*)NULL;
  free(insstmt_PROGRESS);

  free(errmsg);
  TempDB.disconnectDB();

 

}

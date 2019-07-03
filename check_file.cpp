/******************************************************************************************/
/**** FILE: check_file.cpp                                                             ****/
/******************************************************************************************/
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <string>
#include <iostream>
#include <fstream>
#include "WiSim.h"

using namespace std;
/******************************************************************************************/
/**** FUNCTION: check_file                                                             ****/
/******************************************************************************************/
void NetworkClass::check_file(char *filename)
{
 int error_found=0;
    FILE *fp;
   

    sprintf(msg, "Checking File \"%s\"\n", filename);
    PRMSG(stdout, msg);

    if ( !(fp = fopen(filename, "rb")) ) {
        sprintf(msg, "ERROR: cannot open geometry file %s\n", filename);
        PRMSG(stdout, msg);
        error_state = 1;
        return;
    }

 //   char buff[2048];
    char *p1,*p2,*p3,*p4,*p5,*p6,*p0,*p7,*p8,*p9;
    int f=1,p=0;

    char line[1000];
    char line1[1000];
    char GW[1000][7];
    char CSID[1000][13];
    char ch, *chptr,*chptr1;
    int i=0;
    int j=0;
    int n=0;
    int flag;


// ifstream text (filename);
// text.getline (buff,2048,'\n');
// while(text.getline (buff,2048,'\n'))
//   {
   while ( fgetline(fp, line) )
{
      printf("--%s--", line);
  
  
       strcpy(line1,line);
       chptr1 = line;     
       p++;


  /*       p0=strtok(buff,"  ");
         p1=strtok(NULL,"  ");
         p2=strtok(NULL,"  ");
         p3=strtok(NULL,"  ");
         p4=strtok(NULL,"  ");
         p5=strtok(NULL,"  ");
         p6=strtok(NULL,"  ");
         p7=strtok(NULL,"  ");
         p8=strtok(NULL,"  ");
         p9=strtok(NULL,"  ");
    */   
         p0=strtok(chptr1,"\t ");
         p1=strtok(NULL,"\t");
         p2=strtok(NULL,"\t");
         p3=strtok(NULL,"\t");
         p4=strtok(NULL,"\t");
         p5=strtok(NULL,"\t");
         p6=strtok(NULL,"\t");
         p7=strtok(NULL,"\t");
         p8=strtok(NULL,"\t");
          p9=strtok(NULL,"\t\n ");

         printf("LINUNUM = %d\n", p);
         if (p0) {
             printf("P0 = \"%s\"\n", p0);
         } else {
             printf("P0 = NULL\n");
         }
         if (p1) {
             printf("P1 = \"%s\"\n", p1);
         } else {
             printf("P1 = NULL\n");
         }
         if (p2) {
             printf("P2 = \"%s\"\n", p2);
         } else {
             printf("P2 = NULL\n");
         }
         if (p3) {
             printf("P3 = \"%s\"\n", p3);
         } else {
             printf("P3 = NULL\n");
         }
         if (p4) {
             printf("P4 = \"%s\"\n", p4);
         } else {
             printf("P4 = NULL\n");
         }
         if (p5) {
             printf("P5 = \"%s\"\n", p5);
         } else {
             printf("P5 = NULL\n");
         }
         if (p6) {
             printf("P6 = \"%s\"\n", p6);
         } else {
             printf("P6 = NULL\n");
         }
         if (p7) {
             printf("P7 = \"%s\"\n", p7);
         } else {
             printf("P7 = NULL\n");
         }
         if (p8) {
             printf("P8 = \"%s\"\n", p8);
         } else {
             printf("P8 = NULL\n");
         }
        if (p9) {
             printf("P9 = \"%s\"\n", p9);
         } else {
             printf("P9 = NULL\n");
         }
         printf("\n");

         if((p0)&&(p1)&&(p2)&&(p3)&&(p4)&&(p5)&&(p6)&&(p7)&&(p8)&&(p9))
           {
            f++;
         }
         else
         { sprintf(msg, " line %d is wrong",p);
         PRMSG(stdout, msg);
               error_found = 1;
        }
//chptr1++; 
      

       // printf("%s", line);
       int j=0;
     
       printf("@@%s@@",line1);      
       chptr = line1;
       ch = *chptr;
       GW[i][0]=ch;
       j++;
       if(ch!=' ' && ch!='\t' && ch!='\n' && ch!=EOF) {
           chptr++;
           ch = *chptr;
       }
       while(ch!=' '&& ch!='\t' && ch!='\n' && ch!=EOF){
           GW[i][j]=ch;
           chptr++;
           ch = *chptr;
           j++;
       }
       GW[i][j]='\0';

       j=0;
       while(ch==' ' || ch=='\t' && ch!=EOF) {
           chptr++;
           ch = *chptr;
       }
       flag = 1;
       while(ch!=' '&& ch!='\t' && ch!='\n' && ch!=EOF){
           if( ch == '.' ) {
               flag = 0;
               break;
           }
           CSID[i][j]=ch;
           chptr++;
           ch = *chptr;
           j++;
        }
        if( !flag || j<=3 ) {
            CSID[i][0] = '\t'; 
            j = 1;
        }
        CSID[i][j]='\0';

    

 i++;
}

n=i;


for(i=0;i<n;i++)
  printf("@@%s@@%s@@\n",GW[i],CSID[i]);

for(i=0;i<n;i++)
{
   if(strcmp(GW[i]," ")==0 ||strcmp(GW[i],"\t")==0){
 sprintf(msg, "GW-CSC-CS lack at line %d\n",i+1);
    PRMSG(stdout, msg);
   error_found = 1;
        
    }
   if(strcmp(CSID[i],"")==0||strcmp(CSID[i],"\t")==0){
   sprintf(msg, "CSID lack at line %d\n",i+1);
    PRMSG(stdout, msg);
   error_found = 1;
        
    }
}



    for(i=0;i<n;i++) {  
        for(j=i+1;j<n;j++) {
            if((strcmp(GW[i],GW[j])==0) &&!((strcmp(GW[i]," ")==0 || strcmp(GW[i],"\t")==0))){
                sprintf(msg, "GW-CSC-CS same at line %d and %d\n",i+1,j+1);
                PRMSG(stdout, msg);
                error_found = 1;
            }
            if((strcmp(CSID[i],CSID[j])==0) && CSID[i] != NULL
                &&!((strcmp(CSID[i],"")==0 || strcmp(CSID[i],"\t")==0))){
                sprintf(msg, "CSID same at line %d and %d\n",i+1,j+1);
                PRMSG(stdout, msg);
               error_found = 1;
            }
       }
    }

    if (error_found) {
     error_state = 1;
     }
    fclose(fp);

    return;

    
}
/******************************************************************************************/

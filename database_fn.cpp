#include "database_fn.h"
#include <oci.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>


DBClass::DBClass()
{
     envhp = (OCIEnv*)0;
     errhp = (OCIError*)0;
     svchp = (OCISvcCtx*)0;
     srvhp = (OCIServer*)0;
     stmthp = (OCIStmt*)0;
     authp = (OCISession*)0;
     dschp = (OCIDescribe*)0;
     bndp = (OCIBind**)0;
     status = (sword)0;
     bndp_num = 1;
}


DBClass::~DBClass()
{
}


int DBClass::connectDB( char* svcname, char* username, char* password, char* errmsg )
{  
     (void) OCIInitialize((ub4) OCI_DEFAULT, (dvoid *)0,
                       (dvoid * (*)(dvoid *, size_t)) 0,
                       (dvoid * (*)(dvoid *, dvoid *, size_t))0,
                       (void (*)(dvoid *, dvoid *)) 0 );

     (void) OCIEnvInit( (OCIEnv **)&envhp, (ub4)OCI_DEFAULT, (size_t) 0,
                     (dvoid **) 0 );

     (void) OCIHandleAlloc( (dvoid *) envhp, (dvoid **)&errhp, OCI_HTYPE_ERROR,
                   (size_t) 0, (dvoid **) 0);

     (void) OCIHandleAlloc( (dvoid *)envhp, (dvoid **) &srvhp, OCI_HTYPE_SERVER,
                   (size_t) 0, (dvoid **) 0);
  
     (void) OCIHandleAlloc( (dvoid *)envhp, (dvoid **) &svchp, OCI_HTYPE_SVCCTX,
                   (size_t) 0, (dvoid **) 0);
     
     status = OCIServerAttach( srvhp, errhp, (text *)svcname, strlen(svcname), OCI_DEFAULT );
     text errbuf[512];
//     ub4 buflen;
     sb4 errcode;
     if ( status != OCI_SUCCESS ) {
          switch( status ) {
               case OCI_SUCCESS_WITH_INFO:
                    printf("WARNING - OCI_SUCCESS_WITH_INFO\n");
		    break;
               case OCI_ERROR:
                    OCIErrorGet (errhp, (ub4) 1, (text *) NULL, &errcode,
                                errbuf, (ub4)sizeof(errbuf), OCI_HTYPE_ERROR);
		    sprintf( errmsg, "Error - %s\n", errbuf );
		    return 0;
               default:
	            sprintf( errmsg, "Error -  \n" );
                    return 0;
          }
     }
        
     (void) OCIAttrSet( (dvoid *) svchp, OCI_HTYPE_SVCCTX, (dvoid *)srvhp,
                     (ub4) 0, OCI_ATTR_SERVER, (OCIError *) errhp);

     (void) OCIHandleAlloc((dvoid *) envhp, (dvoid **)&authp,
                        (ub4) OCI_HTYPE_SESSION, (size_t) 0, (dvoid **) 0);

     (void) OCIAttrSet((dvoid *)authp, (ub4) OCI_HTYPE_SESSION,
                 (dvoid *) username, (ub4) strlen(username),
                 (ub4) OCI_ATTR_USERNAME, errhp);

     (void) OCIAttrSet((dvoid *)authp, (ub4) OCI_HTYPE_SESSION,
                 (dvoid *) password, (ub4) strlen(password),
                 (ub4) OCI_ATTR_PASSWORD,errhp);

     OCISessionBegin ( svchp, errhp, authp, OCI_CRED_RDBMS,
                          (ub4) OCI_DEFAULT);

     (void) OCIAttrSet((dvoid *)svchp, (ub4) OCI_HTYPE_SVCCTX,
                   (dvoid *)authp, (ub4) 0,
                   (ub4) OCI_ATTR_SESSION, errhp);
     return 1;
}


void DBClass::disconnectDB()
{
     OCISessionEnd( svchp, errhp, authp, (ub4) OCI_DEFAULT);
     OCIServerDetach( srvhp, errhp, 0 );
     OCIHandleFree( authp, (ub4)OCI_HTYPE_SESSION );
     OCIHandleFree( svchp, (ub4)OCI_HTYPE_SVCCTX );
     OCIHandleFree( srvhp, (ub4)OCI_HTYPE_SERVER );
     OCIHandleFree( errhp, (ub4)OCI_HTYPE_ERROR );
     OCIHandleFree( envhp, (ub4)OCI_HTYPE_ENV );
     OCITerminate( OCI_DEFAULT );
}


void DBClass::beginSQL()
{
     OCIHandleAlloc((dvoid*)envhp,(dvoid**)&stmthp,
                   OCI_HTYPE_STMT,(size_t)0,(dvoid**)0);
}


void DBClass::endSQL()
{
     OCIHandleFree( stmthp, (ub4)OCI_HTYPE_STMT );
}


void DBClass::beginBind( char* insstmt )
{
     (void) OCIHandleAlloc((dvoid *) envhp, (dvoid **) &dschp,
                       (ub4) OCI_HTYPE_DESCRIBE,
                        (size_t) 0, (dvoid **) 0);

     OCIStmtPrepare(stmthp, errhp, (text *) insstmt,
                         (ub4) strlen(insstmt),
                         (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
     bndp_num = 1;
}


void DBClass::endBind()
{
     free( bndp );
     OCIHandleFree( dschp, (ub4) OCI_HTYPE_DESCRIBE );
}


void DBClass::processSQL( char* sqlstmt )
{
     OCIStmtPrepare (stmthp,errhp,(text*)sqlstmt,
                 (ub4)strlen(sqlstmt),
                 (ub4)OCI_NTV_SYNTAX,(ub4)OCI_DEFAULT);
     OCIStmtExecute(svchp,stmthp,errhp,(ub4)1,(ub4)0,
                 (CONST OCISnapshot*)0,(OCISnapshot*)0,
                 (ub4)OCI_DEFAULT);
     OCITransCommit(svchp,errhp,(ub4)0);
}


void DBClass::bindInt( char* bindName, int* bindValue )
{
     if( bndp_num == 1 )  
          bndp = (OCIBind**)malloc( sizeof(OCIBind*) );
     else
          bndp = (OCIBind**)realloc( bndp, bndp_num*sizeof(OCIBind*) );
     OCIBindByName(stmthp, &(bndp[bndp_num-1]), errhp, (text *) bindName,
                   (sb4) -1, (dvoid *)bindValue,
                   (sb4) sizeof(int), SQLT_INT,
                   (dvoid *) 0, (ub2 *)0, (ub2 *)0, (ub4) 0, (ub4 *) 0,
                   (ub4) OCI_DEFAULT);
     bndp_num++;
}


void DBClass::bindDouble( char* bindName, double* bindValue )
{
     if( bndp_num == 1 )  
          bndp = (OCIBind**)malloc( sizeof(OCIBind*) );
     else
          bndp = (OCIBind**)realloc( bndp, bndp_num*sizeof(OCIBind*) );
     OCIBindByName(stmthp, &(bndp[bndp_num-1]), errhp, (text *) bindName,
                   (sb4) -1, (void *)bindValue,
                   (sb4) sizeof(double), SQLT_FLT,
                   (dvoid *) 0, (ub2 *)0, (ub2 *)0, (ub4) 0, (ub4 *) 0,
                   (ub4) OCI_DEFAULT);
     bndp_num++;
}


void DBClass::bindString( char* bindName, char* bindValue )
{
     if( bndp_num == 1 )  
          bndp = (OCIBind**)malloc( sizeof(OCIBind*) );
     else
          bndp = (OCIBind**)realloc( bndp, bndp_num*sizeof(OCIBind*) );
     OCIBindByName(stmthp, &(bndp[bndp_num-1]), errhp, (text *) bindName,
                   (sb4) -1, (dvoid *)bindValue,
                   (sb4) strlen(bindValue), SQLT_CHR,
                   (dvoid *) 0, (ub2 *)0, (ub2 *)0, (ub4) 0, (ub4 *) 0,
                   (ub4) OCI_DEFAULT);
     bndp_num++;
}


void DBClass::execSQL()
{
     OCIStmtExecute(svchp, stmthp, errhp, (ub4) 1, (ub4) 0,
                            (OCISnapshot *) NULL, (OCISnapshot *) NULL,
                            (ub4) OCI_DEFAULT);
     OCITransCommit(svchp, errhp, (ub4) 0);   
}


int DBClass::readString( char *selectall, char *data[], int pos, int num  )
{
     OCIDefine *defnp = (OCIDefine *) 0;
     sb2 outind = 0; 
     text *temp;
     int i=0;
     int j=0;
     
     temp = (text *)malloc( num*sizeof(char)+1 );

     OCIHandleAlloc( (dvoid *)envhp, (dvoid **) &stmthp,
                   OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0);

     OCIStmtPrepare( stmthp, errhp, (text *)selectall,
                   (ub4) strlen(selectall),
                   (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);

     OCIDefineByPos(stmthp, &defnp, errhp, pos, (dvoid *)temp,
                   num*sizeof(char), SQLT_CHR, (dvoid *)&outind, (ub2 *)0,
                   (ub2 *)0, OCI_DEFAULT);
 
     status = OCIStmtExecute(svchp, stmthp, errhp, (ub4) 1, (ub4) 0,
               (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);
     
     if ( status )
     {
          if (status != OCI_NO_DATA) {
	       free( temp );
               return OCI_ERROR;
	  }
     }
     do
     {
          for( j=num-1; j>=0; j-- ) {
                if( temp[j]!='\0' && temp[j]!='\n' && temp[j]!='\t' && temp[j]!=' ' )
		    break;
	  } 
	  temp[j+1] = '\0';	    
          strcpy(data[i],(char*)temp);
//	  data[i]+='\0';
	  i++;
     }
     while ( (status = OCIStmtFetch(stmthp, errhp, (ub4) 1, (ub4) OCI_FETCH_NEXT,
            (ub4) OCI_DEFAULT)) == OCI_SUCCESS ||
            status == OCI_SUCCESS_WITH_INFO);  
     free( temp );
     return (int)OCI_SUCCESS;
}


int DBClass::readDouble( char *selectall, double *data, int pos )
{
     OCIDefine *defnp = (OCIDefine *) 0;
     double temp;
     int i=0;
     OCIHandleAlloc( (dvoid *)envhp, (dvoid **) &stmthp,
                   OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0);

     OCIStmtPrepare(stmthp, errhp, (text *)selectall,
                                (ub4) strlen(selectall),
                                (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);

     OCIDefineByPos(stmthp, &defnp, errhp, pos, (dvoid *)&temp,
                   sizeof(double), SQLT_FLT, (dvoid *) 0, (ub2 *)0,
                   (ub2 *)0, OCI_DEFAULT);
  
     status = OCIStmtExecute( svchp, stmthp, errhp, (ub4) 1, (ub4) 0,
                          (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);
     if ( status )
     {
          if (status != OCI_NO_DATA)
               return OCI_ERROR;
     }

     do
     {
          data[i] = temp;
	  i++;
     }
     while ( (status = OCIStmtFetch(stmthp, errhp, (ub4) 1, (ub4) OCI_FETCH_NEXT,
            (ub4) OCI_DEFAULT)) == OCI_SUCCESS ||
            status == OCI_SUCCESS_WITH_INFO);

     return (int)OCI_SUCCESS;
}


int DBClass::readInt( char *selectall, int *data, int pos )
{
     OCIDefine *defnp = (OCIDefine *) 0;
     int temp;
     int i=0;
     OCIHandleAlloc( (dvoid *)envhp, (dvoid **) &stmthp,
                   OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0);

     OCIStmtPrepare( stmthp, errhp, (text *)selectall,
                   (ub4) strlen(selectall),
                   (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);

     OCIDefineByPos(stmthp, &defnp, errhp, pos, (dvoid *)&temp,
                   (int)sizeof(int), SQLT_INT, (dvoid *) 0, (ub2 *)0,
                   (ub2 *)0, OCI_DEFAULT);
  
     status = OCIStmtExecute(svchp, stmthp, errhp, (ub4) 1, (ub4) 0,
               (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);
     if ( status )
     {
          if (status != OCI_NO_DATA)
               return OCI_ERROR;
     }

     do
     {
          data[i] = temp;
	  i++;
     }
     while ((status = OCIStmtFetch( stmthp, errhp, (ub4) 1, (ub4) OCI_FETCH_NEXT,
                               (ub4) OCI_DEFAULT)) == OCI_SUCCESS ||
                               status == OCI_SUCCESS_WITH_INFO);

     return (int)OCI_SUCCESS;
}

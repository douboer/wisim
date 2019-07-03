#include <oci.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>


class DBClass {

     public:
          DBClass();
          ~DBClass();
          int connectDB( char* svcname, char* username, char* password, char* errmsg );//return 1 if success and 0 not success
          void disconnectDB();
          void beginSQL(); //allocate statement handle
          void processSQL( char* sqlstmt );//prepare, execute and commit
          void endSQL(); //free statement handle
          void beginBind( char* insstmt ); //allocate describe handle and prepare
          void bindInt( char* bindName, int* bindValue ); //bind bindValue to bindName 
          void bindDouble( char* bindName, double* bindValue );
          void bindString( char* bindName, char* bindValue );
          void execSQL(); //execute and commit
          void endBind();  //free describe handle
          int readString( char *selectall, char *data[], int pos, int length );
          int readDouble( char *selectall, double *data, int pos );
          int readInt( char *selectall, int *data, int pos );

     private:
          OCIEnv* envhp;
          OCIError* errhp;
          OCISvcCtx* svchp;
          OCIServer* srvhp;
          OCIStmt* stmthp;
          OCISession *authp;
          OCIDescribe *dschp;
          OCIBind** bndp;
          int bndp_num;
          sword status;  
};


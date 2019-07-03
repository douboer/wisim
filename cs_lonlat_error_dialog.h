/******************************************************************************************/
/**** PROGRAM: cs_lonlat_error_dialog.h                                                ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#include <qdialog.h>
#include <q3table.h>
#include <vector>
#include "WiSim.h"

using namespace std;

typedef vector<int> VEC_INT;
typedef vector<long> VEC_LONG;


class CsLonlatErrorDialog : public QDialog
{
    Q_OBJECT

public:
    CsLonlatErrorDialog( vector<VEC_LONG>& cell_error_list, QString StrLabel, NetworkClass* network_p, QWidget * parent = 0, const char * name = 0 );
    ~CsLonlatErrorDialog();

public slots:
    void onSaveClick();
    void onSort( const QString& selection );

private:
	void sortTable();
    Q3Table* cs_error_table;
    vector<VEC_LONG> cs_error_list;
	int* sorted_table_index;
	QString criteria_desc;

	NetworkClass* np;
};


#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
//Added by qt3to4:
#include <QResizeEvent>
#include <QLabel>
#include <QCloseEvent>

#include "gconst.h"

class QSplitter;
class Q3Canvas;
class QToolButton;
class QLabel;
class QToolBar;

class Call;
class CellText;
class CommandWindow;
class FigureEditor;
class GCellClass;
class InfoWindow;
class NetworkClass;
class RunSimulation;
class SectorgroupDia;
class VisibilityWindow;
class MenuStructureClass;

/******************************************************************************************/
/**** CLASS: MainWindowClass                                                           ****/
/******************************************************************************************/
class MainWindowClass : public QMainWindow
{
    Q_OBJECT

    friend class NetworkClass;
    friend class FigureEditor;
    friend class MeshClass;
    friend class VisibilityWindow;
    friend class VisibilityList;
    friend class MenuStructureClass;

    //liutao
    friend class CsLonlatErrorOptionDialog;
    
    friend int main(int argc, char** argv);
    friend void print_gui_message(FILE *fp, char *line);

public:
    MainWindowClass(NetworkClass *, QWidget* parent=0, const char* name=0, Qt::WFlags f=0);
    ~MainWindowClass();
    NetworkClass *get_np() {return np; }
    void set_command_window_prompt();
    void gui_command_entered(char *cmd);
    void gui_mode_changed();
    void gui_mode_changed_msc();
    bool view_menu_is_checked(int win_type);
    void view_menu_set_checked(int win_type, bool checked);

protected:
    virtual void resizeEvent(QResizeEvent*);
    void closeEvent( QCloseEvent* );

public slots:
    void help();

private slots:
    void testPRMSG();

    void execute_gui_exit();
    void execute_read_geometry_db();
    void execute_save_geometry_db();
    void execute_close_geometry();
    void execute_display_excel_geometry();
    void execute_include();
    void execute_add_cell_at();
    void execute_check_road_test_data();

    void execute_read_file(int type = GConst::geometryFile);
    void execute_save_file(int type = GConst::geometryFile, int idx = -1);

    void execute_set_mode(int mode);
    void listCanvasItems();
    void toggle_command_window(int a = GConst::visToggle);
    void toggle_visibility_window(int a = GConst::visToggle);
    void toggle_info_window(int a = GConst::visToggle);
    void vis_visibility_window(int a);
    void vis_command_window(int a);
    void vis_info_window(int a);
    void update_pwr_unit();
    void visPopOut();
    void visPopIn();
    void cmdPopOut();
    void cmdPopIn();
    void infoPopOut();
    void infoPopIn();

signals:
    void gui_command_emitted(char *);
    void num_comm_request_changed(int);

private:
    MenuStructureClass *create_menu_structure();
    Q3Canvas          *canvas;
    FigureEditor     *editor;

    CommandWindow    *command_window;
    VisibilityWindow *visibility_window;
    InfoWindow       *info_window;
    QLabel       *msg;
    QLabel       *state_lbl;

    MenuStructureClass *msc;

    QSplitter *s1, *s2, *s3;
    int dbf_id, num_event;
    NetworkClass* np;
    int geometry_modified;
    int rtd_modified;
    int unsaved_sim_data;
    QToolBar * fileTools;

    void reset_visibility_window();
    int warn_unsaved_changes(int action);
};
/******************************************************************************************/

#endif

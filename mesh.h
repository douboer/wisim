/******************************************************************************************/
/**** FILE: mesh.h                                                                     ****/
/******************************************************************************************/

#ifndef MESH_H
#define MESH_H

#include <iostream>

class NetworkClass;
class PolygonClass;
class LineClass;
class IntIntClass;
template<class T> class ListClass;

std::ostream& operator<<(std::ostream& s, class ConnectionClass& val);

/******************************************************************************************/
/**** CLASS: ConnectionClass                                                           ****/
/******************************************************************************************/
class ConnectionClass
{
public:
    ConnectionClass();
    ConnectionClass(int n, int c);
    ~ConnectionClass();
    int operator==(ConnectionClass& val);
    int operator>(ConnectionClass& val);
    friend std::ostream& operator<<(std::ostream& s, ConnectionClass& val);
    int node, cnum;
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: MeshNodeClass                                                             ****/
/******************************************************************************************/
class MeshNodeClass
{
public:
    MeshNodeClass(int posn_x, int posn_y);
    ~MeshNodeClass();
    int posn_x, posn_y;
    int num_conn;
    int *scan_idx;
    int metric;
    ConnectionClass **iconn, **oconn;
    static int **scan_array;

    void print_data(int mn);
};
/******************************************************************************************/

/******************************************************************************************/
/**** CLASS: MeshClass                                                                 ****/
/******************************************************************************************/
class MeshClass
{
public:
    MeshClass(int max_num_mesh_node, int **p_scan_array);
    ~MeshClass();
    MeshNodeClass **mesh_node_list;
    int num_mesh_node, max_num_mesh_node;

    void create_initial_mesh(NetworkClass *np);
    void simplify(NetworkClass *np, double scan_fractional_area);
    void remove_tiny_segments(NetworkClass *np);
    void print_data();
    void convert_to_polygons(ListClass<int> *&polygon_list, ListClass<int> *&color_list, ListClass<int> *&scan_idx_list);
    int comp_true_metric(NetworkClass *np, PolygonClass **polygon_list, int *scan_idx_list, int scan_idx_list_size);

    void assign_colors(ListClass<int> *scan_idx_list, ListClass<IntIntClass> *edgelist, ListClass<int> *color_list);

private:
    int get_mesh_node(const int posn_x, const int posn_y);
    void delete_mesh_node(MeshNodeClass *&mesh_node);
    void remove_linear_mesh_pts();
    void comp_mesh_node_metric(int mn, NetworkClass *np);
};
/******************************************************************************************/

#endif

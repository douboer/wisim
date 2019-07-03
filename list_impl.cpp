#include "doubleintint.h"
#include "intint.h"
#include "intintint.h"
#include "list.cpp"
#include "map_layer.h"
#include "mesh.h"
#include "road_test_data.h"
#include "strint.h"
#include "charstr.h"

class STParamClass;

template class ListClass<int>;
template class ListClass<double>;
template class ListClass<ConnectionClass>;
template class ListClass<DoubleIntIntClass>;
template class ListClass<IntIntClass>;
template class ListClass<IntIntIntClass>;
template class ListClass<StrIntClass>;
template class ListClass<RoadTestPtClass>;
template class ListClass<CharStrClass>;
template class ListClass<MapLayerClass *>;
template class ListClass<STParamClass *>;
template class ListClass<char *>;
template class ListClass<void *>;

template void sort2(ListClass<int> *, ListClass<int> *);
template void sort2(ListClass<CharStrClass> *, ListClass<int> *);
template void sort3(ListClass<int> *, ListClass<int> *, ListClass<int> *);

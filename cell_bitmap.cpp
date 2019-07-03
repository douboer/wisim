/******************************************************************************************/
/**** FILE: cell_bitmap.cpp                                                            ****/
/**** Michael Mandell 9/8/04                                                           ****/
/******************************************************************************************/

#include "cconst.h"
#include "WiSim.h"
#include "WiSim_gui.h"

#include <qbitmap.h>
#include <qimage.h>

QBitmap expand_bitmap(QBitmap &input_bm);

int GCellClass::num_sizes = 3;

//int GCellClass::size_list[] = {5, 9, 13};
int GCellClass::size_list[] = {10, 19, 23};

static unsigned char star_bits_5[] = {
    0x04, 0x04, 0x1f, 0x0e, 0x0a};
static unsigned char star_bits_9[] = {
    0x10, 0x00, 0x10, 0x00, 0x38, 0x00, 0xff, 0x01, 0x7c, 0x00, 0x7c, 0x00,
    0x6c, 0x00, 0x44, 0x00, 0x82, 0x00};
static unsigned char star_bits_13[] = {
    0x40, 0x00, 0x40, 0x00, 0xe0, 0x00, 0xe0, 0x00, 0xfc, 0x07, 0xff, 0x1f,
    0xfc, 0x07, 0xf8, 0x03, 0xf8, 0x03, 0xb8, 0x03, 0x1c, 0x07, 0x0c, 0x06,
    0x02, 0x08};


static unsigned char a_bits_5[] = {
   0x04, 0x04, 0x1f, 0x04, 0x04};
static unsigned char a_bits_9[] = {
   0x38, 0x00, 0x38, 0x00, 0x38, 0x00, 0xff, 0x01, 0xff, 0x01, 0xff, 0x01,
   0x38, 0x00, 0x38, 0x00, 0x38, 0x00};
static unsigned char a_bits_13[] = {
   0xe0, 0x00, 0xe0, 0x00, 0xe0, 0x00, 0xe0, 0x00, 0xe0, 0x00, 0xff, 0x1f,
   0xff, 0x1f, 0xff, 0x1f, 0xe0, 0x00, 0xe0, 0x00, 0xe0, 0x00, 0xe0, 0x00,
   0xe0, 0x00};


static unsigned char b_bits_5[] = {
   0x06, 0x0a, 0x1e, 0x02, 0x1f};
static unsigned char b_bits_9[] = {
   0x18, 0x00, 0x28, 0x00, 0x48, 0x00, 0xf8, 0x00, 0x08, 0x00, 0x08, 0x00,
   0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00};
static unsigned char b_bits_13[] = {
   0x60, 0x00, 0xa0, 0x00, 0x20, 0x01, 0x20, 0x02, 0x20, 0x04, 0xe0, 0x0f,
   0x20, 0x00, 0x20, 0x00, 0xfe, 0x0f, 0xfe, 0x0f, 0xfe, 0x0f, 0xfe, 0x0f,
   0xfe, 0x0f};


static unsigned char c_bits_5[] = {
   0x15, 0x0e, 0x1f, 0x0e, 0x15};
static unsigned char c_bits_9[] = {
   0x11, 0x01, 0x92, 0x00, 0x54, 0x00, 0x38, 0x00, 0xff, 0x01, 0x38, 0x00,
   0x54, 0x00, 0x92, 0x00, 0x11, 0x01};
static unsigned char c_bits_13[] = {
   0x41, 0x10, 0x42, 0x08, 0x44, 0x04, 0x48, 0x02, 0x50, 0x01, 0xe0, 0x00,
   0xff, 0x1f, 0xe0, 0x00, 0x50, 0x01, 0x48, 0x02, 0x44, 0x04, 0x42, 0x08,
   0x41, 0x10};

/******************************************************************************************/
/**** FUNCTION: GCellClass::setCellSize()                                              ****/
/******************************************************************************************/
void GCellClass::setCellSize(int size_idx)
{
    size          = size_list[size_idx  ];
    size_selected = size + CConst::select_cell_expand;
}
/******************************************************************************************/
/**** FUNCTION: GCellClass::set_bitmaps()                                              ****/
/******************************************************************************************/
void GCellClass::set_bitmaps(int size_idx)
{
#if CDEBUG
    if (CellClass::num_bm != 6) {
        CORE_DUMP;
    }
#endif
    int i;

    bm_list          = (QBitmap **) malloc(CellClass::num_bm*sizeof(QBitmap *));
    selected_bm_list = (QBitmap **) malloc(CellClass::num_bm*sizeof(QBitmap *));

    bm_list[0]          = (QBitmap *) NULL;  // Square
    selected_bm_list[0] = (QBitmap *) NULL;  // Square
    bm_list[1]          = (QBitmap *) NULL;  // Circle
    selected_bm_list[1] = (QBitmap *) NULL;  // Circle

    switch(size_idx) {
        case 0:
            bm_list[2] = new QBitmap(size_list[size_idx], size_list[size_idx], star_bits_5,  TRUE );
            bm_list[3] = new QBitmap(size_list[size_idx], size_list[size_idx], a_bits_5,  TRUE );
            bm_list[4] = new QBitmap(size_list[size_idx], size_list[size_idx], b_bits_5,  TRUE );
            bm_list[5] = new QBitmap(size_list[size_idx], size_list[size_idx], c_bits_5,  TRUE );
            break;
        case 1:
            bm_list[2] = new QBitmap(size_list[size_idx], size_list[size_idx], star_bits_9,  TRUE );
            bm_list[3] = new QBitmap(size_list[size_idx], size_list[size_idx], a_bits_9,  TRUE );
            bm_list[4] = new QBitmap(size_list[size_idx], size_list[size_idx], b_bits_9,  TRUE );
            bm_list[5] = new QBitmap(size_list[size_idx], size_list[size_idx], c_bits_9,  TRUE );
            break;
        case 2:
            bm_list[2] = new QBitmap(size_list[size_idx], size_list[size_idx], star_bits_13, TRUE );
            bm_list[3] = new QBitmap(size_list[size_idx], size_list[size_idx], a_bits_13, TRUE );
            bm_list[4] = new QBitmap(size_list[size_idx], size_list[size_idx], b_bits_13, TRUE );
            bm_list[5] = new QBitmap(size_list[size_idx], size_list[size_idx], c_bits_13, TRUE );
            break;
        default:
            CORE_DUMP;
            break;
    }

    for (i=2; i<=CellClass::num_bm-1; i++) {
        selected_bm_list[i] = new QBitmap(expand_bitmap(*bm_list[i]));
    }
}
/******************************************************************************************/
/**** FUNCTION: GCellClass::clear_bitmaps()                                            ****/
/******************************************************************************************/
void GCellClass::clear_bitmaps()
{
    int i;

    for (i=2; i<=CellClass::num_bm-1; i++) {
        delete bm_list[i];
        delete selected_bm_list[i];
    }

    if (bm_list) {
        free(bm_list);
        free(selected_bm_list);
        bm_list          = (QBitmap **) NULL;
        selected_bm_list = (QBitmap **) NULL;
    }
}
/******************************************************************************************/
/**** FUNCTION: expand_bitmap()                                                        ****/
/******************************************************************************************/
QBitmap expand_bitmap(QBitmap &input_bm)
{
    int i, j, u, v;

    QImage input_image = input_bm.convertToImage();
    QImage output_image = QImage(input_image.width() + CConst::select_cell_expand,
                                 input_image.height() + CConst::select_cell_expand, 32);

    output_image.fill(QColor(Qt::color0).rgb());

#if 0
    printf("COLOR0 = %d\n", (int) (Qt::color0.rgb() & 0x00FFFFFF) );
    printf("COLOR1 = %d\n", (int) (Qt::color1.rgb() & 0x00FFFFFF) );
    printf("INPUT IMAGE\n");
    printf("INPUT WIDTH  = %d\n", input_image.width());
    printf("INPUT HEIGHT = %d\n", input_image.height());
    printf("INPUT IMAGE DEPTH = %d\n", input_image.depth());
    for (j=0; j<=input_image.height()-1; j++) {
        for (i=0; i<=input_image.width()-1; i++) {
            printf("ORIG: %2d %2d -> PIXEL = %d\n", i, j, (int) input_image.pixel(i, j) & 0x00FFFFFF);
        }
    }
    fflush(stdout);
#endif

    for (i=0; i<=input_image.width()-1; i++) {
        for (j=0; j<=input_image.height()-1; j++) {
            if ( (int) (input_image.pixel(i, j) & 0x00FFFFFF) == (int) (QColor(Qt::color0).rgb() & 0x00FFFFFF) ) {
                for (u=-CConst::select_cell_expand/2; u<=CConst::select_cell_expand/2; u++) {
                    for (v=-CConst::select_cell_expand/2; v<=CConst::select_cell_expand/2; v++) {
                        if (u*u+v*v <= CConst::select_cell_expand*CConst::select_cell_expand/4) {
                            output_image.setPixel (i+u+CConst::select_cell_expand/2, j+v+CConst::select_cell_expand/2, QColor(Qt::color0).rgb());
                        }
                    }
                }
            }
        }
    }

#if 0
    printf("OUTPUT IMAGE\n");
    printf("WIDTH  = %d\n", output_image.width());
    printf("HEIGHT = %d\n", output_image.height());
    printf("OUTPUT IMAGE DEPTH = %d\n", output_image.depth());
    for (j=0; j<=output_image.height()-1; j++) {
        for (i=0; i<=output_image.width()-1; i++) {
            printf("OUTPUT: %2d %2d -> PIXEL = %d\n", i, j, (int) output_image.pixel(i, j) & 0x00FFFFFF);
        }
    }
    fflush(stdout);
#endif

    QBitmap output_bm;
    output_bm.convertFromImage(output_image);
    
    return(output_bm);
}
/******************************************************************************************/

/******************************************************************************************/
/**** FILE: set_language.cpp                                                           ****/
/******************************************************************************************/
#include "cconst.h"
#include "wisim.h"
#include "set_language.h"

QFont *application_font = (QFont *) NULL;
QFont *fixed_width_font = (QFont *) NULL;

/******************************************************************************************/
/**** FUNCTION: set_language                                                           ****/
/******************************************************************************************/
void set_language(int language)
{
    static QTranslator *translator = (QTranslator *) NULL;

    if (translator) {
        qApp->removeTranslator( translator );
        delete translator;
        translator = (QTranslator *) NULL;
    }

    if (application_font) { delete application_font; application_font = (QFont *) NULL; }
    if (fixed_width_font) { delete fixed_width_font; fixed_width_font = (QFont *) NULL; }

    if (language != CConst::none) {
        if (language == CConst::zh) {
            translator = new QTranslator(0);
#if TR_EMBED
            translator->load( wisim_zh_qm_data, wisim_zh_qm_len );
#else
            translator->load( QString( "WiSim_zh.qm" ), "." );
#endif
            qApp->installTranslator( translator );
#           ifdef __linux__
                application_font = new QFont("ZYSong18030", 10, QFont::Normal);
                fixed_width_font = new QFont("ZYSong18030", 10, QFont::Normal);
#           else
                application_font = new QFont(QString::fromLocal8Bit("ËÎÌו"), 9, QFont::Normal);
                fixed_width_font = new QFont(QString::fromLocal8Bit("ËÎÌו"), 9, QFont::Normal);
#           endif
            } else {
#           ifdef __linux__
                application_font = new QFont("TimesNewRoman", 10, QFont::Normal);
                fixed_width_font = new QFont("Monospace", 10, QFont::Normal);
#           else
                application_font = new QFont("Arial Unicode MS", 8.5, QFont::Normal);
                fixed_width_font = new QFont("Fixedsys", 8.5, QFont::Normal);
#           endif
        }

        qApp->setFont(*application_font);
    }
}
/******************************************************************************************/

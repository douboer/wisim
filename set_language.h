
#ifndef SET_LANGUAGE_H
#define SET_LANGUAGE_H

#include <qapplication.h>
#include <QTranslator>

#if TR_EMBED
#include "qm_files.h"
#endif

void set_language(int language);

#endif

/****************************************************************************
** $Id:  qt/tooltip.h   3.0.3   edited Oct 12 12:18 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <qwidget.h>
#include <qtooltip.h>


class DynamicTip : public QToolTip
{
public:
    DynamicTip( QWidget * parent );

protected:
    void maybeTip( const QPoint & );
};

#endif

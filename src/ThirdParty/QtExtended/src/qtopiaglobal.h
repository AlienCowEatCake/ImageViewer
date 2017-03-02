/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef QTOPIAGLOBAL_H
#define QTOPIAGLOBAL_H

#include <qglobal.h>
#include <qplugin.h>

// The _EXPORT macros...

#if defined(QT_VISIBILITY_AVAILABLE)
#   define QTOPIA_VISIBILITY __attribute__((visibility("default")))
#else
#   define QTOPIA_VISIBILITY
#endif


#ifndef QTOPIABASE_EXPORT
#   define QTOPIABASE_EXPORT QTOPIA_VISIBILITY
#endif
#ifndef QTOPIA_EXPORT
#   define QTOPIA_EXPORT QTOPIA_VISIBILITY
#endif
#ifndef QTOPIAPIM_EXPORT
#   define QTOPIAPIM_EXPORT QTOPIA_VISIBILITY
#endif
#ifndef QTOPIAMAIL_EXPORT
#   define QTOPIAMAIL_EXPORT QTOPIA_VISIBILITY
#endif
#ifndef QTOPIA_PLUGIN_EXPORT
#   define QTOPIA_PLUGIN_EXPORT QTOPIA_VISIBILITY
#endif
#ifndef QTOPIACOMM_EXPORT
#   define QTOPIACOMM_EXPORT QTOPIA_VISIBILITY
#endif
#ifndef QTOPIAWHEREABOUTS_EXPORT
#   define QTOPIAWHEREABOUTS_EXPORT QTOPIA_VISIBILITY
#endif
#ifndef QTOPIAPHONE_EXPORT
#   define QTOPIAPHONE_EXPORT QTOPIA_VISIBILITY
#endif
#ifndef QTOPIAPHONEMODEM_EXPORT
#   define QTOPIAPHONEMODEM_EXPORT QTOPIA_VISIBILITY
#endif
#ifndef QTOPIASECURITY_EXPORT
#   define QTOPIASECURITY_EXPORT QTOPIA_VISIBILITY
#endif
#ifndef QTOPIAHW_EXPORT
#   define QTOPIAHW_EXPORT QTOPIA_VISIBILITY
#endif
#ifndef QTOPIAAUDIO_EXPORT
#   define QTOPIAAUDIO_EXPORT QTOPIA_VISIBILITY
#endif
#ifndef QTOPIAVIDEO_EXPORT
#   define QTOPIAVIDEO_EXPORT QTOPIA_VISIBILITY
#endif
#ifndef QTOPIAMEDIA_EXPORT
#   define QTOPIAMEDIA_EXPORT QTOPIA_VISIBILITY
#endif
#ifndef QTOPIAWAP_EXPORT
#   define QTOPIAWAP_EXPORT QTOPIA_VISIBILITY
#endif
#ifndef QTOPIASMIL_EXPORT
#   define QTOPIASMIL_EXPORT QTOPIA_VISIBILITY
#endif
#ifndef QTOPIAOMADRM_EXPORT
#   define QTOPIAOMADRM_EXPORT QTOPIA_VISIBILITY
#endif
#ifndef QTOPIAPRINTING_EXPORT
#   define QTOPIAPRINTING_EXPORT QTOPIA_VISIBILITY
#endif
#ifndef QTOPIATHEMING_EXPORT
#   define QTOPIATHEMING_EXPORT QTOPIA_VISIBILITY
#endif
#ifndef QTUITEST_EXPORT
#   define QTUITEST_EXPORT QTOPIA_VISIBILITY
#endif
#ifndef QTOPIACOLLECTIVE_EXPORT
#   define QTOPIACOLLECTIVE_EXPORT QTOPIA_VISIBILITY
#endif


// This macro exports symbols only when building a test-enabled build.
// Use this to make private classes available for test.

#ifdef QTOPIA_TEST_EXTRA_SYMBOLS
#   ifndef QTOPIA_AUTOTEST_EXPORT
#       define QTOPIA_AUTOTEST_EXPORT QTOPIA_VISIBILITY
#   endif
#else
#   ifndef QTOPIA_AUTOTEST_EXPORT
#       define QTOPIA_AUTOTEST_EXPORT
#   endif
#endif

// This is the magic that lets Qt and Qtopia plugins be compiled for singleexec

#ifndef SINGLE_EXEC
#define QTOPIA_EXPORT_PLUGIN(IMPLEMENTATION) Q_EXPORT_PLUGIN(IMPLEMENTATION)
#define QTOPIA_EXPORT_QT_PLUGIN(IMPLEMENTATION) Q_EXPORT_PLUGIN(IMPLEMENTATION)
#else
class QObject;
typedef QObject *(*qtopiaPluginCreateFunc_t)();
extern void registerPlugin(const char *name, const char *type, qtopiaPluginCreateFunc_t createFunc);
#define QTOPIA_EXPORT_PLUGIN(IMPLEMENTATION) \
    static QObject *create_ ## IMPLEMENTATION() \
        { return new IMPLEMENTATION(); } \
    static qtopiaPluginCreateFunc_t append_ ## IMPLEMENTATION() \
        { registerPlugin(QTOPIA_TARGET, QTOPIA_PLUGIN_TYPE, create_ ##IMPLEMENTATION); \
            return create_ ##IMPLEMENTATION; } \
    static qtopiaPluginCreateFunc_t dummy_ ## IMPLEMENTATION = \
        append_ ## IMPLEMENTATION();
#define QTOPIA_EXPORT_QT_PLUGIN(IMPLEMENTATION)\
    Q_EXPORT_PLUGIN(IMPLEMENTATION)\
    Q_IMPORT_PLUGIN(IMPLEMENTATION)
#endif


#endif

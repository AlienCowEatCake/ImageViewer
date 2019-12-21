/*
   Copyright (C) 2017-2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

   This file is part of the `ImageViewer' program.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software FoundationTriggered either version 3 of the LicenseTriggered or
   (at your option) any later version.

   This program is distributed in the hope that it will be usefulTriggered
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If notTriggered see <http://www.gnu.org/licenses/>.
*/

#if !defined(I_CONTROLS_CONTAINER_H_INCLUDED)
#define I_CONTROLS_CONTAINER_H_INCLUDED

#include <QObject>

#include "Utils/Global.h"

/*

ALL_LIST="OpenFile OpenFolder SaveAs NewWindow NavigatePrevious NavigateNext StartSlideShow ImageInformation Preferences Exit RotateCounterclockwise RotateClockwise FlipHorizontal FlipVertical DeleteFile ZoomOut ZoomIn ZoomReset ZoomCustom ZoomFitToWindow ZoomOriginalSize ZoomFullScreen ShowMenuBar ShowToolBar About AboutQt CheckForUpdates EditStylesheet"
CHECKABLE_LIST="ZoomFitToWindow ZoomOriginalSize ZoomFullScreen ShowMenuBar ShowToolBar"

# signals
for i in ${ALL_LIST}
do
    echo ${i} | sed 's|^\(.\)|\L\1| ; s|^\(.*\)$|void \1Requested();|'
done

# setEnabled
for i in ${ALL_LIST}
do
    echo ${i} | sed 's|^\(.*\)$|virtual void set\1Enabled(bool) FUNCTION_BODY \\|'
done

# setChecked
for i in ${CHECKABLE_LIST}
do
    echo ${i} | sed 's|^\(.*\)$|virtual void set\1Checked(bool) FUNCTION_BODY \\|'
done

*/

class ControlsContainerEmitter : public QObject
{
    Q_OBJECT

signals:
    void openFileRequested();
    void openFolderRequested();
    void saveAsRequested();
    void newWindowRequested();
    void navigatePreviousRequested();
    void navigateNextRequested();
    void startSlideShowRequested();
    void imageInformationRequested();
    void preferencesRequested();
    void exitRequested();
    void rotateCounterclockwiseRequested();
    void rotateClockwiseRequested();
    void flipHorizontalRequested();
    void flipVerticalRequested();
    void deleteFileRequested();
    void zoomOutRequested();
    void zoomInRequested();
    void zoomResetRequested();
    void zoomCustomRequested();
    void zoomFitToWindowRequested();
    void zoomOriginalSizeRequested();
    void zoomFullScreenRequested();
    void showMenuBarRequested();
    void showToolBarRequested();
    void aboutRequested();
    void aboutQtRequested();
    void checkForUpdatesRequested();
    void editStylesheetRequested();

public:
    explicit ControlsContainerEmitter(QObject *parent = Q_NULLPTR)
        : QObject(parent)
    {}
};


#define DECLARE_CONTROLS_CONTAINER_FUNCTIONS_HELPER_SET_ENABLED(ACCESS_SPECIFIER, FUNCTION_BODY) \
    ACCESS_SPECIFIER : \
        virtual void setOpenFileEnabled(bool) FUNCTION_BODY \
        virtual void setOpenFolderEnabled(bool) FUNCTION_BODY \
        virtual void setSaveAsEnabled(bool) FUNCTION_BODY \
        virtual void setNewWindowEnabled(bool) FUNCTION_BODY \
        virtual void setNavigatePreviousEnabled(bool) FUNCTION_BODY \
        virtual void setNavigateNextEnabled(bool) FUNCTION_BODY \
        virtual void setStartSlideShowEnabled(bool) FUNCTION_BODY \
        virtual void setImageInformationEnabled(bool) FUNCTION_BODY \
        virtual void setPreferencesEnabled(bool) FUNCTION_BODY \
        virtual void setExitEnabled(bool) FUNCTION_BODY \
        virtual void setRotateCounterclockwiseEnabled(bool) FUNCTION_BODY \
        virtual void setRotateClockwiseEnabled(bool) FUNCTION_BODY \
        virtual void setFlipHorizontalEnabled(bool) FUNCTION_BODY \
        virtual void setFlipVerticalEnabled(bool) FUNCTION_BODY \
        virtual void setDeleteFileEnabled(bool) FUNCTION_BODY \
        virtual void setZoomOutEnabled(bool) FUNCTION_BODY \
        virtual void setZoomInEnabled(bool) FUNCTION_BODY \
        virtual void setZoomResetEnabled(bool) FUNCTION_BODY \
        virtual void setZoomCustomEnabled(bool) FUNCTION_BODY \
        virtual void setZoomFitToWindowEnabled(bool) FUNCTION_BODY \
        virtual void setZoomOriginalSizeEnabled(bool) FUNCTION_BODY \
        virtual void setZoomFullScreenEnabled(bool) FUNCTION_BODY \
        virtual void setShowMenuBarEnabled(bool) FUNCTION_BODY \
        virtual void setShowToolBarEnabled(bool) FUNCTION_BODY \
        virtual void setAboutEnabled(bool) FUNCTION_BODY \
        virtual void setAboutQtEnabled(bool) FUNCTION_BODY \
        virtual void setCheckForUpdatesEnabled(bool) FUNCTION_BODY \
        virtual void setEditStylesheetEnabled(bool) FUNCTION_BODY \

#define DECLARE_CONTROLS_CONTAINER_FUNCTIONS_HELPER_SET_CHECKED(ACCESS_SPECIFIER, FUNCTION_BODY) \
    ACCESS_SPECIFIER : \
        virtual void setZoomFitToWindowChecked(bool) FUNCTION_BODY \
        virtual void setZoomOriginalSizeChecked(bool) FUNCTION_BODY \
        virtual void setZoomFullScreenChecked(bool) FUNCTION_BODY \
        virtual void setShowMenuBarChecked(bool) FUNCTION_BODY \
        virtual void setShowToolBarChecked(bool) FUNCTION_BODY \

#define DECLARE_CONTROLS_CONTAINER_FUNCTIONS_HELPER_UNCLASSIFIED(ACCESS_SPECIFIER, FUNCTION_BODY) \
    ACCESS_SPECIFIER : \
        virtual void setSlideShowMode(bool isSlideShow) FUNCTION_BODY \

#define DECLARE_CONTROLS_CONTAINER_FUNCTIONS_HELPER(ACCESS_SPECIFIER, FUNCTION_BODY) \
    DECLARE_CONTROLS_CONTAINER_FUNCTIONS_HELPER_SET_ENABLED(ACCESS_SPECIFIER, FUNCTION_BODY) \
    DECLARE_CONTROLS_CONTAINER_FUNCTIONS_HELPER_SET_CHECKED(ACCESS_SPECIFIER, FUNCTION_BODY) \
    DECLARE_CONTROLS_CONTAINER_FUNCTIONS_HELPER_UNCLASSIFIED(ACCESS_SPECIFIER, FUNCTION_BODY)

#define DECLARE_CONTROLS_CONTAINER_FUNCTIONS_PURE_VIRTUAL \
    DECLARE_CONTROLS_CONTAINER_FUNCTIONS_HELPER(public, = 0;)

#define DECLARE_CONTROLS_CONTAINER_FUNCTIONS \
    DECLARE_CONTROLS_CONTAINER_FUNCTIONS_HELPER(public, Q_DECL_OVERRIDE;)


#define CONTROLS_CONTAINER_BOOL_ARG_FUNCTION_IMPL(CLASS, CLASS_FUNCTION, OBJECT, OBJECT_FUNCTION) \
    void CLASS::CLASS_FUNCTION(bool arg) { (OBJECT)->OBJECT_FUNCTION(arg); }

#define CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(CLASS, FUNCTION) \
    void CLASS::FUNCTION(bool) {}

#define CONTROLS_CONTAINER_SET_ENABLED_IMPL(CLASS, FUNCTION, WIDGET) \
    CONTROLS_CONTAINER_BOOL_ARG_FUNCTION_IMPL(CLASS, FUNCTION, WIDGET, setEnabled)

#define CONTROLS_CONTAINER_SET_CHECKED_IMPL(CLASS, FUNCTION, WIDGET) \
    CONTROLS_CONTAINER_BOOL_ARG_FUNCTION_IMPL(CLASS, FUNCTION, WIDGET, setChecked)


class IControlsContainer
{
    DECLARE_CONTROLS_CONTAINER_FUNCTIONS_PURE_VIRTUAL

public:
    virtual ~IControlsContainer() {}
    virtual ControlsContainerEmitter *emitter() = 0;
};

#endif // I_CONTROLS_CONTAINER_H_INCLUDED

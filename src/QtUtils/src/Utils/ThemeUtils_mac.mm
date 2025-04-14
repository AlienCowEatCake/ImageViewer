/*
   Copyright (C) 2018-2025 Peter S. Zhigalov <peter.zhigalov@gmail.com>

   This file is part of the `QtUtils' library.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ThemeUtils.h"
#include "ThemeUtils_mac.h"

#include "Workarounds/BeginExcludeOpenTransport.h"
#import <AppKit/AppKit.h>
#include "Workarounds/EndExcludeOpenTransport.h"

#include <AvailabilityMacros.h>

#include "InfoUtils.h"
#include "ObjectiveCUtils.h"

namespace ThemeUtils {

/// @brief Функция для определения темная используемая тема системы или нет
bool SystemHasDarkTheme()
{
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_14_AND_LATER)
    if(@available(*, macOS 10.14))
    {
        NSAppearance *appearance = [NSApp effectiveAppearance];
        NSAppearanceName bestMatchedName = [appearance bestMatchFromAppearancesWithNames:@[NSAppearanceNameAqua, NSAppearanceNameDarkAqua]];
        if(bestMatchedName)
            return [bestMatchedName isEqualToString:NSAppearanceNameDarkAqua];
        return [[appearance name] hasSuffix:@"DarkAqua"];
    }
#endif
    return false;
}

NSImage *GetMacSystemImage(IconTypes type)
{
#if defined (MAC_OS_X_VERSION_MAX_ALLOWED) && (MAC_OS_X_VERSION_MAX_ALLOWED >= 1050)
    if(InfoUtils::MacVersionGreatOrEqual(10, 5))
    {
        switch(type)
        {
#define ADD_ICON_CASE(ICON_TYPE, IMAGE_NAME) \
        case ICON_TYPE: \
            if(NSImage *image = [NSImage imageNamed:IMAGE_NAME]) \
                return image; \
            break;
        ADD_ICON_CASE(ICON_DOCUMENT_OPEN_WITH, NSImageNameRefreshTemplate)
        ADD_ICON_CASE(ICON_GO_NEXT, NSImageNameGoRightTemplate)
        ADD_ICON_CASE(ICON_GO_PREVIOUS, NSImageNameGoLeftTemplate)
        ADD_ICON_CASE(ICON_MEDIA_SLIDESHOW, NSImageNameSlideshowTemplate)
        ADD_ICON_CASE(ICON_VIEW_FULLSCREEN, NSImageNameEnterFullScreenTemplate)
        ADD_ICON_CASE(ICON_VIEW_REFRESH, NSImageNameRefreshTemplate)
#undef ADD_ICON_CASE
        default:
            break;
        }
    }
#endif

#if defined (MAC_OS_X_VERSION_MAX_ALLOWED) && (MAC_OS_X_VERSION_MAX_ALLOWED >= 110000)
    if(@available(*, macOS 11.0))
    {
        switch(type)
        {
#define ADD_ICON_IF(SYMBOL_NAME) \
        if(NSImage *image = [NSImage imageWithSystemSymbolName:SYMBOL_NAME accessibilityDescription:nil]) \
            return image;
#define SKIP_ICON_CASE(ICON_TYPE) \
        case ICON_TYPE: \
            break;
#define ADD_ICON_CASE1(ICON_TYPE, SYMBOL_NAME) \
        case ICON_TYPE: \
            ADD_ICON_IF(SYMBOL_NAME) \
            break;
#define ADD_ICON_CASE2(ICON_TYPE, SYMBOL_NAME1, SYMBOL_NAME2) \
        case ICON_TYPE: \
            ADD_ICON_IF(SYMBOL_NAME1) \
            ADD_ICON_IF(SYMBOL_NAME2) \
            break;
#define ADD_ICON_CASE3(ICON_TYPE, SYMBOL_NAME1, SYMBOL_NAME2, SYMBOL_NAME3) \
        case ICON_TYPE: \
            ADD_ICON_IF(SYMBOL_NAME1) \
            ADD_ICON_IF(SYMBOL_NAME2) \
            ADD_ICON_IF(SYMBOL_NAME3) \
            break;
        ADD_ICON_CASE1(ICON_APPLICATION_EXIT, @"xmark.circle")
        ADD_ICON_CASE1(ICON_DOCUMENT_NEW, @"doc.badge.plus")
        ADD_ICON_CASE1(ICON_DOCUMENT_OPEN, @"folder")
        ADD_ICON_CASE1(ICON_DOCUMENT_OPEN_WITH, @"arrow.clockwise")
        ADD_ICON_CASE1(ICON_DOCUMENT_PRINT, @"printer")
        ADD_ICON_CASE1(ICON_DOCUMENT_PROPERTIES, @"doc.badge.ellipsis")
        ADD_ICON_CASE1(ICON_DOCUMENT_SAVE, @"square.and.arrow.down")
        ADD_ICON_CASE1(ICON_DOCUMENT_SAVE_AS, @"square.and.arrow.down")
        ADD_ICON_CASE1(ICON_EDIT_COPY, @"doc.on.doc")
        ADD_ICON_CASE1(ICON_EDIT_CUT, @"scissors")
        ADD_ICON_CASE2(ICON_EDIT_DELETE, @"trash", @"delete.left")
        ADD_ICON_CASE1(ICON_EDIT_PASTE, @"clipboard")
        ADD_ICON_CASE2(ICON_EDIT_PREFERENCES, @"gearshape", @"gear")
        ADD_ICON_CASE3(ICON_GO_NEXT, @"chevron.right", @"arrowshape.right", @"arrow.right")
        ADD_ICON_CASE3(ICON_GO_PREVIOUS, @"chevron.left", @"arrowshape.left", @"arrow.left")
        ADD_ICON_CASE1(ICON_HELP_ABOUT, @"info.circle")
        SKIP_ICON_CASE(ICON_HELP_ABOUT_QT)
        ADD_ICON_CASE1(ICON_HELP_AUTHORS, @"person.2")
        ADD_ICON_CASE1(ICON_HELP_CONTENTS, @"questionmark.circle")
        ADD_ICON_CASE1(ICON_HELP_LICENSE, @"doc.text")
        ADD_ICON_CASE1(ICON_MEDIA_PLAYBACK_START, @"play")
        ADD_ICON_CASE1(ICON_MEDIA_PLAYBACK_STOP, @"stop")
        ADD_ICON_CASE2(ICON_MEDIA_SLIDESHOW, @"play.rectangle", @"play")
        ADD_ICON_CASE3(ICON_OBJECT_FLIP_HORIZONTAL, @"arrow.left.and.right.righttriangle.left.righttriangle.right", @"flip.horizontal", @"rectangle.landscape.rotate")
        ADD_ICON_CASE3(ICON_OBJECT_FLIP_VERTICAL, @"arrow.up.and.down.righttriangle.up.righttriangle.down", @"flip.vertical", @"rectangle.portrait.rotate")
        ADD_ICON_CASE1(ICON_OBJECT_ROTATE_LEFT, @"rotate.left")
        ADD_ICON_CASE1(ICON_OBJECT_ROTATE_RIGHT, @"rotate.right")
        ADD_ICON_CASE2(ICON_SYNC_SYNCHRONIZING, @"arrow.triangle.2.circlepath", @"arrow.clockwise")
        ADD_ICON_CASE1(ICON_VIEW_FULLSCREEN, @"arrow.up.left.and.arrow.down.right")
        ADD_ICON_CASE1(ICON_VIEW_REFRESH, @"arrow.clockwise")
        ADD_ICON_CASE1(ICON_WINDOW_CLOSE, @"xmark.circle")
        ADD_ICON_CASE1(ICON_WINDOW_NEW, @"macwindow.badge.plus")
        ADD_ICON_CASE1(ICON_ZOOM_CUSTOM, @"magnifyingglass")
        ADD_ICON_CASE2(ICON_ZOOM_FIT_BEST, @"arrow.up.left.and.down.right.magnifyingglass", @"square.arrowtriangle.4.outward")
        ADD_ICON_CASE1(ICON_ZOOM_IN, @"plus.magnifyingglass")
        ADD_ICON_CASE1(ICON_ZOOM_ORIGINAL, @"1.magnifyingglass")
        ADD_ICON_CASE1(ICON_ZOOM_OUT, @"minus.magnifyingglass")
#undef ADD_ICON_CASE3
#undef ADD_ICON_CASE2
#undef ADD_ICON_CASE1
#undef SKIP_ICON_CASE
#undef ADD_ICON_IF
        }
    }
#endif
    return nil;
}

QImage GetMacSystemImage(IconTypes type, const QSize &size)
{
    AUTORELEASE_POOL;
    return ObjCUtils::QImageFromNSImage(GetMacSystemImage(type), size, Qt::KeepAspectRatio);
}

} // namespace ThemeUtils


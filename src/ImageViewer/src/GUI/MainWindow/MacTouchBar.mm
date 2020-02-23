/*
   Copyright (C) 2020 Peter S. Zhigalov <peter.zhigalov@gmail.com>

   This file is part of the `ImageViewer' program.

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

#include "MacTouchBar.h"

#import <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>

#include <QApplication>
#include <QWidget>

#include "Utils/InfoUtils.h"
#include "Utils/ObjectiveCUtils.h"
#include "Utils/ThemeUtils.h"
#include "Utils/WindowUtils.h"

// ====================================================================================================

namespace {

struct SimpleTouchBarItem
{
    id item;

    SimpleTouchBarItem()
        : item(nil)
    {}

    ~SimpleTouchBarItem()
    {
        AUTORELEASE_POOL;
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_12_2_AND_LATER)
        if(@available(macOS 10.12.2, *))
        {
            if(NSCustomTouchBarItem *touchBarItem = reinterpret_cast<NSCustomTouchBarItem*>(item))
                [touchBarItem release];
        }
#endif
    }

    NSString *identifier() const
    {
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_12_2_AND_LATER)
        if(@available(macOS 10.12.2, *))
        {
            if(NSCustomTouchBarItem *touchBarItem = reinterpret_cast<NSCustomTouchBarItem*>(item))
                return [touchBarItem identifier];
        }
#endif
        return nil;
    }
};

struct ButtonedTouchBarItem : SimpleTouchBarItem
{
    NSButton *button;

    ButtonedTouchBarItem()
        : button(nil)
    {}

    ~ButtonedTouchBarItem()
    {
        AUTORELEASE_POOL;
        if(button)
            [button release];
    }

    void setEnabled(bool isEnabled)
    {
        if(!item || !button)
            return;
        AUTORELEASE_POOL;
        BOOL value = isEnabled ? YES : NO;
        [button setEnabled:value];
    }

    void setCheckable(bool isCheckable)
    {
        if(!item || !button)
            return;
        AUTORELEASE_POOL;
        NSButtonType newType = (isCheckable ? NSPushOnPushOffButton : NSMomentaryLightButton);
        [button setButtonType:newType];
    }

    void setChecked(bool isChecked)
    {
        if(!item || !button)
            return;
        AUTORELEASE_POOL;
        NSCellStateValue newState = isChecked ? NSOnState : NSOffState;
        [button setState:newState];
    }

    bool actionSenderMatch(id sender) const
    {
        return button && button == sender;
    }
};

} // namespace

// ====================================================================================================

namespace {

struct TouchBarData
{
    ButtonedTouchBarItem navigatePrevious;
    ButtonedTouchBarItem navigateNext;
    ButtonedTouchBarItem startSlideShow;
    ButtonedTouchBarItem zoomOut;
    ButtonedTouchBarItem zoomIn;
    ButtonedTouchBarItem zoomFullScreen;
    ButtonedTouchBarItem rotateCounterclockwise;
    ButtonedTouchBarItem rotateClockwise;
    ButtonedTouchBarItem openFile;
    ButtonedTouchBarItem saveFileAs;
    ButtonedTouchBarItem deleteFile;
};

} // namespace

// ====================================================================================================

#if defined (AVAILABLE_MAC_OS_X_VERSION_10_12_2_AND_LATER)
namespace {

NSImage *NSImageForIconType(ThemeUtils::IconTypes iconType)
{
    const QIcon themeIcon = ThemeUtils::GetIcon(iconType);
    const QSize iconSize(20, 20);
    const QPixmap iconPixmap = themeIcon.pixmap(iconSize);
    NSImage *image = ObjCUtils::QPixmapToNSImage(iconPixmap, iconSize);
    [image setTemplate:YES];
    return image;
}

} // namespace
#endif

// ====================================================================================================

#if defined (AVAILABLE_MAC_OS_X_VERSION_10_12_2_AND_LATER)
API_AVAILABLE(macos(10.12.2))
@interface TouchBarProvider: NSResponder <NSTouchBarDelegate, NSWindowDelegate>
{
    TouchBarData *m_touchBarData;
    QObject *m_emitterObject;
    id m_qtDelegate;
}

- (id)initWithTouchBarData:(TouchBarData *)touchBarData
          andEmitterObject:(QObject *)emitterObject;

- (void)installForWindow:(NSWindow *)window;
- (void)uninstallForWindow:(NSWindow *)window;

@end


@implementation TouchBarProvider

- (id)initWithTouchBarData:(TouchBarData *)touchBarData
          andEmitterObject:(QObject *)emitterObject
{
    self = [super init];
    m_touchBarData = touchBarData;
    m_emitterObject = emitterObject;

#define MAKE_BUTTONED_ITEM(ITEM, ICON) \
    m_touchBarData->ITEM.button = [NSButton buttonWithImage:NSImageForIconType(ThemeUtils::ICON) target:self action:@selector(itemClicked:)]; \
    NSCustomTouchBarItem *ITEM = [[NSCustomTouchBarItem alloc] initWithIdentifier:@"touchbar_"#ITEM]; \
    [ITEM setView:m_touchBarData->ITEM.button]; \
    m_touchBarData->ITEM.item = ITEM
    MAKE_BUTTONED_ITEM(navigatePrevious, ICON_LEFT);
    MAKE_BUTTONED_ITEM(navigateNext, ICON_RIGHT);
    MAKE_BUTTONED_ITEM(startSlideShow, ICON_PLAY);
    MAKE_BUTTONED_ITEM(zoomOut, ICON_ZOOM_OUT);
    MAKE_BUTTONED_ITEM(zoomIn, ICON_ZOOM_IN);
    MAKE_BUTTONED_ITEM(zoomFullScreen, ICON_FULLSCREEN);
    MAKE_BUTTONED_ITEM(rotateCounterclockwise, ICON_ROTATE_COUNTERCLOCKWISE);
    MAKE_BUTTONED_ITEM(rotateClockwise, ICON_ROTATE_CLOCKWISE);
    MAKE_BUTTONED_ITEM(openFile, ICON_OPEN);
    MAKE_BUTTONED_ITEM(saveFileAs, ICON_SAVE_AS);
    MAKE_BUTTONED_ITEM(deleteFile, ICON_DELETE);
#undef MAKE_BUTTONED_ITEM

    m_touchBarData->zoomFullScreen.setCheckable(true);

    return self;
}

- (NSTouchBar *)makeTouchBar
{
    NSTouchBar *bar = [[NSTouchBar alloc] init];
    [bar setDelegate:self];
    [bar setDefaultItemIdentifiers:@[
#define GET_IDENTIFIER(ITEM) m_touchBarData->ITEM.identifier()
        GET_IDENTIFIER(navigatePrevious),
        GET_IDENTIFIER(navigateNext),
        GET_IDENTIFIER(startSlideShow),
        GET_IDENTIFIER(zoomOut),
        GET_IDENTIFIER(zoomIn),
        GET_IDENTIFIER(zoomFullScreen),
        GET_IDENTIFIER(rotateCounterclockwise),
        GET_IDENTIFIER(rotateClockwise),
        GET_IDENTIFIER(openFile),
        GET_IDENTIFIER(saveFileAs),
        GET_IDENTIFIER(deleteFile),
#undef GET_IDENTIFIER
    ]];
    return bar;
}

- (NSTouchBarItem *)touchBar:(NSTouchBar *)touchBar makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier
{
    (void)(touchBar);
#define CHECK(ITEM) \
    if([identifier isEqualToString:m_touchBarData->ITEM.identifier()]) \
        return reinterpret_cast<NSCustomTouchBarItem*>(m_touchBarData->ITEM.item)
    CHECK(navigatePrevious);
    CHECK(navigateNext);
    CHECK(startSlideShow);
    CHECK(zoomOut);
    CHECK(zoomIn);
    CHECK(zoomFullScreen);
    CHECK(rotateCounterclockwise);
    CHECK(rotateClockwise);
    CHECK(openFile);
    CHECK(saveFileAs);
    CHECK(deleteFile);
#undef CHECK
   return nil;
}

- (void)installForWindow:(NSWindow *)window
{
    m_qtDelegate = [window delegate];
    [window setDelegate:self];
}

- (void)uninstallForWindow:(NSWindow *)window
{
    [window setDelegate:m_qtDelegate];
    m_qtDelegate = nil;
}

- (BOOL)respondsToSelector:(SEL)aSelector
{
    return [m_qtDelegate respondsToSelector:aSelector] || [super respondsToSelector:aSelector];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    [anInvocation invokeWithTarget:m_qtDelegate];
}

- (IBAction)itemClicked:(id)sender
{
#define INVOKE_IF_MATCH(ITEM, METHOD) \
    if(m_touchBarData->ITEM.actionSenderMatch(sender)) \
    { \
        QMetaObject::invokeMethod(m_emitterObject, METHOD); \
        return; \
    }
    INVOKE_IF_MATCH(navigatePrevious        , "navigatePreviousRequested"       )
    INVOKE_IF_MATCH(navigateNext            , "navigateNextRequested"           )
    INVOKE_IF_MATCH(startSlideShow          , "startSlideShowRequested"         )
    INVOKE_IF_MATCH(zoomOut                 , "zoomOutRequested"                )
    INVOKE_IF_MATCH(zoomIn                  , "zoomInRequested"                 )
    INVOKE_IF_MATCH(zoomFullScreen          , "zoomFullScreenRequested"         )
    INVOKE_IF_MATCH(rotateCounterclockwise  , "rotateCounterclockwiseRequested" )
    INVOKE_IF_MATCH(rotateClockwise         , "rotateClockwiseRequested"        )
    INVOKE_IF_MATCH(openFile                , "openFileRequested"               )
    INVOKE_IF_MATCH(saveFileAs              , "saveAsRequested"                 )
    INVOKE_IF_MATCH(deleteFile              , "deleteFileRequested"             )
#undef INVOKE_IF_MATCH
}

@end
#endif

// ====================================================================================================

struct MacTouchBar::Impl
{
    MacTouchBar *macTouchBar;
    TouchBarData touchBarData;
    id touchBarProvider;
    QWidget *widget;
    bool isSlideShowMode;

    explicit Impl(MacTouchBar *macTouchBar)
        : macTouchBar(macTouchBar)
        , touchBarProvider(nil)
        , widget(Q_NULLPTR)
        , isSlideShowMode(false)
    {
        AUTORELEASE_POOL;
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_12_2_AND_LATER)
        if(@available(macOS 10.12.2, *))
        {
            touchBarProvider = [[TouchBarProvider alloc] initWithTouchBarData:&touchBarData andEmitterObject:macTouchBar];
        }
#endif
    }

//    ~Impl()
//    {
//        AUTORELEASE_POOL;
//        macTouchBar->detachFromWindow();
//#if defined (AVAILABLE_MAC_OS_X_VERSION_10_12_2_AND_LATER)
//        if(@available(macOS 10.12.2, *))
//        {
//            [reinterpret_cast<TouchBarProvider*>(touchBarProvider) release];
//        }
//#endif
//    }

    void setSlideShowMode(bool isSlideShow)
    {
        AUTORELEASE_POOL;
        isSlideShowMode = isSlideShow;
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_12_2_AND_LATER)
        NSButton *button = touchBarData.startSlideShow.button;
        if(!button)
            return;
        if(!isSlideShowMode)
            [button setImage:NSImageForIconType(ThemeUtils::ICON_PLAY)];
        else
            [button setImage:NSImageForIconType(ThemeUtils::ICON_STOP)];
#endif
    }
};

// ====================================================================================================

MacTouchBar::MacTouchBar(QObject *parent)
    : ControlsContainerEmitter(parent)
    , m_impl(new Impl(this))
{}

MacTouchBar::~MacTouchBar()
{}

ControlsContainerEmitter *MacTouchBar::emitter()
{
    return this;
}

void MacTouchBar::attachToWindow(QWidget *widget)
{
    AUTORELEASE_POOL;
    detachFromWindow();
    m_impl->widget = widget;
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_12_2_AND_LATER)
    if(@available(macOS 10.12.2, *))
    {
        if(NSWindow *window = WindowUtils::GetNativeWindow(m_impl->widget))
            [reinterpret_cast<TouchBarProvider*>(m_impl->touchBarProvider) installForWindow:window];
    }
#endif
}

void MacTouchBar::detachFromWindow()
{
    AUTORELEASE_POOL;
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_12_2_AND_LATER)
    if(@available(macOS 10.12.2, *))
    {
        if(NSWindow *window = WindowUtils::GetNativeWindow(m_impl->widget))
            [reinterpret_cast<TouchBarProvider*>(m_impl->touchBarProvider) uninstallForWindow:window];
    }
#endif
    m_impl->widget = Q_NULLPTR;
}

CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setOpenFileEnabled, &m_impl->touchBarData.openFile)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setOpenFolderEnabled)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setSaveAsEnabled, &m_impl->touchBarData.saveFileAs)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setNewWindowEnabled)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setNavigatePreviousEnabled, &m_impl->touchBarData.navigatePrevious)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setNavigateNextEnabled, &m_impl->touchBarData.navigateNext)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setStartSlideShowEnabled, &m_impl->touchBarData.startSlideShow)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setImageInformationEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setPreferencesEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setExitEnabled)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setRotateCounterclockwiseEnabled, &m_impl->touchBarData.rotateCounterclockwise)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setRotateClockwiseEnabled, &m_impl->touchBarData.rotateClockwise)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setFlipHorizontalEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setFlipVerticalEnabled)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setDeleteFileEnabled, &m_impl->touchBarData.deleteFile)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setZoomOutEnabled, &m_impl->touchBarData.zoomOut)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setZoomInEnabled, &m_impl->touchBarData.zoomIn)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setZoomResetEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setZoomCustomEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setZoomFitToWindowEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setZoomOriginalSizeEnabled)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setZoomFullScreenEnabled, &m_impl->touchBarData.zoomFullScreen)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setShowMenuBarEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setShowToolBarEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setAboutEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setAboutQtEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setCheckForUpdatesEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setEditStylesheetEnabled)

CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setZoomFitToWindowChecked)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setZoomOriginalSizeChecked)
CONTROLS_CONTAINER_SET_CHECKED_IMPL(MacTouchBar, setZoomFullScreenChecked, &m_impl->touchBarData.zoomFullScreen)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setShowMenuBarChecked)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setShowToolBarChecked)

CONTROLS_CONTAINER_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setSlideShowMode, m_impl, setSlideShowMode)

// ====================================================================================================

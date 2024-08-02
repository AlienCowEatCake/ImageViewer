/*
   Copyright (C) 2020-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

/// @note qDebug macro conflicts
#import <CoreServices/CoreServices.h>

#include "MacTouchBar.h"

#import <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>

#include <QApplication>
#include <QWidget>

#include "Utils/InfoUtils.h"
#include "Utils/LocalizationManager.h"
#include "Utils/ObjectiveCUtils.h"
#include "Utils/ThemeUtils.h"
#include "Utils/ThemeUtils_mac.h"
#include "Utils/WindowUtils.h"

#include "Utils/MacNSInteger.h"

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
        AUTORELEASE_POOL;
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_12_2_AND_LATER)
        if(@available(macOS 10.12.2, *))
        {
            if(NSCustomTouchBarItem *touchBarItem = reinterpret_cast<NSCustomTouchBarItem*>(item))
                return [touchBarItem identifier];
        }
#endif
        return nil;
    }

    void setCustomizationLabel(const QString& label)
    {
        AUTORELEASE_POOL;
        (void)(label);
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_12_2_AND_LATER)
        if(@available(macOS 10.12.2, *))
        {
            if(NSCustomTouchBarItem *touchBarItem = reinterpret_cast<NSCustomTouchBarItem*>(item))
                [touchBarItem setCustomizationLabel:ObjCUtils::QStringToNSString(label)];
        }
#endif
    }
};

struct SegmentedTouchBarItem : SimpleTouchBarItem
{
    NSSegmentedControl *segmentedControl;

    SegmentedTouchBarItem()
        : segmentedControl(nil)
    {}

    ~SegmentedTouchBarItem()
    {
        AUTORELEASE_POOL;
        if(segmentedControl)
            [segmentedControl release];
    }
};

struct GroupedTouchBarItem
{
    SegmentedTouchBarItem *parentItem;
    NSInteger segmentNum;

    GroupedTouchBarItem()
        : parentItem(Q_NULLPTR)
        , segmentNum(-1)
    {}

    void setEnabled(bool isEnabled)
    {
        if(!parentItem || !parentItem->item || !parentItem->segmentedControl)
            return;
        AUTORELEASE_POOL;
        BOOL value = isEnabled ? YES : NO;
        if(segmentNum < 0)
            return;
        [parentItem->segmentedControl setEnabled:value forSegment:segmentNum];
    }

    bool actionSenderMatch(id sender) const
    {
        if(!parentItem || !parentItem->item || !parentItem->segmentedControl)
            return false;
        if(!sender || sender != parentItem->segmentedControl)
            return false;
        AUTORELEASE_POOL;
        NSSegmentedControl *segmentedControl = parentItem->segmentedControl;
        if(segmentNum < 0 || [segmentedControl selectedSegment] != segmentNum)
            return false;
        return true;
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
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_14_AND_LATER)
        NSButtonType newType = (isCheckable ? NSButtonTypePushOnPushOff : NSButtonTypeMomentaryLight);
#else
        NSButtonType newType = (isCheckable ? NSPushOnPushOffButton : NSMomentaryLightButton);
#endif
        [button setButtonType:newType];
    }

    void setChecked(bool isChecked)
    {
        if(!item || !button)
            return;
        AUTORELEASE_POOL;
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_14_AND_LATER)
        NSControlStateValue newState = isChecked ? NSControlStateValueOn : NSControlStateValueOff;
#else
        NSCellStateValue newState = isChecked ? NSOnState : NSOffState;
#endif
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
    SimpleTouchBarItem space;
    SimpleTouchBarItem flexibleSpace;

    GroupedTouchBarItem navigatePrevious;
    GroupedTouchBarItem navigateNext;
    SegmentedTouchBarItem navigateGroup;

    ButtonedTouchBarItem startSlideShow;

    GroupedTouchBarItem zoomOut;
    GroupedTouchBarItem zoomIn;
    SegmentedTouchBarItem zoomGroup;

    ButtonedTouchBarItem zoomFitToWindow;
    ButtonedTouchBarItem zoomOriginalSize;
    ButtonedTouchBarItem zoomFullScreen;

    GroupedTouchBarItem rotateCounterclockwise;
    GroupedTouchBarItem rotateClockwise;
    SegmentedTouchBarItem rotateGroup;

    GroupedTouchBarItem flipHorizontal;
    GroupedTouchBarItem flipVertical;
    SegmentedTouchBarItem flipGroup;

    ButtonedTouchBarItem openFile;
    ButtonedTouchBarItem saveFileAs;
    ButtonedTouchBarItem deleteFile;
    ButtonedTouchBarItem print;
    ButtonedTouchBarItem preferences;
    ButtonedTouchBarItem exit;
};

} // namespace

// ====================================================================================================

#if defined (AVAILABLE_MAC_OS_X_VERSION_10_12_2_AND_LATER)
namespace {

NSImage *NSImageForIconType(ThemeUtils::IconTypes iconType)
{
    if(NSImage *systemImage = ThemeUtils::GetMacSystemImage(iconType))
        return systemImage;

    const QIcon themeIcon = ThemeUtils::GetIcon(iconType);
    const QSize iconSize(20, 20);
    const QPixmap iconPixmap = themeIcon.pixmap(iconSize);
    NSImage *image = ObjCUtils::QPixmapToNSImage(iconPixmap, iconSize);
    [image setTemplate:YES];
    return image;
}

NSImage *NSImageForNameOrIconType(NSString *name, ThemeUtils::IconTypes iconType)
{
    if(name)
        if(NSImage *result = [NSImage imageNamed:name])
            return result;
    return NSImageForIconType(iconType);
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

    m_touchBarData->space.item = [[NSTouchBarItem alloc] initWithIdentifier:NSTouchBarItemIdentifierFixedSpaceSmall];
    m_touchBarData->flexibleSpace.item = [[NSTouchBarItem alloc] initWithIdentifier:NSTouchBarItemIdentifierFlexibleSpace];

#define MAKE_BUTTONED_ITEM(ITEM, ICON, TEMPLATE) \
    m_touchBarData->ITEM.button = [NSButton buttonWithImage:NSImageForNameOrIconType(TEMPLATE, ThemeUtils::ICON) target:self action:@selector(itemClicked:)]; \
    NSCustomTouchBarItem *ITEM = [[NSCustomTouchBarItem alloc] initWithIdentifier:@"touchbar_"#ITEM]; \
    [ITEM setView:m_touchBarData->ITEM.button]; \
    m_touchBarData->ITEM.item = ITEM
#define MAKE_SEGMENTED_PAIR_ITEM(GROUP, ITEM1, ICON1, TEMPLATE1, ITEM2, ICON2, TEMPLATE2) \
    m_touchBarData->GROUP.item = [[NSCustomTouchBarItem alloc] initWithIdentifier:@"touchbar_"#GROUP]; \
    m_touchBarData->GROUP.segmentedControl = [NSSegmentedControl \
        segmentedControlWithImages:@[NSImageForNameOrIconType(TEMPLATE1, ThemeUtils::ICON1), NSImageForNameOrIconType(TEMPLATE2, ThemeUtils::ICON2)] \
                      trackingMode:NSSegmentSwitchTrackingMomentary \
                            target:self \
                            action:@selector(itemClicked:)]; \
    [reinterpret_cast<NSCustomTouchBarItem*>(m_touchBarData->GROUP.item) setView:m_touchBarData->GROUP.segmentedControl]; \
    m_touchBarData->ITEM1.parentItem = &m_touchBarData->GROUP; \
    m_touchBarData->ITEM1.segmentNum = 0; \
    m_touchBarData->ITEM2.parentItem = &m_touchBarData->GROUP; \
    m_touchBarData->ITEM2.segmentNum = 1

    MAKE_SEGMENTED_PAIR_ITEM(navigateGroup, navigatePrevious, ICON_GO_PREVIOUS, NSImageNameTouchBarGoBackTemplate, navigateNext, ICON_GO_NEXT, NSImageNameTouchBarGoForwardTemplate);
    MAKE_BUTTONED_ITEM(startSlideShow, ICON_MEDIA_PLAYBACK_START, NSImageNameTouchBarSlideshowTemplate);
    MAKE_SEGMENTED_PAIR_ITEM(zoomGroup, zoomOut, ICON_ZOOM_OUT, nil, zoomIn, ICON_ZOOM_IN, nil);
    MAKE_BUTTONED_ITEM(zoomFitToWindow, ICON_ZOOM_FIT_BEST, nil);
    MAKE_BUTTONED_ITEM(zoomOriginalSize, ICON_ZOOM_ORIGINAL, nil);
    MAKE_BUTTONED_ITEM(zoomFullScreen, ICON_VIEW_FULLSCREEN, NSImageNameTouchBarEnterFullScreenTemplate);
    MAKE_SEGMENTED_PAIR_ITEM(rotateGroup, rotateCounterclockwise, ICON_OBJECT_ROTATE_LEFT, NSImageNameTouchBarRotateLeftTemplate, rotateClockwise, ICON_OBJECT_ROTATE_RIGHT, NSImageNameTouchBarRotateRightTemplate);
    MAKE_SEGMENTED_PAIR_ITEM(flipGroup, flipHorizontal, ICON_OBJECT_FLIP_HORIZONTAL, nil, flipVertical, ICON_OBJECT_FLIP_VERTICAL, nil);
    MAKE_BUTTONED_ITEM(openFile, ICON_DOCUMENT_OPEN, nil);
    MAKE_BUTTONED_ITEM(saveFileAs, ICON_DOCUMENT_SAVE_AS, nil);
    MAKE_BUTTONED_ITEM(deleteFile, ICON_EDIT_DELETE, NSImageNameTouchBarDeleteTemplate);
    MAKE_BUTTONED_ITEM(print, ICON_DOCUMENT_PRINT, nil);
    MAKE_BUTTONED_ITEM(preferences, ICON_EDIT_PREFERENCES, nil);
    MAKE_BUTTONED_ITEM(exit, ICON_APPLICATION_EXIT, nil);

#undef MAKE_SEGMENTED_PAIR_ITEM
#undef MAKE_BUTTONED_ITEM

    m_touchBarData->zoomFitToWindow.setCheckable(true);
    m_touchBarData->zoomOriginalSize.setCheckable(true);

    return self;
}

- (NSTouchBar *)makeTouchBar
{
    NSTouchBar *bar = [[NSTouchBar alloc] init];
    [bar setDelegate:self];
    [bar setDefaultItemIdentifiers:@[
#define GET_IDENTIFIER(ITEM) m_touchBarData->ITEM.identifier()
        GET_IDENTIFIER(navigateGroup),
        GET_IDENTIFIER(startSlideShow),
        GET_IDENTIFIER(zoomGroup),
        GET_IDENTIFIER(zoomFullScreen),
        GET_IDENTIFIER(rotateGroup),
        GET_IDENTIFIER(openFile),
        GET_IDENTIFIER(saveFileAs),
        GET_IDENTIFIER(deleteFile),
#undef GET_IDENTIFIER
    ]];
    [bar setCustomizationAllowedItemIdentifiers:@[
#define GET_IDENTIFIER(ITEM) m_touchBarData->ITEM.identifier()
        GET_IDENTIFIER(space),
        GET_IDENTIFIER(flexibleSpace),
        GET_IDENTIFIER(navigateGroup),
        GET_IDENTIFIER(startSlideShow),
        GET_IDENTIFIER(zoomGroup),
        GET_IDENTIFIER(zoomFitToWindow),
        GET_IDENTIFIER(zoomOriginalSize),
        GET_IDENTIFIER(zoomFullScreen),
        GET_IDENTIFIER(rotateGroup),
        GET_IDENTIFIER(flipGroup),
        GET_IDENTIFIER(openFile),
        GET_IDENTIFIER(saveFileAs),
        GET_IDENTIFIER(deleteFile),
#if defined (ENABLE_PRINT_SUPPORT)
        GET_IDENTIFIER(print),
#endif
        GET_IDENTIFIER(preferences),
        GET_IDENTIFIER(exit),
#undef GET_IDENTIFIER
    ]];
    if(NSBundle *bundle = [NSBundle mainBundle])
        if(NSString *identifier = [bundle bundleIdentifier])
            [bar setCustomizationIdentifier:[identifier stringByAppendingString:@".maintouchbar"]];
    return bar;
}

- (NSTouchBarItem *)touchBar:(NSTouchBar *)touchBar makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier
{
    (void)(touchBar);
#define CHECK(ITEM) \
    if([identifier isEqualToString:m_touchBarData->ITEM.identifier()]) \
        return reinterpret_cast<NSCustomTouchBarItem*>(m_touchBarData->ITEM.item)
    CHECK(space);
    CHECK(flexibleSpace);
    CHECK(navigateGroup);
    CHECK(startSlideShow);
    CHECK(zoomGroup);
    CHECK(zoomFitToWindow);
    CHECK(zoomOriginalSize);
    CHECK(zoomFullScreen);
    CHECK(rotateGroup);
    CHECK(flipGroup);
    CHECK(openFile);
    CHECK(saveFileAs);
    CHECK(deleteFile);
    CHECK(print);
    CHECK(preferences);
    CHECK(exit);
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
    INVOKE_IF_MATCH(zoomFitToWindow         , "zoomFitToWindowRequested"        )
    INVOKE_IF_MATCH(zoomOriginalSize        , "zoomOriginalSizeRequested"       )
    INVOKE_IF_MATCH(zoomFullScreen          , "zoomFullScreenRequested"         )
    INVOKE_IF_MATCH(rotateCounterclockwise  , "rotateCounterclockwiseRequested" )
    INVOKE_IF_MATCH(rotateClockwise         , "rotateClockwiseRequested"        )
    INVOKE_IF_MATCH(flipHorizontal          , "flipHorizontalRequested"         )
    INVOKE_IF_MATCH(flipVertical            , "flipVerticalRequested"           )
    INVOKE_IF_MATCH(openFile                , "openFileRequested"               )
    INVOKE_IF_MATCH(saveFileAs              , "saveAsRequested"                 )
    INVOKE_IF_MATCH(deleteFile              , "deleteFileRequested"             )
    INVOKE_IF_MATCH(print                   , "printRequested"                  )
    INVOKE_IF_MATCH(preferences             , "preferencesRequested"            )
    INVOKE_IF_MATCH(exit                    , "exitRequested"                   )
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

    explicit Impl(MacTouchBar *macTouchBar)
        : macTouchBar(macTouchBar)
        , touchBarProvider(nil)
        , widget(Q_NULLPTR)
    {
//        AUTORELEASE_POOL;
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_12_2_AND_LATER)
        if(@available(macOS 10.12.2, *))
        {
            touchBarProvider = [[TouchBarProvider alloc] initWithTouchBarData:&touchBarData andEmitterObject:macTouchBar];
            [[NSApplication sharedApplication] setAutomaticCustomizeTouchBarMenuItemEnabled:YES];
            retranslate();
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

    void retranslate()
    {
        touchBarData.navigateGroup.setCustomizationLabel(qApp->translate("MacTouchBar", "Navigate"));
        touchBarData.startSlideShow.setCustomizationLabel(qApp->translate("MacTouchBar", "Slideshow"));
        touchBarData.zoomGroup.setCustomizationLabel(qApp->translate("MacTouchBar", "Zoom"));
        touchBarData.zoomFitToWindow.setCustomizationLabel(qApp->translate("MacTouchBar", "Fit Image To Window Size"));
        touchBarData.zoomOriginalSize.setCustomizationLabel(qApp->translate("MacTouchBar", "Original Size"));
        touchBarData.zoomFullScreen.setCustomizationLabel(qApp->translate("MacTouchBar", "Full Screen"));
        touchBarData.rotateGroup.setCustomizationLabel(qApp->translate("MacTouchBar", "Rotate"));
        touchBarData.flipGroup.setCustomizationLabel(qApp->translate("MacTouchBar", "Flip"));
        touchBarData.openFile.setCustomizationLabel(qApp->translate("MacTouchBar", "Open File"));
        touchBarData.saveFileAs.setCustomizationLabel(qApp->translate("MacTouchBar", "Save File As"));
        touchBarData.deleteFile.setCustomizationLabel(qApp->translate("MacTouchBar", "Delete File"));
        touchBarData.print.setCustomizationLabel(qApp->translate("MacTouchBar", "Print"));
        touchBarData.preferences.setCustomizationLabel(qApp->translate("MacTouchBar", "Preferences"));
        touchBarData.exit.setCustomizationLabel(qApp->translate("MacTouchBar", "Exit"));
    }

    void setSlideShowMode(bool isSlideShow)
    {
        AUTORELEASE_POOL;
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_12_2_AND_LATER)
        if(@available(macOS 10.12.2, *))
        {
            NSButton *button = touchBarData.startSlideShow.button;
            if(!button)
                return;
            if(!isSlideShow)
                [button setImage:NSImageForNameOrIconType(NSImageNameTouchBarSlideshowTemplate, ThemeUtils::ICON_MEDIA_SLIDESHOW)];
            else
                [button setImage:NSImageForNameOrIconType(NSImageNameTouchBarRecordStopTemplate, ThemeUtils::ICON_MEDIA_PLAYBACK_STOP)];
        }
#else
        Q_UNUSED(isSlideShow);
#endif
    }

    void setZoomFullScreenMode(bool isFullScreen)
    {
        AUTORELEASE_POOL;
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_12_2_AND_LATER)
        if(@available(macOS 10.12.2, *))
        {
            NSButton *button = touchBarData.zoomFullScreen.button;
            if(!button)
                return;
            if(!isFullScreen)
                [button setImage:NSImageForNameOrIconType(NSImageNameTouchBarEnterFullScreenTemplate, ThemeUtils::ICON_VIEW_FULLSCREEN)];
            else
                [button setImage:NSImageForNameOrIconType(NSImageNameTouchBarExitFullScreenTemplate, ThemeUtils::ICON_VIEW_FULLSCREEN)];
        }
#else
        Q_UNUSED(isFullScreen);
#endif
    }
};

// ====================================================================================================

MacTouchBar::MacTouchBar(QObject *parent)
    : ControlsContainerEmitter(parent)
    , m_impl(new Impl(this))
{
    connect(LocalizationManager::instance(), SIGNAL(localeChanged(const QString&)), this, SLOT(retranslate()));
}

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

void MacTouchBar::retranslate()
{
    m_impl->retranslate();
}

CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setOpenFileEnabled, &m_impl->touchBarData.openFile)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setOpenFolderEnabled)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setSaveAsEnabled, &m_impl->touchBarData.saveFileAs)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setNewWindowEnabled)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setNavigatePreviousEnabled, &m_impl->touchBarData.navigatePrevious)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setNavigateNextEnabled, &m_impl->touchBarData.navigateNext)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setStartSlideShowEnabled, &m_impl->touchBarData.startSlideShow)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setImageInformationEnabled)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setPrintEnabled, &m_impl->touchBarData.print)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setPreferencesEnabled, &m_impl->touchBarData.preferences)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setExitEnabled, &m_impl->touchBarData.exit)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setRotateCounterclockwiseEnabled, &m_impl->touchBarData.rotateCounterclockwise)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setRotateClockwiseEnabled, &m_impl->touchBarData.rotateClockwise)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setFlipHorizontalEnabled, &m_impl->touchBarData.flipHorizontal)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setFlipVerticalEnabled, &m_impl->touchBarData.flipVertical)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setDeleteFileEnabled, &m_impl->touchBarData.deleteFile)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setZoomOutEnabled, &m_impl->touchBarData.zoomOut)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setZoomInEnabled, &m_impl->touchBarData.zoomIn)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setZoomResetEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setZoomCustomEnabled)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setZoomFitToWindowEnabled, &m_impl->touchBarData.zoomFitToWindow)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setZoomOriginalSizeEnabled, &m_impl->touchBarData.zoomOriginalSize)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacTouchBar, setZoomFullScreenEnabled, &m_impl->touchBarData.zoomFullScreen)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setShowMenuBarEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setShowToolBarEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setAboutEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setAboutQtEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setCheckForUpdatesEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setEditStylesheetEnabled)

CONTROLS_CONTAINER_SET_CHECKED_IMPL(MacTouchBar, setZoomFitToWindowChecked, &m_impl->touchBarData.zoomFitToWindow)
CONTROLS_CONTAINER_SET_CHECKED_IMPL(MacTouchBar, setZoomOriginalSizeChecked, &m_impl->touchBarData.zoomOriginalSize)
CONTROLS_CONTAINER_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setZoomFullScreenChecked, m_impl, setZoomFullScreenMode)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setShowMenuBarChecked)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setShowToolBarChecked)

CONTROLS_CONTAINER_BOOL_ARG_FUNCTION_IMPL(MacTouchBar, setSlideShowMode, m_impl, setSlideShowMode)

// ====================================================================================================

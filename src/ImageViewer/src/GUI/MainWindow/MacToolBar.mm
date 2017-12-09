/*
   Copyright (C) 2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "MacToolBar.h"

#import <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>

#include <QApplication>
#include <QWidget>
#include <QWindow>

#include "Utils/InfoUtils.h"
#include "Utils/LocalizationManager.h"
#include "Utils/ObjectiveCUtils.h"
#include "Utils/ThemeUtils.h"

// ====================================================================================================

namespace {

const NSInteger BUTTON_HEIGHT = 32;
const NSInteger ALONE_BUTTON_WIDTH = 40;
const NSInteger GROUPED_BUTTON_WIDTH = 36;
const NSInteger SEGMENTED_OFFSET = 0;
const QColor BUTTON_BASE_COLOR = qRgb(85, 85, 85);
const QColor BUTTON_ALTERNATE_COLOR = qRgb(255, 255, 255);

} // namespace

// ====================================================================================================

@interface CheckableNSButton : NSButton
{
    @private
    BOOL m_isChecked;
    BOOL m_isCheckable;
}

- (id)initWithFrame:(NSRect)frameRect;

- (BOOL)isCheckable;
- (void)setCheckable:(BOOL)isCheckable;

- (BOOL)isChecked;
- (void)setChecked:(BOOL)isChecked;

@end


@implementation CheckableNSButton

- (id)initWithFrame:(NSRect)frameRect
{
    [super initWithFrame:frameRect];
    m_isChecked = NO;
    m_isCheckable = NO;
    return self;
}

- (BOOL)isCheckable
{
    return m_isCheckable;
}

- (void)setCheckable:(BOOL)isCheckable
{
    NSButtonType newType = (isCheckable ? NSPushOnPushOffButton : NSMomentaryLightButton);
    [self setButtonType:newType];
    m_isCheckable = isCheckable;
}

- (BOOL)isChecked
{
    return m_isChecked;
}

- (void)setChecked:(BOOL)isChecked
{
    NSControlStateValue newState = isChecked ? NSOnState : NSOffState;
    [self setState:newState];
    if((![self isChecked] && isChecked) || ([self isChecked] && !isChecked))
    {
        NSImage *tempImage = [self image];
        [self setImage:[self alternateImage]];
        [self setAlternateImage:tempImage];
    }
    m_isChecked = isChecked;
}

@end

// ====================================================================================================

namespace {

struct SimpleToolBarItem
{
    NSToolbarItem *item;

    SimpleToolBarItem()
        : item(nil)
    {}

    ~SimpleToolBarItem()
    {
        if(item)
            [item release];
    }

    NSString *identifier() const
    {
        if(!item)
            return nil;
        return [item itemIdentifier];
    }

    bool actionSenderMatch(id sender) const
    {
        return item && item == sender;
    }

    void setLabel(const QString &label)
    {
        if(!item)
            return;
        AUTORELEASE_POOL;
        [item setLabel:ObjCUtils::QStringToNSString(label)];
    }

    void setToolTip(const QString &toolTip)
    {
        if(!item)
            return;
        AUTORELEASE_POOL;
        [item setToolTip:ObjCUtils::QStringToNSString(toolTip)];
    }

    void setPaletteLabel(const QString &paletteLabel)
    {
        if(!item)
            return;
        AUTORELEASE_POOL;
        [item setPaletteLabel:ObjCUtils::QStringToNSString(paletteLabel)];
    }
};

struct SegmentedToolBarItem : SimpleToolBarItem
{
    NSSegmentedControl *segmentedControl;

    SegmentedToolBarItem()
        : segmentedControl(nil)
    {}

    ~SegmentedToolBarItem()
    {
        if(segmentedControl)
            [segmentedControl release];
    }
};

struct GroupedToolBarItem : SimpleToolBarItem
{
    NSToolbarItemGroup *group;
    NSSegmentedControl *segmentedControl;

    GroupedToolBarItem()
        : group(nil)
        , segmentedControl(nil)
    {}

    void setEnabled(bool isEnabled)
    {
        if(!item || !segmentedControl || !group)
            return;
        AUTORELEASE_POOL;
        BOOL value = isEnabled ? YES : NO;
        [item setEnabled: value];
        [group setEnabled:NO];
        for(NSToolbarItem *subitem in [group subitems])
        {
            if([subitem isEnabled])
            {
                [group setEnabled:YES];
                break;
            }
        }
        NSInteger segmentNum = segmentNumber();
        if(segmentNum < 0)
            return;
        [segmentedControl setEnabled:value forSegment:segmentNum];
    }

    bool actionSenderMatch(id sender) const
    {
        if(item && item == sender)
            return true;
        if(!sender || sender != segmentedControl)
            return false;
        return [segmentedControl selectedSegment] == segmentNumber();
    }

    void setToolTip(const QString &toolTip)
    {
        if(!item || !segmentedControl)
            return;
        AUTORELEASE_POOL;
        [item setToolTip:ObjCUtils::QStringToNSString(toolTip)];
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_13_AND_LATER)
        if(@available(macOS 10.13, *))
        {
            NSInteger segmentNum = segmentNumber();
            if(segmentNum < 0)
                return;
            [segmentedControl setToolTip:ObjCUtils::QStringToNSString(toolTip) forSegment:segmentNum];
        }
#endif
    }

    NSInteger segmentNumber() const
    {
        if(!item || !group)
            return -1;
        AUTORELEASE_POOL;
        NSArray *groupItems = [group subitems];
        if(![groupItems containsObject:item])
            return -1;
        return static_cast<NSInteger>([[group subitems] indexOfObject:item]);
    }
};

struct ButtonedToolBarItem : SimpleToolBarItem
{
    CheckableNSButton *button;

    ButtonedToolBarItem()
        : button(nil)
    {}

    ~ButtonedToolBarItem()
    {
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

    void setChecked(bool isChecked)
    {
        if(!item || !button)
            return;
        AUTORELEASE_POOL;
        BOOL value = isChecked ? YES : NO;
        [button setChecked:value];
    }

    bool actionSenderMatch(id sender) const
    {
        return button && button == sender;
    }

    void setToolTip(const QString &toolTip)
    {
        if(!button)
            return;
        AUTORELEASE_POOL;
        [button setToolTip:ObjCUtils::QStringToNSString(toolTip)];
    }
};

} // namespace

// ====================================================================================================

namespace {
struct ToolBarData;
} // namespace

@interface MacToolbarDelegate : NSObject <NSToolbarDelegate>
{
    ToolBarData *m_toolBarData;
    QObject *m_emitterObject;
}

- (id)initWithToolBarData:(ToolBarData *)toolBarData
         andEmitterObject:(QObject*)emitterObject;

- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar
     itemForItemIdentifier:(NSToolbarItemIdentifier)itemIdentifier
 willBeInsertedIntoToolbar:(BOOL)flag;

- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar;

- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar;

- (NSArray *)toolbarSelectableItemIdentifiers:(NSToolbar *)toolbar;

- (IBAction)itemClicked:(id)sender;

@end

// ====================================================================================================

namespace {

struct ToolBarData
{
    SimpleToolBarItem space;
    SimpleToolBarItem flexibleSpace;

    GroupedToolBarItem navigatePrevious;
    GroupedToolBarItem navigateNext;
    SegmentedToolBarItem navigateGroup;

    ButtonedToolBarItem startSlideShow;

    GroupedToolBarItem zoomOut;
    GroupedToolBarItem zoomIn;
    SegmentedToolBarItem zoomGroup;

    ButtonedToolBarItem zoomFitToWindow;
    ButtonedToolBarItem zoomOriginalSize;
    ButtonedToolBarItem zoomFullScreen;

    GroupedToolBarItem rotateCounterclockwise;
    GroupedToolBarItem rotateClockwise;
    SegmentedToolBarItem rotateGroup;

    GroupedToolBarItem flipHorizontal;
    GroupedToolBarItem flipVertical;
    SegmentedToolBarItem flipGroup;

    ButtonedToolBarItem openFile;
    ButtonedToolBarItem saveFileAs;
    ButtonedToolBarItem deleteFile;
    ButtonedToolBarItem preferences;
    ButtonedToolBarItem exit;
};

} // namespace

// ====================================================================================================

namespace {

NSImage *NSImageForIconType(ThemeUtils::IconTypes iconType, bool darkBackground = false)
{
    const QIcon themeIcon = ThemeUtils::GetIcon(iconType);
    const QSize iconSize(16, 16);
    QImage iconImage(iconSize, QImage::Format_ARGB32_Premultiplied);
    iconImage.fill(darkBackground ? BUTTON_ALTERNATE_COLOR : BUTTON_BASE_COLOR);
    iconImage.setAlphaChannel(themeIcon.pixmap(iconSize).toImage().alphaChannel());
    return ObjCUtils::QPixmapToNSImage(QPixmap::fromImage(iconImage));
}

} // namespace

// ====================================================================================================

@implementation MacToolbarDelegate

- (id)initWithToolBarData:(ToolBarData *)toolBarData
         andEmitterObject:(QObject*)emitterObject
{
    self = [super init];
    m_toolBarData = toolBarData;
    m_emitterObject = emitterObject;

    m_toolBarData->space.item = [[NSToolbarItem alloc] initWithItemIdentifier:NSToolbarSpaceItemIdentifier];
    m_toolBarData->flexibleSpace.item = [[NSToolbarItem alloc] initWithItemIdentifier:NSToolbarFlexibleSpaceItemIdentifier];

    const NSToolbarItemVisibilityPriority low = NSToolbarItemVisibilityPriorityLow;
    const NSToolbarItemVisibilityPriority std = NSToolbarItemVisibilityPriorityStandard;
    const NSToolbarItemVisibilityPriority high = NSToolbarItemVisibilityPriorityHigh;

#define MAKE_SEGMENTED_PAIR_ITEM(GROUP, PRIORITY, ITEM1, ICON1, ITEM2, ICON2) \
    [self makeSegmentedPairItem:m_toolBarData->GROUP \
            withGroupIdentifier:@#GROUP \
         withVisibilityPriority:PRIORITY \
                  withFirstItem:m_toolBarData->ITEM1 \
            withFirstIdentifier:@#ITEM1 \
                 withFirstImage:NSImageForIconType(ThemeUtils::ICON1) \
                 withSecondItem:m_toolBarData->ITEM2 \
           withSecondIdentifier:@#ITEM2 \
                withSecondImage:NSImageForIconType(ThemeUtils::ICON2) \
    ]
#define MAKE_BUTTONED_ITEM(ITEM, PRIORITY, ICON) \
    [self makeButtonedItem:m_toolBarData->ITEM \
            withIdentifier:@#ITEM \
    withVisibilityPriority:PRIORITY \
                 withImage:NSImageForIconType(ThemeUtils::ICON, false) \
        withAlternateImage:NSImageForIconType(ThemeUtils::ICON, true) \
    ]

    MAKE_SEGMENTED_PAIR_ITEM(navigateGroup, high, navigatePrevious, ICON_LEFT, navigateNext, ICON_RIGHT);
    MAKE_BUTTONED_ITEM(startSlideShow, std, ICON_PLAY);
    MAKE_SEGMENTED_PAIR_ITEM(zoomGroup, high, zoomOut, ICON_ZOOM_OUT, zoomIn, ICON_ZOOM_IN);
    MAKE_BUTTONED_ITEM(zoomFitToWindow, high, ICON_ZOOM_EMPTY);
    MAKE_BUTTONED_ITEM(zoomOriginalSize, high, ICON_ZOOM_IDENTITY);
    MAKE_BUTTONED_ITEM(zoomFullScreen, low, ICON_FULLSCREEN);
    MAKE_SEGMENTED_PAIR_ITEM(rotateGroup, high, rotateCounterclockwise, ICON_ROTATE_COUNTERCLOCKWISE, rotateClockwise, ICON_ROTATE_CLOCKWISE);
    MAKE_SEGMENTED_PAIR_ITEM(flipGroup, std, flipHorizontal, ICON_FLIP_HORIZONTAL, flipVertical, ICON_FLIP_VERTICAL);
    MAKE_BUTTONED_ITEM(openFile, high, ICON_OPEN);
    MAKE_BUTTONED_ITEM(saveFileAs, std, ICON_SAVE_AS);
    MAKE_BUTTONED_ITEM(deleteFile, std, ICON_DELETE);
    MAKE_BUTTONED_ITEM(preferences, low, ICON_SETTINGS);
    MAKE_BUTTONED_ITEM(exit, low, ICON_EXIT);

#undef MAKE_SEGMENTED_PAIR_ITEM
#undef MAKE_BUTTONED_ITEM

    [m_toolBarData->zoomFitToWindow.button setCheckable:YES];
    [m_toolBarData->zoomOriginalSize.button setCheckable:YES];
    [m_toolBarData->zoomFullScreen.button setCheckable:YES];

    return self;
}

- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar
     itemForItemIdentifier:(NSToolbarItemIdentifier)itemIdentifier
 willBeInsertedIntoToolbar:(BOOL)flag
{
    (void)(toolbar);
    (void)(flag);
#define CHECK(MEMBER) \
    if(m_toolBarData->MEMBER.item && [itemIdentifier compare:[m_toolBarData->MEMBER.item itemIdentifier]] == NSOrderedSame) \
        return m_toolBarData->MEMBER.item
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
    CHECK(preferences);
    CHECK(exit);
#undef CHECK
    return nil;
}

- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar
{
    (void)(toolbar);
    return [NSArray arrayWithObjects:
            m_toolBarData->navigateGroup.identifier(),
            m_toolBarData->startSlideShow.identifier(),
            m_toolBarData->zoomGroup.identifier(),
            m_toolBarData->zoomFitToWindow.identifier(),
            m_toolBarData->zoomOriginalSize.identifier(),
            m_toolBarData->zoomFullScreen.identifier(),
            m_toolBarData->rotateGroup.identifier(),
            m_toolBarData->flipGroup.identifier(),
            m_toolBarData->openFile.identifier(),
            m_toolBarData->saveFileAs.identifier(),
            m_toolBarData->deleteFile.identifier(),
            m_toolBarData->preferences.identifier(),
            m_toolBarData->exit.identifier(),
            m_toolBarData->space.identifier(),
            m_toolBarData->flexibleSpace.identifier(),
            nil];
}

- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar
{
    (void)(toolbar);
    return [NSArray arrayWithObjects:
            m_toolBarData->navigateGroup.identifier(),
            m_toolBarData->startSlideShow.identifier(),
            m_toolBarData->flexibleSpace.identifier(),
            m_toolBarData->zoomGroup.identifier(),
            m_toolBarData->zoomFitToWindow.identifier(),
            m_toolBarData->zoomOriginalSize.identifier(),
            m_toolBarData->zoomFullScreen.identifier(),
            m_toolBarData->flexibleSpace.identifier(),
            m_toolBarData->rotateGroup.identifier(),
            m_toolBarData->flipGroup.identifier(),
            m_toolBarData->flexibleSpace.identifier(),
            m_toolBarData->openFile.identifier(),
            m_toolBarData->saveFileAs.identifier(),
            m_toolBarData->deleteFile.identifier(),
            m_toolBarData->flexibleSpace.identifier(),
            m_toolBarData->preferences.identifier(),
            m_toolBarData->exit.identifier(),
            nil];
}

- (NSArray *)toolbarSelectableItemIdentifiers:(NSToolbar *)toolbar
{
    (void)(toolbar);
    return nil;
}

- (IBAction)itemClicked:(id)sender
{
#define INVOKE_IF_MATCH(ITEM, METHOD) \
    if(ITEM.actionSenderMatch(sender)) \
    { \
        QMetaObject::invokeMethod(m_emitterObject, METHOD); \
        return; \
    }
    INVOKE_IF_MATCH(m_toolBarData->navigatePrevious         , "navigatePreviousRequested"       )
    INVOKE_IF_MATCH(m_toolBarData->navigateNext             , "navigateNextRequested"           )
    INVOKE_IF_MATCH(m_toolBarData->startSlideShow           , "startSlideShowRequested"         )
    INVOKE_IF_MATCH(m_toolBarData->zoomOut                  , "zoomOutRequested"                )
    INVOKE_IF_MATCH(m_toolBarData->zoomIn                   , "zoomInRequested"                 )
    INVOKE_IF_MATCH(m_toolBarData->zoomFitToWindow          , "zoomFitToWindowRequested"        )
    INVOKE_IF_MATCH(m_toolBarData->zoomOriginalSize         , "zoomOriginalSizeRequested"       )
    INVOKE_IF_MATCH(m_toolBarData->zoomFullScreen           , "zoomFullScreenRequested"         )
    INVOKE_IF_MATCH(m_toolBarData->rotateCounterclockwise   , "rotateCounterclockwiseRequested" )
    INVOKE_IF_MATCH(m_toolBarData->rotateClockwise          , "rotateClockwiseRequested"        )
    INVOKE_IF_MATCH(m_toolBarData->flipHorizontal           , "flipHorizontalRequested"         )
    INVOKE_IF_MATCH(m_toolBarData->flipVertical             , "flipVerticalRequested"           )
    INVOKE_IF_MATCH(m_toolBarData->openFile                 , "openFileRequested"               )
    INVOKE_IF_MATCH(m_toolBarData->saveFileAs               , "saveAsRequested"                 )
    INVOKE_IF_MATCH(m_toolBarData->deleteFile               , "deleteFileRequested"             )
    INVOKE_IF_MATCH(m_toolBarData->preferences              , "preferencesRequested"            )
    INVOKE_IF_MATCH(m_toolBarData->exit                     , "exitRequested"                   )
#undef INVOKE_IF_MATCH
}

- (void)makeSegmentedPairItem:(SegmentedToolBarItem &)groupItem
          withGroupIdentifier:(NSString *)groupIdentifier
       withVisibilityPriority:(NSToolbarItemVisibilityPriority)visibilityPriority
                withFirstItem:(GroupedToolBarItem &)firstItem
          withFirstIdentifier:(NSString *)firstIdentifier
               withFirstImage:(NSImage *)firstImage
               withSecondItem:(GroupedToolBarItem &)secondItem
         withSecondIdentifier:(NSString *)secondIdentifier
              withSecondImage:(NSImage *)secondImage
{
    NSToolbarItem *first = [[NSToolbarItem alloc] initWithItemIdentifier:firstIdentifier];
//    [first setTarget:self];
//    [first setAction:@selector(itemClicked:)];
    firstItem.item = first;

    NSToolbarItem *second = [[NSToolbarItem alloc] initWithItemIdentifier:secondIdentifier];
//    [second setTarget:self];
//    [second setAction:@selector(itemClicked:)];
    secondItem.item = second;

    NSSegmentedControl *segmentedControl = [[NSSegmentedControl alloc] initWithFrame:NSMakeRect(0, 0, 2 * GROUPED_BUTTON_WIDTH + SEGMENTED_OFFSET, BUTTON_HEIGHT)];
    [segmentedControl setSegmentStyle:NSSegmentStyleTexturedRounded];
    [segmentedControl setTrackingMode:NSSegmentSwitchTrackingMomentary];
    [segmentedControl setSegmentCount:2];
    [segmentedControl setImage:firstImage forSegment:0];
    [segmentedControl setWidth:GROUPED_BUTTON_WIDTH forSegment:0];
    [segmentedControl setImage:secondImage forSegment:1];
    [segmentedControl setWidth:GROUPED_BUTTON_WIDTH forSegment:1];
    groupItem.segmentedControl = segmentedControl;
    firstItem.segmentedControl = segmentedControl;
    secondItem.segmentedControl = segmentedControl;
    [segmentedControl setTarget:self];
    [segmentedControl setAction:@selector(itemClicked:)];

    NSToolbarItemGroup *group = [[NSToolbarItemGroup alloc] initWithItemIdentifier:groupIdentifier];
    [group setSubitems:[NSArray arrayWithObjects:first, second, nil]];
    [group setView:segmentedControl];
    [group setVisibilityPriority:visibilityPriority];
    groupItem.item = group;
    firstItem.group = group;
    secondItem.group = group;
}

- (void)makeButtonedItem:(ButtonedToolBarItem &)buttonedItem
          withIdentifier:(NSString *)identifier
  withVisibilityPriority:(NSToolbarItemVisibilityPriority)visibilityPriority
               withImage:(NSImage *)image
      withAlternateImage:(NSImage *)alternateImage
{
    CheckableNSButton *button = [[CheckableNSButton alloc] initWithFrame:NSMakeRect(0, 0, ALONE_BUTTON_WIDTH, BUTTON_HEIGHT)];
    [button setBezelStyle:NSBezelStyleTexturedRounded];
    [button setImage:image];
    [button setAlternateImage:alternateImage];
    [button setTitle:@""];
    [button setTarget:self];
    [button setAction:@selector(itemClicked:)];

    NSToolbarItem *item = [[NSToolbarItem alloc] initWithItemIdentifier:identifier];
    [item setView:button];
    [item setVisibilityPriority:visibilityPriority];
    buttonedItem.item = item;
    buttonedItem.button = button;
}

@end

// ====================================================================================================

struct MacToolBar::Impl
{
    MacToolBar *macToolBar;
    ToolBarData toolBarData;
    NSToolbar *nativeToolbar;
    MacToolbarDelegate *delegate;
    NSWindow *window;

    bool isSlideShowMode;

    Impl(MacToolBar *macToolBar)
        : macToolBar(macToolBar)
        , isSlideShowMode(false)
    {
        AUTORELEASE_POOL;
        nativeToolbar = [[NSToolbar alloc] initWithIdentifier:@"MacToolBar"];
        delegate = [[MacToolbarDelegate alloc] initWithToolBarData:&toolBarData andEmitterObject:macToolBar];
        window = nil;
        [nativeToolbar setDelegate:delegate];
        [nativeToolbar setAllowsUserCustomization:YES];
        [nativeToolbar setAutosavesConfiguration:YES];

        retranslate();
    }

    ~Impl()
    {
        AUTORELEASE_POOL;
        macToolBar->detachFromWindow();
        [nativeToolbar setDelegate:nil];
        [nativeToolbar release];
        [delegate release];
    }

    void retranslate()
    {
        toolBarData.navigatePrevious.setToolTip(qApp->translate("MacToolBar", "Previous", "Long"));
        toolBarData.navigateNext.setToolTip(qApp->translate("MacToolBar", "Next", "Long"));
        const QString navigateGroupText = qApp->translate("MacToolBar", "Navigate", "Short");
        toolBarData.navigateGroup.setPaletteLabel(navigateGroupText);
        toolBarData.navigateGroup.setLabel(navigateGroupText);

        const QString slideShowShortText = qApp->translate("MacToolBar", "Slideshow", "Short");
        toolBarData.startSlideShow.setPaletteLabel(slideShowShortText);
        toolBarData.startSlideShow.setLabel(slideShowShortText);

        toolBarData.zoomOut.setToolTip(qApp->translate("MacToolBar", "Zoom Out", "Long"));
        toolBarData.zoomIn.setToolTip(qApp->translate("MacToolBar", "Zoom In", "Long"));
        const QString zoomGroupText = qApp->translate("MacToolBar", "Zoom", "Short");
        toolBarData.zoomGroup.setPaletteLabel(zoomGroupText);
        toolBarData.zoomGroup.setLabel(zoomGroupText);

        const QString zoomFitToWindowFullText = qApp->translate("MacToolBar", "Fit Image To Window Size", "Long");
        const QString zoomFitToWindowShortText = qApp->translate("MacToolBar", "Fit Image To Window Size", "Short");
        toolBarData.zoomFitToWindow.setPaletteLabel(zoomFitToWindowShortText);
        toolBarData.zoomFitToWindow.setToolTip(zoomFitToWindowFullText);
        toolBarData.zoomFitToWindow.setLabel(zoomFitToWindowShortText);

        const QString zoomOriginalSizeFullText = qApp->translate("MacToolBar", "Original Size", "Long");
        const QString zoomOriginalSizeShortText = qApp->translate("MacToolBar", "Original Size", "Short");
        toolBarData.zoomOriginalSize.setPaletteLabel(zoomOriginalSizeShortText);
        toolBarData.zoomOriginalSize.setToolTip(zoomOriginalSizeFullText);
        toolBarData.zoomOriginalSize.setLabel(zoomOriginalSizeShortText);

        const QString zoomFullScreenFullText = qApp->translate("MacToolBar", "Full Screen", "Long");
        const QString zoomFullScreenShortText = qApp->translate("MacToolBar", "Full Screen", "Short");
        toolBarData.zoomFullScreen.setPaletteLabel(zoomFullScreenShortText);
        toolBarData.zoomFullScreen.setToolTip(zoomFullScreenFullText);
        toolBarData.zoomFullScreen.setLabel(zoomFullScreenShortText);

        toolBarData.rotateCounterclockwise.setToolTip(qApp->translate("MacToolBar", "Rotate Counterclockwise", "Long"));
        toolBarData.rotateClockwise.setToolTip(qApp->translate("MacToolBar", "Rotate Clockwise", "Long"));
        const QString rotateGroupText = qApp->translate("MacToolBar", "Rotate", "Short");
        toolBarData.rotateGroup.setPaletteLabel(rotateGroupText);
        toolBarData.rotateGroup.setLabel(rotateGroupText);

        toolBarData.flipHorizontal.setToolTip(qApp->translate("MacToolBar", "Flip Horizontal", "Long"));
        toolBarData.flipVertical.setToolTip(qApp->translate("MacToolBar", "Flip Vertical", "Long"));
        const QString flipGroupText = qApp->translate("MacToolBar", "Flip", "Short");
        toolBarData.flipGroup.setPaletteLabel(flipGroupText);
        toolBarData.flipGroup.setLabel(flipGroupText);

        const QString openFileFullText = qApp->translate("MacToolBar", "Open File", "Long");
        const QString openFileShortText = qApp->translate("MacToolBar", "Open File", "Short");
        toolBarData.openFile.setPaletteLabel(openFileShortText);
        toolBarData.openFile.setToolTip(openFileFullText);
        toolBarData.openFile.setLabel(openFileShortText);
        const QString saveFileAsFullText = qApp->translate("MacToolBar", "Save File As", "Long");
        const QString saveFileAsShortText = qApp->translate("MacToolBar", "Save File As", "Short");
        toolBarData.saveFileAs.setPaletteLabel(saveFileAsShortText);
        toolBarData.saveFileAs.setToolTip(saveFileAsFullText);
        toolBarData.saveFileAs.setLabel(saveFileAsShortText);
        const QString deleteFileFullText = qApp->translate("MacToolBar", "Delete File", "Long");
        const QString deleteFileShortText = qApp->translate("MacToolBar", "Delete File", "Short");
        toolBarData.deleteFile.setPaletteLabel(deleteFileShortText);
        toolBarData.deleteFile.setToolTip(deleteFileFullText);
        toolBarData.deleteFile.setLabel(deleteFileShortText);
        const QString preferencesFullText = qApp->translate("MacToolBar", "Preferences", "Long");
        const QString preferencesShortText = qApp->translate("MacToolBar", "Preferences", "Short");
        toolBarData.preferences.setPaletteLabel(preferencesShortText);
        toolBarData.preferences.setToolTip(preferencesFullText);
        toolBarData.preferences.setLabel(preferencesShortText);
        const QString exitFullText = qApp->translate("MacToolBar", "Exit", "Long");
        const QString exitShortText = qApp->translate("MacToolBar", "Exit", "Short");
        toolBarData.exit.setPaletteLabel(exitShortText);
        toolBarData.exit.setToolTip(exitFullText);
        toolBarData.exit.setLabel(exitShortText);

        setSlideShowMode(isSlideShowMode);
    }

    void setSlideShowMode(bool isSlideShow)
    {
        AUTORELEASE_POOL;
        isSlideShowMode = isSlideShow;
        NSButton *button = toolBarData.startSlideShow.button;
        if(!button)
            return;
        if(!isSlideShowMode)
        {
            [button setImage:NSImageForIconType(ThemeUtils::ICON_PLAY)];
            toolBarData.startSlideShow.setToolTip(qApp->translate("MacToolBar", "Start Slideshow", "Long"));
        }
        else
        {
            [button setImage:NSImageForIconType(ThemeUtils::ICON_STOP)];
            toolBarData.startSlideShow.setToolTip(qApp->translate("MacToolBar", "Stop Slideshow", "Long"));
        }
    }
};

// ====================================================================================================

MacToolBar::MacToolBar(QObject *parent)
    : ControlsContainerEmitter(parent)
    , m_impl(new Impl(this))
{
    connect(LocalizationManager::instance(), SIGNAL(localeChanged(const QString&)), this, SLOT(retranslate()));
}

MacToolBar::~MacToolBar()
{}

ControlsContainerEmitter *MacToolBar::emitter()
{
    return this;
}

void MacToolBar::attachToWindow(QWidget *widget)
{
    AUTORELEASE_POOL;
    detachFromWindow();
    NSView *windowView = reinterpret_cast<NSView*>(widget->window()->winId());
    NSWindow *window = reinterpret_cast<NSWindow*>([windowView window]);
    [window setToolbar:m_impl->nativeToolbar];
//    [window setShowsToolbarButton:YES];
    m_impl->window = window;
}

void MacToolBar::detachFromWindow()
{
    if(!m_impl->window)
        return;
    AUTORELEASE_POOL;
    [m_impl->window setToolbar:nil];
    m_impl->window = nil;
}

void MacToolBar::retranslate()
{
    m_impl->retranslate();
}

CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacToolBar, setOpenFileEnabled, &m_impl->toolBarData.openFile)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setOpenFolderEnabled)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacToolBar, setSaveAsEnabled, &m_impl->toolBarData.saveFileAs)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setNewWindowEnabled)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacToolBar, setNavigatePreviousEnabled, &m_impl->toolBarData.navigatePrevious)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacToolBar, setNavigateNextEnabled, &m_impl->toolBarData.navigateNext)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacToolBar, setStartSlideShowEnabled, &m_impl->toolBarData.startSlideShow)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacToolBar, setPreferencesEnabled, &m_impl->toolBarData.preferences)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacToolBar, setExitEnabled, &m_impl->toolBarData.exit)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacToolBar, setRotateCounterclockwiseEnabled, &m_impl->toolBarData.rotateCounterclockwise)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacToolBar, setRotateClockwiseEnabled, &m_impl->toolBarData.rotateClockwise)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacToolBar, setFlipHorizontalEnabled, &m_impl->toolBarData.flipHorizontal)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacToolBar, setFlipVerticalEnabled, &m_impl->toolBarData.flipVertical)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacToolBar, setDeleteFileEnabled, &m_impl->toolBarData.deleteFile)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacToolBar, setZoomOutEnabled, &m_impl->toolBarData.zoomOut)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacToolBar, setZoomInEnabled, &m_impl->toolBarData.zoomIn)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setZoomResetEnabled)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacToolBar, setZoomFitToWindowEnabled, &m_impl->toolBarData.zoomFitToWindow)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacToolBar, setZoomOriginalSizeEnabled, &m_impl->toolBarData.zoomOriginalSize)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MacToolBar, setZoomFullScreenEnabled, &m_impl->toolBarData.zoomFullScreen)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setShowMenuBarEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setShowToolBarEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setAboutEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setAboutQtEnabled)

CONTROLS_CONTAINER_SET_CHECKED_IMPL(MacToolBar, setZoomFitToWindowChecked, &m_impl->toolBarData.zoomFitToWindow)
CONTROLS_CONTAINER_SET_CHECKED_IMPL(MacToolBar, setZoomOriginalSizeChecked, &m_impl->toolBarData.zoomOriginalSize)
CONTROLS_CONTAINER_SET_CHECKED_IMPL(MacToolBar, setZoomFullScreenChecked, &m_impl->toolBarData.zoomFullScreen)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setShowMenuBarChecked)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setShowToolBarChecked)

CONTROLS_CONTAINER_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setSlideShowMode, m_impl, setSlideShowMode)

// ====================================================================================================

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

#include "ObjectsConnectorIDs.h"

// cat ObjectsConnectorIDs.h | sed 's|^extern || ; s|\([^ ]*\);$|\1 = "\1";|'

const char * const UI_STATE_CHANGED_ID = "UI_STATE_CHANGED_ID";

const char * const OPEN_NEW_WINDOW_ID = "OPEN_NEW_WINDOW_ID";
const char * const CLOSE_MAIN_WINDOW_ID = "CLOSE_MAIN_WINDOW_ID";
const char * const QUIT_APPLICATION_ID = "QUIT_APPLICATION_ID";

const char * const OPEN_SINGLE_PATH_ID = "OPEN_SINGLE_PATH_ID";
const char * const OPEN_MULTIPLE_PATHS_ID = "OPEN_MULTIPLE_PATHS_ID";
const char * const OPEN_FILE_WITH_DIALOG_ID = "OPEN_FILE_WITH_DIALOG_ID";
const char * const SAVE_AS_ID = "SAVE_AS_ID";
const char * const DELETE_FILE_ID = "DELETE_FILE_ID";

const char * const ZOOM_IN_ID = "ZOOM_IN_ID";
const char * const ZOOM_OUT_ID = "ZOOM_OUT_ID";
const char * const ZOOM_DEFAULT_ID = "ZOOM_DEFAULT_ID";
const char * const ZOOM_FIT_TO_WINDOW_ID = "ZOOM_FIT_TO_WINDOW_ID";
const char * const ZOOM_ORIGINAL_SIZE_ID = "ZOOM_ORIGINAL_SIZE_ID";
const char * const SWITCH_FULLSCREEN_ID = "SWITCH_FULLSCREEN_ID";

const char * const SELECT_PREVIOUS_ID = "SELECT_PREVIOUS_ID";
const char * const SELECT_NEXT_ID = "SELECT_NEXT_ID";
const char * const SELECT_FIRST_ID = "SELECT_FIRST_ID";
const char * const SELECT_LAST_ID = "SELECT_LAST_ID";

const char * const ROTATE_CCW_ID = "ROTATE_CCW_ID";
const char * const ROTATE_CW_ID = "ROTATE_CW_ID";
const char * const FLIP_HORIZONTAL_ID = "FLIP_HORIZONTAL_ID";
const char * const FLIP_VERTICAL_ID = "FLIP_VERTICAL_ID";

const char * const SWITCH_SLIDESHOW_MODE_ID = "SWITCH_SLIDESHOW_MODE_ID";

const char * const SHOW_PREFERENCES_ID = "SHOW_PREFERENCES_ID";
const char * const SHOW_ABOUT_ID = "SHOW_ABOUT_ID";
const char * const SHOW_ABOUT_QT_ID = "SHOW_ABOUT_QT_ID";

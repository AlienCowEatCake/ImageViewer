/*
    SPDX-FileCopyrightText: 2022 Albert Astals Cid <aacid@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <limits>

// QVector uses some extra space for stuff, hence the 32 here suggested by Thiago Macieira
static constexpr int kMaxQVectorSize = std::numeric_limits<int>::max() - 32;

/*=============================================================================
 * TarotClub - defines.h
 *=============================================================================
 * Global types
 *=============================================================================
 * TarotClub ( http://www.tarotclub.fr ) - This file is part of TarotClub
 * Copyright (C) 2003-2999 - Anthony Rabine
 * anthony@tarotclub.fr
 *
 * TarotClub is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TarotClub is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TarotClub.  If not, see <http://www.gnu.org/licenses/>.
 *
 *=============================================================================
 */

#ifndef _DEFINES_H
#define _DEFINES_H

#include <QtCore>


/*****************************************************************************/
// Game definitions

/**
 * @brief The version string uses Semantic Versioning format
 * @see http://semver.org
 */
#define TAROT_VERSION   "2.3.0-alpha.1"
#define TAROT_TITRE     "TarotClub"
#define TAROT_VNAME     "Christmas"

#define NB_LANGUAGE     2
#define QT_STREAMVER    QDataStream::Qt_5_1
#define MAX_ROUNDS      5

namespace Config
{

#ifdef QT_DEBUG
const QString HomePath  = "./";
#else
const QString HomePath  = QDir::homePath() + "/.tarotclub/";
#endif
const QString GamePath  = HomePath + "/games/";
const QString LogPath   = HomePath + "/logs/";
}

/*****************************************************************************/
enum Place      { SOUTH = 0, EAST = 1, NORTH = 2, WEST = 3, FIFTH = 4, ALL = 5, NOWHERE = 0xFF };
enum Contract   { PASS = 0, TAKE = 1, GUARD = 2, GUARD_WITHOUT = 3, GUARD_AGAINST = 4 };
enum Team       { ATTACK = 0, DEFENSE = 1, NO_TEAM = 0xFF };
enum Handle     { SIMPLE_HANDLE = 0, DOUBLE_HANDLE = 1, TRIPLE_HANDLE = 2 };
/*****************************************************************************/

#endif // _DEFINES_H

//=============================================================================
// End of file defines.h
//=============================================================================

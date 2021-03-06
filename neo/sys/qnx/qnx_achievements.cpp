/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2014 Vincent Simonetti

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/
#pragma hdrstop
#include "../../idlib/precompiled.h"

#include "qnx_achievements.h"
#include "../sys_session_local.h"

extern idCVar achievements_Verbose;

/*
========================
idAchievementSystemQnx::idAchievementSystemQnx
========================
*/
idAchievementSystemQnx::idAchievementSystemQnx() {
}

/*
========================
idAchievementSystemQnx::IsInitialized
========================
*/
bool idAchievementSystemQnx::IsInitialized() {
	return false;
}

/*
================================
idAchievementSystemQnx::AchievementUnlock
================================
*/
void idAchievementSystemQnx::AchievementUnlock( idLocalUser * user, int achievementID ) {
#ifdef USE_SOCIAL
	char achievementIdStr[32];
	int pos = idStr::snPrintf( achievementIdStr, sizeof( achievementIdStr ) - 1, "com.rebuild.doom3bfg.award%d", achievementID );
	if ( pos >= 0 ) {
		achievementIdStr[pos] = '\0';
		//TODO
	}
#endif
}

/*
========================
idAchievementSystemQnx::AchievementLock
========================
*/
void idAchievementSystemQnx::AchievementLock( idLocalUser * user, const int achievementID ) {
}

/*
========================
idAchievementSystemQnx::AchievementLockAll
========================
*/
void idAchievementSystemQnx::AchievementLockAll( idLocalUser * user, const int maxId ) {
}

/*
========================
idAchievementSystemQnx::GetAchievementDescription
========================
*/
bool idAchievementSystemQnx::GetAchievementDescription( idLocalUser * user, const int achievementID, achievementDescription_t & data ) const {
	return false;
}

/*
========================
idAchievementSystemQnx::GetAchievementState
========================
*/
bool idAchievementSystemQnx::GetAchievementState( idLocalUser * user, idArray< bool, idAchievementSystem::MAX_ACHIEVEMENTS > & achievements ) const {
	return false;
}

/*
================================
idAchievementSystemQnx::Pump
================================
*/
void idAchievementSystemQnx::Pump() {
}

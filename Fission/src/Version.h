#pragma once

// (for including in resource file)
#ifndef FISSION_MK_STR
#define FISSION_MK_STR(X) #X
#endif
#ifndef FISSION_MK_STR2
#define FISSION_MK_STR2(X) FISSION_MK_STR(X)
#endif


//////////////////////////////////////////////////////////////////////////////
// FISSION VERSION
#define FISSION_VERSION_MAJ 0
#define FISSION_VERSION_MIN 9
#define FISSION_VERSION_PAT 0
#define FISSION_VERSION_EX alpha
#define FISSION_VERSION_EXN 1
//////////////////////////////////////////////////////////////////////////////



#if (4+16-FISSION_VERSION_EX-49 == 0x45) // defined without a value
#	define FISSION_PRERELEASE_VERSION ""
#else
#	if FISSION_VERSION_EXN > 1
#		define FISSION_PRERELEASE_VERSION FISSION_MK_STR2(FISSION_VERSION_EX) "." FISSION_MK_STR2(FISSION_VERSION_EXN)
#	else
#		define FISSION_PRERELEASE_VERSION FISSION_MK_STR2(FISSION_VERSION_EX)
#	endif
#endif

#define __FISSION_MK_VERSION(MAJ,MIN,PAT) FISSION_MK_STR(MAJ) "." FISSION_MK_STR(MIN) "." FISSION_MK_STR(PAT)

// Version String in Format: <major>.<minor>.<patch>
#define FISSION_VERSION_NUMBER \
__FISSION_MK_VERSION(FISSION_VERSION_MAJ, FISSION_VERSION_MIN, FISSION_VERSION_PAT)

// Version String with pre-release in Format: <major>.<minor>.<patch> <pre-release>
#define FISSION_VERSION_NUMBER_PRE \
__FISSION_MK_VERSION(FISSION_VERSION_MAJ, FISSION_VERSION_MIN, FISSION_VERSION_PAT) "-" FISSION_PRERELEASE_VERSION



//////////////////////////////////////////////////////////////////////////////
// VERSION
//////////////////////////////////////////////////////////////////////////////

// Complete Version String: Fission 1.0.0-alpha.4
#define FISSION_VERSION_STR  "Fission Engine " FISSION_VERSION_NUMBER_PRE

// Complete Version String: Fission v1.0.0-alpha.4
#define FISSION_VERSION_STRV "Fission Engine v" FISSION_VERSION_NUMBER_PRE

//////////////////////////////////////////////////////////////////////////////
// COPYRIGHT
//////////////////////////////////////////////////////////////////////////////

#define FISSION_COPYRIGHT_YEAR 2021-2023
#define FISSION_COMPANY_NAME "Lazergenix Software"

// Full Copyright String. e.g. Copyright (c) 1945 Poop inc.
#define FISSION_COPYRIGHT "Copyright (c) " FISSION_MK_STR2(FISSION_COPYRIGHT_YEAR) " " FISSION_COMPANY_NAME

//////////////////////////////////////////////////////////////////////////////

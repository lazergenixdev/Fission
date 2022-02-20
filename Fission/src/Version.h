#pragma once

// (for including in resource file)
#ifndef FISSION_MK_STR
#define FISSION_MK_STR(X) #X
#endif
#ifndef FISSION_ENGINE
#define FISSION_ENGINE "Fission Engine"
#endif

//////////////////////////////////////////////////////////////////////////////
// Version Elements
#define FISSION_VERSION_MAJ 0
#define FISSION_VERSION_MIN 7
#define FISSION_VERSION_PAT 1
#define FISSION_VERSION_EX "beta"
//////////////////////////////////////////////////////////////////////////////

#define _FISSION_VERSION_STRING(MAJ,MIN,PAT) FISSION_MK_STR(MAJ) "." FISSION_MK_STR(MIN) "." FISSION_MK_STR(PAT)

// Version String in Format: <major>.<minor>.<patch>
#define FISSION_VERSION_STRING \
_FISSION_VERSION_STRING(FISSION_VERSION_MAJ, FISSION_VERSION_MIN, FISSION_VERSION_PAT)

// Full Version String in Format: <major>.<minor>.<patch> <prebuild>
#define FISSION_FULL_VERSION_STRING \
_FISSION_VERSION_STRING(FISSION_VERSION_MAJ, FISSION_VERSION_MIN, FISSION_VERSION_PAT) " " FISSION_VERSION_EX

// Copyright Year: XXXX
#define FISSION_COPYRIGHT_YEAR "2021"

#define FISSION_COMPANY_NAME "Lazergenix Software"

// Full Copyright String. Example: Copyright (c) 1945 Poop inc.
#define FISSION_COPYRIGHT "Copyright (c) " FISSION_COPYRIGHT_YEAR " " FISSION_COMPANY_NAME


//////////////////////////////////////////////////////////////////////////////
// Common

// Complete Version String: Fission 1.0.0 alpha
#define FISSION_COMPLETE_VERSION_STRING   FISSION_ENGINE " " FISSION_FULL_VERSION_STRING

// Complete Version String: Fission v1.0.0 alpha
#define FISSION_COMPLETE_VERSION_STRING_V FISSION_ENGINE " v" FISSION_FULL_VERSION_STRING

//////////////////////////////////////////////////////////////////////////////

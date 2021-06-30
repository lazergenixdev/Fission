#pragma once

#ifndef FISSION_MK_STR
#define FISSION_MK_STR(X) #X
#endif

#ifndef FISSION_ENGINE
#define FISSION_ENGINE "Fission Engine"
#endif

#define FISSION_VERSION_MAJ 0
#define FISSION_VERSION_MIN 6
#define FISSION_VERSION_PAT 2

#define _FISSION_VERSION_STRING(MAJ,MIN,PAT) FISSION_MK_STR(MAJ) "." FISSION_MK_STR(MIN) "." FISSION_MK_STR(PAT)

#define FISSION_VERSION_STRING _FISSION_VERSION_STRING(FISSION_VERSION_MAJ, FISSION_VERSION_MIN, FISSION_VERSION_PAT)

#define FISSION_COPYRIGHT_YEAR "2021"

#define FISSION_COMPANY_NAME "Lazergenix Software"

#define FISSION_COPYRIGHT "Copyright (c) " FISSION_COPYRIGHT_YEAR " " FISSION_COMPANY_NAME

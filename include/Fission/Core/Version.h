
#pragma once
#include "Fission/config.h"

namespace Fission 
{

/**
 * @brief  Get Fission Engine Version
 * 
 * @warning NO ARGUMENTS PASSED SHOULD BE NULL
 * 
 * @param  _Maj: [out] Major Version Number.
 * @param  _Min: [out] Minor Version Number.
 * @param  _Pat: [out] Patch Version Number.
 */
FISSION_API void GetVersion( int *_Maj, int *_Min, int *_Pat );

}

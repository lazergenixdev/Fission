#include <Fission/Core/Version.h>
#include "Version.h"


namespace Fission {

	void GetVersion( int * maj, int * min, int * pat )
	{
		*maj = FISSION_VERSION_MAJ;
		*min = FISSION_VERSION_MIN;
		*pat = FISSION_VERSION_PAT;
	}

	void GetVersionString( wchar_t * _Buffer )
	{
		lstrcpyW( _Buffer, L"" FISSION_ENGINE " v" FISSION_VERSION_STRING );
	}

	void GetVersionString( char * _Buffer )
	{
		strcpy( _Buffer, "" FISSION_ENGINE " v" FISSION_VERSION_STRING );
	}
}

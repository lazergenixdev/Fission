#include <Fission/Core/Version.h>
#include "Version.h"


namespace Fission {

	void GetVersion( int * maj, int * min, int * pat )
	{
		*maj = FISSION_VERSION_MAJ;
		*min = FISSION_VERSION_MIN;
		*pat = FISSION_VERSION_PAT;
	}

}

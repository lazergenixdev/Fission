#include "Fission/Core/Sound.h"

#ifdef FISSION_PLATFORM_WINDOWS
#include "Platform/Windows/SoundXAudio2.h"
#endif

namespace Fission
{

	ref<SoundEngine> SoundEngine::Create( const CreateInfo & info )
	{
		return CreateRef<Platform::SoundEngineXAudio2>( info );
	}

}
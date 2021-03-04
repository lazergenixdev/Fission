#pragma once
#include "Fission/config.h"

namespace Fission {

	struct ISound
	{
	public:
		virtual bool empty() = 0;
		virtual uint32_t length() = 0;
	};

	struct ISoundSource
	{
	public:
		virtual void SetPosition( uint32_t position ) = 0;
		virtual uint32_t GetPosition() = 0;

		virtual void SetPlaying( bool playing ) = 0;
		virtual bool GetPlaying() = 0;
	};

	using OutputID = short;

	struct ISoundEngine
	{
	public:
		struct CreateInfo
		{
			short nOutputs = 1;
		};

		FISSION_API static std::unique_ptr<ISoundEngine> Create( const CreateInfo & info = {} );

		virtual ISound * CreateSound( const wchar_t * file ) = 0;

		virtual ISoundSource * Play( ISound * sound, OutputID _Output, bool start_playing = true ) = 0;

		virtual void SetVolume( OutputID _Output, float value ) = 0;
		virtual float GetVolume( OutputID _Output ) = 0;

		virtual void SetMasterVolume( float value ) = 0;
		virtual float GetMasterVolume() = 0;
	};

} // namespace Fission

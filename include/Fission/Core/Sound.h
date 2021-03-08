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

	namespace Sound {
		typedef short Output;
		typedef Output OutputID;
	}

	struct ISoundEngine
	{
	public:
		struct CreateInfo
		{
			Sound::Output nOutputs = 1;
		};

		FISSION_API static std::unique_ptr<ISoundEngine> Create( const CreateInfo & info = {} );

		virtual ISound * CreateSound( const wchar_t * file ) = 0;

		virtual ISoundSource * Play( ISound * _Sound, Sound::OutputID _Output, bool _Start_Playing = true ) = 0;

		virtual void SetVolume( Sound::OutputID _Output, float _Volume ) = 0;
		virtual float GetVolume( Sound::OutputID _Output ) = 0;

		virtual void SetMasterVolume( float _Volume ) = 0;
		virtual float GetMasterVolume() = 0;

	};

} // namespace Fission

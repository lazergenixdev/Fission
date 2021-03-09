#pragma once
#include "Fission/config.h"

namespace Fission {

	struct ISound
	{
	public:
		virtual bool empty() = 0;
		virtual uint32_t length() = 0;

		virtual ~ISound() = default;
	};

	struct ISoundSource
	{
	public:
		virtual void SetPosition( uint32_t position ) = 0;
		virtual uint32_t GetPosition() = 0;

		virtual void SetPlaying( bool playing ) = 0;
		virtual bool GetPlaying() = 0;

		virtual ~ISoundSource() = default;
	};

	namespace Sound {
		typedef short Output;
		typedef Output OutputID;
	}

	struct SoundEngine
	{
	public:
		struct CreateInfo
		{
			Sound::Output nOutputs = 1;
		};

		FISSION_API static ref<SoundEngine> Create( const CreateInfo & info = {} );

		virtual ref<ISound> CreateSound( const file::path & filepath ) = 0;

		virtual ref<ISoundSource> Play( ISound * _Sound, Sound::OutputID _Output, bool _Play_Looped = false, bool _Start_Playing = true, bool _Track = false ) = 0;

		virtual void SetVolume( Sound::OutputID _Output, float _Volume ) = 0;
		virtual float GetVolume( Sound::OutputID _Output ) = 0;

		virtual void SetMasterVolume( float _Volume ) = 0;
		virtual float GetMasterVolume() = 0;

		virtual ~SoundEngine() = default;

	};

} // namespace Fission

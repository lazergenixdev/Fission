#pragma once
#include "LazerEngine/config.h"

namespace lazer {

namespace experimental {

	interface ISound
	{
	public:
		virtual bool empty() = 0;
		virtual uint32_t length() = 0;
	};

	interface ISoundSource
	{
	public:
		virtual void SetPosition( uint32_t position ) = 0;
		virtual uint32_t GetPosition() = 0;

		virtual void SetPlaying( bool playing ) = 0;
		virtual bool GetPlaying() = 0;
	};

	using ChannelID = short;

	interface ISoundEngine
	{
	public:
		LAZER_API static std::unique_ptr<ISoundEngine> Create();

		// Creates sound channels
		virtual void Configure( ChannelID * ch_ids, int count ) = 0;

		virtual ISound * CreateSound( const wchar_t * file ) = 0;

		virtual ISoundSource * Play( ISound * sound, ChannelID ch_id, bool start_playing = true ) = 0;

		virtual void SetVolume( ChannelID ch_id, float value ) = 0;
		virtual float GetVolume( ChannelID ch_id ) = 0;

		virtual void SetMasterVolume( float value ) = 0;
		virtual float GetMasterVolume() = 0;
	};

}

} // namespace lazer

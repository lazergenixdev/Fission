#pragma once
#include "Fission/Core/Sound.h"
#include <xaudio2.h>

namespace Fission::Platform
{
	struct SoundData
	{
		std::vector<byte> m_samples;
		WAVEFORMATEX m_format;
	};

	struct SoundXAudio2 : public ISound
	{
	public:
		virtual bool empty() override;
		virtual uint32_t length() override;

		SoundData m_Sound;
	};

	struct SoundSourceXAudio2 : public ISoundSource
	{
	public:
		SoundSourceXAudio2( SoundXAudio2 * sound, IXAudio2 * engine, IXAudio2SubmixVoice * pOutput );
		~SoundSourceXAudio2();

		virtual void SetPosition( uint32_t position ) override {}
		virtual uint32_t GetPosition() override { return 0u; }

		virtual void SetPlaying( bool playing ) override;
		virtual bool GetPlaying() override;

	private:
		IXAudio2SourceVoice * m_pVoice;
		bool m_bPlaying;
	};

	struct SoundEngineXAudio2 : public SoundEngine
	{
	public:

		SoundEngineXAudio2( const CreateInfo & info );
		virtual ~SoundEngineXAudio2();

		virtual ref<ISound> CreateSound( const file::path & filepath ) override;

		virtual ref<ISoundSource> Play(
			ISound * _Sound, 
			Sound::OutputID _Output, 
			bool _Play_Looped = false, 
			bool _Start_Playing = true, 
			bool _Track = false ) override;

		virtual void SetVolume( Sound::OutputID _Output, float _Volume ) override;
		virtual float GetVolume( Sound::OutputID _Output ) override;

		virtual void SetMasterVolume( float _Volume ) override;
		virtual float GetMasterVolume() override;

	private:

		std::vector<IXAudio2SubmixVoice *> m_vpSubmixVoices;
		Microsoft::WRL::ComPtr<IXAudio2> m_pXAudio2Engine;
		IXAudio2MasteringVoice * m_pMaster = nullptr;
	};
}
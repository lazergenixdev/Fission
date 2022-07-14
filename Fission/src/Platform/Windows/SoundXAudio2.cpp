#include "SoundXAudio2.h"
#include <Fission/Core/Console.hh>
#include <Fission/Base/Exception.hpp>

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <stdio.h>
#include <mferror.h>

// Audio Reading
#pragma comment( lib, "mfplat" )
#pragma comment( lib, "mfreadwrite" )
#pragma comment( lib, "mfuuid" )

namespace Fission::Platform {


	enum class LoadSoundResult { Success, Failure, };

	LoadSoundResult LoadSoundDataFromFile( const std::filesystem::path & filepath, SoundData * sd );


SoundEngineXAudio2::SoundEngineXAudio2( const CreateInfo & info )
{
	HRESULT hr;
	// todo: move this.
	hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_SPEED_OVER_MEMORY | COINIT_DISABLE_OLE1DDE);

	if( FAILED( hr = XAudio2Create( &m_pXAudio2Engine, 0, XAUDIO2_ANY_PROCESSOR ) ) )
		FISSION_THROW( "XAudio2 Exception", .append( "Failed to Initialize Sound Engine." ) );

	if( FAILED( hr = m_pXAudio2Engine->CreateMasteringVoice( &m_pMaster ) ) )
		FISSION_THROW( "XAudio2 Exception", .append( "Failed to Create Mastering Voice." ) );

	m_vpSubmixVoices.resize( info.nOutputs );
	for( auto && submix : m_vpSubmixVoices )
	{
		if( FAILED( hr = m_pXAudio2Engine->CreateSubmixVoice( &submix, 2, 44100 ) ) )
			FISSION_THROW( "XAudio2 Exception", .append( "Failed to Create Submix Voice." ) );
	}

	if( FAILED(hr = m_pXAudio2Engine->StartEngine() ) )
		FISSION_THROW( "XAudio2 Exception", .append( "Failed to start audio processing thread." ) );

//	Console::WriteLine( colors::Blanchedalmond, "Using XAudio2" );
}

ref<Fission::ISound> SoundEngineXAudio2::CreateSound( const std::filesystem::path & filepath )
{
	auto sound = make_ref<SoundXAudio2>();
	// TODO: this is terrible code.
	LoadSoundDataFromFile( filepath, &sound->m_Sound );
	return sound;
}

ref<ISoundSource> SoundEngineXAudio2::Play( ISound * _Sound, Sound::OutputID _Output, bool _Play_Looped, bool _Start_Playing, bool _Track )
{
	SoundSourceXAudio2::CreateInfo info = {};
	info.bIsLooped = _Play_Looped;
	info.bStartPlaying = _Start_Playing;
	info.bTrack = _Track;
	info.sound = static_cast<SoundXAudio2 *>( _Sound );
	return make_ref<SoundSourceXAudio2>( m_pXAudio2Engine.Get(), m_vpSubmixVoices[_Output], info );
}

void SoundEngineXAudio2::SetVolume( Sound::OutputID _Output, float _Volume )
{
	m_vpSubmixVoices[_Output]->SetVolume( _Volume );
}

float SoundEngineXAudio2::GetVolume( Sound::OutputID _Output )
{
	float _Volume;
	m_vpSubmixVoices[_Output]->GetVolume( &_Volume );
	return _Volume;
}

void SoundEngineXAudio2::SetMasterVolume( float _Volume )
{
	m_pMaster->SetVolume( _Volume );
}

float SoundEngineXAudio2::GetMasterVolume()
{
	float _Volume;
	m_pMaster->GetVolume( &_Volume );
	return _Volume;
}

SoundEngineXAudio2::~SoundEngineXAudio2()
{
	if( m_pXAudio2Engine )
	m_pXAudio2Engine->StopEngine();
	if( m_pMaster )
	m_pMaster->DestroyVoice();
}

bool SoundXAudio2::empty()
{
	return m_Sound.m_samples.empty();
}

uint32_t SoundXAudio2::length()
{
	if( m_Sound.m_samples.empty() ) return 0u;
	uint64_t nBitsPSample = m_Sound.m_format.wBitsPerSample;
	uint64_t nBytes = m_Sound.m_samples.size();
	uint64_t nTotalSamplesPSec = (uint64_t)m_Sound.m_format.nSamplesPerSec * (uint64_t)m_Sound.m_format.nChannels;
	return uint32_t( 8000u * ( nBytes / nBitsPSample ) / nTotalSamplesPSec );
}

void SoundSourceXAudio2::SetPlaying( bool play )
{
	HRESULT hr;
	if( play && !m_bPlaying )
	{
		if( !m_bIsConsuming )
		{
			hr = m_pVoice->SubmitSourceBuffer( &m_XAudioBuffer );
			m_bIsConsuming = true;
		}
		m_bPlaying = true;
		SetPosition( GetPosition() );
		//hr = m_pVoice->Start();
	}
	else if( m_bPlaying )
	{
		hr = m_pVoice->Stop();
		m_bPlaying = false;
	}
}

bool SoundSourceXAudio2::GetPlaying()
{
	return m_bPlaying;
}

void SoundSourceXAudio2::ChannelCallback::OnBufferStart( void * pBufferContext )
{
	//Console::Message( L"XAudio2::OnBufferStart" );
	auto pSource = reinterpret_cast<SoundSourceXAudio2 *>( pBufferContext );
	pSource->m_bPlaying = true;
	pSource->m_bIsConsuming = true;
}

void SoundSourceXAudio2::ChannelCallback::OnBufferEnd( void * pBufferContext )
{
	//Console::Message( L"XAudio2::OnBufferEnd" );
	auto pSource = reinterpret_cast<SoundSourceXAudio2 *>( pBufferContext );
	pSource->m_bPlaying = false;
	pSource->m_bIsConsuming = false;
}

SoundSourceXAudio2::SoundSourceXAudio2( IXAudio2 * engine, IXAudio2SubmixVoice * pOutput, const CreateInfo & info )
	: m_bPlaying( false ), m_bIsConsuming( false ), m_pVoice( nullptr ), m_pSound( info.sound ), m_XAudioBuffer()
{
	if( m_pSound->empty() ) return;
	XAUDIO2_SEND_DESCRIPTOR Send = { 0, pOutput };
	XAUDIO2_VOICE_SENDS SendList = { 1, &Send };

	static ChannelCallback callback;

	engine->CreateSourceVoice( &m_pVoice, &m_pSound->m_Sound.m_format,
		0, XAUDIO2_DEFAULT_FREQ_RATIO, &callback, &SendList );

	m_XAudioBuffer.pContext = this;
	m_XAudioBuffer.pAudioData = m_pSound->m_Sound.m_samples.data();
	m_XAudioBuffer.AudioBytes = (uint32_t)m_pSound->m_Sound.m_samples.size();
	m_XAudioBuffer.LoopCount = ( info.bIsLooped ? XAUDIO2_LOOP_INFINITE : 0u );

	m_pVoice->SubmitSourceBuffer( &m_XAudioBuffer );
	m_bIsConsuming = true;

	//if( info.bStartPlaying )
	//{
		m_pVoice->Start();
		m_bPlaying = true;
	//}
}

SoundSourceXAudio2::~SoundSourceXAudio2()
{
	if( m_pVoice )
	m_pVoice->DestroyVoice();
}

void SoundSourceXAudio2::SetPlaybackSpeed( float speed )
{
	HRESULT hr;
	hr = m_pVoice->SetFrequencyRatio( speed );
#if FISSION_DEBUG
	if( FAILED( hr ) ) throw std::runtime_error( "Failed to set Frequency Ratio" );
#endif
}

float SoundSourceXAudio2::GetPlaybackSpeed()
{
	float ratio;
	m_pVoice->GetFrequencyRatio( &ratio );
	return ratio;
}

void SoundSourceXAudio2::SetPosition( uint32_t position )
{
	if( m_bPlaying )
	{
		m_pVoice->Stop();
		m_pVoice->FlushSourceBuffers();
		XAUDIO2_VOICE_STATE vs;
		m_pVoice->GetState( &vs );
		m_XAudioBuffer.PlayBegin = ( (uint64_t)position * (uint64_t)m_pSound->m_Sound.m_format.nSamplesPerSec ) / 1000u;
		nSampleOffset = vs.SamplesPlayed - m_XAudioBuffer.PlayBegin;
		m_pVoice->SubmitSourceBuffer( &m_XAudioBuffer );
		m_pVoice->Start();
	}
	else
	{
		XAUDIO2_VOICE_STATE vs;
		m_pVoice->GetState( &vs );
		m_XAudioBuffer.PlayBegin = ( (uint64_t)position * (uint64_t)m_pSound->m_Sound.m_format.nSamplesPerSec ) / 1000u;
		nSampleOffset = vs.SamplesPlayed - m_XAudioBuffer.PlayBegin;
		m_pVoice->FlushSourceBuffers();
	}
}

uint32_t SoundSourceXAudio2::GetPosition()
{
	XAUDIO2_VOICE_STATE vs;
	m_pVoice->GetState( &vs );
	if( m_pSound->m_Sound.m_format.nSamplesPerSec == 0 ) return 0;
	return (uint32_t)( ( 1000u * ( vs.SamplesPlayed - nSampleOffset ) ) / m_pSound->m_Sound.m_format.nSamplesPerSec );
}


#define DEBUG_PRINT( fmt, ... ) Console::Message<512>(fmt,__VA_ARGS__)
#define DEBUG_WPRINT( fmt, ... ) Console::Message<512>(fmt,__VA_ARGS__)

#define BREAK_ON_FAIL( hrcall ) if( FAILED( hr = hrcall ) ) break;
#define RETURN_ON_FAIL( hrcall ) if( FAILED( hr = hrcall ) ) return {};

LoadSoundResult LoadSoundDataFromFile( const std::filesystem::path & filepath, SoundData * sd )
{
	struct MF
	{
		MF() { MFStartup( MF_VERSION ); }
		~MF() { MFShutdown(); }
	};

	static MF __mf;
	std::vector<byte> bytes;
	HRESULT hr = S_OK;

	Microsoft::WRL::ComPtr<IMFSourceReader> pReader;

	Console::WriteLine(
		"Reading Sound File: %s"_format(
			filepath.string().c_str()
		) / colors::LightSeaGreen
	);

	hr = MFCreateSourceReaderFromURL( filepath.c_str(), NULL, &pReader );

	if( FAILED( hr ) )
	{
	//	Console::Warning( "Could not load [%s] as a sound!", filepath.string().c_str() );
		return LoadSoundResult::Failure;
	}
	else {}//DEBUG_WPRINT( "\nBegin read [%s]\n", filepath.string().c_str() );

	Microsoft::WRL::ComPtr<IMFMediaType> pAudioType; // Represents the PCM audio format.
	{
		Microsoft::WRL::ComPtr<IMFMediaType> pPartialType;

		// Select the first audio stream, and deselect all other streams.
		RETURN_ON_FAIL( pReader->SetStreamSelection( (DWORD)MF_SOURCE_READER_ALL_STREAMS, FALSE ) );

		RETURN_ON_FAIL( pReader->SetStreamSelection( (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE ) );
		// Create a partial media type that specifies uncompressed PCM audio.
		RETURN_ON_FAIL( MFCreateMediaType( &pPartialType ) );

		RETURN_ON_FAIL( pPartialType->SetGUID( MF_MT_MAJOR_TYPE, MFMediaType_Audio ) );
		RETURN_ON_FAIL( pPartialType->SetGUID( MF_MT_SUBTYPE, MFAudioFormat_PCM ) );

		// Set this type on the source reader. The source reader will
		// load the necessary decoder.
		RETURN_ON_FAIL( pReader->SetCurrentMediaType( (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, pPartialType.Get() ) );

		// Get the complete uncompressed format.
		RETURN_ON_FAIL( pReader->GetCurrentMediaType( (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pAudioType ) );

		// Ensure the stream is selected.
		RETURN_ON_FAIL( pReader->SetStreamSelection( (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE ) );
	}

	UINT32 cbFormat = 0;
	WAVEFORMATEX * pWav = NULL;
	// Convert the PCM audio format into a WAVEFORMATEX structure.
	RETURN_ON_FAIL( MFCreateWaveFormatExFromMFMediaType( pAudioType.Get(), &pWav, &cbFormat ) );

	auto FormatTagString = [] ( WORD tag ) {
		switch( tag )
		{
		case WAVE_FORMAT_PCM:        return "PCM";
		case WAVE_FORMAT_EXTENSIBLE: return "PCM-Ext";
		default: return "Unknown";
		}
	};

	Console::WriteLine(
		"Uncompressed Format: %s %i channel %i-bit @ %iHz"_format(
			FormatTagString( pWav->wFormatTag ),
			pWav->nChannels,
			pWav->wBitsPerSample,
			pWav->nSamplesPerSec
		)/colors::LightSeaGreen
	);

	DWORD cbBuffer = 0;
	BYTE * pAudioData = NULL;
	Microsoft::WRL::ComPtr<IMFSample>		pSample;
	Microsoft::WRL::ComPtr<IMFMediaBuffer>	pBuffer;

	// Get audio samples from the source reader.
	while( true )
	{
		DWORD dwFlags = 0;

		// Read the next sample.
		BREAK_ON_FAIL( pReader->ReadSample(
			(DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
			0, NULL, &dwFlags, NULL, &pSample ) );

		if( dwFlags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED )
		{
		//	DEBUG_WPRINT( "\nType change - not supported by WAVE file format.\n" );
			break;
		}
		if( dwFlags & MF_SOURCE_READERF_ENDOFSTREAM )
		{
		//	DEBUG_WPRINT( "\nEnd of input file.\n" );
			break;
		}
		if( pSample == NULL )
		{
		//	DEBUG_WPRINT( "\nNo sample\n" );
			continue;
		}
		// Get a pointer to the audio data in the sample.
		BREAK_ON_FAIL( pSample->ConvertToContiguousBuffer( &pBuffer ) );
		BREAK_ON_FAIL( pBuffer->Lock( &pAudioData, NULL, &cbBuffer ) );

		// Write this data to the output.
		bytes.insert( bytes.end(), pAudioData, pAudioData + cbBuffer );

		// Unlock the buffer.
		pAudioData = NULL;
		BREAK_ON_FAIL( pBuffer->Unlock() );
	}
	if( SUCCEEDED( hr ) ) {}// DEBUG_WPRINT( "Got %.2f MB bytes of audio data.\n\n", (float)bytes.size() / 1e6f );

	if( pAudioData ) pBuffer->Unlock();

	bytes.shrink_to_fit();

	sd->m_format = std::move( *pWav );
	sd->m_samples = std::move( bytes );

	return LoadSoundResult::Success;
}

}
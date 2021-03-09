#include "SoundXAudio2.h"
#include "Fission/Core/Console.h"

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

	LoadSoundResult LoadSoundDataFromFile( const file::path & filepath, SoundData * sd );


SoundEngineXAudio2::SoundEngineXAudio2( const CreateInfo & info )
{
	HRESULT hr = XAudio2Create( &m_pXAudio2Engine, 0, XAUDIO2_ANY_PROCESSOR );

	if( FAILED( hr ) ) 
		throw exception( "XAudio2 Exception", _lazer_exception_msg.append( "Failed to Initialize Sound Engine." ) );

	hr = m_pXAudio2Engine->CreateMasteringVoice( &m_pMaster );

	if( FAILED( hr ) )
		throw exception( "XAudio2 Exception", _lazer_exception_msg.append( "Failed to Create Mastering Voice." ) );

	m_vpSubmixVoices.resize( info.nOutputs );
	for( auto && submix : m_vpSubmixVoices )
	{
		hr = m_pXAudio2Engine->CreateSubmixVoice( &submix, 2, 44100 );
	if( FAILED( hr ) )
		throw exception( "XAudio2 Exception", _lazer_exception_msg.append( "Failed to Create Submix Voice." ) );
	}
	m_pXAudio2Engine->StartEngine();
}

ref<Fission::ISound> SoundEngineXAudio2::CreateSound( const file::path & filepath )
{
	auto sound = CreateRef<SoundXAudio2>();
	LoadSoundDataFromFile( filepath, &sound->m_Sound );
	return sound;
}

ref<ISoundSource> SoundEngineXAudio2::Play( ISound * _Sound, Sound::OutputID _Output, bool _Play_Looped, bool _Start_Playing, bool _Track )
{
	auto sound = static_cast<SoundXAudio2 *>( _Sound );

	return CreateRef<SoundSourceXAudio2>( sound, m_pXAudio2Engine.Get(), m_vpSubmixVoices[_Output] );
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
	m_pXAudio2Engine->StopEngine();
	m_pMaster->DestroyVoice();
}

bool SoundXAudio2::empty()
{
	return m_Sound.m_samples.empty();
}

uint32_t SoundXAudio2::length()
{
	uint64_t nBitsPSample = m_Sound.m_format.wBitsPerSample;
	uint64_t nBytes = m_Sound.m_samples.size();
	uint64_t nTotalSamplesPSec = (uint64_t)m_Sound.m_format.nSamplesPerSec * (uint64_t)m_Sound.m_format.nChannels;
	return uint32_t( 8000u * ( nBytes / nBitsPSample ) / nTotalSamplesPSec );
}

SoundSourceXAudio2::SoundSourceXAudio2( SoundXAudio2 * sound, IXAudio2 * engine, IXAudio2SubmixVoice * pOutput )
{
	XAUDIO2_SEND_DESCRIPTOR Send = { 0, pOutput };
	XAUDIO2_VOICE_SENDS SendList = { 1, &Send };

	engine->CreateSourceVoice( &m_pVoice, &sound->m_Sound.m_format,
		0, XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, &SendList );

	XAUDIO2_BUFFER xaBuffer = {};
	xaBuffer.pAudioData = sound->m_Sound.m_samples.data();
	xaBuffer.AudioBytes = (uint32_t)sound->m_Sound.m_samples.size();
	xaBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;

	m_pVoice->SubmitSourceBuffer( &xaBuffer );

	m_pVoice->Start();
}

void SoundSourceXAudio2::SetPlaying( bool playing )
{
	if( m_bPlaying == playing ) return;
	m_bPlaying = playing;
	if( m_bPlaying )
	{
		m_pVoice->Start();
	}
	else
	{
		m_pVoice->Stop();
	}
}

bool SoundSourceXAudio2::GetPlaying()
{
	return m_bPlaying;
}

SoundSourceXAudio2::~SoundSourceXAudio2()
{
	m_pVoice->DestroyVoice();
}


#define DEBUG_PRINT( fmt, ... ) printf(fmt,__VA_ARGS__)
#define DEBUG_WPRINT( fmt, ... ) wprintf(fmt,__VA_ARGS__)

#define BREAK_ON_FAIL( hrcall ) if( FAILED( hr = hrcall ) ) break;
#define RETURN_ON_FAIL( hrcall ) if( FAILED( hr = hrcall ) ) return {};

LoadSoundResult LoadSoundDataFromFile( const file::path & filepath, SoundData * sd )
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

	hr = MFCreateSourceReaderFromURL( filepath.c_str(), NULL, &pReader );

	if( FAILED( hr ) )
	{
		DEBUG_WPRINT( L"\nCould not open [%s]\n", filepath.c_str() );
		return LoadSoundResult::Failure;
	}
	else DEBUG_WPRINT( L"\nBegin read [%s]\n", filepath.c_str() );

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
#define CASE_RETURN( macro ) case macro: return #macro
		switch( tag )
		{
			CASE_RETURN( WAVE_FORMAT_PCM );
			CASE_RETURN( WAVE_FORMAT_EXTENSIBLE );
		default: return "Unknown";
		}
#undef CASE_RETURN
	};

	DEBUG_PRINT( "Uncompressed Format:\n"
		"\ttag = [%s]\n"
		"\tchannels = [%i]\n"
		"\tSample Rate Frequency = [%i]\n"
		"\tAverage Bytes/sec = [%i]\n"
		"\tBlock Align = [%i]\n"
		"\tBits per Sample = [%i]\n"
		,
		FormatTagString( pWav->wFormatTag ),
		pWav->nChannels,
		pWav->nSamplesPerSec,
		pWav->nAvgBytesPerSec,
		pWav->nBlockAlign,
		pWav->wBitsPerSample );

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
			DEBUG_WPRINT( L"\nType change - not supported by WAVE file format.\n" );
			break;
		}
		if( dwFlags & MF_SOURCE_READERF_ENDOFSTREAM )
		{
			DEBUG_WPRINT( L"\nEnd of input file.\n" );
			break;
		}
		if( pSample == NULL )
		{
			DEBUG_WPRINT( L"\nNo sample\n" );
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
	if( SUCCEEDED( hr ) ) DEBUG_WPRINT( L"Got %.2f MB bytes of audio data.\n\n", (float)bytes.size()/1e6f );

	if( pAudioData ) pBuffer->Unlock();

	bytes.shrink_to_fit();

	sd->m_format = std::move( *pWav );
	sd->m_samples = std::move( bytes );

	return LoadSoundResult::Success;
}

}
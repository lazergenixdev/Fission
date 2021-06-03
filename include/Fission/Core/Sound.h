/**
*
* @file: Sound.h
* @author: lazergenixdev@gmail.com
*
*
* This file is provided under the MIT License:
*
* Copyright (c) 2021 Lazergenix Software
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/

#pragma once
#include "Fission/config.h"

// todo: more options in API (getting raw samples, or possibly 'baked' samples)

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
		//virtual void SetVolume( float volume ) = 0;
		//virtual float GetVolume() = 0;

		virtual void SetPlaybackSpeed( float speed ) = 0;
		virtual float GetPlaybackSpeed() = 0;

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

		virtual ref<ISound> CreateSound( const std::filesystem::path & filepath ) = 0;

		virtual ref<ISoundSource> Play( ISound * _Sound, Sound::OutputID _Output, bool _Play_Looped = false, bool _Start_Playing = true, bool _Track = false ) = 0;

		virtual void SetVolume( Sound::OutputID _Output, float _Volume ) = 0;
		virtual float GetVolume( Sound::OutputID _Output ) = 0;

		virtual void SetMasterVolume( float _Volume ) = 0;
		virtual float GetMasterVolume() = 0;

		virtual ~SoundEngine() = default;

	};

} // namespace Fission

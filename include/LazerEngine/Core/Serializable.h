#pragma once
#include "LazerEngine/config.h"

namespace lazer {

	interface ISerializable
	{
		virtual bool Load( const file::path & _FilePath ) = 0;
		virtual bool Save( const file::path & _FilePath ) = 0;

		virtual ~ISerializable() = default;

	}; // interface lazer::ISerializable

} // namespace lazer

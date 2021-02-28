#pragma once
#include "Fission/config.h"

namespace Fission {

	interface ISerializable
	{
		virtual bool Load( const file::path & _FilePath ) = 0;
		virtual bool Save( const file::path & _FilePath ) = 0;

		virtual ~ISerializable() = default;

	}; // interface Fission::ISerializable

} // namespace Fission

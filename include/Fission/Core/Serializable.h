#pragma once
#include "Fission/config.h"

namespace Fission {

	struct ISerializable
	{
		virtual bool Load( const file::path & _FilePath ) = 0;
		virtual bool Save( const file::path & _FilePath ) const = 0;

		virtual ~ISerializable() = default;

	}; // struct Fission::ISerializable

} // namespace Fission

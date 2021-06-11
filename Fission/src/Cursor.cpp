#include <Fission/Core/Input/Cursor.hh>

#ifdef FISSION_PLATFORM_WINDOWS
#include "Platform/Windows/WindowsCursor.h"
#endif

namespace Fission {

    std::unique_ptr<Cursor> Cursor::Create( const CreateInfo & info ) {
        return nullptr;
    }

    Cursor * Cursor::Get( Default_ default_cursor ) 
    {
#ifdef FISSION_PLATFORM_WINDOWS
        return Platform::WindowsCursor::Default( default_cursor );
#endif // FISSION_PLATFORM_WINDOWS
    }

} // namespace Fission

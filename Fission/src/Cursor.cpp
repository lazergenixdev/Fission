#include "LazerEngine/Core/Input/Cursor.h"

#ifdef LAZER_PLATFORM_WINDOWS
#include "Platform/Windows/WindowsCursor.h"
#endif

namespace lazer {

    std::unique_ptr<Cursor> Cursor::Create( const CreateInfo & info ) {
        return nullptr;
    }

    Cursor * Cursor::Get( Default_ default_cursor ) 
    {
#ifdef LAZER_PLATFORM_WINDOWS
        return Platform::WindowsCursor::Default( default_cursor );
#endif // LAZER_PLATFORM_WINDOWS
    }

} // namespace lazer

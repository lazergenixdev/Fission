#pragma once
#include "LazerEngine/config.h"

namespace lazer {

    class Cursor 
    {
    public:

        enum Default_ {
            Default_Hidden,
            Default_Arrow,
            Default_TextInput,
            Default_Hand,
            Default_Move,
            Default_Cross,
            Default_Wait,
            Default_SizeY,
            Default_SizeX,
            Default_SizeBLTR,
            Default_SizeTLBR,
        };

        struct CreateInfo {
            int not_implemented_lolxd;
        };

    public:

        // create a custom cursor
        LAZER_API static std::unique_ptr<Cursor> Create( const CreateInfo & info );

        LAZER_API static Cursor * Get( Default_ default_cursor );

        virtual bool Use() = 0;

        virtual ~Cursor() = default;

    }; // class lazer::Cursor

} // namespace lazer

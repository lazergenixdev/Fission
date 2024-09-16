#include <Fission/core/font.hpp>

using namespace fs;

Glyph const * Font_Static::lookup (c32 codepoint) {
    (void)codepoint;
    return nullptr;
}

Font_Static::~Font_Static() {

}

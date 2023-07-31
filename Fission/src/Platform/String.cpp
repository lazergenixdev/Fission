#include <Fission/Base/String.hpp>

#include <stdexcept>

__FISSION_BEGIN__

void convert_utf8_to_utf16(string_utf16* out_buffer, string input) {
	throw std::runtime_error("are you for real?");
}

void convert_utf16_to_utf8(string* out_buffer, string_utf16 input) {
	c8* dst = (char8_t*)out_buffer->data;

	const c16* src = reinterpret_cast<const c16*>(input.data);
	const c16* const end = src + input.count;

	u32 _ax, _bx, _cx, _dx;

	// ghetto switch statement.
	while (src != end)
	{
		if (*src > 0xD7FF)
		{
			if (end - src < 2) { ++src; continue; }

			// 4 bytes

			// Store the codepoint in `_dx`
			_dx = ((*src++ - 0xD800) * 0x400);
			_dx = ((*src++ - 0xDC00) + _dx + 0x10000);

			// Break the codepoint down into parts to be put into utf8
			_ax = (_dx & 0b11100'0000'0000'0000'0000) >> 18;
			_bx = (_dx & 0b00011'1111'0000'0000'0000) >> 12;
			_cx = (_dx & 0b00000'0000'1111'1100'0000) >> 6;
			_dx &= 0b111'111;

			// Set all the "non-codepoint" bits
			*dst++ = c8(0b1111'0000 | _ax);
			*dst++ = c8(0b1000'0000 | _bx);
			*dst++ = c8(0b1000'0000 | _cx);
			*dst++ = c8(0b1000'0000 | _dx);

			continue;
		}
		if (*src > 0b0000'0111'1111'1111)
		{
			// 3 bytes
			_ax = (*src & 0b1111'0000'0000'0000) >> 12;
			_bx = (*src & 0b0000'1111'1100'0000) >> 6;
			_cx = *src++ & 0b0000'0000'0011'1111;

			*dst++ = c8(0b1110'0000 | _ax);
			*dst++ = c8(0b1000'0000 | _bx);
			*dst++ = c8(0b1000'0000 | _cx);

			continue;
		}
		if (*src > 0b0000'0000'0111'1111)
		{
			// 2 bytes
			_ax = (*src & 0b0000'0111'1100'0000) >> 6;
			_bx = *src++ & 0b0000'0000'0011'1111;

			*dst++ = c8(0b1100'0000 | _ax);
			*dst++ = c8(0b1000'0000 | _bx);

			continue;
		}
		/* Default: */
		*dst++ = c8(*src++);
	}

	out_buffer->count = (c8*)dst - out_buffer->data;
}

__FISSION_END__

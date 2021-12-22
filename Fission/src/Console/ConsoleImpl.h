#pragma once
#include <Fission/Core/Console.hh>
#include <Fission/Base/Utility/BitFlag.h>

namespace Fission {

class ConsoleImpl
{
	friend console_iterator;
	friend console_iterator::console_string_view;

private:
	struct StringSegment
	{
		uint32_t StartOffset; //!< Where this segment begins in the character buffer.
		uint32_t NewLines;    //!< Number of newlines after this segment of text.
		color Color;          //!< Color of this segment.
		int PrevSegment = -1; //!< Index to the first segment on the previous line.

		StringSegment( color _Color ) : StartOffset( 0u ), NewLines( 0u ), Color( _Color ) {}
		StringSegment( uint32_t _Start, uint32_t _NewLines, color _Color ) : StartOffset( _Start ), NewLines( _NewLines ), Color( _Color ) {}
	};

	struct LineInfo
	{
		uint32_t startSegment; //!< First segment on this line.
		/*
		0 this is a sentence.\n  (startNewLine=0)
		1 \n                     (startNewLine=1)
		2 hello world\n          (startNewLine=0)
		*/
		uint32_t startNewLine; //!< How many newlines into a segment that has multiple newlines after it.
	};

public:
	ConsoleImpl();
	~ConsoleImpl();

	void SetEnabled( bool _Enable );

	bool IsEnabled();

	void Clear();

	void RegisterCommand( const string& _Command_Name, CommandCallback _Callback );

	void UnregisterCommand( const string & _Command_Name );

	void ExecuteCommand( const string & _Command );

	void WriteLine( const colored_stream& _Text );

	void Write( const colored_stream& _Text );

	int GetLineCount();

	LineInfo _GetSegmentFromLine( uint32_t _Line );

public:
	static ConsoleImpl & Get();

private:
	void add_newline();
	//! @warning `numNewLines` is assumed to be either 0 or 1.
	void add_line( const char* _Buffer, uint32_t _Length, uint32_t numNewLines, const color& _Color );

	void reserve( uint32_t _New_Count );

private:
	static constexpr uint32_t m_BufferCapacity_HardMax = 1000000u;

private:
	std::unordered_map<std::string, CommandCallback> m_CommandMap;

	char * m_CharacterBuffer = nullptr;
	uint32_t m_BufferCount = 0u;
	uint32_t m_BufferCapacity = 2000u;

	std::mutex m_WriteMutex;

	std::vector<StringSegment> m_Segments;
	std::vector<LineInfo> m_Lines;

	bool m_bEnabled = true;
};

}

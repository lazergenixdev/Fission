#pragma once
#include "Fission/Core/Console.h"

namespace Fission {

class ConsoleImpl
{
public:
	ConsoleImpl();
	~ConsoleImpl();

	void SetEnabled( bool _Enable );

	bool IsEnabled();

	void Clear();

	void RegisterCommand( const std::wstring & _Command_Name, CommandCallback _Callback );

	void UnregisterCommand( const std::wstring & _Command_Name );

	void ExecuteCommand( const std::wstring & _Command );

	void WriteLine( color _Color, const wchar_t * _Text );

	void Write( color _Color, const wchar_t * _Text );

	bool GetLine( int _Line_Number, std::wstring * _Output_Text, color * _Output_Color );

	int GetLineCount();

public:
	static ConsoleImpl & Get();

private:
	void reserve( uint32_t _New_Count );

private:
	static constexpr uint32_t m_BufferCapacity_HardMax = 1000000u;

private:
	std::unordered_map<std::wstring, CommandCallback> m_CommandMap;

	wchar_t * m_CharacterBuffer = nullptr;
	uint32_t m_BufferCount = 0u;
	uint32_t m_BufferCapacity = 2000u;

	std::mutex m_WriteMutex;

	struct TextLine {
		uint32_t start;
		color color;
	};

	std::vector<TextLine> m_Lines;

	bool m_bEnabled = true;
};

}

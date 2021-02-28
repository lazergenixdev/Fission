#pragma once
#include "LazerEngine/config.h"

namespace lazer {

	/// <summary>
	/// System acts as an interface for the platform operating system
	/// </summary>
	class System
	{
	public:
		/// <summary>
		/// Initialize Summary.
		/// </summary>
		LAZER_API static void Initialize();

		/// <summary>
		/// Shutdown Summary.
		/// </summary>
		LAZER_API static void Shutdown();

		/// <summary>
		/// Blocks current thread with a OK message box.
		/// </summary>
		/// <param name="Title">- What gets set as the title.</param>
		/// <param name="Text">.</param>
		LAZER_API static void DisplayMessageBox( const std::wstring & _Title, const std::wstring & _Text );

		/// <summary>
		/// Save text to the Clipboard.
		/// </summary>
		/// <param name="Text">- utf8 string to put onto the clipboard.</param>
		LAZER_API static void SaveToClipboard( const std::string & _Text );

		/// <summary>
		/// Writes out information about the system to the console.
		/// </summary>
		LAZER_API static void WriteInfoToConsole();

	}; // class lazer::System

} // namespace lazer

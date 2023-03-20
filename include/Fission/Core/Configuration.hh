/**
*
* @file: Configuration.h
* @author: lazergenixdev@gmail.com
*
*
* This file is provided under the MIT License:
*
* Copyright (c) 2021 Lazergenix Software
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/

/**
 *
 * All application configuration settings are stored in a YAML file.
 * 
 * Config files include how windows will act when first created, and the
 * settings for the graphics (Multi-Sampling, V-Sync, Texture Filtering, ...)
 * 
 * Applications will automatically call `Load()` on startup
 * to get a user's saved settings and `Save()` when the app exits.
 * Default Path: "<user save directory>/etc/appconfig.yml"
 * 
 * It is the programmer's responsibility to call these functions accordingly
 * when the user decides to import new settings, or in the case of a program crash,
 * save the settings after changed so that the user's settings are kept intact.
 * But this is by no means a requirement.
 * 
 */

//! TODO: merge this interface into FEngine (maybe not this exact interface)

//#pragma once
//#include <Fission/Core/Window.hh>
//#include <filesystem>
//
//
//namespace Fission {
//
//    class Config
//    {
//    public:
//        static constexpr const char * DefaultSaveFile = "appconfig";
//
//        //! @brief Load configuration data from a file to the application context
//        //! @note It is not sufficient to just load in new settings for them to be applied,
//        //!         for graphics settings the application will need to call `Recreate()` for
//        //!         them to take effect.
//        //!       Also, window configuration data only apply for newly created windows after `Load()` was called.
//        //! @return 
//        FISSION_API static bool Load( std::filesystem::path _Save_Location = DefaultSaveFile ) noexcept;
//
//        //! @brief Saves all settings in the application context.
//        FISSION_API static bool Save( std::filesystem::path _Save_Location = DefaultSaveFile ) noexcept;
//
//    };
//
//}

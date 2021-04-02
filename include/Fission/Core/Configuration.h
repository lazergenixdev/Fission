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

// todo: more development needed, methods used here I don't see as being scalable

#pragma once
#include <Fission/config.h>
#include <Fission/Core/Window.h>

namespace Fission {

    class Configuration
    {
    public:
        struct WindowConfig
        {
            std::optional<std::string>      Style;
            std::optional<vec2i>            Position;
            std::optional<vec2i>            Size;
            std::optional<bool>             SavePosition;
            std::optional<bool>             SaveSize;
            std::optional<bool>             Fullscreen;
        };

        struct GraphicsConfig
        {
            std::optional<vec2i>            Resolution;
            std::optional<std::string>      API;
            std::optional<std::string>      FrameRate;
            std::optional<std::string>      FSAA;
        };

        FISSION_API static void Load() noexcept;

        FISSION_API static void Save() noexcept;


        FISSION_API static void SetWindowConfig( const Window::Properties & ) noexcept;

        FISSION_API static Window::Properties GetWindowConfig( const Window::Properties & fallback ) noexcept;

        FISSION_API static void SetGraphicsConfig( const GraphicsConfig & ) noexcept;

        FISSION_API static GraphicsConfig GetGraphicsConfig() noexcept;


    };

}

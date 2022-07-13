/**
*
* @file: VertexLayout.h
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

#pragma once
#include <Fission/Base/Buffer.hpp>

namespace Fission::Resource {

	namespace VertexLayoutTypes
	{
		enum Type : int 
		{
			Float,
			Float2,
			Float3,
			Float4,

			Int,
			Int2,
			Int3,
			Int4,
		};

		static constexpr uint32_t GetStride( Type type )
		{
			switch( type )
			{
			case Float:
			case Int:
				return 4u;

			case Float2:
			case Int2:
				return 8u;

			case Float3:
			case Int3:
				return 12u;

			case Float4:
			case Int4:
				return 16u;

			default:return 0;
			}
		}
	}

	class VertexLayout
	{
		using vertex_type = VertexLayoutTypes::Type;
	public:
		VertexLayout() : m_Stride( 0u ) {
			// 64 characters should be enough for most semantic collections
			m_SemanticBuffer.reserve( 64u );
		}

		/**** Push data onto vertex layout ****/

		VertexLayout & Append(const vertex_type & type, const char * semantic) {
			m_SemanticBuffer.emplace_back( *semantic );
			while( *semantic != '\0' )
			{
				m_SemanticBuffer.emplace_back( *( ++semantic ) );
			}
			m_TypeBuffer.emplace_back( type );
			m_Stride += VertexLayoutTypes::GetStride( type );
			return *this;
		}

		/**** Recieve data from vertex layout ****/

		uint32_t GetCount() { return (uint32_t)m_TypeBuffer.size(); }

		uint32_t GetStride() { return m_Stride; }
		const char * GetName( int i ) {
			FISSION_ASSERT( i < (int)GetCount() );
			size_t pos = 0;

			// search for position of name
			while( i != 0 )
				if( m_SemanticBuffer[pos++] == '\0' ) i--;

			return &m_SemanticBuffer[pos];
		}

		vertex_type GetType( int i ) { 
			return m_TypeBuffer[i]; 
		}

	private:
		dynamic_buffer<char> m_SemanticBuffer;
		dynamic_buffer<vertex_type> m_TypeBuffer;
		uint32_t m_Stride;

	}; // class Fission::Resource::VertexLayout

} // namespace Fission::Resource

#pragma once
#include <vector>
#include <cassert>

namespace lazer::Resource {

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

		template <vertex_type _Ty>
		VertexLayout & Append( const char * semantic ) {
			m_SemanticBuffer.emplace_back( *semantic );
			while( *semantic != '\0' )
			{
				m_SemanticBuffer.emplace_back( *( ++semantic ) );
			}
			m_TypeBuffer.emplace_back( _Ty );
			m_Stride += VertexLayoutTypes::GetStride( _Ty );
			return *this;
		}

		/**** Recieve data from vertex layout ****/

		uint32_t GetCount() { return (uint32_t)m_TypeBuffer.size(); }

		uint32_t GetStride() { return m_Stride; }
		const char * GetName( int i ) {
			assert( i < (int)GetCount() );
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
		std::vector<char> m_SemanticBuffer;
		std::vector<vertex_type> m_TypeBuffer;
		uint32_t m_Stride;
	};

}

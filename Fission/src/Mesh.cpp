#include "Mesh.h"
#include "lazer/unfinished.h"

namespace Fission {


	Mesh::Mesh( int vertex_count, int index_count, int color_count ) 
		: m_Data( new MeshData )
	{
		m_Data->vertex_buffer.reserve( vertex_count );
		m_Data->index_buffer.reserve( index_count );
		m_Data->color_buffer.reserve( color_count );
	}

	Mesh::Mesh( const Mesh & src )
		: m_Data( new MeshData( *src.m_Data ) )
	{}

	Mesh::~Mesh()
	{
		delete m_Data;
	}

	void Mesh::push_color( color col ) {
		m_Data->color_buffer.emplace_back( col );
	}

	void Mesh::push_vertex( vec2f position, int color_index ) {
		m_Data->vertex_buffer.emplace_back( position, color_index );
	}

	void Mesh::push_index( uint32_t index ) {
		m_Data->index_buffer.emplace_back( index );
	}

	void Mesh::set_color( uint32_t index, color new_color ) {
		m_Data->color_buffer[index] = new_color;
	}

	uint32_t Mesh::vertex_count() const {
		return (uint32_t)m_Data->vertex_buffer.size();
	}

	uint32_t Mesh::index_count() const {
		return (uint32_t)m_Data->vertex_buffer.size();
	}

	uint32_t Mesh::color_count() const {
		return (uint32_t)m_Data->color_buffer.size();
	}

}

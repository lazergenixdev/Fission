#pragma once

template <typename T>
struct DefaultDelete : public T
{
	template <typename...P>
	DefaultDelete( P&&...params ) : T( std::forward<P>( params )... )
	{}

	virtual void Destroy() override { delete this; }
};

﻿#include "Fission/Core/SurfaceMap.h"

/** @file MaxRectsBinPack.h
	@author Jukka Jyl�nki

	@brief Implements different bin packer algorithms that use the MAXRECTS data structure.

	This work is released to Public Domain, do whatever you want with it.
*/
#include <vector>
#include <algorithm>
#include <utility>
#include <iostream>
#include <limits>
#include <cassert>
#include <cstring>
#include <cmath>
#include <cstdlib>


namespace rbp {

	struct RectSize
	{
		int width;
		int height;
		void * userdata;
	};

	struct Rect
	{
		int x;
		int y;
		int width;
		int height;
		void * userdata;
	};

	/// Returns true if a is contained in b.
	bool IsContainedIn( const Rect & a, const Rect & b )
	{
		return a.x >= b.x && a.y >= b.y
			&& a.x + a.width <= b.x + b.width
			&& a.y + a.height <= b.y + b.height;
	}

	class DisjointRectCollection
	{
	public:
		std::vector<Rect> rects;

		bool Add( const Rect & r )
		{
			// Degenerate rectangles are ignored.
			if( r.width == 0 || r.height == 0 )
				return true;

			if( !Disjoint( r ) )
				return false;
			rects.push_back( r );
			return true;
		}

		void Clear()
		{
			rects.clear();
		}

		bool Disjoint( const Rect & r ) const
		{
			// Degenerate rectangles are ignored.
			if( r.width == 0 || r.height == 0 )
				return true;

			for( size_t i = 0; i < rects.size(); ++i )
				if( !Disjoint( rects[i], r ) )
					return false;
			return true;
		}

		static bool Disjoint( const Rect & a, const Rect & b )
		{
			if( a.x + a.width <= b.x ||
				b.x + b.width <= a.x ||
				a.y + a.height <= b.y ||
				b.y + b.height <= a.y )
				return true;
			return false;
		}
	};

/** MaxRectsBinPack implements the MAXRECTS data structure and different bin packing algorithms that
	use this structure. */
	class MaxRectsBinPack
	{
	public:
		/// Instantiates a bin of size (0,0). Call Init to create a new bin.
		MaxRectsBinPack():binWidth( 0 ),binHeight( 0 ){}

		/// Instantiates a bin of the given size.
		/// @param allowFlip Specifies whether the packing algorithm is allowed to rotate the input rectangles by 90 degrees to consider a better placement.
		MaxRectsBinPack( int width, int height, bool allowFlip = true ){Init( width, height, allowFlip );}

		/// (Re)initializes the packer to an empty bin of width x height units. Call whenever
		/// you need to restart with a new bin.
		void Init( int width, int height, bool allowFlip = true )
		{
			binAllowFlip = allowFlip;
			binWidth = width;
			binHeight = height;

			Rect n;
			n.x = 0;
			n.y = 0;
			n.width = width;
			n.height = height;

			usedRectangles.clear();

			freeRectangles.clear();
			freeRectangles.push_back( n );
		}

		/// Specifies the different heuristic rules that can be used when deciding where to place a new rectangle.
		enum FreeRectChoiceHeuristic
		{
			RectBestShortSideFit, ///< -BSSF: Positions the rectangle against the short side of a free rectangle into which it fits the best.
			RectBestLongSideFit, ///< -BLSF: Positions the rectangle against the long side of a free rectangle into which it fits the best.
			RectBestAreaFit, ///< -BAF: Positions the rectangle into the smallest free rect into which it fits.
			RectBottomLeftRule, ///< -BL: Does the Tetris placement.
			RectContactPointRule ///< -CP: Choosest the placement where the rectangle touches other rects as much as possible.
		};

		/// Inserts the given list of rectangles in an offline/batch mode, possibly rotated.
		/// @param rects The list of rectangles to insert. This vector will be destroyed in the process.
		/// @param dst [out] This list will contain the packed rectangles. The indices will not correspond to that of rects.
		/// @param method The rectangle placement rule to use when packing.
		void Insert( std::vector<RectSize> & rects, std::vector<Rect> & dst, FreeRectChoiceHeuristic method )
		{
			dst.clear();

			while( rects.size() > 0 )
			{
				int bestScore1 = std::numeric_limits<int>::max();
				int bestScore2 = std::numeric_limits<int>::max();
				int bestRectIndex = -1;
				Rect bestNode;

				for( size_t i = 0; i < rects.size(); ++i )
				{
					int score1;
					int score2;
					RectSize & rcSize = rects[i];
					Rect newNode = ScoreRect( rcSize.width, rcSize.height, method, score1, score2 );

					if( score1 < bestScore1 || ( score1 == bestScore1 && score2 < bestScore2 ) )
					{
						bestScore1 = score1;
						bestScore2 = score2;
						bestNode = newNode;
						bestNode.userdata = rcSize.userdata;
						bestRectIndex = i;
					}
				}

				if( bestRectIndex == -1 )
					return;

				PlaceRect( bestNode );
				dst.push_back( bestNode );
				rects.erase( rects.begin() + bestRectIndex );
			}
		}

		/// Inserts a single rectangle into the bin, possibly rotated.
		Rect Insert( int width, int height, FreeRectChoiceHeuristic method )
		{
			Rect newNode;
			// Unused in this function. We don't need to know the score after finding the position.
			int score1 = std::numeric_limits<int>::max();
			int score2 = std::numeric_limits<int>::max();
			switch( method )
			{
			case RectBestShortSideFit: newNode = FindPositionForNewNodeBestShortSideFit( width, height, score1, score2 ); break;
			case RectBottomLeftRule: newNode = FindPositionForNewNodeBottomLeft( width, height, score1, score2 ); break;
			case RectContactPointRule: newNode = FindPositionForNewNodeContactPoint( width, height, score1 ); break;
			case RectBestLongSideFit: newNode = FindPositionForNewNodeBestLongSideFit( width, height, score2, score1 ); break;
			case RectBestAreaFit: newNode = FindPositionForNewNodeBestAreaFit( width, height, score1, score2 ); break;
			}

			if( newNode.height == 0 )
				return newNode;

			size_t numRectanglesToProcess = freeRectangles.size();
			for( size_t i = 0; i < numRectanglesToProcess; ++i )
			{
				if( SplitFreeNode( freeRectangles[i], newNode ) )
				{
					freeRectangles.erase( freeRectangles.begin() + i );
					--i;
					--numRectanglesToProcess;
				}
			}

			PruneFreeList();

			usedRectangles.push_back( newNode );
			return newNode;
		}

		/// Computes the ratio of used surface area to the total bin area.
		float Occupancy() const
		{
			unsigned long usedSurfaceArea = 0;
			for( size_t i = 0; i < usedRectangles.size(); ++i )
				usedSurfaceArea += usedRectangles[i].width * usedRectangles[i].height;

			return (float)usedSurfaceArea / ( binWidth * binHeight );
		}

	private:
		int binWidth;
		int binHeight;

		bool binAllowFlip;

		std::vector<Rect> usedRectangles;
		std::vector<Rect> freeRectangles;

		/// Computes the placement score for placing the given rectangle with the given method.
		/// @param score1 [out] The primary placement score will be outputted here.
		/// @param score2 [out] The secondary placement score will be outputted here. This is used to break ties.
		/// @return This struct identifies where the rectangle would be placed if it were placed.
		Rect ScoreRect( int width, int height, FreeRectChoiceHeuristic method, int & score1, int & score2 ) const
		{
			Rect newNode;
			score1 = std::numeric_limits<int>::max();
			score2 = std::numeric_limits<int>::max();
			switch( method )
			{
			case RectBestShortSideFit: newNode = FindPositionForNewNodeBestShortSideFit( width, height, score1, score2 ); break;
			case RectBottomLeftRule: newNode = FindPositionForNewNodeBottomLeft( width, height, score1, score2 ); break;
			case RectContactPointRule: newNode = FindPositionForNewNodeContactPoint( width, height, score1 );
				score1 = -score1; // Reverse since we are minimizing, but for contact point score bigger is better.
				break;
			case RectBestLongSideFit: newNode = FindPositionForNewNodeBestLongSideFit( width, height, score2, score1 ); break;
			case RectBestAreaFit: newNode = FindPositionForNewNodeBestAreaFit( width, height, score1, score2 ); break;
			}

			// Cannot fit the current rectangle.
			if( newNode.height == 0 )
			{
				score1 = std::numeric_limits<int>::max();
				score2 = std::numeric_limits<int>::max();
			}

			return newNode;
		}

		/// Returns 0 if the two intervals i1 and i2 are disjoint, or the length of their overlap otherwise.
		int CommonIntervalLength( int i1start, int i1end, int i2start, int i2end ) const
		{
			if( i1end < i2start || i2end < i1start )
				return 0;
			return std::min( i1end, i2end ) - std::max( i1start, i2start );
		}

		/// Places the given rectangle into the bin.
		void PlaceRect( const Rect & node )
		{
			size_t numRectanglesToProcess = freeRectangles.size();
			for( size_t i = 0; i < numRectanglesToProcess; ++i )
			{
				if( SplitFreeNode( freeRectangles[i], node ) )
				{
					freeRectangles.erase( freeRectangles.begin() + i );
					--i;
					--numRectanglesToProcess;
				}
			}

			PruneFreeList();

			usedRectangles.push_back( node );
		}

		/// Computes the placement score for the -CP variant.
		int ContactPointScoreNode( int x, int y, int width, int height ) const
		{
			int score = 0;

			if( x == 0 || x + width == binWidth )
				score += height;
			if( y == 0 || y + height == binHeight )
				score += width;

			for( size_t i = 0; i < usedRectangles.size(); ++i )
			{
				if( usedRectangles[i].x == x + width || usedRectangles[i].x + usedRectangles[i].width == x )
					score += CommonIntervalLength( usedRectangles[i].y, usedRectangles[i].y + usedRectangles[i].height, y, y + height );
				if( usedRectangles[i].y == y + height || usedRectangles[i].y + usedRectangles[i].height == y )
					score += CommonIntervalLength( usedRectangles[i].x, usedRectangles[i].x + usedRectangles[i].width, x, x + width );
			}
			return score;
		}

		Rect FindPositionForNewNodeBottomLeft( int width, int height, int & bestY, int & bestX ) const
		{
			Rect bestNode;
			memset( &bestNode, 0, sizeof( Rect ) );

			bestY = std::numeric_limits<int>::max();
			bestX = std::numeric_limits<int>::max();

			for( size_t i = 0; i < freeRectangles.size(); ++i )
			{
				// Try to place the rectangle in upright (non-flipped) orientation.
				if( freeRectangles[i].width >= width && freeRectangles[i].height >= height )
				{
					int topSideY = freeRectangles[i].y + height;
					if( topSideY < bestY || ( topSideY == bestY && freeRectangles[i].x < bestX ) )
					{
						bestNode.x = freeRectangles[i].x;
						bestNode.y = freeRectangles[i].y;
						bestNode.width = width;
						bestNode.height = height;
						bestY = topSideY;
						bestX = freeRectangles[i].x;
					}
				}
				if( binAllowFlip && freeRectangles[i].width >= height && freeRectangles[i].height >= width )
				{
					int topSideY = freeRectangles[i].y + width;
					if( topSideY < bestY || ( topSideY == bestY && freeRectangles[i].x < bestX ) )
					{
						bestNode.x = freeRectangles[i].x;
						bestNode.y = freeRectangles[i].y;
						bestNode.width = height;
						bestNode.height = width;
						bestY = topSideY;
						bestX = freeRectangles[i].x;
					}
				}
			}
			return bestNode;
		}
		Rect FindPositionForNewNodeBestShortSideFit( int width, int height, int & bestShortSideFit, int & bestLongSideFit ) const
		{
			Rect bestNode;
			memset( &bestNode, 0, sizeof( Rect ) );

			bestShortSideFit = std::numeric_limits<int>::max();
			bestLongSideFit = std::numeric_limits<int>::max();

			for( size_t i = 0; i < freeRectangles.size(); ++i )
			{
				// Try to place the rectangle in upright (non-flipped) orientation.
				if( freeRectangles[i].width >= width && freeRectangles[i].height >= height )
				{
					int leftoverHoriz = std::abs( freeRectangles[i].width - width );
					int leftoverVert = std::abs( freeRectangles[i].height - height );
					int shortSideFit = std::min( leftoverHoriz, leftoverVert );
					int longSideFit = std::max( leftoverHoriz, leftoverVert );

					if( shortSideFit < bestShortSideFit || ( shortSideFit == bestShortSideFit && longSideFit < bestLongSideFit ) )
					{
						bestNode.x = freeRectangles[i].x;
						bestNode.y = freeRectangles[i].y;
						bestNode.width = width;
						bestNode.height = height;
						bestShortSideFit = shortSideFit;
						bestLongSideFit = longSideFit;
					}
				}

				if( binAllowFlip && freeRectangles[i].width >= height && freeRectangles[i].height >= width )
				{
					int flippedLeftoverHoriz = std::abs( freeRectangles[i].width - height );
					int flippedLeftoverVert = std::abs( freeRectangles[i].height - width );
					int flippedShortSideFit = std::min( flippedLeftoverHoriz, flippedLeftoverVert );
					int flippedLongSideFit = std::max( flippedLeftoverHoriz, flippedLeftoverVert );

					if( flippedShortSideFit < bestShortSideFit || ( flippedShortSideFit == bestShortSideFit && flippedLongSideFit < bestLongSideFit ) )
					{
						bestNode.x = freeRectangles[i].x;
						bestNode.y = freeRectangles[i].y;
						bestNode.width = height;
						bestNode.height = width;
						bestShortSideFit = flippedShortSideFit;
						bestLongSideFit = flippedLongSideFit;
					}
				}
			}
			return bestNode;
		}
		Rect FindPositionForNewNodeBestLongSideFit( int width, int height, int & bestShortSideFit, int & bestLongSideFit ) const
		{
			Rect bestNode;
			memset( &bestNode, 0, sizeof( Rect ) );

			bestShortSideFit = std::numeric_limits<int>::max();
			bestLongSideFit = std::numeric_limits<int>::max();

			for( size_t i = 0; i < freeRectangles.size(); ++i )
			{
				// Try to place the rectangle in upright (non-flipped) orientation.
				if( freeRectangles[i].width >= width && freeRectangles[i].height >= height )
				{
					int leftoverHoriz = std::abs( freeRectangles[i].width - width );
					int leftoverVert = std::abs( freeRectangles[i].height - height );
					int shortSideFit = std::min( leftoverHoriz, leftoverVert );
					int longSideFit = std::max( leftoverHoriz, leftoverVert );

					if( longSideFit < bestLongSideFit || ( longSideFit == bestLongSideFit && shortSideFit < bestShortSideFit ) )
					{
						bestNode.x = freeRectangles[i].x;
						bestNode.y = freeRectangles[i].y;
						bestNode.width = width;
						bestNode.height = height;
						bestShortSideFit = shortSideFit;
						bestLongSideFit = longSideFit;
					}
				}

				if( binAllowFlip && freeRectangles[i].width >= height && freeRectangles[i].height >= width )
				{
					int leftoverHoriz = std::abs( freeRectangles[i].width - height );
					int leftoverVert = std::abs( freeRectangles[i].height - width );
					int shortSideFit = std::min( leftoverHoriz, leftoverVert );
					int longSideFit = std::max( leftoverHoriz, leftoverVert );

					if( longSideFit < bestLongSideFit || ( longSideFit == bestLongSideFit && shortSideFit < bestShortSideFit ) )
					{
						bestNode.x = freeRectangles[i].x;
						bestNode.y = freeRectangles[i].y;
						bestNode.width = height;
						bestNode.height = width;
						bestShortSideFit = shortSideFit;
						bestLongSideFit = longSideFit;
					}
				}
			}
			return bestNode;
		}
		Rect FindPositionForNewNodeBestAreaFit( int width, int height, int & bestAreaFit, int & bestShortSideFit ) const
		{
			Rect bestNode;
			memset( &bestNode, 0, sizeof( Rect ) );

			bestAreaFit = std::numeric_limits<int>::max();
			bestShortSideFit = std::numeric_limits<int>::max();

			for( size_t i = 0; i < freeRectangles.size(); ++i )
			{
				int areaFit = freeRectangles[i].width * freeRectangles[i].height - width * height;

				// Try to place the rectangle in upright (non-flipped) orientation.
				if( freeRectangles[i].width >= width && freeRectangles[i].height >= height )
				{
					int leftoverHoriz = std::abs( freeRectangles[i].width - width );
					int leftoverVert = std::abs( freeRectangles[i].height - height );
					int shortSideFit = std::min( leftoverHoriz, leftoverVert );

					if( areaFit < bestAreaFit || ( areaFit == bestAreaFit && shortSideFit < bestShortSideFit ) )
					{
						bestNode.x = freeRectangles[i].x;
						bestNode.y = freeRectangles[i].y;
						bestNode.width = width;
						bestNode.height = height;
						bestShortSideFit = shortSideFit;
						bestAreaFit = areaFit;
					}
				}

				if( binAllowFlip && freeRectangles[i].width >= height && freeRectangles[i].height >= width )
				{
					int leftoverHoriz = std::abs( freeRectangles[i].width - height );
					int leftoverVert = std::abs( freeRectangles[i].height - width );
					int shortSideFit = std::min( leftoverHoriz, leftoverVert );

					if( areaFit < bestAreaFit || ( areaFit == bestAreaFit && shortSideFit < bestShortSideFit ) )
					{
						bestNode.x = freeRectangles[i].x;
						bestNode.y = freeRectangles[i].y;
						bestNode.width = height;
						bestNode.height = width;
						bestShortSideFit = shortSideFit;
						bestAreaFit = areaFit;
					}
				}
			}
			return bestNode;
		}
		Rect FindPositionForNewNodeContactPoint( int width, int height, int & bestContactScore ) const
		{
			Rect bestNode;
			memset( &bestNode, 0, sizeof( Rect ) );

			bestContactScore = -1;

			for( size_t i = 0; i < freeRectangles.size(); ++i )
			{
				// Try to place the rectangle in upright (non-flipped) orientation.
				if( freeRectangles[i].width >= width && freeRectangles[i].height >= height )
				{
					int score = ContactPointScoreNode( freeRectangles[i].x, freeRectangles[i].y, width, height );
					if( score > bestContactScore )
					{
						bestNode.x = freeRectangles[i].x;
						bestNode.y = freeRectangles[i].y;
						bestNode.width = width;
						bestNode.height = height;
						bestContactScore = score;
					}
				}
				if( binAllowFlip && freeRectangles[i].width >= height && freeRectangles[i].height >= width )
				{
					int score = ContactPointScoreNode( freeRectangles[i].x, freeRectangles[i].y, height, width );
					if( score > bestContactScore )
					{
						bestNode.x = freeRectangles[i].x;
						bestNode.y = freeRectangles[i].y;
						bestNode.width = height;
						bestNode.height = width;
						bestContactScore = score;
					}
				}
			}
			return bestNode;
		}

		/// @return True if the free node was split.
		bool SplitFreeNode( Rect freeNode, const Rect & usedNode )
		{
			// Test with SAT if the rectangles even intersect.
			if( usedNode.x >= freeNode.x + freeNode.width || usedNode.x + usedNode.width <= freeNode.x ||
				usedNode.y >= freeNode.y + freeNode.height || usedNode.y + usedNode.height <= freeNode.y )
				return false;

			if( usedNode.x < freeNode.x + freeNode.width && usedNode.x + usedNode.width > freeNode.x )
			{
				// New node at the top side of the used node.
				if( usedNode.y > freeNode.y && usedNode.y < freeNode.y + freeNode.height )
				{
					Rect newNode = freeNode;
					newNode.height = usedNode.y - newNode.y;
					freeRectangles.push_back( newNode );
				}

				// New node at the bottom side of the used node.
				if( usedNode.y + usedNode.height < freeNode.y + freeNode.height )
				{
					Rect newNode = freeNode;
					newNode.y = usedNode.y + usedNode.height;
					newNode.height = freeNode.y + freeNode.height - ( usedNode.y + usedNode.height );
					freeRectangles.push_back( newNode );
				}
			}

			if( usedNode.y < freeNode.y + freeNode.height && usedNode.y + usedNode.height > freeNode.y )
			{
				// New node at the left side of the used node.
				if( usedNode.x > freeNode.x && usedNode.x < freeNode.x + freeNode.width )
				{
					Rect newNode = freeNode;
					newNode.width = usedNode.x - newNode.x;
					freeRectangles.push_back( newNode );
				}

				// New node at the right side of the used node.
				if( usedNode.x + usedNode.width < freeNode.x + freeNode.width )
				{
					Rect newNode = freeNode;
					newNode.x = usedNode.x + usedNode.width;
					newNode.width = freeNode.x + freeNode.width - ( usedNode.x + usedNode.width );
					freeRectangles.push_back( newNode );
				}
			}

			return true;
		}

		/// Goes through the free rectangle list and removes any redundant entries.
		void PruneFreeList()
		{
			/*
			///  Would be nice to do something like this, to avoid a Theta(n^2) loop through each pair.
			///  But unfortunately it doesn't quite cut it, since we also want to detect containment.
			///  Perhaps there's another way to do this faster than Theta(n^2).

			if (freeRectangles.size() > 0)
				clb::sort::QuickSort(&freeRectangles[0], freeRectangles.size(), NodeSortCmp);

			for(size_t i = 0; i < freeRectangles.size()-1; ++i)
				if (freeRectangles[i].x == freeRectangles[i+1].x &&
					freeRectangles[i].y == freeRectangles[i+1].y &&
					freeRectangles[i].width == freeRectangles[i+1].width &&
					freeRectangles[i].height == freeRectangles[i+1].height)
				{
					freeRectangles.erase(freeRectangles.begin() + i);
					--i;
				}
			*/

			/// Go through each pair and remove any rectangle that is redundant.
			for( size_t i = 0; i < freeRectangles.size(); ++i )
				for( size_t j = i + 1; j < freeRectangles.size(); ++j )
				{
					if( IsContainedIn( freeRectangles[i], freeRectangles[j] ) )
					{
						freeRectangles.erase( freeRectangles.begin() + i );
						--i;
						break;
					}
					if( IsContainedIn( freeRectangles[j], freeRectangles[i] ) )
					{
						freeRectangles.erase( freeRectangles.begin() + j );
						--j;
					}
				}
		}
	};

}



/************************************************************************************************************************************/
/*                                         Start Implementation                                                                     */
/************************************************************************************************************************************/

using namespace Fission;

surface_map::surface_map()
	: m_MaxSize(MaxWidth,MaxHeight)
{}

bool surface_map::Load( const file::path & file )
{
	try {

	}
	catch( ... )
	{
		return false;
	}
	return false;
}

bool surface_map::Save( const file::path & file ) const
{
	try
	{

	}
	catch (...)
	{
		return false;
	}

	return true;
}

void surface_map::clear()
{
	m_Map.clear();
	m_Surface.reset();
}

bool surface_map::empty() const
{
	return m_Map.empty();
}

const Surface * surface_map::get() const
{
	return m_Surface.get();
}

std::unique_ptr<Fission::Surface> && surface_map::release()
{
	return std::move( m_Surface );
}

void surface_map::remove( const std::string & key )
{
	m_Map.erase( key );
}

void surface_map::emplace( const std::string & key, const Surface * surface )
{
	sub_surface subs;
	subs.source = surface;
	m_Map.emplace( key, subs );
}

sub_surface * surface_map::operator[]( const std::string & key )
{
	auto it = m_Map.find( key );
	if( it == m_Map.end() ) return nullptr;
	return &it->second;
}

const sub_surface * surface_map::operator[]( const std::string & key ) const
{
	auto it = m_Map.find( key );
	if( it == m_Map.end() ) return nullptr;
	return &it->second;
}

bool surface_map::build( BuildFlags flags )
{
	(void)flags; //unused

	std::vector<rbp::RectSize> input;
	input.reserve( m_Map.size() );
	for( auto && [key, subs] : m_Map )
	{
		vec2i size = (vec2i)subs.source->size();
		input.emplace_back( size.x, size.y, &subs );
	}

	std::vector<rbp::Rect> out_rects;

	rbp::MaxRectsBinPack bin( MaxWidth, MaxHeight, false );

	bin.Insert( input, out_rects, rbp::MaxRectsBinPack::RectContactPointRule ); // todo: add condition for failure

	{
		Surface::CreateInfo info;
		info.FillColor = Colors::Transparent;
		info.Width = (uint32_t)m_MaxSize.x;
		info.Height = (uint32_t)m_MaxSize.y;
		m_Surface = Surface::Create( info );
	}

	vec2f scale = { 1.0f / (float)m_MaxSize.x, 1.0f / (float)m_MaxSize.y };
	for( auto && r : out_rects )
	{
		sub_surface * sub = reinterpret_cast<sub_surface *>( r.userdata );
		sub->quad.abs = recti( r.x, r.x + r.width, r.y * scale.y, ( r.y + r.height ) * scale.y );
		sub->quad.rel = (rectf)sub->quad.abs * scale.x; // todo: fix for differing width and height

		m_Surface->insert( r.x, r.y, sub->source );
	}
	
	return true;
}


using table_t = std::unordered_map<std::string, metadata>;
using array_t = std::vector<metadata>;
using number_t = double;
using integer_t = long long;
using string_t = std::string;

template <typename T, typename source_t> static constexpr T & as( const source_t & src ) { return *reinterpret_cast<T *>( src ); }


metadata::metadata( const metadata & src )
{
	*this = src;
}

metadata & metadata::operator=( const metadata & src )
{
	m_Type = src.m_Type;
	switch( m_Type )
	{
	case metadata::number:
	{
		m_pData = new number_t( as<number_t>( src.m_pData ) );
		break;
	}
	case metadata::integer:
	{
		m_pData = new integer_t( as<integer_t>( src.m_pData ) );
		break;
	}
	case metadata::string:
	{
		m_pData = new string_t( std::string( as<string_t>( src.m_pData ) ) );
		break;
	}
	case metadata::array:
	{
		m_pData = new array_t( as<array_t>( src.m_pData ) );
		break;
	}
	case metadata::table:
	{
		m_pData = new table_t( as<table_t>( src.m_pData ) );
		break;
	}
	default:break;
	}
	return *this;
}

metadata::metadata() : m_Type( empty ) {}

metadata::metadata( int _X ): m_Type(integer) { m_pData = new integer_t( (integer_t)_X ); }

metadata::metadata( long long _X ) : m_Type( integer ) { m_pData = new integer_t( (integer_t)_X ); }

metadata::metadata( float _X ) : m_Type( number ) { m_pData = new number_t( (number_t)_X ); }

metadata::metadata( double _X ) : m_Type( number ) { m_pData = new number_t( (number_t)_X ); }

metadata::metadata( const char * _X ) : m_Type( string ) { m_pData = new string_t( std::string( _X ) ); }

metadata::metadata( const std::string & _X ) : m_Type( string ) { m_pData = new string_t( std::string(_X)  ); }

metadata::metadata( nullptr_t _X ) : m_Type( null ) {}

metadata::metadata( metadata && src ): m_Type(src.m_Type), m_pData(src.m_pData)
{
	src.m_Type = empty;
}


metadata::value_t metadata::type() const { return m_Type; }

metadata::~metadata()
{
	switch( m_Type )
	{
	case Fission::metadata::number:
	{
		delete m_pData;
		break;
	}
	case Fission::metadata::integer:
	{
		delete m_pData;
		break;
	}
	case Fission::metadata::string:
	{
		delete reinterpret_cast<string_t*>( m_pData );
		break;
	}
	case Fission::metadata::array:
	{
		as<array_t>( m_pData ).~array_t();
		delete m_pData;
		break;
	}
	case Fission::metadata::table:
	{
		as<table_t>( m_pData ).~table_t();
		delete m_pData;
		break;
	}
	default:break;
	}
	m_Type = empty;
}

const metadata & metadata::operator[]( const std::string & key ) const
{
	if( m_Type != table ) return *this;

	table_t & _table = as<table_t>( m_pData );

	return _table[key];
}

metadata & metadata::operator[]( const std::string & key )
{
	if( m_Type != table )
	{
		switch( m_Type )
		{
		case Fission::metadata::number:
		case Fission::metadata::integer:
		case Fission::metadata::string:
		case Fission::metadata::array:
		case Fission::metadata::table:
		{
			delete m_pData;
			break;
		}
		default:break;
		}
		m_Type = table;
		m_pData = new table_t;
	}

	table_t & _table = as<table_t>( m_pData );

	return _table[key];
}

const metadata & metadata::operator[]( size_t index ) const
{
	if( m_Type != array ) return *this;

	array_t & _array = as<array_t>( m_pData );

	return _array[index];
}

metadata & metadata::operator[]( size_t index )
{
	if( m_Type != array )
	{
		this->~metadata();
		m_Type = array;
		m_pData = new array_t;
	}

	array_t & _array = as<array_t>( m_pData );

	if( index >= _array.size() )
	{
		_array.resize( index + 1 );
	}

	return _array[index];
}

const char * metadata::as_string() const
{
	if( m_Type != string ) return nullptr;

	return as<string_t>( m_pData ).c_str();
}

FISSION_API double metadata::as_number() const
{
	switch( m_Type )
	{
	case Fission::metadata::number: return as<number_t>( m_pData );
	case Fission::metadata::integer: return (number_t)as<integer_t>( m_pData );
	case Fission::metadata::string:
	{
		try {
			number_t n = std::stod( as<string_t>( m_pData ) );
			return n;
		}
		catch( ... ) {}
	}
	default: return 0.0; // fall through and return 0 on error
	}
}

FISSION_API long long metadata::as_integer() const
{
	switch( m_Type )
	{
	case Fission::metadata::number: return (integer_t)as<number_t>( m_pData );
	case Fission::metadata::integer: return as<integer_t>( m_pData );
	case Fission::metadata::string:
	{
		try {
			integer_t n = std::stoll( as<string_t>( m_pData ) );
			return n;
		}
		catch( ... ) {}
	}
	default: return 0.0; // fall through and return 0 on error
	}
}

FISSION_API metadata::const_iterator metadata::cbegin() const
{
	switch( m_Type )
	{
	case metadata::table: return as<table_t>( m_pData ).cbegin();
	default:return const_iterator();
	}
}

FISSION_API metadata::const_iterator metadata::cend() const
{
	switch( m_Type )
	{
	case metadata::table: return as<table_t>( m_pData ).cend();
	default:return const_iterator();
	}
}

FISSION_API size_t metadata::size() const
{
	switch( m_Type )
	{
	case metadata::array:
	{
		return as<array_t>( m_pData ).size();
	}
	default:return 0;
	}
}

void metadata::resize( size_t n )
{
	if( m_Type != array )
	{
		this->~metadata();
		m_Type = array;
		m_pData = new array_t;
	}

	array_t & _array = as<array_t>( m_pData );

	return _array.resize(n);
}

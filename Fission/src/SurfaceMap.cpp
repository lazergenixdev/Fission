#include "Fission/Core/SurfaceMap.h"

/** @file MaxRectsBinPack.h
	@author Jukka Jyl�nki

	@brief Implements different bin packer algorithms that use the MAXRECTS data structure.

	This work is released to Public Domain, do whatever you want with it.
*/
#include <vector>
#include <algorithm>
#include <utility>
#include <fstream>
#include <limits>
#include <cassert>
#include <cstring>
#include <cmath>
#include <cstdlib>

#include "nlohmann/json.hpp"

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
				size_t bestRectIndex = SIZE_MAX;
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

				if( bestRectIndex == SIZE_MAX )
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

static nlohmann::json to_json( const metadata & md )
{
	switch( md.type() )
	{
	case metadata::boolean: return nlohmann::json( md.as_boolean() );
	case metadata::number: return nlohmann::json( md.as_number() );
	case metadata::integer: return nlohmann::json( md.as_integer() );
	case metadata::string: return nlohmann::json( md.as_string() );
	case metadata::array:
	{
		nlohmann::json json;
		for( size_t i = 0; i < md.size(); ++i )
			json.emplace_back( to_json( md[i] ) );
		return json;
	}
	case metadata::table:
	{
		nlohmann::json json;
		for( auto && [key, val] : md )
			json[key] = to_json( val );
		return json;
	}
	default: return nlohmann::json();
	}
}
static metadata from_json( const nlohmann::json & node )
{
	namespace json = nlohmann::detail;
	switch( node.type() )
	{
	case json::value_t::boolean: return node.get<bool>();
	case json::value_t::string: return node.get<std::string>();
	case json::value_t::number_float: return node.get<double>();
	case json::value_t::number_unsigned:
	case json::value_t::number_integer: return node.get<long long>();
	case json::value_t::array:
	{
		metadata out;
		out.resize( node.size() );
		size_t i = 0;
		for( auto & element : node )
			out[i++] = from_json(element);
		return out;
	}
	case json::value_t::object:
	{
		metadata out;
		for( auto & [key,value] : node.items() )
			out[key] = from_json( value );
		return out;
	}
	default: return metadata();
	}
}

surface_map::surface_map()
	: m_MaxSize(MaxWidth,MaxHeight), m_MetaData()
{}

bool surface_map::Load( const file::path & file )
{
	m_MetaData = {};
	m_Map.clear();
	try 
	{
		auto imageFilePath = file.parent_path();
		auto metaFilePath = file;

		metaFilePath.replace_extension( ".json" );

		nlohmann::json desc;
		std::ifstream ifs( metaFilePath );
		ifs >> desc;
		ifs.close();

		if( !m_Surface ) m_Surface = Surface::Create();

		{
			std::string temp = desc["__file__"];
			m_Surface->Load( imageFilePath /= file::path( temp ) );
			desc.erase( "__file__" );
		}

		m_MetaData = from_json( desc["__metadata__"] );
		desc.erase( "__metadata__" );

		vec2f scale = (vec2f)m_Surface->size();
		if( scale.x <= 0.0f || scale.y <= 0.0f ) return false;
		scale = { 1.0f / scale.x, 1.0f / scale.y };

		for( auto && [key, value] : desc.items() )
		{
			sub_surface subs;
			auto & rc = subs.region.abs;

			rc.x.low = value["$left"].get<int>();
			rc.x.high = value["$right"].get<int>();
			rc.y.low = value["$top"].get<int>();
			rc.y.high = value["$bottom"].get<int>();
			subs.region.flipped = value["[flip]"].get<bool>();
			subs.meta = from_json( value["__metadata__"] );

			subs.region.rel = rectf( (rangef)rc.x * scale.x, (rangef)rc.y * scale.y );

			m_Map.emplace( key, subs );
		}
	}
	catch( ... )
	{
		return false;
	}
	return true;
}
#include "Fission/Core/Console.h"

bool surface_map::Save( const file::path & file ) const
{
	if( !m_Surface ) return false;
	try
	{
		auto imageFilePath = file;
		auto metaFilePath = file;

		imageFilePath.replace_extension( ".png" );
		metaFilePath.replace_extension( ".json" );

		nlohmann::json meta;

		meta["__file__"] = imageFilePath.filename().string();
		meta["__metadata__"] = to_json( m_MetaData );
		for( auto && [key, subs] : m_Map )
		{
			auto & data = meta[key];
			auto & rc = subs.region.abs;

			data["$left"] = rc.x.low;
			data["$right"] = rc.x.high;
			data["$top"] = rc.y.low;
			data["$bottom"] = rc.y.high;
			data["[flip]"] = false;

			if( subs.meta.type() != metadata::empty )
				data["__metadata__"] = to_json( subs.meta );
		}

		m_Surface->Save( imageFilePath );
		std::ofstream ofs( metaFilePath );
		ofs << std::setw( 4 ) << meta;
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
		info.fillColor = Colors::Transparent;
		info.width = (uint32_t)m_MaxSize.x;
		info.height = (uint32_t)m_MaxSize.y;
		m_Surface = Surface::Create( info );
	}

	vec2f scale = { 1.0f / (float)m_MaxSize.x, 1.0f / (float)m_MaxSize.y };
	for( auto && r : out_rects )
	{
		sub_surface * sub = reinterpret_cast<sub_surface *>( r.userdata );
		sub->region.abs = recti( r.x, r.x + r.width, r.y, ( r.y + r.height ) );
		sub->region.rel = (rectf)sub->region.abs * scale.x; // todo: fix for differing width and height

		m_Surface->insert( r.x, r.y, sub->source );
	}
	
	return true;
}

void surface_map::set_metadata( const Fission::metadata & meta ) { m_MetaData = meta; }

metadata & surface_map::get_metadata() { return m_MetaData; }

const metadata & surface_map::get_metadata() const { return m_MetaData; }

surface_map::iterator surface_map::begin() { return m_Map.begin(); }

surface_map::iterator surface_map::end() { return m_Map.end(); }


metadata::metadata( const metadata & src )
{
	*this = src;
}

metadata & metadata::operator=( const metadata & src )
{
	m_Type = src.m_Type;
	switch( m_Type )
	{
	case metadata::boolean:
	{
		m_pData = src.m_pData;
		break;
	}
	case metadata::number:
	{
		m_pData.m_number = src.m_pData.m_number;
		break;
	}
	case metadata::integer:
	{
		m_pData.m_integer = src.m_pData.m_integer;
		break;
	}
	case metadata::string:
	{
		m_pData.m_string = new string_t( *src.m_pData.m_string );
		break;
	}
	case metadata::array:
	{
		m_pData.m_array = new array_t( *src.m_pData.m_array );
		break;
	}
	case metadata::table:
	{
		m_pData.m_table = new table_t( *src.m_pData.m_table );
		break;
	}
	default:break;
	}
	return *this;
}

metadata::metadata() : m_Type( empty ) {}

metadata::metadata( bool _X ) : m_Type( boolean ) { m_pData.m_boolean = _X; }

metadata::metadata( int _X ) : m_Type( integer ) { m_pData.m_integer = (integer_t)_X; }

metadata::metadata( long long _X ) : m_Type( integer ) { m_pData.m_integer = (integer_t)_X; }

metadata::metadata( float _X ) : m_Type( number ) { m_pData.m_number = (number_t)_X; }

metadata::metadata( double _X ) : m_Type( number ) { m_pData.m_number = (number_t)_X; }

metadata::metadata( const char * _X ) : m_Type( string ) { m_pData.m_string = new string_t( _X ); }

metadata::metadata( const std::string & _X ) : m_Type( string ) { m_pData.m_string = new string_t( std::string(_X) ); }

metadata::metadata( metadata && src ): m_Type(src.m_Type), m_pData(src.m_pData) { src.m_Type = empty; }

metadata::value_t metadata::type() const { return m_Type; }

metadata::~metadata()
{
	switch( m_Type )
	{
	case Fission::metadata::string:
	{
		delete m_pData.m_string;
		break;
	}
	case Fission::metadata::array:
	{
		delete m_pData.m_array;
		break;
	}
	case Fission::metadata::table:
	{
		delete m_pData.m_table;
		break;
	}
	default:break;
	}
	m_Type = empty;
}

const metadata & metadata::operator[]( const std::string & key ) const
{
	if( m_Type != table ) return *this; // undefined behavior

	table_t & _table = *m_pData.m_table;

	return _table[key];
}

metadata & metadata::operator[]( const std::string & key )
{
	if( m_Type != table )
	{
		this->~metadata();
		m_Type = table;
		m_pData.m_table = new table_t;
	}

	table_t & _table = *m_pData.m_table;

	return _table[key];
}

const metadata & metadata::operator[]( size_t index ) const
{
	if( m_Type != array ) return *this; // undefined behavior

	array_t & _array = *m_pData.m_array;

	return _array[index];
}

metadata & metadata::operator[]( size_t index )
{
	if( m_Type != array )
	{
		this->~metadata();
		m_Type = array;
		m_pData.m_array = new array_t;
	}

	array_t & _array = *m_pData.m_array;

	if( index >= _array.size() )
	{
		_array.resize( index + 1 );
	}

	return _array[index];
}

const char * metadata::as_string() const
{
	if( m_Type != string ) return nullptr; // undefined behavior

	return m_pData.m_string->c_str();
}

bool metadata::as_boolean() const
{
	switch( m_Type )
	{
	case metadata::boolean: return m_pData.m_boolean;
	case metadata::number: return (bool)m_pData.m_number;
	case metadata::integer: return (bool)m_pData.m_integer;
	case metadata::string:
	{
		std::string s = *m_pData.m_string;
		std::for_each( s.begin(), s.end(), [] ( char & c ) {c = std::tolower( c ); } );
		if( s == "true" ) return true;
	}
	default: return false; // fall through and return false on error : undefined behavior
	}
}

double metadata::as_number() const
{
	switch( m_Type )
	{
	case Fission::metadata::number: return m_pData.m_number;
	case Fission::metadata::integer: return (number_t)m_pData.m_integer;
	case Fission::metadata::string:
	{
		try {
			number_t n = std::stod( *m_pData.m_string );
			return n;
		}
		catch( ... ) {}
	}
	default: return 0.0; // fall through and return 0 on error : undefined behavior
	}
}

long long metadata::as_integer() const
{
	switch( m_Type )
	{
	case Fission::metadata::number: return (integer_t)m_pData.m_number;
	case Fission::metadata::integer: return m_pData.m_integer;
	case Fission::metadata::string:
	{
		try {
			integer_t n = std::stoll( *m_pData.m_string );
			return n;
		}
		catch( ... ) {}
	}
	default: return 0; // fall through and return 0 on error : undefined behavior
	}
}

metadata::const_iterator metadata::begin() const
{
	switch( m_Type )
	{
	case metadata::table: return m_pData.m_table->cbegin();
	default:return const_iterator();
	}
}

metadata::const_iterator metadata::end() const
{
	switch( m_Type )
	{
	case metadata::table: return m_pData.m_table->cend();
	default:return const_iterator();
	}
}

size_t metadata::size() const
{
	switch( m_Type )
	{
	case metadata::array:
	{
		return m_pData.m_array->size();
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
		m_pData.m_array = new array_t;
	}

	m_pData.m_array->resize(n);
}

metadata metadata::from_JSON( const std::string & json_str )
{
	metadata meta;
	try
	{
		meta = from_json( nlohmann::json::parse( json_str, {}, true, true ) );
	}
	catch (nlohmann::json::parse_error & e)
	{
		throw lazer::exception("JSON parse error", _lazer_exception_msg.append(e.what()));
	}
	catch (...)
	{
		throw std::logic_error("this don't make no fucking sense");
	}

	return meta;
}

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
	inline bool IsContainedIn( const Rect & a, const Rect & b )
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
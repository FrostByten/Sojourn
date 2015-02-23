#ifndef MAP_H_
#define MAP_H_

#include <vector>
#include "Cell.h"

namespace Marx
{
	typedef unsigned int uint;
	/* 
	*	Map base class
	*   
	*/
	class Map
	{
		public:
			Map(const uint& height, const uint& width);
			unsigned int getHeight();
			unsigned int getWidth();
			void setCell(const uint& x, const uint& y, const Cell& cell);
			Cell getCell(const uint& x, const uint& y);
		
		private:
			uint width_;
			uint height_;
			std::vector<Cell> cells_;
	};
} /* namespace Marx */

#endif /* MAP_H_ */
#include "fluid/mesher.h"

/// \file
/// Implementation of the mesher.

namespace fluid {
	/// The edge table for the marching cubes algorithm, taken from http://paulbourke.net/geometry/polygonise/.
	static const std::uint16_t _edge_table[256]{
		0x000, 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
		0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
		0x190, 0x099, 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
		0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
		0x230, 0x339, 0x033, 0x13a, 0x636, 0x73f, 0x435, 0x53c,
		0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
		0x3a0, 0x2a9, 0x1a3, 0x0aa, 0x7a6, 0x6af, 0x5a5, 0x4ac,
		0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
		0x460, 0x569, 0x663, 0x76a, 0x066, 0x16f, 0x265, 0x36c,
		0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
		0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0x0ff, 0x3f5, 0x2fc,
		0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
		0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x055, 0x15c,
		0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
		0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0x0cc,
		0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
		0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
		0x0cc, 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
		0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
		0x15c, 0x055, 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
		0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
		0x2fc, 0x3f5, 0x0ff, 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
		0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
		0x36c, 0x265, 0x16f, 0x066, 0x76a, 0x663, 0x569, 0x460,
		0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
		0x4ac, 0x5a5, 0x6af, 0x7a6, 0x0aa, 0x1a3, 0x2a9, 0x3a0,
		0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
		0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x033, 0x339, 0x230,
		0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
		0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x099, 0x190,
		0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
		0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x000
	};
	constexpr static std::uint8_t _end = std::numeric_limits<std::uint8_t>::max();
	/// The triangle table for the marching cubes algorithm, taken from http://paulbourke.net/geometry/polygonise/.
	static const std::uint8_t _tri_table[256][16]{
		{ _end },
	{ 0, 8, 3, _end },
	{ 0, 1, 9, _end },
	{ 1, 8, 3, 9, 8, 1, _end },
	{ 1, 2, 10, _end },
	{ 0, 8, 3, 1, 2, 10, _end },
	{ 9, 2, 10, 0, 2, 9, _end },
	{ 2, 8, 3, 2, 10, 8, 10, 9, 8, _end },
	{ 3, 11, 2, _end },
	{ 0, 11, 2, 8, 11, 0, _end },
	{ 1, 9, 0, 2, 3, 11, _end },
	{ 1, 11, 2, 1, 9, 11, 9, 8, 11, _end },
	{ 3, 10, 1, 11, 10, 3, _end },
	{ 0, 10, 1, 0, 8, 10, 8, 11, 10, _end },
	{ 3, 9, 0, 3, 11, 9, 11, 10, 9, _end },
	{ 9, 8, 10, 10, 8, 11, _end },
	{ 4, 7, 8, _end },
	{ 4, 3, 0, 7, 3, 4, _end },
	{ 0, 1, 9, 8, 4, 7, _end },
	{ 4, 1, 9, 4, 7, 1, 7, 3, 1, _end },
	{ 1, 2, 10, 8, 4, 7, _end },
	{ 3, 4, 7, 3, 0, 4, 1, 2, 10, _end },
	{ 9, 2, 10, 9, 0, 2, 8, 4, 7, _end },
	{ 2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, _end },
	{ 8, 4, 7, 3, 11, 2, _end },
	{ 11, 4, 7, 11, 2, 4, 2, 0, 4, _end },
	{ 9, 0, 1, 8, 4, 7, 2, 3, 11, _end },
	{ 4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, _end },
	{ 3, 10, 1, 3, 11, 10, 7, 8, 4, _end },
	{ 1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, _end },
	{ 4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, _end },
	{ 4, 7, 11, 4, 11, 9, 9, 11, 10, _end },
	{ 9, 5, 4, _end },
	{ 9, 5, 4, 0, 8, 3, _end },
	{ 0, 5, 4, 1, 5, 0, _end },
	{ 8, 5, 4, 8, 3, 5, 3, 1, 5, _end },
	{ 1, 2, 10, 9, 5, 4, _end },
	{ 3, 0, 8, 1, 2, 10, 4, 9, 5, _end },
	{ 5, 2, 10, 5, 4, 2, 4, 0, 2, _end },
	{ 2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, _end },
	{ 9, 5, 4, 2, 3, 11, _end },
	{ 0, 11, 2, 0, 8, 11, 4, 9, 5, _end },
	{ 0, 5, 4, 0, 1, 5, 2, 3, 11, _end },
	{ 2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, _end },
	{ 10, 3, 11, 10, 1, 3, 9, 5, 4, _end },
	{ 4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, _end },
	{ 5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, _end },
	{ 5, 4, 8, 5, 8, 10, 10, 8, 11, _end },
	{ 9, 7, 8, 5, 7, 9, _end },
	{ 9, 3, 0, 9, 5, 3, 5, 7, 3, _end },
	{ 0, 7, 8, 0, 1, 7, 1, 5, 7, _end },
	{ 1, 5, 3, 3, 5, 7, _end },
	{ 9, 7, 8, 9, 5, 7, 10, 1, 2, _end },
	{ 10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, _end },
	{ 8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, _end },
	{ 2, 10, 5, 2, 5, 3, 3, 5, 7, _end },
	{ 7, 9, 5, 7, 8, 9, 3, 11, 2, _end },
	{ 9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, _end },
	{ 2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, _end },
	{ 11, 2, 1, 11, 1, 7, 7, 1, 5, _end },
	{ 9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, _end },
	{ 5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, _end },
	{ 11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, _end },
	{ 11, 10, 5, 7, 11, 5, _end },
	{ 10, 6, 5, _end },
	{ 0, 8, 3, 5, 10, 6, _end },
	{ 9, 0, 1, 5, 10, 6, _end },
	{ 1, 8, 3, 1, 9, 8, 5, 10, 6, _end },
	{ 1, 6, 5, 2, 6, 1, _end },
	{ 1, 6, 5, 1, 2, 6, 3, 0, 8, _end },
	{ 9, 6, 5, 9, 0, 6, 0, 2, 6, _end },
	{ 5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, _end },
	{ 2, 3, 11, 10, 6, 5, _end },
	{ 11, 0, 8, 11, 2, 0, 10, 6, 5, _end },
	{ 0, 1, 9, 2, 3, 11, 5, 10, 6, _end },
	{ 5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, _end },
	{ 6, 3, 11, 6, 5, 3, 5, 1, 3, _end },
	{ 0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, _end },
	{ 3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, _end },
	{ 6, 5, 9, 6, 9, 11, 11, 9, 8, _end },
	{ 5, 10, 6, 4, 7, 8, _end },
	{ 4, 3, 0, 4, 7, 3, 6, 5, 10, _end },
	{ 1, 9, 0, 5, 10, 6, 8, 4, 7, _end },
	{ 10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, _end },
	{ 6, 1, 2, 6, 5, 1, 4, 7, 8, _end },
	{ 1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, _end },
	{ 8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, _end },
	{ 7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, _end },
	{ 3, 11, 2, 7, 8, 4, 10, 6, 5, _end },
	{ 5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, _end },
	{ 0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, _end },
	{ 9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, _end },
	{ 8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, _end },
	{ 5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, _end },
	{ 0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, _end },
	{ 6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, _end },
	{ 10, 4, 9, 6, 4, 10, _end },
	{ 4, 10, 6, 4, 9, 10, 0, 8, 3, _end },
	{ 10, 0, 1, 10, 6, 0, 6, 4, 0, _end },
	{ 8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, _end },
	{ 1, 4, 9, 1, 2, 4, 2, 6, 4, _end },
	{ 3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, _end },
	{ 0, 2, 4, 4, 2, 6, _end },
	{ 8, 3, 2, 8, 2, 4, 4, 2, 6, _end },
	{ 10, 4, 9, 10, 6, 4, 11, 2, 3, _end },
	{ 0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, _end },
	{ 3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, _end },
	{ 6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, _end },
	{ 9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, _end },
	{ 8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, _end },
	{ 3, 11, 6, 3, 6, 0, 0, 6, 4, _end },
	{ 6, 4, 8, 11, 6, 8, _end },
	{ 7, 10, 6, 7, 8, 10, 8, 9, 10, _end },
	{ 0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, _end },
	{ 10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, _end },
	{ 10, 6, 7, 10, 7, 1, 1, 7, 3, _end },
	{ 1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, _end },
	{ 2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, _end },
	{ 7, 8, 0, 7, 0, 6, 6, 0, 2, _end },
	{ 7, 3, 2, 6, 7, 2, _end },
	{ 2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, _end },
	{ 2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, _end },
	{ 1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, _end },
	{ 11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, _end },
	{ 8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, _end },
	{ 0, 9, 1, 11, 6, 7, _end },
	{ 7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, _end },
	{ 7, 11, 6, _end },
	{ 7, 6, 11, _end },
	{ 3, 0, 8, 11, 7, 6, _end },
	{ 0, 1, 9, 11, 7, 6, _end },
	{ 8, 1, 9, 8, 3, 1, 11, 7, 6, _end },
	{ 10, 1, 2, 6, 11, 7, _end },
	{ 1, 2, 10, 3, 0, 8, 6, 11, 7, _end },
	{ 2, 9, 0, 2, 10, 9, 6, 11, 7, _end },
	{ 6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, _end },
	{ 7, 2, 3, 6, 2, 7, _end },
	{ 7, 0, 8, 7, 6, 0, 6, 2, 0, _end },
	{ 2, 7, 6, 2, 3, 7, 0, 1, 9, _end },
	{ 1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, _end },
	{ 10, 7, 6, 10, 1, 7, 1, 3, 7, _end },
	{ 10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, _end },
	{ 0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, _end },
	{ 7, 6, 10, 7, 10, 8, 8, 10, 9, _end },
	{ 6, 8, 4, 11, 8, 6, _end },
	{ 3, 6, 11, 3, 0, 6, 0, 4, 6, _end },
	{ 8, 6, 11, 8, 4, 6, 9, 0, 1, _end },
	{ 9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, _end },
	{ 6, 8, 4, 6, 11, 8, 2, 10, 1, _end },
	{ 1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, _end },
	{ 4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, _end },
	{ 10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, _end },
	{ 8, 2, 3, 8, 4, 2, 4, 6, 2, _end },
	{ 0, 4, 2, 4, 6, 2, _end },
	{ 1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, _end },
	{ 1, 9, 4, 1, 4, 2, 2, 4, 6, _end },
	{ 8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, _end },
	{ 10, 1, 0, 10, 0, 6, 6, 0, 4, _end },
	{ 4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, _end },
	{ 10, 9, 4, 6, 10, 4, _end },
	{ 4, 9, 5, 7, 6, 11, _end },
	{ 0, 8, 3, 4, 9, 5, 11, 7, 6, _end },
	{ 5, 0, 1, 5, 4, 0, 7, 6, 11, _end },
	{ 11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, _end },
	{ 9, 5, 4, 10, 1, 2, 7, 6, 11, _end },
	{ 6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, _end },
	{ 7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, _end },
	{ 3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, _end },
	{ 7, 2, 3, 7, 6, 2, 5, 4, 9, _end },
	{ 9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, _end },
	{ 3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, _end },
	{ 6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, _end },
	{ 9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, _end },
	{ 1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, _end },
	{ 4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, _end },
	{ 7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, _end },
	{ 6, 9, 5, 6, 11, 9, 11, 8, 9, _end },
	{ 3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, _end },
	{ 0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, _end },
	{ 6, 11, 3, 6, 3, 5, 5, 3, 1, _end },
	{ 1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, _end },
	{ 0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, _end },
	{ 11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, _end },
	{ 6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, _end },
	{ 5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, _end },
	{ 9, 5, 6, 9, 6, 0, 0, 6, 2, _end },
	{ 1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, _end },
	{ 1, 5, 6, 2, 1, 6, _end },
	{ 1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, _end },
	{ 10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, _end },
	{ 0, 3, 8, 5, 6, 10, _end },
	{ 10, 5, 6, _end },
	{ 11, 5, 10, 7, 5, 11, _end },
	{ 11, 5, 10, 11, 7, 5, 8, 3, 0, _end },
	{ 5, 11, 7, 5, 10, 11, 1, 9, 0, _end },
	{ 10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, _end },
	{ 11, 1, 2, 11, 7, 1, 7, 5, 1, _end },
	{ 0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, _end },
	{ 9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, _end },
	{ 7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, _end },
	{ 2, 5, 10, 2, 3, 5, 3, 7, 5, _end },
	{ 8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, _end },
	{ 9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, _end },
	{ 9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, _end },
	{ 1, 3, 5, 3, 7, 5, _end },
	{ 0, 8, 7, 0, 7, 1, 1, 7, 5, _end },
	{ 9, 0, 3, 9, 3, 5, 5, 3, 7, _end },
	{ 9, 8, 7, 5, 9, 7, _end },
	{ 5, 8, 4, 5, 10, 8, 10, 11, 8, _end },
	{ 5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, _end },
	{ 0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, _end },
	{ 10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, _end },
	{ 2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, _end },
	{ 0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, _end },
	{ 0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, _end },
	{ 9, 4, 5, 2, 11, 3, _end },
	{ 2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, _end },
	{ 5, 10, 2, 5, 2, 4, 4, 2, 0, _end },
	{ 3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, _end },
	{ 5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, _end },
	{ 8, 4, 5, 8, 5, 3, 3, 5, 1, _end },
	{ 0, 4, 5, 1, 0, 5, _end },
	{ 8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, _end },
	{ 9, 4, 5, _end },
	{ 4, 11, 7, 4, 9, 11, 9, 10, 11, _end },
	{ 0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, _end },
	{ 1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, _end },
	{ 3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, _end },
	{ 4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, _end },
	{ 9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, _end },
	{ 11, 7, 4, 11, 4, 2, 2, 4, 0, _end },
	{ 11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, _end },
	{ 2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, _end },
	{ 9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, _end },
	{ 3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, _end },
	{ 1, 10, 2, 8, 7, 4, _end },
	{ 4, 9, 1, 4, 1, 7, 7, 1, 3, _end },
	{ 4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, _end },
	{ 4, 0, 3, 7, 4, 3, _end },
	{ 4, 8, 7, _end },
	{ 9, 10, 8, 10, 11, 8, _end },
	{ 3, 0, 9, 3, 9, 11, 11, 9, 10, _end },
	{ 0, 1, 10, 0, 10, 8, 8, 10, 11, _end },
	{ 3, 1, 10, 11, 3, 10, _end },
	{ 1, 2, 11, 1, 11, 9, 9, 11, 8, _end },
	{ 3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, _end },
	{ 0, 2, 11, 8, 0, 11, _end },
	{ 3, 2, 11, _end },
	{ 2, 3, 8, 2, 8, 10, 10, 8, 9, _end },
	{ 9, 10, 2, 0, 9, 2, _end },
	{ 2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, _end },
	{ 1, 10, 2, _end },
	{ 1, 3, 8, 9, 1, 8, _end },
	{ 0, 9, 1, _end },
	{ 0, 3, 8, _end },
	{ _end }
	};
	/// The index offsets for each vertex.
	static const vec3s _cell_vertex_offsets[8]{
		{ 0, 0, 0 }, { 1, 0, 0 }, { 1, 1, 0 }, { 0, 1, 0 },
	{ 0, 0, 1 }, { 1, 0, 1 }, { 1, 1, 1 }, { 0, 1, 1 },
	};
	/// The vertices that correspond to each edge.
	static const std::uint8_t _edge_vert_table[12][2]{
		{ 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 },
	{ 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 },
	{ 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 }
	};


	void mesher::resize(vec3s size) {
		_surface_function = grid3<double>(size + vec3s(1, 1, 1));
		_hash.resize(size);
	}

	mesher::mesh_t mesher::generate_mesh(const std::vector<vec3d> &particles, double r) {
		_sample_surface_function(particles, r);
		return _marching_cubes();
	}

	double mesher::_kernel(double sqr_dist) const {
		sqr_dist = 1.0 - sqr_dist;
		if (sqr_dist > 0.0) {
			return sqr_dist * sqr_dist * sqr_dist;
		}
		return 0.0;
	}

	void mesher::_sample_surface_function(const std::vector<vec3d> &particles, double r) {
		_hash.clear();
		for (const vec3d &p : particles) {
			vec3i index((p - grid_offset) / cell_size);
			if (index.x > 0 && index.y > 0 && index.z > 0) {
				_hash.add_object_at(vec3s(index), &p);
			}
		}
		vec3s
			min_offset(cell_radius, cell_radius, cell_radius),
			max_offset(cell_radius - 1, cell_radius - 1, cell_radius - 1);
		int zmax = static_cast<int>(_surface_function.get_size().z);
#pragma omp parallel for
		for (int iz = 0; iz < zmax; ++iz) {
			auto z = static_cast<std::size_t>(iz);
			for (std::size_t y = 0; y < _surface_function.get_size().y; ++y) {
				for (std::size_t x = 0; x < _surface_function.get_size().x; ++x) {
					double tot_weight = 0.0, tot_rad = 0.0;
					vec3d tot_pos, grid_pos = grid_offset + cell_size * vec3d(vec3s(x, y, z));
					bool has_particles = false;
					_hash.for_all_nearby_objects(
						vec3s(x, y, z), min_offset, max_offset,
						[&](const vec3d *p) {
							vec3d pos = *p;
							has_particles = true;
							double w = _kernel(
								(pos - grid_pos).squared_length() / (particle_extent * particle_extent)
							);
							tot_weight += w;
							tot_rad += w * r;
							tot_pos += w * pos;
						}
					);
					double value = 1.0;
					if (has_particles) {
						tot_rad /= tot_weight;
						tot_pos /= tot_weight;
						value = (tot_pos - grid_pos).length() - tot_rad;
					}
					_surface_function(x, y, z) = value;
				}
			}
		}
	}

	std::size_t mesher::_add_point(
		std::vector<vec3d> &v, vec3s cell, double *surf_func, std::size_t edge_table_index
	) const {
		std::size_t res = v.size();
		const std::uint8_t *verts = _edge_vert_table[edge_table_index];
		double v1 = surf_func[verts[0]], v2 = surf_func[verts[1]];
		v.emplace_back(
			grid_offset + cell_size * lerp(
				vec3d(cell + _cell_vertex_offsets[verts[0]]),
				vec3d(cell + _cell_vertex_offsets[verts[1]]),
				v1 / (v1 - v2)
				)
		);
		return res;
	}

	/// Stores indices to edge midpoints in a layer.
	struct _edge_midpoints {
		std::size_t
			mid0, ///< The index of the midpoint of edge 0.
			mid3; ///< The index of the midpoint of edge 3.
	};
	mesher::mesh_t mesher::_marching_cubes() const {
		// http://paulbourke.net/geometry/polygonise/
		mesh_t result;
		vec2s layer_size_plus_one(_surface_function.get_size().x, _surface_function.get_size().y);
		grid2<_edge_midpoints>
			mid03_prev(layer_size_plus_one), // midpoints of edges 4-7 of the previouis layer
			mid03_cur(layer_size_plus_one); // midpoints of edges 4-7 of the current layer
		grid2<std::size_t> mid8(layer_size_plus_one); // midpoints of edge 8 of the current layer
		for (std::size_t z = 0; z + 1 < _surface_function.get_size().z; ++z) {
			for (std::size_t y = 0; y + 1 < _surface_function.get_size().y; ++y) {
				for (std::size_t x = 0; x + 1 < _surface_function.get_size().x; ++x) {
					double surf_func[8];
					std::uint8_t occupation = 0;
					for (std::size_t i = 0; i < 8; ++i) {
						double value = _surface_function(vec3s(x, y, z) + _cell_vertex_offsets[i]);
						surf_func[i] = value;
						occupation |= (value < 0 ? 1 : 0) << i;
					}
					std::uint16_t edge_list = _edge_table[occupation];
					if (edge_list == 0) { // either entirely inside or outside of the mesh
						continue;
					}
					std::size_t ids[12]{};
					{
						_edge_midpoints
							&mp00 = mid03_prev(x, y),
							&mp01 = mid03_prev(x + 1, y),
							&mp10 = mid03_prev(x, y + 1),
							&mc00 = mid03_cur(x, y);
						std::size_t
							&m800 = mid8(x, y),
							&m801 = mid8(x + 1, y),
							&m810 = mid8(x, y + 1);

						if (z == 0) {
							if (y == 0) {
								if ((edge_list & (1 << 0)) != 0) {
									mp00.mid0 = _add_point(result.positions, vec3s(x, y, z), surf_func, 0);
								}
							}
							if ((edge_list & (1 << 1)) != 0) {
								mp01.mid3 = _add_point(result.positions, vec3s(x, y, z), surf_func, 1);
							}
							if ((edge_list & (1 << 2)) != 0) {
								mp10.mid0 = _add_point(result.positions, vec3s(x, y, z), surf_func, 2);
							}
							if (x == 0) {
								if ((edge_list & (1 << 3)) != 0) {
									mp00.mid3 = _add_point(result.positions, vec3s(x, y, z), surf_func, 3);
								}
							}
						}
						ids[0] = mp00.mid0;
						ids[1] = mp01.mid3;
						ids[2] = mp10.mid0;
						ids[3] = mp00.mid3;

						if (y == 0) {
							if ((edge_list & (1 << 4)) != 0) {
								mc00.mid0 = _add_point(result.positions, vec3s(x, y, z), surf_func, 4);
							}
						}
						ids[4] = mc00.mid0;

						// 5 & 6 new

						if (x == 0) {
							if ((edge_list & (1 << 7)) != 0) {
								mc00.mid3 = _add_point(result.positions, vec3s(x, y, z), surf_func, 7);
							}
						}
						ids[7] = mc00.mid3;

						if (x == 0 && y == 0) {
							if ((edge_list & (1 << 8)) != 0) {
								m800 = _add_point(result.positions, vec3s(x, y, z), surf_func, 8);
							}
						}
						ids[8] = m800;

						if (y == 0) {
							if ((edge_list & (1 << 9)) != 0) {
								m801 = _add_point(result.positions, vec3s(x, y, z), surf_func, 9);
							}
						}
						ids[9] = m801;

						// 10 new

						if (x == 0) {
							if ((edge_list & (1 << 11)) != 0) {
								m810 = _add_point(result.positions, vec3s(x, y, z), surf_func, 11);
							}
						}
						ids[11] = m810;
					}
					for (std::size_t i : { 5, 6, 10 }) {
						if ((edge_list & (1 << i)) != 0) {
							ids[i] = _add_point(result.positions, vec3s(x, y, z), surf_func, i);
						}
					}
					mid03_cur(x + 1, y).mid3 = ids[5];
					mid03_cur(x, y + 1).mid0 = ids[6];
					mid8(x + 1, y + 1) = ids[10];
					const std::uint8_t *tri_list = _tri_table[occupation];
					for (std::size_t i = 0; tri_list[i] != _end; i += 3) {
						result.indices.emplace_back(ids[tri_list[i]]);
						result.indices.emplace_back(ids[tri_list[i + 1]]);
						result.indices.emplace_back(ids[tri_list[i + 2]]);
					}
				}
			}
			std::swap(mid03_cur, mid03_prev);
		}
		return result;
	}
}

#include "gtest/gtest.h"

#include "fastfall/util/grid_vector.hpp"


#include <numeric>

TEST(grid_vector, constructor) 
{
	{
		grid_vector<int> grid;
		EXPECT_EQ(grid.data(), nullptr);
		EXPECT_EQ(grid.column_size(), 0);
		EXPECT_EQ(grid.row_size(), 0);
	}
	{
		grid_vector grid(4, 6, 2);
		EXPECT_NE(grid.data(), nullptr);
		EXPECT_EQ(grid.column_size(), 4);
		EXPECT_EQ(grid.row_size(), 6);

		std::iota(grid.begin(), grid.end(), 1);

		std::vector<int> eq(4 * 6);
		std::iota(eq.begin(), eq.end(), 1);
		EXPECT_TRUE(std::equal(grid.begin(), grid.end(), eq.begin()));
	}
}

TEST(grid_vector, views)
{
	{
		grid_vector grid(4, 6, 2);
		std::iota(grid.begin(), grid.end(), 1);

		grid_view view = grid.take_view(1, 1, 2, 2);
		EXPECT_EQ(view.at(0, 0), grid.at(1, 1));
		EXPECT_EQ(view.at(1, 1), grid.at(2, 2));
		EXPECT_EQ(view.column_size(), 2);
		EXPECT_EQ(view.row_size(), 2);
	}
}
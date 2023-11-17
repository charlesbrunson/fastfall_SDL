#pragma once

#include <optional>
#include <tuple>
#include <cmath>
#include <iostream>
#include <algorithm>

#include "fastfall/util/math.hpp"

namespace ff {

    template<typename T>
    class line_thru_grid {
    public:
        using sentinel = Vec2i;

        struct iter_value {
            Vec2i pos;
            std::optional<std::pair<Vec2i, Vec2i>> corners;
        };

        class iterator {
        public:
            using value_type      = iter_value;
            using difference_type = Vec2<T>;
            using pointer         = value_type*;
            using reference       = value_type &;

            iterator(Line<T> line_t, Vec2i grid_di, Vec2<T> cell_di, Vec2i pos)
                : line(line_t), value{.pos = pos}, grid_size(grid_di), cell_size(cell_di)
            {
                m = (line.p2.y - line.p1.y) / (line.p2.x - line.p1.x);
                m_inv = (line.p2.x - line.p1.x) / (line.p2.y - line.p1.y);

                sign = Vec2i{
                    line.p1.x < line.p2.x ? 1 : -1,
                    line.p1.y < line.p2.y ? 1 : -1
                };

                scan = Vec2f{
                    (pos.x * cell_size.x) + (sign.x > 0 ? 1 : 0) * cell_size.x,
                    (pos.y * cell_size.y) + (sign.y > 0 ? 1 : 0) * cell_size.y
                };
            }

            iterator(Line<T> line_t, Vec2i grid_di, Vec2<T> cell_di)
            : line(line_t), value(), grid_size(grid_di), cell_size(cell_di)
            {
                m = (line.p2.y - line.p1.y) / (line.p2.x - line.p1.x);
                m_inv = (line.p2.x - line.p1.x) / (line.p2.y - line.p1.y);

                sign = Vec2i{
                    line.p1.x < line.p2.x ? 1 : -1,
                    line.p1.y < line.p2.y ? 1 : -1
                };

                auto right = grid_size.x * cell_size.x;
                auto bottom = grid_size.y * cell_size.y;

                if (sign.x > 0 && line.p1.x < 0) {
                    line.p1.y += (0 - line.p1.x) * m;
                    line.p1.x = 0;
                } else if (sign.x < 0 && line.p1.x > right) {
                    line.p1.y += (right - line.p1.x) * m;
                    line.p1.x = right;
                }

                if (sign.y > 0 && line.p1.y < 0) {
                    line.p1.x += (0 - line.p1.y) * m_inv;
                    line.p1.y = 0;
                } else if (sign.y < 0 && line.p1.y > bottom) {
                    line.p1.x += (bottom - line.p1.y) * m_inv;
                    line.p1.y = bottom;
                }

                scan = Vec2f{
                    (std::floor(line.p1.x / cell_size.x) * cell_size.x) + (sign.x > 0 ? 1 : 0) * cell_size.x,
                    (std::floor(line.p1.y / cell_size.y) * cell_size.y) + (sign.y > 0 ? 1 : 0) * cell_size.y
                };

                value.pos.x = line.p1.x / cell_size.x;
                value.pos.y = line.p1.y / cell_size.y;
            }


            bool operator!=(sentinel end) const {
                return in_grid() && past_sentinel(end);
            }

            iterator &operator++() {

                value.corners.reset();

                auto scan_dist = scan - line.p1;

                auto scanx_y = scan_dist.x * m;
                auto scany_x = scan_dist.y * m_inv;

                auto xstep_dist2 = std::abs(pow(scan_dist.x, 2) + pow(scanx_y, 2));
                auto ystep_dist2 = std::abs(pow(scany_x, 2) + pow(scan_dist.y, 2));

                if (xstep_dist2 == ystep_dist2) {
                    value.corners = {
                        { value.pos.x + sign.x, value.pos.y },
                        { value.pos.x,          value.pos.y + sign.y },
                    };

                    value.pos.x += sign.x;
                    value.pos.y += sign.y;
                    scan.x += sign.x * cell_size.x;
                    scan.y += sign.y * cell_size.y;
                } else if (std::isnan(ystep_dist2) || xstep_dist2 <= ystep_dist2) {
                    value.pos.x += sign.x;
                    scan.x += sign.x * cell_size.x;
                } else if (std::isnan(xstep_dist2) || xstep_dist2 >= ystep_dist2) {
                    value.pos.y += sign.y;
                    scan.y += sign.y * cell_size.y;
                }

                return *this;
            }

            const value_type& operator*() const {
                return value;
            }

            const value_type* operator->() const {
                return &value;
            }

        private:
            [[nodiscard]]
            inline bool in_grid() const {
                return value.pos.x >= 0 && value.pos.x < grid_size.x
                       && value.pos.y >= 0 && value.pos.y < grid_size.y;
            }

            [[nodiscard]]
            inline bool past_sentinel(sentinel end) const {
                return (value.pos.x != end.x + sign.x || value.pos.y != end.y + sign.y)
                       && (value.pos.x != end.x + sign.x || value.pos.y != end.y)
                       && (value.pos.x != end.x || value.pos.y != end.y + sign.y);
            }

            Line<T> line;
            iter_value value;

            Vec2i grid_size;
            Vec2<T> cell_size;

            float m;
            float m_inv;

            Vec2i sign;
            Vec2<T> scan;

        };

    private:
        Line<T> line;
        Vec2i grid_size;
        Vec2<T> cell_size;

    public:
        line_thru_grid() = default;
        line_thru_grid(Line<T> line_t, Vec2i grid_di, Vec2<T> cell_di)
        : line(line_t), grid_size(grid_di), cell_size(cell_di)
        {
        }

        iterator begin() const {
            return {
                line,
                grid_size,
                cell_size
            };
        }

        sentinel end() const {
            return Vec2i{ line.p2.x / cell_size.x, line.p2.y / cell_size.y };
        }
    };

}
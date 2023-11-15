#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/game/phys/collider_coretypes/ColliderQuad.hpp"

#include <assert.h>
#include <math.h>

namespace ff {


class ColliderRegion {
protected:
    virtual std::optional<QuadID> first_quad_in_rect(Rectf area, Recti& tile_area) const = 0;
    virtual std::optional<QuadID> next_quad_in_rect(Rectf area, QuadID quadid, const Recti& tile_area) const = 0;

    virtual std::optional<QuadID> first_quad_in_line(Linef line, Recti& tile_area) const = 0;
    virtual std::optional<QuadID> next_quad_in_line(Linef line, QuadID quadid, const Recti& tile_area) const = 0;

public:
    struct QuadAreaIterator {
    public:
        using value_type = const ColliderQuad;

        QuadAreaIterator(const ColliderRegion* t_region, Rectf t_area, std::optional<QuadID> t_quad, Recti t_tile_area)
            : region(t_region), area(t_area), curr_quad(t_quad), tile_area(t_tile_area)
        {};

        QuadAreaIterator(const QuadAreaIterator&) = default;
        QuadAreaIterator(QuadAreaIterator&&) = default;

        QuadAreaIterator& operator=(const QuadAreaIterator&) = default;
        QuadAreaIterator& operator=(QuadAreaIterator&&) = default;

        QuadAreaIterator& operator++();
        QuadAreaIterator operator++(int) { auto cpy = *this; ++(*this); return cpy; }

        const value_type* operator->() const;
        const value_type& operator* () const;

        bool operator==(const QuadAreaIterator& other) const noexcept { return region == other.region && curr_quad == other.curr_quad; };
        bool operator!=(const QuadAreaIterator& other) const noexcept { return region != other.region || curr_quad != other.curr_quad; };

        [[nodiscard]] Recti get_tile_area() const { return tile_area; }

    private:
        const ColliderRegion* region    = nullptr;
        Rectf area                      = {};
        std::optional<QuadID> curr_quad = {};
        Recti tile_area                 = {};
    };

    struct QuadArea {
        const ColliderRegion* region    = nullptr;
        Rectf area                      = {};

        [[nodiscard]] QuadAreaIterator begin() const;
        [[nodiscard]] QuadAreaIterator end() const;
    };

    struct QuadLineIterator {
    public:
        using value_type = const ColliderQuad;

        QuadLineIterator(const ColliderRegion* t_region, Linef t_line, std::optional<QuadID> t_quad, Recti t_tile_area)
                : region(t_region), line(t_line), curr_quad(t_quad), tile_area(t_tile_area)
        {};

        QuadLineIterator(const QuadLineIterator&) = default;
        QuadLineIterator(QuadLineIterator&&) = default;

        QuadLineIterator& operator=(const QuadLineIterator&) = default;
        QuadLineIterator& operator=(QuadLineIterator&&) = default;

        QuadLineIterator& operator++();
        QuadLineIterator operator++(int) { auto cpy = *this; ++(*this); return cpy; }

        const value_type* operator->() const;
        const value_type& operator* () const;

        bool operator==(const QuadLineIterator& other) const noexcept { return region == other.region && curr_quad == other.curr_quad; };
        bool operator!=(const QuadLineIterator& other) const noexcept { return region != other.region || curr_quad != other.curr_quad; };

        [[nodiscard]] Recti get_tile_area() const { return tile_area; }

    private:
        const ColliderRegion* region    = nullptr;
        Linef line                      = {};
        std::optional<QuadID> curr_quad = {};
        Recti tile_area                 = {};
    };

    struct QuadLine {
        const ColliderRegion* region    = nullptr;
        Linef line                      = {};

        [[nodiscard]] QuadLineIterator begin() const;
        [[nodiscard]] QuadLineIterator end() const;
    };

	explicit ColliderRegion(Vec2i initialPosition = Vec2i(0, 0));
	virtual ~ColliderRegion() = default;

    [[nodiscard]] QuadArea in_rect(Rectf area) const { return { this, area }; }
    [[nodiscard]] QuadLine in_line(Linef line) const { return { this, line }; }

	virtual void update(secs deltaTime) = 0;

	virtual const ColliderQuad* get_quad(QuadID quad_id) const noexcept = 0;

	[[nodiscard]] const ColliderSurface* get_surface_collider(ColliderSurfaceID id) const noexcept;
    [[nodiscard]] const ColliderSurface* get_surface_collider(std::optional<ColliderSurfaceID> id) const noexcept;
    [[nodiscard]] const SurfaceMaterial* get_surface_material(ColliderSurfaceID id) const noexcept;

    [[nodiscard]] Vec2f getPrevPosition() const noexcept;
    [[nodiscard]] Vec2f getPosition() const noexcept;

    [[nodiscard]] bool hasMoved() const noexcept;
    [[nodiscard]] Vec2f getDeltaPosition() const noexcept;

	void teleport(Vec2f pos);
	void setPosition(Vec2f pos, bool updatePrev = true);

    [[nodiscard]] Rectf getBoundingBox() const noexcept;
    [[nodiscard]] Rectf getSweptBoundingBox() const noexcept;

	Vec2f velocity;
	Vec2f delta_velocity;

	virtual bool on_precontact(World& w, const ContinuousContact& contact, secs duration) const { return true; };
	virtual void on_postcontact(World& w, const AppliedContact& contact, secs deltaTime) const {};

protected:
	Rectf boundingBox;
	Rectf prevBoundingBox;

private:
	Vec2f prevPosition;
	Vec2f position;
};

void imgui_component(World&w , ID<ColliderRegion> id);

}

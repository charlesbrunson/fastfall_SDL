#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/game/phys/collider_coretypes/ColliderQuad.hpp"

#include <assert.h>
#include <math.h>

namespace ff {

struct touching_surface_t {
    ColliderSurfaceID id;
    Linef surface;
    Vec2f intersect;
};

class ColliderRegion {
protected:
    virtual std::optional<QuadID> first_quad_in_rect(Rectf area) const = 0;
    virtual std::optional<QuadID> next_quad_in_rect(Rectf area, QuadID quadid) const = 0;

public:
    struct QuadIterator {
    public:
        using value_type = const ColliderQuad;


        QuadIterator(const ColliderRegion* t_region, Rectf t_area, std::optional<QuadID> t_quad = {})
            : region(t_region), area(t_area), curr_quad(t_quad)
        {};

        QuadIterator(const QuadIterator&) = default;
        QuadIterator(QuadIterator&&) = default;

        QuadIterator& operator=(const QuadIterator&) = default;
        QuadIterator& operator=(QuadIterator&&) = default;

        QuadIterator& operator++();
        QuadIterator operator++(int) {
            auto cpy = *this; ++(*this); return cpy;
        }

        const value_type* operator->() const;
        const value_type& operator* () const;

        bool operator==(const QuadIterator& other) const noexcept { return region == other.region && curr_quad == other.curr_quad; };
        bool operator!=(const QuadIterator& other) const noexcept { return region != other.region || curr_quad != other.curr_quad; };

    private:
        const ColliderRegion* region    = nullptr;
        Rectf area                      = {};
        std::optional<QuadID> curr_quad = {};
    };

    struct QuadArea {
        const ColliderRegion* region    = nullptr;
        Rectf area                      = {};

        QuadIterator begin() const;
        QuadIterator end() const;
    };

	explicit ColliderRegion(Vec2i initialPosition = Vec2i(0, 0));
	virtual ~ColliderRegion() = default;

    QuadArea in_rect(Rectf area) {
        return QuadArea{ this, area };
    }

    //template<typename QuadPtrFunction>
    //requires std::is_invocable_v<QuadPtrFunction, const ColliderQuad&>
    //void for_quads_in_rect(Rectf area, QuadPtrFunction&& f) {
    //    for (auto& quad : in_rect(area))
    //    {
    //        f(quad);
    //    }
    //}

    template<typename OutputIt>
    void get_intersecting_surfaces(Linef surface, OutputIt iter) {
        Rectf bounds = math::line_bounds(surface);
        for (auto& quad : in_rect(bounds)) {
            for (auto dir: direction::cardinals) {
                ColliderSurfaceID id{
                    .quad_id = quad.getID(),
                    .dir     = dir
                };

                if (auto surf = quad.getSurface(dir)) {
                    Linef msurf = math::shift(surf->surface, getPosition());
                    Vec2f inter = math::intersection(msurf, surface);
                    bool in_bounds = inter != Vec2f{NAN, NAN} && bounds.contains(inter);
                    if (in_bounds) {
                        *iter = touching_surface_t{
                            .id        = id,
                            .surface   = msurf,
                            .intersect = inter,
                        };
                        ++iter;
                    }
                }
            }
        }
    }

	virtual void update(secs deltaTime) = 0;

	virtual const ColliderQuad* get_quad(QuadID quad_id) const noexcept = 0;

	const ColliderSurface* get_surface_collider(ColliderSurfaceID id) const noexcept;
    const ColliderSurface* get_surface_collider(std::optional<ColliderSurfaceID> id) const noexcept;
    const SurfaceMaterial* get_surface_material(ColliderSurfaceID id) const noexcept;

	Vec2f getPrevPosition() const noexcept;
	Vec2f getPosition() const noexcept;

	bool hasMoved() const noexcept;
	Vec2f getDeltaPosition() const noexcept;

	void teleport(Vec2f pos);
	void setPosition(Vec2f pos, bool updatePrev = true);

	Rectf getBoundingBox() const noexcept;
	Rectf getSweptBoundingBox() const noexcept;

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

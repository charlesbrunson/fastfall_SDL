#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/util/log.hpp"

#include "fastfall/engine/time/time.hpp"

#include "fastfall/game/phys2/fixed.hpp"

namespace ff
{

    struct PhysBox
    {

        /* Bounding boxes & position */
        struct PhysBoxState
        {
            Rectfx rect;
            Vec2fx origin;
            Vec2fx position;

            Vec2fx position() const { return rect.topleft() + origin; }
        };

        PhysBoxState m_curr;
        PhysBoxState m_prev;

        // combine m_curr.rect and m_prev.rect into one bounding box
        Rectfx bounding_box() const;


        void move_to_position(Vec2fx pos) noexcept;
        void teleport_to_position(Vec2fx pos) noexcept;

        /* Our velocities */
        Vec2fx m_precollision_vel; // velocity before collision checks
        Vec2fx m_local_vel;        // velocity before and after collision checks

        Vec2fx get_global_vel() const { return m_local_vel + m_parent_vel + m_surface_vel; }

        // alter parent vel without changing the global vel
        void apply_parent_vel(Vec2fx _vel);
        void reset_parent_vel();

        // alter surface vel without changing the global vel
        void apply_surface_vel(Vec2fx _vel);
        void reset_surface_vel();

        /* Velocities of collider we're on */
        Vec2fx m_parent_vel;  // velocity of the whole collider
        Vec2fx m_surface_vel; // tangent velocity of surface
        Vec2fx m_neutral_vel; // global velocity considered "stopped"

        /* Our accelerations */
        Vec2fx m_gravity;
        Vec2fx m_accel;

        /* accumulated accel values applied to m_accel during update() */
        Vec2fx m_accumulated_accel;
        Vec2fx m_accumulated_decel;

        // process
        void update(secs deltaTime);







    };
}
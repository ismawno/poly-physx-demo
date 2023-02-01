#ifndef ATTACHER_HPP
#define ATTACHER_HPP

#include "entity2D_ptr.hpp"
#include "rigid_bar2D.hpp"
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    class attacher
    {
    public:
        enum attach_type
        {
            SPRING,
            RIGID_BAR
        };

        attacher() = default;

        void update(bool snap_e2_to_center);
        void render(bool snap_e2_to_center);

        void try_attach_first(bool snap_e1_to_center);
        void try_attach_second(bool snap_e2_to_center);

        void cancel();

        const attach_type &type() const;
        void type(const attach_type &type);

        float sp_stiffness() const;
        float sp_dampening() const;
        float sp_length() const;
        float ctr_stiffness() const;
        float ctr_dampening() const;

        void sp_stiffness(float sp_stiffness);
        void sp_dampening(float sp_dampening);
        void sp_length(float sp_length);
        void ctr_stiffness(float ctr_stiffness);
        void ctr_dampening(float ctr_dampening);

        bool has_first() const;

        bool auto_length() const;
        void auto_length(bool auto_length);

    private:
        phys::entity2D_ptr m_e1;
        alg::vec2 m_joint1;
        float m_last_angle,
            m_sp_stiffness = 1.f,
            m_sp_dampening = 0.f,
            m_sp_length = 0.f,
            m_ctr_stiffness = 500.f,
            m_ctr_dampening = 30.f;
        bool m_auto_length = false, m_snap_e1_to_center;
        attach_type m_attach_type = SPRING;

        void rotate_joint();
        void draw_unattached_joint(bool snap_e2_to_center);
    };
}

#endif
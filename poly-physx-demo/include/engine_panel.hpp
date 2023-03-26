#ifndef ENGINE_PANEL_HPP
#define ENGINE_PANEL_HPP

#include "quad_tree2D.hpp"
#include "layer.hpp"

namespace phys_demo
{
    class engine_panel : public phys::layer, public ini::saveable
    {
    public:
        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        bool p_enabled = true;

    private:
        enum integ_method
        {
            RK1 = 0,
            RK2 = 1,
            RK4 = 2,
            RK38 = 3,
            RKF12 = 4,
            RKF45 = 5,
            RKFCK45 = 6,
            RKF78 = 7
        };

        void on_render() override;

        integ_method m_method = RK4;
        bool m_visualize_qt = false;

        void render_integration();
        void render_sliders();
        void render_methods_list();
        void update_method();

        void render_collision();
        void render_collision_params() const;
        void render_coldet_list();
        void render_quad_tree_params();
        void render_path_prediction_settings() const;
        void render_trail_settings() const;

        void draw_quad_tree(const phys::quad_tree2D &qt);
    };
}

#endif
#ifndef DEMO_APP_HPP
#define DEMO_APP_HPP

#include "app.hpp"
#include "selector.hpp"
#include "actions_panel.hpp"
#include "engine_panel.hpp"
#include "perf_panel.hpp"
#include "phys_panel.hpp"
#include "adder.hpp"
#include "attacher.hpp"
#include "outline_manager.hpp"
#include "copy_paste.hpp"
#include "imgui.h"
#include "imgui-SFML.h"

namespace phys_demo
{
    class demo_app : public phys::app
    {
    public:
        demo_app(const demo_app &papp) = delete;

        static demo_app &get();

    private:
        demo_app();

        void on_update() override;
        void on_late_update() override;
        void on_render() override;
        void on_entity_draw(const phys::entity2D_ptr &e, sf::ConvexShape &shape) override;
        void on_event(sf::Event &event) override;

        grabber m_grabber = grabber(engine());
        selector m_selector = selector(engine());
        adder m_adder;
        attacher m_attacher;
        outline_manager m_outline_manager = outline_manager(engine());
        copy_paste m_copy_paste = copy_paste(m_selector);

        actions_panel m_actions_panel = actions_panel(m_grabber, m_selector, m_attacher, m_outline_manager);
        engine_panel m_engine_panel;
        perf_panel m_perf_panel;
        phys_panel m_phys_panel = phys_panel(m_selector, m_outline_manager);

        sf::Clock m_clock;

        void draw_interaction_lines();
    };
}

#endif
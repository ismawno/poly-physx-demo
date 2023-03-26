#ifndef ACTIONS_PANEL_HPP
#define ACTIONS_PANEL_HPP

#include "grab_tab.hpp"
#include "entities_tab.hpp"
#include "attach_tab.hpp"
#include "add_tab.hpp"
#include "app.hpp"

namespace phys_demo
{
    class actions_panel : public phys::layer, public ini::saveable
    {
    public:
        enum actions
        {
            ADD,
            GRAB,
            ATTACH,
            ENTITIES,
            NONE
        };
        actions_panel();
        actions action() const;

        bool p_enabled = true;
        void write(ini::output &out) const override;
        void read(ini::input &in) override;

    private:
        void on_render() override;

        add_tab m_add_tab;
        grab_tab m_grab_tab;
        attach_tab m_attach_tab;
        entities_tab m_entities_tab;
        actions m_action = ADD;

        void render_tabs();
    };
}

#endif
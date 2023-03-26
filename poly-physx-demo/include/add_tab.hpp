#ifndef ADD_TAB_HPP
#define ADD_TAB_HPP

#include "templates.hpp"

namespace phys_demo
{
    class add_tab
    {
    public:
        add_tab() = default;
        void render() const;

    private:
        void render_menu_bar() const;
        void render_shape_list() const;
        void render_entity_inputs() const;
        void render_color_picker() const;
        void render_canvas() const;
    };
}

#endif
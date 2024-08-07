#pragma once

#include "ppx-app/app/app.hpp"
#include "ppx-demo/2D/utility/selection_manager.hpp"
#include "ppx-demo/2D/utility/group_manager.hpp"

namespace ppx::demo
{
class actions_panel;
class engine_panel;
class performance_panel;
class physics_panel;
class scenarios_panel;
class menu_bar;

class demo_app final : public app
{
  public:
    demo_app();

    struct style_settings
    {
        lynx::color *collider_color = &app::collider_color;
        lynx::color *joint_color = &app::joint_color;
        lynx::color body_selection_color;
        lynx::color collider_selection_color;
        lynx::color quad_tree_color;
        lynx::color contact_color;
        lynx::color normal_color;
        lynx::color ray_color;
        lynx::color selection_outline_color;
        std::vector<lynx::color> island_colors;
    } style;

    selection_manager selector;
    group_manager grouper;

    actions_panel *actions;
    engine_panel *engine;
    performance_panel *performance;
    physics_panel *physics;
    scenarios_panel *scenarios;
    menu_bar *menu;

    void add_walls();
    style_settings parse_config_file();
    void reload_config_file();

    std::size_t thread_count() const;
    void thread_count(std::size_t count);

  private:
    void on_late_start() override;
    void on_late_shutdown() override;

    void on_update(float ts) override;
    void on_render(float ts) override;
    bool on_event(const lynx::event2D &event) override;

    void remove_selected();

    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;

    kit::scope<kit::mt::thread_pool> m_thread_pool;
    std::size_t m_thread_count;
};
} // namespace ppx::demo

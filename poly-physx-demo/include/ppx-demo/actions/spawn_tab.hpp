#ifndef PPX_SPAWN_TAB_HPP
#define PPX_SPAWN_TAB_HPP

#include "lynx/drawing/shape.hpp"
#include "ppx/body2D.hpp"

namespace ppx::demo
{
class demo_app;
class spawn_tab
{
  public:
    spawn_tab() = default;
    spawn_tab(demo_app *app);

    void update();
    void render();
    void render_tab();

    void begin_body_spawn();
    void end_body_spawn();
    void cancel_body_spawn();

    YAML::Node encode() const;
    void decode(const YAML::Node &node);

  private:
    enum class shape_type
    {
        RECT,
        CIRCLE,
        NGON,
        CUSTOM
    };
    struct body_template
    {
        body2D::specs specs;
        std::string name;
        float width = 5.f, height = 5.f, ngon_radius = 3.f;
        int ngon_sides = 3;
        shape_type type = shape_type::RECT;
        lynx::color color;
    };

    demo_app *m_app;
    lynx::window2D *m_window;
    kit::scope<lynx::shape2D> m_preview;

    glm::vec2 m_starting_mouse_pos{0.f};
    float m_speed_spawn_multiplier = 0.6f;
    body_template m_current_body_template{};
    bool m_previewing = false;

    std::unordered_map<std::string, body_template> m_templates;

    bool is_current_template_registered() const;

    void render_save_template_prompt();
    void render_load_template_and_removal_prompts();

    void render_body_shape_types_and_properties();
    void render_menu_bar();

    static YAML::Node encode_template(const body_template &btemplate);
    static body_template decode_template(const YAML::Node &node);
};
} // namespace ppx::demo

#endif
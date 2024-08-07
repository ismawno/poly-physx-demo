#pragma once

#include "ppx-demo/2D/app/demo_layer.hpp"

#include "ppx-demo/2D/engine/integration_tab.hpp"
#include "ppx-demo/2D/engine/collision_tab.hpp"
#include "ppx-demo/2D/engine/islands_tab.hpp"
#include "ppx-demo/2D/engine/constraints_tab.hpp"

#include "lynx/app/window.hpp"

namespace ppx::demo
{
class engine_panel final : public demo_layer
{
  public:
    engine_panel();

    integration_tab integration;
    collision_tab collision;
    constraints_tab constraints;
    islands_tab islands;

  private:
    lynx::window2D *m_window;

    bool m_casting = false;
    thick_line2D m_ray_line;
    thick_line2D m_ray_normal_line;
    glm::vec2 m_origin{0.f};

    void on_attach() override;
    void on_render(float ts) override;
    void on_update(float ts) override;

    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;
};
} // namespace ppx::demo

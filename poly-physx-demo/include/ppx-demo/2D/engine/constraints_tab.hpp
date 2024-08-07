#pragma once

#include "lynx/app/window.hpp"
#include "ppx-demo/2D/actions/entities_tab.hpp"

namespace ppx::demo
{
class demo_app;
class constraints_tab
{
  public:
    constraints_tab() = default;
    constraints_tab(demo_app *app);

    void render_imgui_tab();

  private:
    demo_app *m_app;
    lynx::window2D *m_window;
};
} // namespace ppx::demo
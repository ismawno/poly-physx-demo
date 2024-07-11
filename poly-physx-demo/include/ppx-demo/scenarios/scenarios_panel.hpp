#pragma once

#include "ppx-demo/app/demo_layer.hpp"
#include "ppx-demo/scenarios/scenario.hpp"
#include "kit/memory/ptr/scope.hpp"

namespace ppx::demo
{
class scenarios_panel final : public demo_layer
{
    enum class scenario_type
    {
        TUMBLER,
        TUMBLER_PERF
    };

  public:
    scenarios_panel();

    const scenario *current_scenario() const;

  private:
    void on_attach() override;
    void on_update(float ts) override;
    void on_render(float ts) override;

    void render_dropdown_and_scenario_info();
    kit::scope<scenario> create_copied_scenario_from_current_type();
    kit::scope<scenario> create_scenario_from_type(scenario_type sctype);
    void update_scenario();

    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;

    kit::scope<scenario> m_current_scenario = nullptr;
    std::vector<std::pair<scenario_type, kit::scope<scenario>>> m_queued_scenarios;
    scenario_type m_sctype = scenario_type::TUMBLER;
    bool m_was_running = false;
    bool m_auto_stop = true;
};
} // namespace ppx::demo
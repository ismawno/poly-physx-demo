#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/2D/scenarios/tumbler.hpp"
#include "ppx-demo/2D/actions/actions_panel.hpp"
#include "ppx-demo/2D/app/demo_app.hpp"
#include "ppx/serialization/serialization.hpp"

namespace ppx::demo
{
void tumbler::start()
{
    scenario::start();
    const auto chamber = create_chamber();
    m_body_props = from_actions_panel();

    const float cdx = 1.5f * m_width;
    const float cxstart = -0.5f * cdx * (m_tumblers - 1);

    body2D::specs chamber_specs{};
    chamber_specs.props = chamber;
    chamber_specs.angular_velocity = m_angular_velocity;

    body2D::specs body_specs{};
    body_specs.props = m_body_props;
    body_specs.velocity = m_init_vel;

    for (std::uint32_t i = 0; i < m_tumblers; i++)
    {
        const float cx = cxstart + i * cdx;
        chamber_specs.position = {cx, 0.f};
        m_app->world.bodies.add(chamber_specs);

        const float sdx = 15.f;
        const float sxstart = cx - 0.5f * sdx * (m_spawn_points - 1);
        for (std::uint32_t j = 0; j < m_spawn_points; j++)
        {
            const float sx = sxstart + j * sdx;
            body_specs.position = {sx, m_spawn_height};
            m_body_specs.push_back(body_specs);
        }
    }
}

void tumbler::update(const float ts)
{
    if (m_body_count >= m_total_spawns || m_app->paused)
        return;

    m_addition_timer += ts; // so that at lower framerates bodies do not overlap
    if (m_addition_timer < m_addition_wait_time)
        return;
    m_body_count += m_body_specs.size();
    m_addition_timer = 0.f;

    for (const auto &specs : m_body_specs)
        m_app->world.bodies.add(specs);
    if (m_body_count > m_total_spawns)
        m_body_count = m_total_spawns;
}

void tumbler::cleanup()
{
    m_app->world.bodies.clear();
    m_body_specs.clear();
    m_body_count = 0;
    m_body_specs.clear();
}

const char *tumbler::name() const
{
    return "tumbler";
}

std::string tumbler::format() const
{
    return std::format("tumblers-{}-spawns-{}-angvel-{:.1f}-w-{:.0f}-h-{:.0f}", m_tumblers, m_total_spawns,
                       m_angular_velocity, m_width, m_height);
}

bool tumbler::expired() const
{
    return m_body_count >= m_total_spawns;
}

void tumbler::on_imgui_window_render()
{
    if (m_stopped)
    {
        ImGui::DragInt("Total bodies", (int *)&m_total_spawns, 1.f, 0, INT32_MAX);
        ImGui::SliderInt("Tumblers", (int *)&m_tumblers, 1, 24);
        ImGui::SliderInt("Spawn points", (int *)&m_spawn_points, 1, 32);
        ImGui::SliderFloat("Spawn height", &m_spawn_height, 0.f, 0.4f * m_height, "%.1f");
        ImGui::DragFloat("Addition wait time", &m_addition_wait_time, 0.01f, 0.f, 1.f, "%.2f");
        ImGui::DragFloat("Angular speed", &m_angular_velocity, 0.01f, 0.f, FLT_MAX, "%.3f");
        ImGui::DragFloat("Width", &m_width, 1.f, 0.f, FLT_MAX, "%.1f");
        ImGui::DragFloat("Height", &m_height, 1.f, 0.f, FLT_MAX, "%.1f");
    }
    else
    {
        const float time_remaining =
            (m_total_spawns - m_body_count) * m_addition_wait_time / (m_spawn_points * m_tumblers);
        if (time_remaining < 180.f)
            ImGui::Text("Tumbler scenario progress (%.1f seconds)", time_remaining);
        else
            ImGui::Text("Tumbler scenario progress (%.2f minutes)", time_remaining / 60.f);
        ImGui::ProgressBar((float)m_body_count / m_total_spawns, ImVec2(0.f, 0.f));
    }
}

body2D::specs::properties tumbler::create_chamber()
{
    const float thck = 20.f;
    collider2D::specs rect1{};
    rect1.props.shape = collider2D::stype::POLYGON;
    rect1.props.vertices = polygon::rect(m_width, thck);
    rect1.position = {0.f, 0.5f * m_height};

    collider2D::specs rect2 = rect1;
    rect2.position = {0.f, -0.5f * m_height};

    collider2D::specs rect3 = rect1;
    rect3.props.vertices = polygon::rect(thck, m_height);
    rect3.position = {0.5f * m_width, 0.f};

    collider2D::specs rect4 = rect3;
    rect4.position = {-0.5f * m_width, 0.f};

    body2D::specs::properties chamber{};
    chamber.type = body2D::btype::KINEMATIC;
    chamber.colliders.insert(chamber.colliders.end(), {rect1, rect2, rect3, rect4});
    return chamber;
}

body2D::specs::properties tumbler::from_actions_panel()
{
    const auto &proxy = m_app->actions->bodies.current_proxy();
    body2D::specs::properties props = proxy.specs.props;
    for (const auto &cproxy : proxy.cproxies)
        props.colliders.push_back(cproxy.specs);
    return props;
}

YAML::Node tumbler::encode() const
{
    YAML::Node node;
    node["Name"] = "Tumbler";
    node["Tumblers"] = m_tumblers;
    node["Spawn points"] = m_spawn_points;
    node["Angular velocity"] = m_angular_velocity;
    node["Width"] = m_width;
    node["Height"] = m_height;
    node["Addition wait time"] = m_addition_wait_time;
    node["Total spawns"] = m_total_spawns;
    node["Body properties"] = m_body_props;
    node["Spawn height"] = m_spawn_height;
    return node;
}

bool tumbler::decode(const YAML::Node &node)
{
    m_tumblers = node["Tumblers"].as<std::uint32_t>();
    m_spawn_points = node["Spawn points"].as<std::uint32_t>();
    m_angular_velocity = node["Angular velocity"].as<float>();
    m_width = node["Width"].as<float>();
    m_height = node["Height"].as<float>();
    m_addition_wait_time = node["Addition wait time"].as<float>();
    m_total_spawns = node["Total spawns"].as<std::uint32_t>();
    m_body_props = node["Body properties"].as<body2D::specs::properties>();
    m_spawn_height = node["Spawn height"].as<float>();
    return true;
}

} // namespace ppx::demo
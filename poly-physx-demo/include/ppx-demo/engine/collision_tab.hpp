#pragma once

#include "lynx/app/window.hpp"

#include "ppx/collision/detection/brute_force_detection2D.hpp"
#include "ppx/collision/detection/quad_tree_detection2D.hpp"
#include "ppx/collision/detection/sort_sweep_detection2D.hpp"
#include "ppx/collision/contacts/contact_solver2D.hpp"
#include "ppx/collision/contacts/nonpen_contact2D.hpp"
#include "ppx/collision/contacts/spring_contact2D.hpp"

#include "lynx/drawing/line.hpp"
#include "ppx-app/drawables/lines/thick_line2D.hpp"

namespace ppx::demo
{
class demo_app;
class collision_tab
{
  public:
    collision_tab() = default;
    collision_tab(demo_app *app);

    void render_imgui_tab();
    void update();

    YAML::Node encode() const;
    void decode(const YAML::Node &node);

  private:
    demo_app *m_app;
    lynx::window2D *m_window;

    bool m_draw_bounding_boxes = false;
    bool m_visualize_qtree = false;

    std::vector<lynx::line_strip2D> m_bbox_lines;
    std::vector<lynx::line_strip2D> m_qt_lines;
    std::size_t m_qt_active_partitions = 0;

    bool m_draw_contacts = false;

    struct collision_repr
    {
        lynx::ellipse2D point;
        thick_line2D normal;
    };
    using contact_key = collision_contacts2D::contact_key;
    using nonpen_cmap = contact_manager2D<nonpen_contact2D>::contact_map;
    using spring_cmap = contact_manager2D<spring_contact2D>::contact_map;

    std::unordered_map<contact_key, collision_repr> m_contact_lines;

    void render_quad_tree_parameters(quad_tree_detection2D &qtdet);
    void render_nonpen_contact_solver_parameters();
    void render_spring_contact_solver_parameters();

    void update_bounding_boxes();
    void update_quad_tree_lines(const quad_tree::node &qtnode);

    template <typename CMap> void update_contact_lines(const CMap &contacts)
    {
        for (const auto &[hash, cnt] : contacts)
        {
            auto repr = m_contact_lines.find(hash);
            const auto &point = cnt->point();
            if (repr == m_contact_lines.end())
            {
                collision_repr new_repr;
                new_repr.point = lynx::ellipse2D{.3f, lynx::color::green};
                new_repr.point.transform.position = point.point;
                new_repr.normal = thick_line2D{lynx::color::magenta, .1f};

                repr = m_contact_lines.emplace(hash, new_repr).first;
            }

            const float length = 0.5f * std::clamp(100.f * glm::abs(point.penetration), 0.5f, 1.2f);
            const glm::vec2 dir = length * cnt->normal();

            repr->second.point.transform.position = point.point;
            repr->second.normal.p1(point.point);
            repr->second.normal.p2(point.point + dir);
            repr->second.point.color(cnt->enabled ? lynx::color::green : lynx::color::green * 0.6f);
            repr->second.normal.color(cnt->enabled ? lynx::color::magenta : lynx::color::magenta * 0.6f);
        }
    }

    template <typename CMap> void render_contact_lines(const CMap &contacts)
    {
        for (auto it = m_contact_lines.begin(); it != m_contact_lines.end();)
        {
            if (!contacts.contains(it->first))
            {
                it = m_contact_lines.erase(it);
                continue;
            }
            const auto &repr = it->second;
            m_window->draw(repr.point);
            m_window->draw(repr.normal);
            ++it;
        }
    }

    void render_collision_detection_list() const;
    void render_collisions_and_contacts_list() const;
    void render_contact_solvers_list() const;

    void render_cp_narrow_list() const;
    void render_pp_narrow_list() const;

    void render_pp_manifold_list() const;

    void render_bounding_boxes() const;

    void render_quad_tree_lines() const;
};
} // namespace ppx::demo
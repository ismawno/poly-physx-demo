#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/engine/collision_tab.hpp"
#include "ppx-demo/app/demo_app.hpp"

#include "ppx/collision/detection/narrow/gjk_epa_detection2D.hpp"
#include "ppx/collision/contacts/nonpen_contact2D.hpp"
#include "ppx/collision/contacts/spring_contact2D.hpp"

#include "ppx/collision/manifold/clipping_algorithm_manifold2D.hpp"
#include "ppx/collision/manifold/mtv_support_manifold2D.hpp"

namespace ppx::demo
{
collision_tab::collision_tab(demo_app *app) : m_app(app)
{
    m_window = m_app->window();
}

void collision_tab::update()
{
    if (m_draw_bounding_boxes)
        update_bounding_boxes();
    if (m_draw_contacts)
    {
        if (auto nonpen = m_app->world.collisions.contacts<contact_solver2D<nonpen_contact2D>>())
            update_contact_lines(nonpen->contacts());
        else if (auto spring = m_app->world.collisions.contacts<contact_solver2D<spring_contact2D>>())
            update_contact_lines(spring->contacts());
    }

    auto qtdet = m_app->world.collisions.detection<quad_tree_detection2D>();
    if (m_visualize_qtree && qtdet)
    {
        m_qt_active_partitions = 0;
        update_quad_tree_lines(qtdet->quad_tree().root());
    }
}

void collision_tab::render_imgui_tab()
{
    ImGui::Checkbox("Enabled", &m_app->world.collisions.enabled);

    if (m_draw_bounding_boxes)
        render_bounding_boxes();
    if (m_draw_contacts)
    {
        if (auto nonpen = m_app->world.collisions.contacts<contact_solver2D<nonpen_contact2D>>())
            render_contact_lines(nonpen->contacts());
        else if (auto spring = m_app->world.collisions.contacts<contact_solver2D<spring_contact2D>>())
            render_contact_lines(spring->contacts());
    }

    ImGui::Checkbox("Draw bounding boxes", &m_draw_bounding_boxes);
    ImGui::Checkbox("Draw contacts", &m_draw_contacts);
    ImGui::Text("Collision count: %zu", m_app->world.collisions.size());
    render_collisions_list();

    if (ImGui::CollapsingHeader("Detection"))
    {
        ImGui::Checkbox("Enabled", &m_app->world.collisions.detection()->enabled);
        render_collision_detection_list();
        if (ImGui::TreeNode("Narrow detection"))
        {
            render_cp_narrow_list();
            render_pp_narrow_list();
            ImGui::TreePop();
        }

#ifndef KIT_PROFILE
        ImGui::Checkbox("Multithreading", &m_app->world.collisions.detection()->multithreaded);
#endif

        if (auto qtdet = m_app->world.collisions.detection<quad_tree_detection2D>())
            render_quad_tree_parameters(*qtdet);
    }

    render_pp_manifold_list();
    if (ImGui::CollapsingHeader("Contacts"))
    {
        ImGui::Checkbox("Enable", &m_app->world.collisions.contacts()->enabled);
        ImGui::SliderInt("Contact lifetime", (int *)&m_app->world.collisions.contacts()->contact_lifetime, 1, 100);
        render_contact_solvers_list();

        if (m_app->world.collisions.contacts<contact_solver2D<nonpen_contact2D>>())
            render_nonpen_contact_solver_parameters();
        else if (m_app->world.collisions.contacts<contact_solver2D<spring_contact2D>>())
            render_spring_contact_solver_parameters();
    }

    // static bool callbacks = false;
    // static std::unordered_map<collider2D *, lynx::color> original_colors;
    // const auto on_enter = [this](contact2D *contact) {
    //     if (!original_colors.contains(contact->collider1()))
    //     {
    //         const lynx::color &original1 = m_app->shapes().at(contact->collider1())->color();
    //         original_colors[contact->collider1()] = original1;
    //         m_app->shapes().at(contact->collider1())->color(lynx::color{glm::vec3{original1.normalized} * 1.2f});
    //     }
    //     if (!original_colors.contains(contact->collider2()))
    //     {
    //         const lynx::color &original2 = m_app->shapes().at(contact->collider2())->color();
    //         original_colors[contact->collider2()] = original2;
    //         m_app->shapes().at(contact->collider2())->color(lynx::color{glm::vec3{original2.normalized} * 1.2f});
    //     }
    // };
    // const auto on_exit = [this](contact2D &contact) {
    //     if (original_colors.contains(contact.collider1()) && contact.collider1()->contacts().size() == 1)
    //     {
    //         m_app->shapes().at(contact.collider1())->color(original_colors.at(contact.collider1()));
    //         original_colors.erase(contact.collider1());
    //     }
    //     if (original_colors.contains(contact.collider2()) && contact.collider2()->contacts().size() == 1)
    //     {
    //         m_app->shapes().at(contact.collider2())->color(original_colors.at(contact.collider2()));
    //         original_colors.erase(contact.collider2());
    //     }
    // };
    // if (ImGui::Checkbox("Callbacks", &callbacks))
    // {
    //     if (callbacks)
    //     {
    //         m_app->world.collisions.events.on_contact_enter += on_enter;
    //         m_app->world.collisions.events.on_contact_exit += on_exit;
    //     }
    //     else
    //     {
    //         m_app->world.collisions.events.on_contact_enter -= on_enter;
    //         m_app->world.collisions.events.on_contact_exit -= on_exit;
    //     }
    // }
}

void collision_tab::render_collisions_list() const
{
    if (ImGui::TreeNode("Collisions"))
    {
        for (const auto &[hash, col] : m_app->world.collisions)
            if (ImGui::TreeNode(&col, "%s - %s", kit::uuid::name_from_ptr(col.collider1).c_str(),
                                kit::uuid::name_from_ptr(col.collider2).c_str()))
            {
                ImGui::Text("Contact points: %zu", col.manifold.size());
                ImGui::Text("Normal - x: %.5f, y: %.5f", col.mtv.x, col.mtv.y);
                ImGui::Spacing();

                for (std::size_t i = 0; i < col.manifold.size(); i++)
                {
                    const glm::vec2 &point = col.manifold[i].point;
                    ImGui::Text("Contact ID: %u - x: %.5f, y: %.5f", col.manifold[i].id.key, point.x, point.y);
                }
                ImGui::TreePop();
            }
        ImGui::TreePop();
    }
}

void collision_tab::render_collision_detection_list() const
{
    int det_method;
    if (m_app->world.collisions.detection<brute_force_detection2D>())
        det_method = 0;
    else if (m_app->world.collisions.detection<quad_tree_detection2D>())
        det_method = 1;
    else if (m_app->world.collisions.detection<sort_sweep_detection2D>())
        det_method = 2;

    if (ImGui::Combo("Collision detection", &det_method, "Brute force\0Quad tree\0Sort and sweep\0\0"))
    {
        if (det_method == 0)
            m_app->world.collisions.set_detection<brute_force_detection2D>();
        else if (det_method == 1)
            m_app->world.collisions.set_detection<quad_tree_detection2D>();
        else if (det_method == 2)
            m_app->world.collisions.set_detection<sort_sweep_detection2D>();
    }
}

void collision_tab::render_contact_solvers_list() const
{
    int csolver_method;
    if (m_app->world.collisions.contacts<contact_solver2D<nonpen_contact2D>>())
        csolver_method = 0;
    else if (m_app->world.collisions.contacts<contact_solver2D<spring_contact2D>>())
        csolver_method = 1;
    if (ImGui::Combo("Collision contacts", &csolver_method, "Non-penetration contact\0Spring contact\0\0"))
    {
        if (csolver_method == 0)
            m_app->world.collisions.set_contact_solver<contact_solver2D<nonpen_contact2D>>();
        else if (csolver_method == 1)
            m_app->world.collisions.set_contact_solver<contact_solver2D<spring_contact2D>>();
    }
}

void collision_tab::render_cp_narrow_list() const
{
    int alg;
    gjk_epa_detection2D *gjk;
    if ((gjk = m_app->world.collisions.detection()->cp_narrow_detection<gjk_epa_detection2D>()))
        alg = 0;
    if (ImGui::Combo("C-P Narrow algorithm", &alg, "GJK-EPA\0\0"))
    {
        if (alg == 0)
            m_app->world.collisions.detection()->set_cp_narrow_detection<gjk_epa_detection2D>();
    }
    if (gjk)
        ImGui::SliderFloat("C-P EPA Threshold", &gjk->epa_threshold, 1.e-4f, 1.e-1f, "%.4f",
                           ImGuiSliderFlags_Logarithmic);
}

void collision_tab::render_pp_narrow_list() const
{
    int alg;
    gjk_epa_detection2D *gjk;
    if ((gjk = m_app->world.collisions.detection()->pp_narrow_detection<gjk_epa_detection2D>()))
        alg = 0;
    if (ImGui::Combo("P-P Narrow algorithm", &alg, "GJK-EPA\0\0"))
    {
        if (alg == 0)
            m_app->world.collisions.detection()->set_pp_narrow_detection<gjk_epa_detection2D>();
    }
    if (gjk)
        ImGui::SliderFloat("P-P EPA Threshold", &gjk->epa_threshold, 1.e-4f, 1.e-1f, "%.4f",
                           ImGuiSliderFlags_Logarithmic);
}

void collision_tab::render_pp_manifold_list() const
{
    int alg;
    if (m_app->world.collisions.detection()->pp_manifold_algorithm<clipping_algorithm_manifold2D>())
        alg = 0;
    else if (m_app->world.collisions.detection()->pp_manifold_algorithm<mtv_support_manifold2D>())
        alg = 1;
    if (ImGui::Combo("P-P Manifold algorithm", &alg, "Clipping\0MTV Support\0\0"))
    {
        if (alg == 0)
            m_app->world.collisions.detection()->set_pp_manifold_algorithm<clipping_algorithm_manifold2D>();
        else if (alg == 1)
            m_app->world.collisions.detection()->set_pp_manifold_algorithm<mtv_support_manifold2D>();
    }
}

void collision_tab::render_quad_tree_parameters(quad_tree_detection2D &qtdet)
{
    ImGui::Checkbox("Force square shape", &qtdet.force_square_shape);

    auto &props = qtdet.quad_tree().props();
    ImGui::SliderInt("Max colliders per quadrant", (int *)&props.elements_per_quad, 2, 20);
    ImGui::SliderInt("Max depth", (int *)&props.max_depth, 1, 24);
    ImGui::SliderFloat("Min quadrant size", &props.min_quad_size, 4.f, 50.f);

    ImGui::Checkbox("Visualize tree", &m_visualize_qtree);
    if (m_visualize_qtree)
        render_quad_tree_lines();
}

void collision_tab::render_nonpen_contact_solver_parameters()
{
    float friction = 0.f;
    float restitution = 0.f;

    for (const collider2D *collider : m_app->world.colliders)
    {
        friction += collider->friction;
        restitution += collider->restitution;
    }
    friction /= m_app->world.colliders.size();
    restitution /= m_app->world.colliders.size();

    if (ImGui::SliderFloat("Friction", &friction, 0.f, 1.f))
        for (collider2D *collider : m_app->world.colliders)
            collider->friction = friction;
    if (ImGui::SliderFloat("Restitution", &restitution, 0.f, 1.f))
        for (collider2D *collider : m_app->world.colliders)
            collider->restitution = restitution;
}

void collision_tab::render_spring_contact_solver_parameters()
{
    ImGui::SliderFloat("Rigidity", &spring_contact2D::rigidity, 0.f, 5000.f);
    ImGui::SliderFloat("Max normal damping", &spring_contact2D::max_normal_damping, 0.f, 50.f);
    ImGui::SliderFloat("Max tangent damping", &spring_contact2D::max_tangent_damping, 0.f, 50.f);
}

// std vector pretty overkill
static std::vector<glm::vec2> get_bbox_points(const aabb2D &aabb)
{
    const glm::vec2 &mm = aabb.min;
    const glm::vec2 &mx = aabb.max;
    return {glm::vec2(mm.x, mx.y), mx, glm::vec2(mx.x, mm.y), mm, glm::vec2(mm.x, mx.y)};
}

void collision_tab::update_quad_tree_lines(const quad_tree::node &qtnode)
{
    if (qtnode.partitioned)
        for (auto child : qtnode.children)
            update_quad_tree_lines(*child);
    else
    {
        const std::vector<glm::vec2> points = get_bbox_points(qtnode.aabb);
        if (m_qt_active_partitions < m_qt_lines.size())
            for (std::size_t i = 0; i < points.size(); i++)
                m_qt_lines[m_qt_active_partitions][i].position = points[i];
        else
            m_qt_lines.emplace_back(points);
        m_qt_active_partitions++;
    }
}

void collision_tab::update_bounding_boxes()
{
    for (std::size_t i = 0; i < m_app->world.colliders.size(); i++)
    {
        const std::vector<glm::vec2> points = get_bbox_points(m_app->world.colliders[i]->bounding_box());
        if (i < m_bbox_lines.size())
            for (std::size_t j = 0; j < points.size(); j++)
                m_bbox_lines[i][j].position = points[j];
        else
            m_bbox_lines.emplace_back(points);
    }
}

void collision_tab::render_bounding_boxes() const
{
    for (std::size_t i = 0; i < m_app->world.colliders.size(); i++)
        m_window->draw(m_bbox_lines[i]);
}

void collision_tab::render_quad_tree_lines() const
{
    for (std::size_t i = 0; i < m_qt_active_partitions; i++)
        m_window->draw(m_qt_lines[i]);
}

YAML::Node collision_tab::encode() const
{
    YAML::Node node;
    node["Draw bounding boxes"] = m_draw_bounding_boxes;
    node["Draw contacts"] = m_draw_contacts;
    node["Visualize quad tree"] = m_visualize_qtree;

    return node;
}
void collision_tab::decode(const YAML::Node &node)
{
    m_draw_bounding_boxes = node["Draw bounding boxes"].as<bool>();
    m_draw_contacts = node["Draw contacts"].as<bool>();
    m_visualize_qtree = node["Visualize quad tree"].as<bool>();
}
} // namespace ppx::demo
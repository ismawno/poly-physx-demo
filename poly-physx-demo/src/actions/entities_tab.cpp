#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/entities_tab.hpp"
#include "ppx-demo/app/demo_app.hpp"

namespace ppx::demo
{
entities_tab::entities_tab(demo_app *app) : m_app(app)
{
    m_window = m_app->window();
}

void entities_tab::update()
{
    if (m_signal_clear_bodies)
        m_app->world.bodies.clear();
    else
        for (const body2D::const_ptr &body : m_bodies_to_remove)
            if (body)
                m_app->world.bodies.remove(*body);
    for (const collider2D::const_ptr &collider : m_colliders_to_remove)
        if (collider)
            m_app->world.colliders.remove(*collider);

    m_bodies_to_remove.clear();
    m_colliders_to_remove.clear();
    m_signal_clear_bodies = false;
}

void entities_tab::render_imgui_tab()
{
    render_general_options();
    render_selected_bodies_properties();
    render_selected_colliders_properties();
    if (ImGui::CollapsingHeader("Bodies"))
        render_bodies_list();
    if (ImGui::CollapsingHeader("Colliders"))
        render_colliders_list();
    if (ImGui::CollapsingHeader("Saved body presets"))
        render_groups();
}

void entities_tab::render_general_options()
{
    m_signal_clear_bodies = ImGui::Button("Remove all");
    if (ImGui::Button("Remove empty"))
        for (const body2D &body : m_app->world.bodies)
            if (body.empty())
                m_bodies_to_remove.push_back(body.as_ptr());

    ImGui::SameLine();
    if (ImGui::Button("Add walls"))
        m_app->add_walls();
}

void entities_tab::render_bodies_list()
{
    for (body2D &body : m_app->world.bodies)
        if (ImGui::TreeNode(&body, "%s", kit::uuid::name_from_id(body.id).c_str()))
        {
            render_single_body_properties(body);
            ImGui::TreePop();
        }
}
void entities_tab::render_colliders_list()
{
    for (collider2D &collider : m_app->world.colliders)
        if (ImGui::TreeNode(&collider, "%s", kit::uuid::name_from_id(collider.id).c_str()))
        {
            render_single_collider_properties(collider);
            ImGui::TreePop();
        }
}

void entities_tab::render_single_body_properties(body2D &body)
{
    const auto body_ptr = body.as_ptr();

    if (m_app->selector.is_selected(body_ptr) && ImGui::Button("Deselect"))
        m_app->selector.deselect(body_ptr);
    if (!m_app->selector.is_selected(body_ptr) && ImGui::Button("Select"))
        m_app->selector.select(body_ptr);

    if (ImGui::Button("Remove"))
        m_bodies_to_remove.push_back(body_ptr);

    ImGui::Text("Name: %s", kit::uuid::name_from_id(body.id).c_str());
    ImGui::Text("UUID: %llu", (std::uint64_t)body.id);

    static constexpr float drag_speed = 0.3f;
    static constexpr const char *format = "%.1f";

    auto tp = body.type();
    if (ImGui::Combo("Type", (int *)&tp, "Dynamic\0Kinematic\0Static\0\0"))
        body.type(tp);

    float mass = body.props().nondynamic.mass;
    if (ImGui::DragFloat("Mass", &mass, drag_speed, 0.f, FLT_MAX, format))
        body.mass(mass);
    ImGui::DragFloat("Charge", &body.charge, drag_speed, 0.f, 0.f, format);
    ImGui::Text("Inertia: %.1f", body.props().nondynamic.inertia);

    glm::vec2 pos = body.position();
    if (ImGui::DragFloat2("Position", glm::value_ptr(pos), drag_speed, 0.f, 0.f, format))
        body.position(pos);

    glm::vec2 centroid = body.centroid();
    if (ImGui::DragFloat2("Centroid", glm::value_ptr(centroid), drag_speed, 0.f, 0.f, format))
        body.centroid(centroid);

    ImGui::DragFloat2("Velocity", glm::value_ptr(body.velocity), drag_speed, 0.f, 0.f, format);

    float rotation = body.rotation();
    if (ImGui::DragFloat("Rotation", &rotation, 0.1f * drag_speed, 0.f, 0.f, format))
        body.rotation(rotation);

    ImGui::DragFloat("Angular velocity", &body.angular_velocity, drag_speed, 0.f, 0.f, format);

    const glm::vec2 force = body.force();
    ImGui::Text("Force: (%.1f, %.1f)", force.x, force.y);
    ImGui::DragFloat2("Force modifier", glm::value_ptr(body.persistent_force), drag_speed, 0.f, 0.f, format);

    const float torque = body.torque();
    ImGui::Text("Torque: %.1f", torque);
    ImGui::DragFloat("Torque modifier", &body.persistent_torque, drag_speed, 0.f, 0.f, format);

    if (ImGui::TreeNode("Colliders"))
    {
        for (collider2D &collider : body)
            if (ImGui::TreeNode(&collider, "%s", kit::uuid::name_from_id(collider.id).c_str()))
            {
                render_single_collider_properties(collider);
                ImGui::TreePop();
            }
        ImGui::TreePop();
    }
}

static void display_vertices(const kit::dynarray<glm::vec2, PPX_MAX_VERTICES> &vertices, const char *label)
{
    if (ImGui::TreeNode(label))
    {
        for (std::size_t i = 0; i < vertices.size(); i++)
            ImGui::Text("%lu: (%.1f, %.1f)", i + 1, vertices[i].x, vertices[i].y);
        ImGui::TreePop();
    }
}

void entities_tab::render_single_collider_properties(collider2D &collider)
{
    const auto collider_ptr = collider.as_ptr();

    if (m_app->selector.is_selected(collider_ptr) && ImGui::Button("Deselect"))
        m_app->selector.deselect(collider_ptr);
    if (!m_app->selector.is_selected(collider_ptr) && ImGui::Button("Select"))
        m_app->selector.select(collider_ptr);

    if (ImGui::Button("Remove"))
        m_colliders_to_remove.push_back(collider_ptr);

    static constexpr float drag_speed = 0.3f;
    static constexpr const char *format = "%.1f";

    float density = collider.density();
    if (ImGui::DragFloat("Density", &density, drag_speed, 0.f, 0.f, format))
        collider.density(density);

    float charge_density = collider.charge_density();
    if (ImGui::DragFloat("Charge density", &charge_density, drag_speed, 0.f, 0.f, format))
        collider.charge_density(charge_density);

    ImGui::SliderFloat("Friction", &collider.friction, 0.f, 1.f, format);
    ImGui::SliderFloat("Restitution", &collider.restitution, 1.f, 0.f, format);

    glm::vec2 lpos = collider.lposition();
    if (ImGui::DragFloat2("Local position", glm::value_ptr(lpos), drag_speed, 0.f, 0.f, format))
        collider.lposition(lpos);

    glm::vec2 lcentroid = collider.lcentroid();
    if (ImGui::DragFloat2("Local centroid", glm::value_ptr(lcentroid), drag_speed, 0.f, 0.f, format))
        collider.lcentroid(lcentroid);
    glm::vec2 gcentroid = collider.gcentroid();
    if (ImGui::DragFloat2("Global centroid", glm::value_ptr(gcentroid), drag_speed, 0.f, 0.f, format))
        collider.gcentroid(gcentroid);

    float lrotation = collider.lrotation();
    if (ImGui::DragFloat("Local rotation", &lrotation, 0.1f * drag_speed, 0.f, 0.f, format))
        collider.lrotation(lrotation);

    ImGui::Text("Area: %.1f", collider.area());
    ImGui::Text("Inertia: %.1f", collider.inertia());

    if (const auto *poly = collider.shape_if<polygon>())
    {
        if (ImGui::TreeNode("Vertices"))
        {
            display_vertices(poly->vertices.locals, "Locals");
            display_vertices(poly->vertices.globals, "Globals");
            display_vertices(poly->vertices.model, "Model");
            ImGui::TreePop();
        }
    }
    else
        ImGui::Text("Radius: %.1f", collider.shape<circle>().radius());
}

void entities_tab::render_selected_bodies_properties()
{
    const auto &selected = m_app->selector.selected_bodies();

    if (ImGui::TreeNode(&selected, "Selected bodies: %zu", selected.size()))
    {
        if (selected.empty())
        {
            ImGui::Text("No bodies selected");
            ImGui::TreePop();
            return;
        }
        if (selected.size() == 1)
        {
            render_single_body_properties(**selected.begin());
            ImGui::TreePop();
            return;
        }

        if (ImGui::Button("Remove selected"))
            m_bodies_to_remove.insert(m_bodies_to_remove.end(), selected.begin(), selected.end());

        static constexpr float drag_speed = 0.3f;
        static constexpr const char *format = "%.1f";

        float mass = 0.f;
        float charge = 0.f;

        for (const body2D::ptr &body : selected)
        {
            mass += body->props().nondynamic.mass;
            charge += body->charge;
        }
        mass /= selected.size();
        charge /= selected.size();

        if (ImGui::DragFloat("Mass", &mass, drag_speed, 0.f, FLT_MAX, format))
            for (const body2D::ptr &body : selected)
                body->mass(mass);

        if (ImGui::DragFloat("Charge", &charge, drag_speed, 0.f, 0.f, format))
            for (const body2D::ptr &body : selected)
                body->charge = charge;

        static char buffer[24] = "\0";
        if (ImGui::InputTextWithHint("Save as a group", "Group name", buffer, 24,
                                     ImGuiInputTextFlags_EnterReturnsTrue) &&
            buffer[0] != '\0')
        {
            std::string name = buffer;
            std::replace(name.begin(), name.end(), ' ', '-');

            m_app->grouper.save_group_from_selected(name);
            buffer[0] = '\0';
        }
        ImGui::TreePop();
    }
}

void entities_tab::render_selected_colliders_properties()
{
    const auto &selected = m_app->selector.selected_colliders();

    if (ImGui::TreeNode(&selected, "Selected colliders: %zu", selected.size()))
    {
        if (selected.empty())
        {
            ImGui::Text("No colliders selected");
            ImGui::TreePop();
            return;
        }
        if (selected.size() == 1)
        {
            render_single_collider_properties(**selected.begin());
            ImGui::TreePop();
            return;
        }

        if (ImGui::Button("Remove selected"))
            m_colliders_to_remove.insert(m_colliders_to_remove.end(), selected.begin(), selected.end());

        static constexpr float drag_speed = 0.3f;
        static constexpr const char *format = "%.1f";

        float friction = 0.f;
        float restitution = 0.f;

        float density = 0.f;
        float charge_density = 0.f;

        float area = 0.f;
        float inertia = 0.f;

        for (const collider2D::ptr &collider : selected)
        {
            friction += collider->friction;
            restitution += collider->restitution;
            density += collider->density();
            charge_density += collider->charge_density();
            area += collider->area();
            inertia += collider->inertia();
        }
        friction /= selected.size();
        restitution /= selected.size();
        area /= selected.size();
        inertia /= selected.size();
        density /= selected.size();
        charge_density /= selected.size();

        if (ImGui::SliderFloat("Friction", &friction, 0.f, 1.f, format))
            for (const collider2D::ptr &collider : selected)
                collider->friction = friction;

        if (ImGui::SliderFloat("Restitution", &restitution, 0.f, 1.f, format))
            for (const collider2D::ptr &collider : selected)
                collider->restitution = restitution;

        if (ImGui::DragFloat("Density", &density, drag_speed, 0.f, 0.f, format))
            for (const collider2D::ptr &collider : selected)
                collider->density(density);

        if (ImGui::DragFloat("Charge density", &charge_density, drag_speed, 0.f, 0.f, format))
            for (const collider2D::ptr &collider : selected)
                collider->charge_density(charge_density);

        ImGui::Text("Area: %.1f", area);
        ImGui::Text("Inertia: %.1f", inertia);

        ImGui::TreePop();
    }
}

void entities_tab::render_groups() const
{
    for (const auto &[name, group] : m_app->grouper.groups())
    {
        ImGui::PushID(&name);
        if (ImGui::Button("X"))
        {
            m_app->grouper.remove_group(name);
            return;
        }
        ImGui::PopID();
        ImGui::SameLine();
        if (ImGui::Selectable(name.c_str()))
            m_app->grouper.load_group(name);
    }
}
} // namespace ppx::demo
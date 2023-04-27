#include "add_tab.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "globals.hpp"
#include "demo_app.hpp"
#include <SFML/Graphics.hpp>
#include <glm/gtx/norm.hpp>

namespace ppx_demo
{
    void add_tab::render() const
    {
        ImGui::PushItemWidth(150);
        render_menu_bar();
        render_shape_list();
        render_entity_inputs();
        ImGui::Spacing();
        render_color_picker();
        ImGui::PopItemWidth();
    }

    void add_tab::render_menu_bar() const
    {
        adder &addr = demo_app::get().p_adder;
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Entities"))
            {
                if (ImGui::MenuItem("Save", nullptr, nullptr, addr.has_saved_entity()))
                    addr.save_template();
                if (ImGui::MenuItem("Load", nullptr, nullptr, addr.has_saved_entity()))
                    addr.load_template();
                if (ImGui::BeginMenu("Save as..."))
                {
                    static char buffer[24] = "\0";
                    if (ImGui::InputTextWithHint("##", "Entity name", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue) && buffer[0] != '\0')
                    {
                        SUBSTITUTE(buffer, ' ', '-')
                        addr.save_template(buffer);
                        buffer[0] = '\0';
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Load as..."))
                {
                    std::string selected, to_remove;
                    for (const auto &[name, templ] : addr.templates())
                    {
                        if (ImGui::Button("X"))
                            to_remove = name;
                        ImGui::SameLine();
                        if (ImGui::MenuItem(name.c_str()))
                            selected = name;
                    }
                    if (!to_remove.empty())
                        addr.erase_template(to_remove);

                    if (!selected.empty())
                        addr.load_template(selected);
                    ImGui::EndMenu();
                }

                ImGui::EndMenu();
            }
            ImGui::BeginMenu(addr.has_saved_entity() ? ("Current entity: " + addr.p_current_templ.name).c_str() : "No entity template. Select 'Save as...' to create one", false);
            ImGui::EndMenuBar();
        }
    }

    void add_tab::render_shape_list() const
    {
        demo_app &papp = demo_app::get();
        adder &addr = papp.p_adder;
        adder::shape_type &shape = addr.p_current_templ.shape;

        const char *shapes[4] = {"Rectangle", "NGon", "Circle", "Custom"};
        if (ImGui::ListBox("Shapes", (int *)&shape, shapes, IM_ARRAYSIZE(shapes)))
            addr.update_template_vertices();

        const sf::Color &color = papp.entity_color();
        switch (shape)
        {
        case adder::RECT:
        {
            const glm::vec2 size = glm::vec2(addr.p_current_templ.width, addr.p_current_templ.height) * WORLD_TO_PIXEL,
                            pos = glm::vec2(350.f, -30.f) - 0.5f * size;
            ImGui::DrawRectFilled(sf::FloatRect({pos.x, pos.y}, {size.x, size.y}), color);
            break;
        }
        case adder::NGON:
        {
            const float radius = addr.p_current_templ.ngon_radius * WORLD_TO_PIXEL;
            const ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            draw_list->AddNgonFilled({pos.x + 350.f, pos.y - 30.f}, radius, ImColor(color.r, color.g, color.b), (int)addr.p_current_templ.sides);
            break;
        }
        case adder::CIRCLE:
        {
            const float radius = addr.p_current_templ.circle_radius * WORLD_TO_PIXEL;
            const ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            draw_list->AddCircle({pos.x + 350.f, pos.y - 30.f}, radius, ImColor(color.r, color.g, color.b));
            break;
        }
        default:
            break;
        }
    }

    void add_tab::render_entity_inputs() const
    {
        demo_app &papp = demo_app::get();
        adder &addr = papp.p_adder;

        ImGui::DragFloat("Mass", &addr.p_current_templ.entity_templ.mass, 0.2f, 1.f, FLT_MAX, "%.1f");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("The mass of an entity represents how hard it is to move it.");

        ImGui::DragFloat("Charge", &addr.p_current_templ.entity_templ.charge, 0.2f, -FLT_MAX, FLT_MAX, "%.1f");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("The charge of an entity represents how strongly\nit will react to electrical interactions.");
        switch (addr.p_current_templ.shape)
        {
        case adder::RECT:
            ImGui::DragFloat("Width", &addr.p_current_templ.width, 0.2f, 1.f, FLT_MAX, "%.1f");
            // if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            //     ImGui::SetTooltip("Together with the shape, it is directly proportional\nto the inertia that the entity will have.");
            ImGui::DragFloat("Height", &addr.p_current_templ.height, 0.2f, 1.f, FLT_MAX, "%.1f");
            break;
        case adder::NGON:
            ImGui::SliderInt("Sides", (int *)&addr.p_current_templ.sides, 3, 30);
            ImGui::DragFloat("Radius", &addr.p_current_templ.ngon_radius, 0.2f, 1.f, FLT_MAX, "%.1f");
            break;
        case adder::CIRCLE:
            ImGui::DragFloat("Radius", &addr.p_current_templ.circle_radius, 0.2f, 1.f, FLT_MAX, "%.1f");
            break;
        case adder::CUSTOM:
            render_canvas();
            break;
        default:
            break;
        }

        ImGui::Checkbox("Kinematic", &addr.p_current_templ.entity_templ.kinematic);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("If unchecked, the entity will not accelerate by any means.");

        if (papp.p_predictor.p_enabled)
            ImGui::Checkbox("Predict path", &addr.p_predict_path);
        addr.update_template_vertices();
    }

    void add_tab::render_color_picker() const
    {
        demo_app &papp = demo_app::get();

        const sf::Color &color = papp.entity_color();
        float imcolor[3] = {color.r / 255.f, color.g / 255.f, color.b / 255.f};
        if (ImGui::ColorPicker3("Entity color", imcolor, ImGuiColorEditFlags_NoTooltip))
        {
            papp.entity_color({(sf::Uint8)(imcolor[0] * 255.f), (sf::Uint8)(imcolor[1] * 255.f), (sf::Uint8)(imcolor[2] * 255.f)});
            papp.p_adder.p_current_templ.color = color;
        }
    }

    void add_tab::render_canvas() const
    {
        static glm::vec2 scrolling(0.f);
        demo_app &papp = demo_app::get();

        const geo::polygon &poly = std::get<geo::polygon>(papp.p_adder.p_current_templ.entity_templ.shape);
        std::vector<glm::vec2> vertices = poly.vertices();

        const bool is_convex = poly.is_convex();
        if (!is_convex)
        {
            ImGui::SameLine(ImGui::GetWindowWidth() - 575.f);
            ImGui::Text("The polygon is not convex!");
            ImGui::SameLine();
            ImGui::HelpMarker("WIP Why does this matter");
        }

        const ImVec2 canvas_p0 = ImGui::GetCursorScreenPos(),
                     canvas_sz = ImGui::GetContentRegionAvail(),
                     canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

        const glm::vec2 canvas_hdim = glm::vec2(canvas_sz.x, canvas_sz.y) * 0.5f;

        // Draw border and background color
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
        draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

        // This will catch our interactions
        ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft);
        const bool is_hovered = ImGui::IsItemHovered();

        const float scale_factor = 1.5f;
        ImGuiIO &io = ImGui::GetIO();
        const glm::vec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y), // Lock scrolled origin
            pixel_mouse = (glm::vec2(io.MousePos.x, io.MousePos.y) - canvas_hdim - origin) / scale_factor,
            world_mouse = pixel_mouse * PIXEL_TO_WORLD;

        const glm::vec2 towards_poly = poly.closest_direction_from(world_mouse);
        const float max_dist = 5.f;
        const bool valid_to_add = is_hovered && glm::length2(towards_poly) < max_dist;

        std::size_t to_edit = vertices.size() - 1;
        const float thres_distance = 2.f;
        float min_distance = std::numeric_limits<float>::max();

        for (std::size_t i = 0; i < vertices.size(); i++)
        {
            const float dist = glm::distance2(vertices[i], world_mouse);
            if (dist < min_distance)
            {
                min_distance = dist;
                to_edit = i;
            }
        }

        const bool create_vertex = min_distance >= thres_distance;
        if (create_vertex)
        {
            if (valid_to_add && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                vertices.push_back(world_mouse);
            to_edit = vertices.size() - 1;
        }
        if (valid_to_add && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            vertices[to_edit] = world_mouse;

        draw_list->PushClipRect(canvas_p0, canvas_p1, true);
        const float grid_step = 64.f;
        for (float x = fmodf(scrolling.x, grid_step); x < canvas_sz.x; x += grid_step)
            draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
        for (float y = fmodf(scrolling.y, grid_step); y < canvas_sz.y; y += grid_step)
            draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));

        const sf::Color &entity_col = papp.entity_color();
        const auto col = is_convex ? IM_COL32(entity_col.r, entity_col.g, entity_col.b, entity_col.a)
                                   : IM_COL32(255, 0, 0, 255);

        std::vector<ImVec2> points(poly.size());
        for (std::size_t i = 0; i < poly.size(); i++)
        {
            const glm::vec2 p1 = origin + poly[i] * scale_factor * WORLD_TO_PIXEL + canvas_hdim,
                            p2 = origin + poly[i + 1] * scale_factor * WORLD_TO_PIXEL + canvas_hdim;
            const float thickness = 3.f;
            draw_list->AddLine({p1.x, p1.y}, {p2.x, p2.y}, col, thickness);
            points[i] = {p1.x, p1.y};
        }

        if (is_convex)
            draw_list->AddConvexPolyFilled(points.data(), (int)poly.size(), IM_COL32(entity_col.r, entity_col.g, entity_col.b, 120));
        if (valid_to_add)
        {
            const glm::vec2 center = create_vertex ? origin + (pixel_mouse + towards_poly * WORLD_TO_PIXEL) * scale_factor + canvas_hdim
                                                   : origin + vertices[to_edit] * scale_factor * WORLD_TO_PIXEL + canvas_hdim;
            const float radius = 8.f;
            draw_list->AddCircleFilled({center.x, center.y}, radius, IM_COL32(207, 185, 151, 180));
        }
        draw_list->PopClipRect();
    }
}
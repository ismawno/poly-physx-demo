#pragma once

#include "ppx/body/body.hpp"
#include "ppx/collider/collider.hpp"
#include "ppx/joints/spring_joint.hpp"
#include "ppx/joints/distance_joint.hpp"
#include "ppx/joints/revolute_joint.hpp"
#include "ppx/joints/weld_joint.hpp"
#include "ppx/joints/rotor_joint.hpp"
#include "ppx/joints/motor_joint.hpp"
#include "ppx/joints/ball_joint.hpp"
#include "ppx/joints/prismatic_joint.hpp"

#include "lynx/drawing/shape.hpp"

#include "ppx-app/drawables/lines/spring_line.hpp"
#include "ppx-app/drawables/lines/thick_line.hpp"

namespace ppx::demo
{
class demo_app;
class group_manager
{
  public:
    group_manager(demo_app &app);

    void update();
    void render() const;

    void begin_group_from_selected();
    void paste_group();
    void cancel_group();

    void save_group_from_selected(const std::string &name);
    void load_group(const std::string &name);
    void remove_group(const std::string &name);

    bool ongoing_group() const;

    YAML::Node encode() const;
    void decode(const YAML::Node &node);

  private:
    struct body_proxy
    {
        body2D::specs specs;
        std::vector<lynx::color> colors;
    };

    template <typename Specs> struct joint_proxy
    {
        std::size_t bprox_index1;
        std::size_t bprox_index2;
        lynx::color color;
        Specs specs;
    };

    template <typename T> struct jtype_to_line
    {
    };
    template <> struct jtype_to_line<spring_joint2D>
    {
        static inline constexpr std::size_t index = 0;
        using type = spring_line2D;
    };
    template <> struct jtype_to_line<distance_joint2D>
    {
        static inline constexpr std::size_t index = 1;
        using type = thick_line2D;
    };
    template <> struct jtype_to_line<prismatic_joint2D>
    {
        static inline constexpr std::size_t index = 2;
        using type = lynx::thin_line2D;
    };

    template <class... Joints> struct jproxies
    {
        std::tuple<std::vector<joint_proxy<Joints>>...> proxies;
        template <typename Joint> auto &get() const
        {
            using Specs = typename Joint::specs;
            return std::get<std::vector<joint_proxy<Specs>>>(proxies);
        }
        template <typename Joint> auto &get()
        {
            using Specs = typename Joint::specs;
            return std::get<std::vector<joint_proxy<Specs>>>(proxies);
        }
    };
    template <class... Joints> struct jpreviews
    {
        std::tuple<std::vector<typename jtype_to_line<Joints>::type>...> previews;
        template <typename Joint> auto &get() const
        {
            constexpr std::size_t index = jtype_to_line<Joint>::index;
            return std::get<index>(previews);
        }
        template <typename Joint> auto &get()
        {
            constexpr std::size_t index = jtype_to_line<Joint>::index;
            return std::get<index>(previews);
        }
    };

    struct group
    {
        glm::vec2 mean_position{0.f};
        std::vector<body_proxy> bproxies;
        jproxies<spring_joint2D::specs, distance_joint2D::specs, revolute_joint2D::specs, weld_joint2D::specs,
                 rotor_joint2D::specs, motor_joint2D::specs, ball_joint2D::specs, prismatic_joint2D::specs>
            jproxies;

        template <typename Joint> YAML::Node encode_proxies() const;
        template <typename Joint> void decode_proxies(const YAML::Node &node);

        YAML::Node encode(world2D &world) const;
        void decode(const YAML::Node &node, world2D &world);
    };

  public:
    const std::unordered_map<std::string, group> &groups() const;

  private:
    demo_app &m_app;
    lynx::window2D *m_window;

    group m_current_group;

    std::unordered_map<std::string, group> m_groups;

    kit::transform2D<float> m_preview_transform;
    std::vector<kit::transform2D<float>> m_bodies_preview_transforms;
    std::vector<kit::scope<lynx::shape2D>> m_shapes_preview;

    jpreviews<spring_joint2D, distance_joint2D, prismatic_joint2D> m_joints_preview;

    bool m_ongoing_group = false;

    template <typename Joint> void paste_current_joints(const std::vector<std::size_t> &added_indices);

    template <typename Joint>
    void add_joints_to_group(group &grp, const std::vector<const body2D *> &selected_bodies, std::size_t idx1,
                             std::size_t idx2);

    void update_preview_from_current_group();
    template <typename Joint> void update_preview_from_current_joint_proxies();

    group create_group_from_selected();
};
} // namespace ppx::demo

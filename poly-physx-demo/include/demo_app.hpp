#ifndef DEMO_APP_HPP
#define DEMO_APP_HPP

#include "actions_panel.hpp"
#include "adder.hpp"
#include "attacher.hpp"
#include "copy_paste.hpp"
#include "engine_panel.hpp"
#include "follower.hpp"
#include "menu_bar.hpp"
#include "outline_manager.hpp"
#include "perf_panel.hpp"
#include "phys_panel.hpp"
#include "ppx-app/app.hpp"
#include "predictor.hpp"
#include "selector.hpp"
#include "trail_manager.hpp"

namespace ppx_demo
{
class demo_app : public ppx::app
{
  public:
    static demo_app &get();

    void serialize(const std::string &filepath) const;
    bool deserialize(const std::string &filepath);

    void write_save(const std::string &filename) const;
    bool read_save(const std::string &filename);

    void write_save() const;
    bool read_save();

    bool read_example(const std::string &filename);

    bool validate_session();

    void add_borders();
    void remove_selected();

    const std::string &session() const;
    void session(const std::string &session);
    bool has_session() const;

    selector p_selector;
    outline_manager p_outline_manager;
    copy_paste p_copy_paste;
    predictor p_predictor;
    trail_manager p_trails;
    follower p_follower;

    ppx::ref<actions_panel> p_actions_panel;
    ppx::ref<engine_panel> p_engine_panel;
    ppx::ref<perf_panel> p_perf_panel;
    ppx::ref<phys_panel> p_phys_panel;
    ppx::ref<menu_bar> p_menu_bar;

  private:
    demo_app();

    void on_start() override;
    void on_update() override;
    void on_late_update() override;
    void on_render() override;
    void on_entity_draw(const ppx::entity2D_ptr &e, sf::Shape &shape) override;
    void on_event(sf::Event &event) override;
    void on_end() override;

    std::string m_session;
    sf::Clock m_clock;

    demo_app(const demo_app &papp) = delete;
};
} // namespace ppx_demo

#endif
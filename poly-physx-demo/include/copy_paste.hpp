#ifndef COPY_PASTE_HPP
#define COPY_PASTE_HPP

#include "templates.hpp"
#include "selector.hpp"
#include <unordered_map>
#include <SFML/Graphics.hpp>

namespace ppx_demo
{
    class copy_paste : public ini::saveable
    {
    public:
        copy_paste() = default;

        void render();
        void copy();
        void paste();
        void delete_copy();

        void save_group(const std::string &name);
        void load_group(const std::string &name);
        void erase_group(const std::string &name);

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

    private:
        struct group : private ini::saveable
        {
            std::string name;
            alg::vec2 ref_pos;
            std::unordered_map<std::size_t, entity_template> entities;
            std::vector<spring_template> springs;
            std::vector<rigid_bar_template> rbars;

            void write(ini::output &out) const override;
            void read(ini::input &in) override;
        };

    public:
        const std::map<std::string, group> &groups() const;
        const group &current_group() const;

    private:
        group m_copy;
        bool m_has_copy = false;
        std::map<std::string, group> m_groups;

        void preview();
        void copy(group &group);
    };
}

#endif
#ifndef FLAT_LINE_HPP
#define FLAT_LINE_HPP

#include "vec2.hpp"
#include <SFML/Graphics.hpp>

namespace prm
{
    class flat_line : public sf::Drawable
    {
    public:
        flat_line(const alg::vec2 &p1, const alg::vec2 &p2,
                  const sf::Color &color1,
                  const sf::Color &color2);

        flat_line(const alg::vec2 &p1, const alg::vec2 &p2,
                  const sf::Color &color = sf::Color::White);

        const alg::vec2 &p1() const;
        const alg::vec2 &p2() const;

        const sf::Color &color1() const;
        const sf::Color &color2() const;

        void p1(const alg::vec2 &p1);
        void p2(const alg::vec2 &p2);

        void color1(const sf::Color &c1);
        void color2(const sf::Color &c2);

    private:
        alg::vec2 m_p1, m_p2;
        sf::Color m_color1, m_color2;

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
    };
}

#endif
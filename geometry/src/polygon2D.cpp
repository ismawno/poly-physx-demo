#include "polygon2D.hpp"
#include "debug.h"
#include <algorithm>
#include <cstdint>
#include <cmath>
#include <limits>

namespace geo
{
    polygon2D::polygon2D(const std::vector<vec2> &vertices,
                         const vec2 &pos) : polygon2D(vertices)
    {
        DBG_EXIT_IF(m_vertices.size() < 3, "Cannot make polygon with less than 3 vertices.\n")
        const vec2 cvert = centre_of_vertices();
        translate(-cvert);
        sort_vertices_by_angle();
        m_centroid = centre_of_mass();
        translate(pos - m_centroid);
    }
    polygon2D::polygon2D(const std::vector<vec2> &vertices) : m_vertices(vertices)
    {
        DBG_EXIT_IF(m_vertices.size() < 3, "Cannot make polygon with less than 3 vertices.\n")
        const vec2 cvert = centre_of_vertices();
        translate(-cvert);
        sort_vertices_by_angle();
        m_centroid = centre_of_mass();
        translate(cvert);
    }

    vec2 polygon2D::centre_of_mass() const
    {
        const vec2 &v0 = m_vertices[0];

        vec2 num;
        float den = 0.f;
        for (std::size_t i = 1; i < m_vertices.size() - 1; i++)
        {
            const vec2 edg1 = m_vertices[i] - v0, edg2 = m_vertices[i + 1] - v0;
            const float cross = std::abs(edg1.cross(edg2));
            num += (edg1 + edg2) * cross;
            den += cross;
        }
        return v0 + num / (3.f * den);
    }

    vec2 polygon2D::centre_of_vertices() const
    {
        vec2 centre;
        for (const vec2 &v : m_vertices)
            centre += v;
        return centre / m_vertices.size();
    }

    void polygon2D::translate(const vec2 &dpos)
    {
        for (vec2 &v : m_vertices)
            v += dpos;
        m_centroid += dpos;
    }

    polygon2D polygon2D::minkowski_sum(const polygon2D &poly1, const polygon2D &poly2)
    {
        std::vector<vec2> sum;
        sum.reserve(poly1.size() + poly2.size());

        std::size_t i = 0, j = 0;
        while (i < poly1.size() || j < poly2.size())
        {
            sum.emplace_back(poly1[i] + poly2[j]);
            const float cross = (poly1[i + 1] - poly1[i]).cross(poly2[j + 1] - poly2[j]);
            if (cross >= 0.f)
                i++;
            if (cross <= 0.f)
                j++;
        }
        return polygon2D(sum);
    }

    const vec2 &polygon2D::support_vertex(const vec2 &direction) const
    {
        const vec2 &centroid = m_centroid;

        const auto cmp = [&direction, &centroid](const vec2 &v1, const vec2 &v2)
        { return direction.dot(v1 - centroid) < direction.dot(v2 - centroid); };

        const auto &support = std::max_element(m_vertices.begin(), m_vertices.end(), cmp);
        return *support;
    }

    bool polygon2D::is_convex() const
    {
        for (std::size_t i = 0; i < m_vertices.size(); i++)
        {
            const vec2 &prev = (*this)[i], &mid = (*this)[i + 1], &next = (*this)[i + 2];
            const vec2 accel = (next - mid) - (mid - prev);
            if (accel.dot(m_centroid - mid) < 0.f)
                return false;
        }
        return true;
    }

    bool polygon2D::contains_point(const vec2 &p) const
    {
        DBG_LOG_IF(!is_convex(), "Checking if a point is contained in a non convex polygon yields undefined behaviour.\n")
        for (std::size_t i = 0; i < m_vertices.size(); i++)
        {
            const vec2 &v1 = (*this)[i], &v2 = (*this)[i + 1];
            if (line_intersects_edge(v2, v1, p, m_centroid) && line_intersects_edge(p, m_centroid, v2, v1))
                return false;
        }
        return true;
    }

    bool polygon2D::contains_origin() const { return contains_point({0.f, 0.f}); }

    bool polygon2D::overlaps(const polygon2D &poly) const { return (*this - poly).contains_origin(); }

    float polygon2D::distance_to(const vec2 &p) const { return towards_closest_edge_from(p).norm(); }

    float polygon2D::distance_to_origin() const { return distance_to({0.f, 0.f}); }

    float polygon2D::distance_to(const polygon2D &poly) const { return (*this - poly).distance_to_origin(); }

    vec2 polygon2D::towards_segment_from(const vec2 &p1, const vec2 &p2, const vec2 &p)
    {
        const float t = std::clamp((p - p1).dot(p2 - p1) / p1.sq_dist(p2), 0.f, 1.f);
        const vec2 proj = p1 + t * (p2 - p1);
        return proj - p;
    }

    vec2 polygon2D::towards_closest_edge_from(const vec2 &p) const
    {
        float min_dist = std::numeric_limits<float>::max();
        vec2 closest;
        for (std::size_t i = 0; i < m_vertices.size(); i++)
        {
            const vec2 towards = towards_segment_from((*this)[i], (*this)[i + 1], p);
            const float dist = towards.sq_norm();
            if (min_dist > dist)
            {
                min_dist = dist;
                closest = towards;
            }
        }
        return closest;
    }

    bool polygon2D::line_intersects_edge(const vec2 &l1, const vec2 &l2, const vec2 &v1, const vec2 &v2)
    {
        const float a = l2.y - l1.y, b = l1.x - l2.x;
        const float c = l2.x * l1.y - l1.x * l2.y;

        const float d1 = a * v1.x + b * v1.y + c;
        const float d2 = a * v2.x + b * v2.y + c;
        return !((d1 > 0.f && d2 > 0.f) || (d1 < 0.f && d2 < 0.f));
    }

    void polygon2D::sort_vertices_by_angle()
    {
        const auto cmp = [](const vec2 &v1, const vec2 &v2)
        { return v1.angle() < v2.angle(); };
        std::sort(m_vertices.begin(), m_vertices.end(), cmp);
    }

    const std::vector<vec2> &polygon2D::vertices() const { return m_vertices; }

    std::size_t polygon2D::size() const { return m_vertices.size(); }

    const vec2 &polygon2D::operator[](const std::size_t index) const { return m_vertices[index % m_vertices.size()]; }

    polygon2D operator+(const polygon2D &poly) { return poly; }

    polygon2D &operator+(polygon2D &poly) { return poly; }

    polygon2D operator-(const polygon2D &poly)
    {
        std::vector<vec2> vertices;
        vertices.reserve(poly.size());
        for (const vec2 &v : poly.vertices())
            vertices.emplace_back(-v);
        return polygon2D(vertices);
    }

    polygon2D operator+(const polygon2D &poly1, const polygon2D &poly2) { return polygon2D::minkowski_sum(poly1, poly2); }

    polygon2D operator-(const polygon2D &poly1, const polygon2D &poly2) { return polygon2D::minkowski_sum(poly1, -poly2); }
}
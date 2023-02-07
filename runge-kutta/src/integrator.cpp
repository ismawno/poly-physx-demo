#include "integrator.hpp"
#include <cmath>
#define SAFETY_FACTOR 0.85f

namespace rk
{
    integrator::integrator(const butcher_tableau &tb,
                           const std::vector<float> &state,
                           const float tolerance,
                           const float min_dt,
                           const float max_dt) : m_tableau(tb),
                                                 m_state(state),
                                                 m_tolerance(tolerance),
                                                 m_min_dt(min_dt),
                                                 m_max_dt(max_dt),
                                                 m_error(0.f),
                                                 m_valid(true)
    {
        m_kvec.resize(m_tableau.stage());
        resize_to_state();
    }

    std::vector<float> integrator::generate_solution(const float dt,
                                                     const std::vector<float> &state,
                                                     const std::vector<float> &coefs)
    {
        PERF_FUNCTION()
        std::vector<float> sol;
        sol.reserve(state.size());
        for (std::size_t j = 0; j < state.size(); j++)
        {
            float sum = 0.f;
            for (std::uint8_t i = 0; i < m_tableau.stage(); i++)
                sum += coefs[i] * m_kvec[i][j];
            m_valid &= !isnan(sum);
            const float step = sum * dt;
            m_step[j] = step;
            sol.emplace_back(state[j] + step);
        }
        return sol;
    }

    static std::uint32_t ipow(std::uint32_t base, std::uint32_t exponent)
    {
        int result = 1;
        for (;;)
        {
            if (exponent & 1)
                result *= base;
            exponent >>= 1;
            if (!exponent)
                break;
            base *= base;
        }

        return result;
    }

    void integrator::write(ini::output &out) const
    {
        out.write("tolerance", m_tolerance);
        out.write("min_dt", m_min_dt);
        out.write("max_dt", m_max_dt);
        out.write("error", m_error);
        out.write("valid", m_valid);

        out.begin_section("tableau");
        m_tableau.write(out);
        out.end_section();

        std::string key = "state";
        for (std::size_t i = 0; i < m_state.size(); i++)
            out.write(key + std::to_string(i), m_state[i]);

        std::string key = "step";
        for (std::size_t i = 0; i < m_step.size(); i++)
            out.write(key + std::to_string(i), m_step[i]);

        std::string key = "kvec";
        for (std::size_t i = 0; i < m_kvec.size(); i++)
            for (std::size_t j = 0; j < m_kvec[i].size(); j++)
                out.write(key + std::to_string(i) + std::to_string(j), m_kvec[i][j]);
    }

    void integrator::read(ini::input &in)
    {
        m_tolerance = in.readf("tolerance");
        m_min_dt = in.readf("min_dt");
        m_max_dt = in.readf("max_dt");
        m_error = in.readf("error");
        m_valid = (bool)in.readi("valid");

        in.begin_section("tableau");
        m_tableau.read(in);
        in.end_section();

        m_state.clear();
        std::string key = "state";
        std::size_t index = 0;
        while (true)
        {
            const std::string full_key = key + std::to_string(index);
            if (!in.contains_key(full_key))
                break;
            m_state.emplace_back(in.readf(full_key));
        }

        m_step.clear();
        key = "step";
        index = 0;
        while (true)
        {
            const std::string full_key = key + std::to_string(index);
            if (!in.contains_key(full_key))
                break;
            m_step.emplace_back(in.readf(full_key));
        }

        m_kvec.clear();
        key = "kvec";
        index = 0;
        for (std::size_t i = 0; i < m_tableau.stage(); i++)
        {
            m_kvec.emplace_back().reserve(m_state.size());
            while (true)
            {
                const std::string full_key = key + std::to_string(i) + std::to_string(index);
                if (!in.contains_key(full_key))
                    break;
                m_kvec[i].emplace_back(in.readf(full_key));
            }
        }
    }

    bool integrator::dt_too_small(const float dt) const { return dt < m_min_dt; }
    bool integrator::dt_too_big(const float dt) const { return dt > m_max_dt; }
    bool integrator::dt_off_bounds(const float dt) const { return dt_too_small(dt) || dt_too_big(dt); }

    float integrator::embedded_error(const std::vector<float> &sol1, const std::vector<float> &sol2)
    {
        float result = 0.f;
        for (std::size_t i = 0; i < sol1.size(); i++)
            result += (sol1[i] - sol2[i]) * (sol1[i] - sol2[i]);
        return result;
    }

    float integrator::reiterative_error(const std::vector<float> &sol1, const std::vector<float> &sol2) const
    {
        const std::uint32_t coeff = ipow(2, m_tableau.order()) - 1;
        return embedded_error(sol1, sol2) / coeff;
    }

    float integrator::timestep_factor() const
    {
        return SAFETY_FACTOR * std::pow(m_tolerance / m_error, 1.f / m_tableau.order());
    }

    void integrator::reserve(std::size_t size)
    {
        m_state.reserve(size);
        for (std::vector<float> &v : m_kvec)
            v.reserve(size);
    }

    void integrator::resize_to_state()
    {
        for (std::vector<float> &v : m_kvec)
            v.resize(m_state.size());
        m_step.resize(m_state.size());
    }

    const butcher_tableau &integrator::tableau() const { return m_tableau; }
    butcher_tableau &integrator::tableau() { return m_tableau; }

    const std::vector<float> &integrator::state() const { return m_state; }
    std::vector<float> &integrator::state() { return m_state; }

    const std::vector<float> &integrator::step() const { return m_step; }

    void integrator::tableau(const butcher_tableau &tableau)
    {
        m_tableau = tableau;
        m_kvec.resize(m_tableau.stage());
        resize_to_state();
    }
    void integrator::state(std::vector<float> &state)
    {
        m_state = state;
        resize_to_state();
    }

    float integrator::tolerance() const { return m_tolerance; }
    float integrator::min_dt() const { return m_min_dt; }
    float integrator::max_dt() const { return m_max_dt; }
    float integrator::error() const { return m_error; }

    void integrator::tolerance(const float val) { m_tolerance = val; }
    void integrator::min_dt(const float val) { m_min_dt = val; }
    void integrator::max_dt(const float val) { m_max_dt = val; }
}
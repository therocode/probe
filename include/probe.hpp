#pragma once
#include <cstdint>
#include <string>

namespace pr
{
struct probe_state
{
    int64_t active = 0;
    int64_t existing = 0;
};

inline std::string to_string(const probe_state& s)
{
    std::string res = "active:   " + std::to_string(s.active) + "\n";
    res +=            "existing: " + std::to_string(s.existing);
    return res;
}

struct probe_counts
{
    int64_t defco = 0;
    int64_t valco = 0;
    int64_t copco = 0;
    int64_t movco = 0;
    int64_t copas = 0;
    int64_t movas = 0;
    int64_t destr = 0;

    int64_t moves = 0;
    int64_t copies = 0;

    int64_t min_active = 0;
    int64_t max_active = 0;
    int64_t min_existing = 0;
    int64_t max_existing = 0;
};

inline std::string to_string(const probe_counts& c)
{
    std::string res = "-calls-\n";
    res +=            " default constructor: " + std::to_string(c.defco) + "\n";
    res +=            " value constructor:   " + std::to_string(c.valco) + "\n";
    res +=            " copy constructor:    " + std::to_string(c.copco) + "\n";
    res +=            " move constructor:    " + std::to_string(c.movco) + "\n";
    res +=            " copy assignment:     " + std::to_string(c.copas) + "\n";
    res +=            " move assignment:     " + std::to_string(c.movas) + "\n";
    res +=            " destructor:          " + std::to_string(c.destr) + "\n";
    res +=            "-semantics-\n";
    res +=            " copies:              " + std::to_string(c.copies) + "\n";
    res +=            " moves:               " + std::to_string(c.moves) + "\n";
    res +=            "-min/max-\n";
    res +=            " min active:          " + std::to_string(c.min_active) + "\n";
    res +=            " max active:          " + std::to_string(c.max_active) + "\n";
    res +=            " min existing:        " + std::to_string(c.min_existing) + "\n";
    res +=            " max existing:        " + std::to_string(c.max_existing);
    return res;
}

void update_count_min_max(probe_counts& c, int64_t existing, int64_t active)
{
    c.min_existing = std::min(c.min_existing, existing);
    c.max_existing = std::max(c.max_existing, existing);
    c.min_active = std::min(c.min_active, active);
    c.max_active = std::max(c.max_active, active);
};

template <typename value_type>
class probe_t
{
    public:
    probe_t(): m_value(value_type{})
    {
        ++s_last.defco;++s_total.defco;

        ++s_state.existing;
        m_active = true;
        ++s_state.active;

        update_min_max();
    };
    probe_t(const value_type& v): m_value(v)
    {
        ++s_last.valco;++s_total.valco;

        ++s_state.existing;
        m_active = true;
        ++s_state.active;

        update_min_max();
    };
    probe_t(const probe_t& o): m_value(o.m_value)
    {
        ++s_last.copco;++s_total.copco;
        ++s_last.copies;++s_total.copies;

        ++s_state.existing;
        m_active = o.m_active;
        s_state.active += m_active;

        update_min_max();
    };
    probe_t(probe_t&& o) noexcept: m_value(std::move(o.m_value))
    {
        ++s_last.movco;++s_total.movco;
        ++s_last.moves;++s_total.moves;

        ++s_state.existing;
        m_active = o.m_active;
        o.m_active = false;

        update_min_max();
    };
    probe_t& operator=(const probe_t& o)
    {
        ++s_last.copas;++s_total.copas;
        ++s_last.copies;++s_total.copies;

        int activeBefore = m_active + o.m_active;
        m_active = o.m_active;
        int activeAfter = m_active + o.m_active;

        s_state.active += (activeAfter - activeBefore);

        update_min_max();

        m_value = o.m_value;

        return *this;
    }
    probe_t& operator=(probe_t&& o) noexcept
    {
        ++s_last.movas;++s_total.movas;
        ++s_last.moves;++s_total.moves;

        int activeBefore = m_active + o.m_active;
        m_active = o.m_active;
        o.m_active = false;
        int activeAfter = m_active + o.m_active;

        s_state.active += (activeAfter - activeBefore);

        update_min_max();

        m_value = std::move(o.m_value);

        return *this;
    };
    ~probe_t()
    {
        ++s_last.destr;++s_total.destr;

        --s_state.existing;
        s_state.active -= m_active;

        update_min_max();
    };
    static void reset()
    {
        s_state = {};
        s_total = {};
        s_last = {};
    }
    static probe_state state()
    {
        return s_state;
    }
    static probe_counts total()
    {
        return s_total;
    }
    static probe_counts last()
    {
        probe_counts result = s_last;
        s_last = {};
        return result;
    }
    static bool clean()
    {
        return s_state.active == 0 && s_state.existing == 0;
    }
    static std::string report()
    {
        std::string res = "===LAST===\n";
        res += to_string(s_last) + "\n";
        res += "===TOTAL===\n";
        res += to_string(s_total) + "\n";
        res += "===STATE===\n";
        res += to_string(s_state);
        return res;
    }
    private:
    static probe_state s_state;
    static probe_counts s_total;
    static probe_counts s_last;

    bool m_active = true;
    value_type m_value;
    inline void update_min_max()
    {
        update_count_min_max(s_total, s_state.existing, s_state.active);
        update_count_min_max(s_last, s_state.existing, s_state.active);
    }
};

template <typename value_type>
probe_state probe_t<value_type>::s_state = {};
template <typename value_type>
probe_counts probe_t<value_type>::s_total = {};
template <typename value_type>
probe_counts probe_t<value_type>::s_last = {};
}

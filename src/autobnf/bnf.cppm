export module autobnf:bnf;

import std;

export namespace autobnf
{
    struct symbol : std::string
    {
        using std::string::string;

        auto operator()(this auto self, auto rhs)
        {
            return std::vector{std::make_pair(self, std::move(rhs))};
        }
    };


    struct production_rhs : std::vector<symbol>
    {
        using vector::vector;
        production_rhs(symbol s) : vector{std::move(s)} {}

        production_rhs(std::vector<symbol> s) : vector{std::move(s)} {}
    };

    template <typename T>
    concept ProductionRhsLike = std::convertible_to<T, production_rhs>;

    auto operator+(ProductionRhsLike auto lhs, ProductionRhsLike auto rhs) -> production_rhs
    {
        auto lhs_res = production_rhs{std::move(lhs)};
        lhs_res.append_range(production_rhs{std::move(rhs)});
        return lhs_res;
    }


    struct production_rhs_list : std::vector<production_rhs>
    {
        using vector::vector;
        production_rhs_list(symbol s) : production_rhs_list{{s}} {}
        production_rhs_list(production_rhs rhs) : production_rhs_list{rhs} {}
    };

    template <typename T>
    concept ProductionRhsListLike = std::convertible_to<T, production_rhs_list>;

    auto operator|(ProductionRhsListLike auto lhs, ProductionRhsListLike auto rhs) -> production_rhs_list
    {
        auto lhs_res = production_rhs_list{std::move(lhs)};
        lhs_res.append_range(production_rhs_list{std::move(rhs)});
        return lhs_res;
    }


    struct production : std::pair<symbol, production_rhs>
    {
        using pair::pair;
        production(std::pair<symbol, production_rhs> rule) : pair{std::move(rule)} {}
    };

    struct production_list : std::list<production>
    {
        using list::list;
        production_list(production rule) : list{rule} {}
        production_list(std::pair<symbol, production_rhs> rule) : list{rule} {}
    };

    struct production_lhs : symbol
    {
        using symbol::symbol;

        auto operator=(this auto self, production_rhs_list rhs) -> production_list
        {
            production_list rules;
            for (auto&& rule_rhs : rhs) {
                rules.emplace_back(std::move(self), std::move(rule_rhs));
            }
            return rules;
        }
    };


    template <typename T>
    concept ProductionListLike = std::convertible_to<T, production_list>;

    auto operator,(ProductionListLike auto lhs, ProductionListLike auto rhs) -> production_list
    {
        auto lhs_res = production_list{std::move(lhs)};
        lhs_res.append_range(std::move(rhs));
        return lhs_res;
    }

    namespace literals
    {
        auto operator""_sym(const char* name, std::size_t len) -> symbol
        {
            return {name, len};
        }

        auto operator""_rule(const char* name, std::size_t len) -> production_lhs
        {
            return {name, len};
        }
    }
}

template <>
struct std::formatter<autobnf::symbol> : std::formatter<std::string_view>
{
    // template <typename ParseContext>
    // constexpr auto parse(ParseContext& ctx) -> ParseContext::iterator
    // {
    //     auto first = ctx.begin();

    //     if (first != ctx.end() && *first != '}') throw std::format_error("不能为 `autobnf::symbol` 指定格式参数。");

    //     return first;
    // }

    template <typename Context>
    auto format(this const formatter& self, autobnf::symbol s, Context& ctx) -> Context::iterator
    {
        if (s.empty()) return self.std::formatter<std::string_view>::format("null"sv, ctx);
        std::string& base = s;
        return self.formatter<std::string_view>::format(std::format("`{}`", base), ctx);
    }
};

template <>
struct std::formatter<autobnf::production_rhs> : std::formatter<std::string_view>
{
    template <typename Context>
    auto format(this const formatter& self, autobnf::production_rhs s, Context& ctx) -> Context::iterator
    {
        std::string buffer;
        for (auto it = s.begin(); it != s.end(); ++it) {
            std::format_to(std::back_inserter(buffer), "{:{}}{}", "", +(it != s.begin()), *it);
        }
        return self.formatter<std::string_view>::format(buffer, ctx);
    }
};

template <>
struct std::formatter<autobnf::production> : std::formatter<std::string_view>
{
    template <typename Context>
    auto format(this const formatter& self, autobnf::production rule, Context& ctx) -> Context::iterator
    {
        return self.formatter<std::string_view>::format(std::format("{} = {}", rule.first, rule.second), ctx);
    }
};

template <>
struct std::formatter<autobnf::production_list> : std::formatter<std::string_view>
{
    template <typename Context>
    auto format(this const formatter& self, autobnf::production_list list, Context& ctx) -> Context::iterator
    {
        auto buffer = std::string{};

        auto last_lhs = autobnf::symbol{};

        if (list.empty()) return ctx.out();

        auto lhs_fmt = std::string{};
        auto indent = std::string{};
        auto sp = "| "s;

        for (auto it = list.begin(); it != list.end(); ++it) {
            auto eq_last = it->first == last_lhs;
            if (!eq_last) {
                last_lhs = it->first;
                lhs_fmt = std::format("{} = ", it->first);
                indent = std::string(std::formatted_size("{} = ", it->first), ' ');
            }
            if (it != list.begin()) {
                std::format_to(back_inserter(buffer), "\n");
            }
            std::format_to(back_inserter(buffer), "{:>{}s}{}", eq_last ? sp : lhs_fmt, lhs_fmt.size(), it->second);
        }

        return self.formatter<std::string_view>::format(buffer, ctx);
    }
};

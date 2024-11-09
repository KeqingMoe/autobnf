export module autobnf:process;

import std;
import :bnf;

export namespace autobnf
{
    auto order_projection(production_list& list) -> std::map<symbol, std::size_t>
    {
        auto order = std::map<symbol, std::size_t>{};
        for (auto&& [lhs, rhs] : list) {
            if (auto it = order.lower_bound(lhs); it == order.end() || it->first != lhs) {
                order.emplace_hint(it, lhs, order.size());
            }
        }
        return order;
    }

    auto sort(production_list& list, const std::map<autobnf::symbol, std::size_t>& order) -> void
    {
        list.sort([&order](auto&& lhs, auto&& rhs) {
            return order.at(lhs.first) < order.at(rhs.first);
        });
    }

    template <typename Callable>
    concept NonterminalSymbolTransformer = requires(Callable f, symbol s) {
        { f(s) } -> std::convertible_to<symbol>;
    };

    inline constexpr auto default_nonterminal_symbol_transformer = [](auto sym) static {
        sym += '_';
        return sym;
    };

    auto elide_direct_left_recursion(const symbol& lhs, const symbol& new_lhs, std::list<production_rhs>& list)
    {
        auto new_p = std::list<production_rhs>{};
        for (auto it = list.begin(); it != list.end();) {
            auto nit = std::next(it);
            auto&& rhs = *it;
            if (rhs.empty()) continue;
            if (lhs == rhs.front()) {
                rhs.front() = new_lhs;
                std::ranges::rotate(rhs, std::next(rhs.begin()));
                new_p.splice(new_p.end(), list, it);
            } else {
                rhs.emplace_back(new_lhs);
            }
            it = nit;
        }
        new_p.emplace_back(symbol{});
        return new_p;
    }

    auto rewrite_rules(std::list<production_rhs>& rs_i, const symbol& p_j, const std::list<production_rhs>& rs_j)
    {
        for (auto it = rs_i.begin(); it != rs_i.end();) {
            auto&& r_i = *it;
            if (r_i.front() != p_j) {
                ++it;
                continue;
            }
            for (auto r_j : rs_j) {
                r_j.append_range(r_i | std::views::drop(1));
                rs_i.insert(it, std::move(r_j));
            }
            it = rs_i.erase(it);
        }
    }

    auto elide_left_recursion(production_list list, NonterminalSymbolTransformer auto&& transformer)
    {
        auto g0 = list | std::views::as_rvalue | std::ranges::to<std::multimap>();
        auto ntsyms = g0 | std::views::keys | std::ranges::to<std::set>();
        auto g = std::map<symbol, std::list<production_rhs>>{};
        for (auto&& ntsym : ntsyms) {
            auto [first, last] = g0.equal_range(ntsym);
            auto values = std::ranges::subrange{first, last} | std::views::values;
            g.emplace_hint(g.end(), ntsym, values | std::views::as_rvalue | std::ranges::to<std::list>());
        }
        auto transformed = std::set<symbol>{};
        auto delta = decltype(g){};
        for (auto i = g.begin(); i != g.end(); ++i) {
            auto&& [p_i, rs_i] = *i;
            for (auto&& [p_j, rs_j] : std::ranges::subrange{g.begin(), i}) {
                rewrite_rules(rs_i, p_j, rs_j);
            }
            auto flag = std::ranges::any_of(rs_i, [&p_i](auto&& r) {
                return r.front() == p_i;
            });
            if (flag) {
                auto new_p_lhs = symbol{transformer(p_i)};
                auto new_p_rhs = elide_direct_left_recursion(p_i, new_p_lhs, rs_i);
                delta.emplace_hint(delta.end(), std::move(new_p_lhs), std::move(new_p_rhs));
            }
        }
        g.merge(delta);
        static constexpr auto tf = [](auto&& kv) static {
            auto&& [key, values] = kv;
            auto keys = std::views::repeat(key);
            return std::views::zip(keys, values | std::views::as_rvalue);
        };
        return g | std::views::transform(tf) | std::views::join | std::ranges::to<production_list>();
    }

    auto elide_left_recursion(production_list list)
    {
        return elide_left_recursion(std::move(list), default_nonterminal_symbol_transformer);
    }
};

export module autobnf:sets;

import std;
import :bnf;
import :base;

export namespace autobnf
{
    using symbol_set = std::set<symbol>;

    struct first_set
    {
    public:
        first_set(const production_list& g)
        {
            first_[symbol::null].emplace(symbol::null);
            auto casted_g = g | stdv::transform(&production::to_pair);
            auto ntsyms = casted_g | stdv::keys | stdr::to<symbol_set>();
            auto is_tsym = [&ntsyms](auto&& x) {
                return !ntsyms.contains(x);
            };
            for (auto&& rhs : casted_g | stdv::values | stdv::join | stdv::filter(is_tsym)) {
                first_[rhs].emplace(rhs);
            }

            for (auto changing = true; changing;) {
                changing = false;
                for (auto&& [lhs, rhs] : g) {
                    auto syms = operator[](rhs);
                    auto siz = syms.size();
                    first_[lhs].merge(syms);
                    changing |= siz != syms.size();
                }
            }
        }

        auto at(const symbol& sym) const -> const symbol_set&
        {
            return first_.at(sym);
        }

        auto operator[](const symbol& sym) -> const symbol_set&
        {
            return first_[sym];
        }

        auto operator[](stdr::bidirectional_range auto&& syms) const -> symbol_set
        {
            auto at_ = [this](auto&& x) {
                auto it = first_.find(x);
                if (it == first_.end()) return symbol_set{};
                return it->second;
            };
            auto it = stdr::find_if_not(syms, &symbol::is_null);
            auto result = symbol_set{};
            for (auto&& sym : stdr::subrange(syms.begin(), it)) result.merge(at_(sym));
            if (it != syms.end()) {
                result.merge(at_(*it));
                result.erase(symbol::null);
            }
            return result;
        }

    private:
        std::map<symbol, symbol_set> first_;
    };

    struct follow_set
    {
    public:
        follow_set(const production_list& g, const first_set& first)
        {
            auto ntsyms = g | stdv::transform(&production::to_pair) | stdv::keys | stdr::to<symbol_set>();

            for (auto changing = true; changing;) {
                changing = false;
                for (auto&& [lhs, rhs] : g) {
                    auto tailer = follow_[lhs];
                    for (auto&& sym : rhs | stdv::reverse) {
                        auto fi = first.at(sym);
                        std::swap(tailer, fi);
                        if (!ntsyms.contains(sym)) continue;

                        auto&& fo = follow_[sym];
                        auto siz = fo.size();
                        fo.insert_range(fi);
                        if (siz != fo.size()) changing = true;

                        if (!tailer.contains(symbol::null)) continue;
                        tailer.merge(fi);
                        tailer.erase(symbol::null);
                    }
                }
            }
        }

        auto operator[](const symbol& sym) -> const symbol_set&
        {
            return follow_[sym];
        }

        auto at(const symbol& sym) const -> const symbol_set&
        {
            return follow_.at(sym);
        }

    private:
        std::map<symbol, symbol_set> follow_;
    };

    struct select_set
    {
    public:
        select_set(const production_list& g, const first_set& first, const follow_set& follow)
        {
            for (auto&& p : g) {
                auto&& select_p = select_[p];
                auto&& [lhs, rhs] = p;
                select_p = first[rhs];
                if (select_p.contains(symbol::null)) select_p.insert_range(follow.at(lhs));
            }
        }

        auto operator[](const production& p) -> const symbol_set&
        {
            return select_[p];
        }

        auto at(const production& p) const -> const symbol_set&
        {
            return select_.at(p);
        }

    private:
        std::map<production, symbol_set> select_;
    };
}

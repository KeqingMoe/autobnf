#include <algorithm>
#include <format>
import std;
import autobnf;


auto format_symbol_order(auto&& symbol_order)
{
    auto result = std::string{};
    for (auto space = 0;
         auto&& symbol : std::map{std::from_range, std::views::zip(symbol_order | std::views::values, symbol_order | std::views::keys)}
                             | std::views::values /*| std::views::enumerate*/) {
        std::format_to(std::back_inserter(result), "{:>{}}", symbol, symbol.size() + 2 + space);
        if (space == 0) {
            space = 1;
        }
    }
    return result;
}

int main()
{
    using namespace std::literals;
    using namespace autobnf::literals;

    auto g0 =
        ("Goal"_rule = "Expr"_sym,
         "Expr"_rule = "Expr"_sym + "+"_sym + "Term"_sym | "Expr"_sym + "-"_sym + "Term"_sym | "Term"_sym,
         "Term"_rule = "Term"_sym + "*"_sym + "Factor"_sym | "Term"_sym + "/"_sym + "Factor"_sym | "Factor"_sym,
         "Factor"_rule = "("_sym + "Expr"_sym + ")"_sym | "num"_sym | "name"_sym);

    // auto g0 = ("R"_rule = "S"_sym + "a"_sym | "a"_sym, "Q"_rule = "R"_sym + "b"_sym | "b"_sym, "S"_rule = "Q"_sym + "c"_sym | "c"_sym);


    std::println("{}", g0);
    std::println("==============================");

    auto g1 = autobnf::elide_left_recursion(std::move(g0));
    std::println("{}", g1);
    std::println("==============================");

    auto syms = autobnf::symbol_set{};
    for (auto&& [lhs, rhs] : g1) {
        syms.emplace(lhs);
        syms.insert_range(rhs);
    }
    auto ntsyms = g1 | std::views::transform(&autobnf::production::to_pair) | std::views::keys | std::ranges::to<autobnf::symbol_set>();

    std::println("symbols: {::}", syms);
    std::println("nonterminal symbols: {::}", ntsyms);
    std::println("==============================");

    auto first = autobnf::first_set{g1};
    for (auto&& sym : syms) {
        std::println("FIRST({}) = {::}", sym, first[sym]);
    }
    std::println("==============================");

    auto follow = autobnf::follow_set{g1, first};
    for (auto&& sym : ntsyms) {
        std::println("FOLLOW({}) = {::}", sym, follow[sym]);
    }
    std::println("==============================");

    auto estimate_width = [](auto&& x) {
        return std::formatted_size("{}", x);
    };
    auto width = std::ranges::max(g1 | std::views::transform(estimate_width));
    auto select = autobnf::select_set{g1, first, follow};
    for (auto last = autobnf::symbol::null; auto&& p : g1) {
        std::println("{:{}:{}} => SELECT(~) = {::}", p, last == p.first, width, select[p]);
        last = p.first;
    }
}

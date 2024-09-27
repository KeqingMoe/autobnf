#include <print>
#include <algorithm>
#include <concepts>
#include <map>

import autobnf;

int main()
{
    using namespace std::literals;
    using namespace autobnf::literals;

    auto g=(
        "Goal"_rule="Expr"_sym,
        "Expr"_rule="Expr"_sym+"+"_sym+"Term"_sym
                   |"Expr"_sym+"-"_sym+"Term"_sym
                   |"Term"_sym,
        "Term"_rule="Term"_sym+"*"_sym+"Factor"_sym
                   |"Term"_sym+"/"_sym+"Factor"_sym
                   |"Factor"_sym,
        "Factor"_rule="("_sym+"Expr"_sym+")"_sym
                     |"num"_sym
                     |"name"_sym
    );

    autobnf::elide_left_recursion(g);

    auto order = std::map<autobnf::symbol, std::size_t>{};
    for (auto&& s : {"Goal"_sym, "Expr"_sym, "Term"_sym, "Factor"_sym}) {
        order.emplace(s, order.size());
        order.emplace(autobnf::default_terminal_symbol_transformer(s), order.size());
    }
    std::ranges::sort(g, {}, [&order](auto&& x) {
        return order.at(x.first);
    });

    std::println("{}", g);
}

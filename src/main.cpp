import std;
import autobnf;

auto dbg(auto...)
{
    std::println("{}", std::source_location::current().function_name());
}

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
}

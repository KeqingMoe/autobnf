#include <print>

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

    std::println("{}", g);
}

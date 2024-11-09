# autobnf

它可以分析 BNF 表示的文法，并从其生成 `FIRST` `FOLLOW` `SELECT` 集合，从而帮助手动编写递归下降语法分析器。

## 使用方式

您只需要打开 `src/main.cpp` 并对其进行编辑。

### 自定义文法

修改其中的这一部分：

```cpp
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
```

用户自定义字面量后缀 `_sym` ( `autobnf::symbol` ) 表示一个终结符或非终结符， `_rule` ( `autobnf::production_lhs` ) 表示开始书写一系列产生式（它是产生式的左侧部分）。

使用 `+` 连接非终结符得到一条产生式的右侧部分（ `autobnf::production_rhs` ），使用 `|` 连接多个这种右侧部分（ `autobnf::production_rhs_list` ），再通过 `=` 将其与左侧部分相连，得到一组产生式（ `autobnf::production_list` ）。

### 消除左递归

```cpp
autobnf::elide_left_recursion(g);
```

这会消除文法中的所有左递归（包括直接左递归和间接左递归），并产生一些新的非终结符，例如从 `Expr` 产生出 `Expr_` 。产生新非终结符的规则默认是：

```cpp
constexpr auto default_nonterminal_symbol_transformer = [](auto sym) static {
    sym += '_';
    return sym;
};
```

您也可以自己实现一个转换器，不过要保证符合下面这个概念：

```cpp
template <typename Callable>
concept NonterminalSymbolTransformer = requires(Callable f, symbol s) {
    { f(s) } -> std::convertible_to<symbol>;
};
```

然后将您自己的转换器作为 `autobnf::elide_left_recursion` 的第二参数：

```cpp
autobnf::elide_left_recursion(g, [](auto sym) static {
    sym += '\'';
    return sym;
});
```

目前，它暂时不会检查生成的新非终结符是否重复。给定非终结符集合 NT 和非终结符转换器 T，对 ∀s∈NT，若 T(s)∈NT ，则行为未定义。

### 提取左因子以

开发中。

### 生成 `FIRST` `FOLLOW` `SELECT` 集合

开发中。

### 自动生成递归下降语法分析器

开发中。

## 构建

使用 `xmake` 与 C++ Modules 的构建方案。

先切换至 clang 工具链，确保您的 clang 足够新（我使用了 clang 19.1.3）。

```shell
xmake f --toolchain=clang
```

构建：

```shell
xmake
```

运行：

```shell
xmake run
```

若您在构建时，出现类似这样的错误，请您重启 clangd 的语言服务器：

```plaintext
error: error: unable to open output file 'build\.gens\autobnf\windows\x64\release\rules\bmi\cache\modules\4c325ac5\autobnf-bnf.pcm': 'Permission denied'
1 error generated.
  > in src\autobnf\bnf.mpp
```

因为 xmake 存在一些 bug，所以如果遇到任何构建失败的问题（尤其是与 C++ Modules 相关的），您可以先试着清除构建产物、缓存，甚至是清除该项目下任何 xmake 生成的文件。

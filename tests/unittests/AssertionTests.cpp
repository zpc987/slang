#include "Test.h"

TEST_CASE("Named sequences") {
    auto tree = SyntaxTree::fromText(R"(
module m;
    sequence seq(x, y, min, max, delay1);
        x ##delay1 y[*min:max];
    endsequence
endmodule
)");

    Compilation compilation;
    compilation.addSyntaxTree(tree);
    NO_COMPILATION_ERRORS;
}

TEST_CASE("Named properties") {
    auto tree = SyntaxTree::fromText(R"(
module m;
    property p1(x,y);
        ##1 x |-> y;
    endproperty

    property p2;
        @(posedge clk)
        a ##1 (b || c)[->1] |->
            if (b)
                p1(d,e)
            else
                f;
    endproperty
endmodule
)");

    Compilation compilation;
    compilation.addSyntaxTree(tree);
    NO_COMPILATION_ERRORS;
}

TEST_CASE("Concurrent assertion expressions") {
    auto tree = SyntaxTree::fromText(R"(
module m;
    string a;
    int b,c,d,e;

    foo: assert property (a);
    assert property (a ##1 b ##[+] c ##[*] d ##[1:5] e);
    assert property (##0 a[*0:4] ##0 b[=4] ##0 c[->1:2] ##0 c[*] ##1 d[+]);
    assert property (##[0:$] a[*0:$]);
endmodule
)");

    Compilation compilation;
    compilation.addSyntaxTree(tree);
    NO_COMPILATION_ERRORS;
}

TEST_CASE("Concurrent assertion expression errors") {
    auto tree = SyntaxTree::fromText(R"(
class C;
    int bar;
endclass

function int func(output o);
endfunction

module m;
    int a[];
    int b;
    chandle c;
    C d = new;
    logic o;
    localparam real p = 3.14;

    assert property (a ##1 b++);
    assert property (c == null);
    assert property (d.bar);
    assert property (func(o));
    assert property (b[* -1:4]);
    assert property (b ##[4:1] b);
    assert property (b ##p b);
    assert property (##0 b[*3-:4] ##0 b[=]);
    assert property (##[] b ##[1+:5] b);
endmodule
)");

    Compilation compilation;
    compilation.addSyntaxTree(tree);

    auto& diags = compilation.getAllDiagnostics();
    REQUIRE(diags.size() == 12);
    CHECK(diags[0].code == diag::AssertionExprType);
    CHECK(diags[1].code == diag::IncDecNotAllowed);
    CHECK(diags[2].code == diag::CHandleInAssertion);
    CHECK(diags[3].code == diag::ClassMemberInAssertion);
    CHECK(diags[4].code == diag::AssertionFuncArg);
    CHECK(diags[5].code == diag::ValueMustBePositive);
    CHECK(diags[6].code == diag::SeqRangeMinMax);
    CHECK(diags[7].code == diag::ValueMustBeIntegral);
    CHECK(diags[8].code == diag::InvalidSequenceRange);
    CHECK(diags[9].code == diag::ExpectedExpression);
    CHECK(diags[10].code == diag::ExpectedExpression);
    CHECK(diags[11].code == diag::InvalidSequenceRange);
}
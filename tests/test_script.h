#ifndef TEST_SCRIPT_H
#define TEST_SCRIPT_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "script/astwalker.h"

TEST(Script, EvaluateExpression)
{
    std::string script = "x = 1 + 2 * 3 + 4 * (5 + 6)";

    tw::ASTWalker tw;
    tw.run(script);

    const tw::Parameter *param = tw.getParameter("x");

    ASSERT_NE(param, nullptr);
    ASSERT_EQ(param->type(), tw::Int);
    EXPECT_EQ(param->asInt(), 51);
}

TEST(Script, IfElseStatemnent)
{
    std::string script =
            "pred = \"A\" == \"A\"\n"
            "if pred:\n"
            "    result=1\n"
            "else:\n"
            "    result=2";

    tw::ASTWalker tw;
    tw.run(script);

    const tw::Parameter *param = tw.getParameter("result");

    ASSERT_NE(param, nullptr);
    ASSERT_EQ(param->type(), tw::Int);
    EXPECT_EQ(param->asInt(), 1);

    script =
            "pred = \"A\" == \"B\"\n"
            "if pred:\n"
            "    result=1\n"
            "else:\n"
            "    result=2";

    tw.run(script);

    param = tw.getParameter("result");

    ASSERT_NE(param, nullptr);
    ASSERT_EQ(param->type(), tw::Int);
    EXPECT_EQ(param->asInt(), 2);
}

#endif // TEST_SCRIPT_H

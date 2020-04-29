/*
 * pullparser.cpp
 *
 *  Created on: 30 апр. 2020 г.
 *      Author: sadko
 */


#include <lsp-plug.in/test-fw/utest.h>
#include <lsp-plug.in/test-fw/helpers.h>
#include <lsp-plug.in/fmt/config/PullParser.h>
#include <stdarg.h>
#include <math.h>

namespace lsp
{
    using namespace lsp::config;
}

UTEST_BEGIN("runtime.fmt.config", pullparser)

    void test_valid_cases()
    {
        static const char *text =
            "# comment\n"
            "\n\r"
            "key1=value1\n"
            "key2=123# comment\n"
            "key3 = 123.456\n"
            "key4 = 12.5db \n"
            "#comment\n"
            "key5=i32:123\n"
            "key6=f32:\"+12.5 dB\"\n"
            "key7=-inf\n"
            "key8=  \"+inf\"\n"
            "\n"
            "key9=blob:\"12345\"\n"
            "key10=blob:\"text/plain:123456\"\n"
            "key11=  string with spaces   # comment\n"
            "key12 = \"string \\\"with\\\" escapes\" \n"
            "key13 = string \\\"with other\\\" escapes \n"
            "key14 = string \\# not comment \n"
            "/tree/arg1 = str:\"value\" \n";

        PullParser p;
        param_t xp;
        const param_t *pp;

        // Wrap the string
        UTEST_ASSERT((pp = p.current()) == NULL);
        UTEST_ASSERT(p.wrap(text) == STATUS_OK);
        UTEST_ASSERT((pp = p.current()) == NULL);

        // key1
        UTEST_ASSERT(p.next() == STATUS_OK);
        UTEST_ASSERT((pp = p.current()) != NULL);
        UTEST_ASSERT(pp->name.equals_ascii("key1"));
        UTEST_ASSERT(pp->flags == SF_TYPE_STR);
        UTEST_ASSERT(::strcmp(pp->str, "value1") == 0);

        // key2
        UTEST_ASSERT(p.next() == STATUS_OK);
        UTEST_ASSERT((pp = p.current()) != NULL);
        UTEST_ASSERT(pp->name.equals_ascii("key2"));
        UTEST_ASSERT(pp->flags == SF_TYPE_I32);
        UTEST_ASSERT(pp->i32 == 123);

        // key3
        UTEST_ASSERT(p.next() == STATUS_OK);
        UTEST_ASSERT((pp = p.current()) != NULL);
        UTEST_ASSERT(pp->name.equals_ascii("key3"));
        UTEST_ASSERT(pp->flags == SF_TYPE_F32);
        UTEST_ASSERT(float_equals_relative(pp->f32, 123.456, 0.0001));

        // key4
        UTEST_ASSERT(p.next() == STATUS_OK);
        UTEST_ASSERT((pp = p.current()) != NULL);
        UTEST_ASSERT(pp->name.equals_ascii("key4"));
        UTEST_ASSERT(pp->flags == (SF_TYPE_F32 | SF_DECIBELS));
        UTEST_ASSERT(float_equals_relative(pp->f32, 4.21696, 1e-5)); // 12.5 dB

        // key5
        UTEST_ASSERT(p.next() == STATUS_OK);
        UTEST_ASSERT((pp = p.current()) != NULL);
        UTEST_ASSERT(pp->name.equals_ascii("key5"));
        UTEST_ASSERT(pp->flags == (SF_TYPE_I32 | SF_TYPE_SET));
        UTEST_ASSERT(pp->i32 == 123);

        // key6
        UTEST_ASSERT(p.next() == STATUS_OK);
        UTEST_ASSERT((pp = p.current()) != NULL);
        UTEST_ASSERT(pp->name.equals_ascii("key6"));
        UTEST_ASSERT(pp->flags == (SF_TYPE_F32 | SF_TYPE_SET | SF_QUOTED | SF_DECIBELS));
        UTEST_ASSERT(float_equals_relative(pp->f32, 4.21696, 1e-5)); // 12.5 dB

        // key7
        UTEST_ASSERT(p.next(&xp) == STATUS_OK);
        UTEST_ASSERT(xp.name.equals_ascii("key7"));
        UTEST_ASSERT(xp.flags == SF_TYPE_F32);
        UTEST_ASSERT(::isinf(xp.f32) && (xp.f32 < 0)); // -inf

        // key8
        UTEST_ASSERT(p.next(&xp) == STATUS_OK);
        UTEST_ASSERT(xp.name.equals_ascii("key8"));
        UTEST_ASSERT(xp.flags == (SF_TYPE_STR | SF_QUOTED));
        UTEST_ASSERT(::strcmp(xp.str, "+inf") == 0); // +inf

        // key9
        UTEST_ASSERT(p.next() == STATUS_OK);
        UTEST_ASSERT((pp = p.current()) != NULL);
        UTEST_ASSERT(pp->name.equals_ascii("key9"));
        UTEST_ASSERT(pp->flags == (SF_TYPE_BLOB | SF_TYPE_SET | SF_QUOTED));
        UTEST_ASSERT(pp->blob.length == 5);
        UTEST_ASSERT(pp->blob.ctype == NULL);
        UTEST_ASSERT(::strcmp(pp->blob.data, "12345") == 0);

        // key10
        UTEST_ASSERT(p.next() == STATUS_OK);
        UTEST_ASSERT((pp = p.current()) != NULL);
        UTEST_ASSERT(pp->name.equals_ascii("key10"));
        UTEST_ASSERT(pp->flags == (SF_TYPE_BLOB | SF_TYPE_SET | SF_QUOTED));
        UTEST_ASSERT(pp->blob.length == 6);
        UTEST_ASSERT(::strcmp(pp->blob.ctype, "text/plain") == 0);
        UTEST_ASSERT(::strcmp(pp->blob.data, "123456") == 0);

        // key11
        UTEST_ASSERT(p.next() == STATUS_OK);
        UTEST_ASSERT((pp = p.current()) != NULL);
        UTEST_ASSERT(pp->name.equals_ascii("key11"));
        UTEST_ASSERT(pp->flags == SF_TYPE_STR);
        UTEST_ASSERT(::strcmp(pp->str, "string with spaces") == 0);

        // key12
        UTEST_ASSERT(p.next() == STATUS_OK);
        UTEST_ASSERT((pp = p.current()) != NULL);
        UTEST_ASSERT(pp->name.equals_ascii("key12"));
        UTEST_ASSERT(pp->flags == (SF_TYPE_STR | SF_QUOTED));
        UTEST_ASSERT(::strcmp(pp->str, "string \"with\" escapes") == 0);

        // key13
        UTEST_ASSERT(p.next() == STATUS_OK);
        UTEST_ASSERT((pp = p.current()) != NULL);
        UTEST_ASSERT(pp->name.equals_ascii("key13"));
        UTEST_ASSERT(pp->flags == SF_TYPE_STR);
        UTEST_ASSERT(::strcmp(pp->str, "string \"with other\" escapes") == 0);

        // key13
        UTEST_ASSERT(p.next() == STATUS_OK);
        UTEST_ASSERT((pp = p.current()) != NULL);
        UTEST_ASSERT(pp->name.equals_ascii("key14"));
        UTEST_ASSERT(pp->flags == SF_TYPE_STR);
        UTEST_ASSERT(::strcmp(pp->str, "string # not comment") == 0);

        // /tree/arg1
        UTEST_ASSERT(p.next() == STATUS_OK);
        UTEST_ASSERT((pp = p.current()) != NULL);
        UTEST_ASSERT(pp->name.equals_ascii("/tree/arg1"));
        UTEST_ASSERT(pp->flags == (SF_TYPE_STR | SF_TYPE_SET | SF_QUOTED));
        UTEST_ASSERT(::strcmp(pp->str, "value") == 0);

        // eof
        UTEST_ASSERT(p.next() == STATUS_EOF);

        UTEST_ASSERT(p.close() == STATUS_OK);
    }

    UTEST_MAIN
    {
        printf("Testing valid cases...\n");
        test_valid_cases();
    }

UTEST_END

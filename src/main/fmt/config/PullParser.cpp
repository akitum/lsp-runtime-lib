/*
 * PullParser.cpp
 *
 *  Created on: 29 апр. 2020 г.
 *      Author: sadko
 */

#include <lsp-plug.in/fmt/config/PullParser.h>
#include <lsp-plug.in/io/InFileStream.h>
#include <lsp-plug.in/io/InStringSequence.h>
#include <lsp-plug.in/io/InSequence.h>
#include <lsp-plug.in/stdlib/math.h>
#include <locale.h>
#include <errno.h>

namespace lsp
{
    namespace config
    {
        typedef struct type_t
        {
            const char *prefix;
            size_t flags;
        } type_t;

        static const type_t all_types[] =
        {
            { "i32:", SF_TYPE_I32 },
            { "u32:", SF_TYPE_U32 },
            { "f32:", SF_TYPE_F32 },
            { "i64:", SF_TYPE_I64 },
            { "u64:", SF_TYPE_U64 },
            { "f64:", SF_TYPE_F64 },
            { "str:", SF_TYPE_STR },
            { "blob:", SF_TYPE_BLOB },
            { NULL, 0 }
        };
        
        PullParser::PullParser()
        {
            pIn         = NULL;
            nWFlags     = 0;
            nFlags      = 0;
        }
        
        PullParser::~PullParser()
        {
            close();
        }

        status_t PullParser::open(const char *path, const char *charset)
        {
            if (pIn != NULL)
                return STATUS_BAD_STATE;
            else if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            io::InFileStream *ifs = new io::InFileStream();
            if (ifs == NULL)
                return STATUS_NO_MEM;
            status_t res = ifs->open(path);
            if (res == STATUS_OK)
            {
                res     = wrap(ifs, WRAP_CLOSE | WRAP_DELETE, charset);
                if (res == STATUS_OK)
                    return res;
                ifs->close();
            }
            delete ifs;

            return res;
        }

        status_t PullParser::open(const LSPString *path, const char *charset)
        {
            if (pIn != NULL)
                return STATUS_BAD_STATE;
            else if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            io::InFileStream *ifs = new io::InFileStream();
            if (ifs == NULL)
                return STATUS_NO_MEM;
            status_t res = ifs->open(path);
            if (res == STATUS_OK)
            {
                res     = wrap(ifs, WRAP_CLOSE | WRAP_DELETE, charset);
                if (res == STATUS_OK)
                    return res;
                ifs->close();
            }
            delete ifs;

            return res;
        }

        status_t PullParser::open(const io::Path *path, const char *charset)
        {
            if (pIn != NULL)
                return STATUS_BAD_STATE;
            else if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            io::InFileStream *ifs = new io::InFileStream();
            if (ifs == NULL)
                return STATUS_NO_MEM;
            status_t res = ifs->open(path);
            if (res == STATUS_OK)
            {
                res     = wrap(ifs, WRAP_CLOSE | WRAP_DELETE, charset);
                if (res == STATUS_OK)
                    return res;
                ifs->close();
            }
            delete ifs;

            return res;
        }

        status_t PullParser::wrap(const char *str, const char *charset)
        {
            if (pIn != NULL)
                return STATUS_BAD_STATE;
            else if (str == NULL)
                return STATUS_BAD_ARGUMENTS;

            io::InStringSequence *seq = new io::InStringSequence();
            if (seq == NULL)
                return STATUS_NO_MEM;

            status_t res = seq->wrap(str, charset);
            if (res == STATUS_OK)
            {
                if ((res = wrap(seq, WRAP_CLOSE | WRAP_DELETE)) == STATUS_OK)
                    return res;
                seq->close();
            }

            delete seq;
            return res;
        }

        status_t PullParser::wrap(const LSPString *str)
        {
            if (pIn != NULL)
                return STATUS_BAD_STATE;
            else if (str == NULL)
                return STATUS_BAD_ARGUMENTS;

            io::InStringSequence *seq = new io::InStringSequence();
            if (seq == NULL)
                return STATUS_NO_MEM;

            status_t res = seq->wrap(str);
            if (res == STATUS_OK)
            {
                if ((res = wrap(seq, WRAP_CLOSE | WRAP_DELETE)) == STATUS_OK)
                    return res;
                seq->close();
            }

            delete seq;
            return res;
        }

        status_t PullParser::wrap(io::IInStream *is, size_t flags, const char *charset)
        {
            if (pIn != NULL)
                return STATUS_BAD_STATE;
            else if (is == NULL)
                return STATUS_BAD_ARGUMENTS;

            io::InSequence *seq = new io::InSequence();
            if (seq == NULL)
                return STATUS_NO_MEM;

            status_t res = seq->wrap(is, flags, charset);
            if (res == STATUS_OK)
            {
                if ((res = wrap(seq, WRAP_CLOSE | WRAP_DELETE)) == STATUS_OK)
                    return res;
                seq->close();
            }

            delete seq;
            return res;
        }

        status_t PullParser::wrap(io::IInSequence *seq, size_t flags)
        {
            if (pIn != NULL)
                return STATUS_BAD_STATE;
            else if (seq == NULL)
                return STATUS_BAD_ARGUMENTS;

            pIn             = seq;
            nWFlags         = flags;

            return STATUS_OK;
        }

        status_t PullParser::close()
        {
            status_t res = STATUS_OK;

            // Release input sequence
            if (pIn != NULL)
            {
                if (nWFlags & WRAP_CLOSE)
                {
                    if (res == STATUS_OK)
                        res = pIn->close();
                    else
                        pIn->close();
                }

                if (nWFlags & WRAP_DELETE)
                    delete pIn;

                pIn     = NULL;
            }

            return res;
        }

        status_t PullParser::next(param_t *param)
        {
            status_t result = STATUS_OK;

            while (true)
            {
                // Clear buffer
                sLine.clear();
                sKey.clear();
                sValue.clear();

                // Read line from file
                result      = pIn->read_line(&sLine, true);
                if (result != STATUS_OK)
                {
                    if (result == STATUS_EOF)
                        result = STATUS_OK;
                    break;
                }

                // Parse the line
                result = parse_line();
                if (result == STATUS_OK)
                {
                    result  = commit_param();
                    break;
                }
                else if (result != STATUS_SKIP)
                    break;
            }

            return result;
        }

        const param_t *PullParser::current() const
        {
            return ((sParam.flags & SF_TYPE_MASK) != SF_NONE) ? &sParam : NULL;
        }

        status_t PullParser::current(param_t *ev) const
        {
            if (pIn == NULL)
                return STATUS_CLOSED;

            if ((sParam.flags & SF_TYPE_MASK) == SF_NONE)
                return STATUS_NO_DATA;

            if ((ev != NULL) && (!ev->copy(&sParam)))
                return STATUS_NO_MEM;

            return STATUS_OK;
        }

        bool PullParser::skip_spaces(size_t &off)
        {
            size_t len      = sLine.length();

            while (off < len)
            {
                lsp_wchar_t ch = sLine.at(off);
                if ((ch != ' ') && (ch != '\t'))
                    break;
                off ++;
            }

            return ((off >= len) || (sLine.at(off) == '#')); // Return true if line is void
        }

        status_t PullParser::read_key(size_t &off)
        {
            size_t len      = sLine.length();

            while (off < len)
            {
                lsp_wchar_t ch = sLine.at(off++);

                if ((ch == ' ') || (ch == '\t'))
                    return (sKey.is_empty()) ? STATUS_BAD_FORMAT : STATUS_OK;
                else if (ch == '=') // Assignment occurred?
                {
                    if (sKey.is_empty())
                        return STATUS_BAD_FORMAT;
                    --off;
                    break;
                }
                else if (ch == '#')
                {
                    --off;
                    break;
                }
                else if (((ch >= 'a') && (ch <= 'z')) ||
                        ((ch >= 'A') && (ch <= 'Z')) ||
                        ((ch >= '0') && (ch <= '9')) ||
                        (ch == '_') || (ch == '/'))
                {
                    if (!sKey.append(ch))
                        return STATUS_NO_MEM;
                }
                else
                    return STATUS_BAD_FORMAT;
            }

            // Validate that key should start with '/' or not contain any '/' character
            if (sKey.index_of('/') > 0)
                return STATUS_BAD_FORMAT;

            return STATUS_OK;
        }

        status_t PullParser::read_type(size_t &off)
        {
            size_t len      = sLine.length();
            // Skip spaces
            for (const type_t *p = all_types; p->prefix != NULL; ++p)
            {
                if (sLine.contains_at_ascii(off, p->prefix))
                {
                    nFlags     |= p->flags | SF_TYPE_SET;
                    off        += ::strlen(p->prefix);
                    return STATUS_OK;
                }
            }

            return STATUS_OK;
        }

        status_t PullParser::read_value(size_t &off)
        {
            size_t len      = sLine.length();
            ssize_t trim    = -1;

            // Check that value is quoted
            if (sLine.first() == '"')
            {
                nFlags         |= SF_QUOTED;
                ++off;
            }

            // Check that type is present
            while (off < len)
            {
                lsp_wchar_t ch = sLine.at(off++);

                switch (ch)
                {
                    // Escape sequence
                    case '\\':
                    {
                        if (off >= len)
                            return (sLine.append(ch)) ? STATUS_OK : STATUS_NO_MEM;

                        ch = sLine.at(off++);
                        lsp_swchar_t escape = -1;

                        switch (ch)
                        {
                            case 'n': escape = '\n'; break;
                            case 'r': escape = '\r'; break;
                            case 't': escape = '\t'; break;
                            case '\\': escape = '\\'; break;
                            case '\"': escape = '\"'; break;
                            default:
                                escape      = '\\';
                                --off;
                                break;
                        }
                        if (!sLine.append(escape))
                            return STATUS_NO_MEM;
                        break;
                    }

                    // Spaces
                    case ' ':
                    case '\t':
                        if (nFlags & SF_QUOTED)
                        {
                            if (!sValue.append(ch))
                                return STATUS_NO_MEM;
                        }
                        else if (trim < 0)
                            trim    = sValue.length();
                        break;

                    // Quotes
                    case '\"':
                        if (!(nFlags & SF_QUOTED))
                            return STATUS_BAD_FORMAT;
                        return (skip_spaces(off)) ? STATUS_OK : STATUS_BAD_FORMAT;

                    // End of line
                    case '\n':
                        if (nFlags & SF_QUOTED)
                            return STATUS_BAD_FORMAT;
                        return STATUS_OK;

                    // Other characters
                    default:
                        if (!sValue.append(ch))
                            return STATUS_NO_MEM;
                        trim    = -1;
                        break;
                }
            }

            if (nFlags & SF_QUOTED)
                return STATUS_BAD_FORMAT;
            else if (trim >= 0)
                sValue.set_length(trim);

            return STATUS_OK;
        }

        status_t PullParser::parse_line()
        {
            sKey.clear();
            sValue.clear();
            nFlags          = 0;

            // Empty line?
            size_t off=0, len = sLine.length();
            if (skip_spaces(off))
                return STATUS_SKIP;

            // Fetch the key value
            status_t res    = read_key(off);
            if (res != STATUS_OK) // Error while parsing line
                return res;
            if (skip_spaces(off))
                return (sKey.is_empty()) ? STATUS_SKIP : STATUS_OK;

            // Analyze character
            lsp_wchar_t ch = sLine.at(off);
            if ((sKey.is_empty()) || (ch != '='))
                return STATUS_BAD_FORMAT;

            // Fetch the value type
            if (skip_spaces(off))
                return STATUS_BAD_FORMAT;
            res             = read_type(off);
            if (res != STATUS_OK)
                return res;

            // Fetch the value's value
            if (skip_spaces(off))
                return STATUS_BAD_FORMAT;
            res             = read_value(off);
            if (res != STATUS_OK)
                return res;

            // Ensure that line ends correctly
            return (skip_spaces(off)) ? STATUS_OK : STATUS_BAD_FORMAT;
        }

        status_t PullParser::commit_param()
        {
            param_t tmp;
            status_t res    = STATUS_OK;

            // If type is explicitly set
            if (nFlags & SF_TYPE_SET)
            {
                switch (nFlags & SF_TYPE_MASK)
                {
                    case SF_TYPE_I32: res = parse_int32(&sValue, &tmp.i32); break;
                    case SF_TYPE_U32: res = parse_uint32(&sValue, &tmp.u32); break;
                    case SF_TYPE_F32: res = parse_float(&sValue, &tmp.f32, &nFlags); break;
                    case SF_TYPE_I64: res = parse_int64(&sValue, &tmp.i64); break;
                    case SF_TYPE_U64: res = parse_uint64(&sValue, &tmp.u64); break;
                    case SF_TYPE_F64: res = parse_double(&sValue, &tmp.f64, &nFlags); break;
                    case SF_TYPE_STR:
                        if ((tmp.str = sValue.clone_utf8()) == NULL)
                            res     = STATUS_NO_MEM;
                        break;
                    case SF_TYPE_BLOB:
                        tmp.blob.ctype  = NULL;
                        tmp.blob.data   = NULL;
                        res = parse_blob(&sValue, &tmp.blob);
                        break;
                    default:
                        return STATUS_UNKNOWN_ERR;
                }
                tmp.flags       = nFlags;
                if (res == STATUS_OK)
                    sParam.swap(&tmp);
                return res;
            }

            // Type is not explicitly set
            if (sValue.index_of('.') < 0)
            {
                // Try to parse as integer
                if ((res = parse_int32(&sValue, &tmp.i32)) == STATUS_OK)
                {
                    tmp.flags     = nFlags | SF_TYPE_I32;
                    sParam.swap(&tmp);
                    return STATUS_OK;
                }
            }

            // Try to parse as float
            if ((res = parse_float(&sValue, &tmp.f32, &nFlags)) == STATUS_OK)
            {
                tmp.flags     = nFlags | SF_TYPE_F32;
                sParam.swap(&tmp);
                return STATUS_OK;
            }

            // Return as a string
            if ((sParam.str = sValue.clone_utf8()) == NULL)
                res     = STATUS_NO_MEM;

            sParam.swap(&tmp);
            return STATUS_OK;
        }

        status_t PullParser::parse_int32(const LSPString *str, int32_t *dst)
        {
            const char *s = str->get_utf8();
            if (s == NULL)
                return STATUS_NO_MEM;

            errno = 0;
            char *end = NULL;
            long value = ::strtol(s, &end, 10);

            if ((errno != 0) || (*end != '\0'))
                return STATUS_BAD_FORMAT;

            *dst = value;
            return STATUS_OK;
        }

        status_t PullParser::parse_uint32(const LSPString *str, uint32_t *dst)
        {
            const char *s = str->get_utf8();
            if (s == NULL)
                return STATUS_NO_MEM;

            errno = 0;
            char *end = NULL;
            unsigned long value = ::strtoul(s, &end, 10);

            if ((errno != 0) || (*end != '\0'))
                return STATUS_BAD_FORMAT;

            *dst = value;
            return STATUS_OK;
        }

        status_t PullParser::parse_int64(const LSPString *str, int64_t *dst)
        {
            const char *s = str->get_utf8();
            if (s == NULL)
                return STATUS_NO_MEM;

            errno = 0;
            char *end = NULL;
            #ifdef PLAFTORM_BSD
                long value = ::strtol(s, &end, 10);
            #else
                long long value = ::strtoll(s, &end, 10);
            #endif /* PLATFORM_BSD */

            if ((errno != 0) || (*end != '\0'))
                return STATUS_BAD_FORMAT;

            *dst = value;
            return STATUS_OK;
        }

        status_t PullParser::parse_uint64(const LSPString *str, uint64_t *dst)
        {
            const char *s = str->get_utf8();
            if (s == NULL)
                return STATUS_NO_MEM;

            errno = 0;
            char *end = NULL;
            #ifdef PLAFTORM_BSD
                unsigned long value = ::strtoul(s, &end, 10);
            #else
                unsigned long long value = ::strtoull(s, &end, 10);
            #endif /* PLATFORM_BSD */

            if ((errno != 0) || (*end != '\0'))
                return STATUS_BAD_FORMAT;

            *dst = value;
            return STATUS_OK;
        }

        status_t PullParser::parse_float(const LSPString *str, float *dst, size_t *flags)
        {
            const char *s = str->get_utf8();
            if (s == NULL)
                return STATUS_NO_MEM;

            // Save and update locale
            char *saved = ::setlocale(LC_NUMERIC, NULL);
            if (saved != NULL)
            {
                size_t len = ::strlen(saved) + 1;
                char *saved_copy = static_cast<char *>(alloca(len));
                ::memcpy(saved_copy, saved, len);
                saved       = saved_copy;
            }
            ::setlocale(LC_NUMERIC, "C");

            // Parse float
            errno = 0;
            char *end   = NULL;
            float value = ::strtof(s, &end);
            size_t xf   = 0;

            bool success = (errno == 0);
            if ((success) && (end != NULL))
            {
                // Skip spaces
                while (((*end) == ' ') || ((*end) == '\t'))
                    ++ end;
                if (((end[0] == 'd') || (end[0] == 'D')) &&
                    ((end[1] == 'b') || (end[1] == 'B')))
                {
                    xf     |= SF_DECIBELS;
                    value   = expf(value * M_LN10 * 0.05f);
                    end    += 2;
                }
                // Skip spaces
                while (((*end) == ' ') || ((*end) == '\t'))
                    ++ end;
                if (*end != '\0')
                    success = false;
            }

            // Restore locale
            if (saved != NULL)
                ::setlocale(LC_NUMERIC, saved);

            // Return result
            if (!success)
                return STATUS_BAD_FORMAT;

            *dst        = value;
            *flags     |= xf;
            return STATUS_OK;
        }

        status_t PullParser::parse_double(const LSPString *str, double *dst, size_t *flags)
        {
            const char *s = str->get_utf8();
            if (s == NULL)
                return STATUS_NO_MEM;

            // Save and update locale
            char *saved = ::setlocale(LC_NUMERIC, NULL);
            if (saved != NULL)
            {
                size_t len = ::strlen(saved) + 1;
                char *saved_copy = static_cast<char *>(alloca(len));
                ::memcpy(saved_copy, saved, len);
                saved       = saved_copy;
            }
            ::setlocale(LC_NUMERIC, "C");

            // Parse float
            errno = 0;
            char *end       = NULL;
            double value    = ::strtod(s, &end);
            size_t xf       = 0;

            bool success = (errno == 0);
            if ((success) && (end != NULL))
            {
                // Skip spaces
                while (((*end) == ' ') || ((*end) == '\t'))
                    ++ end;
                if (((end[0] == 'd') || (end[0] == 'D')) &&
                    ((end[1] == 'b') || (end[1] == 'B')))
                {
                    xf     |= SF_DECIBELS;
                    value   = exp(value * M_LN10 * 0.05);
                    end    += 2;
                }
                // Skip spaces
                while (((*end) == ' ') || ((*end) == '\t'))
                    ++ end;
                if (*end != '\0')
                    success = false;
            }

            // Restore locale
            if (saved != NULL)
                ::setlocale(LC_NUMERIC, saved);

            // Return result
            if (!success)
                return STATUS_BAD_FORMAT;

            *dst        = value;
            *flags     |= xf;
            return STATUS_OK;
        }

        status_t PullParser::parse_blob(const LSPString *str, blob_t *dst)
        {
            ssize_t idx = str->index_of(':');
            if (idx <= 0)
            {
                dst->ctype  = NULL;
                dst->data   = str->clone_utf8();
                if (dst->data == NULL)
                    return STATUS_NO_MEM;
            }
            else
            {
                dst->ctype  = str->clone_utf8(ssize_t(0), idx);
                dst->data   = str->clone_utf8(idx + 1);
                if ((dst->ctype == NULL) || (dst->data == NULL))
                    return STATUS_NO_MEM;
            }

            dst->length     = ::strlen(dst->data);
            return STATUS_OK;
        }
    
    } /* namespace config */
} /* namespace lsp */

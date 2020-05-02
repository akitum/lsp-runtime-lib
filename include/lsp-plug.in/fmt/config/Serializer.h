/*
 * ConfigSerializer.h
 *
 *  Created on: 1 мая 2020 г.
 *      Author: sadko
 */

#ifndef LSP_PLUG_IN_FMT_CONFIG_SERIALIZER_H_
#define LSP_PLUG_IN_FMT_CONFIG_SERIALIZER_H_

#include <lsp-plug.in/runtime/version.h>
#include <lsp-plug.in/common/types.h>
#include <lsp-plug.in/common/status.h>
#include <lsp-plug.in/runtime/LSPString.h>
#include <lsp-plug.in/io/IOutSequence.h>
#include <lsp-plug.in/io/IOutStream.h>
#include <lsp-plug.in/io/Path.h>
#include <lsp-plug.in/fmt/config/types.h>

namespace lsp
{
    namespace config
    {
        /**
         * Configuration serializer
         */
        class Serializer
        {
            private:
                Serializer & operator = (const Serializer &);

            protected:
                io::IOutSequence       *pOut;
                size_t                  nWFlags;

            protected:
                status_t    write_key(const LSPString *key);

                status_t    write_escaped(const LSPString *key, size_t flags);
                status_t    write_uint(uint64_t v, size_t flags);
                status_t    write_int(int64_t v, size_t flags);
                status_t    write_float(double v, size_t flags);

            public:
                explicit Serializer();
                virtual ~Serializer();

            public:
                /**
                 * Open parser
                 * @param path UTF-8 path to the file
                 * @param charset character set
                 * @return status of operation
                 */
                status_t    open(const char *path, const char *charset = NULL);

                /**
                 * Open parser
                 * @param path string representation of path to the file
                 * @param charset character set
                 * @return status of operation
                 */
                status_t    open(const LSPString *path, const char *charset = NULL);

                /**
                 * Open parser
                 * @param path path to the file
                 * @param charset character set
                 * @return status of operation
                 */
                status_t    open(const io::Path *path, const char *charset = NULL);

                /**
                 * Wrap string with parser
                 * @param str string to wrap
                 * @return status of operation
                 */
                status_t    wrap(LSPString *str);

                /**
                 * Wrap input sequence with parser
                 * @param seq sequence to use for reads
                 * @param flags wrapping flags
                 * @return status of operation
                 */
                status_t    wrap(io::IOutSequence *seq, size_t flags = WRAP_NONE);

                /**
                 * Wrap input stream with parser
                 * @param os output stream
                 * @param version JSON version
                 * @param flags wrapping flags
                 * @param charset character set
                 * @return status of operation
                 */
                status_t    wrap(io::IOutStream *os, size_t flags = WRAP_NONE, const char *charset = NULL);

                /**
                 * Close parser
                 * @return status of operation
                 */
                status_t    close();

            public:
                /**
                 * Write comment to the output stream
                 * @param key key
                 * @param v comment
                 * @return status of operation
                 */
                status_t    write_comment(const LSPString *v);

                /**
                 * Write comment to the output stream
                 * @param key key
                 * @param v UTF-8 encoded comment
                 * @return status of operation
                 */
                status_t    write_comment(const char *v);

                /**
                 * Write empty line to the output file
                 * @return status of operation
                 */
                status_t    writeln();

                /**
                 * Write value to the output stream
                 * @param key key
                 * @param v value
                 * @param flags value flags
                 * @return status of operation
                 */
                status_t    write(const LSPString *key, const value_t *v, size_t flags);
                status_t    write(const char *key, const value_t *v, size_t flags);

                /**
                 * Write 32-bit signed integer to the output stream
                 * @param key key
                 * @param v value
                 * @param flags value flags
                 * @return status of operation
                 */
                status_t    write_i32(const LSPString *key, int32_t value, size_t flags);
                status_t    write_i32(const char *key, int32_t value, size_t flags);

                /**
                 * Write 32-bit unsigned integer to the output stream
                 * @param key key
                 * @param v value
                 * @param flags value flags
                 * @return status of operation
                 */
                status_t    write_u32(const LSPString *key, uint32_t value, size_t flags);
                status_t    write_u32(const char *key, uint32_t value, size_t flags);

                /**
                 * Write 32-bit floating-point to the output stream
                 * @param key key
                 * @param v value
                 * @param flags value flags
                 * @return status of operation
                 */
                status_t    write_f32(const LSPString *key, float value, size_t flags);
                status_t    write_f32(const char *key, float value, size_t flags);

                /**
                 * Write 64-bit signed integer to the output stream
                 * @param key key
                 * @param v value
                 * @param flags value flags
                 * @return status of operation
                 */
                status_t    write_i64(const LSPString *key, int64_t value, size_t flags);
                status_t    write_i64(const char *key, int64_t value, size_t flags);

                /**
                 * Write 64-bit unsigned integer to the output stream
                 * @param key key
                 * @param v value
                 * @param flags value flags
                 * @return status of operation
                 */
                status_t    write_u64(const LSPString *key, uint64_t value, size_t flags);
                status_t    write_u64(const char *key, uint64_t value, size_t flags);

                /**
                 * Write 64-bit floating-point to the output stream
                 * @param key key
                 * @param v value
                 * @param flags value flags
                 * @return status of operation
                 */
                status_t    write_f64(const LSPString *key, double value, size_t flags);
                status_t    write_f64(const char *key, double value, size_t flags);

                /**
                 * Write text string
                 * @param key key
                 * @param v string
                 * @param flags value flags
                 * @return status of operation
                 */
                status_t    write_string(const LSPString *key, const LSPString *v, size_t flags);
                status_t    write_string(const LSPString *key, const char *v, size_t flags);
                status_t    write_string(const char *key, const LSPString *v, size_t flags);
                status_t    write_string(const char *key, const char *v, size_t flags);

                /**
                 * Write blob data
                 * @param key key
                 * @param v blob data
                 * @param flags value flags
                 * @return status of operation
                 */
                status_t    write_blob(const LSPString *key, const blob_t *v, size_t flags);
        };
    
    } /* namespace config */
} /* namespace lsp */

#endif /* LSP_PLUG_IN_FMT_CONFIG_SERIALIZER_H_ */
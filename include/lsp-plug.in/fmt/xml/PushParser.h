/*
 * PushParser.h
 *
 *  Created on: 29 окт. 2019 г.
 *      Author: sadko
 */

#ifndef LSP_PLUG_IN_FMT_XML_PUSHPARSER_H_
#define LSP_PLUG_IN_FMT_XML_PUSHPARSER_H_

#include <lsp-plug.in/runtime/version.h>
#include <lsp-plug.in/fmt/xml/PullParser.h>
#include <lsp-plug.in/fmt/xml/IXMLHandler.h>
#include <lsp-plug.in/lltl/parray.h>

namespace lsp
{
    namespace xml
    {

        class PushParser
        {
            private:
                PullParser          sParser;

            private:
                status_t            parse_document(IXMLHandler *handler);
                void                drop_list(lltl::parray<LSPString> *list);

            public:
                explicit PushParser();
                virtual ~PushParser();

            public:
                /**
                 * Parse file
                 * @param handler XML handler
                 * @param path UTF-8 path to the file
                 * @param charset character set encoding of the file
                 * @return status of operation
                 */
                status_t            parse_file(IXMLHandler *handler, const char *path, const char *charset = NULL);

                /**
                 * Parse file
                 * @param handler XML handler
                 * @param path path to the file
                 * @param charset character set encoding of the file
                 * @return status of operation
                 */
                status_t            parse_file(IXMLHandler *handler, const LSPString *path, const char *charset = NULL);

                /**
                 * Parse file
                 * @param handler XML handler
                 * @param path path to the file
                 * @param charset character set encoding of the file
                 * @return status of operation
                 */
                status_t            parse_file(IXMLHandler *handler, const io::Path *path, const char *charset = NULL);

                /**
                 * Parse data from input stream
                 * @param handler XML handler
                 * @param is input stream
                 * @param flags wrap flags
                 * @param charset character set
                 * @return status of operation
                 */
                status_t            parse_data(IXMLHandler *handler, io::IInStream *is, size_t flags = WRAP_NONE, const char *charset = NULL);

                /**
                 * Parse data from UTF-8 encoded string
                 * @param handler XML handler
                 * @param str data input
                 * @param charset character set
                 * @return status of operation
                 */
                status_t            parse_data(IXMLHandler *handler, const char *str, const char *charset = NULL);

                /**
                 * Parse data from text string
                 * @param handler XML handler
                 * @param str data input
                 * @return status of operation
                 */
                status_t            parse_data(IXMLHandler *handler, const LSPString *str);

                /**
                 * Parse data from character sequence
                 * @param handler XML handler
                 * @param seq input sequence
                 * @param flags wrap flags
                 * @return status of operation
                 */
                status_t            parse_data(IXMLHandler *handler, io::IInSequence *seq, size_t flags = WRAP_NONE);
        };
    
    } /* namespace xml */
} /* namespace lsp */

#endif /* LSP_PLUG_IN_FMT_XML_PUSHPARSER_H_ */
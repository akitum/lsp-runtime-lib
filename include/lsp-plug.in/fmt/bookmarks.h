/*
 * bookmarks.h
 *
 *  Created on: 14 окт. 2019 г.
 *      Author: sadko
 */

#ifndef LSP_PLUG_IN_FMT_BOOKMARKS_H_
#define LSP_PLUG_IN_FMT_BOOKMARKS_H_

#include <lsp-plug.in/runtime/version.h>
#include <lsp-plug.in/lltl/parray.h>
#include <lsp-plug.in/common/status.h>
#include <lsp-plug.in/runtime/LSPString.h>
#include <lsp-plug.in/io/Path.h>
#include <lsp-plug.in/io/IInSequence.h>
#include <lsp-plug.in/io/IOutSequence.h>

#define GTK3_BOOKMARK_PATH          ".config/gtk-3.0/bookmarks"
#define GTK2_BOOKMARK_PATH          ".gtk-bookmarks"
#define QT5_BOOKMARK_PATH           ".local/share/user-places.xbel"
#define LSP_BOOKMARK_PATH           LSP_ARTIFACT_ID FILE_SEPARATOR_S "bookmarks.json"

namespace lsp
{
    namespace bookmarks
    {
        enum bm_origin_t
        {
            BM_LSP      = 1 << 0,
            BM_GTK2     = 1 << 1,
            BM_GTK3     = 1 << 2,
            BM_QT5      = 1 << 3
        };

        typedef struct bookmark_t
        {
            LSPString   path;       // Path to the directory
            LSPString   name;       // Bookmark name
            size_t      origin;     // Bookmark origin: LSP, GTK3, QT
        } bookmark_t;

        /**
         * Read GTK3 bookmarks
         * @param dst list to store bookmarks
         * @param path location of the file in UTF-8 encoding
         * @param charset file encoding character set, NULL for default locale
         * @return status of operation
         */
        status_t read_bookmarks_gtk3(lltl::parray<bookmark_t> *dst, const char *path, const char *charset=NULL);

        /**
         * Read GTK3 bookmarks
         * @param dst list to store bookmarks
         * @param path location of the file
         * @param charset file encoding character set, NULL for default locale
         * @return status of operation
         */
        status_t read_bookmarks_gtk3(lltl::parray<bookmark_t> *dst, const LSPString *path, const char *charset=NULL);

        /**
         * Read GTK3 bookmarks
         * @param dst list to store bookmarks
         * @param path location of the file
         * @param charset file encoding character set, NULL for default locale
         * @return status of operation
         */
        status_t read_bookmarks_gtk3(lltl::parray<bookmark_t> *dst, const io::Path *path, const char *charset=NULL);

        /**
         * Read GTK3 bookmarks
         * @param dst list to store bookmarks
         * @param in input character sequence
         * @return status of operation
         */
        status_t read_bookmarks_gtk3(lltl::parray<bookmark_t> *dst, io::IInSequence *in);

        /**
         * Read GTK3 bookmarks
         * @param dst list to store bookmarks
         * @param path location of the file in UTF-8 encoding
         * @param charset file encoding character set, NULL for default locale
         * @return status of operation
         */
        status_t read_bookmarks_gtk2(lltl::parray<bookmark_t> *dst, const char *path, const char *charset=NULL);

        /**
         * Read GTK3 bookmarks
         * @param dst list to store bookmarks
         * @param path location of the file
         * @param charset file encoding character set, NULL for default locale
         * @return status of operation
         */
        status_t read_bookmarks_gtk2(lltl::parray<bookmark_t> *dst, const LSPString *path, const char *charset=NULL);

        /**
         * Read GTK3 bookmarks
         * @param dst list to store bookmarks
         * @param path location of the file
         * @param charset file encoding character set, NULL for default locale
         * @return status of operation
         */
        status_t read_bookmarks_gtk2(lltl::parray<bookmark_t> *dst, const io::Path *path, const char *charset=NULL);

        /**
         * Read GTK3 bookmarks
         * @param dst list to store bookmarks
         * @param in input character sequence
         * @return status of operation
         */
        status_t read_bookmarks_gtk2(lltl::parray<bookmark_t> *dst, io::IInSequence *in);

        /**
         * Read QT5 bookmarks
         * @param dst list to store bookmarks
         * @param path location of the file in UTF-8 encoding
         * @param charset file encoding character set, NULL for default locale
         * @return status of operation
         */
        status_t read_bookmarks_qt5(lltl::parray<bookmark_t> *dst, const char *path, const char *charset=NULL);

        /**
         * Read QT5 bookmarks
         * @param dst list to store bookmarks
         * @param path location of the file
         * @param charset file encoding character set, NULL for default locale
         * @return status of operation
         */
        status_t read_bookmarks_qt5(lltl::parray<bookmark_t> *dst, const LSPString *path, const char *charset=NULL);

        /**
         * Read QT5 bookmarks
         * @param dst list to store bookmarks
         * @param path location of the file
         * @param charset file encoding character set, NULL for default locale
         * @return status of operation
         */
        status_t read_bookmarks_qt5(lltl::parray<bookmark_t> *dst, const io::Path *path, const char *charset=NULL);

        /**
         * Read QT5 bookmarks
         * @param dst list to store bookmarks
         * @param in input character sequence
         * @return status of operation
         */
        status_t read_bookmarks_qt5(lltl::parray<bookmark_t> *dst, io::IInSequence *in);

        /**
         * Read LSP bookmarks in JSON5 format
         * @param dst list to store bookmarks
         * @param path location of the file in UTF-8 encoding
         * @param charset file encoding character set, NULL for default locale
         * @return status of operation
         */
        status_t read_bookmarks(lltl::parray<bookmark_t> *dst, const char *path, const char *charset=NULL);

        /**
         * Read LSP bookmarks in JSON5 format
         * @param dst list to store bookmarks
         * @param path location of the file
         * @param charset file encoding character set, NULL for default locale
         * @return status of operation
         */
        status_t read_bookmarks(lltl::parray<bookmark_t> *dst, const LSPString *path, const char *charset=NULL);

        /**
         * Read LSP bookmarks in JSON5 format
         * @param dst list to store bookmarks
         * @param path location of the file
         * @param charset file encoding character set, NULL for default locale
         * @return status of operation
         */
        status_t read_bookmarks(lltl::parray<bookmark_t> *dst, const io::Path *path, const char *charset=NULL);

        /**
         * Read LSP bookmarks in JSON5 format
         * @param dst list to store bookmarks
         * @param in input character sequence
         * @return status of operation
         */
        status_t read_bookmarks(lltl::parray<bookmark_t> *dst, io::IInSequence *in);

        /**
         * Save LSP bookmarks in JSON5 format
         * @param src list to store bookmarks
         * @param path location of the file in UTF-8 encoding
         * @param charset file encoding character set, NULL for default locale
         * @return status of operation
         */
        status_t save_bookmarks(const lltl::parray<bookmark_t> *src, const char *path, const char *charset=NULL);

        /**
         * Save LSP bookmarks in JSON5 format
         * @param src list to store bookmarks
         * @param path location of the file
         * @param charset file encoding character set, NULL for default locale
         * @return status of operation
         */
        status_t save_bookmarks(const lltl::parray<bookmark_t> *src, const LSPString *path, const char *charset=NULL);

        /**
         * Save LSP bookmarks in JSON5 format
         * @param src list to store bookmarks
         * @param path location of the file
         * @param charset file encoding character set, NULL for default locale
         * @return status of operation
         */
        status_t save_bookmarks(const lltl::parray<bookmark_t> *src, const io::Path *path, const char *charset=NULL);

        /**
         * Save LSP bookmarks in JSON5 format
         * @param src list to store bookmarks
         * @param out output character sequence
         * @return status of operation
         */
        status_t save_bookmarks(const lltl::parray<bookmark_t> *src, io::IOutSequence *out);

        /**
         * Merge bookmarks:
         *   - add all existing in src bookmarks to dst if they are non-existing in dst
         *   - set origin flag in dst to 'origin' for all bookmarks that exist in dst and origin
         *   - unset origin flsg in dst for all bookmarks that do not exist in src
         *   - remove all bookmarks that have zero origin
         * @param dst destination storage of bookmarks
         * @param src source change set
         * @param origin origin flag associated with source
         * @param changes pointer to integer to add number of changes
         * @return status of operation
         */
        status_t merge_bookmarks(lltl::parray<bookmark_t> *dst, size_t *changes, const lltl::parray<bookmark_t> *src, bm_origin_t origin);

        /**
         * Destroy bookmarks
         * @param list list of bookmarks
         */
        void destroy_bookmarks(lltl::parray<bookmark_t> *list);
    }
}

#endif /* LSP_PLUG_IN_FMT_BOOKMARKS_H_ */
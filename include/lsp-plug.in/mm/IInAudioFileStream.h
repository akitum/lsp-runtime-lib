/*
 * IInAudioFileStream.h
 *
 *  Created on: 19 апр. 2020 г.
 *      Author: sadko
 */

#ifndef LSP_PLUG_IN_MM_IINAUDIOFILESTREAM_H_
#define LSP_PLUG_IN_MM_IINAUDIOFILESTREAM_H_

#include <lsp-plug.in/runtime/version.h>
#include <lsp-plug.in/runtime/LSPString.h>
#include <lsp-plug.in/io/Path.h>
#include <lsp-plug.in/mm/IInAudioStream.h>

#ifdef PLATFORM_WINDOWS
#else
    #include <sndfile.h>
#endif

namespace lsp
{
    namespace mm
    {
        class IInAudioFileStream: public IInAudioStream
        {
            protected:
            #ifdef PLATFORM_WINDOWS
            #else
                SNDFILE            *hHandle;
                bool                bSeekable;
            #endif

            protected:
            #ifndef PLATFORM_WINDOWS
                static status_t     decode_sf_error(SNDFILE *fd);
            #endif

                virtual ssize_t     direct_read(void *dst, size_t nframes, size_t rfmt, size_t *afmt);

                status_t            close_handle();

            public:
                explicit IInAudioFileStream();
                virtual ~IInAudioFileStream();

            public:
                /**
                 * Open audio stream
                 * @param path path to the audio file
                 * @return status of operation
                 */
                virtual status_t    open(const char *path);

                /**
                 * Open audio stream
                 * @param path path to the audio file
                 * @return status of operation
                 */
                virtual status_t    open(const LSPString *path);

                /**
                 * Open audio stream
                 * @param path path to the audio file
                 * @return status of operation
                 */
                virtual status_t    open(const io::Path *path);

                virtual status_t    close();

                virtual wssize_t    skip(wsize_t nframes);

                virtual wssize_t    seek(wsize_t nframes);
        };
    
    } /* namespace mm */
} /* namespace lsp */

#endif /* LSP_PLUG_IN_MM_IINAUDIOFILESTREAM_H_ */

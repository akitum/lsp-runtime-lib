/*
 * sample.cpp
 *
 *  Created on: 16 апр. 2020 г.
 *      Author: sadko
 */

#include <lsp-plug.in/common/endian.h>
#include <lsp-plug.in/mm/sample.h>
#include <lsp-plug.in/stdlib/string.h>

namespace lsp
{
    namespace mm
    {
        typedef void (*cvt_sample_t) (void *dst, const void *src, size_t samples);

        static inline uint32_t read24bit(const uint8_t *p)
        {
            __IF_LE(return (uint32_t(p[0])) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16));
            __IF_BE(return (uint32_t(p[2])) | (uint32_t(p[1]) << 8) | (uint32_t(p[0]) << 16));
        }

        static inline void write24bit(uint8_t *p, uint32_t x)
        {
            __IF_LE(
                p[0] = uint8_t(x);
                p[1] = uint8_t(x >> 8);
                p[2] = uint8_t(x >> 16);
            )
            __IF_BE(
                p[0] = uint8_t(x >> 16);
                p[1] = uint8_t(x >> 8);
                p[2] = uint8_t(x);
            )
        }

        bool sample_endian_to_cpu(void *buf, size_t samples, size_t format)
        {
            size_t fmt = sformat_endian(format);
            if (fmt == __IF_LEBE(SFMT_LE, SFMT_BE))
                return true;
            else if (fmt != __IF_LEBE(SFMT_BE, SFMT_LE))
                return false;

            switch (sformat_format(format))
            {
                case SFMT_U8:
                case SFMT_S8:
                    break;
                case SFMT_U16:
                case SFMT_S16:
                    ::lsp::byte_swap(static_cast<uint16_t *>(buf), samples);
                    break;
                case SFMT_U32:
                case SFMT_S32:
                    ::lsp::byte_swap(static_cast<uint32_t *>(buf), samples);
                    break;
                case SFMT_F32:
                    ::lsp::byte_swap(static_cast<f32_t *>(buf), samples);
                    break;
                case SFMT_F64:
                    ::lsp::byte_swap(static_cast<f64_t *>(buf), samples);
                    break;

                case SFMT_U24: case SFMT_S24:
                {
                    uint8_t x, *ptr = static_cast<uint8_t *>(buf);
                    while (samples--)
                    {
                        x       = ptr[0];
                        ptr[0]  = ptr[2];
                        ptr[2]  = x;
                        ptr    += 3;
                    }
                    break;
                }

                default:
                    return false;
            }

            return true;
        }

        #define CVT_SHIFT(TYPE)         (uint32_t(1 << (sizeof(TYPE)*8 - 1)))
        #define CVT_RANGE(TYPE)         (uint32_t(1 << (sizeof(TYPE)*8 - 1))-1)

        // Integer conversions
        #define CVT_UI_TO_UI(DTYPE, STYPE, SHIFT) \
            for (STYPE *sptr = static_cast<STYPE *>(src); samples > 0; --samples, ++sptr, ++dptr) \
                *dptr   = (DTYPE)((*sptr) SHIFT);

        #define CVT_UI_TO_SI(DTYPE, STYPE, SHIFT) \
            for (STYPE *sptr = static_cast<STYPE *>(src); samples > 0; --samples, ++sptr, ++dptr) \
                *dptr   = (DTYPE)((*sptr - CVT_SHIFT(STYPE)) SHIFT);

        #define CVT_SI_TO_UI(DTYPE, STYPE, SHIFT) \
            for (STYPE *sptr = static_cast<STYPE *>(src); samples > 0; --samples, ++sptr, ++dptr) \
                *dptr   = (DTYPE)((*sptr + CVT_SHIFT(STYPE)) SHIFT);

        #define CVT_SI_TO_SI(DTYPE, STYPE, SHIFT) \
            for (STYPE *sptr = static_cast<STYPE *>(src); samples > 0; --samples, ++sptr, ++dptr) \
                *dptr   = (DTYPE)((*sptr) SHIFT);

        #define CVT_UI_TO_FX(DTYPE, STYPE) \
            for (STYPE *sptr = static_cast<STYPE *>(src); samples > 0; --samples, ++sptr, ++dptr) \
                *dptr   = ((STYPE)(*sptr - CVT_SHIFT(STYPE))) * (DTYPE(1.0) / CVT_RANGE(STYPE));

        #define CVT_SI_TO_FX(DTYPE, STYPE) \
            for (STYPE *sptr = static_cast<STYPE *>(src); samples > 0; --samples, ++sptr, ++dptr) \
                *dptr   = ((STYPE)(*sptr)) * (DTYPE(1.0) / CVT_RANGE(STYPE));

        #define CVT_UI_TO_XI(DTYPE, STYPE, SHIFT) \
            if (sign) \
                CVT_UI_TO_SI(DTYPE, STYPE, SHIFT) \
            else \
                CVT_UI_TO_UI(DTYPE, STYPE, SHIFT)

        #define CVT_SI_TO_XI(DTYPE, STYPE, SHIFT) \
            if (sign) \
                CVT_SI_TO_SI(DTYPE, STYPE, SHIFT) \
            else \
                CVT_SI_TO_SI(DTYPE, STYPE, SHIFT)

        // Integer 24-bit conversions
        #define CVT_U24_TO_UI(DTYPE, SHIFT) \
            for (uint8_t *sptr = static_cast<uint8_t *>(src); samples > 0; --samples, sptr += 3, ++dptr) \
                *dptr   = (DTYPE)(read24bit(sptr) SHIFT);

        #define CVT_U24_TO_SI(DTYPE, SHIFT) \
            for (uint8_t *sptr = static_cast<uint8_t *>(src); samples > 0; --samples, sptr += 3, ++dptr) \
                *dptr   = (DTYPE)((read24bit(sptr) - 0x800000) SHIFT);

        #define CVT_S24_TO_UI(DTYPE, SHIFT) \
            for (uint8_t *sptr = static_cast<uint8_t *>(src); samples > 0; --samples, sptr += 3, ++dptr) \
                *dptr   = (DTYPE)((read24bit(sptr) + 0x800000) SHIFT);

        #define CVT_S24_TO_SI(DTYPE, SHIFT) \
            for (uint8_t *sptr = static_cast<uint8_t *>(src); samples > 0; --samples, sptr += 3, ++dptr) \
                *dptr   = (DTYPE)(read24bit(sptr) SHIFT);

        #define CVT_U24_TO_FX(DTYPE) \
            for (uint8_t *sptr = static_cast<uint8_t *>(src); samples > 0; --samples, sptr += 3, ++dptr) \
                *dptr   = (int32_t(read24bit(sptr) - 0x800000) * (DTYPE(1.0)/0x7fffff));

        #define CVT_S24_TO_FX(DTYPE) \
            for (uint8_t *sptr = static_cast<uint8_t *>(src); samples > 0; --samples, sptr += 3, ++dptr) \
                *dptr   = (int32_t(read24bit(sptr)) * (DTYPE(1.0)/0x7fffff));

        #define CVT_U24_TO_XI(DTYPE, SHIFT) \
            if (sign) \
                CVT_U24_TO_SI(DTYPE, SHIFT) \
            else \
                CVT_U24_TO_UI(DTYPE, SHIFT)

        #define CVT_S24_TO_XI(DTYPE, SHIFT) \
            if (sign) \
                CVT_S24_TO_SI(DTYPE, SHIFT) \
            else \
                CVT_S24_TO_UI(DTYPE, SHIFT)


        #define CVT_UI_TO_UI24(STYPE, SHIFT) \
            for (STYPE *sptr = static_cast<STYPE *>(src); samples > 0; --samples, ++sptr, dptr += 3) \
                write24bit(dptr, uint32_t((*sptr) SHIFT));

        #define CVT_UI_TO_SI24(STYPE, SHIFT) \
            for (STYPE *sptr = static_cast<STYPE *>(src); samples > 0; --samples, ++sptr, dptr += 3) \
                write24bit(dptr, uint32_t((*sptr - CVT_SHIFT(STYPE)) SHIFT));

        #define CVT_SI_TO_UI24(STYPE, SHIFT) \
            for (STYPE *sptr = static_cast<STYPE *>(src); samples > 0; --samples, ++sptr, dptr += 3) \
                write24bit(dptr, uint32_t((*sptr + CVT_SHIFT(STYPE)) SHIFT));

        #define CVT_SI_TO_SI24(STYPE, SHIFT) \
            for (STYPE *sptr = static_cast<STYPE *>(src); samples > 0; --samples, ++sptr, dptr += 3) \
                write24bit(dptr, uint32_t((*sptr) SHIFT));

        #define CVT_UI_TO_XI24(STYPE, SHIFT) \
            if (sign) \
                CVT_UI_TO_SI24(STYPE, SHIFT) \
            else \
                CVT_UI_TO_UI24(STYPE, SHIFT)

        #define CVT_SI_TO_XI24(STYPE, SHIFT) \
            if (sign) \
                CVT_SI_TO_SI24(STYPE, SHIFT) \
            else \
                CVT_SI_TO_UI24(STYPE, SHIFT)

        #define CVT_SI24_TO_UI24() \
            for (uint8_t *sptr = static_cast<uint8_t *>(src); samples > 0; --samples, sptr += 3, dptr += 3) \
                write24bit(dptr, read24bit(sptr) + 0x800000);

        #define CVT_UI24_TO_SI24() \
            for (uint8_t *sptr = static_cast<uint8_t *>(src); samples > 0; --samples, sptr += 3, dptr += 3) \
                write24bit(dptr, read24bit(sptr) - 0x800000);

        // Float conversions
        #define CVT_F32_TO_UI(DTYPE) \
            for (f32_t *sptr = static_cast<f32_t *>(src); samples > 0; --samples, ++sptr, ++dptr) \
                *dptr   = (DTYPE)(*sptr * float(CVT_RANGE(DTYPE))) + (DTYPE)CVT_SHIFT(DTYPE);
        #define CVT_F32_TO_SI(DTYPE) \
            for (f32_t *sptr = static_cast<f32_t *>(src); samples > 0; --samples, ++sptr, ++dptr) \
                *dptr   = (DTYPE)(*sptr * float(CVT_RANGE(DTYPE)));

        #define CVT_F32_TO_XI(DTYPE) \
            if (sign) \
                CVT_F32_TO_SI(DTYPE) \
            else \
                CVT_F32_TO_UI(DTYPE)


        // Double conversions
        #define CVT_F64_TO_UI(DTYPE) \
            for (f64_t *sptr = static_cast<f64_t *>(src); samples > 0; --samples, ++sptr, ++dptr) \
                *dptr   = (DTYPE)(*sptr * double(CVT_RANGE(DTYPE))) + (DTYPE)CVT_SHIFT(DTYPE);
        #define CVT_F64_TO_SI(DTYPE) \
            for (f64_t *sptr = static_cast<f64_t *>(src); samples > 0; --samples, ++sptr, ++dptr) \
                *dptr   = (DTYPE)(*sptr * double(CVT_RANGE(DTYPE)));

        #define CVT_F64_TO_XI(DTYPE) \
            if (sign) \
                CVT_F64_TO_SI(DTYPE) \
            else \
                CVT_F64_TO_UI(DTYPE)

        #define CVT_FX_TO_SI24(STYPE) \
            for (STYPE *sptr = static_cast<STYPE *>(src); samples > 0; --samples, ++sptr, dptr += 3) \
                write24bit(dptr, int32_t(*sptr * 0x7fffff));
        #define CVT_FX_TO_UI24(STYPE) \
            for (STYPE *sptr = static_cast<STYPE *>(src); samples > 0; --samples, ++sptr, dptr += 3) \
                write24bit(dptr, int32_t(*sptr * 0x7fffff) - 0x800000);
        #define CVT_FX_TO_XI24(STYPE) \
            if (sign) \
                CVT_FX_TO_SI24(STYPE) \
            else \
                CVT_FX_TO_UI24(STYPE)

        #define CVT_FX_TO_FX(DTYPE, STYPE) \
            for (STYPE *sptr = static_cast<STYPE *>(src); samples > 0; --samples, ++sptr, ++dptr) \
                *dptr   = (DTYPE)(*sptr);


        bool convert_to_8bit(void *dst, void *src, size_t samples, size_t to, size_t from)
        {
            int sign = sformat_signed(to);
            if (sign < 0)
                return false;
            uint8_t *dptr = static_cast<uint8_t *>(dst);

            switch (sformat_format(from))
            {
                case SFMT_U8:
                    if (sign)   CVT_SI_TO_UI(uint8_t, uint8_t, )
                    else        ::memcpy(dptr, src, samples * sizeof(uint8_t));
                    return true;
                case SFMT_S8:
                    if (sign)   ::memcpy(dptr, src, samples * sizeof(uint8_t));
                    else        CVT_UI_TO_SI(uint8_t, uint8_t, )
                    return true;

                case SFMT_U16: CVT_UI_TO_XI(uint8_t, uint16_t, >> 8)    return true;
                case SFMT_S16: CVT_SI_TO_XI(uint8_t, uint16_t, >> 8)    return true;
                case SFMT_U24: CVT_U24_TO_XI(uint8_t, >> 16)            return true;
                case SFMT_S24: CVT_S24_TO_XI(uint8_t, >> 16)            return true;
                case SFMT_U32: CVT_UI_TO_XI(uint8_t, uint32_t, >> 24)   return true;
                case SFMT_S32: CVT_SI_TO_XI(uint8_t, uint32_t, >> 24)   return true;
                case SFMT_F32: CVT_F32_TO_XI(uint8_t)                   return true;
                case SFMT_F64: CVT_F64_TO_XI(uint8_t)                   return true;

                default:
                    break;
            }

            return false;
        }

        bool convert_to_16bit(void *dst, void *src, size_t samples, size_t to, size_t from)
        {
            int sign = sformat_signed(to);
            if (sign < 0)
                return false;
            uint16_t *dptr = static_cast<uint16_t *>(dst);

            switch (sformat_format(from))
            {
                case SFMT_U8:  CVT_UI_TO_XI(uint16_t, uint8_t, << 8)    return true;
                case SFMT_S8:  CVT_SI_TO_XI(uint16_t, uint8_t, << 8)    return true;

                case SFMT_U16:
                    if (sign)   CVT_SI_TO_UI(uint16_t, uint16_t, )
                    else        ::memcpy(dptr, src, samples * sizeof(uint16_t));
                    return true;
                case SFMT_S16:
                    if (sign)   ::memcpy(dptr, src, samples * sizeof(uint16_t));
                    else        CVT_UI_TO_SI(uint16_t, uint16_t, )
                    return true;

                case SFMT_U24: CVT_U24_TO_XI(uint16_t, >> 8)            return true;
                case SFMT_S24: CVT_S24_TO_XI(uint16_t, >> 8)            return true;
                case SFMT_U32: CVT_UI_TO_XI(uint16_t, uint32_t, >> 16)  return true;
                case SFMT_S32: CVT_SI_TO_XI(uint16_t, uint32_t, >> 16)  return true;
                case SFMT_F32: CVT_F32_TO_XI(uint16_t)                  return true;
                case SFMT_F64: CVT_F64_TO_XI(uint16_t)                  return true;

                default:
                    break;
            }

            return false;
        }

        bool convert_to_24bit(void *dst, void *src, size_t samples, size_t to, size_t from)
        {
            int sign = sformat_signed(to);
            if (sign < 0)
                return false;
            uint8_t *dptr = static_cast<uint8_t *>(dst);

            switch (sformat_format(from))
            {
                case SFMT_U8:  CVT_UI_TO_XI24(uint8_t, << 16)           return true;
                case SFMT_S8:  CVT_SI_TO_XI24(uint8_t, << 16)           return true;

                case SFMT_U16: CVT_UI_TO_XI24(uint16_t, << 8)           return true;
                case SFMT_S16: CVT_SI_TO_XI24(uint16_t, << 8)           return true;

                case SFMT_U24:
                    if (sign)   CVT_SI24_TO_UI24()
                    else        ::memcpy(dptr, src, samples * sizeof(uint8_t) * 3);
                    return true;
                case SFMT_S24:
                    if (sign)   ::memcpy(dptr, src, samples * sizeof(uint8_t) * 3);
                    else        CVT_UI24_TO_SI24()
                    return true;

                case SFMT_U32: CVT_UI_TO_XI24(uint32_t, >> 16)          return true;
                case SFMT_S32: CVT_SI_TO_XI24(uint32_t, >> 16)          return true;
                case SFMT_F32: CVT_FX_TO_XI24(f32_t)                    return true;
                case SFMT_F64: CVT_FX_TO_XI24(f64_t)                    return true;

                default:
                    break;
            }

            return false;
        }

        bool convert_to_32bit(void *dst, void *src, size_t samples, size_t to, size_t from)
        {
            int sign = sformat_signed(to);
            if (sign < 0)
                return false;
            uint32_t *dptr = static_cast<uint32_t *>(dst);

            switch (sformat_format(from))
            {
                case SFMT_U8:  CVT_UI_TO_XI(uint32_t, uint8_t, << 24)   return true;
                case SFMT_S8:  CVT_SI_TO_XI(uint32_t, uint8_t, << 24)   return true;

                case SFMT_U16: CVT_UI_TO_XI(uint32_t, uint16_t, << 16)  return true;
                case SFMT_S16: CVT_SI_TO_XI(uint32_t, uint16_t, << 16)  return true;

                case SFMT_U24: CVT_U24_TO_XI(uint32_t, << 8)            return true;
                case SFMT_S24: CVT_S24_TO_XI(uint32_t, << 8)            return true;

                case SFMT_U32:
                    if (sign)   CVT_SI_TO_UI(uint32_t, uint32_t, )
                    else        ::memcpy(dptr, src, samples * sizeof(uint32_t));
                    return true;
                case SFMT_S32:
                    if (sign)   ::memcpy(dptr, src, samples * sizeof(uint32_t));
                    else        CVT_UI_TO_SI(uint32_t, uint32_t, )
                    return true;

                case SFMT_F32: CVT_F32_TO_XI(uint32_t)                  return true;
                case SFMT_F64: CVT_F64_TO_XI(uint32_t)                  return true;

                default:
                    break;
            }

            return false;
        }

        bool convert_to_f32(void *dst, void *src, size_t samples, size_t to, size_t from)
        {
            f32_t *dptr = static_cast<f32_t *>(dst);

            switch (sformat_format(from))
            {
                case SFMT_U8:  CVT_UI_TO_FX(f32_t, int8_t)              return true;
                case SFMT_S8:  CVT_UI_TO_FX(f32_t, int8_t)              return true;

                case SFMT_U16: CVT_UI_TO_FX(f32_t, int16_t)             return true;
                case SFMT_S16: CVT_UI_TO_FX(f32_t, int16_t)             return true;

                case SFMT_U24: CVT_U24_TO_FX(f32_t)                     return true;
                case SFMT_S24: CVT_S24_TO_FX(f32_t)                     return true;

                case SFMT_U32: CVT_UI_TO_FX(f32_t, int32_t)             return true;
                case SFMT_S32: CVT_UI_TO_FX(f32_t, int32_t)             return true;

                case SFMT_F32:
                    ::memcpy(dptr, src, samples * sizeof(f32_t));
                    return true;
                case SFMT_F64: CVT_FX_TO_FX(f32_t, f64_t)               return true;

                default:
                    break;
            }

            return false;
        }

        bool convert_to_f64(void *dst, void *src, size_t samples, size_t to, size_t from)
        {
            f64_t *dptr = static_cast<f64_t *>(dst);

            switch (sformat_format(from))
            {
                case SFMT_U8:  CVT_UI_TO_FX(f64_t, int8_t)              return true;
                case SFMT_S8:  CVT_UI_TO_FX(f64_t, int8_t)              return true;

                case SFMT_U16: CVT_UI_TO_FX(f64_t, int16_t)             return true;
                case SFMT_S16: CVT_UI_TO_FX(f64_t, int16_t)             return true;

                case SFMT_U24: CVT_U24_TO_FX(f64_t)                     return true;
                case SFMT_S24: CVT_S24_TO_FX(f64_t)                     return true;

                case SFMT_U32: CVT_UI_TO_FX(f64_t, int32_t)             return true;
                case SFMT_S32: CVT_UI_TO_FX(f64_t, int32_t)             return true;

                case SFMT_F32: CVT_FX_TO_FX(f64_t, f32_t)               return true;
                case SFMT_F64:
                    ::memcpy(dptr, src, samples * sizeof(f64_t));
                    return true;

                default:
                    break;
            }

            return false;
        }

        bool convert_samples(void *dst, void *src, size_t samples, size_t to, size_t from)
        {
            // Convert sample endianess
            if (!sample_endian_to_cpu(src, samples, from))
                return false;

            // Apply sample conversion
            switch (sformat_format(to))
            {
                case SFMT_U8:
                case SFMT_S8:
                    return convert_to_8bit(dst, src, samples, to, from);

                case SFMT_U16:
                case SFMT_S16:
                    return convert_to_16bit(dst, src, samples, to, from);

                case SFMT_U24:
                case SFMT_S24:
                    return convert_to_24bit(dst, src, samples, to, from);

                case SFMT_U32:
                case SFMT_S32:
                    return convert_to_32bit(dst, src, samples, to, from);

                case SFMT_F32:
                    return convert_to_f32(dst, src, samples, to, from);

                case SFMT_F64:
                    return convert_to_f64(dst, src, samples, to, from);

                default:
                    break;
            }

            return false;
        }


    }
}



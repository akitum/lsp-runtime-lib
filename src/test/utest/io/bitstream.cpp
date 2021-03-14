/*
 * Copyright (C) 2021 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2021 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-runtime-lib
 * Created on: 4 мар. 2021 г.
 *
 * lsp-runtime-lib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * lsp-runtime-lib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with lsp-runtime-lib. If not, see <https://www.gnu.org/licenses/>.
 */

#include <lsp-plug.in/test-fw/utest.h>
#include <lsp-plug.in/io/OutBitStream.h>
#include <lsp-plug.in/io/OutMemoryStream.h>
#include <lsp-plug.in/stdlib/string.h>

using namespace lsp;

UTEST_BEGIN("runtime.io", bitstream)

    void test_write_bits(io::IOutStream *os)
    {
        io::OutBitStream obs;
        UTEST_ASSERT(obs.wrap(os, WRAP_NONE) == STATUS_OK);

        static const uint8_t data[] =
        {
            0x12, 0xcf, 0xa1, 0xcf, 0x19, 0x12, 0x12, 0x6d,
            0x28, 0xa4, 0x51, 0x73, 0x15, 0xbf, 0xa7, 0xbd,
            0xa8, 0xfd, 0xcf, 0xb2, 0xf1, 0xf1, 0xd0, 0x42,
            0xe5, 0x8d, 0x05, 0x88, 0xf7, 0x32, 0x79, 0xe8
        };

        // Emit '9'
        UTEST_ASSERT(obs.writeb(true) == STATUS_OK);
        UTEST_ASSERT(obs.writev(false) == STATUS_OK);
        UTEST_ASSERT(obs.writev(false) == STATUS_OK);
        UTEST_ASSERT(obs.writeb(true) == STATUS_OK);

        // 1-byte write
        UTEST_ASSERT(obs.writev(uint8_t(0xce)) == STATUS_OK);                   // Emit 'ce'
        UTEST_ASSERT(obs.writev(uint8_t(0xa5), 4) == STATUS_OK);                // Emit '5'

        // 2-byte write
        UTEST_ASSERT(obs.writev(uint16_t(0x1324)) == STATUS_OK);                // Emit '1324'
        UTEST_ASSERT(obs.writev(uint16_t(0xfc4b), 12) == STATUS_OK);            // Emit 'c4b'

        // 4-byte write
        UTEST_ASSERT(obs.writev(uint32_t(0xa7cb329e)) == STATUS_OK);            // Emit 'a7cb329e'
        UTEST_ASSERT(obs.writev(uint32_t(0x5a8c3679), 28) == STATUS_OK);        // Emit 'a8c3679'

        // 8-byte write
        UTEST_ASSERT(obs.writev(uint64_t(0xbf61cd168a7df102ULL)) == STATUS_OK); // Emit 'bf61cd168a7df102'
        UTEST_ASSERT(obs.writev(uint64_t(0x3e561924d5993bf7), 60) == STATUS_OK);// Emit 'e561924d5993bf7'

        // Arrays
        UTEST_ASSERT(obs.write(&data[0], 9) == 9);                              // Emit '12cfa1cf1912126d28'
        UTEST_ASSERT(obs.bwrite(&data[9], 180) == 180);                         // Emit 'a4517315bfa7bda8fdcfb2f1f1d042e58d0588f732798'

        // Emit 'b'
        UTEST_ASSERT(obs.writeb(true) == STATUS_OK);
        UTEST_ASSERT(obs.writeb(false) == STATUS_OK);
        UTEST_ASSERT(obs.writeb(true) == STATUS_OK);
        UTEST_ASSERT(obs.writeb(true) == STATUS_OK);

        UTEST_ASSERT(obs.flush() == STATUS_OK);                                 // Emit '0'
        UTEST_ASSERT(obs.writev(uint32_t(0x5ec9), 12) == STATUS_OK);            // Emit 'ec9'

        UTEST_ASSERT(obs.close() == STATUS_OK);                                 // Emit '0'
    }

    UTEST_MAIN
    {
        io::OutMemoryStream oms;

        // Control sequence:
        // 9ce51324c4ba7cb329ea8c3679bf61cd168a7df102e561924d5993bf712cfa1cf1912126d28a4517315bfa7bda8fdcfb2f1f1d042e58d0588f732798

        static const uint8_t check[] =
        {
            0x9c, 0xe5, 0x13, 0x24,
            0xc4, 0xba, 0x7c, 0xb3,
            0x29, 0xea, 0x8c, 0x36,
            0x79, 0xbf, 0x61, 0xcd,
            0x16, 0x8a, 0x7d, 0xf1,
            0x02, 0xe5, 0x61, 0x92,
            0x4d, 0x59, 0x93, 0xbf,
            0x71, 0x2c, 0xfa, 0x1c,
            0xf1, 0x91, 0x21, 0x26,
            0xd2, 0x8a, 0x45, 0x17,
            0x31, 0x5b, 0xfa, 0x7b,
            0xda, 0x8f, 0xdc, 0xfb,
            0x2f, 0x1f, 0x1d, 0x04,
            0x2e, 0x58, 0xd0, 0x58,
            0x8f, 0x73, 0x27, 0x98,
            0xb0, 0xec, 0x90
        };

        test_write_bits(&oms);
        UTEST_ASSERT(oms.size() == sizeof(check));

        printf("check: ");
        for (size_t i=0; i<sizeof(check); ++i)
            printf("%02x ", check[i]);
        printf("\n");

        const uint8_t *data = oms.data();
        printf("data : ");
        for (size_t i=0; i<sizeof(check); ++i)
            printf("%02x ", data[i]);
        printf("\n");

        UTEST_ASSERT(memcmp(data, check, sizeof(check)) == 0);


        // Drop the array
        oms.drop();
    }

UTEST_END




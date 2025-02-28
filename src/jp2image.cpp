// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2021 Exiv2 authors
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */

// *****************************************************************************

// included header files
#include "config.h"

#include "jp2image.hpp"
#include "tiffimage.hpp"
#include "image.hpp"
#include "image_int.hpp"
#include "basicio.hpp"
#include "enforce.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "types.hpp"
#include "safe_op.hpp"

// + standard includes
#include <string>
#include <cstring>
#include <iostream>
#include <cassert>
#include <cstdio>

// JPEG-2000 box types
const uint32_t kJp2BoxTypeJp2Header   = 0x6a703268; // 'jp2h'
const uint32_t kJp2BoxTypeImageHeader = 0x69686472; // 'ihdr'
const uint32_t kJp2BoxTypeColorHeader = 0x636f6c72; // 'colr'
const uint32_t kJp2BoxTypeUuid        = 0x75756964; // 'uuid'
const uint32_t kJp2BoxTypeClose       = 0x6a703263; // 'jp2c'

// from openjpeg-2.1.2/src/lib/openjp2/jp2.h
/*#define JPIP_JPIP 0x6a706970*/

#define     JP2_JP   0x6a502020    /**< JPEG 2000 signature box */
#define     JP2_FTYP 0x66747970    /**< File type box */
#define     JP2_JP2H 0x6a703268    /**< JP2 header box (super-box) */
#define     JP2_IHDR 0x69686472    /**< Image header box */
#define     JP2_COLR 0x636f6c72    /**< Colour specification box */
#define     JP2_JP2C 0x6a703263    /**< Contiguous codestream box */
#define     JP2_URL  0x75726c20    /**< Data entry URL box */
#define     JP2_PCLR 0x70636c72    /**< Palette box */
#define     JP2_CMAP 0x636d6170    /**< Component Mapping box */
#define     JP2_CDEF 0x63646566    /**< Channel Definition box */
#define     JP2_DTBL 0x6474626c    /**< Data Reference box */
#define     JP2_BPCC 0x62706363    /**< Bits per component box */
#define     JP2_JP2  0x6a703220    /**< File type fields */

/* For the future */
/* #define JP2_RES 0x72657320 */  /**< Resolution box (super-box) */
/* #define JP2_JP2I 0x6a703269 */  /**< Intellectual property box */
/* #define JP2_XML  0x786d6c20 */  /**< XML box */
/* #define JP2_UUID 0x75756994 */  /**< UUID box */
/* #define JP2_UINF 0x75696e66 */  /**< UUID info box (super-box) */
/* #define JP2_ULST 0x756c7374 */  /**< UUID list box */

// JPEG-2000 UUIDs for embedded metadata
//
// See http://www.jpeg.org/public/wg1n2600.doc for information about embedding IPTC-NAA data in JPEG-2000 files
// See http://www.adobe.com/devnet/xmp/pdfs/xmp_specification.pdf for information about embedding XMP data in JPEG-2000 files
const unsigned char kJp2UuidExif[] = "JpgTiffExif->JP2";
const unsigned char kJp2UuidIptc[] = "\x33\xc7\xa4\xd2\xb8\x1d\x47\x23\xa0\xba\xf1\xa3\xe0\x97\xad\x38";
const unsigned char kJp2UuidXmp[]  = "\xbe\x7a\xcf\xcb\x97\xa9\x42\xe8\x9c\x71\x99\x94\x91\xe3\xaf\xac";

// See section B.1.1 (JPEG 2000 Signature box) of JPEG-2000 specification
const unsigned char Jp2Signature[12] = { 0x00, 0x00, 0x00, 0x0c, 0x6a, 0x50, 0x20, 0x20, 0x0d, 0x0a, 0x87, 0x0a };

const unsigned char Jp2Blank[] = { 0x00,0x00,0x00,0x0c,0x6a,0x50,0x20,0x20,0x0d,0x0a,0x87,0x0a,0x00,0x00,0x00,0x14,
                                   0x66,0x74,0x79,0x70,0x6a,0x70,0x32,0x20,0x00,0x00,0x00,0x00,0x6a,0x70,0x32,0x20,
                                   0x00,0x00,0x00,0x2d,0x6a,0x70,0x32,0x68,0x00,0x00,0x00,0x16,0x69,0x68,0x64,0x72,
                                   0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x01,0x07,0x07,0x00,0x00,0x00,0x00,
                                   0x00,0x0f,0x63,0x6f,0x6c,0x72,0x01,0x00,0x00,0x00,0x00,0x00,0x11,0x00,0x00,0x00,
                                   0x00,0x6a,0x70,0x32,0x63,0xff,0x4f,0xff,0x51,0x00,0x29,0x00,0x00,0x00,0x00,0x00,
                                   0x01,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                   0x01,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x07,
                                   0x01,0x01,0xff,0x64,0x00,0x23,0x00,0x01,0x43,0x72,0x65,0x61,0x74,0x6f,0x72,0x3a,
                                   0x20,0x4a,0x61,0x73,0x50,0x65,0x72,0x20,0x56,0x65,0x72,0x73,0x69,0x6f,0x6e,0x20,
                                   0x31,0x2e,0x39,0x30,0x30,0x2e,0x31,0xff,0x52,0x00,0x0c,0x00,0x00,0x00,0x01,0x00,
                                   0x05,0x04,0x04,0x00,0x01,0xff,0x5c,0x00,0x13,0x40,0x40,0x48,0x48,0x50,0x48,0x48,
                                   0x50,0x48,0x48,0x50,0x48,0x48,0x50,0x48,0x48,0x50,0xff,0x90,0x00,0x0a,0x00,0x00,
                                   0x00,0x00,0x00,0x2d,0x00,0x01,0xff,0x5d,0x00,0x14,0x00,0x40,0x40,0x00,0x00,0x00,
                                   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x93,0xcf,0xb4,
                                   0x04,0x00,0x80,0x80,0x80,0x80,0x80,0xff,0xd9
                                 };

//! @cond IGNORE
struct Jp2BoxHeader
{
    uint32_t length;
    uint32_t type;
};

struct Jp2ImageHeaderBox
{
    uint32_t imageHeight;
    uint32_t imageWidth;
    uint16_t componentCount;
    uint8_t  bitsPerComponent;
    uint8_t  compressionType;
    uint8_t  colorspaceIsUnknown;
    uint8_t  intellectualPropertyFlag;
    uint16_t compressionTypeProfile;
};

struct Jp2UuidBox
{
    uint8_t  uuid[16];
};
//! @endcond

// *****************************************************************************
// class member definitions
namespace Exiv2
{

    Jp2Image::Jp2Image(BasicIo::UniquePtr io, bool create)
            : Image(ImageType::jp2, mdExif | mdIptc | mdXmp, std::move(io))
    {
        if (create)
        {
            if (io_->open() == 0)
            {
#ifdef EXIV2_DEBUG_MESSAGES
                std::cerr << "Exiv2::Jp2Image:: Creating JPEG2000 image to memory" << std::endl;
#endif
                IoCloser closer(*io_);
                if (io_->write(Jp2Blank, sizeof(Jp2Blank)) != sizeof(Jp2Blank))
                {
#ifdef EXIV2_DEBUG_MESSAGES
                    std::cerr << "Exiv2::Jp2Image:: Failed to create JPEG2000 image on memory" << std::endl;
#endif
                }
            }
        }
    } // Jp2Image::Jp2Image

    std::string Jp2Image::mimeType() const
    {
        return "image/jp2";
    }

    void Jp2Image::setComment(const std::string& /*comment*/)
    {
        // Todo: implement me!
        throw(Error(kerInvalidSettingForImage, "Image comment", "JP2"));
    } // Jp2Image::setComment

    static void lf(std::ostream& out,bool& bLF)
    {
        if ( bLF ) {
            out << std::endl;
            out.flush();
            bLF = false ;
        }
    }

    static bool isBigEndian()
    {
        union {
            uint32_t i;
            char c[4];
        } e = { 0x01000000 };

        return e.c[0] != 0;
    }

    static std::string toAscii(long n)
    {
        const auto p = reinterpret_cast<const char*>(&n);
        std::string result;
        bool bBigEndian = isBigEndian();
        for ( int i = 0 ; i < 4 ; i++) {
            result += p[ bBigEndian ? i : (3-i) ];
        }
        return result;
    }

static void boxes_check(size_t b,size_t m)
{
    if ( b > m ) {
#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::Jp2Image::readMetadata box maximum exceeded" << std::endl;
#endif
        throw Error(kerCorruptedMetadata);
    }
}

    void Jp2Image::readMetadata()
    {
#ifdef EXIV2_DEBUG_MESSAGES
        std::cerr << "Exiv2::Jp2Image::readMetadata: Reading JPEG-2000 file " << io_->path() << std::endl;
#endif
        if (io_->open() != 0)
        {
            throw Error(kerDataSourceOpenFailed, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isJp2Type(*io_, true))
        {
            if (io_->error() || io_->eof()) throw Error(kerFailedToReadImageData);
            throw Error(kerNotAnImage, "JPEG-2000");
        }

        long              position  = 0;
        Jp2BoxHeader      box       = {0,0};
        Jp2BoxHeader      subBox    = {0,0};
        Jp2ImageHeaderBox ihdr      = {0,0,0,0,0,0,0,0};
        Jp2UuidBox        uuid      = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
        size_t            boxes     = 0 ;
        size_t            boxem     = 1000 ; // boxes max

        while (io_->read(reinterpret_cast<byte*>(&box), sizeof(box)) == sizeof(box)) {
            boxes_check(boxes++,boxem );
            position   = io_->tell();
            box.length = getLong(reinterpret_cast<byte*>(&box.length), bigEndian);
            box.type = getLong(reinterpret_cast<byte*>(&box.type), bigEndian);
#ifdef EXIV2_DEBUG_MESSAGES
            std::cout << "Exiv2::Jp2Image::readMetadata: "
                      << "Position: " << position
                      << " box type: " << toAscii(box.type)
                      << " length: " << box.length
                      << std::endl;
#endif
            enforce(box.length <= sizeof(box)+io_->size()-io_->tell() , Exiv2::kerCorruptedMetadata);

            if (box.length == 0) return ;

            if (box.length == 1)
            {
                // FIXME. Special case. the real box size is given in another place.
            }

            switch(box.type)
            {
                case kJp2BoxTypeJp2Header:
                {
#ifdef EXIV2_DEBUG_MESSAGES
                    std::cout << "Exiv2::Jp2Image::readMetadata: JP2Header box found" << std::endl;
#endif
                    long restore = io_->tell();

                    while (io_->read(reinterpret_cast<byte*>(&subBox), sizeof(subBox)) == sizeof(subBox) &&
                           subBox.length) {
                        boxes_check(boxes++, boxem) ;
                        subBox.length = getLong(reinterpret_cast<byte*>(&subBox.length), bigEndian);
                        subBox.type = getLong(reinterpret_cast<byte*>(&subBox.type), bigEndian);
                        if (subBox.length > io_->size() ) {
                            throw Error(kerCorruptedMetadata);
                        }
#ifdef EXIV2_DEBUG_MESSAGES
                        std::cout << "Exiv2::Jp2Image::readMetadata: "
                        << "subBox = " << toAscii(subBox.type) << " length = " << subBox.length << std::endl;
#endif
                        if(subBox.type == kJp2BoxTypeColorHeader && subBox.length != 15)
                        {
#ifdef EXIV2_DEBUG_MESSAGES
                            std::cout << "Exiv2::Jp2Image::readMetadata: "
                                     << "Color data found" << std::endl;
#endif

                            const long pad = 3 ; // 3 padding bytes 2 0 0
                            const size_t data_length = Safe::add(subBox.length, static_cast<uint32_t>(8));
                            // data_length makes no sense if it is larger than the rest of the file
                            if (data_length > io_->size() - io_->tell()) {
                                throw Error(kerCorruptedMetadata);
                            }
                            DataBuf data(static_cast<long>(data_length));
                            io_->read(data.data(), data.size());
                            const long    iccLength = data.read_uint32(pad, bigEndian);
                            // subtracting pad from data.size() is safe:
                            // data.size() is at least 8 and pad = 3
                            if (iccLength > data.size() - pad) {
                                throw Error(kerCorruptedMetadata);
                            }
                            DataBuf icc(iccLength);
                            icc.copyBytes(0, data.c_data(pad), icc.size());
#ifdef EXIV2_DEBUG_MESSAGES
                            const char* iccPath = "/tmp/libexiv2_jp2.icc";
                            FILE* f = fopen(iccPath,"wb");
                            if ( f ) {
                                fwrite(icc.c_data(),icc.size(),1,f);
                                fclose(f);
                            }
                            std::cout << "Exiv2::Jp2Image::readMetadata: wrote iccProfile " << icc.size() << " bytes to " << iccPath << std::endl ;
#endif
                            setIccProfile(icc);
                        }

                        if( subBox.type == kJp2BoxTypeImageHeader)
                        {
                            io_->read(reinterpret_cast<byte*>(&ihdr), sizeof(ihdr));
#ifdef EXIV2_DEBUG_MESSAGES
                            std::cout << "Exiv2::Jp2Image::readMetadata: Ihdr data found" << std::endl;
#endif
                            ihdr.imageHeight = getLong(reinterpret_cast<byte*>(&ihdr.imageHeight), bigEndian);
                            ihdr.imageWidth = getLong(reinterpret_cast<byte*>(&ihdr.imageWidth), bigEndian);
                            ihdr.componentCount = getShort(reinterpret_cast<byte*>(&ihdr.componentCount), bigEndian);
                            ihdr.compressionTypeProfile =
                                getShort(reinterpret_cast<byte*>(&ihdr.compressionTypeProfile), bigEndian);

                            pixelWidth_  = ihdr.imageWidth;
                            pixelHeight_ = ihdr.imageHeight;
                        }

                        io_->seek(restore,BasicIo::beg);
                        if ( io_->seek(subBox.length, Exiv2::BasicIo::cur) != 0 ) {
                            throw Error(kerCorruptedMetadata);
                        }
                        restore = io_->tell();
                    }
                    break;
                }

                case kJp2BoxTypeUuid:
                {
#ifdef EXIV2_DEBUG_MESSAGES
                    std::cout << "Exiv2::Jp2Image::readMetadata: UUID box found" << std::endl;
#endif

                    if (io_->read(reinterpret_cast<byte*>(&uuid), sizeof(uuid)) == sizeof(uuid)) {
                        DataBuf rawData;
                        long    bufRead;
                        bool    bIsExif = memcmp(uuid.uuid, kJp2UuidExif, sizeof(uuid))==0;
                        bool    bIsIPTC = memcmp(uuid.uuid, kJp2UuidIptc, sizeof(uuid))==0;
                        bool    bIsXMP  = memcmp(uuid.uuid, kJp2UuidXmp , sizeof(uuid))==0;

                        if(bIsExif)
                        {
#ifdef EXIV2_DEBUG_MESSAGES
                           std::cout << "Exiv2::Jp2Image::readMetadata: Exif data found" << std::endl ;
#endif
                            enforce(box.length >= sizeof(box) + sizeof(uuid), kerCorruptedMetadata);
                            rawData.alloc(box.length - (sizeof(box) + sizeof(uuid)));
                            bufRead = io_->read(rawData.data(), rawData.size());
                            if (io_->error()) throw Error(kerFailedToReadImageData);
                            if (bufRead != rawData.size()) throw Error(kerInputDataReadFailed);

                            if (rawData.size() > 8) // "II*\0long"
                            {
                                // Find the position of Exif header in bytes array.
                                const char a = rawData.read_uint8(0);
                                const char b = rawData.read_uint8(1);
                                long pos = (a == b && (a=='I' || a=='M')) ? 0 : -1;

                                // #1242  Forgive having Exif\0\0 in rawData.pData_
                                const byte exifHeader[] = { 0x45, 0x78, 0x69, 0x66, 0x00, 0x00 };
                                for (long i = 0; pos < 0 && i < rawData.size() - static_cast<long>(sizeof(exifHeader));
                                     i++) {
                                    if (rawData.cmpBytes(i, exifHeader, sizeof(exifHeader)) == 0)
                                    {
                                        pos = i+sizeof(exifHeader);
#ifndef SUPPRESS_WARNINGS
                                        EXV_WARNING << "Reading non-standard UUID-EXIF_bad box in " << io_->path() << std::endl;
#endif

                                    }
                                }

                                // If found it, store only these data at from this place.
                                if (pos >= 0 )
                                {
#ifdef EXIV2_DEBUG_MESSAGES
                                    std::cout << "Exiv2::Jp2Image::readMetadata: Exif header found at position " << pos << std::endl;
#endif
                                    ByteOrder bo = TiffParser::decode(exifData(),
                                                                      iptcData(),
                                                                      xmpData(),
                                                                      rawData.c_data(pos),
                                                                      rawData.size() - pos);
                                    setByteOrder(bo);
                                }
                            }
                            else
                            {
#ifndef SUPPRESS_WARNINGS
                                EXV_WARNING << "Failed to decode Exif metadata." << std::endl;
#endif
                                exifData_.clear();
                            }
                        }

                        if(bIsIPTC)
                        {
#ifdef EXIV2_DEBUG_MESSAGES
                           std::cout << "Exiv2::Jp2Image::readMetadata: Iptc data found" << std::endl;
#endif
                            enforce(box.length >= sizeof(box) + sizeof(uuid), kerCorruptedMetadata);
                            rawData.alloc(box.length - (sizeof(box) + sizeof(uuid)));
                            bufRead = io_->read(rawData.data(), rawData.size());
                            if (io_->error()) throw Error(kerFailedToReadImageData);
                            if (bufRead != rawData.size()) throw Error(kerInputDataReadFailed);

                            if (IptcParser::decode(iptcData_, rawData.c_data(), rawData.size()))
                            {
#ifndef SUPPRESS_WARNINGS
                                EXV_WARNING << "Failed to decode IPTC metadata." << std::endl;
#endif
                                iptcData_.clear();
                            }
                        }

                        if(bIsXMP)
                        {
#ifdef EXIV2_DEBUG_MESSAGES
                           std::cout << "Exiv2::Jp2Image::readMetadata: Xmp data found" << std::endl;
#endif
                           enforce(box.length >= sizeof(box) + sizeof(uuid), kerCorruptedMetadata);
                           rawData.alloc(box.length - static_cast<uint32_t>(sizeof(box) + sizeof(uuid)));
                           bufRead = io_->read(rawData.data(), rawData.size());
                           if (io_->error())
                               throw Error(kerFailedToReadImageData);
                           if (bufRead != rawData.size())
                               throw Error(kerInputDataReadFailed);
                           xmpPacket_.assign(rawData.c_str(), rawData.size());

                           std::string::size_type idx = xmpPacket_.find_first_of('<');
                           if (idx != std::string::npos && idx > 0) {
#ifndef SUPPRESS_WARNINGS
                                EXV_WARNING << "Removing " << static_cast<uint32_t>(idx)
                                            << " characters from the beginning of the XMP packet" << std::endl;
#endif
                                xmpPacket_ = xmpPacket_.substr(idx);
                           }

                            if (!xmpPacket_.empty() && XmpParser::decode(xmpData_, xmpPacket_)) {
#ifndef SUPPRESS_WARNINGS
                                EXV_WARNING << "Failed to decode XMP metadata." << std::endl;
#endif
                            }
                        }
                    }
                    break;
                }

                default:
                {
                    break;
                }
            }

            // Move to the next box.
            io_->seek(static_cast<long>(position - sizeof(box) + box.length), BasicIo::beg);
            if (io_->error()) throw Error(kerFailedToReadImageData);
        }

    } // Jp2Image::readMetadata

    void Jp2Image::printStructure(std::ostream& out, PrintStructureOption option, int depth)
    {
        if (io_->open() != 0)
            throw Error(kerDataSourceOpenFailed, io_->path(), strError());

        // Ensure that this is the correct image type
        if (!isJp2Type(*io_, false)) {
            if (io_->error() || io_->eof())
                throw Error(kerFailedToReadImageData);
            throw Error(kerNotAJpeg);
        }

        bool bPrint = option == kpsBasic || option == kpsRecursive;
        bool bRecursive = option == kpsRecursive;
        bool bICC = option == kpsIccProfile;
        bool bXMP = option == kpsXMP;
        bool bIPTCErase = option == kpsIptcErase;

        if (bPrint) {
            out << "STRUCTURE OF JPEG2000 FILE: " << io_->path() << std::endl;
            out << " address |   length | box       | data" << std::endl;
        }

        if ( bPrint || bXMP || bICC || bIPTCErase ) {

            long              position  = 0;
            Jp2BoxHeader      box       = {1,1};
            Jp2BoxHeader      subBox    = {1,1};
            Jp2UuidBox        uuid      = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
            bool              bLF       = false;

            while (box.length && box.type != kJp2BoxTypeClose &&
                   io_->read(reinterpret_cast<byte*>(&box), sizeof(box)) == sizeof(box)) {
                position   = io_->tell();
                box.length = getLong(reinterpret_cast<byte*>(&box.length), bigEndian);
                box.type = getLong(reinterpret_cast<byte*>(&box.type), bigEndian);
                enforce(box.length <= sizeof(box)+io_->size()-io_->tell() , Exiv2::kerCorruptedMetadata);

                if (bPrint) {
                    out << Internal::stringFormat("%8ld | %8ld | ", position - sizeof(box),
                                                  static_cast<size_t>(box.length))
                        << toAscii(box.type) << "      | ";
                    bLF = true;
                    if (box.type == kJp2BoxTypeClose)
                        lf(out, bLF);
                }
                if (box.type == kJp2BoxTypeClose)
                    break;

                switch (box.type) {
                    case kJp2BoxTypeJp2Header: {
                        lf(out, bLF);

                        while (io_->read(reinterpret_cast<byte*>(&subBox), sizeof(subBox)) == sizeof(subBox) &&
                               io_->tell() < position + static_cast<long>(box.length))  // don't read beyond the box!
                        {
                            const size_t address = io_->tell() - sizeof(subBox);
                            subBox.length = getLong(reinterpret_cast<byte*>(&subBox.length), bigEndian);
                            subBox.type = getLong(reinterpret_cast<byte*>(&subBox.type), bigEndian);

                            if (subBox.length < sizeof(box) || subBox.length > io_->size() - io_->tell()) {
                                throw Error(kerCorruptedMetadata);
                            }

                            DataBuf data(subBox.length - sizeof(box));
                            io_->read(data.data(), data.size());
                            if (bPrint) {
                                out << Internal::stringFormat("%8ld | %8ld |  sub:", static_cast<size_t>(address),
                                                              static_cast<size_t>(subBox.length))
                                    << toAscii(subBox.type) << " | "
                                    << Internal::binaryToString(makeSlice(data, 0, std::min(30L, data.size())));
                                bLF = true;
                            }

                            if (subBox.type == kJp2BoxTypeColorHeader) {
                                long pad = 3;  // don't know why there are 3 padding bytes

                                // Bounds-check for the `getULong()` below, which reads 4 bytes, starting at `pad`.
                                enforce(data.size() >= pad + 4, kerCorruptedMetadata);

                                if (bPrint) {
                                    out << " | pad:";
                                    for (int i = 0; i < 3; i++)
                                        out << " " << static_cast<int>(data.read_uint8(i));
                                }
                                long iccLength = data.read_uint32(pad, bigEndian);
                                if (bPrint) {
                                    out << " | iccLength:" << iccLength;
                                }
                                enforce(iccLength <= data.size() - pad, kerCorruptedMetadata);
                                if (bICC) {
                                    out.write(data.c_str(pad), iccLength);
                                }
                            }
                            lf(out, bLF);
                        }
                    } break;

                    case kJp2BoxTypeUuid: {
                        if (io_->read(reinterpret_cast<byte*>(&uuid), sizeof(uuid)) == sizeof(uuid)) {
                            bool bIsExif = memcmp(uuid.uuid, kJp2UuidExif, sizeof(uuid)) == 0;
                            bool bIsIPTC = memcmp(uuid.uuid, kJp2UuidIptc, sizeof(uuid)) == 0;
                            bool bIsXMP = memcmp(uuid.uuid, kJp2UuidXmp, sizeof(uuid)) == 0;

                            bool bUnknown = !(bIsExif || bIsIPTC || bIsXMP);

                            if (bPrint) {
                                if (bIsExif)
                                    out << "Exif: ";
                                if (bIsIPTC)
                                    out << "IPTC: ";
                                if (bIsXMP)
                                    out << "XMP : ";
                                if (bUnknown)
                                    out << "????: ";
                            }

                            DataBuf rawData;
                            enforce(box.length >= sizeof(uuid) + sizeof(box), kerCorruptedMetadata);
                            rawData.alloc(box.length - sizeof(uuid) - sizeof(box));
                            long bufRead = io_->read(rawData.data(), rawData.size());
                            if (io_->error())
                                throw Error(kerFailedToReadImageData);
                            if (bufRead != rawData.size())
                                throw Error(kerInputDataReadFailed);

                            if (bPrint) {
                                out << Internal::binaryToString(
                                        makeSlice(rawData, 0, rawData.size()>40?40:rawData.size()));
                                out.flush();
                            }
                            lf(out, bLF);

                            if (bIsExif && bRecursive && rawData.size() > 8) { // "II*\0long"
                                const char a = rawData.read_uint8(0);
                                const char b = rawData.read_uint8(1);
                                if (a == b && (a == 'I' || a == 'M')) {
                                    BasicIo::UniquePtr p = BasicIo::UniquePtr(new MemIo(rawData.c_data(), rawData.size()));
                                    printTiffStructure(*p, out, option, depth);
                                }
                            }

                            if (bIsIPTC && bRecursive) {
                                IptcData::printStructure(out, makeSlice(rawData, 0, rawData.size()), depth);
                            }

                            if (bIsXMP && bXMP) {
                                out.write(rawData.c_str(), rawData.size());
                            }
                        }
                    } break;

                    default:
                        break;
                }

                // Move to the next box.
                io_->seek(static_cast<long>(position - sizeof(box) + box.length), BasicIo::beg);
                if (io_->error())
                    throw Error(kerFailedToReadImageData);
                if (bPrint)
                    lf(out, bLF);
            }
        }
    }  // JpegBase::printStructure

    void Jp2Image::writeMetadata()
    {
        if (io_->open() != 0)
        {
            throw Error(kerDataSourceOpenFailed, io_->path(), strError());
        }
        IoCloser closer(*io_);
        BasicIo::UniquePtr tempIo(new MemIo);
        assert (tempIo.get() != 0);

        doWriteMetadata(*tempIo); // may throw
        io_->close();
        io_->transfer(*tempIo); // may throw

    } // Jp2Image::writeMetadata

#ifdef __clang__
// ignore cast align errors.  dataBuf.pData_ is allocated by malloc() and 4 (or 8 byte aligned).
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-align"
#endif

    void Jp2Image::encodeJp2Header(const DataBuf& boxBuf,DataBuf& outBuf)
    {
        DataBuf output(boxBuf.size() + iccProfile_.size() + 100); // allocate sufficient space
        long    outlen = sizeof(Jp2BoxHeader) ; // now many bytes have we written to output?
        long    inlen = sizeof(Jp2BoxHeader) ; // how many bytes have we read from boxBuf?
        enforce(sizeof(Jp2BoxHeader) <= static_cast<size_t>(output.size()), Exiv2::kerCorruptedMetadata);
        auto pBox = reinterpret_cast<const Jp2BoxHeader*>(boxBuf.c_data());
        uint32_t length = getLong(reinterpret_cast<const byte*>(&pBox->length), bigEndian);
        enforce(length <= static_cast<size_t>(output.size()), Exiv2::kerCorruptedMetadata);
        uint32_t      count  = sizeof (Jp2BoxHeader);
        auto p = boxBuf.c_str();
        bool          bWroteColor = false ;

        while ( count < length && !bWroteColor ) {
            enforce(sizeof(Jp2BoxHeader) <= length - count, Exiv2::kerCorruptedMetadata);
            auto pSubBox = reinterpret_cast<const Jp2BoxHeader*>(p + count);

            // copy data.  pointer could be into a memory mapped file which we will decode!
            Jp2BoxHeader   subBox ; memcpy(&subBox,pSubBox,sizeof(subBox));
            Jp2BoxHeader   newBox =  subBox;

            if ( count < length ) {
                subBox.length = getLong(reinterpret_cast<byte*>(&subBox.length), bigEndian);
                subBox.type = getLong(reinterpret_cast<byte*>(&subBox.type), bigEndian);
#ifdef EXIV2_DEBUG_MESSAGES
                std::cout << "Jp2Image::encodeJp2Header subbox: "<< toAscii(subBox.type) << " length = " << subBox.length << std::endl;
#endif
                enforce(subBox.length > 0, Exiv2::kerCorruptedMetadata);
                enforce(subBox.length <= length - count, Exiv2::kerCorruptedMetadata);
                count        += subBox.length;
                newBox.type   = subBox.type;
            } else {
                subBox.length=0;
                newBox.type = kJp2BoxTypeColorHeader;
                count = length;
            }

            uint32_t newlen = subBox.length;
            if ( newBox.type == kJp2BoxTypeColorHeader ) {
                bWroteColor = true ;
                if ( ! iccProfileDefined() ) {
                    const char* pad   = "\x01\x00\x00\x00\x00\x00\x10\x00\x00\x05\x1cuuid";
                    uint32_t    psize = 15;
                    newlen            = sizeof(newBox) + psize ;
                    enforce(newlen <= static_cast<size_t>(output.size() - outlen), Exiv2::kerCorruptedMetadata);
                    ul2Data(reinterpret_cast<byte*>(&newBox.length), psize, bigEndian);
                    ul2Data(reinterpret_cast<byte*>(&newBox.type), newBox.type, bigEndian);
                    output.copyBytes(outlen                     ,&newBox            ,sizeof(newBox));
                    output.copyBytes(outlen+sizeof(newBox)      ,pad                ,psize         );
                } else {
                    const char* pad   = "\x02\x00\x00";
                    uint32_t    psize = 3;
                    newlen            = sizeof(newBox) + psize + iccProfile_.size();
                    enforce(newlen <= static_cast<size_t>(output.size() - outlen), Exiv2::kerCorruptedMetadata);
                    ul2Data(reinterpret_cast<byte*>(&newBox.length), newlen, bigEndian);
                    ul2Data(reinterpret_cast<byte*>(&newBox.type), newBox.type, bigEndian);
                    output.copyBytes(outlen                     ,&newBox            ,sizeof(newBox)  );
                    output.copyBytes(outlen+sizeof(newBox)      , pad               ,psize           );
                    output.copyBytes(outlen+sizeof(newBox)+psize,iccProfile_.c_data(),iccProfile_.size());
                }
            } else {
                enforce(newlen <= static_cast<size_t>(output.size() - outlen), Exiv2::kerCorruptedMetadata);
                output.copyBytes(outlen,boxBuf.c_data(inlen),subBox.length);
            }

            outlen += newlen;
            inlen  += subBox.length;
        }

        // allocate the correct number of bytes, copy the data and update the box header
        outBuf.alloc(outlen);
        outBuf.copyBytes(0,output.c_data(),outlen);
        auto oBox = reinterpret_cast<Jp2BoxHeader*>(outBuf.data());
        ul2Data(reinterpret_cast<byte*>(&oBox->type), kJp2BoxTypeJp2Header, bigEndian);
        ul2Data(reinterpret_cast<byte*>(&oBox->length), outlen, bigEndian);
    } // Jp2Image::encodeJp2Header

#ifdef __clang__
#pragma clang diagnostic pop
#endif

    void Jp2Image::doWriteMetadata(BasicIo& outIo)
    {
        if (!io_->isopen()) throw Error(kerInputDataReadFailed);
        if (!outIo.isopen()) throw Error(kerImageWriteFailed);

#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::Jp2Image::doWriteMetadata: Writing JPEG-2000 file " << io_->path() << std::endl;
        std::cout << "Exiv2::Jp2Image::doWriteMetadata: tmp file created " << outIo.path() << std::endl;
#endif

        // Ensure that this is the correct image type
        if (!isJp2Type(*io_, true))
        {
            if (io_->error() || io_->eof()) throw Error(kerInputDataReadFailed);
            throw Error(kerNoImageInInputData);
        }

        // Write JPEG2000 Signature.
        if (outIo.write(Jp2Signature, 12) != 12) throw Error(kerImageWriteFailed);

        Jp2BoxHeader box = {0,0};

        byte    boxDataSize[4];
        byte    boxUUIDtype[4];
        DataBuf bheaderBuf(8);     // Box header : 4 bytes (data size) + 4 bytes (box type).

        // FIXME: Andreas, why the loop do not stop when EOF is taken from _io. The loop go out by an exception
        // generated by a zero size data read.

        while (io_->tell() < static_cast<long>(io_->size())) {
#ifdef EXIV2_DEBUG_MESSAGES
            std::cout << "Exiv2::Jp2Image::doWriteMetadata: Position: " << io_->tell() << " / " << io_->size() << std::endl;
#endif

            // Read chunk header.

            bheaderBuf.clear();
            long bufRead = io_->read(bheaderBuf.data(), bheaderBuf.size());
            if (io_->error()) throw Error(kerFailedToReadImageData);
            if (bufRead != bheaderBuf.size()) throw Error(kerInputDataReadFailed);

            // Decode box header.

            box.length = bheaderBuf.read_uint32(0, bigEndian);
            box.type   = bheaderBuf.read_uint32(4, bigEndian);

#ifdef EXIV2_DEBUG_MESSAGES
            std::cout << "Exiv2::Jp2Image::doWriteMetadata: box type: " << toAscii(box.type)
                      << " length: " << box.length << std::endl;
#endif

            if (box.length == 0)
            {
#ifdef EXIV2_DEBUG_MESSAGES
                std::cout << "Exiv2::Jp2Image::doWriteMetadata: Null Box size has been found. "
                             "This is the last box of file." << std::endl;
#endif
                box.length = static_cast<uint32_t>(io_->size() - io_->tell() + 8);
            }
            if (box.length < 8)
            {
                // box is broken, so there is nothing we can do here
                throw Error(kerCorruptedMetadata);
            }

            // Prevent a malicious file from causing a large memory allocation.
            enforce(box.length - 8 <= static_cast<size_t>(io_->size() - io_->tell()), kerCorruptedMetadata);

            // Read whole box : Box header + Box data (not fixed size - can be null).
            DataBuf boxBuf(box.length);                             // Box header (8 bytes) + box data.
            boxBuf.copyBytes(0, bheaderBuf.c_data(), 8);        // Copy header.
            bufRead = io_->read(boxBuf.data(8), box.length - 8); // Extract box data.
            if (io_->error())
            {
#ifdef EXIV2_DEBUG_MESSAGES
                std::cout << "Exiv2::Jp2Image::doWriteMetadata: Error reading source file" << std::endl;
#endif

                throw Error(kerFailedToReadImageData);
            }

            if (bufRead != static_cast<long>(box.length - 8)) {
#ifdef EXIV2_DEBUG_MESSAGES
                std::cout << "Exiv2::Jp2Image::doWriteMetadata: Cannot read source file data" << std::endl;
#endif
                throw Error(kerInputDataReadFailed);
            }

            switch(box.type)
            {
                case kJp2BoxTypeJp2Header:
                {
                    DataBuf newBuf;
                    encodeJp2Header(boxBuf,newBuf);
#ifdef EXIV2_DEBUG_MESSAGES
                    std::cout << "Exiv2::Jp2Image::doWriteMetadata: Write JP2Header box (length: " << box.length << ")" << std::endl;
#endif
                    if (outIo.write(newBuf.data(), newBuf.size()) != newBuf.size()) throw Error(kerImageWriteFailed);

                    // Write all updated metadata here, just after JP2Header.

                    if (exifData_.count() > 0)
                    {
                        // Update Exif data to a new UUID box

                        Blob blob;
                        ExifParser::encode(blob, littleEndian, exifData_);
                        if (!blob.empty()) {
                            DataBuf rawExif(static_cast<long>(blob.size()));
                            rawExif.copyBytes(0, &blob[0], blob.size());

                            DataBuf boxData(8 + 16 + rawExif.size());
                            ul2Data(boxDataSize, boxData.size(), Exiv2::bigEndian);
                            ul2Data(boxUUIDtype, kJp2BoxTypeUuid, Exiv2::bigEndian);
                            boxData.copyBytes(0,      boxDataSize,       4);
                            boxData.copyBytes(4,      boxUUIDtype,       4);
                            boxData.copyBytes(8,      kJp2UuidExif,      16);
                            boxData.copyBytes(8 + 16, rawExif.c_data(), rawExif.size());

#ifdef EXIV2_DEBUG_MESSAGES
                            std::cout << "Exiv2::Jp2Image::doWriteMetadata: Write box with Exif metadata (length: "
                                      << boxData.size() << std::endl;
#endif
                            if (outIo.write(boxData.c_data(), boxData.size()) != boxData.size()) throw Error(kerImageWriteFailed);
                        }
                    }

                    if (iptcData_.count() > 0)
                    {
                        // Update Iptc data to a new UUID box

                        DataBuf rawIptc = IptcParser::encode(iptcData_);
                        if (rawIptc.size() > 0)
                        {
                            DataBuf boxData(8 + 16 + rawIptc.size());
                            ul2Data(boxDataSize, boxData.size(), Exiv2::bigEndian);
                            ul2Data(boxUUIDtype, kJp2BoxTypeUuid, Exiv2::bigEndian);
                            boxData.copyBytes(0,      boxDataSize,    4);
                            boxData.copyBytes(4,      boxUUIDtype,    4);
                            boxData.copyBytes(8,      kJp2UuidIptc,   16);
                            boxData.copyBytes(8 + 16, rawIptc.c_data(), rawIptc.size());

#ifdef EXIV2_DEBUG_MESSAGES
                            std::cout << "Exiv2::Jp2Image::doWriteMetadata: Write box with Iptc metadata (length: "
                                      << boxData.size() << std::endl;
#endif
                            if (outIo.write(boxData.c_data(), boxData.size()) != boxData.size()) throw Error(kerImageWriteFailed);
                        }
                    }

                    if (!writeXmpFromPacket()) {
                        if (XmpParser::encode(xmpPacket_, xmpData_) > 1)
                        {
#ifndef SUPPRESS_WARNINGS
                            EXV_ERROR << "Failed to encode XMP metadata." << std::endl;
#endif
                        }
                    }
                    if (!xmpPacket_.empty()) {
                        // Update Xmp data to a new UUID box

                        DataBuf xmp(reinterpret_cast<const byte*>(xmpPacket_.data()), static_cast<long>(xmpPacket_.size()));
                        DataBuf boxData(8 + 16 + xmp.size());
                        ul2Data(boxDataSize, boxData.size(), Exiv2::bigEndian);
                        ul2Data(boxUUIDtype, kJp2BoxTypeUuid, Exiv2::bigEndian);
                        boxData.copyBytes(0,      boxDataSize,   4);
                        boxData.copyBytes(4,      boxUUIDtype,   4);
                        boxData.copyBytes(8,      kJp2UuidXmp,   16);
                        boxData.copyBytes(8 + 16, xmp.c_data(), xmp.size());

#ifdef EXIV2_DEBUG_MESSAGES
                        std::cout << "Exiv2::Jp2Image::doWriteMetadata: Write box with XMP metadata (length: "
                                  << boxData.size() << ")" << std::endl;
#endif
                        if (outIo.write(boxData.c_data(), boxData.size()) != boxData.size()) throw Error(kerImageWriteFailed);
                    }

                    break;
                }

                case kJp2BoxTypeUuid:
                {
                    enforce(boxBuf.size() >= 24, Exiv2::kerCorruptedMetadata);
                    if (boxBuf.cmpBytes(8, kJp2UuidExif, 16) == 0)
                    {
#ifdef EXIV2_DEBUG_MESSAGES
                        std::cout << "Exiv2::Jp2Image::doWriteMetadata: strip Exif Uuid box" << std::endl;
#endif
                    }
                    else if (boxBuf.cmpBytes(8, kJp2UuidIptc, 16) == 0)
                    {
#ifdef EXIV2_DEBUG_MESSAGES
                        std::cout << "Exiv2::Jp2Image::doWriteMetadata: strip Iptc Uuid box" << std::endl;
#endif
                    }
                    else if (boxBuf.cmpBytes(8, kJp2UuidXmp,  16) == 0)
                    {
#ifdef EXIV2_DEBUG_MESSAGES
                        std::cout << "Exiv2::Jp2Image::doWriteMetadata: strip Xmp Uuid box" << std::endl;
#endif
                    }
                    else
                    {
#ifdef EXIV2_DEBUG_MESSAGES
                        std::cout << "Exiv2::Jp2Image::doWriteMetadata: write Uuid box (length: " << box.length << ")" << std::endl;
#endif
                        if (outIo.write(boxBuf.c_data(), boxBuf.size()) != boxBuf.size()) throw Error(kerImageWriteFailed);
                    }
                    break;
                }

                default:
                {
#ifdef EXIV2_DEBUG_MESSAGES
                    std::cout << "Exiv2::Jp2Image::doWriteMetadata: write box (length: " << box.length << ")" << std::endl;
#endif
                    if (outIo.write(boxBuf.c_data(), boxBuf.size()) != boxBuf.size()) throw Error(kerImageWriteFailed);

                    break;
                }
            }
        }

#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::Jp2Image::doWriteMetadata: EOF" << std::endl;
#endif

    } // Jp2Image::doWriteMetadata

    // *************************************************************************
    // free functions
    Image::UniquePtr newJp2Instance(BasicIo::UniquePtr io, bool create)
    {
        Image::UniquePtr image(new Jp2Image(std::move(io), create));
        if (!image->good())
        {
            image.reset();
        }
        return image;
    }

    bool isJp2Type(BasicIo& iIo, bool advance)
    {
        const int32_t len = 12;
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof())
        {
            return false;
        }
        bool matched = (memcmp(buf, Jp2Signature, len) == 0);
        if (!advance || !matched)
        {
            iIo.seek(-len, BasicIo::cur);
        }
        return matched;
    }
}                                       // namespace Exiv2

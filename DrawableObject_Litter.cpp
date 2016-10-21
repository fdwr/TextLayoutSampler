#if 0
#include "D:\src\TextLayoutSampler\temphack\Offset.h"
#include "D:\src\TextLayoutSampler\temphack\ToolCheckedPtr.h"
#include "D:\src\TextLayoutSampler\temphack\ToolByteOrder.h"
#include "D:\src\TextLayoutSampler\temphack\ToolOpenTypeDefs.h"
#include "D:\src\TextLayoutSampler\temphack\ToolOpenTypeFaceData.h"
#include "D:\src\TextLayoutSampler\temphack\FontFaceHelpers.h"
#endif

#if 0
/// <summary>
/// Fonts may contain multiple drawable data formats for glyphs. These flags specify which formats
/// are supported in the font, either at a font-wide level or per glyph, and the app uses them to
/// tell DWrite which formats to return when asking for that data or splitting the glyph run.
/// Note that some tables can return multiple different formats (the sbix table might have JPEG
/// data for one glyph and PNG data for a different glyph) and different tables can return the
/// same format (the CBDT and sbix table both return PNG data).
/// </summary>
enum DWRITE_GLYPH_IMAGE_FORMATS
{
    /// <summary>
    /// Indicates no data is available for this glyph.
    /// </summary>
    DWRITE_GLYPH_IMAGE_FORMATS_NONE        = 0x00000000,

    /// <summary>
    /// The glyph has TrueType outlines.
    /// </summary>
    DWRITE_GLYPH_IMAGE_FORMATS_TRUETYPE    = 0x00000001,

    /// <summary>
    /// The glyph has CFF outlines.
    /// </summary>
    DWRITE_GLYPH_IMAGE_FORMATS_CFF         = 0x00000002,

    /// <summary>
    /// The glyph has multilayered COLR data.
    /// </summary>
    DWRITE_GLYPH_IMAGE_FORMATS_COLR        = 0x00000004,

    /// <summary>
    /// The glyph has SVG outlines, as standard XML.
    /// </summary>
    DWRITE_GLYPH_IMAGE_FORMATS_SVG         = 0x00000008,

    /// <summary>
    /// The glyph has PNG image data, with standard PNG IHDR.
    /// </summary>
    DWRITE_GLYPH_IMAGE_FORMATS_PNG         = 0x00000010,

    /// <summary>
    /// The glyph has JPEG image data, with standard JIFF header.
    /// </summary>
    DWRITE_GLYPH_IMAGE_FORMATS_JPEG        = 0x00000020,

    /// <summary>
    /// The glyph has TIFF image data.
    /// </summary>
    DWRITE_GLYPH_IMAGE_FORMATS_TIFF        = 0x00000040,

    /// <summary>
    /// The glyph has raw 32-bit premultiplied BGRA data.
    /// </summary>
    DWRITE_GLYPH_IMAGE_FORMATS_PREMULTIPLIED_B8G8R8A8 = 0x00000080,
};

#ifdef DEFINE_ENUM_FLAG_OPERATORS
DEFINE_ENUM_FLAG_OPERATORS(DWRITE_GLYPH_IMAGE_FORMATS);
#endif

#endif


#if 0
FontTablePtr::FontTablePtr(
    IDWriteFontFace* fontFace,
    uint32_t openTypeTableTag
    )
{
    if (fontFace == nullptr)
        return;

    const void* tableData;
    uint32_t tableSize;
    BOOL exists = false;

    fontFace->TryGetFontTable(
        openTypeTableTag,
        &tableData,
        OUT &tableSize,
        OUT &tableContext_,
        OUT &exists
        );

    if (exists)
    {
        *static_cast<Base*>(this) = Base(const_cast<void*>(tableData), tableSize);
        fontFace_ = fontFace;
    }
}


void FontTablePtr::swap(FontTablePtr& other)
{
    Base& base = *this;
    Base& otherBase = other;
    std::swap(base, otherBase);
    other.fontFace_.swap(fontFace_);
    std::swap(tableContext_, other.tableContext_);
}


FontTablePtr::~FontTablePtr()
{
    if (fontFace_ != nullptr)
    {
        fontFace_->ReleaseFontTable(tableContext_);
    }
}


#pragma warning(disable: 4200) // Shut up stupid warning. It's a very useful extension. "nonstandard extension used : zero - sized array in struct / union"


#pragma pack( push, 1 )
#pragma warning(push)
#pragma warning(disable: 4200)

struct PngIHDRHeader
{
    BigEndianULong width;
    BigEndianULong height;
    uint8_t bitDepth;
    uint8_t colorType;
    uint8_t compressionMethod;
    uint8_t filterMethod;
    uint8_t interlaceMethod;
};

struct PngHeader
{
    static constexpr uint8_t expectedFileSignature[] = { 0x89, 'P','N','G', 13,10,26,10 };

    uint8_t signature[1 + 3 + 4]; // 89h,'PNG',13,10,26,10
    BigEndianULong dataLength;
    BigEndianULong chunkType;

    // The first chunk inside a PNG must be IHDR.
    PngIHDRHeader ihdr;
};

enum class JfifMarkerType : uint8_t
{
    StartOfFrameHuffmanBaselineDct = 192,
    StartOfFrameHuffmanExtendedSequentialDct = 193,
    StartOfFrameHuffmanProgressiveDct = 194,
    StartOfFrameHuffmanLossless = 195,
    DefineHuffmanTable = 196,
    StartOfFrameHuffmanDifferentialSequentialDct = 197,
    StartOfFrameHuffmanDifferentialProgressiveDct = 198,
    StartOfFrameHuffmanDifferentialLossless = 199,
    JpegExtension = 200,
    StartOfFrameArithmeticCodingSequentialDct = 201,
    StartOfFrameArithmeticCodingProgressiveDct = 202,
    StartOfFrameArithmeticCodingLossless = 203,
    DefineArithmeticCoding = 204,
    StartOfFrameArithmeticCodingDifferentialSequentialDct = 205,
    StartOfFrameArithmeticCodingDifferentialProgressiveDct = 206,
    StartOfFrameArithmeticCodingDifferentialLossless = 207,
    Restart0 = 208,
    Restart1 = 209,
    Restart2 = 210,
    Restart3 = 211,
    Restart4 = 212,
    Restart5 = 213,
    Restart6 = 214,
    Restart7 = 215,
    StartOfImage = 216,
    EndOfImage = 217,
    StartOfScan = 218,
    DefineQuantizationTable = 219,
    DefineNumberOfLines = 220,
    DefineRestartInterval = 221,
    DefineHeirarchicalProgression = 222,
    ExpandReferenceComponents = 223,
    Application0 = 224,
    Application1 = 225,
    Application2 = 226,
    Application3 = 227,
    Application4 = 228,
    Application5 = 229,
    Application6 = 230,
    Application7 = 231,
    Application8 = 232,
    Application9 = 233,
    Application10 = 234,
    Application11 = 235,
    Application12 = 236,
    Application13 = 237,
    Application14 = 238,
    Application15 = 239,
    Comment = 254,
};

struct JfifMarker
{
    static constexpr uint8_t expectedFileSignature[] = { 0xFF, 0xD8 }; // StartOfImage

    uint8_t marker[2];  // first byte is FF, second byte is JfifMarkerType.
};

struct JfifSegment
{
    // https://en.wikipedia.org/wiki/JPEG_File_Interchange_Format
    BigEndianUShort length;     // Length of APP0 Field
};

struct JfifSegmentImage : JfifSegment
{
    uint8_t precision;
    BigEndianUShort height;                 // Y
    BigEndianUShort width;                  // X
    uint8_t frameImageComponentCount;       // Nf
    uint8_t componentIdentifier;            // Ci
    uint8_t horizontalScalingFactor : 4;    // Hi
    uint8_t verticalScalingFactor : 4;      // Vi
    uint8_t quantizationTableSelector;      // Tqi
};

bool FindJfifStartOfFrameOffset(
    FontCheckedPtr tablePtr,
    _Inout_ uint32_t& returnedSegmentOffset
    )
{
    uint32_t segmentOffset = returnedSegmentOffset;
    size_t const tableSize = tablePtr.size();
    uint8_t const* tableData = tablePtr.data();

    for (;;)
    {
        // Skip any non-FF's; then skip any FF's which are used as padding.
        for (; segmentOffset < tableSize && tableData[segmentOffset] != 0xFF; ++segmentOffset) { }
        for (; segmentOffset < tableSize && tableData[segmentOffset] == 0xFF; ++segmentOffset) { }
        if (segmentOffset >= tableSize)
            return false;

        JfifMarkerType markerType = static_cast<JfifMarkerType>(tableData[segmentOffset++]);
        switch (markerType)
        {
        case JfifMarkerType::StartOfFrameHuffmanBaselineDct:
        case JfifMarkerType::StartOfFrameHuffmanExtendedSequentialDct:
        case JfifMarkerType::StartOfFrameHuffmanProgressiveDct:
        case JfifMarkerType::StartOfFrameHuffmanLossless:
        case JfifMarkerType::StartOfFrameHuffmanDifferentialSequentialDct:
        case JfifMarkerType::StartOfFrameHuffmanDifferentialProgressiveDct:
        case JfifMarkerType::StartOfFrameHuffmanDifferentialLossless:
        case JfifMarkerType::StartOfFrameArithmeticCodingSequentialDct:
        case JfifMarkerType::StartOfFrameArithmeticCodingProgressiveDct:
        case JfifMarkerType::StartOfFrameArithmeticCodingLossless:
        case JfifMarkerType::StartOfFrameArithmeticCodingDifferentialSequentialDct:
        case JfifMarkerType::StartOfFrameArithmeticCodingDifferentialProgressiveDct:
        case JfifMarkerType::StartOfFrameArithmeticCodingDifferentialLossless:
            returnedSegmentOffset = segmentOffset;
            return true;

        case JfifMarkerType::StartOfScan:
        case JfifMarkerType::StartOfImage:
        case JfifMarkerType::EndOfImage:
            return false;
        }

        // Read the segment length and skip the ignorable segment.
        auto& segment = tablePtr.ReadAt<JfifSegment>(segmentOffset);
        uint32_t oldSegmentOffset = segmentOffset;
        uint32_t segmentLength = segment.length;
        segmentOffset += segmentLength;
        if (segmentOffset < oldSegmentOffset)
            return false; // Overflow!
    }

    return false;
}

// http://www.fileformat.info/format/tiff/corion.htm
struct TiffHeader
{
    static constexpr uint8_t expectedFileSignatureLogicalEndian[] = { 'I','I',42,0 };
    static constexpr uint8_t expectedFileSignatureBackwardsEndian[] = { 'M','M',0,42 };
    static constexpr uint32_t expectedFileSignatureSize = 4;

    uint8_t byteOrder[2];             // 'II' or 'MM'
    BiEndianUInt16 version;           // =42
    BiEndianUInt32 imageFileDirectoryOffset; // Offset to the directory of file tags
};

struct TiffImageFileDirectory
{
    BiEndianUInt16 numberOfEntries;

    struct Record
    {
        BiEndianUInt16 tagType; // e.g. 100h = width, 101h = height
        BiEndianUInt16 fieldType; // 1 = byte, 3 = word, 4 = dword
        BiEndianUInt32 fieldCount; // in element counts depending on the data type (not always bytes)
        BiEndianUInt32 dataOffset; // offset in file to beginning of data, or direct value if <=4 bytes
    };

    enum TagType
    {
        TagTypeWidth = 256,
        TagTypeHeight = 257,
    };

    enum FieldType
    {
        FieldTypeNone = 0,
        FieldTypeUint8 = 1,
        FieldTypeAscii = 2,
        FieldTypeUint16 = 3,
        FieldTypeUint32 = 4,
        FieldTypeRationalUint32 = 5,
        FieldTypeInt8 = 6,
        FieldTypeUndefined = 7,
        FieldTypeInt16 = 8,
        FieldTypeInt32= 9,
        FieldTypeRationalInt32 = 10,
        FieldTypeFloat32 = 11,
        FieldTypeFloat64 = 12,
    };

    Record records[];

    // uint32_t nextDirectory; // offset to next file directory, or 0 if none.
};

#pragma warning(pop)
#pragma pack( pop )


uint32_t GetTiffValue(BiEndianUInt32 value, bool isBe)
{
    return isBe ? uint32_t(value.be) : uint32_t(value.le);
}

uint32_t GetTiffValue(BiEndianUInt16 value, bool isBe)
{
    return isBe ? uint32_t(value.be) : uint32_t(value.le);
}


union TiffDataValue
{
    uint8_t valueUInt8;
    int8_t valueInt8;
    BiEndianUInt16 valueUInt16;
    BiEndianInt16 valueInt16;
    BiEndianUInt32 valueUInt32;
    BiEndianInt32 valueInt32;
};

// Read a single scalar value, such as a byte, word, or dword.
static bool GetTiffImageDirectoryScalarValue(
    array_ref<TiffImageFileDirectory::Record const> records,
    uint16_t tagType,
    bool isBe,
    _Out_ uint32_t& returnValue
    )
{
    returnValue = 0;
    uint32_t value = 0;

    for (auto& record : records)
    {
        if (GetTiffValue(record.tagType, isBe) == tagType)
        {
            if (GetTiffValue(record.fieldCount, isBe) != 1) // This function returns exactly one value.
                return false; 

            // Read types that are 8-bit, 16-bit, or 32-bit. Ignore any others.
            auto& dataValue = reinterpret_cast<TiffDataValue const&>(record.dataOffset);
            switch (GetTiffValue(record.fieldType, isBe))
            {
            case TiffImageFileDirectory::FieldTypeUint8:  value = dataValue.valueUInt8; break;
            case TiffImageFileDirectory::FieldTypeInt8:   value = dataValue.valueInt8; break;
            case TiffImageFileDirectory::FieldTypeUint16: value = isBe ? uint32_t(dataValue.valueUInt16.be) : uint32_t(dataValue.valueUInt16.le); break;
            case TiffImageFileDirectory::FieldTypeInt16:  value = isBe ? uint32_t(dataValue.valueInt16.be ) : uint32_t(dataValue.valueInt16.le ); break;
            case TiffImageFileDirectory::FieldTypeUint32: value = isBe ? uint32_t(dataValue.valueUInt32.be) : uint32_t(dataValue.valueUInt32.le); break;
            case TiffImageFileDirectory::FieldTypeInt32:  value = isBe ? uint32_t(dataValue.valueInt32.be ) : uint32_t(dataValue.valueInt32.le ); break;
            default: return false;
            }

            returnValue = value;
            return true;
        }
    }

    return false; // No matching tag found.
}


// Returns width and height by reading image header.
std::pair<uint32_t, uint32_t> GetImageDimension(
    DWRITE_GLYPH_IMAGE_FORMATS imageFormat,
    FontCheckedPtr tablePtr,
    uint32_t tableOffset
    )
{
    uint32_t width = 0, height = 0;

    switch (imageFormat)
    {
    case DWRITE_GLYPH_IMAGE_FORMATS_PNG:
        {
            PngHeader const& header = tablePtr.ReadAt<PngHeader>(tableOffset);
            if (memcmp(header.signature, PngHeader::expectedFileSignature, sizeof(header.signature)) == 0)
            {
                width = header.ihdr.width;
                height = header.ihdr.height;
            }
        }
        break;

    case DWRITE_GLYPH_IMAGE_FORMATS_JPEG:
        {
            JfifMarker const& header = tablePtr.ReadAt<JfifMarker>(tableOffset);
            if (memcmp(header.marker, JfifMarker::expectedFileSignature, sizeof(header.marker)) == 0)
            {
                uint32_t segmentOffset = sizeof(JfifMarker); // Skip start of image.
                if (FindJfifStartOfFrameOffset(tablePtr, IN OUT segmentOffset))
                {
                    auto& startOfFrame = tablePtr.ReadAt<JfifSegmentImage>(segmentOffset);
                    width = startOfFrame.width;
                    height = startOfFrame.height;
                }
            }
        }
        break;

    case DWRITE_GLYPH_IMAGE_FORMATS_TIFF:
        {
            TiffHeader const& header = tablePtr.ReadAt<TiffHeader>(tableOffset);
            if (memcmp(&header, TiffHeader::expectedFileSignatureLogicalEndian, TiffHeader::expectedFileSignatureSize) == 0
            ||  memcmp(&header, TiffHeader::expectedFileSignatureBackwardsEndian, TiffHeader::expectedFileSignatureSize) == 0)
            {
                bool isBe = (header.byteOrder[0] == 'M');
                uint32_t imageFileDirectoryOffset = GetTiffValue(header.imageFileDirectoryOffset, isBe);
                auto& tiffImageFileDirectory = tablePtr.ReadAt<TiffImageFileDirectory>(imageFileDirectoryOffset);
                auto records = tablePtr.ReadArrayRefAt(tiffImageFileDirectory.records, GetTiffValue(tiffImageFileDirectory.numberOfEntries, isBe));
                GetTiffImageDirectoryScalarValue(records, TiffImageFileDirectory::TagTypeWidth, isBe, OUT width);
                GetTiffImageDirectoryScalarValue(records, TiffImageFileDirectory::TagTypeHeight, isBe, OUT height);
            }
        }
        break;
    }
    return {width,height};
}


class MyIDWriteFontFace4
{
public:
    MyIDWriteFontFace4(
        IDWriteFontFace* fontFace,
        IDWriteFactory* dwriteFactory
        )
    {
        dwriteFactory_.Set(dwriteFactory);
        fontFace_.Set(fontFace);
    }

    /// <summary>
    /// Gets all the glyph data formats supported by the font.
    /// </summary>
    /// <remarks>
    /// If glyphs contain any COLR, SVG, PNG, JPEG, or TIFF data, then IsColorFont font will also
    /// be true.
    /// </remarks>
    STDMETHOD(GetGlyphDataFormats)(
        _Out_ DWRITE_GLYPH_IMAGE_FORMATS* glyphDataFormats
        )
    {
        *glyphDataFormats = DWRITE_GLYPH_IMAGE_FORMATS_NONE;
        if (sbixTablePtr_.IsNull())
        {
            FontTablePtr sbixTablePtr(fontFace_, DWRITE_MAKE_OPENTYPE_TAG('s','b','i','x'));
            sbixTablePtr_.swap(sbixTablePtr);
        }
        if (!sbixTablePtr_.IsNull())
        {
            auto& sbixTable = sbixTablePtr_.ReadAt<SbixTable>(0);
            uint32_t version = sbixTable.version;
            uint32_t strikeCount = sbixTable.strikeCount;
            uint32_t flags = sbixTable.flags;
            //*glyphDataFormats |= DWRITE_GLYPH_IMAGE_FORMATS_PNG;
        }
        // TryGetFontTable SVG PNG CBDT.
        return E_NOTIMPL;
    }

    /// <summary>
    /// Gets the data format of a specific glyph and size. Glyphs often have at least TrueType
    /// or CFF outlines, but they may also have SVG outlines, or they may have only bitmaps
    /// without no TrueType/CFF outlines. Some data formats, notably the PNG/JPEG ones, are size
    /// specific and will return no match when there isn't an entry in that size range.
    /// </summary>
    STDMETHOD(GetGlyphDataFormats)(
        UINT16 glyphId,
        UINT32 fontEmSizeBegin,
        UINT32 fontEmSizeEnd,
        _Out_ DWRITE_GLYPH_IMAGE_FORMATS* glyphDataFormats
        )
    {
        // TryGetFontTable SVG PNG CBDT.
        return E_NOTIMPL;
    }

    /// <summary>
    /// Gets a pointer to the glyph data based on the desired data format. If the glyph does not
    /// have that format, the function does not return an error, but it returns empty data.
    /// If more than one desired format is available, the function returns the best match
    /// considering type and size (actualGlyphDataFormat will contain exactly one flag set,
    /// or none). When color bitmaps are requested and when data is available at that size,
    /// color bitmaps have the highest priority, followed by SVG, and finally glyph outlines.
    /// Note the function only returns SVG and raster data, and requesting only TrueType or CFF
    /// outlines yields empty data.
    /// </summary>
    /// <remarks>
    /// This function calls IDWriteFontFileStream::ReadFileFragment to get the table data.
    /// The origin is typically zero but will be non-zero for bitmaps, pointing from the top
    /// left corner of the image to the coordinate which should align with the baseline.
    /// </remarks>
    /// <remarks>
    /// The glyphDataUniqueId is valuable for caching purposes, so that if the same resource
    /// is returned more than once, you can quickly compare it to hash map. Checking the file data
    /// pointer alone is nearly enough but not sufficient, because while it works for our
    /// IDWriteLocalFileLoader, it is not necessarily unique with other loaders which could return a 
    /// different pointer on subsequent calls.
    /// </remarks>
    STDMETHOD(GetGlyphData)(
        _In_ uint16_t glyphId,
        UINT32 desiredPixelsPerEm,
        DWRITE_GLYPH_IMAGE_FORMATS desiredGlyphDataFormats,
        _Out_ DWRITE_GLYPH_IMAGE_FORMATS* actualGlyphDataFormat,
        _Outptr_result_bytebuffer_(*glyphDataSize) void const** glyphData,
        _Out_ uint32_t* glyphDataSize,
        _Out_ void** glyphDataContext,
        _Out_ uint32_t* glyphDataUniqueId,
        _Out_ uint32_t* actualPixelsPerEm,
        _Out_ D2D1_POINT_2L* horizontalOrigin,
        _Out_ D2D1_POINT_2L* verticalOrigin
        )
    {
        #if 0
        {
            std::pair<uint32_t, uint32_t> imageSize;
            FontCheckedPtr imageDataPtr;
            std::vector<uint8_t> fileBytes;

            ReadBinaryFile(u"c:\\temp\\foo.png", OUT fileBytes);
            imageDataPtr = FontCheckedPtr(fileBytes);
            imageSize = GetImageDimension(DWRITE_GLYPH_IMAGE_FORMATS_PNG, imageDataPtr, 0);

            ReadBinaryFile(u"c:\\temp\\foo.tif", OUT fileBytes);
            imageDataPtr = FontCheckedPtr(fileBytes);
            imageSize = GetImageDimension(DWRITE_GLYPH_IMAGE_FORMATS_TIFF, imageDataPtr, 0);

            ReadBinaryFile(u"c:\\temp\\foo_progressive.jpg", OUT fileBytes);
            imageDataPtr = FontCheckedPtr(fileBytes);
            imageSize = GetImageDimension(DWRITE_GLYPH_IMAGE_FORMATS_JPEG, imageDataPtr, 0);

            ReadBinaryFile(u"c:\\temp\\foo.jpg", OUT fileBytes);
            imageDataPtr = FontCheckedPtr(fileBytes);
            imageSize = GetImageDimension(DWRITE_GLYPH_IMAGE_FORMATS_JPEG, imageDataPtr, 0);

            ReadBinaryFile(u"C:\\TEMP\\tiff-sample-images-be\\tiger-minisblack-tile-04.tif", OUT fileBytes);
            imageDataPtr = FontCheckedPtr(fileBytes);
            imageSize = GetImageDimension(DWRITE_GLYPH_IMAGE_FORMATS_TIFF, imageDataPtr, 0);
        }
        #endif


        *actualGlyphDataFormat = DWRITE_GLYPH_IMAGE_FORMATS_NONE;
        *glyphData = nullptr;
        *glyphDataSize = 0;
        *glyphDataContext = nullptr;
        *glyphDataUniqueId = 0;
        *actualPixelsPerEm = 0;
        *horizontalOrigin = {};
        *verticalOrigin = {};

        uint32_t const glyphCount = fontFace_->GetGlyphCount();
        if (glyphCount == 0)
            return S_OK; // Nop

        if (glyphId >= glyphCount)
            glyphId = 0;

        if (desiredGlyphDataFormats & DWRITE_GLYPH_IMAGE_FORMATS_PNG)
        {
            if (sbixTablePtr_.IsNull() && cbdtTablePtr_.IsNull() && svgTablePtr_.IsNull())
            {
                if (sbixTablePtr_.IsNull())
                {
                    FontTablePtr sbixTablePtr(fontFace_, DWRITE_MAKE_OPENTYPE_TAG('s', 'b', 'i', 'x'));
                    sbixTablePtr_.swap(sbixTablePtr);
                }
                if (cbdtTablePtr_.IsNull())
                {
                    FontTablePtr cbdtTablePtr(fontFace_, DWRITE_MAKE_OPENTYPE_TAG('C', 'B', 'D', 'T'));
                    cbdtTablePtr_.swap(cbdtTablePtr);
                }
                if (cbdtTablePtr_.IsNull())
                {
                    FontTablePtr cbdtTablePtr(fontFace_, DWRITE_MAKE_OPENTYPE_TAG('E', 'B', 'D', 'T'));
                    cbdtTablePtr_.swap(cbdtTablePtr);
                }
                if (cblcTablePtr_.IsNull())
                {
                    FontTablePtr cblcTablePtr(fontFace_, DWRITE_MAKE_OPENTYPE_TAG('C', 'B', 'L', 'C'));
                    cblcTablePtr_.swap(cblcTablePtr);
                }
                if (cblcTablePtr_.IsNull())
                {
                    FontTablePtr cblcTablePtr(fontFace_, DWRITE_MAKE_OPENTYPE_TAG('E', 'B', 'L', 'C'));
                    cblcTablePtr_.swap(cblcTablePtr);
                }
                if (svgTablePtr_.IsNull())
                {
                    FontTablePtr svgTablePtr(fontFace_, DWRITE_MAKE_OPENTYPE_TAG('S', 'V', 'G', ' '));
                    svgTablePtr_.swap(svgTablePtr);
                }
            }

            if (!sbixTablePtr_.IsNull())
            {
                auto& sbixTable = sbixTablePtr_.ReadAt<SbixTable>(0);
                uint32_t version = sbixTable.version;
                uint32_t flags = sbixTable.flags;
                uint32_t strikeCount = sbixTable.strikeCount;

                for (uint32_t i = 0; i < strikeCount; ++i)
                {
                    auto* strikeOffsets = sbixTablePtr_.ReadArrayAt(sbixTable.strikeOffsets, strikeCount);
                    auto& strikeRecord  = sbixTablePtr_.ReadAt<SbixTable::StrikeRecord>(strikeOffsets[i]);

                    if (strikeRecord.ppem >= desiredPixelsPerEm || i == strikeCount - 1)
                    {
                        auto glyphDataOffsets = sbixTablePtr_.ReadArrayAt(strikeRecord.glyphDataOffsets, glyphCount + 1);
                        uint32_t glyphOffset = glyphDataOffsets[glyphId];
                        uint32_t nextGlyphOffset = glyphDataOffsets[glyphId + 1];
                        if (nextGlyphOffset <= glyphOffset)
                        {
                            continue;
                        }
                        size_t recordDataSize = nextGlyphOffset - glyphOffset;
                        if (glyphOffset == 0 || recordDataSize < sizeof(SbixTable::StrikeRecordData))
                        {
                            continue; // Skip empty slots.
                        }

                        std::pair<uint32_t, uint32_t> imageSize;
                        auto const& strikeRecordData = sbixTablePtr_.ReadRelativeTo<SbixTable::StrikeRecordData>(strikeRecord, glyphOffset);
                        FontCheckedPtr imageDataPtr(sbixTablePtr_);
                        imageDataPtr += strikeRecordData.data - imageDataPtr.data();

                        switch (strikeRecordData.graphicType)
                        {
                        case DWRITE_MAKE_OPENTYPE_TAG('p','n','g',' '):
                            imageSize = GetImageDimension(DWRITE_GLYPH_IMAGE_FORMATS_PNG, imageDataPtr, 0);
                            *actualGlyphDataFormat = DWRITE_GLYPH_IMAGE_FORMATS_PNG;
                            break;

                        case DWRITE_MAKE_OPENTYPE_TAG('j','p','e','g'):
                            imageSize = GetImageDimension(DWRITE_GLYPH_IMAGE_FORMATS_JPEG, imageDataPtr, 0);
                            *actualGlyphDataFormat = DWRITE_GLYPH_IMAGE_FORMATS_JPEG;
                            break;

                        case DWRITE_MAKE_OPENTYPE_TAG('t','i','f','f'):
                            // http://www.fileformat.info/format/tiff/corion.htm
                            // char[2] == II or MM
                            // version uint16_t = 42
                            // uint32_t offset to IFD
                            //
                            // IFD:
                            // uint16_t number of entries
                            //  record:
                            //      uint16_t tag type 100h=width, 101h=height
                            //      uint16_t field type, 3=word, 4=dword
                            //      uint32_t field length
                            //      uint32_t data offset
                            //  for width/height, n=1, word or dword
                            imageSize = GetImageDimension(DWRITE_GLYPH_IMAGE_FORMATS_TIFF, imageDataPtr, 0);
                            *actualGlyphDataFormat = DWRITE_GLYPH_IMAGE_FORMATS_TIFF;
                            break;

                        case DWRITE_MAKE_OPENTYPE_TAG('d','u','p','e'):
                            // Get glyph data once more.
                            break;

                        default:
                            continue;
                        }

                        int32_t height = imageSize.second;
                        D2D1_POINT_2L adjustedHorizontalOrigin = D2D1_POINT_2L{ strikeRecordData.originOffsetX, int32_t(height) - strikeRecordData.originOffsetY };

                        uint32_t imageDataSize = nextGlyphOffset - glyphOffset - sizeof(SbixTable::StrikeRecordData);
                        *glyphData = sbixTablePtr_.ReadArrayAt<uint8_t>(strikeRecordData.data, imageDataSize);
                        *glyphDataSize = imageDataSize;
                        *glyphDataContext = sbixTablePtr_.tableContext_;
                        *glyphDataUniqueId = glyphOffset; // add table offset!
                        *actualPixelsPerEm = strikeRecord.ppem;
                        *horizontalOrigin = adjustedHorizontalOrigin;
                        *verticalOrigin = D2D1_POINT_2L{ 0,0 }; // zero for now
                        *actualGlyphDataFormat = DWRITE_GLYPH_IMAGE_FORMATS_PNG;

                        return S_OK;
                    }
                }
            }

            if (!cbdtTablePtr_.IsNull() && !cblcTablePtr_.IsNull())
            {
                EblcHeader const& eblcHeader = cblcTablePtr_.ReadAt<EblcHeader>(0);

                size_t const bitmapSizeCount = eblcHeader.count;
                EblcEntry const* bitmapSizes = cblcTablePtr_.ReadArrayAt<EblcEntry>(sizeof(EblcHeader), bitmapSizeCount);
                size_t bitmapSizeIndex;
                uint32_t subtableGlyphCount = 0;
                size_t subtableOffset = 0;
                uint8_t ppem = 0;
                uint32_t bitDepth = 0;
                uint32_t firstGlyphIndex = 0;
                uint32_t lastGlyphIndex = 0;

                // Enumerate each ppem size. Note that they are typically in ascending
                // order, but can actually come in any order.
                for (bitmapSizeIndex = 0; bitmapSizeIndex < bitmapSizeCount; ++bitmapSizeIndex)
                {
                    EblcEntry const& bitmapSize        = bitmapSizes[bitmapSizeIndex];
                    size_t const subtableCount         = bitmapSize.numberOfIndexSubTables;
                    size_t const subtableEntriesOffset = bitmapSize.indexSubTableArrayOffset;
                    EblcIndexSubtableEntry const* subtableEntries = cblcTablePtr_.ReadArrayAt<EblcIndexSubtableEntry>(subtableEntriesOffset, subtableCount);

                    // Get the pixels-per-em (0..255) of this size record, and store it.
                    // Note that a table can have multiple entries at a given ppem,
                    // with differing bit depths and/or differing horizontal/vertical
                    // forms.

                    ppem = bitmapSize.ppemX;
                    bitDepth = bitmapSize.bitDepth;
                    if (bitmapSize.ppemY != ppem)
                    {
                        // We only care about embedded bitmaps that have a 1:1 aspect ratio
                        // (which all of the system fonts do).
                        continue;
                    }
                    //if (bitmapSize.bitDepth != 32)
                    //{
                    //    // Skip anything except BGRA.
                    //    continue;
                    //}

                    firstGlyphIndex = bitmapSize.firstGlyphIndex;
                    lastGlyphIndex = bitmapSize.lastGlyphIndex;
                    if (glyphId < firstGlyphIndex || glyphId > lastGlyphIndex)
                    {
                        continue;
                    }

                    // Enumerate each subrange, as the glyph bitmaps have sparse ranges.
                    size_t subtableIndex;
                    for (subtableIndex = 0; subtableIndex < subtableCount; ++subtableIndex)
                    {
                        EblcIndexSubtableEntry const& subtableEntry = subtableEntries[subtableIndex];
                        firstGlyphIndex = subtableEntry.firstGlyphIndex;
                        lastGlyphIndex = subtableEntry.lastGlyphIndex;
                        if (glyphId < firstGlyphIndex || glyphId > lastGlyphIndex)
                        {
                            continue;
                        }

                        subtableGlyphCount = lastGlyphIndex - firstGlyphIndex + 1;
                        subtableOffset = subtableEntriesOffset + subtableEntry.additionalOffsetToIndexSubtable;
                        break;
                    }

                    if (subtableIndex >= subtableCount)
                        return S_OK;

                    if (ppem < desiredPixelsPerEm && bitmapSizeIndex != bitmapSizeCount - 1)
                    {
                        // Skip small ones.
                        continue;
                    }

                    break;
                }

                if (bitmapSizeIndex >= bitmapSizeCount)
                    return S_OK; // No match.

                if (subtableGlyphCount == 0)
                    return S_OK; // No match.

                EblcIndexSubtable const& subtable = cblcTablePtr_.ReadAt<EblcIndexSubtable>(subtableOffset); 
                uint32_t const imageFormat = subtable.imageFormat;     // format of 'EBDT' image data 
                uint32_t const baseImageDataOffset = subtable.imageDataOffset; // offset to first glyph's image data in 'EBDT' table, exact type depending on imageFormat.
                uint32_t imageDataOffset = 0;
                uint32_t nextImageDataOffset = 0;
                EbxxBigGlyphMetrics const* bigGlyphMetrics = nullptr;
                EbxxSmallGlyphMetrics const* smallGlyphMetrics = nullptr;

                switch (subtable.indexFormat)
                {
                case 1: // Variable metrics glyphs with 4 byte offsets.
                    {
                        uint32_t arrayEntry = glyphId - firstGlyphIndex;
                        auto& subtable = cblcTablePtr_.ReadAt<EblcIndexSubtable1>(subtableOffset);
                        auto offsets = cblcTablePtr_.ReadArrayAt(subtable.offsetArray, subtableGlyphCount + 1);
                        imageDataOffset = offsets[arrayEntry];
                        nextImageDataOffset = offsets[arrayEntry+1];
                    }
                    break;

                case 2: // All glyphs have identical metrics.
                    {
                        auto& subtable = cblcTablePtr_.ReadAt<EblcIndexSubtable2>(subtableOffset);
                        uint32_t imageSize = subtable.imageSize;
                        imageDataOffset = glyphId * imageSize; // todo: overflow check
                        nextImageDataOffset = imageDataOffset + imageSize;
                        bigGlyphMetrics = &subtable.bigMetrics;
                    }
                    break;

                case 3: // Variable metrics glyphs with 2 byte offsets.
                    {
                        uint32_t arrayEntry = glyphId - firstGlyphIndex;
                        auto& subtable = cblcTablePtr_.ReadAt<EblcIndexSubtable3>(subtableOffset);
                        auto offsets = cblcTablePtr_.ReadArrayAt(subtable.offsetArray, subtableGlyphCount + 1);
                        imageDataOffset = offsets[arrayEntry];
                        nextImageDataOffset = offsets[arrayEntry+1];
                    }
                    break;

                case 4: // Variable metrics glyphs with sparse glyph codes.
                    {
                        // The actual number of glyphs may be less than (last - first + 1).
                        auto& subtable = cblcTablePtr_.ReadAt<EblcIndexSubtable4>(subtableOffset);
                        uint32_t const glyphCount = subtable.glyphCount;
                        auto glyphArray = cblcTablePtr_.ReadArrayAt(subtable.glyphArray, glyphCount + 1);

                        for (uint32_t arrayEntry = 0; arrayEntry < glyphCount; ++arrayEntry)
                        {
                            if (glyphArray[arrayEntry].glyphIndex == glyphId)
                            {
                                imageDataOffset = glyphArray[arrayEntry].offset;
                                nextImageDataOffset = glyphArray[arrayEntry + 1].offset;
                                //CallFunction(imageFormat, ppem, imageDataOffset, nextImageDataOffset, bigGlyphMetrics, callback);
                            }
                        }
                    }
                    break;

                case 5: // Constant metrics glyphs with sparse glyph codes.
                    {
                        auto& subtable = cblcTablePtr_.ReadAt<EblcIndexSubtable5>(subtableOffset);
                        bigGlyphMetrics = &subtable.bigMetrics;
                        uint32_t const glyphCount = subtable.glyphCount;
                        uint32_t const imageDataSize = subtable.imageSize;
                        auto glyphArray = cblcTablePtr_.ReadArrayAt(subtable.glyphArray, glyphCount);

                        for (uint32_t arrayEntry = 0; arrayEntry < glyphCount; ++arrayEntry)
                        {
                            if (glyphArray[arrayEntry] == glyphId)
                            {
                                imageDataOffset = arrayEntry * imageDataSize;
                                nextImageDataOffset = imageDataOffset + imageDataSize;
                                break;
                            }
                        }

                    }
                    break;

                default:
                    //INPUT_ASSERT_FAIL_MSG("Unknown type of subtable format.");
                    break;
                }

                if (nextImageDataOffset <= imageDataOffset)
                {
                    if (nextImageDataOffset < imageDataOffset)
                    {
                        //-INPUT_ASSERT_FAIL_MSG("Invalid glyph data offset.");
                    }
                    return S_OK; // Note the empty data case is valid. Just skip it.
                }

                imageDataOffset += baseImageDataOffset;
                nextImageDataOffset += baseImageDataOffset;

#if 1
                struct GlyphBitmapFormatInfo
                {
                    uint32_t byteSize;
                    uint32_t smallMetricsFieldOffset;
                    uint32_t bigMetricsFieldOffset;
                    uint32_t imageDataFieldOffset;
                    uint32_t imageDataSizeFieldOffset;
                    DWRITE_GLYPH_IMAGE_FORMATS dataFormat;
                };

                // Since there are so many different formats to handle which are mostly the same yet
                // just different enough to greatly complicate parsing, generalize their small
                // differences via a table of structure offsets.
                uint32_t constexpr Undefined = ~0u;
                static constexpr GlyphBitmapFormatInfo glyphBitmapFormatsInfo[20] = {
                    {Undefined},
                    {sizeof(EbdtGlyphBitmapFormat1), offsetof(EbdtGlyphBitmapFormat1, smallMetrics), Undefined, offsetof(EbdtGlyphBitmapFormat1, imageData), Undefined, DWRITE_GLYPH_IMAGE_FORMATS_PREMULTIPLIED_B8G8R8A8},
                    {sizeof(EbdtGlyphBitmapFormat2), offsetof(EbdtGlyphBitmapFormat2, smallMetrics), Undefined, offsetof(EbdtGlyphBitmapFormat2, imageData), Undefined, DWRITE_GLYPH_IMAGE_FORMATS_PREMULTIPLIED_B8G8R8A8},
                    {Undefined}, // Deprecated
                    {Undefined}, // Only support on Mac
                    {sizeof(EbdtGlyphBitmapFormat5), Undefined, Undefined, offsetof(EbdtGlyphBitmapFormat5, imageData), Undefined, DWRITE_GLYPH_IMAGE_FORMATS_PREMULTIPLIED_B8G8R8A8},
                    {sizeof(EbdtGlyphBitmapFormat6), Undefined, offsetof(EbdtGlyphBitmapFormat6, bigMetrics),   offsetof(EbdtGlyphBitmapFormat6, imageData), Undefined, DWRITE_GLYPH_IMAGE_FORMATS_PREMULTIPLIED_B8G8R8A8},
                    {sizeof(EbdtGlyphBitmapFormat7), Undefined, offsetof(EbdtGlyphBitmapFormat7, bigMetrics),   offsetof(EbdtGlyphBitmapFormat7, imageData), Undefined, DWRITE_GLYPH_IMAGE_FORMATS_PREMULTIPLIED_B8G8R8A8},
                    {sizeof(EbdtGlyphBitmapFormat8), offsetof(EbdtGlyphBitmapFormat8, smallMetrics), Undefined, Undefined, Undefined, DWRITE_GLYPH_IMAGE_FORMATS_PREMULTIPLIED_B8G8R8A8 },
                    {sizeof(EbdtGlyphBitmapFormat9), Undefined, offsetof(EbdtGlyphBitmapFormat7, bigMetrics), Undefined, Undefined, DWRITE_GLYPH_IMAGE_FORMATS_PREMULTIPLIED_B8G8R8A8 },
                    {Undefined}, // Nonexistent formats
                    {Undefined},
                    {Undefined},
                    {Undefined},
                    {Undefined},
                    {Undefined},
                    {Undefined},
                    {sizeof(EbdtGlyphBitmapFormat17), offsetof(EbdtGlyphBitmapFormat17, smallMetrics), Undefined, offsetof(EbdtGlyphBitmapFormat17, imageData), offsetof(EbdtGlyphBitmapFormat17, imageDataSize), DWRITE_GLYPH_IMAGE_FORMATS_PNG},
                    {sizeof(EbdtGlyphBitmapFormat18), offsetof(EbdtGlyphBitmapFormat18, bigMetrics), Undefined, offsetof(EbdtGlyphBitmapFormat18, imageData), offsetof(EbdtGlyphBitmapFormat18, imageDataSize), DWRITE_GLYPH_IMAGE_FORMATS_PNG},
                    {sizeof(EbdtGlyphBitmapFormat19), Undefined, Undefined, offsetof(EbdtGlyphBitmapFormat19, imageData), offsetof(EbdtGlyphBitmapFormat19, imageDataSize), DWRITE_GLYPH_IMAGE_FORMATS_PNG},
                };

                if (imageFormat >= ARRAYSIZE(glyphBitmapFormatsInfo))
                {
                    return S_OK; // Unsupported format.
                }

                auto& formatInfo = glyphBitmapFormatsInfo[imageFormat];
                if (formatInfo.byteSize == Undefined || formatInfo.imageDataFieldOffset == Undefined)
                {
                    return S_OK; // Unsupported format.
                }

                // It is safe to cast the types directly without a distinct Read call for each different
                // type because the byte array read will validate that sizeof(bitmapFormat) is readable.
                uint32_t imageDataSize = nextImageDataOffset - imageDataOffset;
                uint8_t const* imageData = cbdtTablePtr_.ReadArrayAt<uint8_t>(imageDataOffset, std::max(imageDataSize, formatInfo.byteSize));

                // If this image format type includes its own metrics fields, read it rather than using the
                // generic metrics from the EBLC.
                if (formatInfo.bigMetricsFieldOffset != Undefined)
                {
                    bigGlyphMetrics = reinterpret_cast<EbxxBigGlyphMetrics const*>(imageData + formatInfo.bigMetricsFieldOffset);
                    smallGlyphMetrics = nullptr;
                }
                else if (formatInfo.smallMetricsFieldOffset != Undefined)
                {
                    smallGlyphMetrics = reinterpret_cast<EbxxSmallGlyphMetrics const*>(imageData + formatInfo.smallMetricsFieldOffset);
                    bigGlyphMetrics = nullptr;
                }

                // Skip ahead to the image data beyond any leading fields.
                if (formatInfo.imageDataFieldOffset != Undefined)
                {
                    imageDataOffset += formatInfo.imageDataFieldOffset;
                    imageDataSize -= formatInfo.imageDataFieldOffset;
                }
                else
                {
                    imageDataSize = 0; // No data. We can still read metrics though.
                }

                // The format has its own image data length, rather than just using the EBLC size.
                if (formatInfo.imageDataSizeFieldOffset != Undefined)
                {
                    imageDataSize = *reinterpret_cast<OpenTypeULong const*>(imageData + formatInfo.imageDataSizeFieldOffset);
                }
#endif

#if 0
                switch (subtable.imageFormat)
                {
                case 17:
                    {
                        auto& image = cbdtTablePtr_.ReadAt<EbdtGlyphBitmapFormat17>(imageDataOffset);
                        smallGlyphMetrics = &image.smallMetrics;
                        imageDataOffset += offsetof(EbdtGlyphBitmapFormat17, imageData);
                        imageDataSize = image.imageDataSize;
                    }
                    break;

                case 18:
                    {
                        auto& image = cbdtTablePtr_.ReadAt<EbdtGlyphBitmapFormat18>(imageDataOffset);
                        bigGlyphMetrics = &image.bigMetrics;
                        imageDataOffset += offsetof(EbdtGlyphBitmapFormat18, imageData);
                        imageDataSize = image.imageDataSize;
                    }
                    break;

                case 19:
                    {
                        auto& image = cbdtTablePtr_.ReadAt<EbdtGlyphBitmapFormat19>(imageDataOffset);
                        imageDataOffset += offsetof(EbdtGlyphBitmapFormat19, imageData);
                        imageDataSize = image.imageDataSize;
                    }
                    break;
                }
#endif

                D2D1_POINT_2L adjustedHorizontalOrigin = D2D1_POINT_2L{ 0,0 };
                D2D1_POINT_2L adjustedVerticalOrigin = D2D1_POINT_2L{ 0,0 };
                if (smallGlyphMetrics != nullptr)
                {
                    adjustedHorizontalOrigin.x = smallGlyphMetrics->bearingX;
                    adjustedHorizontalOrigin.y = smallGlyphMetrics->bearingY;
                }
                if (bigGlyphMetrics != nullptr)
                {
                    adjustedHorizontalOrigin.x = bigGlyphMetrics->horiBearingX;
                    adjustedHorizontalOrigin.y = bigGlyphMetrics->horiBearingY;
                    adjustedVerticalOrigin.x = bigGlyphMetrics->vertBearingX;
                    adjustedVerticalOrigin.y = bigGlyphMetrics->vertBearingY;
                }

                auto dataFormat = formatInfo.dataFormat;
                if (dataFormat == DWRITE_GLYPH_IMAGE_FORMATS_PREMULTIPLIED_B8G8R8A8 && bitDepth != 32)
                {
                    dataFormat = DWRITE_GLYPH_IMAGE_FORMATS_NONE;
                }

                *glyphData = cbdtTablePtr_.ReadArrayAt<uint8_t>(imageDataOffset, imageDataSize);
                *glyphDataSize = imageDataSize;
                *glyphDataContext = cbdtTablePtr_.tableContext_;
                *glyphDataUniqueId = imageDataOffset; // add table offset!
                *actualPixelsPerEm = ppem;
                *horizontalOrigin = adjustedHorizontalOrigin;
                *verticalOrigin = D2D1_POINT_2L{ 0,0 }; // zero for now
                *actualGlyphDataFormat = formatInfo.dataFormat;

                return S_OK;
            }
        }

        if (desiredGlyphDataFormats & DWRITE_GLYPH_IMAGE_FORMATS_SVG)
        {
            if (!svgTablePtr_.IsNull())
            {
                auto const& svgHeader = svgTablePtr_.ReadAt<SvgTable>(0);
                uint32_t indexOffset = svgHeader.indexOffset;
                auto const& svgIndex = svgTablePtr_.ReadAt<SvgTable::Index>(indexOffset);

                size_t const entriesCount = svgIndex.entriesCount;
                auto entries = svgTablePtr_.ReadArrayRefAt(svgIndex.entries, entriesCount);

                // Enumerate each ppem size. Note that they are typically in ascending
                // order, but can actually come in any order.
                for (auto& entry : entries)
                {
                    if (glyphId < entry.firstGlyphId || glyphId > entry.lastGlyphId)
                        continue;

                    uint32_t dataOffset = entry.offset + indexOffset;
                    uint32_t dataByteSize = entry.length;

                    *glyphData = svgTablePtr_.ReadArrayAt<uint8_t>(dataOffset, dataByteSize);
                    *glyphDataSize = dataByteSize;
                    *glyphDataContext = svgTablePtr_.tableContext_;
                    *glyphDataUniqueId = dataOffset; // add table offset!
                    *actualPixelsPerEm = 0;
                    *horizontalOrigin = {};
                    *verticalOrigin = {};
                    *actualGlyphDataFormat = DWRITE_GLYPH_IMAGE_FORMATS_SVG;

                    return S_OK;
                }
            } // if
        }

        return S_OK;
    }

    /// <summary>
    /// Releases the table data obtained earlier from ReadGlyphData.
    /// This must be called after ReadGlyphData successfully returned data.
    /// </summary>
    /// <param name="glyphDataContext">Opaque context from ReadGlyphData.</param>
    STDMETHOD_(void, ReleaseGlyphData)(
        void* glyphDataContext
        )
    {
        return;
    }

    ComPtr<IDWriteFontFace> fontFace_;
    ComPtr<IDWriteFactory> dwriteFactory_;
    FontTablePtr sbixTablePtr_;
    FontTablePtr cbdtTablePtr_;
    FontTablePtr cblcTablePtr_;
    FontTablePtr svgTablePtr_;
};

#endif

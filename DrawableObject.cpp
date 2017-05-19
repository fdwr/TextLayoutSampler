//----------------------------------------------------------------------------
//  Author:         Dwayne Robinson
//  History:        2015-06-19 Created
//  Description:    Individual drawable objects. Note that drawable objects
//                  generally don't hold much state themselves (except caching
//                  for performance), getting attributes from their
//                  AttributeSource as needed.
//----------------------------------------------------------------------------
#include "precomp.h"


const D2D_RECT_F DrawableObject::emptyRect = { 0,0,0,0 };
const DX_MATRIX_3X2F DrawableObject::identityTransform = {1,0,0,1,0,0};
const float DrawableObject::defaultWidth = 300;
const float DrawableObject::defaultHeight = 64;
const float DrawableObject::defaultFontSize = 18.0f;
const float DrawableObject::defaultTabWidth = defaultFontSize * 4;
const uint32_t DrawableObject::defaultFontColor = 0xFF000000;
const uint32_t DrawableObject::defaultLayoutColor = 0xFF7FFFD4;
const uint32_t DrawableObject::defaultBackColor = 0xFFFFFFFF;
const uint32_t DrawableObject::defaultCanvasColor = 0xFF6495ED;


const Attribute DrawableObject::attributeList[DrawableObjectAttributeTotal] =
{
    {Attribute::TypeUInteger32,     Attribute::SemanticEnumExclusive,CategoryLight, DrawableObjectAttributeFunction, u"function", u"Function", u"IDWriteTextLayout", functions },
    {Attribute::TypeBool8,          Attribute::SemanticEnumExclusive,0            , DrawableObjectAttributeVisibility, u"visible", u"Visible", u"visible", visibilities },
    {Attribute::TypeString16,       Attribute::SemanticNone,         0            , DrawableObjectAttributeLabel, u"label", u"Label", u"", {} },
    {Attribute::TypeString16,       Attribute::SemanticLongText,     CategoryLight, DrawableObjectAttributeText, u"text", u"Text", u"This is a text", textDefaults },
    {Attribute::TypeArrayUInteger16,Attribute::SemanticNone,         0            , DrawableObjectAttributeGlyphs, u"glyphs", u"Glyphs", u"0 1 2 3 4", glyphDefaults },
    {Attribute::TypeArrayFloat32,   Attribute::SemanticNone,         0            , DrawableObjectAttributeAdvances, u"advances", u"Advances", u"",{} },
    {Attribute::TypeArrayFloat32,   Attribute::SemanticDelta,        0            , DrawableObjectAttributeOffsets, u"offsets", u"Offsets", u"",{} },
    {Attribute::TypeFloat32,        Attribute::SemanticNone,         CategoryLight, DrawableObjectAttributeFontSize, u"font_size", u"Font size (DIPs)", u"16", fontSizes },
    {Attribute::TypeString16,       Attribute::SemanticNone,         CategoryLight, DrawableObjectAttributeFontFamily, u"font_family", u"Font family", u"Segoe UI",{} },
    {Attribute::TypeUInteger32,     Attribute::SemanticEnum,         CategoryLight, DrawableObjectAttributeWeight, u"weight", u"Weight", u"400", weights },
    {Attribute::TypeUInteger32,     Attribute::SemanticEnum,         CategoryLight, DrawableObjectAttributeStretch, u"stretch", u"Stretch", u"Normal", stretches },
    {Attribute::TypeUInteger32,     Attribute::SemanticEnum,         CategoryLight, DrawableObjectAttributeSlope, u"slope", u"Slope", u"Regular", slopes },
    {Attribute::TypeString16,       Attribute::SemanticFilePath,     0            , DrawableObjectAttributeFontFilePath, u"font_file_path", u"Font file path", u"",{} },
    {Attribute::TypeUInteger32,     Attribute::SemanticNone,         0            , DrawableObjectAttributeFontFaceIndex, u"face_index", u"Face index", u"0",{} },
    {Attribute::TypeUInteger32,     Attribute::SemanticEnumExclusive,0            , DrawableObjectAttributeFontSimulations, u"font_simulations", u"Font simulations", u"None", fontSimulations },
    {Attribute::TypeUInteger32,     Attribute::SemanticEnumExclusive,0            , DrawableObjectAttributeDWriteFontFaceType, u"dwrite_font_face_type", u"DWrite font face type", u"None", dwriteFontFaceTypes },
    {Attribute::TypeFloat32,        Attribute::SemanticNone,         0            , DrawableObjectAttributePadding, u"padding", u"Padding", u"8",{} },
    {Attribute::TypeFloat32,        Attribute::SemanticNone,         CategoryLight, DrawableObjectAttributeWidth, u"width", u"Width", u"300", layoutSizes },
    {Attribute::TypeFloat32,        Attribute::SemanticNone,         CategoryLight, DrawableObjectAttributeHeight, u"height", u"Height", u"50", layoutSizes },
    {Attribute::TypeArrayFloat32,   Attribute::SemanticNone,         0            , DrawableObjectAttributePosition, u"position", u"Position X Y", u"0 0",{} },
    {Attribute::TypeArrayFloat32,   Attribute::SemanticNone,         0            , DrawableObjectAttributeTransform, u"transform", u"Transform", u"1 0 0 1 0 0", transforms },
    {Attribute::TypeUInteger32,     Attribute::SemanticNone,         0            , DrawableObjectAttributePixelZoom, u"pixel_zoom", u"Pixel zoom", u"1", pixelZooms },
    {Attribute::TypeUInteger32,     Attribute::SemanticEnumExclusive,CategoryLight, DrawableObjectAttributeReadingDirection, u"reading_direction", u"Reading direction", u"LTR TTB", readingDirections },
    {Attribute::TypeUInteger32,     Attribute::SemanticEnumExclusive,0            , DrawableObjectAttributeColumnAlignment, u"column_alignment", u"Column alignment", u"leading", alignments },
    {Attribute::TypeUInteger32,     Attribute::SemanticEnumExclusive,0            , DrawableObjectAttributeRowAlignment, u"row_alignment", u"Row alignment", u"leading", alignments },
    {Attribute::TypeUInteger32,     Attribute::SemanticEnumExclusive,0            , DrawableObjectAttributeJustification, u"justification", u"Justification", u"unjustified", justifications },
    {Attribute::TypeArrayUInteger32,Attribute::SemanticCharacterTags,CategoryLight, DrawableObjectAttributeTypographicFeatures, u"typographic_features", u"Typographic features", u"", typographicFeatures },
    {Attribute::TypeUInteger32,     Attribute::SemanticEnumExclusive,CategoryLight, DrawableObjectAttributeLineWrappingMode, u"wrapping_mode", u"Wrapping mode", u"", wrappingModes },
    {Attribute::TypeUInteger32,     Attribute::SemanticEnumExclusive,0,             DrawableObjectAttributeDWriteRenderingMode, u"dwrite_rendering_mode", u"DWrite rendering mode", u"", dwriteRenderingModes },
    {Attribute::TypeUInteger32,     Attribute::SemanticEnumExclusive,0,             DrawableObjectAttributeGdiRenderingMode, u"gdi_rendering_mode", u"GDI rendering mode", u"", gdiRenderingModes },
    {Attribute::TypeUInteger32,     Attribute::SemanticEnumExclusive,0,             DrawableObjectAttributeGdiPlusRenderingMode, u"gdiplus_rendering_mode", u"GDI+ rendering mode", u"", gdiPlusRenderingModes },
    {Attribute::TypeUInteger32,     Attribute::SemanticEnumExclusive,0            , DrawableObjectAttributeDWriteMeasuringMode, u"dwrite_measuring_mode", u"DWrite measuring mode", u"", dwriteMeasuringModes },
    {Attribute::TypeUInteger32,     Attribute::SemanticEnumExclusive,0            , DrawableObjectAttributeDWriteVerticalGlyphOrientation, u"dwrite_vertical_glyph_orientation", u"DWrite vertical glyph orientation", u"", dwriteVerticalGlyphOrientation },
    {Attribute::TypeString16,       Attribute::SemanticNone,         CategoryLight, DrawableObjectAttributeLanguageList, u"language_list", u"Language list", u"", languages },
    {Attribute::TypeUInteger32,     Attribute::SemanticColor,        0            , DrawableObjectAttributeTextColor, u"text_color", u"Text color", u"000000", textColors, u"Color as #FFFFFFFF, 255 128 0, or name" },
    {Attribute::TypeUInteger32,     Attribute::SemanticColor,        0            , DrawableObjectAttributeBackColor, u"back_color", u"Back color", u"#FFFFFFFF", textColors, u"Color as #FFFFFFFF, 255 128 0, or name"},
    {Attribute::TypeUInteger32,     Attribute::SemanticColor,        0            , DrawableObjectAttributeLayoutColor, u"layout_color", u"Layout color", u"#FF7FFFD4", textColors, u"Color as #FFFFFFFF, 255 128 0, or name"},
    {Attribute::TypeUInteger32,     Attribute::SemanticNone,         CategoryLight, DrawableObjectAttributeColorPaletteIndex, u"color_palette_index", u"Color palette index", u"0xFFFFFFFF", colorPaletteIndices },
    {Attribute::TypeBool8,          Attribute::SemanticEnumExclusive,CategoryLight, DrawableObjectAttributeColorFont, u"color_font", u"Color font", u"on", enabledValues },
    {Attribute::TypeBool8,          Attribute::SemanticEnumExclusive,0            , DrawableObjectAttributePixelSnapping, u"pixel_snapping", u"Pixel Snapping", u"on", enabledValues },
    {Attribute::TypeBool8,          Attribute::SemanticEnumExclusive,0            , DrawableObjectAttributeClipping, u"clipping", u"Clipping", u"off", enabledValues },
    {Attribute::TypeBool8,          Attribute::SemanticEnumExclusive,0            , DrawableObjectAttributeUnderline, u"underline", u"Underline", u"off", enabledValues },
    {Attribute::TypeBool8,          Attribute::SemanticEnumExclusive,0            , DrawableObjectAttributeStrikethrough, u"strikethrough", u"Strikethrough", u"off", enabledValues },
    {Attribute::TypeBool8,          Attribute::SemanticEnumExclusive,0            , DrawableObjectAttributeFontFallback, u"font_fallback", u"Font fallback", u"on", enabledValues },
    {Attribute::TypeFloat32,        Attribute::SemanticNone,         0            , DrawableObjectAttributeTabWidth, u"tab_width", u"Tab width", u"40",{} },
    {Attribute::TypeUInteger32,     Attribute::SemanticEnumExclusive,0            , DrawableObjectAttributeHotkeyMode, u"hotkey_mode", u"Hotkey mode", u"None", hotkeyDisplays },
    {Attribute::TypeUInteger32,     Attribute::SemanticEnumExclusive,0            , DrawableObjectAttributeTrimmingGranularity, u"trimming_granularity", u"Trimming granularity", u"40", trimmingGranularities },
    {Attribute::TypeBool8,          Attribute::SemanticEnumExclusive,0            , DrawableObjectAttributeTrimmingSign, u"trimming_sign", u"Trimming sign", u"enabled", enabledValues },
    {Attribute::TypeCharacter32,    Attribute::SemanticNone,         0            , DrawableObjectAttributeTrimmingDelimiter, u"trimming_delimiter", u"Trimming delimiter", u"", trimmingDelimiters },
    {Attribute::TypeBool8,          Attribute::SemanticNone,         0            , DrawableObjectAttributeUser32DrawTextAsEditControl, u"user32_drawtext_edit_control", u"User32 DrawText DT_EDITCONTROL", u"", enabledValues },
};
static_assert(DrawableObjectAttributeTotal == 50, "A new attribute enum has been added. Update this table.");


const Attribute::PredefinedValue DrawableObject::functions[] = {
    { DrawableObjectFunctionNop, u"Nop"},
    { DrawableObjectFunctionDWriteBitmapRenderTargetLayoutDraw, u"IDWriteBitmapRenderTarget IDWriteTextLayout"},
    { DrawableObjectFunctionDirect2DDrawTextLayout, u"D2D DrawTextLayout"},
    { DrawableObjectFunctionDirect2DDrawText, u"D2D DrawText"},
    { DrawableObjectFunctionUser32DrawText, u"User32 DrawText" },
    { DrawableObjectFunctionGdiPlusDrawString, u"GDIPlus DrawString" },
    { DrawableObjectFunctionDWriteBitmapRenderTargetDrawGlyphRun, u"IDWriteBitmapRenderTarget DrawGlyphRun" },
    { DrawableObjectFunctionDirect2DDrawGlyphRun, u"D2D DrawGlyphRun"},
    { DrawableObjectFunctionGdiTextOut, u"GDI ExtTextOut" },
    { DrawableObjectFunctionGdiPlusDrawDriverString, u"GDIPlus DrawDriverString"},
    { DrawableObjectFunctionDrawColorBitmapGlyphRun, u"D2D DrawColorBitmapGlyphRun"},
    { DrawableObjectFunctionDrawSvgGlyphRun, u"D2D DrawSvgGlyphRun"},
    #if 0
    { , u"Uniscribe ScriptStringOut"},
    { , u"GDI+ DrawString"},
    { , u"GDI+ DrawDriverString"},
    { , u"GDI GetCharacterPlacement"},
    { , u"GDI GetGlyphIndices"},
    { , u"User32 EDIT"},
    { , u"RichEdit"},
    { , u"Trident"},
    #endif
};

const Attribute::PredefinedValue DrawableObject::visibilities[] = {
    {1, u"visible" },
    {0, u"hidden" },
};

const Attribute::PredefinedValue DrawableObject::enabledValues[] = {
    {0, u"off" },
    {1, u"on"},
};

const Attribute::PredefinedValue DrawableObject::textDefaults[] = {
    {0, u"Empty text", u""},
    {0, u"Feature tests",
        u"Multiple short words for line wrapping\r\n"
        u"SupercalifragilisticexpialidociusIsALongWord\r\n"
        u"あいう𐎀𐎁𐎂𐐀𐐁𐐂\r\n"
        u"‫‬⁡ - hidden characters\r\n"
        u"&File && &Edit\r\n"
        u"A\tTab\tIs\tHere\r\n"
        u"םֱמֱןֱנֱסֱעֱףֱפֱץֱצֱקֱרֱשֱת\r\n"
        u"!? \u001Flre\u001F<\u202A!?\u202C> \u001Frle\u001F<\u202B!?\u202C> \u001Flro\u001F<\u202D!?\u202C> \u001Frlo\u001F<\u202E!?\u202C> \u001Flrm\u001F<\u200E!?\u200E> \u001Frlm\u001F<\u200F!?\u200F>\r\n"
        u"🀀🁶🂡🌄🌠🐧😎🚀🚤♤♥♦♧♨\r\n"
        u"あいう‫‬⁡𐎀𐎁𐎂𐐀𐐁𐐂\r\n"
        u"c:\\windows\\fonts\\arial.ttf\r\n"
        u"〔㊖：㊙あ：い㌀𠀀𠀁𠁤芦󠄴芦󠄴卿卿󠄁。〕"
        u"⇦⇧⇨⇩←↑→↓⇐⇑⇒⇓⇔⇠⇡⇢↞↟↠"
    },
    {0, u"English pangram", u"the quick brown fox jumps over the lazy dog."},
    {0, u"Arabic pangram", u"صِف خَلقَ خَودِ كَمِثلِ الشَمسِ إِذ بَزَغَت - يَحظى الضَجيعُ بِها نَجلاءَ مِعطارِ" },
    {0, u"Bulgarian pangram", u"За миг бях в чужд плюшен скърцащ фотьойл." },
    {0, u"Greek pangram", u"Τάχιστη αλώπηξ βαφής ψημένη γη, δρασκελίζει υπέρ νωθρού κυνός" },
    {0, u"Hebrew pangram", u"דג סקרן שט בים מאוכזב ולפתע מצא חברה" },
    {0, u"Icelandic pangram", u"Kæmi ný öxi hér, ykist þjófum nú bæði víl og ádrepa." },
    {0, u"Japanese pangram", u"とりなくこゑす ゆめさませ みよあけわたる ひんかしを そらいろはえて おきつへに ほふねむれゐぬ もやのうち" },
    {0, u"Korean pangram", u"키스의 고유조건은 입술끼리 만나야 하고 특별한 기술은 필요치 않다." },
    {0, u"Myanmar pangram", u"သီဟိုဠ်မှ ဉာဏ်ကြီးရှင်သည် အာယုဝဍ္ဎနဆေးညွှန်းစာကို ဇလွန်ဈေးဘေးဗာဒံပင်ထက် အဓိဋ္ဌာန်လျက် ဂဃနဏဖတ်ခဲ့သည်။" },
    {0, u"Thai pangram", u"เป็นมนุษย์สุดประเสริฐเลิศคุณค่า กว่าบรรดาฝูงสัตว์เดรัจฉาน จงฝ่าฟันพัฒนาวิชาการ อย่าล้างผลาญฤๅเข่นฆ่าบีฑาใคร ไม่ถือโทษโกรธแช่งซัดฮึดฮัดด่า หัดอภัยเหมือนกีฬาอัชฌาสัย ปฏิบัติประพฤติกฎกำหนดใจ พูดจาให้จ๊ะๆ จ๋าๆ น่าฟังเอยฯ" },
    {0, u"Arabic", u"عربية" },
    {0, u"Arabic Persian", u"یونی‌کُد" },
    {0, u"Armenian", u"Յունիկօդ" },
    {0, u"Bengali", u"য়ূনিকোড" },
    {0, u"Bopomofo", u"ㄊㄨㄥ˅ ㄧˋ ㄇㄚ˅" },
    {0, u"Braille", u"⠠⠃⠗⠁⠊⠇⠇⠑" },
    {0, u"Canadian Aboriginal", u"ᔫᗂᑰᑦ" },
    {0, u"Cherokee", u"ᏳᏂᎪᏛ" },
    {0, u"Cyrillic", u"Юникод" },
    {0, u"Devanagari", u"यूनिकोड" },
    {0, u"Ethiopic", u"ዩኒኮድ" },
    {0, u"Georgian", u"უნიკოდი" },
    {0, u"Greek", u"Γιούνικοντ" },
    {0, u"Gujarati", u"યૂનિકોડ" },
    {0, u"Gurmukhi", u"ਯੂਨਿਕੋਡ" },
    {0, u"Han", u"统一码" },
    {0, u"Han Ext-B", u"𠀀𠀁𠀂𠀃𠀄𠀅𠀆𠀇𠀈𠀉𠀊𠀋𠀌𠀍𠀎𠀏" },
    {0, u"Hangul", u"유니코드" },
    {0, u"Hangul Old", u"갭〔걺〮겈〯〕" },
    {0, u"Hebrew", u"יוניקוד" },
    {0, u"Japanese", u"ひらがな、カタカナ、漢字" },
    {0, u"Javanese", u"ꦩꦸꦭ꧀ꦪꦏ꧀ꦏꦏꦺꦈꦩ꧀ꦧꦸꦭ꧀ꦧꦶꦤꦁꦔꦸꦤ꧀ꦠꦩ꧀ꦩꦤ꧀ꦱꦂꦫꦶꦏꦊꦏ꧀ꦱꦤ꧀ꦤꦏ꧀ꦏꦏꦼꦤ꧀ꦏꦤ꧀ꦛꦶꦥꦚꦼꦁꦏꦸꦪꦸꦁꦔꦶꦥꦸꦤ꧀ꦪꦪꦱ꧀ꦱꦤ꧀ꦏꦭ꧀ꦭꦲꦸꦱ꧀ꦠꦼꦒꦸꦭ꧀ꦧꦼꦤ꧀ꦏꦶꦲꦤ꧀ꦱꦲꦏꦂꦉꦱ꧀ꦩꦺꦏ꧀ꦏꦏꦼꦤ꧀ꦢꦶꦤꦶꦁꦲꦶꦁꦏꦁꦩꦶꦤꦸꦭ꧀ꦪꦯꦿꦶꦯꦸꦭ꧀ꦠꦤ꧀ꦲꦩꦁꦏꦸꦨꦸꦮꦤ ꧇꧑꧐꧇ ꦒꦸꦧꦼꦂꦤꦸꦂꦝꦃꦲꦺꦫꦃꦆꦱ꧀ꦠꦶꦩꦺꦮꦃꦪꦺꦴꦒꦾꦏꦂꦠꦠꦸꦮꦶꦤ꧀ꦲꦶꦁꦏꦁꦩꦶꦤꦸꦭ꧀ꦪꦠꦸꦮꦤ꧀ꦪꦺꦴꦱꦺꦧ꧀ꦭꦤ꧀ꦕꦺꦴ ꧈ ꦩꦶꦤꦺꦴꦁꦏꦮꦏꦶꦭ꧀ꦪꦪꦱꦤ꧀ꦏꦭ꧀ꦭꦺꦴꦲꦸꦱ꧀ꦠꦼꦒꦸꦭ꧀ꦧꦺꦤꦏꦶꦲꦤ꧀꧇ (ꦥꦺꦴꦂꦠꦸꦒꦭ꧀) ꦲꦶꦁꦱꦸꦂꦪꦏꦥꦶꦁ ꧇꧒꧒꧇ ꦄꦒꦸꦱ꧀ꦠꦸꦱ꧀ ꧇꧒꧐꧐꧔"},
    {0, u"Hiragana", u"ゆにこおど" },
    {0, u"Katakana", u"ユニコード" },
    {0, u"Kannada", u"ಯೂನಿಕೋಡ್" },
    {0, u"Khmer", u"យូនីគោដ" },
    {0, u"Latin IPA", u"ˈjunɪˌkoːd" },
    {0, u"Lao", u"ພາສາລາວ" },
    {0, u"Malayalam", u"യൂനികോഡ്" },
    {0, u"Mongolian", u"ᠮᠣᠩᠭᠣᠯ" },
    {0, u"Myanmar", u"မြန်မာအက္ခရာ" },
    {0, u"N'Ko", u"ߒߞߏ" },
    {0, u"Ogham", u"ᚔᚒᚅᚔᚉᚑᚇ" },
    {0, u"Oriya", u"ୟୂନିକୋଡ" },
    {0, u"'Phags-pa", u"ꡁꡅꡜꡤ" },
    {0, u"Runic", u"ᛡᚢᚾᛁᚳᚩᛞ" },
    {0, u"Sinhala", u"යණනිකෞද්" },
    {0, u"Syriac", u"ܝܘܢܝܩܘܕ" },
    {0, u"Tamil", u"யூனிகோட்" },
    {0, u"Telegu", u"యూనికోడ్" },
    {0, u"Thai", u"ภาษาไทย, สวัสดีเช้านี้" },
    {0, u"Tibetan (Dzongkha)", u"ཨུ་ནི་ཀོཌྲ། - ཝེ་ཁེ་རིག་མཛོད་ནི་རང་དབང་གི་རིག་གནས་ཀུན་བཏུས་དཔེ་མཛོད་ཅིག་ཡིན་ལ་འདི་རུ་མི་སུས་ཀྱང་རྩོམ་སྒྲིག་ཆོག གལ་ཏེ་ཁྱེད་ཀྱིས་བོད་ཡིག་མཁྱེན་ན་ལམ་སེང་རྩོམ་ཡིག་སྤེལ་ཆོག་མ་ཟད་ནང་དུ་ཉར་ཟིན་པའི་རྩོམ་ཡིག་ཐམས་ཅད་བོད་ཡིག་གཉེར་མཁན་ཚང་མར་ཕན་པས་ཝེ་ཁེ་བོད་འགྱུར་མའི་ནང་དུ་ཧུར་བརྩོན་གྱིས་རྩོམ་ཡིག་ཁ་སྣོན་མཛོད་དང་ལས་འདིས་བོད་ཡིག་མི་ཉམས་གོང་འཕེལ་དུ་གཏོང་བར་ཧ་ཅང་དགེ་མཚན་མཆིས་སོ། །" },
    {0, u"Vai", u"ꕙꔤ" },
    {0, u"Yi", u"ꆈꌠꁱꂷ" },
    {0, u"Numbers", u"0123,456.789" },
    {0, u"Emoji", u"🀀🁶🂡🌄🌠🐧😎🚀🚤♤♥♦♧♨" },
    {0, u"Variation selectors",
        u"᠋ - Mongolian VS\r\n"
        u"︀ - U+FE00 VS\r\n"
        u"󠄀 - U+E0100 VS\r\n"
        u"倦倦󠄀倦 - VS middle ideograph\r\n"
    },
    {0, u"Surrogate pairs",
        u"𠠺 <- complete\r\n"
        u"\xD800 <- leading only\r\n"
        u"\xDC00 <- trailing only\r\n"
        u"\xDC00\xD800 <- reversed\r\n"
        u"𠀀𠀁𠁤🌄🌠🐧"
    },
    {0, u"Arrows",
        u"Arrows:\r\n"
        u"←↑→↓↔↕↖↗↘↙↚↛↜↝↞↟↠↡↢↣↤↥↦↧↨↩↪↫↬↭↮↯↰↱↲↳↴↵↶↷↸↹↺↻↼↽↾↿⇀⇁⇂⇃⇄⇅⇆⇇⇈⇉⇊⇋⇌⇍⇎⇏⇐⇑⇒⇓⇔⇕⇖⇗⇘⇙⇚⇛⇜⇝⇞⇟⇠⇡⇢⇣⇤⇥⇦⇧⇨⇩⇪⇫⇬⇭⇮⇯⇰⇱⇲⇳⇴⇵⇶⇷⇸⇹⇺⇻⇼⇽⇾⇿\r\n"
        u"\r\n"
        u"Dingbat arrows:\r\n"
        u"➘➙➚➛➜➝➞➟➠➡➢➣➤➥➦➧➨➩➪➫➬➭➮➯➱➲➳➔➴➵➶➷➸➹➺➻➼➽➾\r\n"
        u"\r\n"
        u"Supplemental arrows A:\r\n"
        u"⟰⟱⟲⟳⟴⟵⟶⟷⟸⟹⟺⟻⟼⟽⟾⟿\r\n"
        u"\r\n"
        u"Supplemental arrows B:\r\n"
        u"⤀⤁⤂⤃⤄⤅⤆⤇⤈⤉⤊⤋⤌⤍⤎⤏⤐⤑⤒⤓⤔⤕⤖⤗⤘⤙⤚⤛⤜⤝⤞⤟⤠⤡⤢⤣⤤⤥⤦⤧⤨⤩⤪⤫⤬⤭⤮⤯⤰⤱⤲⤳⤴⤵⤶⤷⤸⤹⤺⤻⤼⤽⤾⤿⥀⥁⥂⥃⥄⥅⥆⥇⥈⥉⥊⥋⥌⥍⥎⥏⥐⥑⥒⥓⥔⥕⥖⥗⥘⥙⥚⥛⥜⥝⥞⥟⥠⥡⥢⥣⥤⥥⥦⥧⥨⥩⥪⥫⥬⥭⥮⥯⥰⥱⥲⥳⥴⥵⥶⥷⥸⥹⥺⥻⥼⥽⥾⥿\r\n"
        u"\r\n"
        u"Miscellaneous symbols and arrows:\r\n"
        u"⬀⬁⬂⬃⬄⬅⬆⬇⬈⬉⬊⬋⬌⬍⬎⬏⬐⬑⬰⬱⬲⬳⬴⬵⬶⬷⬸⬹⬺⬻⬼⬽⬾⬿⭀⭁⭂⭃⭄⭅⭆⭇⭈⭉⭊⭋⭌\r\n"
        u"\r\n"
        u"Miscellaneous symbols (pointers):\r\n"
        u"☚☛☜☝☞☟\r\n"
        u"\r\n"
        u"Non-pointers (but sometimes change orientation):\r\n"
        u"✂✄\r\n"
        u"\r\n"
        u"Consistency check:\r\n"
        u"←→↑↓\r\n"
        u"↤↦↥↧\r\n"
        u"⇐⇒⇑⇓\r\n"
        u"⇠⇢⇡⇣\r\n"
        u"⇦⇨⇧⇩"
    },
    {0, u"Parentheses",
        u"Broken in LTR:\r\n"
        u"גלויה (4x6)\r\n"
        u"צבע מלא (24bpp)\r\n"
        u"تفريغ ذاكرة صغيرة (128 كيلوبايت)\r\n"
        u"Dump זיכרון קטן (128 KB)\r\n"
        u"صغير جداً (0 - 2 غيغابايت)\r\n"
        u"צבע מלא (24bpp)\r\n"
        u"\r\n"
        u"Broken in RTL:\r\n"
        u"זיכרון מצב מוגן של MS-DOS (DPMI)\r\n"
        u"מקורות נתונים (ODBC)\r\n"
        u"מערכת ההפעלה (WCAG) 2.0\r\n"
        u"WWW (World Wide Web) מערכת\r\n"
        u"Dump זיכרון קטן (128 KB)\r\n"
        u"\r\n"
        u"Not solved:\r\n"
        u"גלויה (4\"x6\")\r\n"
        u"שגיאת EnableEUDC()\r\n"
        u"خطأ EnableEUDC()\r\n"
        u"© Microsoft Corporation.\r\n"
        u"כברירת מחדל (96 DPI) כלילת\r\n"
        u"\r\n"
        u"Correct as-is:\r\n"
        u"All Files (*.*)"
    },
    {0, u"Brackets:",
        u"()[]{}༺༻༼༽᚛᚜‚„⁅⁆⁽⁾₍₎〈〉❨❩❪❫❬❭❮❯❰❱❲❳❴❵⟅⟆⟦⟧⟨⟩⟪⟫⟬⟭⟮⟯⦃⦄⦅⦆⦇⦈⦉⦊⦋⦌⦍⦎⦏⦐⦑⦒⦓⦔⦕⦖⦗⦘⧘⧙⧚⧛⧼⧽⸢⸣⸤⸥⸦⸧⸨⸩〈〉《》「」『』【】〔〕〖〗〘〙〚〛〝〞〟﴾﴿︗︘︵︶︷︸︹︺︻︼︽︾︿﹀﹁﹂﹃﹄﹇﹈﹙﹚﹛﹜﹝﹞（）［］｛｝｟｠｢｣\r\n"
        u"\r\n"
        u"NonCJK:\r\n"
        u"()[]{}༺༻༼༽᚛᚜⁅⁆⁽⁾₍₎〈〉❨❩❪❫❬❭❮❯❰❱❲❳❴❵⟅⟆⟦⟧⟨⟩⟪⟫⟬⟭⟮⟯⦃⦄⦅⦆⦇⦈⦉⦊⦋⦌⦍⦎⦏⦐⦑⦒⦓⦔⦕⦖⦗⦘⧘⧙⧚⧛⧼⧽⸢⸣⸤⸥⸦⸧⸨⸩﴾﴿\r\n"
        u"\r\n"
        u"Full width:\r\n"
        u"〈〉《》「」『』【】〔〕〖〗〘〙〚〛｟｠\r\n"
        u"\r\n"
        u"Vertical presentation forms:\r\n"
        u"︗︘︵︶︷︸︹︺︻︼︽︾︿﹀﹁﹂﹃﹄﹇﹈\r\n"
        u"\r\n"
        u"Smallform variants:\r\n"
        u"﹙﹚﹛﹜﹝﹞\r\n"
        u"\r\n"
        u"Fullwidth western forms:\r\n"
        u"（）［］｛｝\r\n"
        u"\r\n"
        u"Halfwidth:\r\n"
        u"｢｣\r\n"
        u"\r\n"
        u"NonCJK: ‚„\r\n"
        u"Fullwidth: 〝〞〟\r\n"
        u"\r\n"
        u"Reference text:\r\n"
        u"WesternひらがなＦＵＬＬＷＩＤＴＨ"
    },
    {0, u"Diacritics",
        u"a  b̃̊  c  Ä  Ä  Ä̃  ÄB̆ĈD̃\r\n"
        u"diăcritićş diăcritićs̹\r\n"
        u"What a concrète resumé you have.\r\n"
        u"[び]  [ひ￿゙]  [ご] [ご]\r\n"
        u"ぉ゚ が ̂MM̂̂ ̂DD̂̂ Phiệt"
    },
    {0, u"Ligation",
        u"Devanagari: 'क' + 'ो' -> 'को'\r\n"
        u"Kannada: 'ಕ' + 'ೋ' -> 'ಕೋ'\r\n"
        u"Tamil: 'ட' + 'ொ' -> 'டொ'"
    },
    {0, u"Numeric context",
        u"กขฃ 123 hello 123 กขฃ 123 \r\n"
        u"1234\r\n"
        u"سش 1234\r\n"
        u"سش 1234 آأسشص"
    },
    {0, u"Private User Area",
        u""
    },
    {0, u"Vertical ties",
        u"8（８）\r\n"
        u"“abc”⸂a⸃\r\n"
        u"a˦bc˪˵a˶a⁀b\r\n"
        u"c‒d３－５\r\n"
        u"a‐‑‒–— ―‖‗b\r\n"
        u"a﹍a⁀b 車⁀落\r\n"
        u"ぺa⏜bc ab⎴c⎵d⎶⏢ ⏳⏚⏳⏁⌛⌚ⅨⅩⅪⅪ⇐⇑⇒├┝┠⎺⏏⎺⏌⏐⏐⏐⎯"
    },
#if 0
    {0, u"Global sampler",
        u"Arabic: أصبح بوسعك الآن تنزيل نسخة كاملة من موسوعة ويكيبيديا العربية لتصفحها بلا اتصال بالإنترنت.\r\n\r\n"
        u"Armenian: Հայերենը հնդեվրոպական լեզուների ընտանիքի նույնանուն խմբի լեզու է։ Այն Հայաստանի Հանրապետության և Լեռնային Ղարաբաղի պետական լեզուն է։\r\n\r\n"
        u"Bengla: ভাষা (ইংরেজি: Language) ধারণাটির কোন সুনির্দিষ্ট, যৌক্তিক ও অবিতর্কিত সংজ্ঞা দেয়া কঠিন, কেননা যেকোন কিছুর সংজ্ঞা ভাষার মাধ্যমেই দিতে হয়। \r\n\r\n"
        u"Buginese: ᨅᨔ ᨕᨗᨁᨗ ᨊᨄᨅᨗᨌᨑᨕᨗ 4 ᨍᨘᨈᨊ ᨈᨕᨘ ᨊᨔᨛ ᨈᨕᨘ ᨕᨘᨁᨗ ᨑᨗ ᨈᨊ ᨕᨘᨁᨗ (ᨔᨘᨒᨙᨓᨔᨗ ᨔᨛᨒᨈ, ᨕᨗᨉᨚᨙᨊᨔᨗᨕ) ᨄᨀᨚᨑᨚ ᨈᨚ ᨑᨗ ᨆᨒᨕᨗᨔᨗᨕ᨞ ᨕᨛᨃ ᨕᨔᨑ ᨊ ᨊᨔᨛᨂᨗ ᨖᨘᨑᨘᨄᨘ ᨔᨘᨒᨄ ᨑᨄ᨞\r\n\r\n"
        u"Canadian Syllabics: ᑕᒪᒃᑯᐊ ᐊᕕᒃᓯᒪᔭᕆᐊᓖᑦ, ᐅᕙᓂ ᐃᓪᓗᒧᑦ ᐃᓯᓚᐅᖅᑐᖓ, ᓇᓪᓕᐊᑐᐃᓐᓇᖅ ᓯᕗᓪᓕᐅᒐᓗᐊᕈᓂ ᓱᖁᑕᐅᙱᑦᑐᖅ: ᐃᓯᓚᐅᖅᑐᖓ ᐃᓪᓗᒧᑦ ᑐᑭᖃᕈᓐᓇᕆᕗᖅ ᐃᓪᓗᒧᑦ ᐃᓯᓚᐅᖅᑐᖓ ᐋᖅᑭᒃᓯᒪᓂᖓᓂᒃ. ᑖᓐᓇ (–ᒧᑦ) ᐅᖃᐅᑎᐅᑉ ᓯᓂᖓ ᐅᖃᐅᑎᒋᔭᐅᔪᒥᒃ ᓇᓗᓇᐃᒃᑯᑕᐅᙱᒻᒪᑦ ᐃᓚᓕᐅᑎᒐᑐᐃᓐᓇᐅᒐᒥ ᐸᑐᒃᓯᒪᔭᕆᐊᓕᒃ ᐅᖃᐅᑕᐅᔪᒥᒃ ᓇᓗᓇᐃᒃᑯᑕᓕᖕᒧᑦ, ᐆᑦᑑᑎᒋᔭᑦᑎᓐᓂ ᐅᖃᐅᑕᐅᔪᖅ ᓯᓂᖃᖅᐳᖅ (–ᑐᖓ)-ᒥᒃ ᐅᖃᐅᓯᖃᕐᒪᑦ ᐱᓕᕆᔪᒥᒃ.\r\n\r\n"
        u"Cherokee: ᎠᏰᎵ ᏚᎾᏙᏢᏒ ᎠᎴ ᎦᏚ ᎠᏄᏬᏍᏗ ᎤᏬᎳᏨ ᎾᎥᎢ ᎯᎠ ᎠᏫᏒᏗ ᎧᏃᎮᎭ ᎠᏰᎵ ᎤᏙᏢᏒ, ᎠᎴ ᎾᏓᏛᏁᎲ 250,000 ᏂᎬᎾᏛ ᎠᏰᎵ ᎤᏬᎳᏨ ᏣᎳᎩ, ᎤᎭ ᎠᏰᎵ ᎭᏫᎾᏗᏢ ᏓᎵᏆ, ᎣᎦᎳᎰᎻ (ᎯᎠ ᏣᎳᎩ ᎠᏰᎵ ᎤᏙᏢᏒ ᎠᎴ ᎠᏫᏒᏗ ᎩᏚᏩ ᏗᏂᏤᎷᎯᏍᎩ ᏣᎳᎩ ᎠᏂᏴᏫ) ᎠᎴ ᎾᎾᎢ ᏣᎳᎩ, ᎤᏴᏢ ᎧᎶᎵᎾ (ᎧᎸᎬᎢᏗᏢ ᏗᏂᏤᎷᎯᏍᎩ ᏣᎳᎩ ᎠᏂᏴᏫ). ᎤᏔᏂᏗ ᎦᏙᎯ-ᎤᏬᎳᏨ ᏣᎳᎩ ᎠᏂᎳᏍᏓᎸ ᎤᎭ ᎠᏰᎵ ᎭᏫᎾᏗᏢ ᏣᏥᏱ, ᏨᎫᎵ ᎠᎴ ᎠᎳᏆᎹ. ᏐᎢ ᎡᏆ ᎠᎴ ᎤᏍᏗ ᎬᏙᏗ-ᎤᏬᎳᏨ ᏣᎳᎩ ᏧᎾᏙᏢᎯ ᎠᎴ ᎤᏂᎷᏨ ᎭᏫᎾᏗᏢ ᏲᏩᏁᎬ, ᎻᏑᎵ, ᏖᎾᏏ, ᎠᎴ ᏐᎢ ᎦᎷᎯᏍᏗ ᎭᏫᎾᏗᏢ ᎯᎠ ᎠᏫᏒᏗ ᎧᏃᎮᎭ.\r\n\r\n"
        u"Coptic: ⲙⲁⲛ̀ⲛⲁⲧ ϫⲉ ⲧⲉⲛⲁⲥⲡⲓ ⲛ̀ⲣⲉⲙⲛ̀ⲭⲏⲙⲓ ⲟⲩⲟⲛ ϩⲁⲛⲙⲁⲛ̀ⲛⲁⲧ ⲉⲑⲛⲁⲛⲉϥ ⲉⲑⲃⲉ ϯⲁⲥⲡⲓ ⲛ̀ⲣⲉⲙⲛⲭⲏⲙⲓ ϧⲉⲛ ⲡⲓⲟⲩⲧⲉϣⲛⲉ. ⲟⲩⲙⲁⲛ̀ⲛⲁⲧ ⲉⲑⲛⲁⲁϥ ⲉϥⲱⲗⲓ ϩⲁⲛⲙⲏϣ ⲛ̀ⲗⲱⲟⲩ ⲛⲉⲙ ϩⲁⲛⲥϧⲁⲓ ⲛ̀ϯⲙⲉⲧⲣⲉⲙⲛ̀ⲭⲏⲙⲓ ϫⲉ ⲧⲉⲛⲁⲥⲡⲓ ⲛ̀ⲣⲉⲙⲛ̀ⲭⲏⲙⲓ ⲡⲉ.\r\n\r\n"
        u"Cyrillic: Язы́к — знаковая система, соотносящая понятийное содержание и типовое звучание (написание). Языки изучает лингвистика (языкознание). Знаковые системы вообще — предмет изучения семиотики. Влияние структуры языка на человеческое мышление и поведение изучается психолингвистикой.\r\n\r\n"
        u"Devanagari: भाषा वह साधन है जिसके द्वारा हम अपने विचारों को व्यक्त करते है और इसके लिये हम वाचिक ध्वनियों का उपयोग करते हैं।\r\n\r\n"
        u"Ethiopic: ቋንቋ የድምጽ፣ የምልክት ወይም የምስል ቅንብር ሆኖ ለማሰብ ወይም የታሰበን ሃሳብ ለሌላ ለማስተላለፍ የሚረዳ መሳሪያ ነው። በአጭሩ ቋንቋ የምልክቶች ስርዓትና እኒህን ምልክቶች ለማቀናበር የሚያስፈልጉ ህጎች ጥንቅር ነው።\r\n\r\n"
        u"Georgian: ენა — ენათმეცნიერული ცნება-ტერმინი, რომელიც აკუსტიკურად და ოპტიკურად აღქმადი ნიშნების სისტემას აღნიშნავს.\r\n\r\n"
        u"Glagolitic: ⰄⰑⰁⰓⰡ ⰒⰓⰋⰕⰋ ⰂⰟ ⰂⰋⰍⰋⰒⰡⰄⰊⰩ ⁙ Ⱄⰵ ⰵⱄⱅⱏ ⰿⱏⱀⱁⰳⱁⱗⰸⱏⰻⰽⰰ ⱁⱅⰲⱃⱐⱄⱅⰰ ⱗⰽⱛⰽⰾⱁⱂⱑⰴⰺⱑ ⱀⰰⱃⰻⱌⰰⰵⰿⰰ Ⰲⰻⰽⰻⱂⱑⰴⰺⱑ · ⱙⰶⰵ ⰽⱏⰶⱐⰴⱁ ⰿⱁⰶⰵⱅⱏ ⰻⰸⰿⱑⱀⱑⱅⰻ ⁙ Ⰲⰻⰽⰻⱂⱑⰴⰺⱑ ⱂⱐⱄⰰⱀⰰ ⱄⰾⱁⰲⱑⱀⱐⱄⰽⱏⰻⰻⰿⱐ ⱗⰸⱏⰻⰽⱁⰿⱐ ⱀⰰⱍⱔⱅⰰ ⰵⱄⱅⱏ ⱓⱀⰹⱑ 2006 ⰾⱑⱅⰰ Ⰴⱐⱀⱐⱄⱐ Ⰲⰻⰽⰻⱂⱑⰴⰺⰻ 509 ⱍⰾⱑⱀⱏ ⱄⱘⱅⱏ\r\n\r\n"
        u"Gothic: 𐍂𐌴𐌹𐌺𐌹 𐍃𐍅𐌴𐌱𐌴 𐌲𐌰𐌻𐌹𐍃𐌾𐍉𐍃 𐍂𐌴𐌹𐌺𐌹 𐍃𐍅𐌴𐌱𐌴 𐌲𐌰𐌻𐌹𐍃𐌾𐍉 𐍅𐌰𐍃 𐌸𐌰𐍄𐌰 𐍆𐍂𐌿𐌼𐌹𐍃𐍄𐍉 𐍂𐌴𐌹𐌺𐌹 𐌲𐌰𐍃𐌺𐌰𐍀𐌾𐌰𐌽𐌰𐍄𐌰 𐌰𐍆 𐌲𐌴𐍂𐌼𐌰𐌽𐌹𐍃𐌺𐌰𐌹 𐌸𐌹𐌿𐌳𐌰𐌹 𐌹𐌽 𐌻𐌰𐌽𐌳𐌰𐌼 𐍂𐌴𐌹𐌺𐌹 𐍂𐍉𐌼𐌰𐌽𐌴. 𐍃𐌰𐍄𐌹𐌸𐌰𐍄𐌰 𐍅𐌰𐍃 𐌹𐌽 410 𐌾𐌰𐌲 𐍆𐍂𐌰𐌵𐌹𐍃𐍄𐌹𐌸𐌰𐍄𐌰 𐍅𐌰𐍃 𐌹𐌽 584 𐌰𐍆 𐍅𐌹𐍃𐌿𐌲𐌿𐍄𐌰𐌼. 𐌼𐌹𐌽𐌽𐌹𐌶𐌰 𐌸𐌰𐌼𐌼𐌰 𐍅𐌹𐍃𐌿𐌲𐌿𐍄𐌰𐌽𐌴 𐌹𐍄𐌰 𐌽𐌹 𐌰𐌹𐍅 𐍅𐌰𐍃 𐌼𐌰𐌷𐍄𐌴𐌹𐌲.\r\n\r\n"
        u"Greek: Η Νέα Σαλαμίνα Αμμοχώστου είναι κυπριακός ποδοσφαιρικός σύλλογος που αποτελεί το σημαντικότερο και μακροβιότερο τμήμα του σωματείου της Νέας Σαλαμίνας Αμμοχώστου.\r\n\r\n"
        u"Gujarati: વ્યાપક અર્થમાં નિશાનીઓ અને નિયમો દ્વારા બનતું એક માળખાને ભાષા કહે છે. ભાષાઓનો ઉપયોગ વિચારોની આપ-લે માટે થાય છે પરંતુ ભાષાઓનો ઉપયોગ ત્યાં સુધી મર્યાદિત નથી.\r\n\r\n"
        u"Gurmukhi: ਪੰਜਾਬ ਉੱਤਰ-ਪੱਛਮੀ ਭਾਰਤ ਦਾ ਇਕ ਰਾਜ ਹੈ, ਜੋ ਵੱਡੇ ਪੰਜਾਬ ਖੇਤ‍ਰ ਦਾ ਇਕ ਭਾਗ ਹੈ। ਇਸਦਾ ਦੂਸਰਾ ਭਾਗ ਪਾਕਿਸਤਾਨ ਵਿੱਚ ਹੈ। ਇਸਦੀ ਸਰਹੱਦ ਉੱਤਰ ਵਿੱਚ ਜੰਮੂ ਅਤੇ ਕਸ਼ਮੀਰ, ਉੱਤਰ-ਪੂਰਬ ਵਿੱਚ ਹਿਮਾਚਲ ਪ੍ਰਦੇਸ਼, ਦੱਖਣ-ਪੂਰਬ ਵਿੱਚ ਹਰਿਆਣੇ, ਦੱਖਣ-ਪੱਛਮ ਵਿੱਚ ਰਾਜਸਥਾਨ ਅਤੇ ਪੱਛਮ ਵਿੱਚ ਪਾਕਿਸਤਾਨੀ ਪੰਜਾਬ ਨਾਲ ਲੱਗਦੀ ਹੈ। ਇਸਦੇ ਮੁੱਖ ਸ਼ਹਿਰ ਅੰਮ੍ਰਿਤਸਰ, ਲੁਧਿਆਣਾ, ਜਲੰਧਰ, ਬਠਿੰਡਾ, ਫ਼ਿਰੋਜ਼ਪੁਰ, ਮੋਹਾਲੀ ਅਤੇ ਪਟਿਆਲਾ ਹਨ ਅਤੇ ਰਾਜਧਾਨੀ ਚੰਡੀਗੜ੍ਹ ਹੈ।\r\n\r\n"
        u"Han Chinese (Simplified): 《天与地》是香港电视广播有限公司的时装电视剧，由林保怡、陈豪、黄德斌、佘诗曼及邵美琪领衔主演，监制为戚其义。\r\n\r\n"
        u"Han Chinese (Traditional): 語言為常人皆有之能。異地之人語言亦異，然人皆須學方能語。語言要而雜，闡義難矣。人多以為，語言為具示法與合理語法而成之溝通與推理系統。多語通觀念、意、思和義以手勢、音、符與文。分斯者時，語言學模糊性其中也。\r\n\r\n"
        u"Hebrew: פורטל גאוגרפיה הוא שער לכל הנושאים שקשורים לגאוגרפיה. ניתן למצוא בו קישורים אל תחומי המשנה של הענף, מושגי יסוד בתחום, תמונות מרהיבות ועוד.\r\n\r\n"
        u"Korean: 쾰른 대성당(독일어: Kölner Dom, 정식명칭: Hohe Domkirche St. Peter und Maria)은 독일 쾰른에 소재한 고딕양식의 대주교좌 성당이다. 이 성당은 독일에서 가장 잘 알려진 건축물이며, 1996년 유네스코 세계 문화유산에 등재되었다.\r\n\r\n"
        u"Japanese: ウィキペディアはオープンコンテントの百科事典です。方針に賛同していただけるなら、誰でも記事を編集したり新しく作成したりできます。ガイドブックを読んでから、サンドボックスで練習してみましょう。質問は利用案内でどうぞ。\r\n\r\n"
        u"Javanese: ꦩꦸꦭ꧀ꦪꦏ꧀ꦏꦏꦺꦈꦩ꧀ꦧꦸꦭ꧀ꦧꦶꦤꦁꦔꦸꦤ꧀ꦠꦩ꧀ꦩꦤ꧀ꦱꦂꦫꦶꦏꦊꦏ꧀ꦱꦤ꧀ꦤꦏ꧀ꦏꦏꦼꦤ꧀ꦏꦤ꧀ꦛꦶꦥꦚꦼꦁꦏꦸꦪꦸꦁꦔꦶꦥꦸꦤ꧀ꦪꦪꦱ꧀ꦱꦤ꧀ꦏꦭ꧀ꦭꦲꦸꦱ꧀ꦠꦼꦒꦸꦭ꧀ꦧꦼꦤ꧀ꦏꦶꦲꦤ꧀ꦱꦲꦏꦂꦉꦱ꧀ꦩꦺꦏ꧀ꦏꦏꦼꦤ꧀ꦢꦶꦤꦶꦁꦲꦶꦁꦏꦁꦩꦶꦤꦸꦭ꧀ꦪꦯꦿꦶꦯꦸꦭ꧀ꦠꦤ꧀ꦲꦩꦁꦏꦸꦨꦸꦮꦤ ꧇꧑꧐꧇ ꦒꦸꦧꦼꦂꦤꦸꦂꦝꦃꦲꦺꦫꦃꦆꦱ꧀ꦠꦶꦩꦺꦮꦃꦪꦺꦴꦒꦾꦏꦂꦠꦠꦸꦮꦶꦤ꧀ꦲꦶꦁꦏꦁꦩꦶꦤꦸꦭ꧀ꦪꦠꦸꦮꦤ꧀ꦪꦺꦴꦱꦺꦧ꧀ꦭꦤ꧀ꦕꦺꦴ ꧈ ꦩꦶꦤꦺꦴꦁꦏꦮꦏꦶꦭ꧀ꦪꦪꦱꦤ꧀ꦏꦭ꧀ꦭꦺꦴꦲꦸꦱ꧀ꦠꦼꦒꦸꦭ꧀ꦧꦺꦤꦏꦶꦲꦤ꧀꧇ (ꦥꦺꦴꦂꦠꦸꦒꦭ꧀) ꦲꦶꦁꦱꦸꦂꦪꦏꦥꦶꦁ ꧇꧒꧒꧇ ꦄꦒꦸꦱ꧀ꦠꦸꦱ꧀ ꧇꧒꧐꧐꧔\r\n\r\n"
        u"Kannada: ಭಾಷೆ ಮಾಹಿತಿಯ ಸಂವಹನೆಗೆ ನಿರೂಪಿತವಾಗಿರುವ ಸಂಕೇತಗಳ ಪದ್ದತಿ. ಈ ಸಂಕೇತಗಳು ಉಚ್ಛರಿತವಾಗಿರಬಹುದು, ಲಿಖಿತವಾಗಿರಬಹುದು ಅಥವ ಅಭಿನಿತವಾಗಿರಬಹುದು. ಭಾಷೆ ಮಾನವನ ಅನುಪಮ ಗುಣಗಳಲ್ಲಿ ಒಂದಾಗಿದೆ. \r\n\r\n"
        u"Khmer: ទោះបីជាកម្ពុជាបានសម្រេចបានឯករាជ្យនៅចុងឆ្នាំ១៩៥៣ក៏ដោយ ក៏ស្ថានភាពកងទ័ពរបស់ខ្លួនមិននឹងនរដែរ ។ ក្រុមបក្សពួកដែលមិនមែនកម្មុយនិស្តនៃពួកខ្មែរឥស្សរៈ​បានចូលរួមក្នុងរដ្ឋាភិបាល ក៏ប៉ុន្តែសកម្មភាពវៀតមិញនិយមកុម្មុយនិស្ត និងសមាគមខ្មែរឥស្សរៈបានកើនឡើងច្រើនដង ដែលកងកម្លាំងរបស់បារាំងបណ្ដេញអោយនៅតិចតួចនៅកន្លែងផ្សេងៗទៀត ។ នៅខែមេសា ឆ្នាំ១៩៥៤ កងវរសេនាតូចវៀតមិញជាច្រើនបានឆ្លងកាត់ព្រំដែនចូលមកកម្ពុជា ។ កងកម្លាំងរាជានិយមបានប្រយុទ្ធនឹងពួកគេ ក៏ប៉ុន្តែមិនអាចបង្ខំអោយមានការដកទ័ពថយទាំងស្រុងរបស់ពួកគេឡើយ ។ ម្យ៉ាង ពួកកុម្មុយនិស្តកំពុងតែប៉ុនប៉ងពង្រឹងតំណែងដែលនឹងតថ្លៃនៅសន្និសីទសឺណែវដែលបានកំណត់កាលវិភាគចាប់ផ្ដើមនៅចុងខែមេសា ។\r\n\r\n"
        u"Lao: ພາສາໃນຄວາມຫມາຍຢ່າງກວ້າງ ​ຫມາຍຖຶງ ກະຣິຍາອາການທີ່ສະແດງອອກມາແລ້ວສາມາດເຮັດຄວາມເຂົ້າໃຈກັນໄດ້ ​ບ່ວ່າຈະແມ່ນ​ຣະຫວ່າງມະນຸດກັບມະນຸດ ມະນຸດກັບສັດ ຫຼືສັດກັບສັດ ສ່ວນພາສາໃນຄວາມຫມາຍຢ່າງແຄບນັ້ນ ຫມານຖຶງ ເສີຍງເວົ້າທີ່ມະນຸດໃຊືສື່ສານກັນເທົ່ານັ້ນ\r\n\r\n"
        u"Latin: Praim Minista blong Tonga, itokaut pinis olsem, King blong Tonga na man husat ibin bringim Demokrasi igo long kantri, King George Tupou the fifth,i dai pinis. King Tupou the fifth krismas blong en 63 ibin dai aste long Hong Kong.Yangpla brata blong en ibin stap wantem em taem emi bin dai. King Tupou ino bin marit na brata blong en, Crown Prince, bai kisim ples blong en na kamap olsem nupla King. Niusmeri long Nukua'lofa, Monalisa Palu, ibin tokim Redio Australia, olsem King Tupou ibin wanpla man husat ibin kamapim ol gutpla senis long kantri.\r\n\r\n"
        u"Vietnamese: Mùi cỏ cháy là một bộ phim điện ảnh Việt Nam thuộc thể loại tâm lý xã hội, chiến tranh. Bối cảnh chính của phim là Mùa hè đỏ lửa 1972 với trận chiến tại Thành cổ Quảng Trị.\r\n\r\n"
        u"Lisu: ꓬꓲ ꓚꓰ ꓬꓲ ꓪꓴ ꓗꓪ ꓪꓴ‐ꓢ ꓡꓰ ꓟꓴ ꓗꓪ_ ꓐꓰ ꓟꓲ ꓠꓯ ꓔꓯ ꓚꓰꓼ_ ꓡꓳ ꓿ * ꓟꓲ ꓠꓯ ꓠꓬ ꓬꓲ ꓑꓰ, ꓟꓽ ꓙꓳ ꓟ ꓬꓲ ꓖꓳꓽ ꓥ ꓢꓲ _, ꓠꓯ ꓞꓲ,  ꓠꓬ ꓠꓯ._ ꓟ ꓬꓲ ꓙꓬ ꓕꓮ ꓢꓲ ꓗꓪ ꓓ_ ꓡꓳ ꓿ ꓪꓴ‐ꓢ ꓦ., ꓠꓬ ꓬꓲ ꓙꓬ ꓕꓮ ꓢꓲ ꓗꓪ ꓮʼ ꓔꓬ_ ꓡꓳ ꓿\r\n\r\n"
        u"Malayalam: അറേബ്യൻ ഉപദ്വീപിലെ ഏറ്റവും വലിയ രാഷ്ട്രമാണ് സൗദി അറേബ്യ. മദ്ധ്യപൗരസ്ത്യദേശത്തെ ഒരു സമ്പന്നരാഷ്ട്രമായ സൗദി അറേബ്യയുടെ തലസ്ഥാനം റിയാദ് ആണ്. സമ്പൂർണരാജഭരണമാണ് ഇവിടത്തെ ഭരണക്രമം. ഭരിക്കുന്ന രാജകുടുംബത്തിന്റെ നാമത്തിലറിയപ്പെടുന്ന അപൂർവ്വരാജ്യങ്ങളിലൊന്നുമാണിത്. അബ്ദുള്ള ബിൻ അബ്ദുൽ അസീസ് രാജാവാണ്‌ സൗദി അറേബ്യയിലെ ഇപ്പോഴത്തെ ഭരണാധികാരി. രണ്ട് വിശുദ്ധ പള്ളികളുടെ നാട് എന്ന പേരിലും സൗദി അറേബ്യ അറിയപ്പെടാറുണ്ട്. ഇസ്ലാമികരാഷ്ട്രമായ സൗദി അറേബ്യയിലെ 99 ശതമാനം ജനങ്ങളും മുസ്ലിങ്ങളാണ്. മുസ്ലിങ്ങളുടെ വിശുദ്ധനഗരങ്ങളായ മക്കയും മദീനയും ഇവിടെ സ്ഥിതിചെയ്യുന്നു. ചുട്ടുപൊള്ളുന്ന വരണ്ട കാലാവസ്ഥയാണ് സൗദി അറേബ്യയുടേത്.\r\n\r\n"
        u"Mongolian: ᠸᠢᠻᠢᠫᠡᠳᠢᠶᠠ ᠴᠢᠯᠦᠺᠡᠲᠦ ᠨᠡᠪᠲᠡᠷᠻᠡᠢ ᠲᠣᠯᠢ ᠪᠢᠴᠢᠺ ᠪᠣᠯᠠᠢ᠃ ᠮᠣᠩᠭᠣᠯ ᠦᠰᠦᠭ ᠦᠨ ᠤᠯᠠᠷᠢᠬᠤ ᠳᠦᠷᠢᠮ ᠢ ᠰᠢᠯᠭᠠᠬᠤ ᠮᠠᠲ᠋ᠧᠷᠢᠶᠠᠯ᠃\r\n\r\n"
        u"Myanmar: လက်ပံတောင်းတောင် သပိတ်အရေးအခင်းသည်၊ လက်ပတောင်း၊ ဆားလင်းကြီးမြို့၊ မုံရွာတဘက်ကမ်းတွင်တည်ရှိသည့် ကြေးနီစီမံကိန်းအား ရပ်ဆိုင်းရေးအတွက် ဒေသခံ ရွာသူရွာသားများမှ ဆန္ဒပြသပိတ်မှောက်ခဲ့ကြခြင်းဖြစ်သည်။ ထိုကြေးနီစီမံကိန်းအား ကန့်ကွက်ရန် စတင်ဆန္ဒပြခဲ့ကြသူများမှာ မုံရွာ ဆုတောင်းပြည့် ဘုရား၌ ဝတ်ပြုဆုတောင်းခဲ့ရာမှ သြဂုတ်လ ၃၀ရက်နေ့တွင် အာဏာပိုင်တို့၏ ဖမ်းဆီးခြင်းကို ခံခဲ့ရသည်။ ၁၄ရက်မျှ အဖမ်းခံရပြီးနောက်ပိုင်း ၎င်းတို့အားလုံး ပြန်လည်လွတ်မြောက်ခဲ့သည်။ ထိုစီမံကိန်းအားရပ်တန့်ရန် လိုလားသူများမှလည်း ရန်ကုန်မြို့တွင် လူတထောင်ခန့် ဆန္ဒပြခဲ့ကြသည်။ ထိုစီမံကိန်းအား ပူးတွဲလုပ်ဆောင်နေသည့် ဦးပိုင်ကုမ္ပဏီ နှင့် ဝမ်ပေါင်ကုမ္ပဏီတို့မှ သဘာဝ ပါတ်ဝန်းကျင် ပျက်စီးမှုများသည့် ၎င်းတို့၏ စီမံကိန်းများကြောင့်မဟုတ်ဟု ငြင်းဆိုခဲ့သည်။\r\n\r\n"
        u"New Tai Lue: ᦊᦄᦾᦰᦡᦳᦰᦂᦄᦠᦰᦉᦊᦄᦾᦰᦡᦳᦰᦂᦄᦠᦰᦉᦡᦕᦂᦄᦠᦰᦂᦕᦡᦉᦄᦠᦰᦕᦂᦂᦊᦄᦾᦰᦡᦳᦰᦂᦄᦠᦰᦉᦡᦕᦂᦄᦠᦰᦂᦕᦡᦉᦄᦠᦰᦕᦂᦡᦄᦂᦊᦄᦾᦰᦡᦳ ᦰᦂᦄᦠᦰᦉᦡᦕᦂᦄᦠᦰᦂᦕᦡᦉᦄᦠᦰᦕᦂᦡᦄ ᦂᦡᦕᦂᦄᦠᦰᦊᦄᦾᦰᦡᦳᦰᦂᦄᦠᦰ ᦉᦊᦄᦾᦰᦡᦳᦰᦂᦄᦠᦰᦉᦡᦕᦂᦄᦠᦰᦂᦕᦡᦉᦄᦠᦰᦕᦂᦡᦄ ᦂᦊᦄᦾᦰᦡᦳᦰᦂ ᦄᦠᦰᦉᦡᦕᦂᦄᦠᦰᦂᦕᦡᦉᦄᦠᦰᦕᦂᦡᦄᦂᦊᦄᦾᦰᦡᦳᦰᦂᦄᦠᦰᦉᦡᦕᦂᦄᦠᦰᦂᦕᦡᦉᦄᦠᦰᦕᦂᦡᦄ ᦂᦡᦕᦂᦄᦠᦰᦂᦕᦡᦉᦄᦠᦰᦕᦂᦡᦄ ᦂᦂᦕᦡᦉᦄᦠᦰᦕᦂᦡᦄ ᦂ\r\n\r\n"
        u"N'ko: ߞߏ ߡߍ߲ ߞߵߊ߬ ߞߍ߫ ߊ߲ ߛߋ߫ ߘߊ߫ ߞߊ߬ ߕߟߋ߬ߓߊ߰ߓߟߐߟߐ ߘߊߦߟߍ߬ ߒߞߏ ߦߋ߫ ߸ ߊ߲ ߧߴߊ߲ ߞߊߘߊ߲߫ ߏ߬ ߘߐ߫ ߞߙߊߕߊߕߊ߫ ߞߊ߬ ߒ߬ ߓߊߘߋ߲ ߕߐ߬ߡߊ ߟߎ߬ ߟߊߞߍ߫ ߊ߬ ߞߊ߬ߟߊߡߊ߬߸\r\n\r\n"
        u"Ogham: ᚋᚐᚊᚔᚇᚓᚉᚉᚓᚇᚇᚐᚄ ᚐᚃᚔ ᚈᚒᚒᚐᚅᚔᚐᚄ\r\n\r\n"
        u"Ol Chiki: ᱥᱚᱸᱴᱚᱞᱤ\r\n\r\n"
        u"Old Italic: 𐌀 𐌁 𐌂 𐌃 𐌄 𐌅 𐌆 𐌇 𐌈 𐌉 𐌊 𐌋 𐌌 𐌍 𐌎 𐌏 𐌐 𐌑 𐌒 𐌓 𐌔 𐌕 𐌖 𐌗 𐌘 𐌙 \r\n\r\n"
        u"Oriya: ନେତାଜୀ ସୁଭାଷ ଚନ୍ଦ୍ର ବୋଷ (୨୩ ଜାନୁଆରୀ ୧୮୯୭–ଜଣାନାହିଁ), ଭାରତର ଜଣେ ଅଗ୍ରଣି ସ୍ଵାଧୀନତା ସଂଗ୍ରାମୀ ଥିଲେ । ସୁଭାଷ ବାପା ଜାନକୀ ନାଥ ବୋଷଙ୍କ ଔରସରୁ ଓ ମାଆ ପ୍ରଭାବତୀ ଦେବୀଙ୍କ ଠାରୁ ୧୮୯୭ ମସିହା ଜାନୁଆରୀ ୨୩ ତାରିଖ ଦିନ କଟକର ଓଡ଼ିଆ ବଜାର ଠାରେ ଜନ୍ମ ନେଇଥିଲେ । ଜାନକୀନାଥ ବୋଷଙ୍କର ପୁତ୍ରଭାବରେ ଜନ୍ମ ଗ୍ରହଣ କରିଥିବା ସୁଭାଷ ଭାରତ ତଥା ସମଗ୍ର ବିଶ୍ଵର ବିସ୍ମୟ ବିଦ୍ରୋହୀ ସଂଗ୍ରାମୀ ନେତା ଭାବରେ ପରିଚିତ ।\r\n\r\n"
        u"Osmanya: 𐒋𐒘𐒈𐒑𐒛𐒒𐒕𐒀\r\n\r\n"
        u"Phags Pa: ꡁꡅꡆꡇꡟꡞ ꡇꡠꡲꡐꡳꡝꡢꡪꡬꡭꡮꡰ꡷꡶꡵ꡳꡝꡡ ꡁ\r\n\r\n"
        u"Runic: ᚱᚢᚾᛁᚲ\r\n\r\n"
        u"Sinhala: භාෂාව යනු සන්නිවේදන ක්‍රමයකි. මානව භාෂණ හා කථන භාෂා සංකේත සහ එම සංකේත හසුරුවන ව්‍යාකරණය ඇතුළත් පද්ධතිය ලෙස විස්තර කළ හැකි ය.\r\n\r\n"
        u"Sora Sompeng: 𑃐𑃦𑃝𑃢\r\n\r\n"
        u"Syriac: ܫܠܡ ܘܠܐܠܗܐ ܏ܫܘܒ܆ ܘܠܕܘܝܐ ܕܣܡ ܗܠܝܢ ܫܘܒܩܢܐ܀\r\n\r\n"
        u"Tai Le: ᥐ́ᥑᥒᥓᥔ ᥕᥖᥗᥘ ᥙᥚᥛᥜ ᥝᥞᥟᥠ ᥡᥢᥣ ᥤ́ᥥ́ᥦᥧᥨ ᥩᥪᥫᥬᥭᥰ ᥱᥲᥳᥴ ᥐᥑᥒᥓᥔ ᥕᥖᥗᥘ ᥙᥚᥛᥜ ᥝᥞᥟᥠ ᥡᥢᥣ ᥤᥥᥦᥧᥨ ᥩᥪᥫᥬᥭᥰ ᥱᥲᥳᥴ ᥐᥑᥒᥓᥔ ᥕᥖᥗᥘ ᥙᥚᥛᥜ ᥝᥞᥟᥠ ᥡᥢᥣ ᥤᥥᥦᥧᥨ ᥩᥪᥫᥬᥭᥰ ᥱᥲᥳᥴ ᥐᥑᥒᥓᥔ ᥕᥖᥗᥘ ᥙᥚᥛᥜ ᥝᥞᥟᥠ ᥡᥢᥣ ᥤᥥᥦᥧᥨ ᥩᥪᥫᥬᥭᥰ ᥱᥲᥳᥴ ᥐᥑᥒᥓᥔ ᥕᥖᥗᥘ ᥙᥚᥛᥜ ᥝᥞᥟᥠ ᥡᥢᥣ ᥤᥥᥦᥧᥨ ᥩᥪᥫᥬᥭᥰ ᥱᥲᥳᥴ ᥐᥑᥒᥓᥔ ᥕᥖᥗᥘ ᥙᥚᥛᥜ ᥝᥞᥟᥠ ᥡᥢᥣ ᥤᥥᥦᥧᥨ ᥩᥪᥫᥬᥭᥰ ᥱᥲᥳᥴ\r\n\r\n"
        u"Tamil: ஒரு மொழி என்பது, தொடர்பாடலுக்குப் பயன்படுகின்ற ஒரு முறைமை ஆகும். இது ஒரு தொகுதிக் குறியீடுகளையும், அவற்றை முறையாகக் கையாளுவதற்கான விதிமுறைகளையும் கொண்டுள்ளது.\r\n\r\n"
        u"Telugu: యోగా (సంస్కృతం: योग) అంటే వ్యాయామ సాధనల సమాహారాల ఆధ్యాత్మిక రూపం. ఇది హిందూత్వ అధ్యాత్మిక సాధనలలో ఒక భాగం. మోక్షసాధనలో భాగమైన ధ్యానం అంతఃదృష్టి, పరమానంద ప్రాప్తి లాంటి అధ్యాత్మిక పరమైన సాధనలకు పునాది. దీనిని సాధన చేసే వాళ్ళను యోగులు అంటారు. వీరు సాధారణ సంఘ జీవితానికి దూరంగా మునులు సన్యాసులవలె అడవులలో ఆశ్రమ జీవితం గడుపుతూ సాధన శిక్షణ లాంటివి నిర్వహిస్తుంటారు. ధ్యానయోగం ఆధ్యాత్మిక సాధనకు మానసిక ఆరోగ్యానికి చక్కగా తోడ్పడుతుంది. \r\n\r\n"
        u"Thaana: ދިވެހިބަހަކީ ދިވެހިރާއްޖޭގެ ރަސްމީ ބަހެވެ. މި ބަހުން ވާހަކަ ދައްކައި އުޅެނީ ދިވެހިރާއްޖޭގެ އަހުލުވެރިންގެ އިތުރުން ހިންދުސްތާނުގެ މަލިކު ގެ އަހުލުވެރިންނެވެ. އެބައިމީހުން ވާހަކަ ދައްކައި\r\n\r\n"
        u"Thai: สมเด็จพระราชินีนาถเบียทริกซ์แห่งเนเธอร์แลนด์ (Beatrix of the Netherlands; พระนามแบบเต็ม เบียทริกซ์ วิลเฮลมินา อาร์มการ์ด เจ้าหญิงแห่งเนเธอร์แลนด์ เจ้าหญิงแห่งออเรนจ์-นัสเซา เจ้าหญิงแห่งลิปเปอ-บีสเตอร์เฟลด์; พระราชสมภพ 31 มกราคม พ.ศ. 2481) เสวยราชสมบัติเป็นสมเด็จพระราชินีนาถแห่งราชอาณาจักรเนเธอร์แลนด์ตั้งแต่วันที่ 30 เมษายน พ.ศ. 2523\r\n\r\n"
        u"Tibetan: ཝེ་ཁེ་རིག་མཛོད་ནི་རང་དབང་གི་རིག་གནས་ཀུན་བཏུས་དཔེ་མཛོད་ཅིག་ཡིན་ལ་འདི་རུ་མི་སུས་ཀྱང་རྩོམ་སྒྲིག་ཆོག གལ་ཏེ་ཁྱེད་ཀྱིས་བོད་ཡིག་མཁྱེན་ན་ལམ་སེང་རྩོམ་ཡིག་སྤེལ་ཆོག་མ་ཟད་ནང་དུ་ཉར་ཟིན་པའི་རྩོམ་ཡིག་ཐམས་ཅད་བོད་ཡིག་གཉེར་མཁན་ཚང་མར་ཕན་པས་ཝེ་ཁེ་བོད་འགྱུར་མའི་ནང་དུ་ཧུར་བརྩོན་གྱིས་རྩོམ་ཡིག་ཁ་སྣོན་མཛོད་དང་ལས་འདིས་བོད་ཡིག་མི་ཉམས་གོང་འཕེལ་དུ་གཏོང་བར་ཧ་ཅང་དགེ་མཚན་མཆིས་སོ། །\r\n\r\n"
        u"Tifinagh: ⵙ⵿ⵜⵤⵔⵜ ⵎⵛⵏⴰ ⵈⵍⴾⴹ ⵌⵏⵓⵏ ⴹ ⵎⴹⵍ⵰ ⵎⴹⵍ ⵓⵔ ⴶⴰ ⵜⵎⵓⵜ ⵓⵍⵢⵜ ⵤⴰ ⵓⵔ ⵜⵂⴰ ⵓⵍⴰ⵰ ⵍⵙⵏ⵿ⵜ ⵎⵏ ⴾⵏⵏⵏ ⴶⵜ ⵓⵔ ⵜⵍⴰ ⵔ ⵛⵢⵢ ⵓⵔⵏⵏ ⴼⵍⴰ ⵏ ⵎⵏ ⵓⵏ ⵎⵔⵏ ⵔⵈ ⵏ ⵎⵛⵏⴰ ⵏ⵿ⵜⴰ ⵍⵢ ⴼⵍ ⵎⵏ ⵓⵏ⵰ ⵏⴰ ⵎⵛⵏⴰ «ⵎⵍⵜ ⵏⵔ⵰» ⵜⵤⵔ ⵎⵝ ⵏⵔ⵰ ⵏⵔ ⵓⵏ ⵎⵙ ⵔⵜ ⵍⵗⵏ ⵗⵔ ⵎⵛⵏⴰ ⵜⵤⵔ ⵤⵎⵤⵢⵜ ⴹ ⵛⵢⵢ⵰ ⴶⴰ ⵎⵛⵏⴰ ⵢ ⵏⵔ ⵙⵎ ⵤⵍ ⵎⵔⵏ ⵛⵢⵢ ⴶⵙⵏⵜ ⵙⵎ ⵂⴹ ⴶ ⵂⴹ ⵜⴶⴰ ⵜⴼⵓⵜ ⵆⵍ ⵓⵏ ⵤⵍ ⵓ ⵤⵔⵏ⵰\r\n\r\n"
        u"Vai: ꕉꕜꕮ ꔔꘋ ꖸ ꔰ ꗋꘋ ꕮꕨ ꔔꘋ ꖸ ꕎ ꕉꖸꕊ ꕴꖃ ꕃꔤꘂ ꗱ,\r\n\r\n"
        u"Yi: ꆈꌠꁱꂷ"
    },
#endif
#if 0
    {0, u"Alphabets",
        u"Arabic              'ب' U+0628 ARABIC LETTER BEH\r\n"
        u"Armenian            'Ա' U+0531 ARMENIAN CAPITAL LETTER AYB\r\n"
        u"Balinese            'ᬓ' U+1B13 BALINESE LETTER KA\r\n"
        u"Bengali             'ক' U+0995 BENGALI LETTER KA\r\n"
        u"Bopomofo            'ㄅ' U+3105 BOPOMOFO LETTER B\r\n"
        u"Braille             '⠝' U+281D BRAILLE PATTERN DOTS-1345\r\n"
        u"Buginese            'ᨀ' U+1A00 BUGINESE LETTER KA\r\n"
        u"Buhid               'ᝃ' U+1743 BUHID LETTER KA\r\n"
        u"Canadian_Aboriginal 'ᓀ' U+14C0 CANADIAN SYLLABICS NE\r\n"
        u"Carian              '𐊠' U+102A0 CARIAN LETTER A\r\n"
        u"Cham                'ꨆ' U+AA06 CHAM LETTER KA\r\n"
        u"Cherokee            'Ꮳ' U+13E3 CHEROKEE LETTER TSA\r\n"
        u"Common              '“' U+201C LEFT DOUBLE QUOTATION MARK\r\n"
        u"Coptic              'Ⲁ' U+2C80 COPTIC CAPITAL LETTER ALFA\r\n"
        u"Cuneiform           '𒀀' U+12000 CUNEIFORM SIGN A\r\n"
        u"Cypriot             '𐠀' U+10800 CYPRIOT SYLLABLE A\r\n"
        u"Cyrillic            'Я' U+042F CYRILLIC CAPITAL LETTER YA\r\n"
        u"Deseret             '𐐔' U+10414 DESERET CAPITAL LETTER DEE\r\n"
        u"Devanagari          'क' U+0915 DEVANAGARI LETTER KA\r\n"
        u"Ethiopic            'አ' U+12A0 ETHIOPIC SYLLABLE GLOTTAL A\r\n"
        u"Georgian            'დ' U+10D3 GEORGIAN LETTER DON\r\n"
        u"Glagolitic          'Ⰳ' U+2C03 GLAGOLITIC CAPITAL LETTER GLAGOLI\r\n"
        u"Gothic              '𐌰' U+10330 GOTHIC LETTER AHSA\r\n"
        u"Greek               'Ω' U+03A9 GREEK CAPITAL LETTER OMEGA\r\n"
        u"Gujarati            'ક' U+0A95 GUJARATI LETTER KA\r\n"
        u"Gurmukhi            'ਕ' U+0A15 GURMUKHI LETTER KA\r\n"
        u"Han                 '字' U+5B57 CJK UNIFIED IDEOGRAPH-5B57 : zì\r\n"
        u"Hangul              'ᄒ' U+1112 HANGUL CHOSEONG HIEUH\r\n"
        u"Hanunoo             'ᜣ' U+1723 HANUNOO LETTER KA\r\n"
        u"Hebrew              'א' U+05D0 HEBREW LETTER ALEF\r\n"
        u"Hiragana            'か' U+304B HIRAGANA LETTER KA\r\n"
        u"Inherited           '̌' U+030C COMBINING CARON\r\n"
        u"Kannada             'ಕ' U+0C95 KANNADA LETTER KA\r\n"
        u"Katakana            'カ' U+30AB KATAKANA LETTER KA\r\n"
        u"Kayah_Li            'ꤊ' U+A90A KAYAH LI LETTER KA\r\n"
        u"Kharoshthi          '𐨐' U+10A10 KHAROSHTHI LETTER KA\r\n"
        u"Khmer               'ក' U+1780 KHMER LETTER KA\r\n"
        u"Lao                 'ລ' U+0EA5 LAO LETTER LO LOOT = LAO LETTER LO\r\n"
        u"Latin               'L' U+004C LATIN CAPITAL LETTER L\r\n"
        u"Lepcha              'ᰀ' U+1C00 LEPCHA LETTER KA\r\n"
        u"Limbu               'ᤁ' U+1901 LIMBU LETTER KA\r\n"
        u"Linear_B            '𐀀' U+10000 LINEAR B SYLLABLE B008 A\r\n"
        u"Lycian              '𐊀' U+10280 LYCIAN LETTER A\r\n"
        u"Lydian              '𐤠' U+10920 LYDIAN LETTER A\r\n"
        u"Malayalam           'ക' U+0D15 MALAYALAM LETTER KA\r\n"
        u"Mongolian           'ᠦ' U+1826 MONGOLIAN LETTER UE\r\n"
        u"Myanmar             'က' U+1000 MYANMAR LETTER KA\r\n"
        u"New_Tai_Lue         'ᦂ' U+1982 NEW TAI LUE LETTER HIGH KA\r\n"
        u"Nko                 'ߒ' U+07CD NKO LETTER E\r\n"
        u"Ogham               'ᚏ' U+168F OGHAM LETTER RUIS\r\n"
        u"Ol_Chiki            'ᱚ' U+1C5A OL CHIKI LETTER LA\r\n"
        u"Old_Italic          '𐌀' U+10300 OLD ITALIC LETTER A\r\n"
        u"Old_Persian         '𐎠' U+103A0 OLD PERSIAN SIGN A\r\n"
        u"Oriya               'କ' U+0B15 ORIYA LETTER KA\r\n"
        u"Osmanya             '𐒀' U+10480 OSMANYA LETTER ALEF\r\n"
        u"Phags_Pa            'ꡀ' U+A840 PHAGS-PA LETTER KA\r\n"
        u"Phoenician          '𐤀' U+10900 PHOENICIAN LETTER ALF\r\n"
        u"Rejang              'ꤰ' U+A930 REJANG LETTER KA\r\n"
        u"Runic               'ᚠ' U+16A0 RUNIC LETTER FEHU FEOH FE F\r\n"
        u"Saurashtra          'ꢂ' U+A882 SAURASHTRA LETTER A\r\n"
        u"Shavian             '𐑗' U+10457 SHAVIAN LETTER CHURCH\r\n"
        u"Sinhala             'අ' U+0D85 SINHALA LETTER AYANNA\r\n"
        u"Sundanese           'ᮃ' U+1B83 SUNDANESE LETTER A\r\n"
        u"Syloti_Nagri        'ꠀ' U+A800 SYLOTI NAGRI LETTER A\r\n"
        u"Syriac              'ܐ' U+0710 SYRIAC LETTER ALAPH\r\n"
        u"Tagalog             'ᜃ' U+1703 TAGALOG LETTER KA\r\n"
        u"Tagbanwa            'ᝣ' U+1763 TAGBANWA LETTER KA\r\n"
        u"Tai_Le              'ᥐ' U+1950 TAI LE LETTER KA\r\n"
        u"Tamil               'க' U+0B95 TAMIL LETTER KA\r\n"
        u"Telugu              'క' U+0C15 TELUGU LETTER KA\r\n"
        u"Thaana              'ތ' U+078C THAANA LETTER THAA\r\n"
        u"Thai                'ท' U+0E17 THAI CHARACTER THO THAHAN\r\n"
        u"Tibetan             'ཀ' U+0F40 TIBETAN LETTER KA\r\n"
        u"Tifinagh            'ⴲ' U+2D32 TIFINAGH LETTER YABH\r\n"
        u"Ugaritic            '𐎀' U+10380 UGARITIC LETTER ALPA\r\n"
        u"Vai                 'ꕉ' U+A549 VAI SYLLABLE A\r\n"
        u"Yi                  'ꊈ' U+A288 YI SYLLABLE WO\r\n"
        u"Zzzz                '\xFFF8' U+FFF8 <not a character>"
    },
#endif
};

const Attribute::PredefinedValue DrawableObject::glyphDefaults[] = {
    {0, u"Use text cmap instead", u""},
    {0, u"Empty glyph run", u" "},
    {0, u"0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31" },
};

const Attribute::PredefinedValue DrawableObject::readingDirections[] = {
    //  ---/    \---    \-->    <--/   |   /|  |\  /\  |\   |  /\  /|
    //    /      \       \        /    |  / |  | \  |  | \  |  |  / |
    //   /        \       \      /     | /  |  |  \ |  |  \ |  | /  |
    //  /-->    <--\    ---\    /---   |/  \/  |   \|  \/  \|  |/   |
    {0, u"LTR TTB" }, // →↓
    {1, u"RTL TTB" }, // ←↓
    {2, u"LTR BTT" }, // →↑
    {3, u"RTL BTT" }, // ←↑
    {4, u"TTB LTR" }, // ↓→
    {5, u"BTT LTR" }, // ↑→
    {6, u"TTB RTL" }, // ↓←
    {7, u"BTT RTL" }, // ↑←
};

const Attribute::PredefinedValue DrawableObject::fontSizes[] = {
    {  1},
    {  2, nullptr, u"1.5"},
    {  2},
    {  3},
    {  4},
    {  5, nullptr, u"4.5"},
    {  5},
    {  6},
    {  7},
    {  8},
    {  9},
    { 10},
    { 11},
    { 12},
    { 13},
    { 14},
    { 15},
    { 16},
    { 17},
    { 18},
    { 19},
    { 20},
    { 22},
    { 24},
    { 26},
    { 28},
    { 30},
    { 32},
    { 36},
    { 40},
    { 44},
    { 48},
    { 52},
    { 58},
    { 64},
    { 72},
    { 80},
    { 88},
    { 96},
    {104},
    {112},
    {120},
    {128},
    {144},
    {160},
    {192},
    {256},
};

const Attribute::PredefinedValue DrawableObject::layoutSizes[] = {
    {   1},
    {  32},
    {  40},
    {  64},
    { 100},
    { 128},
    { 200},
    { 256},
    { 300},
    { 480},
    { 512},
    { 768},
    { 1024},
};

const Attribute::PredefinedValue DrawableObject::textColors[] = {
    {0x00000000, u"Transparent"},
    {0xFFF0F8FF, u"AliceBlue"},
    {0xFFFAEBD7, u"AntiqueWhite" },
    {0xFF00FFFF, u"Aqua" },
    {0xFF7FFFD4, u"Aquamarine" },
    {0xFFF0FFFF, u"Azure" },
    {0xFFF5F5DC, u"Beige" },
    {0xFFFFE4C4, u"Bisque" },
    {0xFF000000, u"Black" },
    {0xFFFFEBCD, u"BlanchedAlmond" },
    {0xFF0000FF, u"Blue" },
    {0xFF8A2BE2, u"BlueViolet" },
    {0xFFA52A2A, u"Brown" },
    {0xFFDEB887, u"BurlyWood" },
    {0xFF5F9EA0, u"CadetBlue" },
    {0xFF7FFF00, u"Chartreuse" },
    {0xFFD2691E, u"Chocolate" },
    {0xFFFF7F50, u"Coral" },
    {0xFF6495ED, u"CornflowerBlue" },
    {0xFFFFF8DC, u"Cornsilk" },
    {0xFFDC143C, u"Crimson" },
    {0xFF00FFFF, u"Cyan" },
    {0xFF00008B, u"DarkBlue" },
    {0xFF008B8B, u"DarkCyan" },
    {0xFFB8860B, u"DarkGoldenRod" },
    {0xFFA9A9A9, u"DarkGray" },
    {0xFF006400, u"DarkGreen" },
    {0xFFBDB76B, u"DarkKhaki" },
    {0xFF8B008B, u"DarkMagenta" },
    {0xFF556B2F, u"DarkOliveGreen" },
    {0xFFFF8C00, u"DarkOrange" },
    {0xFF9932CC, u"DarkOrchid" },
    {0xFF8B0000, u"DarkRed" },
    {0xFFE9967A, u"DarkSalmon" },
    {0xFF8FBC8F, u"DarkSeaGreen" },
    {0xFF483D8B, u"DarkSlateBlue" },
    {0xFF2F4F4F, u"DarkSlateGray" },
    {0xFF00CED1, u"DarkTurquoise" },
    {0xFF9400D3, u"DarkViolet" },
    {0xFFFF1493, u"DeepPink" },
    {0xFF00BFFF, u"DeepSkyBlue" },
    {0xFF696969, u"DimGray" },
    {0xFF1E90FF, u"DodgerBlue" },
    {0xFFB22222, u"FireBrick" },
    {0xFFFFFAF0, u"FloralWhite" },
    {0xFF228B22, u"ForestGreen" },
    {0xFFFF00FF, u"Fuchsia" },
    {0xFFDCDCDC, u"Gainsboro" },
    {0xFFF8F8FF, u"GhostWhite" },
    {0xFFFFD700, u"Gold" },
    {0xFFDAA520, u"GoldenRod" },
    {0xFF808080, u"Gray" },
    {0xFF00FF00, u"Green" },
    {0xFFADFF2F, u"GreenYellow" },
    {0xFFF0FFF0, u"HoneyDew" },
    {0xFFFF69B4, u"HotPink" },
    {0xFFCD5C5C, u"IndianRed" },
    {0xFF4B0082, u"Indigo" },
    {0xFFFFFFF0, u"Ivory" },
    {0xFFF0E68C, u"Khaki" },
    {0xFFE6E6FA, u"Lavender" },
    {0xFFFFF0F5, u"LavenderBlush" },
    {0xFF7CFC00, u"LawnGreen" },
    {0xFFFFFACD, u"LemonChiffon" },
    {0xFFADD8E6, u"LightBlue" },
    {0xFFF08080, u"LightCoral" },
    {0xFFE0FFFF, u"LightCyan" },
    {0xFFFAFAD2, u"LightGoldenRodYellow" },
    {0xFFD3D3D3, u"LightGray" },
    {0xFF90EE90, u"LightGreen" },
    {0xFFFFB6C1, u"LightPink" },
    {0xFFFFA07A, u"LightSalmon" },
    {0xFF20B2AA, u"LightSeaGreen" },
    {0xFF87CEFA, u"LightSkyBlue" },
    {0xFF778899, u"LightSlateGray" },
    {0xFFB0C4DE, u"LightSteelBlue" },
    {0xFFFFFFE0, u"LightYellow" },
    {0xFFBFFF00, u"Lime" },
    {0xFF32CD32, u"LimeGreen" },
    {0xFFFAF0E6, u"Linen" },
    {0xFFFF00FF, u"Magenta" },
    {0xFF800000, u"Maroon" },
    {0xFF66CDAA, u"MediumAquaMarine" },
    {0xFF0000CD, u"MediumBlue" },
    {0xFFBA55D3, u"MediumOrchid" },
    {0xFF9370DB, u"MediumPurple" },
    {0xFF3CB371, u"MediumSeaGreen" },
    {0xFF7B68EE, u"MediumSlateBlue" },
    {0xFF00FA9A, u"MediumSpringGreen" },
    {0xFF48D1CC, u"MediumTurquoise" },
    {0xFFC71585, u"MediumVioletRed" },
    {0xFF191970, u"MidnightBlue" },
    {0xFFF5FFFA, u"MintCream" },
    {0xFFFFE4E1, u"MistyRose" },
    {0xFFFFE4B5, u"Moccasin" },
    {0xFFFFDEAD, u"NavajoWhite" },
    {0xFF000080, u"Navy" },
    {0xFF000080, u"NavyBlue" },
    {0xFFFDF5E6, u"OldLace" },
    {0xFF808000, u"Olive" },
    {0xFF6B8E23, u"OliveDrab" },
    {0xFFFFA500, u"Orange" },
    {0xFFFF4500, u"OrangeRed" },
    {0xFFDA70D6, u"Orchid" },
    {0xFFEEE8AA, u"PaleGoldenRod" },
    {0xFF98FB98, u"PaleGreen" },
    {0xFFAFEEEE, u"PaleTurquoise" },
    {0xFFDB7093, u"PaleVioletRed" },
    {0xFFFFEFD5, u"PapayaWhip" },
    {0xFFFFDAB9, u"PeachPuff" },
    {0xFFCD853F, u"Peru" },
    {0xFFFFC0CB, u"Pink" },
    {0xFFDDA0DD, u"Plum" },
    {0xFFB0E0E6, u"PowderBlue" },
    {0xFF800080, u"Purple" },
    {0xFF663399, u"RebeccaPurple" },
    {0xFFFF0000, u"Red" },
    {0xFFBC8F8F, u"RosyBrown" },
    {0xFF4169E1, u"RoyalBlue" },
    {0xFF8B4513, u"SaddleBrown" },
    {0xFFFA8072, u"Salmon" },
    {0xFFF4A460, u"SandyBrown" },
    {0xFF2E8B57, u"SeaGreen" },
    {0xFFFFF5EE, u"SeaShell" },
    {0xFFA0522D, u"Sienna" },
    {0xFFC0C0C0, u"Silver" },
    {0xFF87CEEB, u"SkyBlue" },
    {0xFF6A5ACD, u"SlateBlue" },
    {0xFF708090, u"SlateGray" },
    {0xFFFFFAFA, u"Snow" },
    {0xFF00FF7F, u"SpringGreen" },
    {0xFF4682B4, u"SteelBlue" },
    {0xFFD2B48C, u"Tan" },
    {0xFF008080, u"Teal" },
    {0xFFD8BFD8, u"Thistle" },
    {0xFFFF6347, u"Tomato" },
    {0x80FF0000, u"TranslucentRed" },
    {0x80FFFF00, u"TranslucentYellow" },
    {0x8000FF00, u"TranslucentGreen" },
    {0x8000FFFF, u"TranslucentCyan" },
    {0x800000FF, u"TranslucentBlue" },
    {0x80FF00FF, u"TranslucentMagenta" },
    {0x80FFFFFF, u"TranslucentWhite" },
    {0x80808080, u"TranslucentGray" },
    {0x80000000, u"TranslucentBlack" },
    {0xFF40E0D0, u"Turquoise" },
    {0xFFEE82EE, u"Violet" },
    {0xFFF5DEB3, u"Wheat" },
    {0xFFFFFFFF, u"White" },
    {0xFFF5F5F5, u"WhiteSmoke" },
    {0xFFFFFF00, u"Yellow" },
    {0xFF9ACD32, u"YellowGreen" },
};

const Attribute::PredefinedValue DrawableObject::colorPaletteIndices[] = {
    {0},
    {1},
};

const Attribute::PredefinedValue DrawableObject::weights[] = {
    {100, u"Thin" },
    {200, u"Extra Light" },
    {300, u"Light" },
    {350, u"Semilight" },
    {400, u"Normal" },
    {500, u"Medium" },
    {600, u"Demi Bold" },
    {700, u"Bold" },
    {800, u"Extra Bold" },
    {900, u"Black" },
    {950, u"Extra Black" },
};

const Attribute::PredefinedValue DrawableObject::stretches[] = {
    {1, u"Ultra Condensed" },
    {2, u"Extra Condensed" },
    {3, u"Condensed" },
    {4, u"Semi Condensed" },
    {5, u"Normal" },
    {6, u"Semi Expanded" },
    {7, u"Expanded" },
    {8, u"Extra Expanded" },
    {9, u"Ultra Expanded" },
};

const Attribute::PredefinedValue DrawableObject::slopes[] = {
    {0, u"Regular" },
    {1, u"Oblique" },
    {2, u"Italic" },
};

const Attribute::PredefinedValue DrawableObject::alignments[] = {
    {0, u"leading" },
    {2, u"center" },
    {1, u"trailing" },
};

const Attribute::PredefinedValue DrawableObject::justifications[] = {
    { 0, u"unjustified" },
    { 1, u"justified" },
};

const Attribute::PredefinedValue DrawableObject::wrappingModes[] = {
    // { uint32_t(DWRITE_WORD_WRAPPING_WRAP), u"Wrap" }, // Deprecated
    {uint32_t(LineWrappingModeNone), u"No wrap" },
    {uint32_t(LineWrappingModeWordCharacter), u"Word with break" },
    {uint32_t(LineWrappingModeWord), u"Word" },
    {uint32_t(LineWrappingModeCharacter), u"Character" },
};

const Attribute::PredefinedValue DrawableObject::typographicFeatures[] = {
    {0, u"Defaults", u"" },
    {0, u"Standard", u"rlig rclt locl ccmp calt liga clig kern mark mkmk dist" },
    {0, u"Required", u"rlig rclt ccmp mark mkmk dist" },
    {0, u"Vertical", u"vert vkrn vpal" },
    {0, u"Stylistic variant", u"ss07" },
};

const Attribute::PredefinedValue DrawableObject::languages[] = {
    {0, u"Invariant", u"" },
    {0, u"English US", u"en-US" },
    {0, u"English Britain", u"en-GB" },
    {0, u"Romanian", u"ro-RO" },
    {0, u"Русский язык Russian", u"ru-RU" },
    {0, u"हिन्दी Hindi India", u"hi-IN" },
    {0, u"日本語 Japanese", u"ja-JP" },
    {0, u"中文 Chinese simplified", u"zh-Hans" },
    {0, u"中文 Chinese traditional", u"zh-Hant" },
    {0, u"中文 Chinese Taiwan", u"zh-TW" },
    {0, u"한글 Hangul Korean", u"ko-KR" },
    {0, u"עִבְרִית Hebrew Israel", u"he-IL" },
    {0, u"الْعَرَبيّة Arabic Egypt", u"ar-EG" },
    {0, u"الْعَرَبيّة Arabic Iraq", u"ar-IQ" },
};

const Attribute::PredefinedValue DrawableObject::fontSimulations[] = {
    {uint32_t(DWRITE_FONT_SIMULATIONS_NONE), u"none" },
    {uint32_t(DWRITE_FONT_SIMULATIONS_BOLD), u"bold" },
    {uint32_t(DWRITE_FONT_SIMULATIONS_OBLIQUE), u"oblique" },
    {uint32_t(DWRITE_FONT_SIMULATIONS_BOLD | DWRITE_FONT_SIMULATIONS_OBLIQUE), u"bold oblique" },
};

const Attribute::PredefinedValue DrawableObject::dwriteMeasuringModes[] = {
    {uint32_t(DWRITE_MEASURING_MODE_NATURAL), u"Natural" },
    {uint32_t(DWRITE_MEASURING_MODE_GDI_CLASSIC), u"GDI compatible" },
    {uint32_t(DWRITE_MEASURING_MODE_GDI_NATURAL), u"GDI compatible natural" },
};

const Attribute::PredefinedValue DrawableObject::dwriteRenderingModes[] = {
    {uint32_t(DWRITE_RENDERING_MODE_DEFAULT), u"Default" },
    {uint32_t(DWRITE_RENDERING_MODE_ALIASED), u"Aliased" },
    {uint32_t(DWRITE_RENDERING_MODE_GDI_CLASSIC), u"GDI Classic" },
    {uint32_t(DWRITE_RENDERING_MODE_GDI_NATURAL), u"GDI Natural" },
    {uint32_t(DWRITE_RENDERING_MODE_NATURAL), u"Natural" },
    {uint32_t(DWRITE_RENDERING_MODE_NATURAL_SYMMETRIC), u"Natural Symmetric" },
    {uint32_t(DWRITE_RENDERING_MODE_OUTLINE), u"Outline" },
};

const Attribute::PredefinedValue DrawableObject::gdiRenderingModes[] = {
    {uint32_t(DEFAULT_QUALITY), u"Default" },
    {uint32_t(DRAFT_QUALITY), u"Draft" },
    {uint32_t(PROOF_QUALITY), u"Proof" },
    {uint32_t(NONANTIALIASED_QUALITY), u"Nonantialiased" },
    {uint32_t(ANTIALIASED_QUALITY), u"Antialiased" },
    {uint32_t(CLEARTYPE_QUALITY), u"ClearType" },
    {uint32_t(CLEARTYPE_NATURAL_QUALITY), u"ClearType Natural" },
};

const Attribute::PredefinedValue DrawableObject::dwriteVerticalGlyphOrientation[] = {
    { uint32_t(DWRITE_VERTICAL_GLYPH_ORIENTATION_DEFAULT), u"Default" },
    { uint32_t(DWRITE_VERTICAL_GLYPH_ORIENTATION_STACKED), u"Stacked" },
};

const Attribute::PredefinedValue DrawableObject::gdiPlusRenderingModes[] = {
    {uint32_t(Gdiplus::TextRenderingHintSystemDefault           ), u"Default" },
    {uint32_t(Gdiplus::TextRenderingHintSingleBitPerPixelGridFit), u"SingleBitPerPixelGridFit" },
    {uint32_t(Gdiplus::TextRenderingHintSingleBitPerPixel       ), u"SingleBitPerPixel" },
    {uint32_t(Gdiplus::TextRenderingHintAntiAliasGridFit        ), u"AntiAliasGridFit" },
    {uint32_t(Gdiplus::TextRenderingHintAntiAlias               ), u"AntiAlias" },
    {uint32_t(Gdiplus::TextRenderingHintClearTypeGridFit        ), u"ClearTypeGridFit" },
};

const Attribute::PredefinedValue DrawableObject::dwriteFontFaceTypes[] = {
    {uint32_t(DWRITE_FONT_FACE_TYPE_UNKNOWN), u"Unknown" },
    {uint32_t(DWRITE_FONT_FACE_TYPE_CFF), u"CFF" },
    {uint32_t(DWRITE_FONT_FACE_TYPE_TRUETYPE), u"TrueType" },
    {uint32_t(DWRITE_FONT_FACE_TYPE_TRUETYPE_COLLECTION), u"TrueType collection" },
    {uint32_t(DWRITE_FONT_FACE_TYPE_TYPE1), u"Postscript Type1" },
    {uint32_t(DWRITE_FONT_FACE_TYPE_VECTOR), u"Vector" },
    {uint32_t(DWRITE_FONT_FACE_TYPE_BITMAP), u"Bitmap" },
    {uint32_t(DWRITE_FONT_FACE_TYPE_RAW_CFF), u"Raw CFF" },
};

const Attribute::PredefinedValue DrawableObject::transforms[] = {
    {0, u"None", u"" },
    {0, u"Identity", u"1 0 0 1" },
    {0, u"Rotate 90", u"0 1 -1 0" },
    {0, u"Rotate 180", u"-1 0 0 -1" },
    {0, u"Rotate 270", u"0 -1 1 0" },
    {0, u"Flip horizontal", u"-1 0 0 1" },
    {0, u"Flip vertical", u"1 0 0 -1" },
    {0, u"Flip positive diagonal", u"0 1 1 0" },
    {0, u"Flip negative diagonal", u"0 -1 -1 0" },
    {0, u"Rotate 45", u".707 .707 -.707 .707" },
    {0, u"Rotate 135", u"-.707 .707 -.707 -.707" },
    {0, u"Rotate 225", u"-.707 -.707 .707 -.707" },
    {0, u"Rotate 315", u".707 -.707 .707 .707" },
    {0, u"Translate right", u"1 0 0 1 50 0" },
    {0, u"Translate left", u"1 0 0 1 -50 0" },
    {0, u"Translate down", u"1 0 0 1 0 50" },
    {0, u"Translate up", u"1 0 0 1 0 -50" },
    {0, u"Scale twice", u"2 0 0 2" },
};

const Attribute::PredefinedValue DrawableObject::pixelZooms[] = {
    {0, u"None", u"" },
    {2, u"2" },
    {4, u"4" },
    {8, u"8" },
    {16, u"16" },
};

const Attribute::PredefinedValue DrawableObject::hotkeyDisplays[] = {
    {DrawableObjectHotkeyModeNone, u"None"},
    {DrawableObjectHotkeyModeShow, u"Show"},
    {DrawableObjectHotkeyModeHide, u"Hide"},
};

const Attribute::PredefinedValue DrawableObject::trimmingGranularities[] = {
    {DrawableObjectTrimmingGranularityNone, u"None"},
    {DrawableObjectTrimmingGranularityCharacter, u"Character"},
    {DrawableObjectTrimmingGranularityWord, u"Word"},
    {DrawableObjectTrimmingGranularityPartialGlyph, u"Partial glyph"},
};

const Attribute::PredefinedValue DrawableObject::trimmingDelimiters[] = {
    {0, u"None", u""},
    {0, u"Backslash", u"\\"},
};


bool DrawableObject::IsGdiOrGdiPlusFunction(DrawableObjectFunction functionType) throw()
{
    // Check whether it's a GDI/GDI+ or DWrite based function.
    static_assert(DrawableObjectFunctionTotal == 12, "Update this switch statement.");
    switch (functionType)
    {
    case DrawableObjectFunctionGdiTextOut:
    case DrawableObjectFunctionUser32DrawText:
    case DrawableObjectFunctionGdiPlusDrawString:
    case DrawableObjectFunctionGdiPlusDrawDriverString:
        return true;
    }
    return false;
}


HRESULT DrawableObject::Draw(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    float x,
    float y,
    DX_MATRIX_3X2F const& transform
    )
{
    return S_OK;
}


HRESULT DrawableObject::GetBounds(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas, 
    _Out_ D2D_RECT_F& layoutBounds,
    _Out_ D2D_RECT_F& contentBounds
    )
{
    layoutBounds = emptyRect;
    contentBounds = emptyRect;
    return S_OK;
}


HRESULT DrawableObject::Update(IAttributeSource& attributeSource)
{
    return S_OK;
}


DrawableObject* DrawableObject::Create(DrawableObjectFunction functionType)
{
    switch (functionType)
    {
    default:
    case DrawableObjectFunctionNop: return new DrawableObject();
    case DrawableObjectFunctionDWriteBitmapRenderTargetLayoutDraw: return new DrawableObjectDWriteBitmapRenderTargetLayoutDraw();
    case DrawableObjectFunctionDWriteBitmapRenderTargetDrawGlyphRun: return new DrawableObjectDWriteBitmapRenderTargetDrawGlyphRun();
    case DrawableObjectFunctionDirect2DDrawTextLayout: return new DrawableObjectDirect2DDrawText(/*shouldCreateTextLayout*/true);
    case DrawableObjectFunctionDirect2DDrawText: return new DrawableObjectDirect2DDrawText(/*shouldCreateTextLayout*/false);
    case DrawableObjectFunctionDirect2DDrawGlyphRun: return new DrawableObjectDirect2DDrawGlyphRun();
    case DrawableObjectFunctionGdiTextOut: return new DrawableObjectGdiTextOut();
    case DrawableObjectFunctionUser32DrawText: return new DrawableObjectUser32DrawText();
    case DrawableObjectFunctionGdiPlusDrawString: return new DrawableObjectGdiPlusDrawString();
    case DrawableObjectFunctionGdiPlusDrawDriverString: return new DrawableObjectGdiPlusDrawDriverString();
    case DrawableObjectFunctionDrawColorBitmapGlyphRun: return new DrawableObjectDirect2DDrawColorBitmapGlyphRun();
    case DrawableObjectFunctionDrawSvgGlyphRun: return new DrawableObjectDirect2DDrawSvgGlyphRun();
    }
}


void DrawableObject::GenerateLabel(IAttributeSource& attributeSource, _Inout_ std::u16string& label)
{
    // Update the current label, either using the explicit one or generating
    // one dynamically according to set attributes.
    array_ref<char16_t const> defaultLabel = attributeSource.GetString(DrawableObjectAttributeLabel);
    if (!defaultLabel.empty())
    {
        if (wcscmp(ToWChar(defaultLabel.data()), ToWChar(label.c_str())) != 0)
        {
            label.assign(defaultLabel.data(), defaultLabel.size());
        }
    }
    else
    {
        // Generate the label.
        label.clear();
        for (uint32_t attributeIndex = 0; attributeIndex < DrawableObjectAttributeTotal; ++attributeIndex)
        {
            char16_t const* prefix = nullptr;

            switch (attributeIndex)
            {
            // Skip any of these attributes, which should not contribute to the label.
            case DrawableObjectAttributeText:
            case DrawableObjectAttributeVisibility:
            case DrawableObjectAttributeLabel:
            case DrawableObjectAttributeWidth:
            case DrawableObjectAttributeHeight:
            case DrawableObjectAttributePadding:
            case DrawableObjectAttributePixelZoom:
            case DrawableObjectAttributeBackColor:
            case DrawableObjectAttributeLayoutColor:
                continue;

            case DrawableObjectAttributeColorFont:
            case DrawableObjectAttributePixelSnapping:
            case DrawableObjectAttributeClipping:
            case DrawableObjectAttributeUnderline:
            case DrawableObjectAttributeStrikethrough:
            case DrawableObjectAttributeFontFallback:
            case DrawableObjectAttributeTrimmingSign:
            case DrawableObjectAttributeUser32DrawTextAsEditControl:
                prefix = DrawableObject::attributeList[attributeIndex].name;
                break;
            }

            // Append another value, separating with comma and optional prefix.
            array_ref<char16_t const> text = attributeSource.GetString(DrawableObjectAttribute(attributeIndex));
            if (!text.empty())
            {
                if (!label.empty())
                    label.append(u", ", size_t(2));
                if (prefix != nullptr)
                {
                    label.append(prefix);
                    label.append(u"=", size_t(1));
                }
                label.append(text.data(), text.size());
            }
        }
    }
}


HRESULT SaveDWriteFontFile(
    IDWriteFontFace* fontFace,
    _In_z_ char16_t const* filePath
    )
{
    ComPtr<IDWriteFontFile> fontFile;
    ComPtr<IDWriteFontFileStream> fontFileStream;
    ComPtr<IDWriteFontFileLoader> fontFileLoader;
    void const* fontFileReferenceKey = nullptr;
    void const* fragment = nullptr;
    void* fragmentContext = nullptr;
    uint64_t fileSize = 0;
    uint32_t fontFileReferenceKeySize = 0;

    IFR(GetFontFile(fontFace, &fontFile));
    IFR(fontFile->GetLoader(OUT &fontFileLoader));
    IFR(fontFile->GetReferenceKey(OUT &fontFileReferenceKey, OUT &fontFileReferenceKeySize));
    IFR(fontFileLoader->CreateStreamFromKey(fontFileReferenceKey, fontFileReferenceKeySize, OUT &fontFileStream));
    IFR(fontFileStream->GetFileSize(OUT &fileSize));
    IFR(fontFileStream->ReadFileFragment(OUT &fragment, 0, fileSize, OUT &fragmentContext));
    return WriteBinaryFile(filePath, fragment, static_cast<uint32_t>(fileSize));
}


HRESULT SaveGdiFontFile(
    HFONT font,
    _In_z_ char16_t const* filePath
    )
{
    GdiDeviceContext hdc = CreateDC(L"DISPLAY", nullptr, nullptr, nullptr);
    HFONT previousFont = SelectFont(hdc, font);
    std::vector<uint8_t> fileData;
    auto fileSize = GetFontData(hdc, 0, 0, nullptr, 0);
    if (fileSize != GDI_ERROR)
    {
        fileData.resize(fileSize);
        GetFontData(hdc, 0, 0, fileData.data(), static_cast<DWORD>(fileData.size()));
    }
    SelectFont(hdc, previousFont);
    // todo: Ensure font is actual font, not a fallback like Arial.
    hdc.Clear();

    if (fileSize == GDI_ERROR)
    {
        return E_FAIL; // GetLastError doesn't seem to return a useful result (ERROR_SUCCESS).
    }

    return WriteBinaryFile(filePath, fileData);
}



HRESULT DrawableObject::SaveFontFile(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    char16_t const* filePath
    )
{
    ComPtr<IDWriteFontFace> fontFace;
    IFR(GetDWriteFontFace(attributeSource, drawingCanvas, OUT &fontFace));
    return SaveDWriteFontFile(fontFace, filePath);
}


HRESULT DrawableObject::GetDWriteFontFace(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    _COM_Outptr_ IDWriteFontFace** fontFace
    )
{
    *fontFace = nullptr;

    auto functionType = attributeSource.GetValue(DrawableObjectAttributeFunction, DrawableObjectFunctionNop);

    // Check whether it's a GDI or DWrite based function.
    if (IsGdiOrGdiPlusFunction(functionType))
    {
        CachedGdiFont gdiFont;
        ComPtr<IDWriteGdiInterop> gdiInterop;
        ComPtr<IDWriteFontFace> localFontFace;
        IFR(gdiFont.EnsureCached(attributeSource, drawingCanvas));

        array_ref<char16_t const> desiredFamilyName = attributeSource.GetString(DrawableObjectAttributeFontFamily);
        char16_t actualFamilyName[256];

        GdiDeviceContext hdc = CreateDC(L"DISPLAY", nullptr, nullptr, nullptr);
        HFONT previousFont = SelectFont(hdc, gdiFont.font);
        IDWriteFactory* factory = drawingCanvas.GetDWriteFactoryWeakRef();
        IFR(factory->GetGdiInterop(OUT &gdiInterop));
        IFR(gdiInterop->CreateFontFaceFromHdc(hdc, OUT &localFontFace));
        GetTextFace(hdc, static_cast<uint32_t>(countof(actualFamilyName)), OUT ToWChar(actualFamilyName));
        SelectFont(hdc, previousFont);

        if (wcscmp(ToWChar(actualFamilyName), ToWChar(desiredFamilyName.data())) != 0)
            return DWRITE_E_NOFONT; // GDI could not load the given font.

        *fontFace = localFontFace.Detach();
    }
    else
    {
        CachedDWriteFontFace dwriteFontFace;
        IFR(dwriteFontFace.Update(attributeSource, drawingCanvas));
        *fontFace = dwriteFontFace.fontFace.Detach();
    }

    return S_OK;
}



void AppendNumber(
    _Inout_ std::u16string& fileName,
    char16_t const* format,
    uint32_t number
    )
{
    // Generate the filename: base + number + extension.
    size_t const baseFileNameSize = fileName.size();
    size_t const maxDigitCount = 12;
    fileName.resize(baseFileNameSize + maxDigitCount);
    _snwprintf_s(ToWChar(&fileName[0] + baseFileNameSize), maxDigitCount, maxDigitCount, ToWChar(format), number);
    fileName.resize(wcslen(ToWChar(fileName.data())));
}


DWRITE_GLYPH_IMAGE_FORMATS const c_imageDataFormats =
    DWRITE_GLYPH_IMAGE_FORMATS_SVG  |
    DWRITE_GLYPH_IMAGE_FORMATS_PNG  |
    DWRITE_GLYPH_IMAGE_FORMATS_JPEG |
    DWRITE_GLYPH_IMAGE_FORMATS_TIFF |
    DWRITE_GLYPH_IMAGE_FORMATS_PREMULTIPLIED_B8G8R8A8;
    // Exclude TrueType, CFF, COLR


HRESULT DrawableObject::ExportFontGlyphData(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    array_ref<char16_t const> filePathPrefix
    )
{
    ComPtr<IDWriteFontFace> fontFace;
    ComPtr<IDWriteFontFace4> fontFace4;
    DWRITE_FONT_METRICS fontMetrics;

    IFR(GetDWriteFontFace(attributeSource, drawingCanvas, OUT &fontFace));

    std::map<uint32_t, uint32_t> glyphToUnicodeCodepoint;
    IFR(fontFace->QueryInterface(OUT &fontFace4));
    fontFace->GetMetrics(OUT &fontMetrics);

    DWRITE_GLYPH_IMAGE_FORMATS glyphImageFormats = fontFace4->GetGlyphImageFormats();

    if ((glyphImageFormats & c_imageDataFormats) == DWRITE_GLYPH_IMAGE_FORMATS_NONE)
        return S_FALSE;

    // Map every glyph in the font to a Unicode character for the file naming.
    {
        std::vector<uint32_t> unicodeCharacters(UnicodeTotal);
        std::vector<uint16_t> glyphIds(UnicodeTotal);
        std::iota(unicodeCharacters.data(), unicodeCharacters.data() + UnicodeTotal, 0);
        IFR(fontFace->GetGlyphIndices(unicodeCharacters.data(), UnicodeTotal, glyphIds.data()));
        for (uint32_t ch = 0; ch < UnicodeTotal; ++ch)
        {
            auto glyphId = glyphIds[ch];
            if (glyphId != 0)
                glyphToUnicodeCodepoint.insert({glyphId, ch});
        }
    }

    std::u16string filePath(filePathPrefix.data(), filePathPrefix.data_end());

    for (uint32_t glyphId = 0, glyphCount = fontFace->GetGlyphCount(); glyphId < glyphCount; ++glyphId)
    {
        fontFace4->GetGlyphImageFormats(glyphId, 0, UINT32_MAX, OUT &glyphImageFormats);

        // Mask out any unknown formats (or monochrome outline formats like TrueType and CFF).
        glyphImageFormats &= c_imageDataFormats;

        // Enumerate all the image formats found for this glyph.
        for (DWRITE_GLYPH_IMAGE_FORMATS currentGlyphImageFormat = DWRITE_GLYPH_IMAGE_FORMATS(1);
             glyphImageFormats >= currentGlyphImageFormat;
             currentGlyphImageFormat = DWRITE_GLYPH_IMAGE_FORMATS(currentGlyphImageFormat << 1))
        {
            if (glyphImageFormats & currentGlyphImageFormat)
            {
                glyphImageFormats &= ~currentGlyphImageFormat;

                // Generate the filename: base + glyphid + [unicode] extension.
                char16_t const* filenameExtension = u".bin";
                switch (currentGlyphImageFormat)
                {
                case DWRITE_GLYPH_IMAGE_FORMATS_SVG:  filenameExtension = u".svg";  break;
                case DWRITE_GLYPH_IMAGE_FORMATS_PNG:  filenameExtension = u".png";  break;
                case DWRITE_GLYPH_IMAGE_FORMATS_JPEG: filenameExtension = u".jpeg"; break;
                case DWRITE_GLYPH_IMAGE_FORMATS_TIFF: filenameExtension = u".tiff"; break;
                // case DWRITE_GLYPH_IMAGE_FORMATS_PREMULTIPLIED_B8G8R8A8: keep .bin
                }

                filePath.resize(filePathPrefix.size());
                AppendNumber(IN OUT filePath, u"g%05d", glyphId);
                uint32_t unicodeCharacter = glyphToUnicodeCodepoint[glyphId];
                if (unicodeCharacter != 0)
                {
                    filePath.append(u"_U+", 3);
                    AppendNumber(IN OUT filePath, u"%04X", unicodeCharacter);
                }
                filePath += filenameExtension;

                // Write the image data to a file.
                DWRITE_GLYPH_IMAGE_DATA glyphData;
                void* glyphDataContext = nullptr;
                fontFace4->GetGlyphImageData(
                    glyphId,
                    fontMetrics.designUnitsPerEm,
                    currentGlyphImageFormat,
                    OUT &glyphData,
                    OUT &glyphDataContext
                    );

                HRESULT hr = WriteBinaryFile(filePath.c_str(), glyphData.imageData, glyphData.imageDataSize);

                fontFace4->ReleaseGlyphImageData(glyphDataContext);

                IFR(hr);
            }
        }
    }

    return S_OK;
}


HRESULT DrawableObject::GetFontCharacters(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    bool getOnlyColorFontCharacters,
    _Out_ std::u16string& characters
    )
{
    ComPtr<IDWriteFontFace> fontFace;
    IFR(GetDWriteFontFace(attributeSource, drawingCanvas, OUT &fontFace));

    std::vector<uint16_t> characterCounts;
    IFR(GetFontCharacterCoverageCounts(
        { &fontFace, 1 },
        { },
        getOnlyColorFontCharacters,
        [&](uint32_t i, uint32_t total) {;},
        OUT characterCounts
        ));

    IFR(GetStringFromCoverageCount(characterCounts, 1, UINT32_MAX, OUT characters));

    return S_OK;
}


HRESULT CachedTransform::Update(IAttributeSource& attributeSource)
{
    array_ref<float const> transformData = attributeSource.GetValues<float>(DrawableObjectAttributeTransform);
    auto transformDataSize = transformData.size();
    for (size_t i = 0; i < 6; ++i)
    {
        transform.m[i] = (i < transformDataSize) ? transformData[i] : DrawableObject::identityTransform.m[i];
    }

    return S_OK;
}


class DECLSPEC_UUID("DDF3A7A6-8D13-4F43-8565-6E66560AD385") GdiAddedFontResource : public ComObject
{
public:
    GdiAddedFontResource(array_ref<char16_t const> filePath)
    :   filePath_(filePath.data(), filePath.size())
    {
        // Avoid the font being enumerated because we don't want to ChooseFont dialog to see a temporary font.
        AddFontResourceEx(ToWChar(filePath.data()), FR_PRIVATE | FR_NOT_ENUM, nullptr);
    }

    ~GdiAddedFontResource()
    {
        RemoveFontResourceEx(ToWChar(filePath_.data()), FR_PRIVATE | FR_NOT_ENUM, nullptr);
    }

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(IID const& iid, _Out_ void** object) throw() override
    {
        COM_BASE_RETURN_INTERFACE(iid, GdiAddedFontResource, object);
        COM_BASE_RETURN_INTERFACE(iid, IUnknown, object);
        COM_BASE_RETURN_NO_INTERFACE(object);
    }

    std::u16string filePath_;
};


HRESULT CachedGdiFont::EnsureCached(IAttributeSource& attributeSource, DrawingCanvas& drawingCanvas)
{
    float fontSize = attributeSource.GetValue(DrawableObjectAttributeFontSize, DrawableObject::defaultFontSize);

    array_ref<char16_t const> familyName = attributeSource.GetString(DrawableObjectAttributeFontFamily);
    array_ref<char16_t const> customFontFilePath = attributeSource.GetString(DrawableObjectAttributeFontFilePath);

    // Use either filename or family name.
    ComPtr<GdiAddedFontResource> gdiAddedFontResource;
    if (!customFontFilePath.empty() && drawingCanvas.GetSharedResource(customFontFilePath.data(), OUT &gdiAddedFontResource) == E_NOT_SET)
    {
        gdiAddedFontResource.Set(new GdiAddedFontResource(customFontFilePath));
        drawingCanvas.SetSharedResource(customFontFilePath.data(), gdiAddedFontResource.Get());
    }

    bool hasUnderline = attributeSource.GetValue(DrawableObjectAttributeUnderline, false);
    bool hasStrikethrough = attributeSource.GetValue(DrawableObjectAttributeStrikethrough, false);
    DWRITE_FONT_WEIGHT fontWeight = attributeSource.GetValue(DrawableObjectAttributeWeight, DWRITE_FONT_WEIGHT_NORMAL);
    DWRITE_FONT_STYLE fontStyle = attributeSource.GetValue(DrawableObjectAttributeSlope, DWRITE_FONT_STYLE_NORMAL);
    auto gdiRenderingModeQuality = attributeSource.GetValue(DrawableObjectAttributeGdiRenderingMode, DEFAULT_QUALITY);

    LOGFONT logFont = {};
    wcsncpy_s(logFont.lfFaceName, ToWChar(familyName.data()), _TRUNCATE);
    logFont.lfHeight            = -static_cast<LONG>(floor(fontSize));
    logFont.lfWidth             = 0;
    logFont.lfEscapement        = 0;
    logFont.lfOrientation       = 0;
    logFont.lfWeight            = fontWeight;
    logFont.lfItalic            = (fontStyle > DWRITE_FONT_STYLE_NORMAL);
    logFont.lfUnderline         = hasUnderline;
    logFont.lfStrikeOut         = hasStrikethrough;
    logFont.lfCharSet           = DEFAULT_CHARSET;
    logFont.lfOutPrecision      = OUT_DEFAULT_PRECIS;
    logFont.lfClipPrecision     = CLIP_DEFAULT_PRECIS;
    logFont.lfQuality           = gdiRenderingModeQuality;
    logFont.lfPitchAndFamily    = DEFAULT_PITCH;

    // If vertical reading direction, and there is not already an '@', then add one.
    uint32_t readingDirection = attributeSource.GetValue(DrawableObjectAttributeReadingDirection, 0ui32);
    if ((readingDirection & 4) && logFont.lfFaceName[0] != '@')
    {
        memmove(&logFont.lfFaceName[1], &logFont.lfFaceName[0], sizeof(logFont.lfFaceName) - sizeof(logFont.lfFaceName[0]));
        logFont.lfFaceName[LF_FACESIZE - 1] = '\0';
        logFont.lfFaceName[0] = '@';
    }

    font = CreateFontIndirect(&logFont);

    if (font.IsNull())
        return DWRITE_E_NOFONT; // Does GetLastError work with CreateFontIndirect? Otherwise use the DWrite error, which is close.

    return S_OK;
}


HRESULT DrawableObjectGdiTextOut::GetBounds(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    _Out_ D2D_RECT_F& layoutBounds,
    _Out_ D2D_RECT_F& contentBounds
    )
{
    layoutBounds = emptyRect;
    array_ref<char16_t const> text = attributeSource.GetString(DrawableObjectAttributeText);
    array_ref<uint16_t const> glyphs = attributeSource.GetValues<uint16_t>(DrawableObjectAttributeGlyphs);
    uint32_t readingDirection = attributeSource.GetValue(DrawableObjectAttributeReadingDirection, 0ui32);

    font_.EnsureCached(attributeSource, drawingCanvas);
    HDC hdc = drawingCanvas.GetHDC();

    SIZE integerSize = {};
    HFONT previousFont = SelectFont(hdc, font_.font);

    // There is a bug in GetTextExtentPoint32 that if the text is too simple,
    // it doesn't measure line feeds correctly (or at least, inconsistently
    // with what ExtTextOut actually draws). Inserting any complex character
    // like an LRM or fallback characters triggers the correct measurement.
    GetTextExtentPoint32(hdc, ToWChar(text.data()), int(text.size()), OUT &integerSize);
    SelectFont(hdc, previousFont);

    if (!glyphs.empty() || !attributeSource.GetString(DrawableObjectAttributeGlyphs).empty())
    {
        float layoutWidth = attributeSource.GetValue(DrawableObjectAttributeWidth, DrawableObject::defaultWidth);
        integerSize.cx = LONG(floor(layoutWidth)); // todo::: measure actual glyph advances.
    }

    contentBounds.left = 0;
    contentBounds.top = 0;
    contentBounds.right = static_cast<float>(integerSize.cx);
    contentBounds.bottom = static_cast<float>(integerSize.cy);

    if (readingDirection & 1) // Swap for RTL.
    {
        contentBounds.left = -contentBounds.right;
        contentBounds.right = 0;
    }

    return S_OK;
}


HRESULT DrawableObjectGdiTextOut::Draw(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    float x,
    float y,
    DX_MATRIX_3X2F const& transform
    )
{
    array_ref<char16_t const> text = attributeSource.GetString(DrawableObjectAttributeText);
    array_ref<uint16_t const> glyphs = attributeSource.GetValues<uint16_t>(DrawableObjectAttributeGlyphs);

    font_.EnsureCached(attributeSource, drawingCanvas);
    HDC hdc = drawingCanvas.GetHDC();
    HFONT previousFont = SelectFont(hdc, font_.font);

    uint32_t bgraTextColor = attributeSource.GetValue(DrawableObjectAttributeTextColor, defaultFontColor);
    SetTextColor(hdc, ToColorRef(bgraTextColor));
    SetBkMode(hdc, TRANSPARENT);

    // Use TA_RTLREADING instead of ETO_RTLREADING or SetLayout LAYOUT_RTL.
    uint32_t readingDirection = attributeSource.GetValue(DrawableObjectAttributeReadingDirection, 0ui32);
    uint32_t textAlignFlags = TA_NOUPDATECP;
    if (readingDirection & 1)
        textAlignFlags |= TA_RTLREADING | TA_RIGHT;

    uint32_t oldTextAlignment = GetTextAlign(hdc);
    SetTextAlign(hdc, textAlignFlags);

    uint32_t textOutFlags = 0;

    bool isClipped = attributeSource.GetValue(DrawableObjectAttributeClipping, false);
    if (isClipped)
        textOutFlags |= ETO_CLIPPED;

    // Unused flags:
    // ETO_OPAQUE left unset, drawing the background from GetBounds instead.
    // ETO_NUMERICSLATIN / ETO_NUMERICSLOCAL

    // todo:::  Figure out why new lines cause issue with GetTextExtentPoint32.
    //          Strangely the measurement inconsistency bug between it and ExtTextOut
    //          only happens with simple ASCII. Using any characters beyond 0..127
    //          fixes the issue.
    SetWorldTransform(hdc, &transform.gdi);

    SIZE integerSize = {};
    GetTextExtentPoint32(hdc, ToWChar(text.data()), int(text.size()), OUT &integerSize);
    RECT rect = { int(x), int(y), int(x) + integerSize.cx, int(y) + integerSize.cy };

    if (!glyphs.empty() || !attributeSource.GetString(DrawableObjectAttributeGlyphs).empty())
    {
        text.reset(glyphs.reinterpret_as<char16_t const>());
        textOutFlags |= ETO_GLYPH_INDEX;
        // todo::: Pass glyph advances if non-empty. ETO_PDY
    }

    ExtTextOut(hdc, int(x), int(y), textOutFlags, &rect, ToWChar(text.data()), uint32_t(text.size()), nullptr);

    SetWorldTransform(hdc, &DrawableObject::identityTransform.gdi);
    SetTextAlign(hdc, oldTextAlignment);
    SelectFont(hdc, previousFont);

    return S_OK;
}


HRESULT DrawableObjectUser32DrawText::GetBounds(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    _Out_ D2D_RECT_F& layoutBounds,
    _Out_ D2D_RECT_F& contentBounds
    )
{
    return DrawInternal(attributeSource, drawingCanvas, /*x*/0, /*y*/0, DrawableObject::identityTransform, &layoutBounds, &contentBounds);
}


HRESULT DrawableObjectUser32DrawText::Draw(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    float x,
    float y,
    DX_MATRIX_3X2F const& transform
    )
{
    return DrawInternal(attributeSource, drawingCanvas, x, y, transform, /*layoutBounds*/nullptr, /*contentBounds*/nullptr);
}


HRESULT DrawableObjectUser32DrawText::DrawInternal(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    float x,
    float y,
    DX_MATRIX_3X2F const& transform,
    _Out_opt_ D2D_RECT_F* layoutBounds, // x and y are expected to be zero if non-null.
    _Out_opt_ D2D_RECT_F* contentBounds
    )
{
    font_.EnsureCached(attributeSource, drawingCanvas);

    ////////////////////
    // Read attributes

    array_ref<char16_t const> text = attributeSource.GetString(DrawableObjectAttributeText);
    COLORREF bgraTextColor = attributeSource.GetValue(DrawableObjectAttributeTextColor, COLORREF(defaultFontColor));
    DrawableObjectAlignmentMode columnAlignment = attributeSource.GetValue(DrawableObjectAttributeColumnAlignment, DrawableObjectAlignmentModeLeading);
    DrawableObjectAlignmentMode rowAlignment = attributeSource.GetValue(DrawableObjectAttributeRowAlignment, DrawableObjectAlignmentModeLeading);
    DrawableObjectLineWrappingMode wrappingMode = attributeSource.GetValue(DrawableObjectAttributeLineWrappingMode, LineWrappingModeWordCharacter);
    DrawableObjectHotkeyMode hotkeyMode = attributeSource.GetValue(DrawableObjectAttributeHotkeyMode, DrawableObjectHotkeyModeNone);
    DrawableObjectTrimmingGranularity trimmingGranularity = attributeSource.GetValue(DrawableObjectAttributeTrimmingGranularity, DrawableObjectTrimmingGranularityNone);
    char32_t trimmingDelimiter = attributeSource.GetValue(DrawableObjectAttributeTrimmingDelimiter, '\0');
    uint32_t readingDirection = attributeSource.GetValue(DrawableObjectAttributeReadingDirection, 0ui32);
    uint32_t textAlignFlags = TA_TOP | TA_LEFT | TA_NOUPDATECP; // Leave these alone, preferring DT_RTLREADING over TA_RTLREADING.
    bool isReversedReadingDirection = !!(readingDirection & 1); // RTL/BTT
    bool isVerticalReadingDirection = !!(readingDirection & 4); // TTB/BTT

    ////////////////////
    // Calculate layout rectangle.

    float layoutWidth = attributeSource.GetValue(DrawableObjectAttributeWidth, DrawableObject::defaultWidth);
    float layoutHeight = attributeSource.GetValue(DrawableObjectAttributeHeight, DrawableObject::defaultHeight);
    if (layoutBounds != nullptr)
    {
        layoutBounds->left = 0;
        layoutBounds->top = 0;
        layoutBounds->right = floor(layoutWidth);
        layoutBounds->bottom = floor(layoutHeight);
    }

    // Calculate rect where DrawText will draw.
    LONG left = static_cast<LONG>(x);
    LONG top = static_cast<LONG>(y);
    LONG intLayoutWidth = static_cast<LONG>(floor(layoutWidth));
    LONG intLayoutHeight = static_cast<LONG>(floor(layoutHeight));
    RECT rect = {left, top, left + intLayoutWidth, top + intLayoutHeight};
    DX_MATRIX_3X2F finalTransform;

    ////////////////////
    // For vertical, rotate the rectangle cw90, and compute the new transform.
    // DrawText does not (very surprisingly) just do this right in the first place.
    if (isVerticalReadingDirection)
    {
        std::swap(intLayoutWidth, intLayoutHeight);
        rect.left   = top;
        rect.top    = -left - intLayoutHeight;
        rect.right  = top + intLayoutWidth;
        rect.bottom = -left;
        DX_MATRIX_3X2F cw90 = {0,1,-1,0,0,0};
        CombineMatrix(cw90, transform, OUT finalTransform);
    }
    else
    {
        finalTransform = transform;
    }

    ////////////////////
    // Set all drawing flags.

    uint32_t drawTextFlags = 0;

    switch (hotkeyMode)
    {
    default:
    case DrawableObjectHotkeyModeNone: drawTextFlags |= DT_NOPREFIX;    break;
    case DrawableObjectHotkeyModeShow: /* no change to drawTextFlags */ break;
    case DrawableObjectHotkeyModeHide: drawTextFlags |= DT_HIDEPREFIX;  break;
    }

    bool isClipped = attributeSource.GetValue(DrawableObjectAttributeClipping, false);
    if (!isClipped)
    {
        drawTextFlags |= DT_NOCLIP;
    }
    if (wrappingMode != LineWrappingModeNone)
    {
        drawTextFlags |= DT_WORDBREAK;
    }
    if (isReversedReadingDirection) // RTL/BTT
    {
        drawTextFlags |= DT_RTLREADING;
        drawTextFlags |=
            (columnAlignment == DrawableObjectAlignmentModeCenter) ? DT_CENTER :
            (columnAlignment == DrawableObjectAlignmentModeTrailing) ? DT_LEFT :
            DT_RIGHT;
    }
    else // LTR/TTB
    {
        drawTextFlags |=
            (columnAlignment == DrawableObjectAlignmentModeCenter) ? DT_CENTER :
            (columnAlignment == DrawableObjectAlignmentModeTrailing) ? DT_RIGHT :
            DT_LEFT;
    }

    // Map our trimming to GDI, at least as closely as possible.
    // Note GDI only supports ellipsis (no trimming without ellipsis sign).
    switch (trimmingGranularity)
    {
    case DrawableObjectTrimmingGranularityCharacter:
    case DrawableObjectTrimmingGranularityPartialGlyph: drawTextFlags |= DT_END_ELLIPSIS; break;
    case DrawableObjectTrimmingGranularityWord: drawTextFlags |= DT_WORD_ELLIPSIS; break;
    }
    if (trimmingGranularity != DrawableObjectTrimmingGranularityNone && trimmingDelimiter == '\\')
    {
        drawTextFlags |= DT_PATH_ELLIPSIS;
    }
    
    array_ref<float const> tabWidths = attributeSource.GetValues<float>(DrawableObjectAttributeTabWidth);
    if (!tabWidths.empty())
    {
        // Don't bother supporting tab stop distances since GDI doesn't allow precise units,
        // merely multiples of average characters.
        drawTextFlags |= DT_EXPANDTABS;
    }

    if (attributeSource.GetValue(DrawableObjectAttributeUser32DrawTextAsEditControl, false))
    {
        drawTextFlags |= DT_EDITCONTROL;
    }

    /* Unused flags.
    DT_NOFULLWIDTHCHARBREAK;
    DT_SINGLELINE;
    DT_VCENTER
    DT_TABSTOP
    */

    if (contentBounds != nullptr)
    {
        drawTextFlags |= DT_CALCRECT;
    }

    ////////////////////
    // Draw the text.

    HDC hdc = drawingCanvas.GetHDC();
    HFONT previousFont = SelectFont(hdc, font_.font);
    uint32_t oldTextAlignment = GetTextAlign(hdc);
    SetTextAlign(hdc, textAlignFlags);
    SetTextColor(hdc, ToColorRef(bgraTextColor));
    SetBkMode(hdc, TRANSPARENT);
    SetWorldTransform(hdc, &finalTransform.gdi);

    DRAWTEXTPARAMS drawTextParams = {sizeof(drawTextParams)};

    // Avoid empty rect because DrawText draws nothing, even with DT_NOCLIP set!
    // (other text API's simply wrap as tightly as they can)
    rect.right = std::max(rect.right, rect.left + 1);

    // Cast away the constness because DrawTextEx unwisely took the input string as non-const.
    wchar_t* wcharText = const_cast<wchar_t*>(ToWChar(text.data()));

    // Since DT_VCENTER actually works really poorly, just do it ourselves.
    // DrawText returns a rectangle with a correct bottom edge and a zeroed top,
    // which is not useful. Additionally, it doesn't work if you have more than
    // one line of text.
    #ifdef USE_DRAWTEXT_DT_VCENTER
    if (std::none_of(text.begin(), text.end(), [](wchar_t ch)->bool {return ch == 0x000A || ch == 0x000D; }))
    {
        drawTextFlags |= DT_SINGLELINE;
        drawTextFlags |=
            (rowAlignment == DrawableObjectAlignmentModeCenter) ? DT_VCENTER :
            (rowAlignment == DrawableObjectAlignmentModeTrailing) ? DT_BOTTOM :
            DT_TOP;
    }
    #else
    if (rowAlignment != DrawableObjectAlignmentModeLeading)
    {
        RECT contentRect = rect;
        DrawTextEx(hdc, wcharText, static_cast<uint32_t>(text.size()), IN OUT &contentRect, drawTextFlags | DT_CALCRECT, &drawTextParams);
        auto contentHeight = contentRect.bottom - contentRect.top;
        switch (rowAlignment)
        {
        case DrawableObjectAlignmentModeTrailing: rect.top = rect.bottom - contentHeight; break;
        case DrawableObjectAlignmentModeCenter: rect.top = (intLayoutHeight - contentHeight) / 2; break;
        }
    }
    #endif

    DrawTextEx(hdc, wcharText, static_cast<uint32_t>(text.size()), IN OUT &rect, drawTextFlags, &drawTextParams);

    SetWorldTransform(hdc, &DrawableObject::identityTransform.gdi);
    SetTextAlign(hdc, oldTextAlignment);
    SelectFont(hdc, previousFont);

    ////////////////////
    // Return the calculated bounds.

    if (contentBounds != nullptr)
    {
        LONG columnWidth = rect.right - rect.left;
        LONG right;

        // Fix the DT_CALCRECT bad behavior. Even though it returns coordinates
        // in a rect, it doesn't actually set left and top fields!? This means
        // RTL and other alignments appear at the wrong location :/.
        switch (drawTextFlags & 3)
        {
        case DT_RIGHT:
            left = intLayoutWidth - columnWidth;
            right = intLayoutWidth;
            break;

        case DT_CENTER:
            left = (intLayoutWidth - columnWidth) / 2;
            right = left + columnWidth;
            break;

        case DT_LEFT: // Leave alone.
        default:
            left = 0;
            right = columnWidth;
            break;
        }

        rect.right = rect.left + right;
        rect.left = rect.left + left;
        if (isVerticalReadingDirection)
        {
            left        = rect.left;
            rect.left   = -rect.bottom;
            rect.bottom = rect.right;
            rect.right  = -rect.top;
            rect.top    = left;
        }

        contentBounds->left   = static_cast<float>(rect.left);
        contentBounds->top    = static_cast<float>(rect.top);
        contentBounds->right  = static_cast<float>(rect.right);
        contentBounds->bottom = static_cast<float>(rect.bottom);
    }

    return S_OK;
}


HRESULT CachedDWriteFontFace::Update(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas
    )
{
    // Invalidate the cached face.
    if (attributeSource.IsCookieSame(DrawableObjectAttributeFontFilePath, IN OUT cookieFontFilePath)
    &&  attributeSource.IsCookieSame(DrawableObjectAttributeFontFamily, IN OUT cookieFamilyName)
    &&  attributeSource.IsCookieSame(DrawableObjectAttributeWeight, IN OUT cookieWeight)
    &&  attributeSource.IsCookieSame(DrawableObjectAttributeStretch, IN OUT cookieStretch)
    &&  attributeSource.IsCookieSame(DrawableObjectAttributeSlope, IN OUT cookieSlope)
    &&  attributeSource.IsCookieSame(DrawableObjectAttributeFontSimulations, IN OUT cookieFontSimulations)
    &&  attributeSource.IsCookieSame(DrawableObjectAttributeFontFaceIndex, IN OUT cookieFontFaceIndex)
    &&  attributeSource.IsCookieSame(DrawableObjectAttributeDWriteFontFaceType, IN OUT cookieDWriteFontFaceType)
    &&  fontFace != nullptr
        )
    {
        return S_OK;
    }

    std::string sampleText;
    std::vector<uint32_t> codePointArray;
    std::copy(sampleText.begin(), sampleText.end(), codePointArray.begin());

    fontFace.clear();

    array_ref<char16_t const> customFontFilePath = attributeSource.GetString(DrawableObjectAttributeFontFilePath);

    // Use either filename or family name.
    // todo: Consider whether to search for a name inside a custom font collection rather than faceIndex.
    if (!customFontFilePath.empty())
    {
        if (GetFileAttributes(ToWChar(customFontFilePath.data())) == -1)
            return DWRITE_E_FILENOTFOUND;

        auto fontSimulations = attributeSource.GetValue(DrawableObjectAttributeFontSimulations, DWRITE_FONT_SIMULATIONS_NONE);
        auto fontFaceIndex = attributeSource.GetValue(DrawableObjectAttributeFontFaceIndex, 0ui32);
        auto fontFaceType = attributeSource.GetValue(DrawableObjectAttributeDWriteFontFaceType, DWRITE_FONT_FACE_TYPE_UNKNOWN);
        return CreateFontFaceFromFile(
            drawingCanvas.GetDWriteFactoryWeakRef(),
            ToWChar(customFontFilePath.data()),
            fontFaceIndex,
            fontFaceType,
            fontSimulations,
            OUT &this->fontFace
            );
    }
    else // Use family name in collection.
    {
        auto fontSimulations = attributeSource.GetValue(DrawableObjectAttributeFontSimulations, DWRITE_FONT_SIMULATIONS_NONE);

        array_ref<char16_t const> fontFamilyName = attributeSource.GetString(DrawableObjectAttributeFontFamily);
        DWRITE_FONT_WEIGHT fontWeight = attributeSource.GetValue(DrawableObjectAttributeWeight, DWRITE_FONT_WEIGHT_NORMAL);
        DWRITE_FONT_STRETCH fontStretch = attributeSource.GetValue(DrawableObjectAttributeStretch, DWRITE_FONT_STRETCH_NORMAL);
        DWRITE_FONT_STYLE fontStyle = attributeSource.GetValue(DrawableObjectAttributeSlope, DWRITE_FONT_STYLE_NORMAL);

        IDWriteFactory* factory = drawingCanvas.GetDWriteFactoryWeakRef();
        ComPtr<IDWriteFontCollection> systemFontCollection;
        IFR(factory->GetSystemFontCollection(OUT &systemFontCollection));
        IFR(CreateFontFace(
            systemFontCollection,
            ToWChar(fontFamilyName.data()),
            fontWeight,
            fontStretch,
            fontStyle,
            OUT &this->fontFace
            ));

        // If fontSimulations are set, recreate it using them instead.
        if (fontSimulations != DWRITE_FONT_SIMULATIONS_NONE)
        {
            ComPtr<IDWriteFontFace> simulatedFontFace;
            IFR(RecreateFontFace(
                drawingCanvas.GetDWriteFactoryWeakRef(),
                this->fontFace,
                fontSimulations,
                &simulatedFontFace
                ));
            std::swap(simulatedFontFace, this->fontFace);
        }
    }

    return S_OK;
}


HRESULT CachedDWriteRenderingParams::Update(IAttributeSource& attributeSource, DrawingCanvas& drawingCanvas)
{
    // Invalidate the cached rendering params.
    if (attributeSource.IsCookieSame(DrawableObjectAttributeDWriteRenderingMode, IN OUT cookieRenderingMode)
    &&  renderingParams != nullptr)
    {
        return S_OK;
    }

    renderingParams.clear();

    // Get the rendering mode and check with the one the canvas is already using.
    // If compatible, just use it directly.
    auto dwriteRenderingMode = attributeSource.GetValue(DrawableObjectAttributeDWriteRenderingMode, DWRITE_RENDERING_MODE_DEFAULT);
    auto* canvasRenderingParams = drawingCanvas.GetDirectWriteRenderingParamsWeakRef();
    if (canvasRenderingParams != nullptr && canvasRenderingParams->GetRenderingMode() == dwriteRenderingMode)
    {
        renderingParams = canvasRenderingParams;
        return S_OK;
    }

    // Otherwise create custom one, using the default one as a template for any unspecified parameters.
    auto* dwriteFactory = drawingCanvas.GetDWriteFactoryWeakRef();
    ComPtr<IDWriteRenderingParams> defaultRenderingParams;
    IFR(dwriteFactory->CreateRenderingParams(OUT &defaultRenderingParams));
    return dwriteFactory->CreateCustomRenderingParams(
        defaultRenderingParams->GetGamma(), // default=1.8f
        defaultRenderingParams->GetEnhancedContrast(), // default=0.5f
        defaultRenderingParams->GetClearTypeLevel(), // default=0.5f
        defaultRenderingParams->GetPixelGeometry(), // default=RGB for most monitors
        dwriteRenderingMode,
        OUT &renderingParams
        );
}


// This probably should only exist on the stack within Draw or GetSize calls,
// and not in a class definition since it contains pointers to data structures
// that exist transiently.
class CachedDWriteGlyphRun : public DWRITE_GLYPH_RUN
{
public:
    HRESULT Update(IAttributeSource& attributeSource, DrawingCanvas& drawingCanvas, _In_ IDWriteFontFace* fontFace);
    HRESULT GetGlyphAdvancesIfNull(); // Call Update first.

protected:
    std::vector<uint16_t> glyphBuffer_; // only used for nominal glyphs if attribute source returned none.
    std::vector<float> glyphAdvances_; // only used if null glyph advances, and they are desired.
};


HRESULT CachedDWriteGlyphRun::Update(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    _In_ IDWriteFontFace* newFontFace // weak reference
    )
{
    array_ref<DWRITE_GLYPH_OFFSET const> glyphOffsets;
    array_ref<uint16_t const> glyphs = attributeSource.GetValues<uint16_t>(DrawableObjectAttributeGlyphs);
    array_ref<float const> glyphAdvances = attributeSource.GetValues<float>(DrawableObjectAttributeAdvances);
    array_ref<float const> glyphOffsetFloats = attributeSource.GetValues<float>(DrawableObjectAttributeOffsets);
    float fontSize = attributeSource.GetValue(DrawableObjectAttributeFontSize, DrawableObject::defaultFontSize);
    uint32_t readingDirection = attributeSource.GetValue(DrawableObjectAttributeReadingDirection, 0ui32);

    // If no glyphs were given, but text was, then read the text to get nominal glyph id's via the cmap.
    if (glyphs.empty() && attributeSource.GetString(DrawableObjectAttributeGlyphs).empty())
    {
        array_ref<char16_t const> text = attributeSource.GetString(DrawableObjectAttributeText);
        if (!text.empty())
        {
            std::vector<char32_t> utf32text;
            utf32text.resize(text.size());

            size_t convertedLength = static_cast<uint32_t>(ConvertTextUtf16ToUtf32NoReplacement(
                text,
                OUT utf32text,
                nullptr // sourceCount
                ));
            utf32text.resize(convertedLength);
            glyphBuffer_.resize(convertedLength);

            uint32_t* codePoints = reinterpret_cast<uint32_t*>(utf32text.data());
            IFR(newFontFace->GetGlyphIndices(codePoints, uint32_t(convertedLength), OUT glyphBuffer_.data()));
            glyphs = glyphBuffer_;
        }
    }

    // Recast the dx/dy float pairs to DWRITE_GLYPH_OFFSET's.
    if (glyphOffsetFloats.size() >= glyphs.size() * 2U)
    {
        glyphOffsets.reset(glyphOffsetFloats.reinterpret_as<DWRITE_GLYPH_OFFSET const>());
    }

    glyphAdvances_.clear();

    // If the array of advances is smaller than the glyph run, repeat the last advance.
    if (!glyphAdvances.empty() && glyphAdvances.size() < glyphs.size())
    {
        glyphAdvances_.resize(glyphs.size());
        uint32_t lastGlyphIndex = static_cast<uint32_t>(glyphAdvances.size()) - 1;
        uint32_t currentGlyphIndex = 0;
        for (auto& advance : glyphAdvances_)
        {
            advance = glyphAdvances[std::min(currentGlyphIndex, lastGlyphIndex)];
            ++currentGlyphIndex;
        }
        glyphAdvances = glyphAdvances_;
    }

    DWRITE_GLYPH_RUN& glyphRun = *static_cast<DWRITE_GLYPH_RUN*>(this);
    glyphRun = {
        newFontFace,
        fontSize,
        static_cast<uint32_t>(glyphs.size()),
        glyphs.data(),
        glyphAdvances.size() < glyphs.size() ? nullptr : glyphAdvances.data(),
        glyphOffsets.size()  < glyphs.size() ? nullptr : glyphOffsets.data(),
        !!(readingDirection & 4), // isSideways
        readingDirection & 1 // bidiLevel
    };

    return S_OK;
}


HRESULT CachedDWriteGlyphRun::GetGlyphAdvancesIfNull()
{
    DWRITE_GLYPH_RUN& glyphRun = static_cast<DWRITE_GLYPH_RUN&>(*this);
    if (glyphRun.fontFace == nullptr)
        return DWRITE_E_NOFONT;

    if (glyphRun.glyphAdvances != nullptr || glyphRun.glyphCount == 0)
        return S_OK;

    // todo: Replace with 
    //IFR(GetFontFaceAdvances(
    //    cachedGlyphRun.fontFace,
    //    cachedGlyphRun.fontEmSize,
    //    { cachedGlyphRun.glyphIndices, cachedGlyphRun.glyphCount },
    //    measuringMode,
    //    !!cachedGlyphRun.isSideways,
    //    OUT glyphAdvancesBuffer
    //    ));
    //glyphAdvances = glyphAdvancesBuffer.data();

    std::vector<int32_t> glyphAdvances(glyphRun.glyphCount);
    ComPtr<IDWriteFontFace1> fontFace1;
    if (SUCCEEDED(fontFace->QueryInterface(OUT &fontFace1)))
    {
        fontFace1->GetDesignGlyphAdvances(glyphRun.glyphCount, glyphRun.glyphIndices, OUT glyphAdvances.data(), glyphRun.isSideways);
    }

    // Scale glyph advances down.
    DWRITE_FONT_METRICS fontMetrics;
    fontFace->GetMetrics(OUT &fontMetrics);
    glyphAdvances_.resize(glyphRun.glyphCount);
    for (uint32_t i = 0; i < glyphRun.glyphCount; ++i)
    {
        glyphAdvances_[i] = glyphRun.fontEmSize * glyphAdvances[i] / fontMetrics.designUnitsPerEm;
    }

    // Update the glyph run.
    glyphRun.glyphAdvances = glyphAdvances_.data();

    return S_OK;
}


const static DWRITE_READING_DIRECTION g_dwriteReadingDirectionValues[8] = {
    DWRITE_READING_DIRECTION_LEFT_TO_RIGHT,
    DWRITE_READING_DIRECTION_RIGHT_TO_LEFT,
    DWRITE_READING_DIRECTION_LEFT_TO_RIGHT,
    DWRITE_READING_DIRECTION_RIGHT_TO_LEFT,
    DWRITE_READING_DIRECTION_TOP_TO_BOTTOM,
    DWRITE_READING_DIRECTION_BOTTOM_TO_TOP,
    DWRITE_READING_DIRECTION_TOP_TO_BOTTOM,
    DWRITE_READING_DIRECTION_BOTTOM_TO_TOP,
};
const static DWRITE_FLOW_DIRECTION g_dwriteFlowDirectionValues[8] = {
    DWRITE_FLOW_DIRECTION_TOP_TO_BOTTOM,
    DWRITE_FLOW_DIRECTION_TOP_TO_BOTTOM,
    DWRITE_FLOW_DIRECTION_BOTTOM_TO_TOP,
    DWRITE_FLOW_DIRECTION_BOTTOM_TO_TOP,
    DWRITE_FLOW_DIRECTION_LEFT_TO_RIGHT,
    DWRITE_FLOW_DIRECTION_LEFT_TO_RIGHT,
    DWRITE_FLOW_DIRECTION_RIGHT_TO_LEFT,
    DWRITE_FLOW_DIRECTION_RIGHT_TO_LEFT,
};


HRESULT CachedDWriteTextFormat::EnsureCached(IAttributeSource& attributeSource, DrawingCanvas& drawingCanvas)
{
    if (!textFormat.IsNull())
    {
        return S_OK;
    }

    auto* factory = drawingCanvas.GetDWriteFactoryWeakRef();

    array_ref<char16_t const> fontFamilyName = attributeSource.GetString(DrawableObjectAttributeFontFamily);
    array_ref<char16_t const> customFontFilePath = attributeSource.GetString(DrawableObjectAttributeFontFilePath);
    array_ref<char16_t const> languageTag = attributeSource.GetString(DrawableObjectAttributeLanguageList);
    float fontSize = attributeSource.GetValue(DrawableObjectAttributeFontSize, DrawableObject::defaultFontSize);
    float tabWidth = attributeSource.GetValue(DrawableObjectAttributeTabWidth, DrawableObject::defaultTabWidth);

    DWRITE_FONT_WEIGHT fontWeight = attributeSource.GetValue(DrawableObjectAttributeWeight, OUT DWRITE_FONT_WEIGHT_NORMAL);
    DWRITE_FONT_STRETCH fontStretch = attributeSource.GetValue(DrawableObjectAttributeStretch, OUT DWRITE_FONT_STRETCH_NORMAL);
    DWRITE_FONT_STYLE fontStyle = attributeSource.GetValue(DrawableObjectAttributeSlope, OUT DWRITE_FONT_STYLE_NORMAL);
    DWRITE_TEXT_ALIGNMENT columnAlignment = attributeSource.GetValue(DrawableObjectAttributeColumnAlignment, DWRITE_TEXT_ALIGNMENT_LEADING);
    DWRITE_PARAGRAPH_ALIGNMENT rowAlignment = attributeSource.GetValue(DrawableObjectAttributeRowAlignment, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    DrawableObjectLineWrappingMode wrappingMode = attributeSource.GetValue(DrawableObjectAttributeLineWrappingMode, LineWrappingModeWordCharacter);
    DWRITE_WORD_WRAPPING dwriteWrappingMode = static_cast<DWRITE_WORD_WRAPPING>(wrappingMode + 1);
    DrawableObjectJustificationMode justification = attributeSource.GetValue(DrawableObjectAttributeJustification, DrawableObjectJustificationModeUnjustified);
    DrawableObjectTrimmingGranularity trimmingGranularity = attributeSource.GetValue(DrawableObjectAttributeTrimmingGranularity, DrawableObjectTrimmingGranularityNone);
    char32_t trimmingDelimiter = attributeSource.GetValue(DrawableObjectAttributeTrimmingDelimiter, '\0');
    bool hasTrimmingSign = attributeSource.GetValue(DrawableObjectAttributeTrimmingSign, true);

    // Support custom font collection from file path.
    ComPtr<IDWriteFontCollection> fontCollection;
    if (!customFontFilePath.empty() && drawingCanvas.GetSharedResource(customFontFilePath.data(), OUT &fontCollection) == E_NOT_SET)
    {
        if (GetFileAttributes(ToWChar(customFontFilePath.data())) == -1)
            return DWRITE_E_FILENOTFOUND;

        CreateFontCollection(
            factory,
            ToWChar(customFontFilePath.data()),
            static_cast<uint32_t>(customFontFilePath.size()),
            OUT &fontCollection
            );

        drawingCanvas.SetSharedResource(customFontFilePath.data(), fontCollection.Get());
    }

    textFormat.clear();
    IFR(factory->CreateTextFormat(
        ToWChar(fontFamilyName.data()),
        fontCollection, // may be null
        fontWeight,
        fontStyle,
        fontStretch,
        fontSize,
        ToWChar(languageTag.data()), // localeName
        OUT &textFormat
        ));

    uint32_t readingDirection = attributeSource.GetValue(DrawableObjectAttributeReadingDirection, OUT 0ui32);
    textFormat->SetReadingDirection(g_dwriteReadingDirectionValues[readingDirection & 7]);
    textFormat->SetFlowDirection(g_dwriteFlowDirectionValues[readingDirection & 7]);
    textFormat->SetTextAlignment((justification == DrawableObjectJustificationModeUnjustified) ? columnAlignment : DWRITE_TEXT_ALIGNMENT_JUSTIFIED);
    textFormat->SetParagraphAlignment(rowAlignment);
    if (FAILED(textFormat->SetWordWrapping(dwriteWrappingMode)))
    {
        textFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP); // If run on Windows 7, which does not support the newer enums.
    };

    DWRITE_TRIMMING trimming;
    trimming.delimiter = trimmingDelimiter;
    trimming.granularity = static_cast<DWRITE_TRIMMING_GRANULARITY>(trimmingGranularity);
    trimming.delimiterCount = 1;
    ComPtr<IDWriteInlineObject> trimmingSign;
    if (trimming.granularity > DWRITE_TRIMMING_GRANULARITY_WORD)
    {
        trimming.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;
    }
    if (hasTrimmingSign)
    {
        factory->CreateEllipsisTrimmingSign(textFormat, OUT &trimmingSign);
    }
    textFormat->SetTrimming(&trimming, trimmingSign);

    array_ref<float const> tabWidths = attributeSource.GetValues<float>(DrawableObjectAttributeTabWidth);
    if (!tabWidths.empty())
    {
        textFormat->SetIncrementalTabStop(tabWidths[0]);
    }

    ComPtr<IDWriteTextFormat1> textFormat1;
    textFormat->QueryInterface(OUT &textFormat1);

    // There isn't a simple switch to turn off fallback in IDWriteTextFormat,
    // but you can just create an empty font fallback definition.
    bool useFontFallback = attributeSource.GetValue(DrawableObjectAttributeFontFallback, true);
    if (!useFontFallback)
    {
        ComPtr<IDWriteFactory2> factory2;
        ComPtr<IDWriteFontFallbackBuilder> fontFallbackBuilder;
        ComPtr<IDWriteFontFallback> fontFallback;
        if (textFormat1 != nullptr
        &&  SUCCEEDED(factory->QueryInterface(OUT &factory2))
        &&  SUCCEEDED(factory2->CreateFontFallbackBuilder(OUT &fontFallbackBuilder))
        &&  SUCCEEDED(fontFallbackBuilder->CreateFontFallback(OUT &fontFallback)))
        {
            textFormat1->SetFontFallback(fontFallback);
        }
    }

    DWRITE_VERTICAL_GLYPH_ORIENTATION verticalGlyphOrientation = attributeSource.GetValue(DrawableObjectAttributeDWriteVerticalGlyphOrientation, DWRITE_VERTICAL_GLYPH_ORIENTATION_DEFAULT);
    if (textFormat1 != nullptr)
    {
        textFormat1->SetVerticalGlyphOrientation(verticalGlyphOrientation);
    }

    return S_OK;
};


HRESULT CachedDWriteTextLayout::EnsureCached(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    _In_ IDWriteTextFormat* textFormat
    )
{
    if (textFormat == nullptr)
        return E_INVALIDARG;

    if (!textLayout.IsNull())
        return S_OK;

    auto* factory = drawingCanvas.GetDWriteFactoryWeakRef();

    DrawableObjectFunction function = attributeSource.GetValue(DrawableObjectAttributeFunction, DrawableObjectFunctionNop);
    array_ref<char16_t const> text = attributeSource.GetString(DrawableObjectAttributeText);
    float layoutWidth = attributeSource.GetValue(DrawableObjectAttributeWidth, DrawableObject::defaultWidth);
    float layoutHeight = attributeSource.GetValue(DrawableObjectAttributeHeight, DrawableObject::defaultHeight);
    DWRITE_MEASURING_MODE measuringMode = attributeSource.GetValue(DrawableObjectAttributeDWriteMeasuringMode, DWRITE_MEASURING_MODE_NATURAL);
    if (function == DrawableObjectFunctionDirect2DDrawText)
    {
        // Special case for D2D DrawText which ignores the measuring mode,
        // since there is no way for the API to pass it forward.
        measuringMode = DWRITE_MEASURING_MODE_NATURAL;
    }

    textLayout.clear();
    IFR(CreateTextLayout(
        factory,
        ToWChar(text.data()),
        static_cast<uint32_t>(text.size()),
        textFormat,
        layoutWidth,
        layoutHeight,
        measuringMode,
        OUT &textLayout
        ));

    array_ref<uint32_t const> features;
    attributeSource.GetValues(DrawableObjectAttributeTypographicFeatures, OUT features);
    if (!features.empty())
    {
        ComPtr<IDWriteTypography> typography;
        if (SUCCEEDED(factory->CreateTypography(OUT &typography)))
        {
            for (auto const& featureTag : features)
            {
                DWRITE_FONT_FEATURE feature = { DWRITE_FONT_FEATURE_TAG(featureTag), 1 };
                typography->AddFontFeature(feature);
            }

            textLayout->SetTypography(typography, { 0, UINT32_MAX });
        }
    }

    // The IDWriteTextFormat is missing a few setters. So set the other
    // properties after the IDWriteTextLayout is created.
    if (attributeSource.GetValue(DrawableObjectAttributeUnderline, false))
    {
        textLayout->SetUnderline(true, { 0, UINT32_MAX });
    }
    if (attributeSource.GetValue(DrawableObjectAttributeStrikethrough, false))
    {
        textLayout->SetStrikethrough(true, { 0, UINT32_MAX });
    }

    return S_OK;
};


HRESULT DrawableObjectDWriteGlyphRun::GetBounds(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    _Out_ D2D_RECT_F& layoutBounds,
    _Out_ D2D_RECT_F& contentBounds
    )
{
    layoutBounds = emptyRect;

    IFR(fontFace_.Update(attributeSource, drawingCanvas));

    CachedDWriteGlyphRun cachedGlyphRun;
    IFR(cachedGlyphRun.Update(attributeSource, drawingCanvas, fontFace_.fontFace));

    uint32_t readingDirection = attributeSource.GetValue(DrawableObjectAttributeReadingDirection, OUT 0ui32);
    DWRITE_MEASURING_MODE measuringMode = attributeSource.GetValue(DrawableObjectAttributeDWriteMeasuringMode, DWRITE_MEASURING_MODE_NATURAL);

    // Get the font metrics to determine glyph run width & height.
    DWRITE_FONT_METRICS fontMetrics = {};
    GetFontFaceMetrics(
        cachedGlyphRun.fontFace,
        cachedGlyphRun.fontEmSize,
        measuringMode,
        OUT &fontMetrics
        );

    // Calculate content width.
    float width = 0;
    float const* glyphAdvances = cachedGlyphRun.glyphAdvances;
    std::vector<float> glyphAdvancesBuffer(cachedGlyphRun.glyphCount);

    // Get advance widths from font face if none were given.
    if (cachedGlyphRun.glyphAdvances == nullptr)
    {
        IFR(GetFontFaceAdvances(
            cachedGlyphRun.fontFace,
            cachedGlyphRun.fontEmSize,
            { cachedGlyphRun.glyphIndices, cachedGlyphRun.glyphCount },
            measuringMode,
            !!cachedGlyphRun.isSideways,
            OUT glyphAdvancesBuffer
            ));
        glyphAdvances = glyphAdvancesBuffer.data();
    }
    width = std::accumulate(glyphAdvances, glyphAdvances + cachedGlyphRun.glyphCount, 0.0f);

    // Calculate content height.
    int32_t ascent = fontMetrics.ascent;
    int32_t descent = fontMetrics.descent;
    int32_t height = ascent + descent;

    // Split the ascent and descent halfway between the height.
    if (cachedGlyphRun.isSideways) // true for ideographs
    {
        ascent = (height + 1) >> 1; // divide by 2 rounding up
        descent = (height) >> 1; // divide by 2 rounding down
    }

    contentBounds.left = 0;
    contentBounds.right = width;
    contentBounds.top = -ascent * cachedGlyphRun.fontEmSize / fontMetrics.designUnitsPerEm;
    contentBounds.bottom = descent * cachedGlyphRun.fontEmSize / fontMetrics.designUnitsPerEm;;

    if (readingDirection & 1) // Swap for RTL.
    {
        contentBounds.left = -contentBounds.right;
        contentBounds.right = 0;
    }

    return S_OK;
}


HRESULT DrawableObjectDWriteBitmapRenderTargetDrawGlyphRun::Draw(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    float x,
    float y,
    DX_MATRIX_3X2F const& transform
    )
{
    IFR(fontFace_.Update(attributeSource, drawingCanvas));
    IFR(renderingParams_.Update(attributeSource, drawingCanvas));
    CachedDWriteGlyphRun cachedGlyphRun;
    IFR(cachedGlyphRun.Update(attributeSource, drawingCanvas, fontFace_.fontFace));

    uint32_t bgraTextColor = attributeSource.GetValue(DrawableObjectAttributeTextColor, defaultFontColor);
    DWRITE_MEASURING_MODE measuringMode = attributeSource.GetValue(DrawableObjectAttributeDWriteMeasuringMode, DWRITE_MEASURING_MODE_NATURAL);

    auto* renderTarget = drawingCanvas.GetDWriteBitmapRenderTargetWeakRef();
    renderTarget->SetCurrentTransform(&transform.dwrite);

    uint32_t colorPaletteIndex = attributeSource.GetValue(DrawableObjectAttributeColorPaletteIndex, 0);
    bool enableColorFonts = attributeSource.GetValue(DrawableObjectAttributeColorFont, true);
    if (!enableColorFonts)
        colorPaletteIndex = 0xFFFFFFFF;

    HRESULT hr = S_OK;
    if (enableColorFonts)
    {
        hr = DrawColorGlyphRun(
            drawingCanvas.GetDWriteFactoryWeakRef(),
            renderTarget,
            cachedGlyphRun,
            transform.dwrite,
            measuringMode,
            x,
            y,
            renderingParams_.renderingParams,
            ToColorRef(bgraTextColor),
            colorPaletteIndex
            );
    }
    else
    {
        hr = renderTarget->DrawGlyphRun(
            x,
            y,
            measuringMode,
            &cachedGlyphRun,
            renderingParams_.renderingParams,
            ToColorRef(bgraTextColor),
            nullptr
            );
    }

    renderTarget->SetCurrentTransform(&DrawableObject::identityTransform.dwrite);

    return hr;
}


HRESULT DrawableObjectDirect2DDrawGlyphRun::Draw(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    float x,
    float y,
    DX_MATRIX_3X2F const& transform
    )
{
    IFR(fontFace_.Update(attributeSource, drawingCanvas));
    IFR(renderingParams_.Update(attributeSource, drawingCanvas));
    CachedDWriteGlyphRun cachedGlyphRun;
    IFR(cachedGlyphRun.Update(attributeSource, drawingCanvas, fontFace_.fontFace));

    // Set color.
    auto* brush = drawingCanvas.GetD2DBrushWeakRef();
    uint32_t bgraTextColor = attributeSource.GetValue(DrawableObjectAttributeTextColor, defaultFontColor);
    brush->SetColor(ToD2DColor(bgraTextColor));

    DWRITE_MEASURING_MODE measuringMode = attributeSource.GetValue(DrawableObjectAttributeDWriteMeasuringMode, DWRITE_MEASURING_MODE_NATURAL);
    uint32_t colorPaletteIndex = attributeSource.GetValue(DrawableObjectAttributeColorPaletteIndex, 0);
    bool enableColorFonts = attributeSource.GetValue(DrawableObjectAttributeColorFont, true);
    if (!enableColorFonts)
        colorPaletteIndex = 0xFFFFFFFF;

    auto* d2dRenderTarget = drawingCanvas.GetD2DRenderTargetWeakRef();
    d2dRenderTarget->SetTextRenderingParams(renderingParams_.renderingParams);
    d2dRenderTarget->SetTransform(&transform.d2d);
    d2dRenderTarget->BeginDraw();

    // hack::::
    #if 0
    DWRITE_FONT_METRICS fontMetrics;
    std::vector<DWRITE_GLYPH_METRICS> glyphMetricsArray(cachedGlyphRun.glyphCount);
    fontFace_.fontFace->GetMetrics(OUT &fontMetrics);
    fontFace_.fontFace->GetDesignGlyphMetrics(
        cachedGlyphRun.glyphIndices,
        cachedGlyphRun.glyphCount,
        OUT glyphMetricsArray.data(),
        FALSE
    );

    float gx = 0;

    for (uint32_t i = 0, ci = cachedGlyphRun.glyphCount; i < ci; ++i)
    {
        DWRITE_GLYPH_METRICS gm = glyphMetricsArray[i];

        DWritExGlyphMetrics glyphMetrics;

        glyphMetrics.Set(gm);
        D2D_RECT_F rect = {
            float(glyphMetrics.left),
            float(glyphMetrics.top),
            float(glyphMetrics.right),
            float(glyphMetrics.bottom)
        };
        float scaleFactor = cachedGlyphRun.fontEmSize / fontMetrics.designUnitsPerEm;
        rect.left   *= scaleFactor;
        rect.top    *= scaleFactor;
        rect.right  *= scaleFactor;
        rect.bottom *= scaleFactor;
        rect.left   += x + gx;
        rect.top    += y;
        rect.right  += x + gx;
        rect.bottom += y;

        brush->SetColor(ToD2DColor(0xFFFFFFFF));
        d2dRenderTarget->DrawRectangle(&rect, brush);
        brush->SetColor(ToD2DColor(bgraTextColor));

        int32_t advance = cachedGlyphRun.isSideways ? gm.advanceHeight : gm.advanceWidth;
        if (cachedGlyphRun.bidiLevel & 1)
            advance = -advance;
        gx += advance * scaleFactor;
    }
    #endif

    if (enableColorFonts)
    {
        DrawColorGlyphRun(
            drawingCanvas.GetDWriteFactoryWeakRef(),
            d2dRenderTarget,
            cachedGlyphRun,
            transform.dwrite,
            measuringMode,
            x,
            y,
            brush,
            colorPaletteIndex
            );
    }
    else
    {
        d2dRenderTarget->DrawGlyphRun(
            { x, y },
            &cachedGlyphRun,
            brush,
            measuringMode
            );
    }

    d2dRenderTarget->EndDraw();
    d2dRenderTarget->SetTransform(&DrawableObject::identityTransform.d2d);

    return S_OK;
}


HRESULT DrawableObjectDirect2DDrawColorBitmapGlyphRun::Draw(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    float x,
    float y,
    DX_MATRIX_3X2F const& transform
    )
{
    ////////////////////
    // Get the glyph run.
    IFR(fontFace_.Update(attributeSource, drawingCanvas));
    CachedDWriteGlyphRun cachedGlyphRun;
    IFR(cachedGlyphRun.Update(attributeSource, drawingCanvas, fontFace_.fontFace));
    if (cachedGlyphRun.glyphIndices == nullptr)
        return S_OK;

    cachedGlyphRun.GetGlyphAdvancesIfNull();
    cachedGlyphRun.fontEmSize = std::max(cachedGlyphRun.fontEmSize, 0.01f);

    ComPtr<IDWriteFontFace4> fontFace4;
    IFR(fontFace_.fontFace->QueryInterface(OUT &fontFace4));

    ComPtr<IDWriteFactory4> dwriteFactory4;
    IFR(drawingCanvas.GetDWriteFactoryWeakRef()->QueryInterface(OUT &dwriteFactory4));

    DWRITE_MEASURING_MODE measuringMode = attributeSource.GetValue(DrawableObjectAttributeDWriteMeasuringMode, DWRITE_MEASURING_MODE_NATURAL);

    auto* d2dRenderTarget = drawingCanvas.GetD2DRenderTargetWeakRef();
    auto* wicFactory = drawingCanvas.GetWicFactoryWeakRef();
    DWRITE_GLYPH_IMAGE_FORMATS actualGlyphDataFormat = DWRITE_GLYPH_IMAGE_FORMATS_NONE;

    auto glyphImageFormats = fontFace4->GetGlyphImageFormats();

    // Prioritize by quality.
    if (glyphImageFormats & DWRITE_GLYPH_IMAGE_FORMATS_PREMULTIPLIED_B8G8R8A8)
        actualGlyphDataFormat = DWRITE_GLYPH_IMAGE_FORMATS_PREMULTIPLIED_B8G8R8A8;
    else if (glyphImageFormats & DWRITE_GLYPH_IMAGE_FORMATS_PNG)
        actualGlyphDataFormat = DWRITE_GLYPH_IMAGE_FORMATS_PNG;
    else if (glyphImageFormats & DWRITE_GLYPH_IMAGE_FORMATS_TIFF)
        actualGlyphDataFormat = DWRITE_GLYPH_IMAGE_FORMATS_TIFF;
    else if (glyphImageFormats & DWRITE_GLYPH_IMAGE_FORMATS_JPEG)
        actualGlyphDataFormat = DWRITE_GLYPH_IMAGE_FORMATS_JPEG;

    ComPtr<ID2D1DeviceContext4> d2dRenderTarget4;
    IFR(d2dRenderTarget->QueryInterface(OUT &d2dRenderTarget4));

    d2dRenderTarget->SetTransform(&transform.d2d);
    d2dRenderTarget->BeginDraw();

    //d2dRenderTarget4->SetDpi(96*2, 96*2);
    d2dRenderTarget4->DrawColorBitmapGlyphRun(
        actualGlyphDataFormat,
        { x, y },
        &cachedGlyphRun,
        measuringMode,
        D2D1_COLOR_BITMAP_GLYPH_SNAP_OPTION_DEFAULT
        );
    //d2dRenderTarget4->SetDpi(96, 96);

    d2dRenderTarget->EndDraw();
    d2dRenderTarget->SetTransform(&DrawableObject::identityTransform.d2d);

    return S_OK;
}


HRESULT DrawableObjectDirect2DDrawSvgGlyphRun::Draw(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    float x,
    float y,
    DX_MATRIX_3X2F const& transform
    )
{
    ////////////////////
    // Get the glyph run.
    IFR(fontFace_.Update(attributeSource, drawingCanvas));
    CachedDWriteGlyphRun cachedGlyphRun;
    IFR(cachedGlyphRun.Update(attributeSource, drawingCanvas, fontFace_.fontFace));
    if (cachedGlyphRun.glyphIndices == nullptr)
        return S_OK;

    ComPtr<IDWriteFactory4> dwriteFactory4;
    ComPtr<IDWriteFontFace4> fontFace4;
    ComPtr<ID2D1DeviceContext4> d2dRenderTarget4;
    IFR(fontFace_.fontFace->QueryInterface(OUT &fontFace4));
    IFR(drawingCanvas.GetDWriteFactoryWeakRef()->QueryInterface(OUT &dwriteFactory4));

    DWRITE_GLYPH_IMAGE_FORMATS glyphImageFormats = fontFace4->GetGlyphImageFormats();
    if (!(glyphImageFormats & DWRITE_GLYPH_IMAGE_FORMATS_SVG))
        return S_OK;

    cachedGlyphRun.GetGlyphAdvancesIfNull();
    cachedGlyphRun.fontEmSize = std::max(cachedGlyphRun.fontEmSize, 0.01f);

    DWRITE_MEASURING_MODE measuringMode = attributeSource.GetValue(DrawableObjectAttributeDWriteMeasuringMode, DWRITE_MEASURING_MODE_NATURAL);
    uint32_t colorPaletteIndex = attributeSource.GetValue(DrawableObjectAttributeColorPaletteIndex, 0);

    auto* d2dRenderTarget = drawingCanvas.GetD2DRenderTargetWeakRef();
    IFR(d2dRenderTarget->QueryInterface(OUT &d2dRenderTarget4));
    d2dRenderTarget->SetTransform(&transform.d2d);
    d2dRenderTarget->BeginDraw();

    //d2dRenderTarget4->SetDpi(96*2, 96*2);
    d2dRenderTarget4->DrawSvgGlyphRun(
        { x, y },
        &cachedGlyphRun,
        nullptr,
        nullptr,
        colorPaletteIndex,
        measuringMode
        );
    //d2dRenderTarget4->SetDpi(96, 96);

    d2dRenderTarget->EndDraw();
    d2dRenderTarget->SetTransform(&DrawableObject::identityTransform.d2d);

    return S_OK;
}


HRESULT DrawableObjectDWriteTextLayout::Update(
    IAttributeSource& attributeSource
    )
{
    // Clear cached values.
    textFormat_.Invalidate();
    textLayout_.Invalidate();
    return S_OK;
}


HRESULT DrawableObjectDWriteTextLayout::GetBounds(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    _Out_ D2D_RECT_F& layoutBounds,
    _Out_ D2D_RECT_F& contentBounds
    )
{
    contentBounds = emptyRect;
    float layoutWidth = attributeSource.GetValue(DrawableObjectAttributeWidth, DrawableObject::defaultWidth);
    float layoutHeight = attributeSource.GetValue(DrawableObjectAttributeHeight, DrawableObject::defaultHeight);
    layoutBounds = {0, 0, layoutWidth, layoutHeight};

    IFR(textFormat_.EnsureCached(attributeSource, drawingCanvas));
    IFR(textLayout_.EnsureCached(attributeSource, drawingCanvas, textFormat_.textFormat));

    DWRITE_TEXT_METRICS1 textMetrics = {};
    ComPtr<IDWriteTextLayout2> textLayout2;
    if (SUCCEEDED(textLayout_.textLayout->QueryInterface(OUT &textLayout2)))
    {
        IFR(textLayout2->GetMetrics(OUT &textMetrics));
    }
    else
    {
        // Pre-Windows 8.1 did not support vertical text.
        IFR(textLayout_.textLayout->GetMetrics(OUT &textMetrics));
        textMetrics.heightIncludingTrailingWhitespace = textMetrics.height;
    }
    
    contentBounds.left = textMetrics.left;
    contentBounds.top = textMetrics.top;
    contentBounds.right = textMetrics.left + textMetrics.widthIncludingTrailingWhitespace;
    contentBounds.bottom = textMetrics.top + textMetrics.heightIncludingTrailingWhitespace;

    return S_OK;
}


HRESULT DrawableObjectDWriteBitmapRenderTargetLayoutDraw::Draw(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    float x,
    float y,
    DX_MATRIX_3X2F const& transform
    )
{
    IFR(textFormat_.EnsureCached(attributeSource, drawingCanvas));
    IFR(textLayout_.EnsureCached(attributeSource, drawingCanvas, textFormat_.textFormat));
    IFR(renderingParams_.Update(attributeSource, drawingCanvas));

    // Set color.
    uint32_t bgraTextColor = attributeSource.GetValue(DrawableObjectAttributeTextColor, defaultFontColor);
    uint32_t colorPaletteIndex = attributeSource.GetValue(DrawableObjectAttributeColorPaletteIndex, 0);
    bool enablePixelSnapping = attributeSource.GetValue(DrawableObjectAttributePixelSnapping, true);
    bool enableColorFonts = attributeSource.GetValue(DrawableObjectAttributeColorFont, true);
    if (!enableColorFonts)
        colorPaletteIndex = 0xFFFFFFFF;

    auto* renderTarget = drawingCanvas.GetDWriteBitmapRenderTargetWeakRef();
    renderTarget->SetCurrentTransform(&transform.dwrite);

    auto hr = DrawTextLayout(
        drawingCanvas.GetDWriteFactoryWeakRef(),
        renderTarget,
        renderingParams_.renderingParams,
        textLayout_.textLayout,
        x,
        y,
        ToColorRef(bgraTextColor),
        colorPaletteIndex,
        enablePixelSnapping
        );

    renderTarget->SetCurrentTransform(&DrawableObject::identityTransform.dwrite);

    return hr;
}


DrawableObjectDirect2DDrawText::DrawableObjectDirect2DDrawText(bool shouldCreateTextLayout)
    :   shouldCreateTextLayout_(shouldCreateTextLayout)
{
}


HRESULT DrawableObjectDirect2DDrawText::Draw(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    float x,
    float y,
    DX_MATRIX_3X2F const& transform
    )
{
    IFR(textFormat_.EnsureCached(attributeSource, drawingCanvas));
    IFR(renderingParams_.Update(attributeSource, drawingCanvas));

    // Set color.
    auto* brush = drawingCanvas.GetD2DBrushWeakRef();
    uint32_t bgraTextColor = attributeSource.GetValue(DrawableObjectAttributeTextColor, defaultFontColor);
    brush->SetColor(ToD2DColor(bgraTextColor));

    if (shouldCreateTextLayout_)
    {
        IFR(textLayout_.EnsureCached(attributeSource, drawingCanvas, textFormat_.textFormat));
    }

    bool enableColorFonts = attributeSource.GetValue(DrawableObjectAttributeColorFont, true);
    bool enablePixelSnapping = attributeSource.GetValue(DrawableObjectAttributePixelSnapping, true);
    bool isClipped = attributeSource.GetValue(DrawableObjectAttributeClipping, false);

    auto* d2dRenderTarget = drawingCanvas.GetD2DRenderTargetWeakRef();
    ComPtr<ID2D1DeviceContext> deviceContext;
    d2dRenderTarget->QueryInterface(OUT &deviceContext);

    D2D1_DRAW_TEXT_OPTIONS drawTextOptions = D2D1_DRAW_TEXT_OPTIONS_NONE;
    if (enableColorFonts && deviceContext != nullptr /* Windows 8.1 added device context and color support*/)
    {
        drawTextOptions |= D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT;
    }
    if (!enablePixelSnapping)
    {
        drawTextOptions |= D2D1_DRAW_TEXT_OPTIONS_NO_SNAP;
    }
    if (isClipped)
    {
        drawTextOptions |= D2D1_DRAW_TEXT_OPTIONS_CLIP;
    }

    d2dRenderTarget->SetTextRenderingParams(renderingParams_.renderingParams);
    d2dRenderTarget->SetTransform(&transform.d2d);
    d2dRenderTarget->BeginDraw();

    if (shouldCreateTextLayout_)
    {
        D2D1_POINT_2F point = { x, y };
        d2dRenderTarget->DrawTextLayout(
            point,
            textLayout_.textLayout,
            brush,
            drawTextOptions
            );
    }
    else
    {
        // Determine size.
        float layoutWidth = attributeSource.GetValue(DrawableObjectAttributeWidth, defaultWidth);
        float layoutHeight = attributeSource.GetValue(DrawableObjectAttributeHeight, defaultHeight);
        D2D_RECT_F layoutRect = { x, y, x + layoutWidth, y + layoutHeight };

        array_ref<char16_t const> text = attributeSource.GetString(DrawableObjectAttributeText);
        DWRITE_MEASURING_MODE measuringMode = attributeSource.GetValue(DrawableObjectAttributeDWriteMeasuringMode, DWRITE_MEASURING_MODE_NATURAL);

        d2dRenderTarget->DrawText(
            ToWChar(text.data()),
            static_cast<uint32_t>(text.size()),
            textFormat_.textFormat,
            &layoutRect,
            brush,
            drawTextOptions,
            measuringMode
            );
    }

    d2dRenderTarget->EndDraw();
    d2dRenderTarget->SetTransform(&DrawableObject::identityTransform.d2d);

    return S_OK;
}


DEFINE_ENUM_FLAG_OPERATORS(Gdiplus::StringFormatFlags);
DEFINE_ENUM_FLAG_OPERATORS(Gdiplus::FontStyle);
DEFINE_ENUM_FLAG_OPERATORS(Gdiplus::DriverStringOptions);


HRESULT MapGdiPlusStatusToHResult(Gdiplus::Status status)
{
    switch (status)
    {
    case Gdiplus::Ok: return S_OK;
    // Gdiplus::GenericError
    case Gdiplus::InvalidParameter: return E_INVALIDARG;
    case Gdiplus::OutOfMemory: return E_OUTOFMEMORY;
    case Gdiplus::ObjectBusy: return E_PENDING;
    case Gdiplus::InsufficientBuffer: return E_NOT_SUFFICIENT_BUFFER;
    case Gdiplus::NotImplemented: return E_NOTIMPL;
    // Gdiplus::WrongState
    case Gdiplus::Aborted: return E_ABORT;
    case Gdiplus::FileNotFound: return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    case Gdiplus::ValueOverflow: return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);
    case Gdiplus::AccessDenied: return E_ACCESSDENIED;
    case Gdiplus::UnknownImageFormat: return HRESULT_FROM_WIN32(ERROR_INVALID_PIXEL_FORMAT);
    // Gdiplus::FontFamilyNotFound
    // Gdiplus::FontStyleNotFound
    // Gdiplus::NotTrueTypeFont
    // Gdiplus::UnsupportedGdiplusVersion
    case Gdiplus::GdiplusNotInitialized: return E_NOT_VALID_STATE;
    // Gdiplus::PropertyNotFound
    // Gdiplus::PropertyNotSupported
    // Gdiplus::ProfileNotFound
    case Gdiplus::Win32Error:
        {
            auto error = GetLastError();
            if (error != ERROR_SUCCESS)
                return HRESULT_FROM_WIN32(error);
        }
        __fallthrough;

    default: return E_FAIL;
    }
}


HRESULT CachedGdiPlusStartup::EnsureCached()
{
    if (gdiplusToken == NULL)
    {
        Gdiplus::GdiplusStartupInputEx gdiplusStartupInput(Gdiplus::GdiplusStartupDefault);
        auto status = Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
        if (gdiplusToken == NULL)
        {
            return MapGdiPlusStatusToHResult(status);
        }
    }
    return S_OK;
}


void CachedGdiPlusStartup::Invalidate()
{
    gdiplusToken.Clear();
}


HRESULT CachedGdiPlusStringFormat::EnsureCached(IAttributeSource& attributeSource, DrawingCanvas& drawingCanvas)
{
    if (!stringFormat.empty())
        return S_OK;

    uint32_t readingDirection = attributeSource.GetValue(DrawableObjectAttributeReadingDirection, 0ui32);
    DrawableObjectAlignmentMode columnAlignment = attributeSource.GetValue(DrawableObjectAttributeColumnAlignment, DrawableObjectAlignmentModeLeading);
    DrawableObjectAlignmentMode rowAlignment = attributeSource.GetValue(DrawableObjectAttributeRowAlignment, DrawableObjectAlignmentModeLeading);
    DrawableObjectLineWrappingMode wrappingMode = attributeSource.GetValue(DrawableObjectAttributeLineWrappingMode, LineWrappingModeWordCharacter);
    DrawableObjectHotkeyMode hotkeyMode = attributeSource.GetValue(DrawableObjectAttributeHotkeyMode, DrawableObjectHotkeyModeNone);
    DrawableObjectTrimmingGranularity trimmingGranularity = attributeSource.GetValue(DrawableObjectAttributeTrimmingGranularity, DrawableObjectTrimmingGranularityNone);
    char32_t trimmingDelimiter = attributeSource.GetValue(DrawableObjectAttributeTrimmingDelimiter, '\0');
    bool hasTrimmingSign = attributeSource.GetValue(DrawableObjectAttributeTrimmingSign, true);
    bool isClipped = attributeSource.GetValue(DrawableObjectAttributeClipping, false);
    bool useFontFallback = attributeSource.GetValue(DrawableObjectAttributeFontFallback, true);

    Gdiplus::StringFormatFlags stringFormatFlags = Gdiplus::StringFormatFlagsMeasureTrailingSpaces;

    if (wrappingMode == LineWrappingModeNone) stringFormatFlags |= Gdiplus::StringFormatFlagsNoWrap;
    if (!isClipped) stringFormatFlags |= Gdiplus::StringFormatFlagsNoClip;

    if (readingDirection & 4) // vertical
    {
        stringFormatFlags |= Gdiplus::StringFormatFlagsDirectionVertical;
        if (readingDirection & 2) stringFormatFlags |= Gdiplus::StringFormatFlagsDirectionRightToLeft;
    }
    else // horizontal
    {
        if (readingDirection & 1) stringFormatFlags |= Gdiplus::StringFormatFlagsDirectionRightToLeft; // RTL/BTT
    }
    if (!useFontFallback)
    {
        stringFormatFlags |= Gdiplus::StringFormatFlagsNoFontFallback;
    }

    Gdiplus::StringAlignment columnStringAlignment =
        (columnAlignment == DrawableObjectAlignmentModeCenter) ? Gdiplus::StringAlignmentCenter :
        (columnAlignment == DrawableObjectAlignmentModeLeading) ? Gdiplus::StringAlignmentNear :
        Gdiplus::StringAlignmentFar;
    Gdiplus::StringAlignment rowStringAlignment =
        (rowAlignment == DrawableObjectAlignmentModeCenter) ? Gdiplus::StringAlignmentCenter :
        (rowAlignment == DrawableObjectAlignmentModeLeading) ? Gdiplus::StringAlignmentNear :
        Gdiplus::StringAlignmentFar;

    Gdiplus::StringTrimming stringTrimming = Gdiplus::StringTrimmingNone;
    Gdiplus::HotkeyPrefix hotkeyPrefix = Gdiplus::HotkeyPrefixNone;

    #if 0
    stringFormatFlags |= Gdiplus::StringFormatFlagsNoFitBlackBox;
    stringFormatFlags |= Gdiplus::StringFormatFlagsBypassGDI; // want pure GDI+ to be sure
    if (showHiddenCharacters)
    {
        stringFormatFlags |= Gdiplus::StringFormatFlagsDisplayFormatControl;
    }
    #endif

    // Map our trimming to GDI+, at least as closely as possible.
    switch (trimmingGranularity)
    {
    case DrawableObjectTrimmingGranularityNone:
        stringTrimming = Gdiplus::StringTrimmingNone;
        break;

    case DrawableObjectTrimmingGranularityCharacter:
    case DrawableObjectTrimmingGranularityPartialGlyph:
        stringTrimming = hasTrimmingSign ? Gdiplus::StringTrimmingEllipsisCharacter : Gdiplus::StringTrimmingCharacter;
        break;

    case DrawableObjectTrimmingGranularityWord:
        stringTrimming = hasTrimmingSign ? Gdiplus::StringTrimmingEllipsisWord : Gdiplus::StringTrimmingWord;
        break;
    }
    // Note GDI+ intertwined granularity with ellipsis :/ and lacks path trimming without ellipsis.
    if (trimmingGranularity != DrawableObjectTrimmingGranularityNone && trimmingDelimiter == '\\')
        stringTrimming = Gdiplus::StringTrimmingEllipsisPath;

    switch (hotkeyMode)
    {
    default:
    case DrawableObjectHotkeyModeNone: hotkeyPrefix = Gdiplus::HotkeyPrefixNone; break;
    case DrawableObjectHotkeyModeShow: hotkeyPrefix = Gdiplus::HotkeyPrefixShow; break;
    case DrawableObjectHotkeyModeHide: hotkeyPrefix = Gdiplus::HotkeyPrefixHide; break;
    }

    // We can't just assign the new value because GDI+ obnoxiously hides its
    // assigment operator. So, emplace it directly.
    stringFormat.emplace(Gdiplus::StringFormat::GenericTypographic()); // Gdiplus::StringFormat::GenericDefault()
    stringFormat->SetFormatFlags(stringFormatFlags);
    stringFormat->SetHotkeyPrefix(hotkeyPrefix);
    stringFormat->SetTrimming(stringTrimming);
    stringFormat->SetAlignment(columnStringAlignment);
    stringFormat->SetLineAlignment(rowStringAlignment);

    // Note there is unfortunately no way to change the margin or tracking that GenericDefault applies,
    // nor is there a way to set the language and also use GenericTypographic.

    array_ref<float const> tabWidths = attributeSource.GetValues<float>(DrawableObjectAttributeTabWidth);
    if (!tabWidths.empty())
    {
        stringFormat->SetTabStops(0, int(tabWidths.size()), tabWidths.data());
    }

    return S_OK;
};


HRESULT CachedGdiPlusFont::EnsureCached(IAttributeSource& attributeSource, DrawingCanvas& drawingCanvas, bool isDriverString)
{
    if (!font.empty())
        return S_OK;

    bool hasUnderline = attributeSource.GetValue(DrawableObjectAttributeUnderline, false);
    bool hasStrikethrough = attributeSource.GetValue(DrawableObjectAttributeStrikethrough, false);
    float fontSize = attributeSource.GetValue(DrawableObjectAttributeFontSize, DrawableObject::defaultFontSize);
    array_ref<char16_t const> familyName = attributeSource.GetString(DrawableObjectAttributeFontFamily);
    DWRITE_FONT_WEIGHT fontWeight = attributeSource.GetValue(DrawableObjectAttributeWeight, DWRITE_FONT_WEIGHT_NORMAL);
    DWRITE_FONT_STYLE fontSlope = attributeSource.GetValue(DrawableObjectAttributeSlope, DWRITE_FONT_STYLE_NORMAL);

    Gdiplus::FontStyle fontStyle = Gdiplus::FontStyleRegular;
    if (!isDriverString)
    {
        // DrawDriverString fails to draw anything if either of these are set.
        if (hasUnderline)                       fontStyle |= Gdiplus::FontStyleUnderline;
        if (hasStrikethrough)                   fontStyle |= Gdiplus::FontStyleStrikeout;
    }
    if (fontWeight >= DWRITE_FONT_WEIGHT_BOLD)  fontStyle |= Gdiplus::FontStyleBold;
    if (fontSlope != DWRITE_FONT_STYLE_NORMAL)  fontStyle |= Gdiplus::FontStyleItalic;

    fontFamily.emplace(ToWChar(familyName.data()));
    font.emplace(&*fontFamily, fontSize, fontStyle, Gdiplus::UnitPixel);

    return MapGdiPlusStatusToHResult(font->GetLastStatus());
};


HRESULT DrawableObjectGdiPlusDrawString::Update(IAttributeSource& attributeSource)
{
    cachedStringFormat_.Invalidate();
    cachedFont_.Invalidate();
    return S_OK;
}


HRESULT DrawableObjectGdiPlusDrawString::GetBounds(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    _Out_ D2D_RECT_F& layoutBounds,
    _Out_ D2D_RECT_F& contentBounds
    )
{
    contentBounds = emptyRect;
    float layoutWidth = attributeSource.GetValue(DrawableObjectAttributeWidth, DrawableObject::defaultWidth);
    float layoutHeight = attributeSource.GetValue(DrawableObjectAttributeHeight, DrawableObject::defaultHeight);
    layoutBounds = {0, 0, layoutWidth, layoutHeight};

    IFR(cachedStartup_.EnsureCached());
    IFR(cachedStringFormat_.EnsureCached(attributeSource, drawingCanvas));
    IFR(cachedFont_.EnsureCached(attributeSource, drawingCanvas, /*isDriverString*/false));

    array_ref<char16_t const> text = attributeSource.GetString(DrawableObjectAttributeText);

    Gdiplus::Graphics gdiPlusGraphics(drawingCanvas.GetHDC());

    // Calculate the text size.
    Gdiplus::RectF rectIn(0,0, layoutWidth, layoutHeight);
    Gdiplus::RectF rectOut;
    Gdiplus::Status status = gdiPlusGraphics.MeasureString(
        ToWChar(text.data()),
        int(text.size()),
        &cachedFont_.font.value(),
        rectIn,
        &cachedStringFormat_.stringFormat.value(),
        OUT &rectOut
        );
    
    contentBounds = { rectOut.X, rectOut.Y, rectOut.X + rectOut.Width, rectOut.Y + rectOut.Height };

    return MapGdiPlusStatusToHResult(status);
}


HRESULT DrawableObjectGdiPlusDrawString::Draw(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    float x,
    float y,
    DX_MATRIX_3X2F const& transform
    )
{
    IFR(cachedStartup_.EnsureCached());
    IFR(cachedStringFormat_.EnsureCached(attributeSource, drawingCanvas));
    IFR(cachedFont_.EnsureCached(attributeSource, drawingCanvas, /*isDriverString*/false));

    uint32_t bgraTextColor = attributeSource.GetValue(DrawableObjectAttributeTextColor, defaultFontColor);
    Gdiplus::SolidBrush solidBrush(bgraTextColor);

    Gdiplus::Graphics gdiPlusGraphics(drawingCanvas.GetHDC());

    Gdiplus::TextRenderingHint textRenderingHint = attributeSource.GetValue(DrawableObjectAttributeGdiPlusRenderingMode, Gdiplus::TextRenderingHintSystemDefault);
    gdiPlusGraphics.SetTextRenderingHint(textRenderingHint);

    float layoutWidth = attributeSource.GetValue(DrawableObjectAttributeWidth, DrawableObject::defaultWidth);
    float layoutHeight = attributeSource.GetValue(DrawableObjectAttributeHeight, DrawableObject::defaultHeight);
    array_ref<char16_t const> text = attributeSource.GetString(DrawableObjectAttributeText);

    Gdiplus::Matrix gdiPlusTransform(
        transform.xx, transform.xy,
        transform.yx, transform.yy,
        transform.dx, transform.dy
        );
    gdiPlusGraphics.SetTransform(&gdiPlusTransform);

    Gdiplus::RectF gdipLayoutRect = {x, y, layoutWidth, layoutHeight};
    Gdiplus::Status status = gdiPlusGraphics.DrawString(
        ToWChar(text.data()),
        int(text.size()),
        &cachedFont_.font.value(),
        gdipLayoutRect,
        &cachedStringFormat_.stringFormat.value(),
        &solidBrush
        );

    gdiPlusGraphics.ResetTransform();

    return MapGdiPlusStatusToHResult(status);
}


HRESULT DrawableObjectGdiPlusDrawDriverString::Update(IAttributeSource& attributeSource)
{
    cachedFont_.Invalidate();
    return S_OK;
}


HRESULT DrawableObjectGdiPlusDrawDriverString::GetBounds(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    _Out_ D2D_RECT_F& layoutBounds,
    _Out_ D2D_RECT_F& contentBounds
    )
{
    // We can't get the actual bounds from GDI+. So estimate it.

    layoutBounds = emptyRect;
    IFR(cachedStartup_.EnsureCached());
    IFR(cachedFont_.EnsureCached(attributeSource, drawingCanvas, /*isDriverString*/true));

    uint32_t readingDirection = attributeSource.GetValue(DrawableObjectAttributeReadingDirection, 0ui32);

    auto const& font = cachedFont_.font.value();
    Gdiplus::FontStyle fontStyle = Gdiplus::FontStyle(font.GetStyle());
    float fontSize = font.GetSize();

    auto const& fontFamily = cachedFont_.fontFamily.value();
    int32_t ascent  = fontFamily.GetCellAscent(fontStyle);
    int32_t descent = fontFamily.GetCellDescent(fontStyle);
    int32_t emSize  = fontFamily.GetEmHeight(fontStyle);

    if (readingDirection & 4) // vertical
    {
        int32_t height = ascent + descent; // Split the ascent and descent halfway between the height.
        ascent = (height + 1) >> 1; // divide by 2 rounding up
        descent = (height) >> 1; // divide by 2 rounding down
        float layoutHeight = attributeSource.GetValue(DrawableObjectAttributeHeight, DrawableObject::defaultHeight);
        contentBounds = { -ascent * fontSize / emSize, 0, descent * fontSize / emSize, layoutHeight };
    }
    else // horizontal
    {
        // Unfortunately we don't really know the width. So just use the layout width.
        float layoutWidth = attributeSource.GetValue(DrawableObjectAttributeWidth, DrawableObject::defaultWidth);
        contentBounds = { 0, -ascent * fontSize / emSize, layoutWidth, descent * fontSize / emSize };
    }

    return S_OK;
}


HRESULT DrawableObjectGdiPlusDrawDriverString::Draw(
    IAttributeSource& attributeSource,
    DrawingCanvas& drawingCanvas,
    float x,
    float y,
    DX_MATRIX_3X2F const& transform
    )
{
    IFR(cachedStartup_.EnsureCached());
    IFR(cachedFont_.EnsureCached(attributeSource, drawingCanvas, /*isDriverString*/true));

    uint32_t bgraTextColor = attributeSource.GetValue(DrawableObjectAttributeTextColor, defaultFontColor);
    uint32_t readingDirection = attributeSource.GetValue(DrawableObjectAttributeReadingDirection, 0ui32);
    array_ref<char16_t const> text = attributeSource.GetString(DrawableObjectAttributeText);
    array_ref<uint16_t const> glyphs = attributeSource.GetValues<uint16_t>(DrawableObjectAttributeGlyphs);

    Gdiplus::DriverStringOptions drawDriverStringFlags = Gdiplus::DriverStringOptionsRealizedAdvance;
    if (readingDirection & 4)
    {
        drawDriverStringFlags |= Gdiplus::DriverStringOptionsVertical;
    }
    if (glyphs.empty() && attributeSource.GetString(DrawableObjectAttributeGlyphs).empty())
    {
        drawDriverStringFlags |= Gdiplus::DriverStringOptionsCmapLookup;
        glyphs.reset(text.reinterpret_as<uint16_t const>());
        // todo::: Convert and pass glyph advances if non-empty.
    }

    Gdiplus::SolidBrush solidBrush(bgraTextColor);
    Gdiplus::PointF origin(x, y);

    Gdiplus::Graphics gdiPlusGraphics(drawingCanvas.GetHDC());

    Gdiplus::Matrix gdiPlusTransform(
        transform.xx, transform.xy,
        transform.yx, transform.yy,
        transform.dx, transform.dy
        );
    gdiPlusGraphics.SetTransform(&gdiPlusTransform);

    gdiPlusGraphics.DrawDriverString(
        glyphs.data(),
        int(glyphs.size()),
        &cachedFont_.font.value(),
        &solidBrush,
        &origin,
        drawDriverStringFlags,
        nullptr // transform
        );

    gdiPlusGraphics.ResetTransform();

    return S_OK;
}

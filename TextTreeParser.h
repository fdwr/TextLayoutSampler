//+---------------------------------------------------------------------------
//
//  Contents:   Generic text tree parser
//
//  Author:     Dwayne Robinson
//
//  History:    2013-08-29   dwayner    Created
//
//----------------------------------------------------------------------------
#pragma once


class TextTreeParser;


// Tree of text nodes, applicable most heirarchical text file formats
// (such as JSON, XML, INI, BibTex, CSV, Boost PropertyTree INFO).
//
// Limitations:
//      Combined text of all names and attributes cannot exceed 4GB (which
//      implies too that all individuals name and values are < 4GB).
class TextTree
{
    friend TextTreeParser;

public:
    enum Syntax
    {
        SyntaxUnknown,
        SyntaxJsonex, // Read/write support
        SyntaxWindowsInitialization, // Read support
        SyntaxXml, // Write support
        SyntaxBibTeX, // No support
        SyntaxCommaSeparatedValue, // No support
        SyntaxBibTex, // No support
    };

    struct Node
    {
        // What type the parsed node is.
        //
        // When reading a node, it's better to be mostly agnostic about the specific syntax
        // details. This means it does not matter whether you read JSON or XML, and it
        // removes the annoying ambiguity in XML where you may encounter files that either
        // use elements or attributes.
        //
        // When writing, the detail may matter more if you do not want to lose information.
        enum Type : uint32_t
        {
            TypeGenericMask     = 0xFFFFFFF0,   // Mask off the detail

            TypeNone            = 0x00000000,   // Invalid node or empty. (XmlNodeType_None)

            // Single value with no children (like JSON and XML attributes)
            TypeValue           = 0x00000010,   // Generic value type
            TypeText,                           // Document text (XML XmlNodeType_Text XmlNodeType_Whitespace)
            TypeString,                         // String value (XML attribute value, JSON string value)
            TypeNumber,                         // Unquoted number
            TypeData,                           // Data dump (XML CDATA XmlNodeType_CDATA)
            TypeIdentifier,                     // Unquoted identifier or keyword (true, false, null, or other)

            // Named or anonymous key with 0..many children keys or values.
            TypeKey             = 0x00000020,   // Generic key type
            TypeRoot,                           // Only the root node has this value.
            TypeElement,                        // Element/key name (XML XmlNodeType_Element/XmlNodeType_EndElement).
            TypeAttribute,                      // Attribute of an element, containing a single value (XML XmlNodeType_Attribute,JSON). The phar in <a phar="lap"> or {phar:"lap"}.
            TypeFunction,                       // Function call (JSON). The rgb in "rgb(12,34,56)".
            TypeArray,                          // Array (JSON). The phar in phar[].
            TypeObject,                         // Object (JSON). The phar in phar{}.
            TypeSection,                        // Section divider (INI). [Windows]

            TypeComment         = 0x00000030,   // Generic comment type (XmlNodeType_Comment)

            TypeIgnorable       = 0x00000040,   // Generic ignorable type: XmlNodeType_DocumentType
            TypeDirective,                      // Out of band information (XML XmlNodeType_ProcessingInstruction). <!DOCTYPE html>
            TypeDeclaration,                    // XmlNodeType_XmlDeclaration <?xml version='1.0'?>
        };

        Type type;                      // Type of node.
        uint32_t start;                 // Starting character offset.
        uint32_t length;                // Character count of identifier/value/data.
        uint32_t level;                 // Nesting level. The first node is zero.

        inline Type GetGenericType() const noexcept;

        // To get the text, the node forwards the request onto the text tree,
        // as node itself does not actually hold the text. The function here
        // is mainly for discoverability.
        inline void GetText(const TextTree& textTree, OUT std::u16string& text) const { textTree.GetText(*this, OUT text); };
    };

    enum AdvanceNodeDirection : uint32_t
    {
        // todo: Add pure increment/decrement in pre-order.
        AdvanceNodeDirectionSibling,            // Move between nodes at the same level (use positive or negative count to change direction)
        AdvanceNodeDirectionLineage,            // Move between nodes that are heirarchically connected, child/parent (use positive or negative count to change direction)
        AdvanceNodeDirectionSiblingEnd,         // Move to next sibling or end
        AdvanceNodeDirectionLineageEnd,         // Move to next parent/child or end
        AdvanceNodeDirectionSiblingNext,        // Next sibling at the same level (equivalent to AdvanceNodeDirectionSibling with positive count)
        AdvanceNodeDirectionSiblingPrevious,    // Previous sibling at the same level (equivalent to AdvanceNodeDirectionSibling with negative count)
        AdvanceNodeDirectionLineageChild,       // First child of current node (equivalent to AdvanceNodeDirectionLineage with positive count)
        AdvanceNodeDirectionLineageParent,      // Parent of current node (equivalent to AdvanceNodeDirectionLineage with negative count)
        AdvanceNodeDirectionSiblingNextEnd,     // Next sibling at the same level or to the end
        AdvanceNodeDirectionSiblingPreviousEnd, // Previous sibling at the same level or to the end
        AdvanceNodeDirectionLineageChildEnd,    // First child of current node or just after innermost parent (similar to AdvanceNodeDirectionLineage with positive count)
        AdvanceNodeDirectionLineageParentEnd,   // Parent of current node or outermost parent (similar to AdvanceNodeDirectionLineage with negative count)
        AdvanceNodeDirectionMask = 0x0000000F,  // Mask of the just the meaningful direction part
        AdvanceNodeDirectionNull = 0x00000080,  // No direction possible
    };

    class NodePointer : public std::input_iterator_tag
    {
        TextTree& textTree_;
        uint32_t nodeIndex_ = 0;
        AdvanceNodeDirection nodeDirection_ = AdvanceNodeDirectionSibling;

    public:
        NodePointer(TextTree& textTree, uint32_t nodeIndex = 0, AdvanceNodeDirection nodeDirection = AdvanceNodeDirectionSiblingNextEnd);
        NodePointer(const NodePointer& other);

        NodePointer begin();
        NodePointer end();

        bool IsValid() const; // Whether the pointer is valid. If invalid, other calls will return empty or default results.
        void MarkInvalid();

        NodePointer operator[](__in_z const char16_t* keyName) const;
        NodePointer Find( // Search for the given text from the pointer.
            __in_ecount(textLength) char16_t const* text,
            uint32_t textLength,
            bool shouldSearchImmediateChildren = true // Search immediate children rather than siblings of this pointer.
            ) const;

        std::u16string GetText() const; // Returns empty string if it does not exist.
        std::u16string GetSubvalue() const; // Returns empty string if it does not exist.
        std::u16string GetSubvalue(const std::u16string& defaultText) const; // Supply a default value if the value is not found.
        std::u16string GetSubvalue(__in_ecount(defaultTextLength) const char16_t* defaultText, uint32_t defaultTextLength) const;

        NodePointer AppendChild(
            TextTree::Node::Type type,
            __in_ecount(textLength) char16_t const* text,
            uint32_t textLength
            );

        bool SetKeyValue(
            __in_z char16_t const* keyName,
            __in_ecount(valueTextLength) const char16_t* valueText,
            uint32_t valueTextLength
            );

        bool operator==(const NodePointer& other) const;
        inline bool operator!=(const NodePointer& other) const {return !operator==(other);}
        Node& operator*() const; // It's an assertion error to call operator* if IsValid() == false.
        NodePointer& operator++(); // It's okay to increment beyond the last valid item, using STL style end (but that iterator must not be dereferenced then).
        NodePointer& operator--(); // It's okay to increment beyond the last valid item, using STL style end (but that iterator must not be dereferenced then).
        NodePointer operator++(int);
        NodePointer operator--(int);
        NodePointer& operator+=(int32_t nodeCount);
        NodePointer& operator-=(int32_t nodeCount);
    };

    // Functions as an input iterator. It is not default constructible.
    // http://www.cplusplus.com/reference/iterator/
    using iterator = NodePointer;

public:
    void Clear();
    uint32_t GetNodeCount() const noexcept;
    bool empty() const noexcept; // Node there exists a virtual root. So after calling ReadNodes, it will be non-empty even if the file was empty.
    iterator begin();   // Iterator warks entire tree, top down (pre-order).
    iterator end();
    iterator BeginFirstChild(); // You cannot deference this iterator, but you can use it to search from.

    // Returns reference that remains valid until the tree is modified.
    Node& GetNode(uint32_t nodeIndex);
    const Node& GetNode(uint32_t nodeIndex) const;

    // Reads the text of a single node, returning a weak pointer that remains
    // valid until the tree is modified. It is not nul-terminated!
    const char16_t* GetText(const Node& node, __out uint32_t& textLength) const noexcept;

    // Reads the text of a single node into the string.
    // The node must be one from this tree, retrieved via GetNode.
    void GetText(const Node& node, OUT std::u16string& text) const;

    // Gets the text at the given index.
    void GetText(uint32_t nodeIndex, OUT std::u16string& text) const;

    // Set the text.
    // The node must be one from this tree, retrieved via GetNode.
    void SetText(__inout Node& node, const std::u16string& text);

    void SetText(__inout Node& node, __in_ecount(textLength) const char16_t* text, uint32_t textLength);

    // Advances from one node to another given the direction and count.
    // A negative count reverses direction. For example, moving Next with a
    // negative value will move Previous. Moving in (Child) with a negative
    // direction will actually move out (Parent).
    //
    // Returns:
    //    - true if it was able to advance, with matchingNodeIndex updated.
    //    - false if it reaches the end of the list in either direction before
    //      satisfying the requested node count. If it made no progress at all,
    //      nodeIndex remains unchanged, else it returns the last node it was
    //      able to advance to.
    bool AdvanceNode(AdvanceNodeDirection direction, int32_t nodeCount, __inout uint32_t& matchingNodeIndex) const;

    // Shorter helpers.
    bool AdvanceNextNode(__inout uint32_t& matchingNodeIndex) const;
    bool AdvancePreviousNode(__inout uint32_t& matchingNodeIndex) const;
    bool AdvanceChildNode(__inout uint32_t& matchingNodeIndex) const; // Advance to first child from given node.
    bool AdvanceParentNode(__inout uint32_t& matchingNodeIndex) const; // Advance to parent of given node.
    bool AdvanceLastSiblingNode(__inout uint32_t& matchingNodeIndex) const;
    bool AdvanceFirstSiblingNode(__inout uint32_t& matchingNodeIndex) const;
    bool AdvanceIntoEmptyNode(__inout uint32_t& matchingNodeIndex) const;

    // Appends a node to the very end of the tree, with an explicit heirarchy level. Usually you should use Insert instead.
    void Append(TextTree::Node::Type type, uint32_t level, __in_ecount(textLength) char16_t const* text, uint32_t textLength);

    // Insert a new node relative to the given one.
    bool Insert(
        uint32_t nodeIndex,
        bool insertAfter, // insert directly after nodeIndex, else before. If nodeIndexIsParent true, insert after the last child.
        bool nodeIndexIsParent, // start looking from first child of nodeIndex
        TextTree::Node::Type type,
        __in_ecount(textLength) char16_t const* text,
        uint32_t textLength,
        __out uint32_t& newNodeIndex
        );

    // Insert a new child node under the parent. This is equivalent to calling
    // Insert with insertAfter and nodeIndexIsParent both true.
    bool AppendChild(
        uint32_t parentNodeIndex,
        TextTree::Node::Type type,
        __in_ecount(textLength) char16_t const* text,
        uint32_t textLength,
        __out uint32_t& newNodeIndex
        );

    // Delete the node at the index and any children.
    // If shouldRemove is true, remove the node. Otherwise just nullify
    // it in-place (TypeNone).
    bool Delete(uint32_t nodeIndex, bool shouldRemove);

    // Find the node matching the given text (case insensitive).
    // If more than one exists, the first match is returned.
    // The search is not recursive (only at siblings or immediate
    // children if firstNodeIndexIsParent) and stops at the last sibling.
    // This is most efficient at leaf branches. 
    //
    // Returns:
    //    - true if a match was found, with matchingNodeIndex updated.
    //    - false if no match was found, matchingNodeIndex remaining unchanged.
    bool Find(
        uint32_t firstNodeIndex,
        bool firstNodeIndexIsParent, // start looking from first child of firstNodeIndex rather than firstNodeIndex.
        __in_ecount(textLength) char16_t const* text,
        uint32_t textLength,
        TextTree::Node::Type expectedType, // may be None if irrelevant
        __out uint32_t& matchingNodeIndex // remains unchanged if no match
        ) const;

    // Find the node matching the given key name.
    // Returns false if not found, or if the node is not a key.
    bool FindKey(
        uint32_t parentNodeIndex, // 0 is the root.
        __in_z char16_t const* keyName,
        __out uint32_t& matchingKeyNodeIndex // remains unchanged if no match
        ) const;

    // Reads a single value under a key into the string.
    // Returns:
    //    - true if the node had a single subvalue, with 'text' initialized.
    //    - false if the node had no values/multiple subvalues, with 'text' empty.
    //
    // True:    Key:"value"
    //          Key="value"
    //          Key=""
    //          Key(value)
    //          <A Key="value"/>
    //          <Key>value</Key>
    //          Key = {value}       // BibTeX
    //          Key: // hello       // JSONEX
    //               "value"
    //
    // False:   Key(value,horse)
    //          Key()
    //          Key:[value1,value2].
    //          <Key Value> // such as <input checked> attribute minimization in HTML 4
    //
    // The node index passed is that of the key, not the value.
    bool GetSingleSubvalue(
        uint32_t keyNodeIndex,
        OUT std::u16string& text
        ) const;

    // Get the value of a named key, starting from firstNodeIndex (a sibling at the same level).
    //
    // Returns:
    //    - true if a match was found, with 'text' initialized.
    //    - false if no match, or the node had no values/multiple subvalues, with 'text' empty.
    //
    bool GetKeyValue(
        uint32_t parentNodeIndex, // 0 is the root.
        __in_z char16_t const* keyName,
        OUT std::u16string& text
        ) const;

    // Set the key, either finding the existing one or creating a new key.
    // If more than one named key exists, return the index of the first one.
    // Matching is case insensitive.
    // Callers typically call SetKeyValue instead for individual pairs.
    bool SetKey(
        uint32_t parentNodeIndex, // 0 is the root.
        __in_z char16_t const* keyName,
        TextTree::Node::Type type, // TextTree::Node::TypeAttribute
        __inout uint32_t& keyNodeIndex // unchanged if not created
        );

    // Set the value of the given key by index.
    bool SetSubvalue(
        uint32_t keyNodeIndex,
        __in_ecount(valueTextLength) const char16_t* valueText,
        uint32_t valueTextLength,
        TextTree::Node::Type type = TextTree::Node::TypeValue
        );

    // Set the value of the given key by index. Overload for wstring.
    bool SetSubvalue(
        uint32_t keyNodeIndex,
        const std::u16string& valueText,
        TextTree::Node::Type type = TextTree::Node::TypeValue
        );

    // Set a key:value pair under the parent index.
    // Replace the key's value if it already exists.
    // Add it if not.
    bool SetKeyValue(
        uint32_t parentNodeIndex, // 0 is the root.
        __in_z char16_t const* keyName,
        const std::u16string& valueText
        );

    // Set a key:value pair under the parent index.
    // Replace the key's value if it already exists.
    // Add it if not.
    bool SetKeyValue(
        uint32_t parentNodeIndex, // 0 is the root.
        __in_z char16_t const* keyName,
        __in_ecount(valueTextLength) const char16_t* valueText,
        uint32_t valueTextLength
        );

    // Set a key:value pair under the parent index.
    // Overload for the common integer value.
    bool SetKeyValue(
        uint32_t parentNodeIndex, // 0 is the root.
        __in_z char16_t const* keyName,
        uint32_t value
        );

    // The root node is empty. This is common for JSON files which have an empty {} or [] containing all the nodes.
    bool SkipEmptyNodes(__inout uint32_t& nodeIndex) const;
    bool SkipRootNode(__inout uint32_t& nodeIndex) const;

private:
    std::vector<Node> nodes_;
    std::u16string nodesText_;  // Holds decoded text for cases for numeric codes: \u03A3 or &#931; or &#x03A3.
};


class TextTreeParser
{
    // Base class, supporting either tree or iterative pull approaches.
    // The parser is forward-only and non-caching.
public:
    enum Options
    {
        OptionsDefault              = 0x00000000,
        /*
        OptionsUnquotedKeys         = 0x00000001,   // (JSON) Allow unquoted keys if purely alphanumeric ASCII. So phar:"lap" is legal instead of needing "phar":"lap".
        OptionsUnquotedValues       = 0x00000002,   // (JSON) Allow unquoted values if numbers or purely alphanumeric ASCII. So axis:top is legal instead of needing axis:"top".
        OptionsNoRedundantColons    = 0x00000004,   // (JSON) Allow no colons between the identifier and opening brace. So object{} is legal instead of needing object:{}.
        */
        OptionsDiscardPureWhitespace= 0x00000008,   // (XML) Ignore spans of pure whitespace (space and CR/LF).
        OptionsNoEscapeSequence     = 0x00000010,   // (JSON) Allow unquoted keys if purely alphanumeric ASCII. So phar:"lap" is legal instead of needing "phar":"lap".
    };

    struct Error
    {
        uint32_t errorTextIndex;
        const char16_t* errorMessage;    // Weak pointer to static text data.
    };

public:
    static TextTree::Syntax DetermineType(
        __in_ecount(textLength) const char16_t* text,
        uint32_t textLength
        );

    TextTreeParser();

    // Initialize a parser using the given text.
    // The text pointer must remain valid for the lifetime of the class (or
    // until Reset), because it does not make a copy of the data.
    TextTreeParser(
        __in_ecount(textLength) const char16_t* text,
        uint32_t textLength,
        Options options
        );

    // Accepts STL containers and raw C arrays with contiguous memory and
    // begin()/end() functions, including:
    //    
    //      wstring, vector<char16_t>, initializer_list<char16_t>, char16_t[n]
    //
    template<typename ContiguousSequenceContainer>
    inline TextTreeParser(const ContiguousSequenceContainer& text, Options options)
        :   TextTreeParser(&(*std::begin(text)), static_cast<uint32_t>(std::end(text) - std::begin(text)), options)
    {}

    // Reinitialize the parser with new data/options. This clears any state
    // from previous parsing operations, including errors.
    // The text pointer must remain valid for the lifetime of the class (or
    // until Reset), because it does not make a copy of the data.
    void Reset(
        __in_ecount(textLength) const char16_t* text,
        uint32_t textLength,
        Options options
        );

    // Accepts STL containers and raw C arrays with contiguous memory and
    // begin()/end() functions, including:
    //    
    //      wstring, vector<char16_t>, initializer_list<char16_t>, char16_t[n]
    //
    template<typename ContiguousSequenceContainer>
    inline void Reset(const ContiguousSequenceContainer& text, Options options)
    {
        Reset(&(*std::begin(text)), static_cast<uint32_t>(std::end(text) - std::begin(text)), options);
    }

    // Read a single node, extracting type and text from it.
    virtual bool ReadNode(
        __out TextTree::Node& node,
        __inout std::u16string& nodeText
        );

    // Reads the entire string into the text tree's nodes.
    bool ReadNodes(__inout TextTree& textTree);

    // Get the level (depth) of the current node.
    //
    // Given <Parent><Child></Child></Parent>
    // When positioned at:
    //      [virtual root node]   the level is 0.
    //      <Parent>  start node, the level is 1.
    //      <Child>   start node, the level is 2.
    //      </Child>  end node,   the level is 3.
    //      </Parent> end node,   the level is 2.
    //      end of the string,    the level is 1.
    uint32_t GetCurrentLevel();

    // The parser has read all the way to the end. The parser is no longer
    // useful, unless you Reset to a new source.
    bool IsAtEnd();

    uint32_t GetErrorCount();
    void GetErrorDetails(uint32_t errorIndex, __out uint32_t& errorTextIndex, __out const char16_t** userErrorMessage);

protected:
    static void AppendCharacter(
        __inout std::u16string& nodeText,
        char32_t ch
        );

    void AdvanceCodeUnit();
    void RecedeCodeUnit();
    char32_t PeekCodeUnit();
    char32_t PeekCodeUnit(int32_t delta);
    char32_t ReadCodeUnit();
    char32_t ReadCStyleEscapeCharacter(); // Implemented directly here since common enough to be shared.

    void ReportError(uint32_t errorTextIndex, const char16_t* userErrorMessage);

protected:
    virtual void ResetDerived();

protected:
    __field_ecount_opt(textLength_) char16_t const* text_ = nullptr;   // Weak pointer should be valid for lifetime of the parsing operation (or until Reset).
    uint32_t textLength_ = 0;
    uint32_t textIndex_ = 0; // Current read index
    uint32_t treeLevel_ = 0; // Current heirarchy level
    Options options_ = OptionsDefault;
    std::vector<Error> errors_;
};


class JsonexParser : public TextTreeParser
{
    //  The parser reads pure JSON with the following relaxations for brevity/convenience:
    //  - unquoted strings for both keys and values if purely alphanumeric ASCII.
    //    mandatory quoting of keys in object literals is just silly.
    //  - redundant comma between values is optional when whitespace is sufficient.
    //  - redundant colon between key and values is optional when no whitespace.
    //  - nested colon scope. some:command:value instead of some:{command:{value}}.
    //  - named closure is allowed to find the matching scope for readers.
    //  - trailing comma is fine.
    //
    //  It advances in a single forward pass without backtracking, and needs to
    //  look ahead at most two tokens to determine a node's type. Tokenization is
    //  context free.
    //
    //      Key              = KeyName : Key                            // So "a:b:c:d" is legal rather than needing "a:{b:{c:d}}"
    //                       | KeyName : Element<NoWS>Closure
    //                       | KeyName<NoWS>NestedElement<NoWS>Closure  // "a{}" is one named object, but "a {}" is a value followed by an unnamed object.
    //      NestedElement    = Array | Object | Function
    //      Object           = { Elements }
    //      Array            = [ Elements ]
    //      Function         = ( Elements )
    //      Closure          = <empty>
    //      Closure          | /Value
    //      Elements         = <empty>
    //                       | Element
    //                       | Element ElementSeparator                 // Trailing comma is intentionally legal
    //                       | Element ElementSeparator Elements
    //      NestedElement    = Array | Object | Function
    //      Element          = Value | Array | Object | Function
    //      Value            = String | UnquotedString
    //      ElementSeparator = Whitespace | Comma
    //      KeyName          = String | UnquotedString
    //      String           = ""
    //                       | " Characters "
    //      UnquotedString   = AlphanumericCharacters
    //
    //  The following nodes may be returned.
    //      Object      - { Elements }
    //                  - Value{ Elements }
    //                  - Value : { Elements }
    //      Array       - [ Elements ]
    //                  - Value[ Elements ]
    //                  - Value : [ Elements ]
    //      Function    - ( Elements )
    //                  - Value( Elements )
    //                  - Value : ( Elements )
    //      Attribute   - KeyName : Value
    //      Value       - Value

    using Base = TextTreeParser;

public:
    JsonexParser() {}

    JsonexParser(
        __in_ecount(textLength) const char16_t* text,
        uint32_t textLength,
        Options options
        );

    template<typename ContiguousSequenceContainer>
    inline JsonexParser(const ContiguousSequenceContainer& text, Options options)
        :   JsonexParser(&(*std::begin(text)), static_cast<uint32_t>(std::end(text) - std::begin(text)), options)
    {}

    virtual bool ReadNode(
        __out TextTree::Node& node,
        __inout std::u16string& nodeText
        );

protected:
    bool ReadWord(
        __out TextTree::Node& node,
        __inout std::u16string& nodeText
        );

    void CheckClosingTag(
        __in const TextTree::Node& node,
        __in const std::u16string& nodeText
        );

    bool SkipSpacesAndLineBreaks();

    void SkipComment();

    void SkipInvalidWordCharacters();

    void ClearAttributeOnStack();

    virtual void ResetDerived();

protected:
    std::vector<TextTree::Node> nodeStack_;
};


class IniParser : public TextTreeParser
{
    // Sample INI file:
    //
    // ; This is a sample configuration file
    // # Comments may also start with a hash.
    // 
    // key : value
    //
    // [first_section]
    // one = 1
    // five = 5
    // animal = BIRD
    // 
    // [second_section]
    // path = "/usr/local/bin"
    // URL = "http://www.example.com/~username"
    // 
    // [third_section]
    // phpversion[] = "5.0"
    // phpversion[] = "5.1"

    using Base = TextTreeParser;

public:
    IniParser();

    IniParser(
        __in_ecount(textLength) const char16_t* text, // Pointer should be valid for the lifetime of the class.
        uint32_t textLength,
        Options options
        );

    template<typename ContiguousSequenceContainer>
    inline IniParser(const ContiguousSequenceContainer& text, Options options)
        :   IniParser(&(*std::begin(text)), static_cast<uint32_t>(std::end(text) - std::begin(text)), options)
    { }

    virtual bool ReadNode(
        __out TextTree::Node& node,
        __inout std::u16string& nodeText
        );

    bool ReadWord(
        TextTree::Node::Type expectedType,
        __out TextTree::Node& node,
        __inout std::u16string& nodeText
        );

protected:
    void InitializeDerived();
    void ResetDerived();
    void SkipSpaces();
    void SkipSpacesAndLineBreaks();

protected:
    uint32_t sectionLevel_ = 0; // Start out of level 0 for key:value pairs until reaching a section.
};


class TextTreeWriter // Base class
{
public:
    enum Options
    {
        OptionsDefault              = TextTreeParser::OptionsDefault,
        /*
        OptionsUnquotedKeys         = TextTreeParser::OptionsUnquotedKeys,      // (JSON) Quote all keys. Keys are usually not quoted if purely alphanumeric ASCII.
        OptionsUnquotedValues       = TextTreeParser::OptionsUnquotedValues,    // (JSON) Quote all values. Values are usually not quoted if numbers or purely alphanumeric ASCII.
        OptionsNoRedundantColons    = TextTreeParser::OptionsNoRedundantColons, // (JSON) Quote all values. Values are usually not quoted if numbers or purely alphanumeric ASCII.
        */
        OptionsDiscardPureWhitespace= TextTreeParser::OptionsDiscardPureWhitespace, // (XML/JSON) Strip spans of pure whitespace (space and CR/LF).
        OptionsNoEscapeSequence     = TextTreeParser::OptionsNoEscapeSequence,
    };

public:
    TextTreeWriter(Options options);

    // Write an entire parse tree or parse tree fragment.
    HRESULT WriteNodes(const TextTree& textTree);

    virtual HRESULT WriteNode(
        TextTree::Node::Type type,
        __in_ecount(textLength) const char16_t* text,
        uint32_t textLength
        );

    // Returns a weak pointer of the entire text which remains valid until the tree is modified. It is not nul-terminated!
    const char16_t* GetText(__out uint32_t& textLength) const noexcept;

    array_ref<char16_t const> GetText() const noexcept;

    // Reads the text into the string.
    void GetText(OUT std::u16string& text) const;

    virtual HRESULT EnterNode();

    virtual HRESULT ExitNode();

    bool WantWhitespace();

    static uint32_t GetTextLength(
        __in_ecount(textLength) const char16_t* text,
        uint32_t textLength = 0xFFFFFFFF
        ) noexcept;

protected:
    std::u16string text_; // Starts empty and grows with each written node.
    uint32_t nodeLevel_ = 0; // Current heirarchy level
    Options options_ = OptionsDefault;
};


class JsonexWriter : public TextTreeWriter
{
public:
    using Base = TextTreeWriter;

    JsonexWriter(Options options);

    virtual HRESULT WriteNode(
        TextTree::Node::Type type,
        __in_ecount(textLength) const char16_t* text,
        uint32_t textLength
        );

    virtual HRESULT EnterNode();

    virtual HRESULT ExitNode();

    HRESULT WriteValueString(
        __in_ecount(textLength) const char16_t* text,
        uint32_t textLength = 0xFFFFFFFF
        );

    HRESULT WriteValueNumber(
        __in_ecount(textLength) const char16_t* text,
        uint32_t textLength = 0xFFFFFFFF
        );

    HRESULT BeginKey(
        __in_ecount(textLength) const char16_t* text,
        uint32_t textLength = 0xFFFFFFFF
        );

    HRESULT BeginArray(
        __in_ecount_opt(textLength) const char16_t* text = nullptr,
        uint32_t textLength = 0xFFFFFFFF
        );

    HRESULT BeginObject(
        __in_ecount_opt(textLength) const char16_t* text = nullptr,
        uint32_t textLength = 0xFFFFFFFF
        );

    HRESULT EndScope();

protected:
    const static size_t spacesPerIndent_ = 2;

    TextTree::Node::Type ResolveNodeType(
        TextTree::Node::Type type
        );

    void WriteStringInternal(
        __in_ecount(textLength) const char16_t* text,
        uint32_t textLength
        );

    void WriteCommentInternal(
        __in_ecount(textLength) const char16_t* text,
        uint32_t textLength
        );

    void WriteIndentation();

protected:
    std::u16string spaceBuffer_;
    std::vector<TextTree::Node::Type> nodeTypeStack_;
};



class XmlWriter : public TextTreeWriter
{
public:
    using Base = TextTreeWriter;

    XmlWriter(Options options);

    virtual HRESULT WriteNode(
        TextTree::Node::Type type,
        __in_ecount(textLength) const char16_t* text,
        uint32_t textLength
        );

    virtual HRESULT EnterNode();

    virtual HRESULT ExitNode();

    HRESULT WriteText(
        __in_ecount(textLength) const char16_t* text,
        uint32_t textLength = 0xFFFFFFFF
        );

    HRESULT WriteValueString(
        __in_ecount(textLength) const char16_t* text,
        uint32_t textLength = 0xFFFFFFFF
        );

    HRESULT WriteValueNumber(
        __in_ecount(textLength) const char16_t* text,
        uint32_t textLength = 0xFFFFFFFF
        );

    HRESULT BeginElement(
        __in_ecount(textLength) const char16_t* text,
        uint32_t textLength = 0xFFFFFFFF
        );

    HRESULT BeginAttribute(
        __in_ecount(textLength) const char16_t* text,
        uint32_t textLength = 0xFFFFFFFF
        );

    HRESULT EndScope();

protected:
    const static size_t spacesPerIndent_ = 2;

    TextTree::Node::Type ResolveNodeType(
        TextTree::Node::Type type
        );

    void WriteStringInternal(
        __in_ecount(textLength) const char16_t* text,
        uint32_t textLength,
        TextTree::Node::Type type
        );

protected:
    bool isInsideOpeningTag_;
    TextTree::Node::Type previousType_;
    std::u16string spaceBuffer_;
    std::vector<TextTree::Node> nodeStack_;
};

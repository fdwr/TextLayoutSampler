//----------------------------------------------------------------------------
//
//  Contents:   Generic text tree parser
//
//  Author:     Dwayne Robinson
//
//  History:    2013-08-29   dwayner    Created
//
//----------------------------------------------------------------------------
#include "precomp.h"

import Common.String;
import Common.ArrayRef;

module TextTreeParser;
export
{
    #include "TextTreeParser.h"
}

////////////////////////////////////////

namespace
{
    inline char16_t GetSingleHexDigit(uint32_t value)
    {
        value &= 0xF;
        return static_cast<char16_t>(
            value + (value < 10 ? '0' : 'A' - 10)
            );
    }
}


TextTree::NodePointer::NodePointer(
    TextTree& textTree,
    uint32_t nodeIndex,
    AdvanceNodeDirection nodeDirection
    )
:   textTree_(textTree),
    nodeIndex_(nodeIndex),
    nodeDirection_(nodeDirection)
{}


TextTree::NodePointer::NodePointer(const NodePointer& other)
:   textTree_(other.textTree_),
    nodeIndex_(other.nodeIndex_),
    nodeDirection_(other.nodeDirection_)
{}


bool TextTree::NodePointer::operator==(const NodePointer& other) const
{
    return nodeIndex_ == other.nodeIndex_;
}


bool TextTree::NodePointer::IsValid() const
{
    // Should not be deferencing an invalid iterator.
    return !(nodeDirection_ & AdvanceNodeDirectionNull)
        && nodeIndex_ < textTree_.nodes_.size();
}


void TextTree::NodePointer::MarkInvalid()
{
    nodeDirection_ = AdvanceNodeDirection(nodeDirection_ | AdvanceNodeDirectionNull);
}



TextTree::Node& TextTree::NodePointer::operator*() const
{
    // Should not be deferencing an invalid iterator.
    assert(IsValid());

    return textTree_.GetNode(nodeIndex_);
}


TextTree::NodePointer& TextTree::NodePointer::operator++()
{
    // todo: Check for end? Check for parent index?
    if (!textTree_.AdvanceNode(nodeDirection_, 1, IN OUT nodeIndex_))
    {
        MarkInvalid(); // todo: consider whether to mark as invalid or not, because one could back up via -- after calling this.
    }
    return *this;
}


TextTree::NodePointer& TextTree::NodePointer::operator--()
{
    // todo: Check for end? Check for parent index?
    if (!textTree_.AdvanceNode(nodeDirection_, -1, IN OUT nodeIndex_))
    {
        MarkInvalid();
    }
    return *this;
}


TextTree::NodePointer TextTree::NodePointer::operator++(int)
{
    // todo: Check for end? Check for parent index?
    NodePointer pointer(*this);
    return ++pointer;
}


TextTree::NodePointer TextTree::NodePointer::operator--(int)
{
    NodePointer pointer(*this);
    return --pointer;
}


TextTree::NodePointer& TextTree::NodePointer::operator+=(int32_t nodeCount)
{
    // todo: Check for end? Check for parent index?
    if (!textTree_.AdvanceNode(nodeDirection_, nodeCount, IN OUT nodeIndex_))
    {
        MarkInvalid(); // todo: consider whether to mark as invalid or not, because one could back up via -- after calling this.
    }
    return *this;
}


TextTree::NodePointer& TextTree::NodePointer::operator-=(int32_t nodeCount)
{
    // todo: Check for end? Check for parent index?
    if (!textTree_.AdvanceNode(nodeDirection_, -nodeCount, IN OUT nodeIndex_))
    {
        MarkInvalid();
    }
    return *this;
}


TextTree::NodePointer TextTree::NodePointer::operator[](__in_z const char16_t* keyName) const
{
    return Find(keyName, static_cast<uint32_t>(wcslen(ToWChar(keyName))), /*shouldSearchImmediateChildren*/ true);
}


TextTree::NodePointer TextTree::NodePointer::begin()
{
    // Get the iterator pointing to the first child.
    // If the current node pointer is invalid, propagate that, and return an invalid one.

    NodePointer pointer(textTree_, nodeIndex_);
    if (!IsValid() || !textTree_.AdvanceNode(AdvanceNodeDirectionLineageChild, 1, IN OUT pointer.nodeIndex_))
    {
        pointer.MarkInvalid();
    }
    return pointer;
}


TextTree::NodePointer TextTree::NodePointer::end()
{
    // Get the iterator pointing just after the last child.
    // If the current node pointer is invalid, propagate that, and return an invalid one.

    NodePointer pointer(textTree_, nodeIndex_);
    if (!IsValid()
    ||  !textTree_.AdvanceNode(AdvanceNodeDirectionLineageChild, 1, IN OUT pointer.nodeIndex_) // todo: debate AdvanceNodeDirectionLineageChild and true/false
    ||  !textTree_.AdvanceNode(AdvanceNodeDirectionSiblingNextEnd, INT_MAX, IN OUT pointer.nodeIndex_))
    {
        pointer.MarkInvalid(); // todo: debate whether the call should be false if count not reached
    }

    return pointer;
}


TextTree::NodePointer TextTree::NodePointer::Find(
    __in_ecount(textLength) char16_t const* text,
    uint32_t textLength,
    bool shouldSearchImmediateChildren
    ) const
{
    if (nodeDirection_ & AdvanceNodeDirectionNull)
        return *this; // Already invalid, so just return existing self directly.

    uint32_t matchingNodeIndex = nodeIndex_;

    if (textTree_.Find(
        matchingNodeIndex,
        shouldSearchImmediateChildren, // firstNodeIndexIsParent
        text,
        textLength,
        TextTree::Node::TypeNone, // no specific type
        OUT matchingNodeIndex // remains unchanged if no match
        ))
    {
        // Return match with new node index (and same direction).
        return NodePointer(textTree_, matchingNodeIndex, nodeDirection_);
    }

    // No match found. Return invalid pointer.
    return NodePointer(textTree_, nodeIndex_, AdvanceNodeDirection(nodeDirection_ | AdvanceNodeDirectionNull));
}


std::u16string TextTree::NodePointer::GetText() const
{
    std::u16string text;

    if (IsValid())
    {
        textTree_.GetText(nodeIndex_, IN OUT text);
    }

    return text;
}


std::u16string TextTree::NodePointer::GetSubvalue() const
{
    std::u16string text;

    if (IsValid())
    {
        textTree_.GetSingleSubvalue(nodeIndex_, IN OUT text);
    }

    return text;
}


std::u16string TextTree::NodePointer::GetSubvalue(const std::u16string& defaultText) const
{
    // wstring helper just forwards along.
    return GetSubvalue(defaultText.c_str(), static_cast<uint32_t>(defaultText.size()));
}


std::u16string TextTree::NodePointer::GetSubvalue(__in_ecount(defaultTextLength) const char16_t* defaultText, uint32_t defaultTextLength) const
{
    std::u16string text;

    uint32_t textLength = TextTreeWriter::GetTextLength(defaultText, defaultTextLength);

    if (!IsValid() || !textTree_.GetSingleSubvalue(nodeIndex_, IN OUT text))
    {
        // Value was not found, so return the supplied default.
        text.assign(defaultText, textLength);
    }

    return text;
}



TextTree::NodePointer TextTree::NodePointer::AppendChild(
    TextTree::Node::Type type,
    __in_ecount(textLength) char16_t const* text,
    uint32_t textLength
    )
{
    NodePointer pointer(textTree_, nodeIndex_);
    if (!IsValid()
    ||  !textTree_.AppendChild(nodeIndex_, type, text, textLength, OUT pointer.nodeIndex_))
    {
        pointer.MarkInvalid(); // todo: debate whether the call should be false if count not reached
    }
    return pointer;
}


bool TextTree::NodePointer::SetKeyValue(
    __in_z char16_t const* keyName,
    __in_ecount(valueTextLength) const char16_t* valueText,
    uint32_t valueTextLength
    )
{
    return textTree_.SetKeyValue(nodeIndex_, keyName, valueText, valueTextLength);
}


uint32_t TextTree::GetNodeCount() const noexcept
{
    return static_cast<uint32_t>(nodes_.size());
}


bool TextTree::empty() const noexcept
{
    return nodes_.empty(); // todo: always create empty root.
}


TextTree::iterator TextTree::begin()
{
    return NodePointer(*this);
}


TextTree::iterator TextTree::end()
{
    return NodePointer(*this, GetNodeCount());
}


TextTree::iterator TextTree::BeginFirstChild()
{
    return NodePointer(*this, 1, AdvanceNodeDirectionSiblingNextEnd);
}


void TextTree::Clear()
{
    // Reset the tree and release all memory.
    nodes_.clear();
    nodes_.shrink_to_fit();
    nodesText_.clear();
    nodesText_.shrink_to_fit();
}


TextTree::Node::Type TextTree::Node::GetGenericType() const noexcept
{
    return static_cast<TextTree::Node::Type>(this->type & TypeGenericMask);
}


TextTree::Node& TextTree::GetNode(uint32_t nodeIndex)
{
    assert(nodeIndex < nodes_.size());
    if (nodeIndex > nodes_.size())
    {
        return *static_cast<Node*>(nullptr); // Force AV.
    }
    return nodes_[nodeIndex];
}


const TextTree::Node& TextTree::GetNode(uint32_t nodeIndex) const
{
    assert(nodeIndex < nodes_.size());
    if (nodeIndex > nodes_.size())
    {
        return *static_cast<Node*>(nullptr); // Force AV.
    }
    return nodes_[nodeIndex];
}


const char16_t* TextTree::GetText(const Node& node, __out uint32_t& textLength) const noexcept
{
    assert(size_t(&node - nodes_.data()) < nodes_.size());
    textLength = node.length;
    return nodesText_.data() + node.start;
}


void TextTree::GetText(const Node& node, OUT std::u16string& text) const
{
    assert(size_t(&node - nodes_.data()) < nodes_.size());
    auto textPointer = &nodesText_[node.start];
    text.assign(textPointer, textPointer + node.length);
}


void TextTree::GetText(uint32_t nodeIndex, OUT std::u16string& text) const
{
    auto& node = GetNode(nodeIndex);
    auto textPointer = &nodesText_[node.start];
    text.assign(textPointer, textPointer + node.length);
}


void TextTree::SetText(__inout Node& node, const std::u16string& text)
{
    SetText(node, text.c_str(), static_cast<uint32_t>(text.size()));
}


void TextTree::SetText(__inout Node& node, __in_ecount(textLength) const char16_t* text, uint32_t textLength)
{
    assert(size_t(&node - nodes_.data()) < nodes_.size());
    const uint32_t start  = static_cast<uint32_t>(nodesText_.size());
    nodesText_.append(text, textLength);
    node.start = start;
    node.length = textLength;
}


bool TextTree::AdvanceNode(
    AdvanceNodeDirection direction,
    int32_t nodeCount,
    __inout uint32_t& matchingNodeIndex
    ) const
{
    // Resolve the specific directions into a generic direction with count, negated if needed.
    auto resolvedDirection = direction & AdvanceNodeDirectionMask;
    bool endsAreInclusive = false;
    switch (direction)
    {
    case AdvanceNodeDirectionSibling:               break;
    case AdvanceNodeDirectionLineage:               break;
    case AdvanceNodeDirectionSiblingNext:           resolvedDirection = AdvanceNodeDirectionSibling;                         break;
    case AdvanceNodeDirectionSiblingPrevious:       resolvedDirection = AdvanceNodeDirectionSibling; nodeCount = -nodeCount; break;
    case AdvanceNodeDirectionLineageChild:          resolvedDirection = AdvanceNodeDirectionLineage;                         break;
    case AdvanceNodeDirectionLineageParent:         resolvedDirection = AdvanceNodeDirectionLineage; nodeCount = -nodeCount; break;
    case AdvanceNodeDirectionSiblingNextEnd:        resolvedDirection = AdvanceNodeDirectionSibling;                         endsAreInclusive = true; break;
    case AdvanceNodeDirectionSiblingPreviousEnd:    resolvedDirection = AdvanceNodeDirectionSibling; nodeCount = -nodeCount; endsAreInclusive = true; break;
    case AdvanceNodeDirectionLineageChildEnd:       resolvedDirection = AdvanceNodeDirectionLineage;                         endsAreInclusive = true; break;
    case AdvanceNodeDirectionLineageParentEnd:      resolvedDirection = AdvanceNodeDirectionLineage; nodeCount = -nodeCount; endsAreInclusive = true; break;
    default:                                        return false;
    }

    const auto nodesCount = nodes_.size();
    auto nodeIndex = matchingNodeIndex;
    auto parentNodeLevel = 0u;
    if (nodeIndex < nodesCount)
    {
        parentNodeLevel = GetNode(nodeIndex).level;
    }
    else if (nodeCount > 0)
    {
        return false; // Cannot move past end of list. todo: if endsAreInclusive = true, then allow nodeIndex == nodesCount?
    }
    else // Clamp the count and leave level = 0.
    {
        nodeIndex = static_cast<uint32_t>(nodesCount);
    }

    if (nodeCount == 0)
    {
        return true; // Successfully moved zero nodes.
    }

    if (nodeCount > 0) // Search forward.
    {
        if (nodeIndex < nodesCount)
        {
            nodeIndex++;
            while (nodeIndex < nodesCount)
            {
                // Look for sibling at same level or child at lower level.

                const auto nodeLevel = nodes_[nodeIndex].level;
                if (nodeLevel < parentNodeLevel)
                    break;

                bool isMatch;
                if (resolvedDirection == AdvanceNodeDirectionSibling)
                {
                    isMatch = (nodeLevel == parentNodeLevel);
                }
                else // resolvedDirection == AdvanceNodeDirectionLineage
                {
                    isMatch = (nodeLevel > parentNodeLevel);
                    if (!isMatch)
                    {
                        break;
                    }
                    parentNodeLevel = nodeLevel; // Update to new level.
                }

                if (isMatch)
                {
                    // Count another match, and update the node index.
                    matchingNodeIndex = nodeIndex;
                    if (--nodeCount <= 0)
                    {
                        return true;
                    }
                }
                ++nodeIndex;
            }
        }

        if (endsAreInclusive)
        {
            matchingNodeIndex = nodeIndex;
            return true; // todo: debate whether this should be false if count not reached
        }
    }
    else // Search backward.
    {
        while (nodeIndex > 0)
        {
            --nodeIndex;
            const auto nodeLevel = nodes_[nodeIndex].level;
            bool isMatch;
            if (resolvedDirection == AdvanceNodeDirectionSibling)
            {
                if (nodeLevel < parentNodeLevel)
                    break;
                isMatch = (nodeLevel == parentNodeLevel);
            }
            else // resolvedDirection == AdvanceNodeDirectionLineage
            {
                isMatch = (nodeLevel < parentNodeLevel);
            }

            if (isMatch)
            {
                matchingNodeIndex = nodeIndex;
                if (++nodeCount >= 0)
                {
                    return true;
                }
            }
        }

        if (endsAreInclusive)
        {
            return true; // todo: debate whether this should be false if count not reached
        }
    }

    return false;
}


bool TextTree::AdvanceNextNode(__inout uint32_t& matchingNodeIndex) const
{
    return AdvanceNode(AdvanceNodeDirectionSiblingNext, 1, IN OUT matchingNodeIndex);
}


bool TextTree::AdvancePreviousNode(__inout uint32_t& matchingNodeIndex) const
{
    return AdvanceNode(AdvanceNodeDirectionSiblingPrevious, 1, IN OUT matchingNodeIndex);
}


bool TextTree::AdvanceChildNode(__inout uint32_t& nodeIndex) const
{
    return AdvanceNode(AdvanceNodeDirectionLineageChild, 1, IN OUT nodeIndex);
}


bool TextTree::AdvanceParentNode(__inout uint32_t& nodeIndex) const
{
    return AdvanceNode(AdvanceNodeDirectionLineageParent, 1, IN OUT nodeIndex);
}


bool TextTree::AdvanceLastSiblingNode(__inout uint32_t& matchingNodeIndex) const
{
    AdvanceNode(AdvanceNodeDirectionSiblingNext, INT32_MAX, IN OUT matchingNodeIndex);
    return true; // Ignore whether the count was satisfied.
}


bool TextTree::AdvanceFirstSiblingNode(__inout uint32_t& matchingNodeIndex) const
{
    AdvanceNode(AdvanceNodeDirectionSiblingPrevious, INT32_MAX, IN OUT matchingNodeIndex);
    return true; // Ignore whether the count was satisfied.
}


bool TextTree::AdvanceIntoEmptyNode(__inout uint32_t& matchingNodeIndex) const
{
    if (matchingNodeIndex >= nodes_.size())
        return false;

    // Check the first node to see if it is empty.
    auto& node = nodes_[matchingNodeIndex];
    if (node.length == 0 && node.GetGenericType() == node.TypeKey)
        return AdvanceChildNode(IN OUT matchingNodeIndex);

    return false;
}


bool TextTree::Find(
    uint32_t firstNodeIndex,
    bool firstNodeIndexIsParent, // start looking from first child of firstNodeIndex
    __in_ecount(textLength) char16_t const* text,
    uint32_t textLength,
    TextTree::Node::Type expectedType,
    __out uint32_t& matchingNodeIndex // remains unchanged if no match
    ) const
{
    // Get first child node.
    const auto nodesCount = nodes_.size();
    auto nodeIndex = firstNodeIndex;

    if (firstNodeIndexIsParent && !AdvanceChildNode(IN OUT nodeIndex))
    {
        return false;
    }

    // Search for node with matching text and type.
    // If more than one node exists, return the first match.
    while (nodeIndex < nodesCount)
    {
        uint32_t currentTextLength = 0;
        auto& node = nodes_[nodeIndex];
        auto currentText = GetText(node, OUT currentTextLength);

        if (currentTextLength == textLength && _wcsnicmp(ToWChar(text), ToWChar(currentText), currentTextLength) == 0)
        {
            // Return true if the text matches and it is the expected type (or the type is irrelevant).
            if (expectedType == TextTree::Node::TypeNone
            ||  node.type == expectedType
            ||  node.GetGenericType() == expectedType)
            {
                matchingNodeIndex = nodeIndex;
                return true;
            }
        }

        if (!AdvanceNextNode(IN OUT nodeIndex))
            break;
    }

    return false;
}


bool TextTree::FindKey(
    uint32_t parentNodeIndex,
    __in_z char16_t const* keyName,
    __inout uint32_t& matchingKeyNodeIndex // remains unchanged if no match
    ) const
{
    return Find(
                parentNodeIndex,
                true, // firstNodeIndexIsParent
                keyName,
                static_cast<uint32_t>(wcslen(ToWChar(keyName))),
                TextTree::Node::TypeKey,
                OUT matchingKeyNodeIndex
                );
}


bool TextTree::GetSingleSubvalue(
    uint32_t keyNodeIndex,
    OUT std::u16string& text
    ) const
{
    // Reads the subvalue of a key, expecting exactly one.
    // A missing value or multiple values return false with empty text.

    text.clear();

    const auto nodesCount = nodes_.size();
    auto nodeIndex = keyNodeIndex;
    if (keyNodeIndex >= nodesCount)
        return false;

    auto& keyNode = GetNode(nodeIndex);
    if (keyNode.GetGenericType() != Node::TypeKey)
        return false;

    // Search for value node after key.
    auto keyNodeLevel = keyNode.level;
    for (++nodeIndex; nodeIndex < nodesCount; ++nodeIndex)
    {
        auto& node = GetNode(nodeIndex);
        if (node.level != keyNodeLevel + 1)
            return false; // Following node is not a child, so found no value.

        auto type = node.GetGenericType();
        if (type == node.TypeValue)
            break; // Found the value, but still need to check if there is more than one value.
    }

    // Check if scanned all nodes without finding a value.
    if (nodeIndex >= nodesCount)
        return false;

    // Check that there is exactly one value node for this key.
    for (uint32_t nextNodeIndex = nodeIndex + 1; nextNodeIndex < nodesCount; ++nextNodeIndex)
    {
        auto& node = GetNode(nextNodeIndex);
        if (node.level <= keyNodeLevel)
            break; // Exited the key (the following node is at the same level or above the key), so stop looking for values.

        auto type = node.GetGenericType();
        if (type == node.TypeComment || type == node.TypeIgnorable)
            continue; // Ignore comments and directives.

        // Any other type means that more than one value was found, not a single value.
        return false;
    }

    const Node& valueNode = GetNode(nodeIndex);
    auto textPointer = &nodesText_[valueNode.start];
    text.assign(textPointer, textPointer + valueNode.length);

    return true;
}


bool TextTree::GetKeyValue(
    uint32_t parentNodeIndex,
    __in_z char16_t const* keyName,
    OUT std::u16string& valueText
    ) const
{
    valueText.clear();

    // Resolve keyName to a node index.
    uint32_t keyNodeIndex;
    if (!FindKey(parentNodeIndex, keyName, OUT keyNodeIndex))
        return false;

    return GetSingleSubvalue(keyNodeIndex, OUT valueText);
}


__success(return == true)
bool TextTree::SetKey(
    uint32_t parentNodeIndex,
    __in_z char16_t const* keyName,
    TextTree::Node::Type type,
    OUT uint32_t& keyNodeIndex // unchanged if not set (must have been created or already exist)
    )
{
    // Set the key, either finding the existing one or inserting a new key.

    auto nodeIndex = parentNodeIndex;

    // If key already exists, just return it.
    if (FindKey(parentNodeIndex, keyName, OUT nodeIndex))
    {
        keyNodeIndex = nodeIndex;
        return true;
    }

    return Insert(
                nodeIndex,
                /*insertAfter*/ true,
                /*nodeIndexIsParent*/ true,
                type, // TextTree::Node::TypeAttribute usually
                keyName,
                static_cast<uint32_t>(wcslen(ToWChar(keyName))),
                OUT keyNodeIndex
                );
}


bool TextTree::SetSubvalue(
    uint32_t keyNodeIndex,
    __in_ecount(valueTextLength) const char16_t* valueText,
    uint32_t valueTextLength,
    TextTree::Node::Type type
    )
{
    // Validate index, and that it is actually a key.

    const auto nodesCount = nodes_.size();
    if (keyNodeIndex >= nodesCount)
        return false;

    auto& keyNode = GetNode(keyNodeIndex);
    // Presume the key node is actually a key. It would be weird to set a
    // value underneath something like a comment or another value.
    assert(keyNode.GetGenericType() == Node::TypeKey);

    // Delete existing subvalues.

    const auto firstChildNodeIndex = keyNodeIndex + 1;
    const auto keyNodeLevel = keyNode.level;
    const auto childNodeLevel = keyNodeLevel + 1;
    auto nodeIndex = firstChildNodeIndex;
    for (; nodeIndex < nodesCount; ++nodeIndex)
    {
        auto& node = GetNode(nodeIndex);
        if (node.level <= keyNodeLevel)
            break; // Following node is not a child, so found no value.

        node.type = TextTree::Node::TypeNone;
        node.level = childNodeLevel;
    }

    // Add the new node, either inserting or overwriting the old value.

    TextTree::Node node = {};
    node.start = static_cast<uint32_t>(nodesText_.size());
    node.length = valueTextLength;
    node.type = type;
    node.level = childNodeLevel;
    nodesText_.append(valueText, valueTextLength);

    if (nodeIndex == firstChildNodeIndex)
    {
        nodes_.insert(nodes_.begin() + firstChildNodeIndex, node);
    }
    else
    {
        nodes_[firstChildNodeIndex] = node;
    }

    return true;
}


bool TextTree::SetSubvalue(
    uint32_t keyNodeIndex,
    const std::u16string& valueText,
    TextTree::Node::Type type
    )
{
    return SetSubvalue(keyNodeIndex, valueText.c_str(), static_cast<uint32_t>(valueText.size()), type);
}


bool TextTree::SetKeyValue(
    uint32_t parentNodeIndex,
    __in_z char16_t const* keyName,
    uint32_t value
    )
{
    char16_t valueText[12];
    auto valueTextEnd = valueText + ARRAYSIZE(valueText);
    auto p = valueTextEnd;
    do
    {
        --p;
        *p = (value % 10) + '0';
        value /= 10;
    }
    while (value > 0);

    uint32_t valueTextLength = static_cast<uint32_t>(valueTextEnd - p);

    uint32_t keyNodeIndex;
    return SetKey(parentNodeIndex, keyName, TextTree::Node::TypeAttribute, OUT keyNodeIndex)
        && SetSubvalue(keyNodeIndex, p, valueTextLength, TextTree::Node::TypeNumber);
}


bool TextTree::SetKeyValue(
    uint32_t parentNodeIndex,
    __in_z char16_t const* keyName,
    __in_ecount(valueTextLength) const char16_t* valueText,
    uint32_t valueTextLength
    )
{
    uint32_t keyNodeIndex;
    return SetKey(parentNodeIndex, keyName, TextTree::Node::TypeAttribute, OUT keyNodeIndex)
        && SetSubvalue(keyNodeIndex, valueText, valueTextLength);
}


bool TextTree::SetKeyValue(
    uint32_t parentNodeIndex,
    __in_z char16_t const* keyName,
    const std::u16string& valueText
    )
{
    uint32_t keyNodeIndex;
    return SetKey(parentNodeIndex, keyName, TextTree::Node::TypeAttribute, OUT keyNodeIndex)
        && SetSubvalue(keyNodeIndex, valueText.c_str(), static_cast<uint32_t>(valueText.size()));
}


void TextTree::Append(TextTree::Node::Type type, uint32_t level, __in_ecount(textLength) char16_t const* text, uint32_t textLength)
{
    TextTree::Node node = {};
    node.start = static_cast<uint32_t>(nodesText_.size());
    node.length = textLength;
    node.type = type;
    node.level = level;
    nodesText_.append(text, textLength);
    nodes_.push_back(node);
}


bool TextTree::Delete(uint32_t nodeIndex, bool shouldRemove)
{
    if (nodeIndex >= nodes_.size())
        return false;

    // Children are also deleted, so determine how many to erase.
    auto endIndex = nodeIndex + 1;
    const auto& node = nodes_[nodeIndex];
    const auto keyLevel = node.level;
    while (endIndex < nodes_.size() && nodes_[endIndex].level < keyLevel)
    {
        ++endIndex;
    }

    if (shouldRemove)
    {
        // Actually remove it, and shift everything down.
        nodes_.erase(nodes_.begin() + nodeIndex, nodes_.begin() + endIndex);
    }
    else
    {
        // Just nullify it, leaving it empty in-place.
        for (auto i = nodeIndex; i < endIndex; ++i)
        {
            auto& deletableNode = nodes_[i];
			deletableNode.type = TextTree::Node::TypeNone;
			deletableNode.level = keyLevel;
        }
    }

    return true;
}


bool TextTree::AppendChild(
    uint32_t parentNodeIndex,
    TextTree::Node::Type type,
    __in_ecount(textLength) char16_t const* text,
    uint32_t textLength,
    __out uint32_t& newNodeIndex
    )
{
    return Insert(
        parentNodeIndex,
        /*insertAfter*/true,
        /*nodeIndexIsParent*/true,
        type,
        text,
        textLength,
        OUT newNodeIndex
        );
}


bool TextTree::Insert(
    uint32_t nodeIndex,
    bool insertAfter, // insert directly after nodeIndex, or after the last child of the parent (when nodeIndexIsParent true).
    bool nodeIndexIsParent, // start looking from first child of nodeIndex
    TextTree::Node::Type type,
    __in_ecount(textLength) char16_t const* text,
    uint32_t textLength,
    __out uint32_t& newNodeIndex
    )
{
    newNodeIndex = nodeIndex;

    uint32_t newNodeLevel = 0;
    if (nodeIndex < nodes_.size())
    {
        newNodeLevel = nodes_[nodeIndex].level;
    }

    // todo: check that we are actually using AdvanceNodeDirectionLineageChildEnd correctly.
    auto direction = insertAfter ? AdvanceNodeDirectionSiblingNextEnd : AdvanceNodeDirectionLineageChildEnd;
    if (nodeIndexIsParent)
    {
        // Resolve from parent to first child node index.
        ++newNodeLevel; // Set level to child (parent + 1).
        if (!AdvanceNode(direction, 1, IN OUT nodeIndex))
            return false; // todo: debate whether the call should be false if count not reached
    }
    else
    {
        // Verify nodeIndex validity, and move after node if insertAfter.
        if (!AdvanceNode(direction, insertAfter ? 1 : 0, IN OUT nodeIndex))
            return false; // todo: debate whether the call should be false if count not reached
    }

    TextTree::Node node = {};
    node.start = static_cast<uint32_t>(nodesText_.size());
    node.length = textLength;
    node.type = type;
    node.level = newNodeLevel;
    nodesText_.append(text, textLength);
    nodes_.insert(nodes_.begin() + nodeIndex, node);
    newNodeIndex = nodeIndex;

    return true;
}


bool TextTree::SkipEmptyNodes(__inout uint32_t& nodeIndex) const
{
    // Check the first node to see if it is an empty key.
    uint32_t matchingNodeIndex = nodeIndex;
    uint32_t nodeCount = static_cast<uint32_t>(nodes_.size());
    for (; matchingNodeIndex < nodeCount; ++matchingNodeIndex)
    {
        auto& node = nodes_[matchingNodeIndex];
        if (node.length > 0 || node.GetGenericType() != node.TypeKey)
            break;
    }

    bool nodesWereSkipped = (nodeIndex != matchingNodeIndex);
    nodeIndex = matchingNodeIndex;
    return nodesWereSkipped;
}


bool TextTree::SkipRootNode(__inout uint32_t& nodeIndex) const
{
    // Check the first node to see if it is an empty key.
    uint32_t matchingNodeIndex = nodeIndex;
    uint32_t nodeCount = static_cast<uint32_t>(nodes_.size());
    for (; matchingNodeIndex < nodeCount; ++matchingNodeIndex)
    {
        auto& node = nodes_[matchingNodeIndex];
        if (node.type != node.TypeRoot)
            break;
    }

    bool nodesWereSkipped = (nodeIndex != matchingNodeIndex);
    nodeIndex = matchingNodeIndex;
    return nodesWereSkipped;
}


namespace
{
    // Parse a character of the form \x1234.
    char32_t GetCStyleEscapedNumber(
        __in_ecount(textLength) const char16_t* text,
        uint32_t textIndex,
        uint32_t textLength,
        __out uint32_t& endingTextIndex,
        uint32_t radix = 10
        )
    {
        char32_t value = 0;
        for ( ; textIndex < textLength; ++textIndex)
        {
            char32_t ch = text[textIndex];
            if (ch < '0') break;
            if (ch >= '0' && ch <= '9')
            {
                ch -= '0';
            }
            else if (ch &= ~0x20, ch >= 'A' && ch <= 'Z')
            {
                ch -= 'A' - 10;
            }
            else
            {
                break; // Reached non alpha-numeric character.
            }
            if (ch > radix)
                break; // Reached a number beyond the radix.

            value *= radix;
            value += ch;
        }

        endingTextIndex = textIndex;

        return value;
    }
}


TextTreeParser::TextTreeParser()
{
    Reset(nullptr, 0, OptionsDefault);
}


TextTreeParser::TextTreeParser(
    __in_ecount(textLength) const char16_t* text, // Pointer should be valid for the lifetime of the class.
    uint32_t textLength,
    Options options
    )
{
    Reset(text, textLength, options);
}


void TextTreeParser::Reset(
    __in_ecount(textLength) const char16_t* text, // Pointer should be valid for the lifetime of the class.
    uint32_t textLength,
    Options options
    )
{
    // For compilers that do not support delegated constructors.
    text_       = text;
    textLength_ = textLength;
    options_    = options;
    textIndex_ = 0;
    treeLevel_ = 0;
    if (textLength > 0 && text_[0] == 0xFEFF)
        ++textIndex_; // Skip byte order mark if present.

    errors_.clear();
    ResetDerived();
}


void TextTreeParser::ResetDerived()
{
    // Do nothing in base class. Derived classes may do something.
}


TextTree::Syntax TextTreeParser::DetermineType(
    __in_ecount(textLength) const char16_t* text,
    uint32_t textLength
    )
{
    for (uint32_t i = 0; i < textLength; ++i)
    {
        char16_t ch = text[i];
        if (ch == ' ' || ch == '\r' || ch == '\n') continue; // Skip all common forms of whitespace.
        if (ch == '{') return TextTree::SyntaxJsonex;
        if (ch == '[') return TextTree::SyntaxJsonex;
        if (ch == '<') return TextTree::SyntaxXml;
    }

    return TextTree::SyntaxUnknown;
}


// Generic C-style escape character support, used by JSONex and INI.
char32_t TextTreeParser::ReadCStyleEscapeCharacter()
{
    if (textIndex_ >= textLength_)
        return '\0';

    char32_t ch = text_[textIndex_++];

    switch (ch)
    {
    case L'r':  return L'\r';  // return (JSON C++)
    case L'n':  return L'\n';  // new line (JSON C++)
    case L't':  return L'\t';  // tab (JSON C++)
    case L'v':  return L'\v';  // vertical tab (C++)
    case L'q':  return L'\"';  // quote (C++)
    case L'b':  return L'\b';  // backspace (JSON C++)
    case L'a':  return L'\a';  // audible bell (C++)
    case L'f':  return L'\f';  // form feed (JSON C++)
    case L'\\': return L'\\';  // backslash (C++)
    case L'/':  return L'/';   // forward slash (JSON)
    case L'"':  return L'"';   // double quote (JSON C++)
    case L'\'': return L'\'';  // single quote (C++)
    case L'?':  return L'?';   // question mark (C++)
    case L'0':  return L'\0';  // nul (C++)
    case L'#':  return L'#';   // hash, since may be comment in INI files
    case L';':  return L';';   // semi-colon, since may be comment in INI files
    case L':':  return L':';   // colon, since may be delimiter in INI files
    case L'=':  return L'=';   // equals, since delimiter in INI files

    case L'x':
    case L'u':
        return GetCStyleEscapedNumber(
            text_,
            textIndex_,
            std::min(textLength_, textIndex_ + 4),
            OUT textIndex_,
            16 // hex
            );

    case L'X':
    case L'U':
        return GetCStyleEscapedNumber(
            text_,
            textIndex_,
            std::min(textLength_, textIndex_ + 8),
            OUT textIndex_,
            16 // hex
            );

    default:
        return '?';
    }
}


bool TextTreeParser::ReadNodes(__inout TextTree& textTree)
{
    // Always allocate at least one node for the root.
    TextTree::Node node = {};
    if (textTree.empty())
    {
        node.type = TextTree::Node::TypeRoot;
        textTree.nodes_.push_back(node);
        ++treeLevel_;
    }

    while (ReadNode(OUT node, OUT textTree.nodesText_))
    {
        textTree.nodes_.push_back(node);
        textTree.nodesText_.push_back('\0'); // Add explicit nul just because it makes the life easier of callers later.
    }
    return true;
}


uint32_t TextTreeParser::GetErrorCount()
{
    return static_cast<uint32_t>(errors_.size());
}


void TextTreeParser::GetErrorDetails(uint32_t errorIndex, __out uint32_t& errorTextIndex, __out const char16_t** errorMessage)
{
    errorTextIndex = errors_[errorIndex].errorTextIndex;
    *errorMessage = errors_[errorIndex].errorMessage;
}


void TextTreeParser::ReportError(uint32_t errorTextIndex, const char16_t* errorMessage)
{
    Error error = {errorTextIndex, errorMessage};
    errors_.push_back(error);
}


bool TextTreeParser::ReadNode(
    __out TextTree::Node& node,
    __inout std::u16string& nodeText
    )
{
    return false;
}


uint32_t TextTreeParser::GetCurrentLevel()
{
    return treeLevel_;
}


bool TextTreeParser::IsAtEnd()
{
    return textIndex_ >= textLength_;
}


// Reads a single code unit, not full code point. This is okay because all the
// important functional code points are in the basic plane.
char32_t TextTreeParser::ReadCodeUnit()
{
    if (textIndex_ >= textLength_)
        return '\0';

    return text_[textIndex_++];
}


// Read, but do not advance.
char32_t TextTreeParser::PeekCodeUnit()
{
    if (textIndex_ >= textLength_)
        return '\0';

    return text_[textIndex_];
}


// Read relative position, and do not advance.
char32_t TextTreeParser::PeekCodeUnit(int32_t delta)
{
    uint32_t textIndex = textIndex_ + delta;
    if (delta < 0 && textIndex > textIndex_ // underflow
    ||  delta > 0 && textIndex < textIndex_ // overflow
    ||  textIndex >= textLength_)           // out of bounds
        return '\0';

    return text_[textIndex];
}


void TextTreeParser::AdvanceCodeUnit()
{
    if (textIndex_ >= textLength_)
        return;

    ++textIndex_;
}


void TextTreeParser::RecedeCodeUnit()
{
    if (textIndex_ <= 0)
        return;

    --textIndex_;
}


// Split into leading and trailing surrogates.
// From http://unicode.org/faq/utf_bom.html#35
__if_not_exists(GetLeadingSurrogate)
{
    inline char16_t GetLeadingSurrogate(char32_t ch)
    {
        return char16_t(0xD800 + (ch >> 10) - (0x10000 >> 10));
    }
}


__if_not_exists(GetTrailingSurrogate)
{
    inline char16_t GetTrailingSurrogate(char32_t ch)
    {
        return char16_t(0xDC00 + (ch & 0x3FF));
    }
}


void TextTreeParser::AppendCharacter(
    __inout std::u16string& nodeText,
    char32_t ch
    )
{
    if (ch > 0xFFFF) // If surrogate pair is beyond the basic multilingual plane, split it.
    {
        nodeText.push_back(GetLeadingSurrogate(ch));
        nodeText.push_back(GetTrailingSurrogate(ch));
    }
    else
    {
        nodeText.push_back(char16_t(ch));
    }
}


JsonexParser::JsonexParser(
    __in_ecount(textLength) const char16_t* text,
    uint32_t textLength,
    Options options
    )
    :   Base(text, textLength, options)
{
    // Test string:
    // Actions:foo, bar rod , cat{} {}, dog{bark} dog:{barkmore} bad:dog:{barklouder} (zig zag) "\b\\:" "a\b" , a:b:c  d:e:f{} ,, //hi
}


void JsonexParser::ResetDerived()
{
    nodeStack_.clear();
}


namespace
{
    bool JsonexIsValidWordCharacter(char32_t ch)
    {
        // This does not have the full Unicode database to follow the ECMA script identifier rules.
        char32_t lowerCaseAscii = ch & ~0x20;
        if (lowerCaseAscii >= 'A' && lowerCaseAscii <= 'Z')
        {
            return true;
        }
        else
        {
            // Digits and associated punctuation, or dollar sign and underscore.
            switch (ch)
            {
            case '$': case '_': case '.': case '-': case '+':
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                return true;
            }
        }
        return false;
    }

    bool JsonexIsControlCharacter(char32_t ch)
    {
        return (ch <= 0x001F || (ch >= 0x007F && ch <= 0x009F));
    }

    bool JsonexIsNewLineCharacter(char32_t ch)
    {
        return ch == '\r' || ch == '\n';
    }

    bool JsonexIsWhitespaceOrLineBreak(char32_t ch)
    {
        return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
    }

    bool JsonexIsWordSeparator(char32_t ch)
    {
        switch (ch)
        {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
        case ':':
        case '=':
        case ',':
        case '{':
        case '[':
        case '(':
        case '}':
        case ']':
        case ')':
        case '\0':
            return true;
        }
        return false;
    }

    TextTree::Node::Type JsonexGetNodeTypeFromCharacter(char32_t ch)
    {
        switch (ch)
        {
        case '{':
        case '}':
            return TextTree::Node::TypeObject;

        case '[':
        case ']':
            return TextTree::Node::TypeArray;

        case '(':
        case ')':
            return TextTree::Node::TypeFunction;

        default:
            // Assume value for anything else.
            return TextTree::Node::TypeValue;
        }
    }
}


bool JsonexParser::SkipSpacesAndLineBreaks()
{
    if (textIndex_ >= textLength_)
        return false;

    bool skippedAnySpaces = false;
    for (; textIndex_ < textLength_; ++textIndex_)
    {
        char16_t ch = text_[textIndex_];
        if (!JsonexIsWhitespaceOrLineBreak(ch))
            return skippedAnySpaces;
            
        skippedAnySpaces = true;
    }
    return skippedAnySpaces;
}


void JsonexParser::SkipComment()
{
    for (; textIndex_ < textLength_; ++textIndex_)
    {
        char16_t ch = text_[textIndex_];
        if (JsonexIsNewLineCharacter(ch))
            return;
    }
}


void JsonexParser::SkipInvalidWordCharacters()
{
    for (; textIndex_ < textLength_; ++textIndex_)
    {
        if (JsonexIsWordSeparator(text_[textIndex_]))
            return;
    }
}


bool JsonexParser::ReadWord(
    __out TextTree::Node& node,
    __inout std::u16string& nodeText
    )
{
    if (textIndex_ >= textLength_)
        return false;

    const uint32_t oldNodeTextSize = static_cast<uint32_t>(nodeText.size());
    const uint32_t startingTextIndex = textIndex_;

    char32_t ch = text_[textIndex_];
    if (JsonexIsWordSeparator(ch))
        return false;

    if (ch == '/') // Start of comment.
    {
        ++textIndex_;
        ch = PeekCodeUnit();
        if (ch != '/')
        {
            ReportError(textIndex_, u"Expect two forward slashes for a comment or for slashes to be used for named closure or in quoted strings.");
            SkipInvalidWordCharacters(); // Skip the invalid word.
            return false;
        }

        ++textIndex_;
        for (; textIndex_ < textLength_; ++textIndex_)
        {
            ch = text_[textIndex_];
            if (JsonexIsControlCharacter(ch)) // Control character found inside string which was not escaped.
            {
                // Only new lines are valid control characters.
                if (!JsonexIsNewLineCharacter(ch))
                {
                    ReportError(textIndex_, u"Control characters found inside comment.");
                    SkipComment(); // Skip the invalid word.
                    return false;
                }

                // Merge any following comment lines, looking ahead to see if it starts with a comment.
                SkipSpacesAndLineBreaks();
                ch = PeekCodeUnit(2);
                if (PeekCodeUnit() != '/' || PeekCodeUnit(1) != '/' || ch == '\0')
                {
                    break; // End of comment
                }
                // Skip the two '/', and read the next meaningful character.
                ReadCodeUnit(); ReadCodeUnit();
                nodeText.append(u"\r\n");
            }

            nodeText.push_back(char16_t(ch));
        }
        node.type = TextTree::Node::TypeComment;
    }
    else if (ch == '"') // Word is quoted.
    {
        ++textIndex_;
        while (textIndex_ < textLength_)
        {
            ch = text_[textIndex_];
            if (JsonexIsControlCharacter(ch)) // Control character found inside string which was not escaped.
            {
                ReportError(textIndex_, u"Control characters found inside string. They must be escaped.");
                SkipInvalidWordCharacters(); // Skip the invalid word.
                return false;
            }

            ++textIndex_;
            if (ch == '"')
            {
                break;
            }
            else if (ch == '\\' && !(options_ & OptionsNoEscapeSequence))
            {
                AppendCharacter(IN OUT nodeText, ReadCStyleEscapeCharacter());
            }
            else
            {
                nodeText.push_back(char16_t(ch));
            }
        }
        node.type = TextTree::Node::TypeValue;
    }
    else // Unquoted identifier or value.
    {
        while (textIndex_ < textLength_)
        {
            ch = text_[textIndex_];
            if (!JsonexIsValidWordCharacter(ch))
            {
                if (JsonexIsWordSeparator(ch))
                    break; // Stopped due to word separator, which is okay.

                ReportError(textIndex_, u"Unsupported characters found inside identifier. Put the identifier in quotes.");
                SkipInvalidWordCharacters(); // Skip the invalid word.
                return false;
            }

            ++textIndex_;
            if (ch == '\\' && !(options_ & OptionsNoEscapeSequence))
            {
                AppendCharacter(IN OUT nodeText, ReadCStyleEscapeCharacter());
            }
            else
            {
                nodeText.push_back(char16_t(ch));
            }
        }
        if (textIndex_ == startingTextIndex)
            return false;
        node.type = TextTree::Node::TypeValue;
    }

    uint32_t newNodeTextSize = static_cast<uint32_t>(nodeText.size());
    node.start  = oldNodeTextSize;
    node.length = newNodeTextSize - oldNodeTextSize;

    return true;
}


void JsonexParser::CheckClosingTag(
    __in const TextTree::Node& node,
    __in const std::u16string& nodeText
    )
{
    // Check that the explicit closing tag matches, if present.
    if (PeekCodeUnit() != '/')
        return;

    ++textIndex_;
    if (PeekCodeUnit() == '/')
    {
        RecedeCodeUnit(); // Don't eat the comment.
        return;
    }

    TextTree::Node closingNode;
    std::u16string closingTag;
    uint32_t textIndex = textIndex_;
    if (!ReadWord(OUT closingNode, IN OUT closingTag))
    {
        ReportError(textIndex, u"Expected explicit closing identifier.");
        return;
    }

    if (nodeText.size() < node.start + node.length)
        return; // The caller's node text does not contain the previous state to compare against.

    if (nodeText.compare(node.start, node.length, closingTag) != 0)
    {
        ReportError(textIndex, u"Closing identifier did not match opening identifier.");
    }
}


void JsonexParser::ClearAttributeOnStack()
{
    // If an unresolved attribute is on the stack, pop it, and restore the level.
    while (!nodeStack_.empty())
    {
        auto& back = nodeStack_.back();
        if (back.type != TextTree::Node::TypeAttribute)
            break;

        treeLevel_ = back.level;
        nodeStack_.pop_back();
    }
}


bool JsonexParser::ReadNode(
    __out TextTree::Node& node,
    __inout std::u16string& nodeText
    )
{
    for (;;)
    {
        if (textIndex_ >= textLength_)
            return false;

        SkipSpacesAndLineBreaks();

        node.start = 0;
        node.length = 0;
        node.level = treeLevel_;
        node.type = TextTree::Node::TypeNone;

        bool haveWord = ReadWord(IN OUT node, IN OUT nodeText);

        if (node.type == TextTree::Node::TypeComment)
        {
            break; // Return now for any comment, since the rest of the code below does not apply.
        }

        bool skippedAnySpaces = SkipSpacesAndLineBreaks();

        // We read the word but do not know what type the node is yet.
        // So read ahead one to decide that.
        char32_t ch = ReadCodeUnit();
        switch (ch)
        {
        case ':':
        case '=':
            if (!haveWord)
            {
                ReportError(textIndex_ - 1, u"Assignment must have a keyword identifier before it.");
                break;
            }

            // Followed by either an opening bracket or a value.
            SkipSpacesAndLineBreaks();
            ch = ReadCodeUnit();
            switch (ch)
            {
            case '{':
            case '[':
            case '(':
                node.type = JsonexGetNodeTypeFromCharacter(ch);
                break;

            default:
                RecedeCodeUnit(); // Leave this character for a later read.
                node.type = TextTree::Node::TypeAttribute;
            }
            break;

        case ',':
            ClearAttributeOnStack();
            break;

        case '{':
        case '[':
        case '(':
            if (haveWord && skippedAnySpaces)
            {
                RecedeCodeUnit(); // Leave this character for a later read.
                ClearAttributeOnStack();
            }
            else
            {
                node.type = JsonexGetNodeTypeFromCharacter(ch);
                haveWord = true; // Set it true even if empty name.
            }
            break;

        case '}':
        case ']':
        case ')':
            // Confirm the closing type matches the opening one.
            if (nodeStack_.empty())
            {
                ReportError(textIndex_ - 1, u"Closing brace did not match opening brace.");
            }
            else
            {
                ClearAttributeOnStack();
                if (!nodeStack_.empty())
                {
                    auto& back = nodeStack_.back();
                    treeLevel_ = back.level;
                    if (back.type != JsonexGetNodeTypeFromCharacter(ch))
                    {
                        ReportError(textIndex_ - 1, u"Closing brace did not match opening brace.");
                    }
                    CheckClosingTag(back, nodeText);
                    nodeStack_.pop_back();
                    ClearAttributeOnStack();
                }
            }
            break;

        case '\0':
            ClearAttributeOnStack();
            break;

        default:
            RecedeCodeUnit(); // Leave this character for a later read.
            ClearAttributeOnStack();
            break;
        }

        if (node.GetGenericType() == TextTree::Node::TypeKey)
        {
            nodeStack_.push_back(node);
            ++treeLevel_;
            break;
        }

        if (node.type != TextTree::Node::TypeNone)
        {
            break;
        }
    }

    return true;
}


namespace
{
    bool IniIsWhitespace(char32_t ch)
    {
        return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
    }

    bool IniIsWhitespaceOrLineBreak(char32_t ch)
    {
        return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
    }

    bool IniIsNewLineCharacter(char32_t ch)
    {
        return ch == '\r' || ch == '\n';
    }
}


IniParser::IniParser()
{
    InitializeDerived();
}


IniParser::IniParser(
    __in_ecount(textLength) const char16_t* text,
    uint32_t textLength,
    Options options
    )
    :   Base(text, textLength, options)
{
    InitializeDerived();
}


void IniParser::InitializeDerived()
{
    sectionLevel_ = 0;
}


void IniParser::ResetDerived()
{
    InitializeDerived();
}


void IniParser::SkipSpaces()
{
    while (textIndex_ < textLength_ && IniIsWhitespace(text_[textIndex_]))
    {
        ++textIndex_;
    }
}


void IniParser::SkipSpacesAndLineBreaks()
{
    while (textIndex_ < textLength_ && IniIsWhitespaceOrLineBreak(text_[textIndex_]))
    {
        ++textIndex_;
    }
}


bool IniParser::ReadWord(
    TextTree::Node::Type expectedType,
    __out TextTree::Node& node,
    __inout std::u16string& nodeText
    )
{
    if (textIndex_ >= textLength_)
        return false;

    const uint32_t oldNodeTextSize = static_cast<uint32_t>(nodeText.size());
    const uint32_t startingTextIndex = textIndex_;

    char32_t firstCh = text_[textIndex_];
    assert(firstCh != '#' && firstCh != ';'); // The comment prefix should have been read already.

    if (expectedType == TextTree::Node::TypeComment)
    {
        node.type = TextTree::Node::TypeComment;
        for (; textIndex_ < textLength_; ++textIndex_)
        {
            char16_t ch = text_[textIndex_];
            if (IniIsNewLineCharacter(ch))
            {
                break;
            }

            nodeText.push_back(char16_t(ch));
        }
    }
    else if (firstCh == '"') // Word is quoted.
    {
        node.type = TextTree::Node::TypeValue;

        ++textIndex_;
        while (textIndex_ < textLength_)
        {
            char16_t ch = text_[textIndex_];
            if (IniIsNewLineCharacter(ch))
            {
                ReportError(textIndex_, u"Quoted word ends before closing quote '\"'.");
                break;
            }

            ++textIndex_;
            if (ch == '"')
            {
                break;
            }
            else if (ch == '\\' && !(options_ & OptionsNoEscapeSequence))
            {
                AppendCharacter(IN OUT nodeText, ReadCStyleEscapeCharacter());
            }
            else
            {
                nodeText.push_back(char16_t(ch));
            }
        }
    }
    else // Unquoted identifier or value.
    {
        while (textIndex_ < textLength_)
        {
            char16_t ch = text_[textIndex_];
            if (IniIsNewLineCharacter(ch))
                break;

            // Break if we reach the end of the section name or key name.
            if (ch == ']' && expectedType == TextTree::Node::TypeSection)
                break;

            if ((ch == ':' || ch == '=') && expectedType == TextTree::Node::TypeAttribute)
                break;

            ++textIndex_;
            if (ch == '\\' && !(options_ & OptionsNoEscapeSequence))
            {
                AppendCharacter(IN OUT nodeText, ReadCStyleEscapeCharacter());
            }
            else
            {
                nodeText.push_back(char16_t(ch));
            }
        }
        // Remove any trailing whitespace.
        while (nodeText.size() > startingTextIndex && nodeText.back() == ' ')
        {
            nodeText.pop_back();
        }
    }

    uint32_t newNodeTextSize = static_cast<uint32_t>(nodeText.size());
    node.type = expectedType;
    node.start  = oldNodeTextSize;
    node.length = newNodeTextSize - oldNodeTextSize;

    return true;
}


bool IniParser::ReadNode(
    __out TextTree::Node& node,
    __inout std::u16string& nodeText
    )
{
    if (textIndex_ >= textLength_)
        return false;

    node.start = 0;
    node.length = 0;
    node.level = treeLevel_;
    node.type = TextTree::Node::TypeNone;

    // We read the word but do not know what type the node is.
    // So read ahead one to decide that.
    SkipSpaces();
    char32_t ch = PeekCodeUnit();
    switch (ch)
    {
    case '[':
        treeLevel_ = 0;
        sectionLevel_ = 1; // Nested inside a section after this point - no longer in the global section.

        AdvanceCodeUnit();
        SkipSpaces();
        ReadWord(TextTree::Node::TypeSection, IN OUT node, IN OUT nodeText);
        SkipSpaces();
        if (PeekCodeUnit() == ']')
        {
            AdvanceCodeUnit();
        }
        else
        {
            ReportError(textIndex_, u"Section name is missing closing bracket ']'.");
        }
        SkipSpacesAndLineBreaks();
        break;

    case '#':
    case ';':
        treeLevel_ = sectionLevel_;
        AdvanceCodeUnit();
        ReadWord(TextTree::Node::TypeComment, IN OUT node, IN OUT nodeText);
        SkipSpacesAndLineBreaks();
        break;

    case ':':
    case '=':
        treeLevel_ = sectionLevel_ + 1;
        AdvanceCodeUnit();
        SkipSpaces();
        ReadWord(TextTree::Node::TypeValue, IN OUT node, IN OUT nodeText);
        SkipSpacesAndLineBreaks();
        break;

    case '\0':
        treeLevel_ = 0;
        return false;

    default:
        // Assume key.
        treeLevel_ = sectionLevel_;
        SkipSpaces();
        ReadWord(TextTree::Node::TypeAttribute, IN OUT node, IN OUT nodeText);
        SkipSpaces();
        break;
    }

    node.level = treeLevel_;

    return true;
}


TextTreeWriter::TextTreeWriter(Options options)
    :   options_(options)
{
}


HRESULT TextTreeWriter::WriteNode(
    TextTree::Node::Type type,
    __in_ecount(textLength) const char16_t* text,
    uint32_t textLength
    )
{
    return E_NOTIMPL;
}


HRESULT TextTreeWriter::EnterNode()
{
    return E_NOTIMPL;
}


HRESULT TextTreeWriter::ExitNode()
{
    return E_NOTIMPL;
}


array_ref<char16_t const> TextTreeWriter::GetText() const noexcept
{
    return text_;
}


const char16_t* TextTreeWriter::GetText(__out uint32_t& textLength) const noexcept
{
    textLength = static_cast<uint32_t>(text_.size());
    return text_.c_str();
}


void TextTreeWriter::GetText(OUT std::u16string& text) const
{
    text = text_;
}


uint32_t TextTreeWriter::GetTextLength(
    __in_ecount_opt(textLength) const char16_t* text,
    uint32_t textLength
    ) noexcept
{
    if (text == nullptr)
        return 0; // Empty string.

    if (textLength != 0xFFFFFFFF)
        return textLength; // Size known, so do not calculate.

    return static_cast<uint32_t>(wcslen(ToWChar(text)));
}


HRESULT TextTreeWriter::WriteNodes(const TextTree& textTree)
{
    const auto nodeCount = textTree.GetNodeCount();
    uint32_t firstNode = 0;
    if (nodeCount > 0 && textTree.GetNode(0).type == TextTree::Node::TypeRoot)
        ++firstNode; // Ignore the root node.

    if (firstNode >= nodeCount)
        return S_OK; // Empty tree, so do nothing.

    const auto initialLevel = textTree.GetNode(firstNode).level;
    auto previousLevel = initialLevel;

    for (uint32_t i = firstNode, ci = textTree.GetNodeCount(); i < ci; ++i)
    {
        uint32_t textLength = 0;
        const auto& node = textTree.GetNode(i);
        auto text = textTree.GetText(node, OUT textLength);
        auto currentLevel = node.level;

        if (currentLevel > previousLevel)
        {
            IFR(EnterNode());
        }
        else while (previousLevel > currentLevel)
        {
            --previousLevel;
            IFR(ExitNode());
        }

        IFR(WriteNode(node.type, text, textLength));
        if (node.GetGenericType() == TextTree::Node::TypeKey)
        {
            // Explicitly enter a level now rather than waiting for next
            // iteration of the loop. This addresses cases where the node
            // is empty with no children.
            ++currentLevel;
            IFR(EnterNode());
        }

        previousLevel = currentLevel;
    }

    // Exit as many nodes as we entered.
    while (previousLevel > initialLevel)
    {
        --previousLevel;
        IFR(ExitNode());
    }

    return S_OK;
}


bool TextTreeWriter::WantWhitespace()
{
    return !(options_ & OptionsDiscardPureWhitespace);
}


JsonexWriter::JsonexWriter(Options options)
    :   Base(options)
{
}


TextTree::Node::Type JsonexWriter::ResolveNodeType(
    TextTree::Node::Type type
    )
{
    switch (type)
    {
    case TextTree::Node::TypeValue:
    case TextTree::Node::TypeString:
    case TextTree::Node::TypeNumber:
    case TextTree::Node::TypeAttribute:
    case TextTree::Node::TypeFunction:
    case TextTree::Node::TypeArray:
    case TextTree::Node::TypeObject:
        // Return as-is.
        return type;
    }

    // Resolve the unknown type to its more generic type.
    type = TextTree::Node::Type(type & TextTree::Node::TypeGenericMask);

    switch (type)
    {
    case TextTree::Node::TypeValue:
    case TextTree::Node::TypeKey:
    case TextTree::Node::TypeComment:
        // Recognized generic type.
        return type;
    }

    return TextTree::Node::TypeNone;
}


HRESULT JsonexWriter::WriteNode(
    TextTree::Node::Type type,
    __in_ecount(textLength) const char16_t* text,
    uint32_t textLength
    )
{
    type = ResolveNodeType(type);
    if (type == TextTree::Node::TypeNone)
    {
        return S_OK; // Ignore unknown types, and do not fail.
    }

    // Insert a comma after the previous item if this is not the first item in
    // the parent container.
    bool wantNewLine = false;
    bool isSubsequentItem = (nodeTypeStack_.size() > nodeLevel_);
    bool wantSpaces = WantWhitespace();
    if (isSubsequentItem)
    {
        if ((nodeTypeStack_[nodeLevel_] & TextTree::Node::TypeGenericMask) == TextTree::Node::TypeComment)
        {
            wantNewLine = true; // Force new line after a single line comment, regardless of whitespace flag.
        }
        else
        {
            text_.push_back(L','); // Separate this node from the previous one.
        }
    }

    // Reset to a new line if needed.
    // Keep attribute:value pairs on the same line, along with value,value,value series.
    if (wantSpaces)
    {
        wantNewLine = true;
        if ((type & TextTree::Node::TypeGenericMask) == TextTree::Node::TypeValue)
        {
            if (isSubsequentItem)
            {
                wantNewLine = (nodeTypeStack_[nodeLevel_] != TextTree::Node::TypeValue);
            }
            else if (nodeTypeStack_.size() + 1 > nodeLevel_)
            {
                wantNewLine = (nodeTypeStack_[nodeLevel_ - 1] != TextTree::Node::TypeAttribute);
            }
        }
    }
    if (wantNewLine)
    {
        if (!text_.empty()) // Write new line, but never start the text file with a leading return (would be a pointless blank line).
            text_.append(u"\r\n");

        WriteIndentation();
    }

    nodeTypeStack_.resize(nodeLevel_);
    nodeTypeStack_.push_back(type);

    switch (type)
    {
    case TextTree::Node::TypeNumber:
        text_.append(text, textLength);
        break;

    case TextTree::Node::TypeValue:
    case TextTree::Node::TypeString:
        WriteStringInternal(text, textLength);
        break;

    case TextTree::Node::TypeAttribute:
        WriteStringInternal(text, textLength);
        text_.push_back(':');
        if (WantWhitespace())
        {
            text_.push_back(' ');
        }
        break;

    case TextTree::Node::TypeFunction:
        if (textLength != 0)
        {
            WriteStringInternal(text, textLength);
            text_.push_back(':');
        }
        text_.append(u"(");
        break;

    case TextTree::Node::TypeArray:
        if (textLength != 0)
        {
            WriteStringInternal(text, textLength);
            text_.push_back(':');
        }
        text_.append(u"[");
        break;

    case TextTree::Node::TypeKey:
    case TextTree::Node::TypeElement:
    case TextTree::Node::TypeObject:
        if (textLength != 0)
        {
            WriteStringInternal(text, textLength);
            text_.push_back(':');
        }
        text_.append(u"{");
        break;

    case TextTree::Node::TypeComment:
        WriteCommentInternal(text, textLength);
        break;
    }

    return S_OK;
}


void JsonexWriter::WriteStringInternal(
    __in_ecount(textLength) const char16_t* text,
    uint32_t textLength
    )
{
    // Write a string with necessary escaping.
    text_.push_back('\"');

    char16_t const* p = text;
    char16_t const* pEnd = text + textLength;

    while (p != pEnd)
    {
        char32_t const ch = *p;
        if (JsonexIsControlCharacter(ch))
        {
            text_.push_back('\\');
            text_.push_back('u');

            char16_t digits[4] = 
            {
                GetSingleHexDigit(ch >> 12),
                GetSingleHexDigit(ch >> 8),
                GetSingleHexDigit(ch >> 4),
                GetSingleHexDigit(ch)
            };

            text_.append(digits, digits + ARRAYSIZE(digits));
        }
        else
        {
            if (ch == L'\\' || ch == L'\"')
            {
                text_.push_back(L'\\');
            }
            text_.push_back(char16_t(ch));
        }
        ++p;
    }

    text_.push_back('\"');
}


void JsonexWriter::WriteCommentInternal(
    __in_ecount(textLength) const char16_t* text,
    uint32_t textLength
    )
{
    // Write each line.
    auto currentText = text;
    auto textEnd = text + textLength;
    while (currentText != textEnd)
    {
        const char16_t newLineCharacters[2] = {'\r', '\n'};
        auto nextLine = std::find_first_of(currentText, textEnd, newLineCharacters, newLineCharacters + ARRAYSIZE(newLineCharacters));
        auto currentTextLength = textEnd - currentText;
        if (nextLine != textEnd)
        {
            // Determine the segment up to the end of the line, and skip any CR/LF pair.
            currentTextLength = nextLine - currentText;
            ++nextLine;
            if (nextLine != textEnd && *(nextLine-1) == '\r' && *nextLine == '\n')
                ++nextLine;
        }
        if (currentText != text)
        {
            text_.append(u"\r\n");
            WriteIndentation();
        }
        text_.append(u"//");
        text_.append(currentText, currentTextLength); // todo: consider multiline comments
        currentText = nextLine;
    }
}


void JsonexWriter::WriteIndentation()
{
    if (!WantWhitespace())
    {
        return;
    }

    const size_t spacesToIndent = nodeLevel_ * spacesPerIndent_;
    if (spacesToIndent > spaceBuffer_.size())
        spaceBuffer_.assign(spacesToIndent, ' ');

    text_.append(spaceBuffer_.c_str(), spacesToIndent);
}


HRESULT JsonexWriter::EnterNode()
{
    ++nodeLevel_;

    return S_OK;
}


HRESULT JsonexWriter::ExitNode()
{
    if (nodeLevel_ <= 0)
        return E_BOUNDS;

    --nodeLevel_;

    if (nodeTypeStack_.size() <= nodeLevel_)
    {
        return S_OK;
    }

    TextTree::Node::Type type = nodeTypeStack_[nodeLevel_];

    char16_t closingPunctuation = '\0';

    switch (type)
    {
    case TextTree::Node::TypeFunction:
        closingPunctuation = ')';
        break;

    case TextTree::Node::TypeArray:
        closingPunctuation = ']';
        break;

    case TextTree::Node::TypeKey:
    case TextTree::Node::TypeObject:
        closingPunctuation = '}';
        break;

    }

    if (closingPunctuation != '\0')
    {
        if (WantWhitespace())
            text_.append(u"\r\n");

        WriteIndentation();

        text_.push_back(closingPunctuation);
    }

    return S_OK;
}


HRESULT JsonexWriter::BeginKey(
    __in_ecount(textLength) const char16_t* text,
    uint32_t textLength
    )
{
    textLength = GetTextLength(text, textLength);

    IFR(JsonexWriter::WriteNode(
                TextTree::Node::TypeAttribute,
                text,
                textLength
                ));

    IFR(EnterNode());

    return S_OK;
}


HRESULT JsonexWriter::WriteValueString(
    __in_ecount(textLength) const char16_t* text,
    uint32_t textLength
    )
{
    textLength = GetTextLength(text, textLength);

    return JsonexWriter::WriteNode(
                TextTree::Node::TypeString,
                text,
                textLength
                );
}


HRESULT JsonexWriter::WriteValueNumber(
    __in_ecount(textLength) const char16_t* text,
    uint32_t textLength
    )
{
    textLength = GetTextLength(text, textLength);

    return JsonexWriter::WriteNode(
                TextTree::Node::TypeNumber,
                text,
                textLength
                );
}


HRESULT JsonexWriter::BeginArray(
    __in_ecount_opt(textLength) const char16_t* text,
    uint32_t textLength
    )
{
    textLength = GetTextLength(text, textLength);

    IFR(JsonexWriter::WriteNode(
                TextTree::Node::TypeArray,
                text,
                textLength
                ));

    IFR(EnterNode());

    return S_OK;
}


HRESULT JsonexWriter::BeginObject(
    __in_ecount_opt(textLength) const char16_t* text,
    uint32_t textLength
    )
{
    textLength = GetTextLength(text, textLength);

    IFR(JsonexWriter::WriteNode(
                TextTree::Node::TypeObject,
                text,
                textLength
                ));

    IFR(EnterNode());

    return S_OK;
}


HRESULT JsonexWriter::EndScope()
{
    return ExitNode();
}


namespace
{
    // Line feed, carriage return, and tab are allowed control characters.
    const uint64_t xmlAllowedControlCharacters      = (1i64 << 0x0009)
                                                    | (1i64 << 0x000A)
                                                    | (1i64 << 0x000D)
                                                    ;
    // These characters in text must be escaped, with either named or numeric entities.
    const uint64_t xmlReservedTextCharacters        = (0x001F ^ xmlAllowedControlCharacters)
                                                    | (1i64 << '<')
                                                    | (1i64 << '>')
                                                    | (1i64 << '&')
                                                    ;
    // Additionally these are excluded inside attribute values.
    const uint64_t xmlReservedValueCharacters       = xmlReservedTextCharacters
                                                    | (1i64 << '\'')
                                                    | (1i64 << '\"')
                                                    ;
    // These are disallowed inside identifier names.
    const uint64_t xmlReservedAttributeCharacters   = (0x001F)
                                                    | (1i64 << '<')
                                                    | (1i64 << '>')
                                                    | (1i64 << '&')
                                                    | (1i64 << '/')
                                                    | (1i64 << '\'')
                                                    | (1i64 << '\"')
                                                    ;

    inline bool XmlIsReservedCharacter(char32_t ch, uint64_t reservedCharactersMask)
    {
        return (ch < 64) && (reservedCharactersMask & (1i64 << ch));
    }

    inline bool XmlIsReservedTextCharacter(char32_t ch)
    {
        return (ch < 64) && (xmlReservedTextCharacters & (1i64 << ch));
    }

    inline bool XmlIsReservedValueCharacter(char32_t ch)
    {
        return (ch < 64) && (xmlReservedValueCharacters & (1i64 << ch));
    }

    inline bool XmlIsReservedAttributeCharacter(char32_t ch)
    {
        return (ch < 64) && (xmlReservedAttributeCharacters & (1i64 << ch));
    }
}

XmlWriter::XmlWriter(Options options)
    :   Base(options),
        previousType_(TextTree::Node::TypeNone),
        isInsideOpeningTag_(false)
{
}


TextTree::Node::Type XmlWriter::ResolveNodeType(
    TextTree::Node::Type type
    )
{
    switch (type)
    {
    case TextTree::Node::TypeValue:
    case TextTree::Node::TypeText:
    case TextTree::Node::TypeElement:
    case TextTree::Node::TypeAttribute:
        // Return as-is.
        return type;
    }

    // Resolve the unknown type to its more generic type.
    type = TextTree::Node::Type(type & TextTree::Node::TypeGenericMask);

    switch (type)
    {
    case TextTree::Node::TypeKey:
        return TextTree::Node::TypeElement;

    case TextTree::Node::TypeValue:

    case TextTree::Node::TypeComment:
        // Recognized generic type.
        return type;
    }

    return TextTree::Node::TypeNone;
}


HRESULT XmlWriter::WriteNode(
    TextTree::Node::Type type,
    __in_ecount(textLength) const char16_t* text,
    uint32_t textLength
    )
{
    type = ResolveNodeType(type);
    if (type == TextTree::Node::TypeNone)
    {
        return S_OK; // Ignore unknown types, and do not fail.
    }

    if (isInsideOpeningTag_)
    {
        if (!(type == TextTree::Node::TypeAttribute || type == TextTree::Node::TypeValue))
        {
            isInsideOpeningTag_ = false;
            text_.append(u" >");
        }
    }

    // Reset to a new line if needed.
    if (!WantWhitespace())
    {
        // Keep values on the same line as attributes.
        bool wantNewLine = true;
        if ((type & TextTree::Node::TypeGenericMask) == TextTree::Node::TypeValue)
        {
            if (nodeStack_.size() + 1 > nodeLevel_)
            {
                wantNewLine = (nodeStack_[nodeLevel_ - 1].type != TextTree::Node::TypeAttribute);
            }
        }
        if (wantNewLine && !text_.empty()) // Never start with a leading return.
        {
            text_.append(u"\r\n");
            const size_t spacesToIndent = nodeLevel_ * spacesPerIndent_;
            spaceBuffer_.assign(spacesToIndent, ' ');
            text_.append(spaceBuffer_);
        }
    }

    TextTree::Node node = {};
    node.type = type;
    nodeStack_.resize(nodeLevel_);
    nodeStack_.push_back(node);

    switch (type)
    {
    case TextTree::Node::TypeValue:
        text_.push_back('\"');
        WriteStringInternal(text, textLength, type);
        text_.push_back('\"');
        break;

    case TextTree::Node::TypeText:
        WriteStringInternal(text, textLength, type);
        break;

    case TextTree::Node::TypeElement:
        text_.push_back('<');
        nodeStack_.back().start = static_cast<uint32_t>(text_.size());
        nodeStack_.back().length = textLength;
        WriteStringInternal(text, textLength, type);
        isInsideOpeningTag_ = true;
        break;

    case TextTree::Node::TypeAttribute:
        WriteStringInternal(text, textLength, type);
        text_.push_back('=');
        break;

    case TextTree::Node::TypeComment:
        text_.append(u"<!-- ");
        WriteStringInternal(text, textLength, type);
        text_.append(u" -->");
        break;
    }

    previousType_ = type;

    return S_OK;
}


void XmlWriter::WriteStringInternal(
    __in_ecount(textLength) const char16_t* text,
    uint32_t textLength,
    TextTree::Node::Type type
    )
{
    char16_t const* p = text;
    char16_t const* pEnd = text + textLength;

    // The type of node being written determines which characters require escaping.
    uint64_t reservedCharactersMask = 0;
    switch (type)
    {
    case TextTree::Node::TypeValue:     reservedCharactersMask = xmlReservedValueCharacters;        break;
    case TextTree::Node::TypeText:      reservedCharactersMask = xmlReservedTextCharacters;         break;
    case TextTree::Node::TypeElement:
    case TextTree::Node::TypeAttribute: reservedCharactersMask = xmlReservedAttributeCharacters;    break;
    case TextTree::Node::TypeComment:   reservedCharactersMask = xmlReservedTextCharacters;         break;
    }

    while (p != pEnd)
    {
        char32_t const ch = *p;
        if (XmlIsReservedCharacter(ch, reservedCharactersMask))
        {
            text_.push_back('&');
            text_.push_back('#');
            text_.push_back('x');

            char16_t digits[4] = 
            {
                GetSingleHexDigit(ch >> 12),
                GetSingleHexDigit(ch >> 8),
                GetSingleHexDigit(ch >> 4),
                GetSingleHexDigit(ch)
            };
            text_.append(digits, digits + ARRAYSIZE(digits));
            text_.push_back(';');
        }
        else
        {
            text_.push_back(char16_t(ch));
        }
        ++p;
    }
}


HRESULT XmlWriter::EnterNode()
{
    ++nodeLevel_;

    return S_OK;
}


HRESULT XmlWriter::ExitNode()
{
    if (nodeLevel_ <= 0)
        return E_BOUNDS;

    --nodeLevel_;

    if (nodeStack_.size() <= nodeLevel_)
    {
        return S_OK;
    }

    const auto& node = nodeStack_[nodeLevel_];

    // Complete the opening tag if still inside it, such as "<body" being incomplete.
    if (isInsideOpeningTag_ && node.type == TextTree::Node::TypeElement)
    {
        isInsideOpeningTag_ = false;
        text_.append(u">");
    }

    switch (node.type)
    {
    case TextTree::Node::TypeKey:
    case TextTree::Node::TypeElement:
        {
            text_.append(u"\r\n");
            const size_t spacesToIndent = nodeLevel_ * spacesPerIndent_;
            spaceBuffer_.assign(spacesToIndent, ' ');
            text_.append(spaceBuffer_);
            text_.append(u"</");
            text_.append(&text_[node.start], node.length);
            text_.append(u">");
        }
        break;
    }

    return S_OK;
}


HRESULT XmlWriter::WriteText(
    __in_ecount(textLength) const char16_t* text,
    uint32_t textLength
    )
{
    textLength = GetTextLength(text, textLength);

    return XmlWriter::WriteNode(
                TextTree::Node::TypeText,
                text,
                textLength
                );
}


HRESULT XmlWriter::WriteValueString(
    __in_ecount(textLength) const char16_t* text,
    uint32_t textLength
    )
{
    textLength = GetTextLength(text, textLength);

    return XmlWriter::WriteNode(
                TextTree::Node::TypeString,
                text,
                textLength
                );
}


HRESULT XmlWriter::WriteValueNumber(
    __in_ecount(textLength) const char16_t* text,
    uint32_t textLength
    )
{
    textLength = GetTextLength(text, textLength);

    return XmlWriter::WriteNode(
                TextTree::Node::TypeNumber,
                text,
                textLength
                );
}


HRESULT XmlWriter::BeginElement(
    __in_ecount(textLength) const char16_t* text,
    uint32_t textLength
    )
{
    textLength = GetTextLength(text, textLength);

    IFR(XmlWriter::WriteNode(
                TextTree::Node::TypeElement,
                text,
                textLength
                ));

    IFR(EnterNode());

    return S_OK;
}


HRESULT XmlWriter::BeginAttribute(
    __in_ecount(textLength) const char16_t* text,
    uint32_t textLength
    )
{
    textLength = GetTextLength(text, textLength);

    IFR(XmlWriter::WriteNode(
                TextTree::Node::TypeAttribute,
                text,
                textLength
                ));

    IFR(EnterNode());

    return S_OK;
}


HRESULT XmlWriter::EndScope()
{
    return ExitNode();
}


HRESULT RunTests()
{
    const char16_t* testString = u"thistest=foo bar(stuff:boo cat[1 2]) singleitem singleitem2";

    // Read all nodes into a text tree.
    TextTree nodes;
    uint32_t testStringLength = static_cast<uint32_t>(wcslen(ToWChar(testString)));
    JsonexParser parser(testString, testStringLength, TextTreeParser::OptionsNoEscapeSequence);
    parser.ReadNodes(IN OUT nodes);

    std::u16string t;
    for (auto i = nodes.begin(), e = nodes.end(); i != e; ++i)
    {
        t += i.GetText();
        t += u"\r\n";
    }

    auto i = nodes.begin();
    auto s = i[u"bar"][u"stuff"].GetSubvalue();
    auto bar = i[u"bar"];
    auto b = bar.begin();
    auto e = bar.end();
    auto zar = i[u"zar"];
    auto b2 = zar.begin();
    auto e2 = zar.end();
    auto sing = i[u"singleitem"];
    auto b3 = sing.begin();
    auto e3 = sing.end();
    auto sing2 = i[u"singleitem2"];
    auto b4 = sing2.begin();
    auto e4 = sing2.end();
    auto j = i;
    ++j;
    --j;
    j++;
    j--;
    j += 2;
    j -= 2;
    assert(i == j);

    auto s2 = i.GetText();
    auto s3 = i.GetSubvalue();
    auto s4 = i.GetSubvalue(u"Hello", 5);

    MessageBox(nullptr, ToWChar(t.c_str()), L"Caption", MB_OK);

    return S_OK;
}

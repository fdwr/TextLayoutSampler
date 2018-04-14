//----------------------------------------------------------------------------
//  Author:     Dwayne Robinson
//  History:    2015-06-19 Created
//----------------------------------------------------------------------------
#include "precomp.h"
#include <vector>
#include <string>


MODULE(Common.ListSubstringPrioritizer)
EXPORT_BEGIN
    #include "Common.ListSubstringPrioritizer.h"
EXPORT_END

////////////////////////////////////////

ListSubstringPrioritizer::ListSubstringPrioritizer(
    array_ref<char16_t const> filterString,
    uint32_t itemCount
    )
{
    items_.resize(itemCount);
    filterString_.assign(filterString.data(), filterString.size());
    ToUpperCase(IN OUT filterString_);

    for (uint32_t itemIndex = 0; itemIndex < itemCount; ++itemIndex)
    {
        auto& item = items_[itemIndex];
        item.index = itemIndex;
        item.weight = IndexAndWeight::WeightValueNoMatch;
    }
}


ListSubstringPrioritizer::WeightValue ListSubstringPrioritizer::GetStringWeight(array_ref<char16_t const> name)
{
    // Early out if there is no filter.
    if (filterString_.empty())
    {
        return IndexAndWeight::WeightValueStringPrefix;
    }

    // Capitalize so we can compare the two strings without case.
    majusculeName_.assign(name.data(), name.size());
    ToUpperCase(IN OUT majusculeName_);

    size_t minStringSize = std::min(majusculeName_.size(), filterString_.size());

    if (wcsncmp(ToWChar(majusculeName_.c_str()), ToWChar(filterString_.c_str()), minStringSize) == 0)
    {
        // Prefix matching has priority.
        return IndexAndWeight::WeightValueStringPrefix;
    }
    else
    {
        auto searchIndex = majusculeName_.find(filterString_);
        if (searchIndex != std::u16string::npos)
        {
            // Substrings have next priority.
            bool beginsWord = (searchIndex == 0 || majusculeName_[searchIndex - 1] == ' ');
            return beginsWord ? IndexAndWeight::WeightValueWordPrefix : IndexAndWeight::WeightValueSubstring;
        }
    }

    // Add non-matching item index to the back of deferred list.
    return IndexAndWeight::WeightValueNoMatch;
}


void ListSubstringPrioritizer::SetItemWeight(uint32_t itemIndex, WeightValue weight)
{
    auto& item = items_.at(itemIndex);
    item.weight = weight;
}


array_ref<uint32_t> ListSubstringPrioritizer::GetItemIndices(_Out_ array_ref<uint32_t> items, bool excludeMismatches)
{
    // Copy the array indices out after ordering them best to worst.
    std::stable_sort(items_.begin(), items_.end());

    size_t i = 0;
    size_t itemCount = std::min(items.size(), items_.size());

    for (i = 0; i < itemCount; ++i)
    {
        auto& item = items_[i];
        if (excludeMismatches && item.weight >= IndexAndWeight::WeightValueNoMatch)
            break;

        items[i] = item.index;
    }

    return array_ref<uint32_t>(items.data(), i);
}

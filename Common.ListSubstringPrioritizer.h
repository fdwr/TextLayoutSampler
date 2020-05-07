//----------------------------------------------------------------------------
//  History:    2015-06-19 Dwayne Robinson - Created
//----------------------------------------------------------------------------
#pragma once

class ListSubstringPrioritizer
{
    struct IndexAndWeight
    {
        enum WeightValue
        {
            WeightValueStringPrefix = 0,
            WeightValueWordPrefix = 1,
            WeightValueSubstring = 2,
            WeightValueNoMatch = 3,
        };

        uint32_t index;
        WeightValue weight;

        bool operator < (IndexAndWeight const& other) const noexcept
        {
            return weight < other.weight;
        }
    };

public:
    using WeightValue = IndexAndWeight::WeightValue;

    ListSubstringPrioritizer(
        array_ref<char16_t const> filterString,
        uint32_t itemCount
        );

    WeightValue GetStringWeight(array_ref<char16_t const> name);
    void SetItemWeight(uint32_t itemIndex, WeightValue weight);
    array_ref<uint32_t> GetItemIndices(_Out_ array_ref<uint32_t> items, bool excludeMismatches);

private:
    std::vector<IndexAndWeight> items_;
    std::u16string filterString_;
    std::u16string majusculeName_;
};

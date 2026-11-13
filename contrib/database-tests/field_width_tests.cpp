#include "Field.h"
#include <array>
#include <iostream>

class TinyIntField : public Field
{
    // Adjacent nonzero bytes expose accidental wider reads, as in mail.checked.
    alignas(uint32) std::array<char,4> bytes{};
public:
    explicit TinyIntField(uint8 value)
    {
        bytes = {char(value), char(0xa5), char(0x5a), char(0x7f)};
        SetByteValue(bytes.data(), 1);
    }
};

int main()
{
    for (unsigned value=0; value<256; ++value)
    {
        TinyIntField field{uint8(value)};
        if (field.GetUInt8()!=value)
        {
            std::cerr << "Raw TINYINT read failed at " << value << '\n';
            return 1;
        }
    }
    std::cout << "256 raw TINYINT flag/sound-type values read without adjacent bytes\n";
}

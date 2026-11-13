#include "GossipBroadcastId.h"
#include <initializer_list>
int main()
{
    using namespace GossipBroadcastId;
    for (auto text : {0u, 1u, 68u, 4938u, 0xffffffu, MaxTextId})
        for (unsigned option = 0; option < 8; ++option)
        {
            std::uint32_t decoded = 0;
            unsigned slot = 99;
            auto id = Encode(text, option);
            if (id <= 77161 || !Decode(id, decoded, slot) || decoded != text || slot != option)
                return 1;
        }
    std::uint32_t text = 42;
    unsigned option = 3;
    if (Decode(1, text, option) || Decode(Base - 1, text, option) || Decode(Limit, text, option))
        return 2;
    if (Encode(MaxTextId + 1, 0) || Encode(12, 8))
        return 3;
    if (Encode(68, 0) == Encode(4938, 0) || Encode(68, 0) == Encode(68, 1))
        return 4;
    return 0;
}

/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/RememberTaxiAction.cpp (#179). AC's WotLK parse is a
 * plain (non bit-packed) `guid >> from >> to`. 5.4.8 bit-packs the guid and, per
 * TaxiHandler.cpp:HandleActivateTaxiOpcode (verified against this core's own handler), reads the two
 * node ids in the OPPOSITE order on the wire (destination first, then source) before the guid bits -
 * see the comment at the read site. CMSG_ACTIVATE_TAXI_EXPRESS mirrors
 * TaxiHandler.cpp:HandleActivateTaxiExpressOpcode exactly (both are new, verified reads, not a
 * behaviour change from AC).
 */

#include "RememberTaxiAction.h"

#include "AiObjectContext.h"
#include "Event.h"
#include "LastMovementValue.h"
#include "ObjectGuid.h"
#include "Playerbots.h"
#include "WorldPacket.h"

bool RememberTaxiAction::Execute(Event event)
{
    WorldPacket p(event.getPacket());
    p.rpos(0);

    switch (p.GetOpcode())
    {
        case CMSG_ACTIVATE_TAXI:
        {
            // 5.4.8 TaxiHandler.cpp:HandleActivateTaxiOpcode: "recvData >> nodes[1] >> nodes[0];" -
            // the wire order is [destination, source], the reverse of the local array/AC order
            // [from, to]. Read into locals first so "last taxi" ends up [from, to] like AC.
            uint32 to, from;
            p >> to >> from;

            ObjectGuid guid;
            guid[4] = p.ReadBit();
            guid[0] = p.ReadBit();
            guid[1] = p.ReadBit();
            guid[2] = p.ReadBit();
            guid[5] = p.ReadBit();
            guid[6] = p.ReadBit();
            guid[7] = p.ReadBit();
            guid[3] = p.ReadBit();

            p.ReadByteSeq(guid[1]);
            p.ReadByteSeq(guid[0]);
            p.ReadByteSeq(guid[6]);
            p.ReadByteSeq(guid[5]);
            p.ReadByteSeq(guid[2]);
            p.ReadByteSeq(guid[4]);
            p.ReadByteSeq(guid[3]);
            p.ReadByteSeq(guid[7]);

            LastMovement& movement = AI_VALUE(LastMovement&, "last taxi");
            movement.taxiNodes.clear();
            movement.taxiNodes.resize(2);
            movement.taxiNodes[0] = from;
            movement.taxiNodes[1] = to;
            movement.taxiMaster = guid;
            return true;
        }
        case CMSG_ACTIVATE_TAXI_EXPRESS:
        {
            // 5.4.8 TaxiHandler.cpp:HandleActivateTaxiExpressOpcode bit/byte order, verified against
            // this core's own handler.
            ObjectGuid guid;
            guid[6] = p.ReadBit();
            guid[7] = p.ReadBit();
            uint32 node_count = p.ReadBits(22);
            guid[2] = p.ReadBit();
            guid[0] = p.ReadBit();
            guid[4] = p.ReadBit();
            guid[3] = p.ReadBit();
            guid[1] = p.ReadBit();
            guid[5] = p.ReadBit();

            p.ReadByteSeq(guid[2]);
            p.ReadByteSeq(guid[7]);
            p.ReadByteSeq(guid[1]);

            std::vector<uint32> nodes;
            for (uint32 i = 0; i < node_count; ++i)
            {
                uint32 node;
                p >> node;
                nodes.push_back(node);
            }

            if (nodes.empty())
                return false;

            p.ReadByteSeq(guid[0]);
            p.ReadByteSeq(guid[5]);
            p.ReadByteSeq(guid[3]);
            p.ReadByteSeq(guid[6]);
            p.ReadByteSeq(guid[4]);

            LastMovement& movement = AI_VALUE(LastMovement&, "last taxi");
            movement.taxiNodes = nodes;
            movement.taxiMaster = guid;
            return true;
        }
    }

    return false;
}

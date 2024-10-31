#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace Utils
{
    enum class PairField
    {
        Origin = 0,
        Destination,
        IsOneWay,
        IsRoundtrip,
        Carriers
    };

    static const std::unordered_map<PairField, std::string> pairFields = {
        { PairField::Origin, "origin" },
        { PairField::Destination, "destination" },
        { PairField::IsOneWay, "isOneWay" },
        { PairField::IsRoundtrip, "isRoundtrip" },
        { PairField::Carriers, "carriers" }
    };

    inline std::string getPairFieldName(const PairField val)
    {
        return pairFields.find(val)->second;
    }

    struct Pair
    {
        std::string origin;
        std::string destination;
        bool isOneWay;
        bool isRoundtrip;
        std::vector<std::string> carriers;
    };

    std::string getSerialized(const Pair& pair)
    {
        try
        {
            boost::property_tree::ptree ptree;
            std::ostringstream buf;

            ptree.put(getPairFieldName(PairField::Origin), pair.origin);
            ptree.put(getPairFieldName(PairField::Destination), pair.destination);
            ptree.put(getPairFieldName(PairField::IsOneWay), pair.isOneWay);
            ptree.put(getPairFieldName(PairField::IsRoundtrip), pair.isRoundtrip);
            //ptree.put(getPairFieldName(PairField::Carriers), pair.carriers);

            boost::property_tree::write_json(buf, ptree, false);

            return buf.str();
        }
        catch(const std::exception& e)
        {
            std::cerr << "An error occured while serializing a pair: " << e.what() << '\n';
            return std::string();
        }
    }

    Pair getDeserialized(const std::string& serializedPair)
    {
        try
        {
            std::istringstream is(serializedPair);
            boost::property_tree::ptree ptree;
            boost::property_tree::read_json(is, ptree);
            Pair pair;

            pair.origin = ptree.get<std::string>(getPairFieldName(PairField::Origin));
            pair.destination = ptree.get<std::string>(getPairFieldName(PairField::Destination));
            pair.isOneWay = ptree.get<bool>(getPairFieldName(PairField::IsOneWay));
            pair.isRoundtrip = ptree.get<bool>(getPairFieldName(PairField::IsRoundtrip));
            for(const auto& node : ptree.get_child(getPairFieldName(PairField::Carriers)))
            {
                pair.carriers.emplace_back(node.second.get_value<std::string>());
            }

            return pair;
        }
        catch(const std::exception& e)
        {
            std::cerr << "An error occured while deserializing a pair: " << e.what() << '\n';
            return Pair();
        }
    }
}

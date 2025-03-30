#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "server-exceptions.h"

namespace Utils
{
    enum class PairField
    {
        Origin = 0,
        Destination,
        Type,
        FareCarrier
    };

    static const std::unordered_map<PairField, std::string> pairFields = {
        { PairField::Origin,            "origin" },
        { PairField::Destination,       "destination" },
        { PairField::Type,              "type" },
        { PairField::FareCarrier,       "fareCarrier" }
    };

    inline std::string getPairFieldName(const PairField val)
    {
        return pairFields.find(val)->second;
    }

    struct Pair
    {
        std::string origin;
        std::string destination;
        bool type; // 0 - one way, 1 - roundtrip
        std::string fareCarrier;

        std::string serialize() const
        {
            try
            {
                boost::property_tree::ptree ptree;
                std::ostringstream buf;

                ptree.put(getPairFieldName(PairField::Origin), origin);
                ptree.put(getPairFieldName(PairField::Destination), destination);
                ptree.put(getPairFieldName(PairField::Type), type);
                ptree.put(getPairFieldName(PairField::FareCarrier), fareCarrier);

                boost::property_tree::write_json(buf, ptree, false);

                return buf.str();
            }
            catch(const std::exception& e)
            {
                const std::string errorMessage = "An error occured while serializing a pair: " + std::string(e.what());
                std::cerr << errorMessage << '\n';
                throw HttpBadRequest(errorMessage);
            }
        }
    };

    static Pair parsePair(const std::string& serializedPair)
    {
        try
        {
            std::istringstream is(serializedPair);
            boost::property_tree::ptree ptree;
            boost::property_tree::read_json(is, ptree);
            Pair pair;

            pair.origin = ptree.get<std::string>(getPairFieldName(PairField::Origin));
            pair.destination = ptree.get<std::string>(getPairFieldName(PairField::Destination));
            pair.type = ptree.get<bool>(getPairFieldName(PairField::Type));
            pair.fareCarrier = ptree.get<std::string>(getPairFieldName(PairField::FareCarrier));

            return pair;
        }
        catch(const std::exception& e)
        {
            const std::string errorMessage = "An error occured while deserializing a pair: " + std::string(e.what());
            std::cerr << errorMessage << '\n';
            throw HttpBadRequest(errorMessage);
        }
    }
}

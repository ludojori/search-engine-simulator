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
        IsOneWay,
        IsRoundtrip,
        FareCarrier
    };

    static const std::unordered_map<PairField, std::string> pairFields = {
        { PairField::Origin,            "origin" },
        { PairField::Destination,       "destination" },
        { PairField::IsOneWay,          "isOneWay" },
        { PairField::IsRoundtrip,       "isRoundtrip" },
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
        bool isOneWay;
        bool isRoundtrip;
        std::string fareCarrier;

        std::string serialize() const
        {
            try
            {
                boost::property_tree::ptree ptree;
                std::ostringstream buf;

                ptree.put(getPairFieldName(PairField::Origin), origin);
                ptree.put(getPairFieldName(PairField::Destination), destination);
                ptree.put(getPairFieldName(PairField::IsOneWay), isOneWay);
                ptree.put(getPairFieldName(PairField::IsRoundtrip), isRoundtrip);
                ptree.put(getPairFieldName(PairField::FareCarrier), fareCarrier);

                boost::property_tree::write_json(buf, ptree, false);

                return buf.str();
            }
            catch(const std::exception& e)
            {
                const std::string errorMessage = "An error occured while serializing a pair: " + std::string(e.what());
                std::cerr << errorMessage << '\n';
                throw std::runtime_error(errorMessage);
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
            pair.isOneWay = ptree.get<bool>(getPairFieldName(PairField::IsOneWay));
            pair.isRoundtrip = ptree.get<bool>(getPairFieldName(PairField::IsRoundtrip));
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

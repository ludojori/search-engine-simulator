#pragma once

#include <string>
#include <unordered_map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "server-exceptions.h"

namespace Utils
{
    enum class FlightType
    {
        OneWay = 0,
        Roundtrip
    };

    enum class CabinType
    {
        Economy = 0,
        PremiumEconomy,
        Business,
        First
    };

    enum class FlightField
    {
        Origin = 0,
        Destination,
        Type,
        DepartureTime,
        ArrivalTime,
        FareCarrier,
        Price,
        Currency,
        Cabin
    };

    static const std::unordered_map<FlightField, std::string> flightFields = {
        { FlightField::Origin,         "origin" },
        { FlightField::Destination,    "destination" },
        { FlightField::Type,           "type" },
        { FlightField::DepartureTime,  "departureTime" },
        { FlightField::ArrivalTime,    "arrivalTime" },
        { FlightField::FareCarrier,    "fareCarrier" },
        { FlightField::Price,          "price" },
        { FlightField::Currency,       "currency" },
        { FlightField::Cabin,          "cabin" }
    };

    inline std::string getFlightFieldName(const FlightField val)
    {
        return flightFields.find(val)->second;
    }

    struct Flight
    {
        std::string origin;
        std::string destination;
        FlightType type;
        std::string departureTime;
        std::string arrivalTime;
        std::string fareCarrier;
        long double price;
        std::string currency;
        CabinType cabin;

        std::string serialize() const
        {
            try
            {
                boost::property_tree::ptree ptree;
                std::ostringstream buf;

                ptree.put(getFlightFieldName(FlightField::Origin), origin);
                ptree.put(getFlightFieldName(FlightField::Destination), destination);
                ptree.put(getFlightFieldName(FlightField::Type), static_cast<int>(type));
                ptree.put(getFlightFieldName(FlightField::DepartureTime), departureTime);
                ptree.put(getFlightFieldName(FlightField::ArrivalTime), arrivalTime);
                ptree.put(getFlightFieldName(FlightField::FareCarrier), fareCarrier);
                ptree.put(getFlightFieldName(FlightField::Price), price);
                ptree.put(getFlightFieldName(FlightField::Currency), currency);
                ptree.put(getFlightFieldName(FlightField::Cabin), static_cast<int>(cabin));

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

        static Flight parse(const std::string& serializedFlight)
        {
            try
            {
                boost::property_tree::ptree ptree;
                std::istringstream buf(serializedFlight);

                boost::property_tree::read_json(buf, ptree);

                Flight flight {
                    .origin = ptree.get<std::string>(getFlightFieldName(FlightField::Origin)),
                    .destination = ptree.get<std::string>(getFlightFieldName(FlightField::Destination)),
                    .type = static_cast<FlightType>(ptree.get<int>(getFlightFieldName(FlightField::Type))),
                    .departureTime = ptree.get<std::string>(getFlightFieldName(FlightField::DepartureTime)),
                    .arrivalTime = ptree.get<std::string>(getFlightFieldName(FlightField::ArrivalTime)),
                    .fareCarrier = ptree.get<std::string>(getFlightFieldName(FlightField::FareCarrier)),
                    .price = ptree.get<float>(getFlightFieldName(FlightField::Price)),
                    .currency = ptree.get<std::string>(getFlightFieldName(FlightField::Currency)),
                    .cabin = static_cast<CabinType>(ptree.get<int>(getFlightFieldName(FlightField::Cabin)))
                };

                return flight;
            }
            catch(const std::exception& e)
            {
                const std::string errorMessage = "An error occured while deserializing a flight: " + std::string(e.what());
                std::cerr << errorMessage << '\n';
                throw HttpBadRequest(errorMessage);
            }
        }
    };
}
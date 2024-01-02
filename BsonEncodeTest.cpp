#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <string>
#include <any>
#include <fstream>
#include <chrono>
#include "bson.hpp"


int main()
{
    std::string profileImage;
    std::cout << "Enter path to profile picture: ";
    std::cin >> profileImage;
    std::ifstream fileStream{profileImage, std::ios::binary};
    if(!fileStream.is_open()) throw std::runtime_error{"\nCouldn't open " + profileImage};
    
    fileStream.seekg(0, std::ios::end);
    std::vector<uint8_t> passport;
    passport.resize(fileStream.tellg());
    fileStream.seekg(0, std::ios::beg);
    fileStream.read((char*)passport.data(), passport.size());

    long double income_per_second{5.29988771002991000100288277700923008};

    std::unordered_map<std::string, std::any> bson{{
        {"age", double{50.59928729017065}},
        {"name", std::string{"Canon"}},
        {
            "account details", 
            canon::bson::document{{
                {"bank name", std::string{"Bank of America"}},
                {
                    "account numbers",
                    std::vector<std::any>{
                        int32_t{73920293},
                        int32_t{80095437},
                        canon::bson::document{{
                            {"credit card", bool{true}},
                            {"card number", int64_t{8893665365537220}},
                            {"expiration", canon::bson::utc_datetime{8893665365100251}},
                            {
                                "compatible banks", 
                                std::vector<std::any>{
                                    std::string{"v liberty"},
                                    std::string{"ms con"},
                                    std::string{"verve"},
                                    std::string{"master"}
                                }
                            }
                        }}
                    }
                }
            }}
        },
        {
            "properties",
            std::vector<std::any>{
                canon::bson::document{{
                    {"name", std::string{"hamburger street Chicago Illinois"}},
                    {
                        "detail",
                        canon::bson::document{{
                            {"price", int32_t{13000}},
                            {"address", std::string{"306 main land avenue"}}
                        }}
                    }
                }},
                canon::bson::document{{
                    {"name", std::string{"swell view street Miami Florida"}},
                    {
                        "detail",
                        canon::bson::document{{
                            {"price", int32_t{16500}},
                            {"address", std::string{"12 oak bank"}}
                        }}
                    }
                }}
            }
        },
        {"profile image", std::make_pair(uint8_t{0x00}, std::move(passport))},
        {"gender", canon::bson::undefined()},
        {"developer", bool{true}},
        {"date registered", canon::bson::utc_datetime{34082957760020}},
        {"debt", nullptr},
        {"profile pattern", std::make_pair(std::string{"acme*"}, std::string{"i"})},
        {
            "safe box id", 
            std::make_pair(
                std::string{"safe box 225"},
                std::vector<uint8_t>{
                    0x0C, 0xBB, 0x19, 0x20, 0xFF, 0x48, 
                    0xC0, 0xFC, 0x91, 0x89, 0x7A, 0xEE
                }
            )
        },
        {"auth code", canon::bson::javascript_code{"() => { return 1; };"}},
        {"signature", canon::bson::symbol{"@@##$$@@"}},
        {
            "api key generator", 
            std::make_pair(
                std::string{"() => { return 2; };"},
                canon::bson::document{{
                    {"anonymous function", std::string{"ground scope"}}
                }}
            )
        },
        {"income per second", std::move(income_per_second)},
        {"position", canon::bson::min_key()},
        {"mitigation", canon::bson::max_key()}
    }};
    auto encode_time_start{std::chrono::system_clock::now()};
    std::vector<uint8_t> bson_data{canon::bson::document{bson}.data()};
    auto encode_time_end{std::chrono::system_clock::now()};
    std::chrono::duration<double> encode_time{encode_time_end - encode_time_start};
    std::cout << std::setprecision(500) << "time taken to encode: " << encode_time.count() << '\n';

    std::ofstream outputFileStream{"Test_Document.bson", std::ios::binary};
    outputFileStream.write((char*)bson_data.data(), bson_data.size());

    return 0;
};
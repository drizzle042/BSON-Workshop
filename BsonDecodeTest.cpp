#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include "bson.hpp"


int main()
{
		/**********************************BSON Test Begin************************************************/
		std::string file;
		std::cout << "Enter a file to read: ";
		std::cout << std::setprecision(500);
		std::cin >> file;

		std::ifstream bsonFileStream{file, std::ios::binary};
		while (!bsonFileStream.is_open())
		{ 
			std::cout << "Couldn't open file, try again: ";
			std::cin >> file;
			bsonFileStream.open(file, std::ios::binary);
		}

		bsonFileStream.seekg(0, std::ios::end);
		std::vector<uint8_t> contents;
		contents.resize(bsonFileStream.tellg());
		bsonFileStream.seekg(0, std::ios::beg);
		bsonFileStream.read((char*)contents.data(), contents.size());

		auto doc_init_time_start{std::chrono::steady_clock::now()};
		canon::bson::document doc{contents.data()};
		auto doc_init_time_end{std::chrono::steady_clock::now()};
		std::chrono::duration<double> doc_init_time{doc_init_time_end - doc_init_time_start};
		std::cout << "time taken to decode: " << doc_init_time.count() << '\n';

		std::cout << "age: " << doc.get<double&>("age") << ",\n";
		std::cout << "name: " << doc.get<std::string&>("name") << ",\n";
		std::cout << "account details: {\n";
		auto& account_details = doc.get<canon::bson::document&>("account details");
		std::cout << "\tbank name: "<< account_details.get<std::string&>("bank name") << ",\n";
		auto& account_numbers = account_details.get<std::vector<std::any>&>("account numbers");
		std::cout << "\taccount numbers: [\n";
		std::cout << "\t\t" << std::any_cast<int32_t&>(account_numbers[0]) << ",\n";
		std::cout << "\t\t" << std::any_cast<int32_t&>(account_numbers[1]) << ",\n";
		auto& account_number = std::any_cast<canon::bson::document&>(account_numbers[2]);
		std::cout << "\t\t{\n";
		std::cout << "\t\t\t" << "credit card: " << std::boolalpha << account_number.get<bool&>("credit card") << ",\n";
		std::cout << "\t\t\t" << "card number: " << account_number.get<int64_t&>("card number") << ",\n";
		std::cout << "\t\t\t" << "expiration: " << (uint64_t)account_number.get<canon::bson::utc_datetime&>("expiration") << ",\n";
		auto& compatible_banks = account_number.get<std::vector<std::any>&>("compatible banks");
		std::cout << "\t\t\tcompatible banks: [\n";
		for (auto& i : compatible_banks)
		{ std::cout << "\t\t\t\t" << std::any_cast<std::string&>(i) << ",\n"; }
		std::cout << "\t\t\t]\n";
		std::cout << "\t\t}\n";
		std::cout << "\t]\n";
		std::cout << "},\n";
		std::cout << "properties: [\n";
		auto& properties = doc.get<std::vector<std::any>&>("properties");
		for(auto& i : properties)
		{
			std::cout << "\t{\n";
			auto& property = std::any_cast<canon::bson::document&>(i);
			std::cout << "\t\tname: " << property.get<std::string&>("name") << ",\n";
			std::cout << "\t\tdetail: {\n";
			auto& propertyDetail = property.get<canon::bson::document&>("detail");
			std::cout << "\t\t\tprice: " << propertyDetail.get<int32_t&>("price") << " dollars,\n";
			std::cout << "\t\t\taddress: " << propertyDetail.get<std::string&>("address") << ",\n";
			std::cout << "\t\t}\n";
			std::cout << "\t},\n";
		}
		std::cout << "],\n";
		auto& profileImage = doc.get<std::pair<uint8_t, std::vector<uint8_t>>&>("profile image");
		std::ofstream profileImageFileStream{"profile image.jpg", std::ios::binary};
		profileImageFileStream.write((char*)profileImage.second.data(), profileImage.second.size());
		std::cout << "gender: " << (void*)(std::nullptr_t)doc.get<canon::bson::undefined&>("gender") << ",\n";
		std::cout << "developer: " << doc.get<bool&>("developer") << ",\n";
		std::chrono::system_clock::time_point date_time_point{std::chrono::seconds((int64_t)doc.get<canon::bson::utc_datetime&>("date registered"))};
		std::time_t date = std::chrono::system_clock::to_time_t(date_time_point);
		std::cout << "date registered: " << std::ctime(&date);
		std::cout << "debt: " << (void*)doc.get<std::nullptr_t>("debt") << '\n';
		auto& regex = doc.get<std::pair<std::string, std::string>&>("profile pattern");
		std::cout << "profile pattern: " << regex.first << " " << regex.second << ",\n";
		auto& safeBoxId = doc.get<std::pair<std::string, std::vector<uint8_t>>&>("safe box id");
		std::cout << "safe box id: {\n";
		std::cout << "\tname: " << safeBoxId.first << ",\n";
		std::cout << "\tpointer: ";
		for(auto& i : safeBoxId.second){ std::cout << std::hex << "0x" << (int)i << " "; }
		std::cout << "\n},\n";
		std::cout << "auth code: " << std::string{doc.get<canon::bson::javascript_code>("auth code")} << ",\n";
		std::cout << "signature: " << std::string{doc.get<canon::bson::symbol>("signature")} << ",\n";
		auto& javascript_code_with_scope = doc.get<std::pair<std::string, canon::bson::document>&>("api key generator");
		std::cout << "api key generator: {\n";
		std::cout << "\tcode: '" << javascript_code_with_scope.first << "',\n";
		std::cout << "\tscope: {\n";
		std::cout << "\t\tanonymous function: " << javascript_code_with_scope.second.get<std::string&>("anonymous function") << "\n";
		std::cout << "\t}\n";
		std::cout << "},\n";
		std::cout << "income per second: " << doc.get<long double&>("income per second") << ",\n";
		std::cout << "position: " << (void*)(std::nullptr_t)doc.get<canon::bson::min_key&>("position") << ",\n";
		std::cout << "mitigation: " << (void*)(std::nullptr_t)doc.get<canon::bson::max_key&>("mitigation") << ",\n";
		/*****************************************BSON Test End**************************************/
};
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
		std::cout << std::setprecision(500);
		std::cout << "Enter a file to read: ";
		std::cin >> file;

		std::ifstream bsonFileStream{file, std::ios::binary};
		while (!bsonFileStream.is_open())
		{ 
			std::cout << "Couldn't open file, try again: ";
			std::cin >> file;
			bsonFileStream.open(file, std::ios::binary);
		}

		auto file_copy_time_start{std::chrono::steady_clock::now()};
		std::vector<uint8_t> contents;
		bsonFileStream.seekg(0, bsonFileStream.end);
		contents.reserve(bsonFileStream.tellg());
		contents.resize(bsonFileStream.tellg());
		bsonFileStream.seekg(0, bsonFileStream.beg);
		bsonFileStream.read((char*)contents.data(), contents.size());
		auto file_copy_time_end{std::chrono::steady_clock::now()};
		std::chrono::duration<double> file_copy_time{file_copy_time_end - file_copy_time_start};
		std::cout << "time taken to copy file contents: " << file_copy_time.count() << "\n";

		auto doc_init_time_start{std::chrono::steady_clock::now()};
		canon::bson::document doc{contents.data()};
		auto doc_init_time_end{std::chrono::steady_clock::now()};
		std::chrono::duration<double> doc_init_time{doc_init_time_end - doc_init_time_start};
		std::cout << "time taken to initialize document and complete decoding: " << doc_init_time.count() << "\n";

		auto console_out_time_start{std::chrono::steady_clock::now()};
		std::cout << "_id: ";
		for (auto& i : doc.get<std::vector<uint8_t>&>("_id"))
		{ std::cout << "0x" << std::hex << (uint32_t)i << " "; }
		std::cout << std::dec << "\n";
		std::cout << "age: " << doc.get<double&>("age") << ",\n";
		std::cout << "name: " << doc.get<std::string&>("name") << ",\n";
		std::cout << "account details: {\n";
		auto& account_details = doc.get<canon::bson::document&>("account details");
		std::cout << "\tbank name: "<< account_details.get<std::string&>("bank name") << ",\n";
		auto& account_numbers = account_details.get<std::vector<std::any>&>("account numbers");
		std::cout << "\taccount numbers: [\n";
		std::cout << "\t\t" << std::any_cast<long int&>(account_numbers[0]) << ",\n";
		std::cout << "\t\t" << std::any_cast<long int&>(account_numbers[1]) << ",\n";
		auto& account_number = std::any_cast<canon::bson::document&>(account_numbers[2]);
		std::cout << "\t\t{\n";
		std::cout << "\t\t\t" << "credit card: " << std::boolalpha << account_number.get<bool&>("credit card") << ",\n";
		std::cout << "\t\t\t" << "card number: " << account_number.get<int32_t&>("card number") << ",\n";
		auto& compatible_banks = account_number.get<std::vector<std::any>&>("compatible banks");
		std::cout << "\t\t\tcompatible banks: [\n";
		for (auto& i : compatible_banks)
		{ std::cout << "\t\t\t\t" << std::any_cast<std::string&>(i) << ",\n"; }
		std::cout << "\t\t\t]\n";
		std::cout << "\t\t}\n";
		std::cout << "\t]\n";
		std::cout << "}\n";
		std::cout << "developer: " << doc.get<bool&>("developer") << ",\n";
		std::cout << "gender: " << (void*)doc.get<std::nullptr_t&>("gender") << ",\n";
		std::chrono::system_clock::time_point date_time_point{std::chrono::seconds(doc.get<int64_t&>("date registered"))};
		std::time_t date = std::chrono::system_clock::to_time_t(date_time_point);
		std::cout << "date registered: " << std::ctime(&date);
		auto& regex = doc.get<std::pair<std::string, std::string>&>("profile pattern");
		std::cout << "profile pattern: " << regex.first << " " << regex.second << ",\n";
		auto& javascript_code_with_scope = doc.get<std::pair<std::string, canon::bson::document>&>("auth code");
		std::cout << "auth code: '" << javascript_code_with_scope.first << "',\n";
		std::cout << "last login: " << std::chrono::duration<uint64_t>(doc.get<uint64_t&>("last login")).count() << "s,\n";
		std::cout << "mortgage credit id: " << doc.get<int64_t&>("mortgage credit id") << ",\n";
		std::cout << "user id: " << doc.get<int32_t&>("user id") << ",\n";
		std::cout << "user symbol: " << doc.get<std::string&>("user symbol") << ",\n";
		std::cout << "income per second: " << doc.get<long double&>("income per second") << ",\n";
		std::cout << "max: " << (void*)doc.get<std::nullptr_t&>("max") << ",\n";
		std::cout << "min: " << (void*)doc.get<std::nullptr_t&>("min") << "\n";
		auto console_out_time_end{std::chrono::steady_clock::now()};
		std::chrono::duration<double> console_out_time{console_out_time_end - console_out_time_start};
		std::cout << "time taken to output document to console: " << console_out_time.count() << "\n";
		/*****************************************BSON Test End**************************************/
};

#ifndef __BSON_H__
#define __BSON_H__

#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <functional>
#include <typeindex>
#include <any>


namespace canon
{
    namespace bson
    {
        class undefined
        {
        public:
            operator std::nullptr_t();
        };

        class utc_datetime
        {
        public:
            explicit utc_datetime(const int64_t&);
            operator int64_t&();
        private:
            int64_t __datetime;
        };

        class javascript_code
        {
        public:
            explicit javascript_code(const std::string&);
            operator std::string&();
        private:
            std::string __code;
        };

        class symbol
        {
        public:
            explicit symbol(const std::string&);
            operator std::string&();
        private:
            std::string __symbol;
        };

        class min_key
        {
        public:
            operator std::nullptr_t();
        };

        class max_key
        {
        public:
            operator std::nullptr_t();
        };

        class document
        {
        public:
            explicit document(const uint8_t*);
            explicit document(const std::unordered_map<std::string, std::any>&);

            template<typename T>
            T get(const std::string&);
            std::vector<uint8_t> data();
            size_t size();

        private:
            std::vector<uint8_t> __encode();
            void __decode(const uint8_t* input);
            std::unordered_map<std::string, std::any> __buffer;
            int32_t __size;
        };
    
/*******************************************Start Implementation**************************************************/
        static_assert(sizeof(uint8_t) == 1, "Size of uint8_t must be exactly 1 bytes as specified in ISO C++");
        static_assert(sizeof(uint32_t) == 4, "Size of uint32_t must be exactly 4 bytes as specified in ISO C++");
        static_assert(sizeof(uint64_t) == 8, "Size of uint64_t must be exactly 8 bytes as specified in ISO C++");
        static_assert(sizeof(long double) == 16, "Size of long double should be 16 bytes to represent decimal128");

        inline undefined::operator std::nullptr_t()
        { return nullptr; }

        inline utc_datetime::utc_datetime(const int64_t& input)
        : __datetime{input} {}

        inline utc_datetime::operator int64_t&()
        { return this->__datetime; }

        inline javascript_code::javascript_code(const std::string& input)
        : __code{input} {}

        inline javascript_code::operator std::string&()
        { return this->__code; }

        inline symbol::symbol(const std::string& input)
        : __symbol{input} {}

        inline symbol::operator std::string&()
        { return this->__symbol; }

        inline min_key::operator std::nullptr_t()
        { return nullptr; }

        inline max_key::operator std::nullptr_t()
        { return nullptr; }

        inline document::document(const uint8_t* input)
        { 
            if (input)
            {
                this->__size = *reinterpret_cast<const int32_t*>(input);
                this->__decode(input);
            }
        }

        inline document::document(const std::unordered_map<std::string, std::any>& input)
        : __buffer{input} {}

        template<typename T>
        inline T document::get(const std::string& key)
        { 
            auto it = this->__buffer.find(key);
            return std::any_cast<T>(it->second);
        }

        inline std::vector<uint8_t> document::data()
        { return this->__encode(); }

        inline size_t document::size()
        { return __size; }

        inline std::vector<uint8_t> document::__encode()
        {
            static std::unordered_map<
                std::type_index, 
                std::function<
                    void(
                        const std::string&,
                        std::any&,
                        std::vector<uint8_t>&
                    )
                >
            > interpreter
            {
                {   /* 'element ::= "0x01" e_name double'                        --64-bit binary floating point */
                    std::type_index{typeid(double)}, 
                    [](
                        const std::string& key,
                        std::any& value,
                        std::vector<uint8_t>& buffer
                    ) -> void
                    {                                                                  /*************************/
                        buffer.push_back(0x01);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()),              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()) + key.size()  /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /* handle double         */
                        auto double_value = std::any_cast<double>(&value);             /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<uint8_t*>(double_value),                  /*************************/
                            reinterpret_cast<uint8_t*>(double_value) + sizeof(double)  /*************************/
                        );                                                             /*************************/
                    }                                                                  /*************************/
                },
                {   /* 'element ::= "0x02" e_name string'                                        --utf-8 string */
                    std::type_index{typeid(std::string)},
                    [](
                        const std::string& key,
                        std::any& value,
                        std::vector<uint8_t>& buffer
                    ) -> void
                    {                                                                  /*************************/
                        buffer.push_back(0x02);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()),              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()) + key.size()  /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /*************************/
                        auto string_value = std::any_cast<std::string>(&value);        /*************************/
                        int32_t string_size = string_value->size() + 1;                /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /* handle string         */
                            reinterpret_cast<uint8_t*>(&string_size),                  /*************************/
                            reinterpret_cast<uint8_t*>(                                /*************************/
                                &string_size                                           /*************************/
                            ) + sizeof(int32_t)                                        /*************************/
                        );                                                             /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<uint8_t*>(string_value->data()),          /*************************/
                            reinterpret_cast<uint8_t*>(                                /*************************/
                                string_value->data()                                   /*************************/
                            ) + string_value->size()                                   /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /*************************/
                    }                                                                  /*************************/
                },
                {   /* 'element ::= "0x03" e_name document'                                 --embedded document */
                    std::type_index{typeid(document)},
                    [](
                        const std::string& key,
                        std::any& value,
                        std::vector<uint8_t>& buffer
                    ) -> void
                    {                                                                  /*************************/
                        buffer.push_back(0x03);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()),              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()) + key.size()  /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /* handle document       */
                        auto document_value = std::any_cast<document>(&value);         /*************************/
                        std::vector<uint8_t> document_data{                            /*************************/
                            document_value->data()                                     /*************************/
                        };                                                             /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            document_data.begin(),                                     /*************************/
                            document_data.end()                                        /*************************/
                        );                                                             /*************************/
                    }                                                                  /*************************/
                },
                {   /* 'element ::= "0x04" e_name document'                                             --array */
                    std::type_index{typeid(std::vector<std::any>)},
                    [](
                        const std::string& key,
                        std::any& value,
                        std::vector<uint8_t>& buffer
                    ) -> void
                    {                                                                  /*************************/
                        buffer.push_back(0x04);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()),              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()) + key.size()  /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /*************************/
                        auto array_value = std::any_cast<std::vector<std::any>>(       /*************************/
                                                &value                                 /*************************/
                                           );                                          /*************************/
                        std::unordered_map<std::string, std::any> buf;                 /*************************/
                        for(size_t i = 0; i < array_value->size(); ++i)                /* handle array          */
                        {                                                              /*************************/
                            buf[std::to_string(i)] = (*array_value)[i];                /*************************/
                        }                                                              /*************************/
                        std::vector<uint8_t> array_data{                               /*************************/
                            document{buf}.data()                                       /*************************/
                        };                                                             /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            array_data.begin(),                                        /*************************/
                            array_data.end()                                           /*************************/
                        );                                                             /*************************/
                    }                                                                  /*************************/
                },
                {   /* 'element ::= "0x05" e_name binary'                                         --binary data */
                    std::type_index{typeid(std::pair<uint8_t, std::vector<uint8_t>>)},
                    [](
                        const std::string& key,
                        std::any& value,
                        std::vector<uint8_t>& buffer
                    ) -> void
                    {                                                                  /*************************/
                        buffer.push_back(0x05);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()),              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()) + key.size()  /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /*************************/
                        auto binary_value = std::any_cast<                             /*************************/
                                                std::pair<                             /*************************/
                                                    uint8_t,                           /*************************/
                                                    std::vector<uint8_t>               /*************************/
                                                >                                      /* handle binary         */
                                            >(&value);                                 /*************************/
                        int32_t binary_size = binary_value->second.size();             /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<uint8_t*>(&binary_size),                  /*************************/
                            reinterpret_cast<uint8_t*>(                                /*************************/
                                &binary_size                                           /*************************/
                            ) + sizeof(int32_t)                                        /*************************/
                        );                                                             /*************************/
                        buffer.push_back(binary_value->first);                         /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            binary_value->second.begin(),                              /*************************/
                            binary_value->second.end()                                 /*************************/
                        );                                                             /*************************/
                    }                                                                  /*************************/
                },
                {   /* 'element ::= "0x06" e_name     '                                             --undefined */
                    std::type_index{typeid(undefined)},
                    [](
                        const std::string& key,
                        std::any& value,
                        std::vector<uint8_t>& buffer
                    ) -> void
                    {                                                                  /*************************/
                        buffer.push_back(0x06);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()),              /* handle undefined      */
                            reinterpret_cast<const uint8_t*>(key.data()) + key.size()  /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /*************************/
                    }                                                                  /*************************/
                },
                {   /* 'element ::= "0x07" e_name (byte*12)'                                         --objectid */
                    std::type_index{typeid(std::vector<uint8_t>)},
                    [](
                        const std::string& key,
                        std::any& value,
                        std::vector<uint8_t>& buffer
                    ) -> void
                    {                                                                  /*************************/
                        buffer.push_back(0x07);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()),              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()) + key.size()  /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /*************************/
                        auto objectid_value = std::any_cast<std::vector<uint8_t>>(     /* handle objectid       */
                                                  &value                               /*************************/
                                              );                                       /*************************/
                        std::copy(                                                     /*************************/
                            buffer.end(),                                              /*************************/
                            objectid_value->begin(),                                   /*************************/
                            objectid_value->end()                                      /*************************/
                        );                                                             /*************************/
                    }                                                                  /*************************/
                },
                {   /* 'element ::= "0x08" e_name boolean'                                            --boolean */
                    std::type_index{typeid(bool)},
                    [](
                        const std::string& key,
                        std::any& value,
                        std::vector<uint8_t>& buffer
                    ) -> void
                    {                                                                  /*************************/
                        buffer.push_back(0x08);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()),              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()) + key.size()  /* handle boolean        */
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /*************************/
                        auto bool_value = std::any_cast<bool>(&value);                 /*************************/
                        buffer.push_back(*reinterpret_cast<uint8_t*>(bool_value));     /*************************/
                    }                                                                  /*************************/
                },
                {   /* 'element ::= "0x09" e_name int64'                                         --utc datetime */
                    std::type_index{typeid(utc_datetime)},
                    [](
                        const std::string& key,
                        std::any& value,
                        std::vector<uint8_t>& buffer
                    ) -> void
                    {                                                                  /*************************/
                        buffer.push_back(0x09);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()),              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()) + key.size()  /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /*************************/
                        int64_t& utc_datetime_value = std::any_cast<utc_datetime&>(    /*************************/
                                                        value                          /*************************/
                                                      );                               /* handle utc datetime   */
                                                                                       /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<uint8_t*>(&utc_datetime_value),           /*************************/
                            reinterpret_cast<uint8_t*>(                                /*************************/
                                &utc_datetime_value                                    /*************************/
                            ) + sizeof(int64_t)                                        /*************************/
                        );                                                             /*************************/
                    }                                                                  /*************************/
                },
                {   /* 'element ::= "0x0A" e_name    '                                                   --null */
                    std::type_index{typeid(std::nullptr_t)},
                    [](
                        const std::string& key,
                        std::any& value,
                        std::vector<uint8_t>& buffer
                    ) -> void
                    {                                                                  /*************************/
                        buffer.push_back(0x0A);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()),              /* handle null           */
                            reinterpret_cast<const uint8_t*>(key.data()) + key.size()  /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /*************************/
                    }                                                                  /*************************/
                },
                {   /* 'element ::= "0x0B" e_name cstring cstring'                                      --regex */
                    std::type_index{typeid(std::pair<std::string, std::string>)},
                    [](
                        const std::string& key,
                        std::any& value,
                        std::vector<uint8_t>& buffer
                    ) -> void
                    {                                                                  /*************************/
                        buffer.push_back(0x0B);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()),              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()) + key.size()  /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /*************************/
                        auto regex_value = std::any_cast<                              /*************************/
                                                std::pair<std::string, std::string>    /*************************/
                                            >(&value);                                 /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /* handle regex          */
                            reinterpret_cast<uint8_t*>(                                /*************************/
                                regex_value->first.data()                              /*************************/
                            ),                                                         /*************************/
                            reinterpret_cast<uint8_t*>(                                /*************************/
                                regex_value->first.data()                              /*************************/
                            ) + regex_value->first.size()                              /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<uint8_t*>(                                /*************************/
                                regex_value->second.data()                             /*************************/
                            ),                                                         /*************************/
                            reinterpret_cast<uint8_t*>(                                /*************************/
                                regex_value->second.data()                             /*************************/
                            ) + regex_value->second.size()                             /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /*************************/
                    }                                                                  /*************************/
                },
                {   /* 'element ::= "0x0C" e_name string (byte*12)'                         --unicode dbpointer */
                    std::type_index{typeid(std::pair<std::string, std::vector<uint8_t>>)},
                    [](
                        const std::string& key,
                        std::any& value,
                        std::vector<uint8_t>& buffer
                    ) -> void
                    {                                                                  /*************************/
                        buffer.push_back(0x0C);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()),              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()) + key.size()  /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /*************************/
                        auto dbpointer_value = std::any_cast<                          /*************************/
                                                  std::pair<                           /*************************/
                                                     std::string,                      /*************************/
                                                     std::vector<uint8_t>              /*************************/
                                                  >                                    /*************************/
                                               >(&value);                              /*************************/
                        int32_t string_size = dbpointer_value->first.size() + 1;       /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<uint8_t*>(&string_size),                  /* handle dbpointer      */
                            reinterpret_cast<uint8_t*>(                                /*************************/
                                &string_size                                           /*************************/
                            ) + sizeof(int32_t)                                        /*************************/
                        );                                                             /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<uint8_t*>(                                /*************************/
                                dbpointer_value->first.data()                          /*************************/
                            ),                                                         /*************************/
                            reinterpret_cast<uint8_t*>(                                /*************************/
                                dbpointer_value->first.data()                          /*************************/
                            ) + dbpointer_value->first.size()                          /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            dbpointer_value->second.begin(),                           /*************************/
                            dbpointer_value->second.end()                              /*************************/
                        );                                                             /*************************/
                    }                                                                  /*************************/
                },
                {   /* 'element ::= "0x0D" e_name string'                                     --javascript code */
                    std::type_index{typeid(javascript_code)},
                    [](
                        const std::string& key,
                        std::any& value,
                        std::vector<uint8_t>& buffer
                    ) -> void
                    {                                                                  /*************************/
                        buffer.push_back(0x0D);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()),              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()) + key.size()  /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /*************************/
                        std::string& code_value = std::any_cast<javascript_code&>(     /*************************/
                                                    value                              /*************************/
                                                  );                                   /*************************/
                        int32_t code_size = code_value.size() + 1;                     /*************************/
                        buffer.insert(                                                 /* handle javascript     */
                            buffer.end(),                                              /* code                  */
                            reinterpret_cast<uint8_t*>(&code_size),                    /*************************/
                            reinterpret_cast<uint8_t*>(&code_size) + sizeof(int32_t)   /*************************/
                        );                                                             /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<uint8_t*>(code_value.data()),             /*************************/
                            reinterpret_cast<uint8_t*>(                                /*************************/
                                code_value.data()                                      /*************************/
                            ) + code_value.size()                                      /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /*************************/
                    }                                                                  /*************************/
                },
                {   /* 'element ::= "0x0E" e_name string'                                              --symbol */
                    std::type_index{typeid(symbol)},
                    [](
                        const std::string& key,
                        std::any& value,
                        std::vector<uint8_t>& buffer
                    ) -> void
                    {                                                                  /*************************/
                        buffer.push_back(0x0E);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()),              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()) + key.size()  /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /*************************/
                        std::string& symbol_value = std::any_cast<symbol&>(value);     /*************************/
                        int32_t symbol_size = symbol_value.size() + 1;                 /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /* handle symbol         */
                            reinterpret_cast<uint8_t*>(&symbol_size),                  /*************************/
                            reinterpret_cast<uint8_t*>(&symbol_size) + sizeof(int32_t) /*************************/
                        );                                                             /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<uint8_t*>(symbol_value.data()),           /*************************/
                            reinterpret_cast<uint8_t*>(                                /*************************/
                                symbol_value.data()                                    /*************************/
                            ) + symbol_value.size()                                    /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /*************************/
                    }                                                                  /*************************/
                },
                {   /* 'element ::= "0x0F" e_name code_w_s'                        --javascript code with scope */
                    std::type_index{typeid(std::pair<std::string, document>)},
                    [](
                        const std::string& key,
                        std::any& value,
                        std::vector<uint8_t>& buffer
                    ) -> void
                    {                                                                  /*************************/
                        buffer.push_back(0x0F);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()),              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()) + key.size()  /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /*************************/
                        auto code_w_scope_value = std::any_cast<                       /*************************/
                                                    std::pair<std::string, document>   /*************************/
                                                  >(&value);                           /*************************/
                        std::vector<uint8_t> document_data{                            /*************************/
                            code_w_scope_value->second.data()                          /*************************/
                        };                                                             /*************************/
                        int32_t total_size = (                                         /*************************/
                                                sizeof(int32_t)                        /*************************/
                                                + sizeof(int32_t)                      /*************************/
                                                + code_w_scope_value->first.size()     /*************************/
                                                + 1                                    /*************************/
                                                + document_data.size()                 /*************************/
                                             );                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<uint8_t*>(&total_size),                   /*************************/
                            reinterpret_cast<uint8_t*>(&total_size) + sizeof(int32_t)  /* handle javascript     */
                        );                                                             /* code with scope       */
                        int32_t string_size = code_w_scope_value->first.size() + 1;    /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<uint8_t*>(&string_size),                  /*************************/
                            reinterpret_cast<uint8_t*>(                                /*************************/
                                &string_size                                           /*************************/
                            ) + sizeof(int32_t)                                        /*************************/
                        );                                                             /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<const uint8_t*>(                          /*************************/
                                code_w_scope_value->first.data()                       /*************************/
                            ),                                                         /*************************/
                            reinterpret_cast<const uint8_t*>(                          /*************************/
                                code_w_scope_value->first.data()                       /*************************/
                            ) + code_w_scope_value->first.size()                       /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            document_data.begin(),                                     /*************************/
                            document_data.end()                                        /*************************/
                        );                                                             /*************************/
                    }                                                                  /*************************/
                },
                {   /* 'element ::= "0x10" e_name int32'                                       --32-bit integer */
                    std::type_index{typeid(int32_t)},
                    [](
                        const std::string& key,
                        std::any& value,
                        std::vector<uint8_t>& buffer
                    ) -> void
                    {                                                                  /*************************/
                        buffer.push_back(0x10);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()),              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()) + key.size()  /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /* handle 32 bit integer */
                        auto int32_value = std::any_cast<int32_t>(&value);             /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<uint8_t*>(int32_value),                   /*************************/
                            reinterpret_cast<uint8_t*>(int32_value) + sizeof(int32_t)  /*************************/
                        );                                                             /*************************/
                    }                                                                  /*************************/
                },
                {   /* 'element ::= "0x11" e_name uint64'                                           --timestamp */
                    std::type_index{typeid(uint64_t)},
                    [](
                        const std::string& key,
                        std::any& value,
                        std::vector<uint8_t>& buffer
                    ) -> void
                    {                                                                  /*************************/
                        buffer.push_back(0x11);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()),              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()) + key.size()  /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /*************************/
                        auto timestamp_value = std::any_cast<uint64_t>(&value);        /* handle timestamp      */
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<uint8_t*>(timestamp_value),               /*************************/
                            reinterpret_cast<uint8_t*>(                                /*************************/
                                timestamp_value                                        /*************************/
                            ) + sizeof(uint64_t)                                       /*************************/
                        );                                                             /*************************/
                    }                                                                  /*************************/
                },
                {   /* 'element ::= "0x12" e_name int64'                                       --64-bit integer */
                    std::type_index{typeid(int64_t)},
                    [](
                        const std::string& key,
                        std::any& value,
                        std::vector<uint8_t>& buffer
                    ) -> void
                    {                                                                  /*************************/
                        buffer.push_back(0x12);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()),              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()) + key.size()  /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /* handle 64 bit integer */
                        auto int64_value = std::any_cast<int64_t>(&value);             /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<uint8_t*>(int64_value),                   /*************************/
                            reinterpret_cast<uint8_t*>(int64_value) + sizeof(int64_t)  /*************************/
                        );                                                             /*************************/
                    }                                                                  /*************************/
                },
                {   /* 'element ::= "0x13" e_name decimal128'                  --128-bit decimal floating point */
                    std::type_index{typeid(long double)},
                    [](
                        const std::string& key,
                        std::any& value,
                        std::vector<uint8_t>& buffer
                    ) -> void
                    {                                                                  /*************************/
                        buffer.push_back(0x13);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()),              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()) + key.size()  /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /* handle 128 bit        */
                        auto decimal_value = std::any_cast<long double>(&value);       /* decimal floating      */
                        buffer.insert(                                                 /* point                 */
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<uint8_t*>(decimal_value),                 /*************************/
                            reinterpret_cast<uint8_t*>(                                /*************************/
                                decimal_value                                          /*************************/
                            ) + sizeof(long double)                                    /*************************/
                        );                                                             /*************************/
                    }                                                                  /*************************/
                },
                {   /* 'element ::= "0xFF" e_name       '                                             --min key */
                    std::type_index{typeid(min_key)},
                    [](
                        const std::string& key,
                        std::any& value,
                        std::vector<uint8_t>& buffer
                    ) -> void
                    {                                                                  /*************************/
                        buffer.push_back(0xFF);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()),              /* handle min key        */
                            reinterpret_cast<const uint8_t*>(key.data()) + key.size()  /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /*************************/
                    }                                                                  /*************************/
                },
                {   /* 'element ::= "0x7F" e_name       '                                             --max key */
                    std::type_index{typeid(max_key)},
                    [](
                        const std::string& key,
                        std::any& value,
                        std::vector<uint8_t>& buffer
                    ) -> void
                    {                                                                  /*************************/
                        buffer.push_back(0x7F);                                        /*************************/
                        buffer.insert(                                                 /*************************/
                            buffer.end(),                                              /*************************/
                            reinterpret_cast<const uint8_t*>(key.data()),              /* handle max key        */
                            reinterpret_cast<const uint8_t*>(key.data()) + key.size()  /*************************/
                        );                                                             /*************************/
                        buffer.push_back(0x00);                                        /*************************/
                    }                                                                  /*************************/
                }
            };
            std::vector<uint8_t> buffer(4, 0);                               /* reserve space for docment size  */
            for(auto& i : this->__buffer)                                    /***********************************/
            {                                                                /***********************************/
                auto& match = interpreter[std::type_index{i.second.type()}]; /* encode element based on type    */
                match(i.first, i.second, buffer);                            /***********************************/
            }                                                                /***********************************/
            buffer.push_back(0x00);                                          /* add null terminal to the end    */
            int32_t bson_size = buffer.size();                               /***********************************/
            std::copy(                                                       /* deduce document size and        */
                reinterpret_cast<uint8_t*>(&bson_size),                      /* write to the space initally     */
                reinterpret_cast<uint8_t*>(&bson_size) + sizeof(int32_t),    /* reserved at the start           */
                buffer.data()                                                /***********************************/
            );                                                               /***********************************/
            return buffer;                                                   /***********************************/
        };

        inline void document::__decode(const uint8_t* input)
        {
            static std::unordered_map<
                uint8_t, 
                std::function<
                    void(
                        const uint8_t*, 
                        int32_t&, 
                        const std::string&, 
                        document*
                    )
                >
            > interpreter
            {
                {
                    0x01, /* 'element ::= "0x01" e_name double'                  --64-bit binary floating point */
                    [](
                        const uint8_t* input, 
                        int32_t& seek, 
                        const std::string& key, 
                        document* bson_t
                    ) -> void 
                    {   /* double ::= (byte*8)                                           ************************/
                        bson_t->__buffer[key] = *reinterpret_cast<const double*>(        /***********************/
                                                    &input[seek]                         /* handle double       */
                                                );                                       /***********************/
                        seek += sizeof(double);                                          /***********************/
                    }                                                                    /***********************/
                },
                {
                    0x02, /* 'element ::= "0x02" e_name string'                                  --utf-8 string */
                    [](
                        const uint8_t* input, 
                        int32_t& seek, 
                        const std::string& key, 
                        document* bson_t
                    ) -> void 
                    {   /* string ::= int32 (byte*) "0x00"                                     ******************/
                        int32_t string_size = *reinterpret_cast<const int32_t*>(&input[seek]); /*****************/
                        seek += sizeof(int32_t);                                               /*****************/
                        bson_t->__buffer[key] = std::string{                                   /*****************/
                                                    reinterpret_cast<const char*>(             /* handle string */
                                                        &input[seek]                           /*****************/
                                                    ),                                         /*****************/
                                                    string_size - 1                            /*****************/
                                                };                                             /*****************/
                        seek += string_size;                                                   /*****************/
                    }                                                                          /*****************/
                },
                {
                    0x03, /* 'element ::= "0x03" e_name document'                           --embedded document */
                    [](
                        const uint8_t* input, 
                        int32_t& seek, 
                        const std::string& key, 
                        document* bson_t
                    ) -> void 
                    {   /* document ::= int32 e_list "0x00"                                        **************/
                        int32_t document_size = *reinterpret_cast<const int32_t*>(&input[seek]);   /*************/
                        bson_t->__buffer[key] = std::any{document{&input[seek]}};                  /* handle    */
                        seek += document_size;                                                     /* document  */
                    }                                                                              /*************/
                },
                {
                    0x04, /* 'element ::= "0x04" e_name document'                                       --array */
                    [](
                        const uint8_t* input, 
                        int32_t& seek, 
                        const std::string& key, 
                        document* bson_t
                    ) -> void 
                    {   /* document ::= int32 e_list "0x00"                                     *****************/
                        int32_t array_size = *reinterpret_cast<const int32_t*>(&input[seek]);   /****************/
                        document array{&input[seek]};                                           /****************/
                        seek += array_size;                                                     /* handle array */
                        std::vector<std::any> buffer;                                           /****************/
                        buffer.reserve(array.size());                                           /****************/
                        buffer.resize(array.__buffer.size());                                   /****************/
                        for(auto& i : array.__buffer)                                           /****************/
                        { buffer[std::stoi(i.first)] = i.second; }                              /****************/
                        bson_t->__buffer[key] = buffer;                                         /****************/
                    }                                                                           /****************/
                },
                {
                    0x05, /* 'element ::= "0x05" e_name binary'                                   --binary data */
                    [](
                        const uint8_t* input, 
                        int32_t& seek, 
                        const std::string& key, 
                        document* bson_t
                    ) -> void 
                    {   /* binary ::= int32 subtype (byte*)                                      ****************/
                        int32_t binary_size = *reinterpret_cast<const int32_t*>(&input[seek]);   /***************/
                        seek += sizeof(int32_t);                                                 /***************/
                        uint8_t subtype = *reinterpret_cast<const uint8_t*>(&input[seek]);       /***************/  
                        seek += sizeof(uint8_t);                                                 /***************/
                        bson_t->__buffer[key] = std::make_pair(                                  /* handle      */
                                                    subtype,                                     /* binary      */
                                                    std::vector<uint8_t>{                        /***************/
                                                        &input[seek],                            /***************/
                                                        &input[seek] + binary_size               /***************/
                                                    }                                            /***************/
                                                );                                               /***************/
                        seek += binary_size;                                                     /***************/
                    }                                                                            /***************/
                },
                {
                    0x06, /* 'element ::= "0x06" e_name     '                                       --undefined */
                    [](
                        const uint8_t* input, 
                        int32_t& seek, 
                        const std::string& key, 
                        document* bson_t
                    ) -> void 
                    {                                                               /****************************/
                        bson_t->__buffer[key] = std::any{undefined()};              /* handle undefined         */
                    }                                                               /****************************/
                },
                {
                    0x07, /* 'element ::= "0x07" e_name (byte*12)'                                   --objectid */
                    [](
                        const uint8_t* input, 
                        int32_t& seek, 
                        const std::string& key, 
                        document* bson_t
                    ) -> void 
                    {                                                               /****************************/
                        int32_t objectid_size = 12;                                 /****************************/
                        bson_t->__buffer[key] = std::vector<uint8_t>{               /****************************/
                                            &input[seek],                           /* handle objectid          */
                                            &input[seek] + objectid_size            /****************************/
                                        };                                          /****************************/
                        seek += objectid_size;                                      /****************************/
                    }                                                               /****************************/
                },
                {
                    0x08, /* 'element ::= "0x08" e_name boolean'                                      --boolean */
                    [](
                        const uint8_t* input, 
                        int32_t& seek, 
                        const std::string& key, 
                        document* bson_t
                    ) -> void 
                    {   /* boolean ::= "0x00" | "0x01"                              *****************************/
                        bson_t->__buffer[key] = (bool)input[seek];                  /* handle boolean           */
                        seek += sizeof(bool);                                       /****************************/
                    }                                                               /****************************/
                },
                {
                    0x09, /* 'element ::= "0x09" e_name int64'                                   --utc datetime */
                    [](
                        const uint8_t* input, 
                        int32_t& seek, 
                        const std::string& key, 
                        document* bson_t
                    ) -> void 
                    {   /* int64 ::= (byte*8)                                             ***********************/
                        bson_t->__buffer[key] = std::any{                                 /**********************/
                                                    utc_datetime{                         /**********************/
                                                        *reinterpret_cast<                /**********************/
                                                            const int64_t*                /* handle utc         */
                                                        >(&input[seek])                   /* datetime           */
                                                    }                                     /**********************/
                                                };                                        /**********************/
                        seek += sizeof(int64_t);                                          /**********************/
                    }                                                                     /**********************/
                },
                {
                    0x0A, /* 'element ::= "0x0A" e_name    '                                             --null */
                    [](
                        const uint8_t* input, 
                        int32_t& seek, 
                        const std::string& key, 
                        document* bson_t
                    ) -> void 
                    {                                                               /****************************/
                        bson_t->__buffer[key] = nullptr;                            /* handle null              */
                    }                                                               /****************************/
                },
                {
                    0x0B, /* 'element ::= "0x0B" e_name cstring cstring'                                --regex */
                    [](
                        const uint8_t* input, 
                        int32_t& seek, 
                        const std::string& key, 
                        document* bson_t
                    ) -> void 
                    {   /* cstring ::= (byte*) "0x00"                        ************************************/
                        std::string pattern = (char*)&input[seek];           /***********************************/
                        seek = seek + pattern.size() + 1;                    /***********************************/
                        std::string options = (char*)&input[seek];           /***********************************/
                        seek = seek + options.size() + 1;                    /* handle regex                    */
                        bson_t->__buffer[key] = std::make_pair(              /***********************************/
                                                    pattern,                 /***********************************/
                                                    options                  /***********************************/
                                                );                           /***********************************/
                    }                                                        /***********************************/
                },
                {
                    0x0C, /* 'element ::= "0x0C" e_name string (byte*12)'                   --unicode dbpointer */
                    [](
                        const uint8_t* input, 
                        int32_t& seek, 
                        const std::string& key, 
                        document* bson_t
                    ) -> void 
                    {   /* string ::= int32 (byte*) "0x00"                          *****************************/
                        int32_t string_size = *reinterpret_cast<const int32_t*>(    /****************************/
                                                    &input[seek]                    /****************************/
                                                );                                  /****************************/
                        seek += sizeof(int32_t);                                    /****************************/
                        std::string pointer_name{                                   /****************************/
                            reinterpret_cast<const char*>(&input[seek]),            /****************************/
                            string_size - 1                                         /****************************/
                        };                                                          /* handle dbpointer         */
                        seek += string_size;                                        /****************************/
                        int32_t pointer_size = 12;                                  /****************************/
                        std::vector<uint8_t> pointer{                               /****************************/
                            &input[seek],                                           /****************************/
                            &input[seek] + pointer_size                             /****************************/
                        };                                                          /****************************/
                        seek += pointer_size;                                       /****************************/
                        bson_t->__buffer[key] = std::make_pair(                     /****************************/
                                                    pointer_name,                   /****************************/
                                                    pointer                         /****************************/
                                                );                                  /****************************/
                    }                                                               /****************************/
                },
                {
                    0x0D, /* 'element ::= "0x0D" e_name string'                               --javascript code */
                    [](
                        const uint8_t* input, 
                        int32_t& seek, 
                        const std::string& key, 
                        document* bson_t
                    ) -> void 
                    {   /* string ::= int32 (byte*) "0x00"                                  *********************/
                        int32_t javascript_code_size = *reinterpret_cast<const int32_t*>(   /********************/
                                                            &input[seek]                    /********************/
                                                        );                                  /********************/
                        seek += sizeof(int32_t);                                            /********************/
                        bson_t->__buffer[key] = std::any{                                   /********************/
                                                    javascript_code{                        /********************/
                                                        std::string{                        /* handle           */
                                                            reinterpret_cast<const char*>(  /* javascript code  */
                                                                &input[seek]                /********************/
                                                            ),                              /********************/
                                                            javascript_code_size - 1        /********************/
                                                        }                                   /********************/
                                                    }                                       /********************/
                                                };                                          /********************/
                        seek += javascript_code_size;                                       /********************/
                    }                                                                       /********************/
                },
                {
                    0x0E, /* 'element ::= "0x0E" e_name string'                                        --symbol */
                    [](
                        const uint8_t* input, 
                        int32_t& seek, 
                        const std::string& key, 
                        document* bson_t
                    ) -> void 
                    {   /* string ::= int32 (byte*) "0x00"                                  *********************/
                        int32_t symbol_size = *reinterpret_cast<const int32_t*>(            /********************/
                                                    &input[seek]                            /********************/
                                              );                                            /********************/
                        seek += sizeof(int32_t);                                            /********************/
                        bson_t->__buffer[key] = std::any{                                   /********************/
                                                    symbol{                                 /********************/
                                                        std::string{                        /* handle symbol    */
                                                            reinterpret_cast<const char*>(  /********************/
                                                                &input[seek]                /********************/
                                                            ),                              /********************/
                                                            symbol_size - 1                 /********************/
                                                        }                                   /********************/
                                                    }                                       /********************/
                                                };                                          /********************/
                        seek += symbol_size;                                                /********************/
                    }                                                                       /********************/
                },
                {
                    0x0F, /* 'element ::= "0x0F" e_name code_w_s'                  --javascript code with scope */
                    [](
                        const uint8_t* input, 
                        int32_t& seek, 
                        const std::string& key, 
                        document* bson_t
                    ) -> void 
                    {   /* code_w_s ::= int32 string document                         ***************************/
                        /* string   ::= int32 (byte*) "0x00"                          ***************************/
                        /* document ::= int32 e_list "0x00"                           ***************************/
                        seek += sizeof(int32_t);                                      /**************************/
                        int32_t string_size = *reinterpret_cast<const int32_t*>(      /**************************/
                                                    &input[seek]                      /**************************/
                                                );                                    /**************************/
                        seek += sizeof(int32_t);                                      /* handle javascript      */
                        std::string code_name{                                        /* code with scope        */
                            reinterpret_cast<const char*>(&input[seek]),              /**************************/
                            string_size - 1                                           /**************************/
                        };                                                            /**************************/
                        seek += string_size;                                          /**************************/
                        int32_t document_size = *reinterpret_cast<const int32_t*>(    /**************************/
                                                    &input[seek]                      /**************************/
                                                );                                    /**************************/
                        document code{&input[seek]};                                  /**************************/
                        seek += document_size;                                        /**************************/
                        bson_t->__buffer[key] = std::make_pair(code_name, code);      /**************************/
                    }                                                                 /**************************/
                },
                {
                    0x10, /* 'element ::= "0x10" e_name int32'                                 --32-bit integer */
                    [](
                        const uint8_t* input, 
                        int32_t& seek, 
                        const std::string& key, 
                        document* bson_t
                    ) -> void 
                    {   /* int32 ::= (byte*4)                                             ***********************/
                        bson_t->__buffer[key] = *reinterpret_cast<const int32_t*>(        /* handle 32-bit      */
                                                    &input[seek]                          /* integer            */
                                                );                                        /**********************/
                        seek += sizeof(int32_t);                                          /**********************/
                    }                                                                     /**********************/
                },
                {
                    0x11, /* 'element ::= "0x11" e_name uint64'                                     --timestamp */
                    [](
                        const uint8_t* input, 
                        int32_t& seek, 
                        const std::string& key, 
                        document* bson_t
                    ) -> void 
                    {   /* uint64 ::= (byte*8)                                             *********************/
                        bson_t->__buffer[key] = *reinterpret_cast<const uint64_t*>(        /********************/
                                                    &input[seek]                           /* handle timestamp */
                                                );                                         /********************/
                        seek += sizeof(uint64_t);                                          /********************/
                    }                                                                      /********************/
                },
                {
                    0x12, /* 'element ::= "0x12" e_name int64'                                --64-bit integer */
                    [](
                        const uint8_t* input, 
                        int32_t& seek, 
                        const std::string& key, 
                        document* bson_t
                    ) -> void 
                    {   /* int64 ::= (byte*8)                                             **********************/
                        bson_t->__buffer[key] = *reinterpret_cast<const int64_t*>(        /*********************/
                                                    &input[seek]                          /* handle 64-bit     */
                                                );                                        /* integer           */
                        seek += sizeof(int64_t);                                          /*********************/
                    }                                                                     /*********************/
                },
                {
                    0x13, /* 'element ::= "0x13" e_name decimal128'           --128-bit decimal floating point */
                    [](
                        const uint8_t* input, 
                        int32_t& seek, 
                        const std::string& key, 
                        document* bson_t
                    ) -> void 
                    {   /* decimal128 ::= (byte*16)                                    *************************/
                        bson_t->__buffer[key] = *reinterpret_cast<const long double*>( /************************/
                                                    &input[seek]                       /* handle 128-bit       */
                                                );                                     /* decimal floating     */
                        seek += sizeof(long double);                                   /* point                */
                    }                                                                  /************************/
                },
                {
                    0xFF, /* 'element ::= "0xFF" e_name       '                                      --min key */
                    [](
                        const uint8_t* input, 
                        int32_t& seek, 
                        const std::string& key, 
                        document* bson_t
                    ) -> void 
                    {                                                           /*******************************/
                        bson_t->__buffer[key] = std::any{min_key()};            /* handle min key              */
                    }                                                           /*******************************/
                },
                {
                    0x7F, /* 'element ::= "0x7F" e_name       '                                      --max key */
                    [](
                        const uint8_t* input, 
                        int32_t& seek, 
                        const std::string& key, 
                        document* bson_t
                    ) -> void 
                    {                                                           /*******************************/
                        bson_t->__buffer[key] = std::any{max_key()};            /* handle max key              */
                    }                                                           /*******************************/
                }
            };
                                                            /***************************************************/
            int32_t seek = input - &input[0];               /* get seek position                               */
                                                            /***************************************************/
            seek += sizeof(int32_t);                        /* start decoding elements after the document size */
                                                            /***************************************************/
            while (seek < this->__size - 1)                 /* exclude null terminal at the end of document    */
            {                                               /***************************************************/
                uint8_t type_indicator = input[seek];       /* get element type                                */
                seek += sizeof(uint8_t);                    /***************************************************/
                std::string key = (char*)&input[seek];      /***************************************************/
                seek = seek + key.size() + 1;               /***************************************************/
                auto& match = interpreter[type_indicator];  /* extracting element based on type                */
                if(match) match(input, seek, key, this);    /***************************************************/
            }                                               /***************************************************/
        };
/********************************************End Implementation*************************************************/
    }
}

#endif
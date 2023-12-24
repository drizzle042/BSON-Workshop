#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <functional>
#include <any>


class objectid {
    public:
        objectid(const uint8_t*);

    private:
        std::vector<uint8_t> __buffer;
};

class bson
{
public:
    explicit bson(const uint8_t* input)
    : __size{*reinterpret_cast<const uint32_t*>(input)} { this->__decode(input); };

    explicit bson(const std::unordered_map<std::string, std::any>& input)
    : __buffer{input} {};

    std::any& operator[](const std::string& key)
    { return this->__buffer[key]; };

    std::vector<uint8_t> data()
    { return this->__encode(); };

    size_t size()
    { return __size; };

private:
    std::vector<uint8_t> __encode(){
        std::vector<uint8_t> buffer;

        return buffer;
    };
    void __decode(const uint8_t* input)
    {
        static std::unordered_map<
            uint8_t, 
            std::function<
                void(
                    const uint8_t*, 
                    int32_t&, 
                    const std::string&, 
                    bson*
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
                    bson* bson_t
                ) -> void 
                {   /* double ::= (byte*8)                                         ************************/
                    (*bson_t)[key] = *reinterpret_cast<const double*>(&input[seek]); /* handle double       */
                    seek += 8;                                                       /***********************/
                }                                                                    /***********************/
            },
            {
                0x02, /* 'element ::= "0x02" e_name string'                                  --utf-8 string */
                [](
                    const uint8_t* input, 
                    int32_t& seek, 
                    const std::string& key, 
                    bson* bson_t
                ) -> void 
                {   /* string ::= int32 (byte*) "0x00"                                     ******************/
                    int32_t string_size = *reinterpret_cast<const int32_t*>(&input[seek]); /*****************/
                    seek += 4;                                                             /*****************/
                    (*bson_t)[key] = std::string{                                          /*****************/
                                        reinterpret_cast<const char*>(&input[seek]),       /* handle string */
                                        string_size - 1                                    /*****************/
                                    };                                                     /*****************/
                    seek += string_size;                                                   /*****************/
                }                                                                          /*****************/
            },
            {
                0x03, /* 'element ::= "0x03" e_name document'                           --embedded document */
                [](
                    const uint8_t* input, 
                    int32_t& seek, 
                    const std::string& key, 
                    bson* bson_t
                ) -> void 
                {   /* document ::= int32 e_list "0x00"                                        **************/
                    uint32_t document_size = *reinterpret_cast<const uint32_t*>(&input[seek]); /* handle    */
                    (*bson_t)[key] = bson{&input[seek]};                                       /* document  */
                    seek += document_size;                                                     /*************/
                }                                                                              /*************/
            },
            {
                0x04, /* 'element ::= "0x04" e_name document'                                       --array */
                [](
                    const uint8_t* input, 
                    int32_t& seek, 
                    const std::string& key, 
                    bson* bson_t
                ) -> void 
                {   /* document ::= int32 e_list "0x00"                                     *****************/
                    uint32_t array_size = *reinterpret_cast<const uint32_t*>(&input[seek]); /****************/
                    bson array{&input[seek]};                                               /****************/
                    seek += array_size;                                                     /* handle array */
                    std::vector<std::any> buffer;                                           /****************/
                    for(auto& i : array.__buffer){ buffer[std::stoi(i.first)] = i.second; } /****************/
                    (*bson_t)[key] = buffer;                                                /****************/
                }                                                                           /****************/
            },
            {
                0x05, /* 'element ::= "0x05" e_name binary'                                   --binary data */
                [](
                    const uint8_t* input, 
                    int32_t& seek, 
                    const std::string& key, 
                    bson* bson_t
                ) -> void 
                {   /* binary ::= int32 subtype (byte*)                                      ****************/
                    uint32_t binary_size = *reinterpret_cast<const uint32_t*>(&input[seek]); /***************/
                    seek += 4;                                                               /***************/
                    uint8_t subtype = input[seek];                                           /***************/  
                    seek++;                                                                  /***************/
                    (*bson_t)[key] = std::make_pair(                                         /* handle      */
                                        subtype,                                             /* binary      */
                                        std::vector<uint8_t>{                                /***************/
                                            &input[seek],                                    /***************/
                                            &input[seek] + binary_size                       /***************/
                                        }                                                    /***************/
                                    );                                                       /***************/
                    seek += binary_size;                                                     /***************/
                }                                                                            /***************/
            },
            {
                0x06, /* 'element ::= "0x06" e_name     '                                       --undefined */
                [](
                    const uint8_t* input, 
                    int32_t& seek, 
                    const std::string& key, 
                    bson* bson_t
                ) -> void 
                {                                                               /****************************/
                    (*bson_t)[key] = nullptr;                                   /* handle undefined         */
                }                                                               /****************************/
            },
            {
                0x07, /* 'element ::= "0x07" e_name (byte*12)'                                   --objectid */
                [](
                    const uint8_t* input, 
                    int32_t& seek, 
                    const std::string& key, 
                    bson* bson_t
                ) -> void 
                {                                                               /****************************/
                    uint32_t objectid_size = 12;                                /****************************/
                    (*bson_t)[key] = std::vector<uint8_t>{                      /****************************/
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
                    bson* bson_t
                ) -> void 
                {   /* boolean ::= "0x00" | "0x01"                              *****************************/
                    (*bson_t)[key] = (bool)input[seek];                         /* handle boolean           */
                    seek++;                                                     /****************************/
                }                                                               /****************************/
            },
            {
                0x09, /* utc datetime */
                [](
                    const uint8_t* input, 
                    int32_t& seek, 
                    const std::string& key, 
                    bson* bson_t
                ) -> void 
                {
                    (*bson_t)[key] = *reinterpret_cast<const int64_t*>(&input[seek]);
                    seek += 8;
                }
            },
            {
                0x0A, /* null */
                [](
                    const uint8_t* input, 
                    int32_t& seek, 
                    const std::string& key, 
                    bson* bson_t
                ) -> void 
                {
                    (*bson_t)[key] = nullptr;
                }
            },
            {
                0x0B, /* regex */
                [](
                    const uint8_t* input, 
                    int32_t& seek, 
                    const std::string& key, 
                    bson* bson_t
                ) -> void 
                {
                    std::string pattern;
                    while (input[seek] != 0x00 /* extracting regex pattern */) {
                        pattern.push_back(input[seek]);
                        seek++;
                    }
                    seek++; /* move past the null terminator */
                    std::string options;
                    while (input[seek] != 0x00 /* extracting regex options */) {
                        options.push_back(input[seek]);
                        seek++;
                    }
                    seek++; /* move past the null terminator */
                    (*bson_t)[key] = std::make_pair(pattern, options);
                }
            },
            {
                0x0C, /* unicode dbpointer */
                [](
                    const uint8_t* input, 
                    int32_t& seek, 
                    const std::string& key, 
                    bson* bson_t
                ) -> void 
                {
                    uint32_t string_size = *reinterpret_cast<const uint32_t*>(
                                                &input[seek]
                                            );
                    seek += 4;
                    std::string pointer_name{
                        reinterpret_cast<const char*>(&input[seek]), 
                        string_size - 1
                    };
                    seek += string_size;
                    std::size_t pointer_size = 12;
                    std::vector<uint8_t> pointer{
                        &input[seek], 
                        &input[seek] + pointer_size
                    };
                    seek += pointer_size;
                    (*bson_t)[key] = std::make_pair(pointer_name, pointer);
                }
            },
            {
                0x0D, /* javascript code */
                [](
                    const uint8_t* input, 
                    int32_t& seek, 
                    const std::string& key, 
                    bson* bson_t
                ) -> void 
                {
                    uint32_t javascript_code_size = *reinterpret_cast<const uint32_t*>(
                                                        &input[seek]
                                                    );
                    seek += 4;
                    (*bson_t)[key] = std::string{
                                        reinterpret_cast<const char*>(&input[seek]), 
                                        javascript_code_size - 1
                                    };
                    seek += javascript_code_size;
                }
            },
            {
                0x0E, /* symbol */
                [](
                    const uint8_t* input, 
                    int32_t& seek, 
                    const std::string& key, 
                    bson* bson_t
                ) -> void 
                {
                    uint32_t symbol_size = *reinterpret_cast<const uint32_t*>(
                                                &input[seek]
                                            );
                    seek += 4;
                    (*bson_t)[key] = std::string{
                                        reinterpret_cast<const char*>(&input[seek]), 
                                        symbol_size - 1
                                     };
                    seek += symbol_size;
                }
            },
            {
                0x0F, /* javascript code with scope */
                [](
                    const uint8_t* input, 
                    int32_t& seek, 
                    const std::string& key, 
                    bson* bson_t
                ) -> void 
                {
                    uint32_t code_size = *reinterpret_cast<const uint32_t*>(
                                            &input[seek]
                                         );
                    seek += 4;
                    uint32_t string_size = *reinterpret_cast<const uint32_t*>(
                                                &input[seek]
                                            );
                    seek += 4;
                    std::string code_name{
                        reinterpret_cast<const char*>(&input[seek]), 
                        string_size - 1
                    };
                    seek += string_size;
                    uint32_t document_size = *reinterpret_cast<const uint32_t*>(
                                                input[seek]
                                            );
                    bson code{&input[seek]};
                    seek += document_size;
                    (*bson_t)[key] = std::make_pair(code_name, code);
                }
            },
            {
                0x10, /* 32-bit integer */
                [](const uint8_t* input, int32_t& seek, const std::string& key, bson* bson_t) -> void {
                    (*bson_t)[key] = *reinterpret_cast<const int32_t*>(&input[seek]);
                    seek += 4;
                }
            },
            {
                0x11, /* timestamp */
                [](const uint8_t* input, int32_t& seek, const std::string& key, bson* bson_t) -> void {
                    (*bson_t)[key] = *reinterpret_cast<const uint64_t*>(&input[seek]);
                    seek += 8;
                }
            },
            {
                0x12, /* 64-bit integer */
                [](
                    const uint8_t* input, 
                    int32_t& seek, 
                    const std::string& key, 
                    bson* bson_t
                ) -> void 
                {
                    (*bson_t)[key] = *reinterpret_cast<const int64_t*>(
                                        &input[seek]
                                    );
                    seek += 8;
                }
            },
            {
                0x13, /* 128-bit decimal floating point */
                [](
                    const uint8_t* input, 
                    int32_t& seek, 
                    const std::string& key, 
                    bson* bson_t
                ) -> void 
                {
                    (*bson_t)[key] = *reinterpret_cast<const long double*>(
                                        &input[seek]
                                    );
                    seek += 16;
                }
            },
            {
                0xFF, /* min key */
                [](
                    const uint8_t* input, 
                    int32_t& seek, 
                    const std::string& key, 
                    bson* bson_t
                ) -> void 
                {
                    (*bson_t)[key] = nullptr;
                }
            },
            {
                0x7F, /* max key */
                [](
                    const uint8_t* input, 
                    int32_t& seek, 
                    const std::string& key, 
                    bson* bson_t
                ) -> void 
                {
                    (*bson_t)[key] = nullptr;
                }
            }
        };
                                                        /***************************************************/
        int32_t seek = input - &input[0];               /* get seek position                               */
                                                        /***************************************************/
        seek += 4;                                      /* start decoding elements after the document size */
                                                        /***************************************************/
        while (seek < this->__size - 1)                 /* exclude null terminal at the end of document    */
        {                                               /***************************************************/
            uint8_t type_indicator = input[seek];       /* get element type                                */
            seek++;                                     /***************************************************/
            std::string key;                            /***************************************************/
            while (input[seek] != 0x00)                 /***************************************************/
            {                                           /***************************************************/
                key.push_back(input[seek]);             /* extracting element key                          */
                seek++;                                 /***************************************************/
            }                                           /***************************************************/
            seek++;                                     /***************************************************/
            auto& match = interpreter[type_indicator];  /* extracting element based on type                */
            if(match) match(input, seek, key, this);    /***************************************************/
        }                                               /***************************************************/

    };
    std::unordered_map<std::string, std::any> __buffer;
    uint32_t __size;
};

int main()
{
    std::vector<uint8_t> bsonData = {
        0x1b, 0x00, 0x00, 0x00, 
        0x08, 
        0x63, 0x6f, 0x6d, 0x70, 0x61, 0x63, 0x74, 0x00, 
        0x01, 
        0x10, 
        0x73, 0x63, 0x68, 0x65, 0x6d, 0x61, 0x00, 
        0x00, 0x00, 0x00, 0x00, 
        0x00
    };

    bson bson_t{bsonData.data()};

    std::cout << "DOCUMENT SIZE: " << bson_t.size() << "\n";
    std::cout << "compact: " << std::boolalpha << std::any_cast<bool>(bson_t["compact"]) << "\n";
    std::cout << "schema: " << std::any_cast<int>(bson_t["schema"]) << "\n";

    return 0;
}
#ifndef __BSON_H__
#define __BSON_H__

#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <functional>
#include <any>


namespace canon
{
    namespace bson
    {
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
    
/*******************************************Start Implementation************************************************/
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
            std::vector<uint8_t> buffer;
            return buffer;
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
                        bson_t->__buffer[key] = *reinterpret_cast<const double*>(        /* handle double       */
                                                    &input[seek]                         /***********************/
                                                );                                       /***********************/
                        seek += 8;                                                       /***********************/
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
                        seek += 4;                                                             /*****************/
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
                        seek += 4;                                                               /***************/
                        uint8_t subtype = *reinterpret_cast<const uint8_t*>(&input[seek]);       /***************/  
                        seek++;                                                                  /***************/
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
                        bson_t->__buffer[key] = nullptr;                            /* handle undefined         */
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
                        seek++;                                                     /****************************/
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
                        bson_t->__buffer[key] = *reinterpret_cast<const int64_t*>(        /* handle utc         */
                                                    &input[seek]                          /* datetime           */
                                                );                                        /**********************/
                        seek += 8;                                                        /**********************/
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
                        seek += pattern.size();                              /***********************************/
                        seek++;                                              /***********************************/
                        std::string options = (char*)&input[seek];           /***********************************/
                        seek += options.size();                              /* handle regex                    */
                        seek++;                                              /***********************************/
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
                        seek += 4;                                                  /****************************/
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
                        seek += 4;                                                          /* handle           */
                        bson_t->__buffer[key] = std::string{                                /* javascript code  */
                                                    reinterpret_cast<const char*>(          /********************/
                                                        &input[seek]                        /********************/
                                                    ),                                      /********************/
                                                    javascript_code_size - 1                /********************/
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
                        seek += 4;                                                          /********************/
                        bson_t->__buffer[key] = std::string{                                /* handle symbol    */
                                                    reinterpret_cast<const char*>(          /********************/
                                                        &input[seek]                        /********************/
                                                    ),                                      /********************/
                                                    symbol_size - 1                         /********************/
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
                        seek += 4;                                                    /**************************/
                        int32_t string_size = *reinterpret_cast<const int32_t*>(      /**************************/
                                                    &input[seek]                      /**************************/
                                                );                                    /**************************/
                        seek += 4;                                                    /* handle javascript      */
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
                        seek += 4;                                                        /**********************/
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
                        seek += 8;                                                         /********************/
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
                        seek += 8;                                                        /*********************/
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
                                            &input[seek]                               /* handle 128-bit       */
                                        );                                             /* decimal floating     */
                        seek += 16;                                                    /* point                */
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
                        bson_t->__buffer[key] = nullptr;                        /* handle MinKey               */
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
                        bson_t->__buffer[key] = nullptr;                        /* handle MaxKey               */
                    }                                                           /*******************************/
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
                std::string key = (char*)&input[seek];      /***************************************************/
                seek += key.size();                         /***************************************************/
                seek++;                                     /***************************************************/
                auto& match = interpreter[type_indicator];  /* extracting element based on type                */
                if(match) match(input, seek, key, this);    /***************************************************/
            }                                               /***************************************************/
        };
/********************************************End Implementation*************************************************/
    }
}

#endif
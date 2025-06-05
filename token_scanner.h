#ifndef __scanner_H__
#define __scanner_H__

#include "def.h"
#include <fstream>
#include <string> 
#include <variant>
#include <vector>



typedef struct{
    Tktype type;
    std::string lexme;
    std::variant<std::monostate, int, float> val;
    int line, column;
    
    
} Token;

class token_scanner
{
private:
    int line;
    int column;
    std::ifstream inputFile;
    char currentChar;
    Token currentToken;
    char NextChar(); 
    char Peek();
    bool eof() const;   // 检查是否到达文件末尾

    /*
    *   @return -1-_ 0-digit 1-letter 2-edge 3-
    */
    int charType();

public:
    token_scanner(const std::string& filename):inputFile(filename){
        if(!inputFile.is_open()){
            // [TODO] error processing
        }
        else if (inputFile.eof()){
            // [TODO] error processing
        }
        
    }
    ~token_scanner(){
        inputFile.close();
        // [TODO] free memory
    }
    /*
    *   @brief purchase current token
    *   @return a token
    */
    Token nextToken();
    /*
    *  @brief return next token,do not ourchase current
    *  @return a token
    */
    Token tryToken();
    /*
    * @brief test next token
    * @return true or false
    */
    bool exceptToken(Tktype);




    
    


    // 用于重新定位的方法
    void seekToBeginning();
    void seekToPosition(std::streampos pos);
    std::streampos getCurrentPosition();
    void movePosition(std::streamoff offset, std::ios_base::seekdir way = std::ios_base::cur);
};

#endif
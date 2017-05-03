//
// Created by zglozman on 4/30/17.
//

#ifndef PROJECT_CRYPTO_H_H
#define PROJECT_CRYPTO_H_H

#include <stdio.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <string>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>
#include <stdlib.h>
#include <memory>
#include <boost/filesystem.hpp>

using namespace std;
namespace fs = boost::filesystem;

class Credential{
private:
    RSA             *r = NULL;
    BIGNUM          *bne = NULL;
    BIO             *bp_public = NULL, *bp_private = NULL;

public:
    Credential(std::string fqdn, std::string prefix);
    ~Credential();
    bool RSASign( const unsigned char* Msg, size_t MsgLen, shared_ptr<string> &EncMsg);
    string readFile2(const string &filename);
    void testPublicKeyBytes();

};

bool generate_key();
std::string decode64(const std::string &val);
#endif //PROJECT_CRYPTO_H_H

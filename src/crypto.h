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
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include <openssl/evp.h>

namespace pt = boost::property_tree;

using namespace std;
namespace fs = boost::filesystem;

class Credential{
private:
    RSA             *r_primary = NULL, *r_secondary = NULL;
    BIGNUM          *bne_primary = NULL, *bne_secondary = NULL;
    EVP_PKEY       *evp_primary = NULL, *evp_secondary = NULL;

    string fullPathPublicPrimary;
    string fullPathPrivatePrimary;
    string fqdn;
    string fullPathPublicBackup;
    string fullPathPrivateBackup;

public:
    Credential(std::string fqdn, std::string prefix);
    ~Credential();
    bool RSASign( const unsigned char* Msg, size_t MsgLen, shared_ptr<string> &EncMsg, RSA *rsa);
    string readFile2(const string &filename);
    shared_ptr<pt::ptree> getRequestJSON();
    void saveKeysToFile(RSA *r, EVP_PKEY *pkey,  string fileNamePk, string fileNamePub);
    string getPublicKeySignatureInPkcs8(RSA *key);
};

bool generate_key();
std::string decode64(const std::string &val);
#endif //PROJECT_CRYPTO_H_H

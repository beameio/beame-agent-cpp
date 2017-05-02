#include <stdio.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <string>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>
#include "crypto.h"
#include <iostream>
#include <boost/filesystem.hpp> 

using namespace std;
namespace fs = boost::filesystem;
std::string decode64(const std::string &val) {
    using namespace boost::archive::iterators;
    using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
    return boost::algorithm::trim_right_copy_if(std::string(It(std::begin(val)), It(std::end(val))), [](char c) {
        return c == '\0';
    });
}
Credential::~Credential() {
    if(bp_public)
        BIO_free_all(bp_public);
    if(bp_private)
        BIO_free_all(bp_private);
    if(r)
        RSA_free(r);
    if(bne)
        BN_free(bne);
}

Credential::Credential(std::string fqdn, std::string pathPrefix) {
    cout << "Generating rsa keys \r\n";
    cout << "Callled with " << fqdn << " Prefix " << pathPrefix << "\r\n";

    fs::create_directories(pathPrefix + "/" + fqdn);
    std::string fullPathPublic = pathPrefix + "/" + fqdn +"/public_key.pem";
    std::string fullPathPrivate = pathPrefix + "/" + fqdn + "/private_key.pem";

    int             bits = 2048;
    unsigned long   e = RSA_F4;
    // 1. generate rsa key
    bne = BN_new();
    int ret = BN_set_word(bne,e);
    if(ret != 1) {
        cout << "bignumber allocation failed probably means there is a problem with openssl \r\n";
        return ;
    }
    r = RSA_new();
    ret = RSA_generate_key_ex(r, bits, bne, NULL);
    if(ret != 1){
        cout << "rsa key generation failed \r\n";
    }
    bp_public = BIO_new_file(fullPathPublic.c_str(), "w+");
    ret = PEM_write_bio_RSAPublicKey(bp_public, r);
    if(ret != 1){
        cout << "writing of public key failed " << fqdn << " " << fullPathPublic << "\r\n";
        return;
    }

    // 3. save private key

    bp_private = BIO_new_file(fullPathPrivate.c_str(), "w+");
    ret = PEM_write_bio_RSAPrivateKey(bp_private, r, NULL, NULL, 0, NULL, NULL);

}

bool generate_key(std::string pathPrefix)
{


}

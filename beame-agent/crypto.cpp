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
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"


std::string decode64(const std::string &val) {

    using namespace boost::archive::iterators;
    using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
    return boost::algorithm::trim_right_copy_if(std::string(It(std::begin(val)), It(std::end(val))), [](char c) {
            return c == '\0';
            });
}
std::string encode64(const std::string &val) {
    using namespace boost::archive::iterators;
    using It = base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
    auto tmp = std::string(It(std::begin(val)), It(std::end(val)));
    return tmp.append((3 - val.size() % 3) % 3, '=');
}

string Credential::readFile2(const string &fileName)
{
    ifstream ifs(fileName.c_str(), ios::in | ios::binary | ios::ate);

    ifstream::pos_type fileSize = ifs.tellg();
    ifs.seekg(0, ios::beg);

    vector<char> bytes(fileSize);
    ifs.read(&bytes[0], fileSize);

    return string(&bytes[0], fileSize);
}

Credential::~Credential() {
    if(evp_primary) {
        EVP_PKEY_free(evp_primary);
    }
    if(evp_secondary) {
        EVP_PKEY_free(evp_secondary);
    }
  /*  if(r_primary)  RSA_free(r_primary);
    if(r_secondary)  RSA_free(r_secondary);
    if(bne_primary)  BN_free(bne_primary);
    if(bne_secondary)  BN_free(bne_secondary);*/
}

void generatePrivateKeys(BIGNUM **bne, RSA **rsa, EVP_PKEY **evp){
    *bne = BN_new();
    int             bits = 2048;
    unsigned long   e = RSA_F4;
    int ret = BN_set_word(*bne,e);
    if(ret != 1) {
        cout << "bignumber allocation failed probably means there is a problem with openssl \r\n";
        return ;
    }
    *rsa = RSA_new();
    ret = RSA_generate_key_ex(*rsa, bits, *bne, NULL);
    if(ret != 1){
        cout << "rsa key generation failed \r\n";
    }
    *evp = EVP_PKEY_new();
    if(*rsa && *evp && EVP_PKEY_assign_RSA(*evp,*rsa)) {
        /* pKey owns pRSA from now */
        if (RSA_check_key(*rsa) <= 0) {
            cout << "EVP Assigment Failed \r\n";
        }
    }
}

void Credential::saveKeysToFile(RSA *r, EVP_PKEY *evp_key, string fileNamePk, string fileNamePub){
    BIO *bp_public = NULL, *bp_private = NULL;
    
    bp_public = BIO_new_file(fileNamePub.c_str(), "w+");
    //int ret = PEM_write_bio_RSAPublicKey(bp_public, r);
    int ret = PEM_write_bio_PUBKEY(bp_public, evp_key);
    if(ret != 1){
        cout << "writing of public key failed " <<  fileNamePub  << "\r\n";
        return;
    }

    // 3. save private key

    bp_private = BIO_new_file(fileNamePk.c_str(), "w+");
    ret = PEM_write_bio_RSAPrivateKey(bp_private, r, NULL, NULL, 0, NULL, NULL); //pkcs1 while  PEM_write_bio_PrivateKey does pkcs7/8?

    if(ret != 1){
        cout << "writing of private key failed " <<  fileNamePk  << "\r\n";
        return;
    }
    if(bp_public)
        BIO_free_all(bp_public);
    if(bp_private)
        BIO_free_all(bp_private);
}

Credential::Credential(std::string fqdn, std::string pathPrefix) {
    cout << "Generating rsa keys \r\n";
    cout << "Callled with " << fqdn << " Prefix " << pathPrefix << "\r\n";
    this->fqdn = fqdn;
    fs::create_directories(pathPrefix + "/" + fqdn);
    fullPathPublicPrimary = pathPrefix + "/" + fqdn +"/public_key.pem";
    fullPathPrivatePrimary = pathPrefix + "/" + fqdn + "/private_key.pem";
    
    fullPathPublicBackup = pathPrefix + "/" + fqdn +"/public_key_bk.pem";
    fullPathPrivateBackup = pathPrefix + "/" + fqdn + "/private_key_bk.pem";
    // 1. generate rsa key (2 of them )
    
    generatePrivateKeys( &this->bne_primary, &this->r_primary, &evp_primary);
    generatePrivateKeys( &this->bne_secondary, &this->r_secondary, &evp_secondary);

    cout << "Saving Primary Key " << fullPathPrivatePrimary << " " << fullPathPublicPrimary << "\r\n";
    saveKeysToFile(r_primary, evp_primary, fullPathPrivatePrimary, fullPathPublicPrimary);
    cout << "Saving Secondary Key " << fullPathPrivatePrimary << " " << fullPathPublicPrimary << "\r\n";
    saveKeysToFile(r_secondary, evp_secondary, fullPathPrivateBackup, fullPathPublicBackup);
}

bool Credential::RSASign( const unsigned char* Msg,
        size_t MsgLen, shared_ptr<string> &EncMsg, RSA *rsa) {

    assert(rsa);

    EVP_MD_CTX* m_RSASignCtx = EVP_MD_CTX_create();
    EVP_PKEY* priKey  = EVP_PKEY_new();
    EVP_PKEY_assign_RSA(priKey, rsa);
    if (EVP_DigestSignInit(m_RSASignCtx,NULL, EVP_sha256(), NULL,priKey)<=0) {
        return false;
    }
    if (EVP_DigestSignUpdate(m_RSASignCtx, Msg, MsgLen) <= 0) {
        return false;
    }
    size_t MsgLenEnc;
    if (EVP_DigestSignFinal(m_RSASignCtx, NULL, &MsgLenEnc) <=0) {
        return false;
    }
    shared_ptr<string> output = shared_ptr<string>(new string());
    output->resize(MsgLenEnc);
    unsigned char *bufferStart = (unsigned  char *) &output->operator[](0);
    if (EVP_DigestSignFinal(m_RSASignCtx, bufferStart, &MsgLenEnc) <= 0) {
        return false;
    }
    EncMsg = output;
    EVP_MD_CTX_cleanup(m_RSASignCtx);
    return true;
}

string Credential::getPublicKeySignatureInPkcs8(RSA *key){

    size_t bigNumberLength = BN_num_bytes(key->n);

    shared_ptr<string> output = shared_ptr<string>(new string(decode64("MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA")));
    
    output->resize(output->length() + 1);
    output->operator[](output->length() -1) = 0;
    unsigned char footer[]= {0x02, 0x03, 0x01, 0x00, 0x01};
    int startBuferEndPosition = output->length();
    output->resize(bigNumberLength +output->length() + sizeof(footer));
    unsigned char *bufferStart = (unsigned  char *) &output->operator[](startBuferEndPosition);
    int returnLength = BN_bn2bin(key->n, bufferStart);
    memcpy((bufferStart  + returnLength),   &footer[0], sizeof(footer));
    shared_ptr<string> signature;
    cout << "\r\n" << encode64(*output) << "\r\n";
    bufferStart = (unsigned  char *) &output->operator[](0);
    this->RSASign(bufferStart, output->length(), signature, key);
    string base64signatue =  encode64(*signature);
    return base64signatue;
}

shared_ptr<pt::ptree> Credential::getRequestJSON(){
    string pkSingnature = getPublicKeySignatureInPkcs8(r_primary);
    cout << "Public Key signature \r\n\t " << pkSingnature << "\r\n";
    string primaryPublicKey = readFile2(fullPathPublicPrimary);
    string backupPublicKey = readFile2(fullPathPublicBackup);
    cout << primaryPublicKey;
    cout << backupPublicKey;
    //int returnLength = BN_bn2bin(r->n, returnBuffer);
    //cout << "Big Number Length " << bigNumberLength << "  \r\n";
    //cout << " Actualy PublicKey Exponent " << BN_bn2hex(r->n) << "\r\n";
   // 3082012230d06092a864886f70d0101010500038201f00308201a02820101
                                                                        
    
    //cout << "WRITING to public key to buffer from position " << output->length() << "sizeof footer " << sizeof(footer) <<" \r\n";


    //cout << "BN TO BIN return " << returnLength;

    //cout << " TOTAL LENGTH : " << output->length() << "\r\n";


    shared_ptr<pt::ptree>  root(new pt::ptree());
    root->put("validity", 31536000);
    root->put("pub.pub", primaryPublicKey);
    root->put("pub.pub_bk", backupPublicKey);
    root->put("pub.signature",pkSingnature);
    root->put("fqdn", fqdn);
    root->put("format", 1);
//    pt::write_json(std::cout, root);
//   
    return root;
}



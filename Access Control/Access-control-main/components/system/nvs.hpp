

#ifndef __NVS_HPP__
#define __NVS_HPP__


#include <stdint.h>
#include <stdbool.h>

#include <string>
#include <variant>
#include <vector>

#include <nvs.h>

#include "../structures.hpp"

using namespace std;


struct NvsEntry {
    int         key;
    char        nvsKey[8];
    NvsType     typ;
    MultiType   data;
    bool        loaded;
    bool        modified;
};



class Nvs {

public:
    Nvs(std::vector<NvsEntry> *vect, const char * tag);
    virtual ~Nvs() {};

public:



public:
    virtual bool setup(const char * sPartitionName, const char * sNamespace);
    virtual void loop(void);

    virtual void loadData();
    virtual void saveData();

    virtual MultiType get(int k, NvsType t);
    virtual const char* getString(int k);
    virtual int   getInt(int k);
    virtual uint32_t getU32(int k);
    virtual uint64_t getU64(int k);
    virtual double getDouble(int k);
    virtual void  set(int k, NvsType t, MultiType val);
    virtual void  setString(int k, char* cstr);
    virtual void  setString(int k, std::string &s);

protected:
    bool _bInit;
    // Nvs *_instance;
    const char *_sPartitionName;
    const char *_sNamespace;
    const char *_tag;

    nvs_handle_t            _hNvsPartition;
    std::vector<NvsEntry>   *_dataVector; //  = { {} }
};

#endif // __NVS_HPP__

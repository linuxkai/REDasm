#ifndef DISASSEMBLERBASE_H
#define DISASSEMBLERBASE_H

#include <functional>
#include "../plugins/format.h"
#include "types/referencetable.h"
#include "disassemblerfunctions.h"

namespace REDasm {

class DisassemblerBase: public DisassemblerFunctions
{
    public:
        DisassemblerBase(Buffer buffer, FormatPlugin* format);
        virtual ~DisassemblerBase();
        Buffer& buffer();
        bool hasReferences(const SymbolPtr &symbol);
        ReferenceVector getReferences(const SymbolPtr &symbol);
        u64 getReferencesCount(const SymbolPtr &symbol);

    public: // Primitive functions
        virtual FormatPlugin* format();
        virtual SymbolTable* symbolTable();
        virtual void pushReference(const SymbolPtr& symbol, address_t address);
        virtual u64 locationIsString(address_t address, bool *wide = NULL) const;
        virtual std::string readString(const SymbolPtr& symbol) const;
        virtual std::string readWString(const SymbolPtr& symbol) const;
        virtual std::string readHex(address_t address, u32 count) const;
        virtual SymbolPtr dereferenceSymbol(const SymbolPtr &symbol, u64 *value = NULL);
        virtual bool dereferencePointer(address_t address, u64& value) const;
        virtual bool getBuffer(address_t address, Buffer& data) const;
        virtual bool readAddress(address_t address, size_t size, u64 &value) const;
        virtual bool readOffset(offset_t offset, size_t size, u64 &value) const;
        virtual std::string readString(address_t address) const;
        virtual std::string readWString(address_t address) const;

   private:
        template<typename T> std::string readStringT(address_t address, std::function<bool(T, std::string&)> fill) const;
        template<typename T> u64 locationIsStringT(address_t address, std::function<bool(T)> isp, std::function<bool(T)> isa) const;

   protected:
        ReferenceTable _referencetable;
        SymbolTable* _symboltable;
        FormatPlugin* _format;
        Buffer _buffer;
};

template<typename T> std::string DisassemblerBase::readStringT(address_t address, std::function<bool(T, std::string&)> fill) const
{
    Buffer b = this->_buffer + this->_format->offset(address);
    u64 count = 0;
    std::string s;

    while(fill(*reinterpret_cast<T*>(b.data), s) && !b.eob())
    {
        count++;
        b += sizeof(T);
    }

    return "\"" + s + "\"";
}

template<typename T> u64 DisassemblerBase::locationIsStringT(address_t address, std::function<bool(T)> isp, std::function<bool(T)> isa) const
{
    if(!this->_format->segment(address))
        return 0;

    bool containsalpha = false;
    u64 count = 0;
    Buffer b = this->_buffer;
    b += this->_format->offset(address);

    while(isp(*reinterpret_cast<T*>(b.data)) && !b.eob())
    {
        count++;

        if(!containsalpha)
            containsalpha = isa(*reinterpret_cast<T*>(b.data));

        if(count >= MIN_STRING)
            break;

        b += sizeof(T);
    }

    if(!containsalpha) // ...it might be just data...
        return 0;

    return count;
}

} // namespace REDasm

#endif // DISASSEMBLERBASE_H

#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>

#define fn_UNIT_SIZE 16 //unit size, in bytes (8-bit units)
#define fn_TYPEDDNA_ID 1

#define fn_BYTE 0
#define fn_SHORT 1
#define fn_INT 2
#define fn_LONG 3

//This file provides the classes for manipulating data on DNA, according to my
//typed data format. Meant to be used in 8-bit byte machines.

/**
 * Base character class for holding DNA data. Contains methods for manipulating
 * single bytes of data (8-bits, char).
 * 
 */
class CharDna
{
private:
    char* m_data;
    uint_fast32_t m_len;
    uint_fast64_t m_seed;
    uint_fast32_t m_ptr;
protected:

    
    void realloc(uint_fast32_t newLen)
    {
        if(newLen < m_ptr)
        {
            //ERROR - just do nothing
            return;
        }
        char* newBuf = new char[newLen];
        for(unsigned int i = 0; i < m_ptr; i++)
        {
            *(newBuf + i) = *(m_data + i);
        }
        memset(newBuf + m_ptr, 0, newLen - m_ptr);
        m_len = newLen;
        delete[] m_data;
        m_data = newBuf;
    }

public:
    CharDna(uint_fast64_t seed, uint_fast32_t init_len) :
        m_data(new char[init_len]),
        m_len(init_len),
        m_seed(seed),
        m_ptr(0)
    {
        memset(m_data, 0, init_len);
    }

    CharDna(uint_fast64_t seed, uint_fast32_t init_len, const char* src) :
        m_data(new char[init_len]),
        m_len(init_len),
        m_seed(seed),
        m_ptr(init_len)
    {
        for(unsigned int i = 0; i < init_len; i++)
        {
            *(m_data + i) = *(src + i);
        }
    }
    
    ~CharDna()
    {
        delete[] m_data;
    }

    CharDna(const CharDna& other) = delete;

    /**
     * Sets the data at the offset to the specified char, allocating new space
     * as necessary. The offset is measured in 8-bit units.
     */
    void set_char(uint_fast32_t offset, char newData)
    {
        if(offset >= m_ptr)
        {
            m_ptr = offset + 1;
        }
        if(m_ptr > m_len)
        {
            realloc(m_ptr * 2);
        }
        *(m_data + offset) = newData;
    }

    /**
     * Adds the specified char to the end of the data array, allocating new
     * space as necessary.
     */
    void append_char(char newData)
    {
        set_char(m_ptr, newData);
    }

    char char_data(uint_fast32_t offset) const
    {
        return *(m_data + offset);
    }

    uint_fast32_t capacity() const
    {
        return m_len;
    }

    uint_fast32_t len() const
    {
        return m_ptr;
    }

    const char* all_data() const
    {
        return const_cast<const char*>(m_data);
    }

    uint_fast64_t seed() const
    {
        return m_seed;
    }
};

/**
 * Wraps a CharDna instance, which enables processing of data in 32-bit units.
 */
class Int32Dna
{
private:
    const std::shared_ptr<CharDna> m32_inst;
    /**
     * Generates the offset, in 4 byte units, to use in appending to the end of
     * the wrapped CharDna instance.
     */
    uint_fast32_t align()
    {
        uint_fast32_t offset = m32_inst->len();
        offset = offset / 4 + ((offset % 4 + 3) / 4);
        return offset;
    }

public:
    explicit Int32Dna(const std::shared_ptr<CharDna> ptr) :
        m32_inst(ptr)
    {
    }

    /**
     * Sets 4 bytes of data at the specified offset. The offset is measured in
     * 32-bit units. Aligns to the next 32-bit block.
     */
    void set_int(uint_fast32_t offset, uint_fast32_t newData)
    {
        offset *= 4;
        for(int i = 0; i < 4; i++) {
            m32_inst->set_char(offset + i, static_cast<char>(newData));
            newData >>= 8;
        }
    }

    /**
     * Adds 4 bytes of data to the end of the data array. The data is aligned
     * to the next 32-bit boundary.
     */
    void append_int(uint_fast32_t newData)
    {
        set_int(align(), newData);
    }

    /**
     * Gets 4 sequential bytes of data at the specified offset. The offset is
     * measured in 32-bit units. The data is stored in little endian format in
     * a 32-bit integer.
     */
    uint_fast32_t int_data(uint_fast32_t offset) const
    {
        uint_fast32_t data = 0;
        offset *= 4;
        for(int i = 0; i < 4; i++) {
            data <<= 8;
            data |= m32_inst->char_data(offset * 4);
        }
        return data;
    }
};

/**
 * Wraps a CharDna instance, which enables processing of data in 64-bit units.
 */
class Long64Dna
{
private:
    const std::shared_ptr<CharDna> m64_inst;
    /**
     * Generates the offset, in 64-bit units, to use in appending to the end of
     * the wrapped CharDna instance.
     */
    uint_fast32_t align()
    {
        uint_fast32_t offset = m64_inst->len();
        offset = offset / 8 + (offset % 8 + 7) / 8;
        return offset;
    }

public:
    explicit Long64Dna(std::shared_ptr<CharDna> instance) :
        m64_inst(instance)
    {
    }

    /**
     * Sets 4 bytes of data at the specified offset. The offset is measured in
     * 64-bit units. Aligns to the next 64-bit block.
     */
    void set_long(uint_fast32_t offset, uint_fast64_t newData)
    {
        offset *= 8;
        for(int i = 0; i < 8; i++) {
            m64_inst->set_char(offset + i, static_cast<char>(newData));
            newData >>= 8;
        }
    }

    /**
     * Adds 4 bytes of data to the end of the data array. The data is aligned
     * to the next 64-bit boundary.
     */
    void append_long(uint_fast64_t newData)
    {
        set_long(align(), newData);
    }

    /**
     * Gets 4 sequential bytes of data at the specified offset. The offset is
     * measured in 64-bit units. The data is stored in little endian format in
     * a 64-bit integer.
     */
    uint_fast64_t long_data(uint_fast32_t offset) const
    {
        uint_fast64_t data = 0;
        offset *= 8;
        for(int i = 0; i < 8; i++) {
            data <<= 8;
            data |= m64_inst->char_data(offset * 4);
        }
        return data;
    }
};

//Manages dna format
class Ribosome32
{
private:
    const std::shared_ptr<CharDna> inst;  
    Int32Dna wrapper;
    uint_fast32_t geneCt;
public:
    Ribosome32(const std::shared_ptr<CharDna> inst) :
        inst(inst),
        wrapper(inst)
    {}

    void addGene(Gene& gene, unsigned int gene_pos)
    {
        
    }
};

class Gene
{
private:
    uint_fast32_t info;
    uint_fast32_t data[4];
    uint_fast32_t error;
    unsigned char cur_geneCt, cur_slot;

    Gene* set_data(uint_fast32_t d1, uint_fast32_t d2, unsigned int mask, unsigned int slot)
    {
        //clear bits in slots
        *(data + 2) &= ~mask << (slot * 8);
        *(data + 3) &= ~mask << (slot * 8);
        //set data in slot
        *(data + 2) |= (d1 & mask) << (slot * 8);
        *(data + 3) |= (d2 & mask) << (slot * 8);
        return this;
    }
public:
    Gene() :
        info(0),
        data{0, 0, 0, 0},
        error(0),
        cur_geneCt(0)
    {
    }

    ~Gene()
    {
    }

    /**
     * Whether an error occurred since the last operation.
     */
    bool error()
    {
        return error;
    }

    bool err_shuffle()
    {
        return error & 0b1;
    }

    bool err_override()
    {
        return error & 0b10;
    }
    
    /*
     * Adds the data to the next available data slot. 
     * 
     * Invariant:
     *      The data blocks are always sorted from largest to smallest.
     * d1 & d2  - the data blocks to add. For dominance data, see the method
     *            append_dom(int32, int32, uint, bool).
     * type     - type. Use the defined macros, fn_BYTE, fn_SHORT, etc. Invalid
     *            values are treated as fn_BYTE.
     * safeMode - if this is set to false, the following operations are
     *            performed under the hood if there are no data slots;
     *            if the operation fails, the next one is attempted.
     *            1. Shuffling: Data is moved around to make room. The
     *               shuffling error bit is set. This also affects the
     *               dominance bits. The info bits are adjusted.
     *            2. Overriding: Data is overridden. The overriding error
     *               bit is set, as well as the bits that were overriden.
     *            If set to true, the method will do nothing if there are no
     *            data slots.
     */ 
    Gene* append_data(uint_fast32_t d1, uint_fast32_t d2, unsigned int type, bool safeMode)
    {

        switch(type)
        {
            //16-bit
            case fn_SHORT:
                if(cur_slot)
                {
                    
                }
                break;
            case fn_INT:
                if(cur_slot == 0)
                {

                }
                break;
            case fn_LONG:
                break;
            default:
                break;
        }
        return this;
    }

    uint_fast32_t get_info()
    {
        return info;
    }

    const uint_fast32_t* get_data()
    {
        return const_cast<const uint_fast32_t*>(data);
    }
};

struct dna_read
{
    const size_t size;
    const CharDna** data;

    dna_read(size_t size, const CharDna** d) :
        size(size),
        data(d)
    {
    }
};

//Ensure little-endianness.
static void write_int32(std::ofstream* stream, uint_fast32_t in)
{
    char buf[4];
    for(unsigned int i = 0; i < 4; i++)
    {
        *(buf + i) = static_cast<char>(in);
        in >>= 8;
    }
    stream->write(buf, 4);
}

static void write_int64(std::ofstream* stream, uint_fast64_t in)
{
    char buf[8];
    for(unsigned int i = 0; i < 8; i++)
    {
        *(buf + i) = static_cast<char>(in & 0xff);
        in >>= 8;
    }
    stream->write(buf, 8);
}

//Ensure little-endianness
static uint_fast32_t read_int32(std::ifstream* stream)
{
    uint_fast32_t result = 0;
    if(stream->is_open())
    {
        char buf[4];
        stream->read(buf, 4);
        for(unsigned int i = 0; i < 4; i++)
        {
            result |= (*(buf + i) & 0xff) << (8 * i);
        }
    }
    return result;

}

static uint_fast64_t read_int64(std::ifstream* stream)
{
    uint_fast64_t result = 0;
    if(stream->is_open())
    {
        char buf[8];
        stream->read(buf, 8);
        for(int i = 0; i < 8; i++)
        {
            result |= (*(buf+i) & 0xff) << (8 * i);
        }
    }
    return result;
}

/**
 * Deserializes the dna objects in the file pointed to by the path.
 */
static std::unique_ptr<dna_read> deserialize(const char* path)
{
    size_t s = 0;
    CharDna** data = nullptr;
    std::ifstream file;
    file.open(path, std::ios::binary);
    if(file.is_open())
    {
        s = static_cast<size_t>(read_int32(&file));
        data = new CharDna*[s];
        uint_fast32_t dna_len;
        uint_fast64_t dna_seed;
        char* dna_data = nullptr;
        for(unsigned int i = 0; i < s; i++)
        {

            dna_len = read_int32(&file);
            if(read_int32(&file) != fn_UNIT_SIZE)
            {
                //Error, cannot read the unit size.
                file.close();
                return nullptr;
            }
            dna_seed = read_int64(&file);
            
            while(read_int32(&file) != '\n')
            {

            }
            if(dna_data == nullptr || sizeof(dna_data) < dna_len)
            {
                dna_data[dna_len];
            }
            file.read(dna_data, dna_len);
            if(!file.bad())
            {
                *(data + i) = new CharDna(dna_seed, dna_len, const_cast<const char*>(dna_data));
            }
            
        }
    }
    file.close();
    std::unique_ptr<dna_read> result = std::make_unique<dna_read>(new dna_read(s, const_cast<const CharDna**>(data)));
    return result;
}

/**
 * Serializes the dna objects to the specified file path.
 */
static void serialize(const char* path, std::initializer_list<CharDna*> list)
{
    std::ofstream file;
    file.open(path, std::ios::binary | std::ios::trunc);
    if(file.is_open())
    {   
        size_t s = list.size();
        //Ensure endianness is constant
        write_int32(&file, s);
        uint_fast32_t dna_len;
        for(CharDna* d : list) 
        {
            dna_len = d->len();
            write_int32(&file, dna_len); //size
            write_int32(&file, fn_UNIT_SIZE); //unit size
            write_int64(&file, d->seed()); //seed
            write_int32(&file, fn_TYPEDDNA_ID); //typed dna id.
            write_int32(&file, '\n');
            file.write(d->all_data(), dna_len);
        }
        file.flush();
    }
    file.close();

}


//Testing
int main() {
    CharDna dna(0, 16);
    Long64Dna wrap64(std::make_shared<CharDna>(dna));
    Int32Dna wrap32(std::make_shared<CharDna>(dna));
    wrap32.append_int(0xff04);
    wrap64.append_long(0xffffffffffff11);
    for(unsigned int i = 0; i < dna.capacity(); i++)
    {
        std::cout << (unsigned int)(dna.char_data(i) & 0xff) << '-';
    }
    std::cout <<std::endl;
    return 0;
}
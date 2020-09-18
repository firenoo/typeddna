#include <string.h>
#include <iostream>
#include <fstream>


//This file provides the classes for manipulating data on DNA, according to my
//typed data format.

/**
 * Base character class for holding DNA data. Contains methods for manipulating
 * single bytes of data (8-bits, char).
 * 
 */
class CharDna
{
protected:

    char* m_data;
    uint_fast32_t m_len;
    uint_fast64_t m_seed;
    uint_fast32_t m_ptr;
    
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

    char char_data(uint_fast32_t offset)
    {
        return *(m_data + offset);
    }

    uint_fast32_t capacity()
    {
        return m_len;
    }

    uint_fast32_t len()
    {
        return m_ptr;
    }

    const char* all_data()
    {
        return const_cast<const char*>(m_data);
    }

    uint_fast64_t seed()
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
    CharDna& m32_inst;
    /**
     * Generates the offset, in 4 byte units, to use in appending to the end of
     * the wrapped CharDna instance.
     */
    uint_fast32_t align()
    {
        uint_fast32_t offset = m32_inst.len();
        offset = offset / 4 + ((offset % 4 + 3) / 4);
        return offset;
    }

public:
    Int32Dna(CharDna& instance) :
        m32_inst(instance)
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
            m32_inst.set_char(offset + i, static_cast<char>(newData));
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
    uint_fast32_t int_data(uint_fast32_t offset)
    {
        uint_fast32_t data = 0;
        offset *= 4;
        for(int i = 0; i < 4; i++) {
            data <<= 8;
            data |= m32_inst.char_data(offset * 4);
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
    CharDna& m64_inst;
    /**
     * Generates the offset, in 64-bit units, to use in appending to the end of
     * the wrapped CharDna instance.
     */
    uint_fast32_t align()
    {
        uint_fast32_t offset = m64_inst.len();
        offset = offset / 8 + (offset % 8 + 7) / 8;
        return offset;
    }

public:
    Long64Dna(CharDna& instance) :
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
            m64_inst.set_char(offset + i, (char)newData);
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
    uint_fast64_t long_data(uint_fast32_t offset)
    {
        uint_fast64_t data = 0;
        offset *= 8;
        for(int i = 0; i < 8; i++) {
            data <<= 8;
            data |= m64_inst.char_data(offset * 4);
        }
        return data;
    }
};

struct dna_read
{
    const size_t size;
    const CharDna** data;

    dna_read(size_t size, const CharDna*** d) :
        size(size),
        data(*d)
    {
    }
    ~dna_read()
    {
        delete[] data;
    }
};

//Ensure little-endianness.
static void write_int32(std::ofstream* stream, uint_fast32_t in)
{
    char* buf = new char[4];
    for(unsigned int i = 0; i < 4; i++)
    {
        *(buf + i) = static_cast<char>(in & 0xff);
        in >>= 8;
    }
    stream->write(buf, 4);
    delete[] buf;
}

static void write_int64(std::ofstream* stream, uint_fast64_t in)
{
    char* buf = new char[8];
    for(unsigned int i = 0; i < 8; i++)
    {
        *(buf + i) = static_cast<char>(in & 0xff);
        in >>= 8;
    }
    stream->write(buf, 8);
    delete[] buf;
}

//Ensure little-endianness
static uint_fast32_t read_int32(std::ifstream* stream)
{
    uint_fast32_t result = 0;
    if(stream->is_open())
    {
        char* buf = new char[4];
        stream->read(buf, 4);
        for(unsigned int i = 0; i < 4; i++)
        {
            result |= (*(buf + i) & 0xff) << (8 * i);
        }
        delete[] buf;

    }
    return result;

}

static uint_fast64_t read_int64(std::ifstream* stream)
{
    uint_fast64_t result = 0;
    if(stream->is_open())
    {
        char* buf = new char[8];
        stream->read(buf, 8);
        for(int i = 0; i < 8; i++)
        {
            result |= (*(buf+i) & 0xff) << (8 * i);
        }
        delete[] buf;
    }
    return result;
}

/**
 * Deserializes the dna objects in the file pointed to by the path.
 */
static dna_read* deserialize(const char* path)
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

            dna_seed = read_int64(&file);
            dna_len = read_int32(&file);
            if(dna_data == nullptr || sizeof(dna_data) < dna_len)
            {
                delete[] dna_data;
                dna_data = new char[dna_len];
            }      
            file.read(dna_data, dna_len);
            *(data + i) = new CharDna(dna_seed, dna_len, const_cast<const char*>(dna_data));
            
        }
    }
    file.close();
    dna_read* result = new dna_read(s, const_cast<const CharDna***>(&data));
    return result;
}

/**
 * Serializes the dna objects to the specified file path.
 */
void serialize(const char* path, std::initializer_list<CharDna*> list)
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
            write_int64(&file, d->seed());
            write_int32(&file, dna_len);
            file.write(d->all_data(), dna_len);
        }
        file.flush();
    }
    file.close();

}



int main() {
    CharDna dna(0, 16);
    Long64Dna wrap64(dna);
    Int32Dna wrap32(dna);
    wrap32.append_int(0xff04);
    wrap64.append_long(0xffffffffffff11);
    for(unsigned int i = 0; i < dna.capacity(); i++)
    {
        std::cout << (unsigned int)(dna.char_data(i) & 0xff) << '-';
    }
    std::cout <<std::endl;
    serialize("test.bin", {&dna});
    dna_read* r = deserialize("test.bin");
    dna = **(r->data);
    std::cout << dna.len() << std::endl;
    for(unsigned int i = 0; i < dna.capacity(); i++)
    {
        std::cout << (unsigned int)(dna.char_data(i) & 0xff) << '-';
    }
    std::cout << std::endl;
    return 0;
}
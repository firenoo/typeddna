#include <string.h>
#include <iostream>
//This file provides the classes for manipulating data on DNA, according to my
//data format.

/**
 * Base character class for holding DNA data. Contains methods for manipulating
 * single bytes of data (8-bits, char).
 * 
 */
class CharDna
{
protected:

    char* m_data;
    unsigned int m_len;
    unsigned long long m_seed;
    unsigned int m_ptr;
    
    void realloc(unsigned int newLen)
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
    CharDna(long long seed, unsigned int init_len) :
        m_data(new char[init_len]),
        m_len(init_len),
        m_seed(seed),
        m_ptr(0)
    {
        memset(m_data, 0, init_len);
    }
    
    ~CharDna()
    {
        delete[] m_data;
    }

    /**
     * Sets the data at the offset to the specified char, allocating new space
     * as necessary. The offset is measured in 8-bit units.
     */
    void set_char(unsigned int offset, char newData)
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

    char char_data(unsigned int offset)
    {
        return *(m_data + offset);
    }

    unsigned int capacity()
    {
        return m_len;
    }

    unsigned int len()
    {
        return m_ptr;
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
     * Generates the offset, in 32-bit units, to use in appending to the end of
     * the wrapped CharDna instance.
     */
    unsigned int align()
    {
        unsigned int offset = m32_inst.len();
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
    void set_int(unsigned int offset, uint_fast32_t newData)
    {
        offset *= 4;
        for(int i = 0; i < 4; i++) {
            m32_inst.set_char(offset + i, (char)newData);
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
    uint_fast32_t int_data(unsigned int offset)
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
    unsigned int align()
    {
        unsigned int offset = m64_inst.len();
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
    void set_long(unsigned int offset, uint_fast64_t newData)
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
    uint_fast64_t long_data(unsigned int offset)
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

int main() {
    CharDna dna(0, 16);
    Long64Dna wrap64(dna);
    Int32Dna wrap32(dna);
<<<<<<< HEAD
=======
    wrap64.append_long(0xffffffffffffffff);
    dna.append_char(4);
    wrap32.append_int(0xffffffff);
>>>>>>> 99aadbf21f97f4354f22f539d5a7dde3ebd26cfd
    wrap32.append_int(0xff04);
    for(unsigned int i = 0; i < dna.capacity(); i++) {
        std::cout << (unsigned int)(dna.char_data(i) & 0xff) << '-';
    }
    std::cout << std::endl;
    return 0;
}
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <vector>
#include <type_traits>

#define fn_UNIT_SIZE 16 //unit size, in bytes (8-bit units)
#define fn_TYPEDDNA_ID 1

#define fn_BYTE 1
#define fn_SHORT 2
#define fn_INT 4
#define fn_LONG 8
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
        memcpy(m_data, src, init_len);
    }

    CharDna(const CharDna& other) :
        m_data(new char[other.m_len]),
        m_len(other.m_len),
        m_seed(other.m_seed),
        m_ptr(0)
    {
        memcpy(m_data, other.m_data, m_len);
    }
    
    ~CharDna()
    {
        delete[] m_data;
    }

    char operator[](uint_fast32_t offset)
    {
        return *(m_data + offset);
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
    explicit Int32Dna(std::shared_ptr<CharDna> ptr) :
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

class Gene;

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

#define errOVERRIDE 1
class Gene
{
private:
    //(0) header
    //(1) data1 - chunks are always stored biggest to smallest.
    //(2) data2
    //(3) dominance
    uint_fast64_t m_data[5];
    uint_fast32_t m_error;
    //# of bytes, shorts, ints.
    unsigned char m_slot;

    Gene* set_data(uint_fast64_t d1, uint_fast64_t d2, unsigned int mask, unsigned int slot)
    {
        //clear bits in slots
        *(m_data + 2) &= ~mask << (slot * 8);
        *(m_data + 3) &= ~mask << (slot * 8);
        //set data in slot
        *(m_data + 2) |= (d1 & mask) << (slot * 8);
        *(m_data + 3) |= (d2 & mask) << (slot * 8);
        return this;
    }

public:
    Gene() :
        m_data{0, 0, 0, 0, 0},
        m_error(0),
        m_slot(0)
    {
    }

    /*
     * Whether an error occurred since the last operation.
     */
    bool is_err()
    {
        return m_error;
    }

    void clear_err()
    {
        m_error = 0;
    }


    /*
     * Clears the specifed error bit(s). Argument is a bit field for which
     * each set bit is cleared in the error bit.
     */
    void clear_err(uint_fast32_t bit)
    {
        m_error &= ~bit;
    }

    bool err_override()
    {
        return (m_error & errOVERRIDE);
    }
    
    /*
     * Adds the 64-bit data to the next available data slot. On success, all
     * error flags are cleared.
     * 
     * d1, d2   - the data blocks to add.
     * force    - If there is not enough space in this gene, setting this to true will
     *            allow the algorithm to override bits starting from the oldest entry. The
     *            override flag is then set. Otherwise this method does nothing.
     * Special case for 64-bit: If force is true, all bits are overriden; it is equivalent
     * to calling clear_data() then calling this method. The override error flag is then set.
     * Otherwise, this method does nothing.
     */ 
    Gene* append_64(uint_fast64_t d1, uint_fast64_t d2, char dom1, char dom2, bool force)
    {
        if(force)
        {
            clear_data();
            m_error |= errOVERRIDE;   
        } else if(m_slot != 0)
        {
            //Do nothing
            return this;
        }
        m_data[1] = d1;
        m_data[2] = d2;
        m_data[3] |= dom1;
        m_data[4] |= dom2;
        m_slot = 8;
    }

    /*
     * Adds the 32-bit data to the next available data slot. On success, all
     * error flags are cleared.
     * 
     * d1, d2   - the data blocks to add.
     * force    - If there is not enough space in this gene, setting this to true will
     *            allow the algorithm to override bits starting from the oldest entry. The
     *            override flag is then set. Otherwise this method does nothing and the override 
     *            flag is set.
     * There are up to 2 available slots for 32-bit data.
     */ 
    Gene* append_32(uint_fast32_t d1, uint_fast32_t d2, char dom1, char dom2, bool force)
    {
        char index;
        if(m_slot > 4 && !force)
        {
            m_error |= 1;
            return this;
        } else if(m_slot > 4)
        {
            m_slot = 4;
            index = 32;
            m_data[1] &= 0xffffffff;
            m_data[2] &= 0xffffffff;
            m_error |= errOVERRIDE;
            m_data[3] &= 0xff << m_slot * 8;
            m_data[4] &= 0xff << m_slot * 8;
        } else
        {
            index = m_slot * 8;
        }
        
        m_data[1] |= d1 << index;
        m_data[2] |= d2 << index;
        m_data[3] |= dom1 << index;
        m_data[4] |= dom2 << index;
        m_slot += 4;
    }

   /*
     * Adds the 16-bit data to the next available data slot. On success, all
     * error flags are cleared.
     * 
     * d1, d2   - the data blocks to add.
     * force    - If there is not enough space in this gene, setting this to true will
     *            allow the algorithm to override bits starting from the oldest entry. The
     *            override flag is then set. Otherwise this method does nothing and the override 
     *            flag is set.
     * There are up to 4 available slots for 16-bit data.
     */ 
    Gene* append_32(uint_fast32_t d1, uint_fast32_t d2, char dom1, char dom2, bool force)
    {
        char index;
        if(m_slot > 6 && !force)
        {
            m_error |= 1;
            return this;
        } else if(m_slot > 4)
        {
            m_slot = 4;
            index = 48;
            m_data[1] &= 0xffffffff;
            m_data[2] &= 0xffffffff;
            m_error |= errOVERRIDE;
            m_data[3] &= 0xff << m_slot * 8;
            m_data[4] &= 0xff << m_slot * 8;
        } else
        {
            index = m_slot * 8;
        }
        
        m_data[1] |= d1 << index;
        m_data[2] |= d2 << index;
        m_data[3] |= dom1 << index;
        m_data[4] |= dom2 << index;
        m_slot += 4;
    }
    /*
     * Adds the 8-bit data to the next available data slot.
     * 
     * d1, d2   - the data blocks to add.
     * type     - type. Use the defined macros, fn_BYTE, fn_SHORT, etc. Invalid
     *            values are treated as fn_BYTE.
     *            If there is not enough room, 
     */ 
    Gene* append_8(uint_fast32_t d1, uint_fast32_t d2, char dom1, char dom2)
    {
        
        
        return this;
    }



    Gene* clear_data()
    {
        m_slot = 0;
        m_data[1] = 0;
        m_data[2] = 0;
        m_data[3] = 0;
        return this;
    }

    const uint_fast32_t* get_data() const
    {
        return const_cast<const uint_fast32_t*>(m_data);
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
 * Inserts all deserialized dna objects into the supplied std::vector.
 */
static int deserialize(const std::string& path, std::vector<CharDna>& vec)
{
    size_t size = 0;
    std::ifstream file;
    file.open(path, std::ios::binary);
    if(file.is_open())
    {
        size = static_cast<size_t>(read_int32(&file));
        uint_fast32_t dna_len;
        uint_fast64_t dna_seed;
        for(unsigned int i = 0; i < size; i++)
        {

            dna_len = read_int32(&file);
            if(read_int32(&file) != fn_UNIT_SIZE)
            {
                //Error, cannot read the unit size.
                file.close();
                return 0;
            }
            dna_seed = read_int64(&file);
            
            while(read_int32(&file) != '\n')
            {
                //Skip header bytes that are not used in this impl.
            }
            char dna_data[dna_len];
            char* ptr = &(dna_data[0]);
            file.read(ptr, dna_len);
            if(file.eof())
            {
                file.close();
                return 0;
            }
            if(!file.bad())
            {
                vec.emplace_back(dna_seed, dna_len, ptr);
            }
        }
        file.close();
        return 1;
    }
    return 0;
}

/**
 * Serializes the dna objects to the specified file path.
 */
static void serialize(const std::string& path, std::initializer_list<CharDna*> list)
{
    std::ofstream file;
    file.open(path, std::ios::binary | std::ios::trunc);
    if(file.is_open())
    {   
        size_t s = list.size();
        //Ensure endianness is constant
        write_int32(&file, s);
        uint_fast32_t dna_len;
        for(const CharDna* d : list) 
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
    std::shared_ptr<CharDna> dptr = std::make_shared<CharDna>(0, 16);
    Long64Dna wrap64(dptr);
    Int32Dna wrap32(dptr);
    wrap32.append_int(0xff04);
    wrap64.append_long(0xffffffffffff11);
    for(unsigned int i = 0; i < dptr->capacity(); i++)
    {
        std::cout << (unsigned int)(dptr->char_data(i) & 0xff) << '-';
    }
    std::cout <<std::endl;
    serialize("test.bin", {dptr.get()});
    std::vector<CharDna> result;
    result.reserve(2);
    if(deserialize("test.bin", result))
    {
        for(CharDna& d : result)
        {
            for(unsigned int i = 0; i < d.capacity(); i++)
            {
                std::cout << (unsigned int)(d.char_data(i) & 0xff) << '-';
            }
        }
    }
    std::cout <<std::endl;
    return 0;
}
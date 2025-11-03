module soft_crc;
/++

Refs: каталог CRC: https://reveng.sourceforge.io/crc-catalogue/all.htm
+/

import std.typecons : Flag, No, Yes;

pure nothrow @nogc
template crc(T, T Polynom, Flag!"Refin" Refin = No.Refin /* , bool Refout = false */)
if(is(T == ubyte) || is(T == ushort) || is(T == uint))
{
    enum CRCsize = T.sizeof * 8;
    enum isCRC_8 = CRCsize == 8;
    enum MSB = 1 << (CRCsize - 1);

    static if (Refin){
        T reflect(T x)
        {
            // from boost::crc::reflect_unsigned
            for (T l = 1u, h = cast(T)(l << (CRCsize - 1)) ; h > l ; h >>= 1, l <<= 1 )
            {
                const T m = h | l, t = x & m;

                if ( (t == h) || (t == l) )
                    x ^= m;
            }
            return x;
        }

        unittest{
            assert(reflect(MSB) == 1);
            static if(is(T == ubyte)){
                assert(reflect(0xF0) == 0x0F);
                assert(false == __traits(compiles, reflect(ubyte.max + 1)));
            }else static if (is(T == ushort)){
                assert(reflect(0xF0_F0) == 0x0F_0F);
                assert(false == __traits(compiles, reflect(ushort.max + 1)));
            }else static if (is(T == uint)){
                assert(reflect(0xF0_F0_F0_F0) == 0x0F_0F_0F_0F);
                assert(false == __traits(compiles, reflect(uint.max + 1L))); // suffix L важен
            }
        }
    }

    static if(Refin && is(T == ubyte)){
        enum P = reflect(Polynom); /// Polynom, если Refin и CRC-8 то перевернутый
    } else
        enum P = Polynom;

    version(CRC_No_Table)
    {
        T crc(T crc, scope const (ubyte)* p_data, size_t size)
        {
            while(size--){
                ubyte d = *(p_data++);
                static if (Refin && !is(T == ubyte))
                    d = reflect(d);

                static if(isCRC_8)
                    crc ^= d;
                else
                    crc ^= d << 8;

                foreach(_; 0..8){
                    static if(Refin)
                        crc = (crc & 1 ? (crc >> 1) ^ P : crc >> 1);
                    else
                        crc = cast(ubyte)(crc & MSB ? (crc << 1) ^ P : crc << 1);
                }
            }
            return crc;
        }
    }
    else
    {
        immutable T[256] table = {
            import std.traits : Signed;
            // функция что существует только в компл-тайме
            // в objdump она не отображается
            // с таблицей даже размер text в бинарнике меньше
            T[256] t = void;
            foreach (T x; 0..256) {
                T crc = x;
                foreach (_; 0..8) {
                    static if(Refin)
                        crc = (crc & 1) ? (crc >> 1) ^ P : crc >> 1;
                    else
                        crc = cast(ubyte)(crc & MSB ? (crc << 1) ^ P : crc << 1);
                }
                t[x] = crc;
            }
            return t;
        }();

        T crc(T crc, scope const(ubyte)* p_data, size_t size)
        {
            while(size--){
                static if(isCRC_8)
                    crc = table[crc ^ *(p_data++)];
                else{
                    static if(Refin)
                        crc = (crc >> 8) ^ table[cast(ubyte)(crc & 0xFF) ^ *(p_data++)];
                    else
                        crc = cast(T)(crc << 8) ^ table[(crc >> (CRCsize - 8)) ^ *(p_data++)];
                }
            }
            return crc;
        }
    }
}

version(D_BetterC) extern(C) __gshared{

// /// Полином: 0x31 = x^8 + x^5 + x^4 + 0
// ubyte crc8_31 (ubyte init, scope const (ubyte)* p_data, size_t size)
// pure
// {
//     return crc!(ubyte, 0x31)(init, p_data, size);
// }

// надо ли сделать функцию, что будет пошагово делать
// принял байт, вызвал, принял вызвал ещё раз
// можно исп и текущею, но тут лишний цикл, можно оптимизировать
}

import core.stdc.stdio;

version(unittest){
    immutable ubyte[9] check = cast(ubyte[9]) "123456789";

    struct CRC_unit {
        string name;
        size_t polynom;
        string comment;
    }

    version(D_BetterC)
    extern(C)
    void main (){
        import std.traits : hasUDA, getUDAs, select;

        static foreach(u; __traits(getUnitTests, __traits(parent, main))){
            static if(hasUDA!(u, CRC_unit)){
                // pragma(msg, getUDAs!(u, CRC_unit));
                u();
                printf("Unittest %s passed\n", getUDAs!(u, CRC_unit)[0].name.ptr);
            }
        }
    }
}

unittest
{
    alias crc8 = crc!(ubyte, 0, Yes.Refin); // проверка reflect unittest
    alias crc16 = crc!(ushort, 0, Yes.Refin); // проверка reflect unittest
    alias crc32 = crc!(uint, 0, Yes.Refin); // проверка reflect unittest
}


@CRC_unit("CRC-8", 0x31, "from SHT20")
unittest
{
    alias crc8_31 = crc!(ubyte, 0x31);

    struct Test (T, size_t size){
        T[size] data;
        T check;
    }

    Test!(ubyte, 2)[4] datas = [
        Test!(ubyte, 2)([0x6E, 0xA8], 0x7D),
        Test!(ubyte, 2)([0x6A, 0x86], 0x67),
        Test!(ubyte, 2)([0x6E, 0xA0], 0xC4),
        Test!(ubyte, 2)([0x6B, 0xC2], 0x6A)
    ];

    for(size_t i = 0; i < datas.length; i++)
    {
        assert(crc8_31(0, datas[i].data.ptr, 2) == datas[i].check);
    }
}

@CRC_unit("CRC-8/NRSC-5", 0x31)
unittest
{
    assert(crc!(ubyte, 0x31)(0xFF, check.ptr, check.length) == 0xF7); // CRC-8/NRSC-5 в каталоге
}

@CRC_unit("CRC-8/CDMA2000", 0x9b)
unittest
{
    assert(crc!(ubyte, 0x9b)(0xFF, check.ptr, check.length) == 0xda);
}

@CRC_unit("CRC-8/DVB-S2", 0xd5)
unittest
{
    assert(crc!(ubyte, 0xd5)(0, check.ptr, check.length) == 0xbc);
}

@CRC_unit("CRC-8/BLUETOOTH", 0xA7)
unittest
{
    assert(crc!(ubyte, 0xa7, Yes.Refin)(0, check.ptr, check.length) == 0x26);
}

// @CRC_unit("CRC-16/GSM", 0x1021)
// unittest
// {
//     assert((crc!(ushort, 0x1021)(0, check.ptr, check.length) ^ 0xFFFF) == 0xCE3C);
// }

@CRC_unit("CRC-16/AUG-CCITT", 0x1021)
unittest
{
    assert(crc!(ushort, 0x1021)(0x1d0f, check.ptr, check.length) == 0xe5cc);
}

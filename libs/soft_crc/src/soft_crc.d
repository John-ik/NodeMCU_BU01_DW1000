module soft_crc;
/++

Refs: каталог CRC: https://reveng.sourceforge.io/crc-catalogue/all.htm
+/
import std.typecons : Flag, No, Yes;
import std.traits : isUnsigned;


T reflect(ubyte w, T)(T x) @safe pure nothrow @nogc
if (isUnsigned!T && w <= 8*T.sizeof)
{
    // from boost::crc::reflect_unsigned
    for (T l = 1u, h = cast(T)(l << (w - 1)) ; h > l ; h >>= 1, l <<= 1 )
    {
        const T m = h | l, t = x & m;

        if ( (t == h) || (t == l) )
            x ^= m;
    }
    return x;
}

unittest{
    assert(reflect!2(1u) == 0b10);
    assert(reflect!3(1u) == 0b100);
    assert(reflect!5(0b101_u) == 0b10100);
    assert(reflect!8(1u << 7) == 1);
    assert(reflect!8(0xF0_u) == 0x0F);
    assert(reflect!16(0xF0_F0_u) == 0x0F_0F);
    assert(reflect!32(0xF0_F0_F0_F0_u) == 0x0F_0F_0F_0F);
    assert(reflect!64(1uL) == 1L << 63);
}


pure nothrow @nogc
template crc(T, T Polynom, Flag!"Refin" Refin = No.Refin, Flag!"Refout" Refout = No.Refout)
if(is(T == ubyte) || is(T == ushort) || is(T == uint) || is(T == ulong))
{
    enum width = T.sizeof * 8;
    enum T dataShift = width - 8;
    enum bool isCRC_8 = width == 8;
    enum T MSB = 1uL << (width - 1);

    T step(ubyte data, T reg) @safe {
        static if(Refin) data = reflect!8(data);

        reg ^= (cast(T)data << dataShift);

        foreach(_; 0..8){ // деление на Polynom (mod 2)
            if (reg & MSB)
                reg = cast(T)(reg << 1) ^ Polynom;
            else
                reg <<= 1;
        }
        return reg;
    }

    version(CRC_No_Table)
    {
        T crc(T init, scope const (ubyte)* p_data, size_t size)
        {
            T reg = init;
            while(size--){
                ubyte d = *(p_data++);

                static if(Refin) d = reflect!8(d);

                reg ^= (d << DataShift);

                foreach(_; 0..8){ // деление на Polynom (mod 2)
                    if (reg & MSB)
                        reg = cast(T)(reg << 1) ^ Polynom;
                    else
                        reg <<= 1;
                }
            }
            static if (Refout)
                return reflect!(CRCsize)(reg);
            else
                return reg;
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
            foreach (ubyte x; 0..256) {
                static if (Refin)
                    T crc = reflect!8(x) << (CRCsize - 8);
                else
                    T crc = x << (CRCsize - 8); // свдиг на 0 же оптимизурется, верно?
                foreach (_; 0..8) {
                    crc = cast(ubyte)(crc & MSB ? (crc << 1) ^ Polynom : crc << 1);
                }
                static if (Refin)
                    t[x] = reflect!CRCsize(crc);
                else
                    t[x] = crc;
            }
            return t;
        }();

        T crc(T crc, scope const(ubyte)* p_data, size_t size)
        {
            static if(Refin)
                crc = reflect!CRCsize(crc);

            while(size--){
                static if(isCRC_8)
                    crc = table[crc ^ *(p_data++)];
                else{
                    static if(Refin)
                        crc = (crc >> 8) ^ table[cast(ubyte)(crc & 0xFF) ^ *(p_data++)];
                    else
                        crc = cast(T)(crc << 8) ^ table[cast(ubyte)((crc >> (CRCsize - 8)) & 0xFF ) ^ *(p_data++)];
                }
            }
            static if (Refout)
                return reflect!CRCsize(crc);
            else
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
    struct CRC_unit {
        string name;
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

unittest // from SHT20
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

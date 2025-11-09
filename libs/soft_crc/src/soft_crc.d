module soft_crc;

import std.typecons : Flag, No, Yes; //? public import
import std.traits : isUnsigned;
import std.digest : isDigest;

/++
Побитовый разворт числа.

Params:
    w = число бит в числе, именно внутри них разворачивается.
    Если нечетный, то середина остается на месте
    T = беззнаковое целое
    a = число которое надо перевернуть

Return: развернутый a
+/
T reflect(ubyte w, T)(const T a) @safe pure nothrow @nogc
if (isUnsigned!T && w <= 8*T.sizeof)
{
    T x = a;
    // from boost::crc::reflect_unsigned
    for (T l = 1u, h = cast(T)(l << (w - 1)) ; h > l ; h >>= 1, l <<= 1 )
    {
        const T m = h | l, t = x & m;

        if ( (t == h) || (t == l) )
            x ^= m;
    }
    return x;
}
///
@safe pure nothrow @nogc
unittest {
    assert(reflect!2(1u) == 0b10);
    assert(reflect!3(1u) == 0b100);
    assert(reflect!5(0b101_u) == 0b10100);
    assert(reflect!8(1u << 7) == 1);
    assert(reflect!8(0xF0_u) == 0x0F);
    assert(reflect!16(0xF0_F0_u) == 0x0F_0F);
    assert(reflect!32(0xF0_F0_F0_F0_u) == 0x0F_0F_0F_0F);
    assert(reflect!64(1uL) == 1L << 63);
}

/++
Универсальный параметрическйи CRC реализующий Template API `std.digest`.

Задается по модели Rocksoft™.
Таблица задана как `static`, т.е. для одинаковых интансов шаблона будет одна.
Она генерируется в CTFE.

Params:
    T = беззнаковое целое в котором поместиться CRC
    Polynom = в нормальной (прямой нотации): x^8 + x^5 + x^4 + 0 = 0x31
    init = значение с которого начинается
    refin = отражать ли входные байты, обратный порядок битов
    refout = отражение бит в результате
    xorout = выполнить XOR перед выдачей результата (после refout, если задан)

Bugs: Не реализован параметр *residue*. Зачем он я не понял

See_Also:
- $(LINK http://en.wikipedia.org/wiki/Cyclic_redundancy_check)
- Ross N. Williams/Anarchriz. Всё о CRC32 $(LINK https://web.archive.org/web/20130407000813/http://rsdn.ru/article/files/classes/SelfCheck/crcguide.pdf)
- Каталог CRC $(LINK https://reveng.sourceforge.io/crc-catalogue/all.htm)
+/
struct CRC(T, T Polynom, T init = 0,
    Flag!"refin" refin = No.refin, Flag!"refout" refout = No.refout,
    T xorout = 0)
if(is(T == ubyte) || is(T == ushort) || is(T == uint) || is(T == ulong))
{
    enum width = T.sizeof * 8; /// ширина CRC в битах
    private enum T dataShift = width - 8; /// сдвиг даты на входе
    private enum bool isCRC_8 = width == 8;
    private enum T MSB = 1uL << (width - 1); /// старшый бит

    T reg = init;

    /// сбрасывает состояние. Релизует `isDigest`
    void start() @safe pure nothrow @nogc {
        reg = init;
    }

    /// ввод data на вход CRC. Релизует `isDigest`
    void put(scope const(ubyte)[] data...) @safe pure nothrow @nogc {
        for(size_t i = 0; i < data.length; i++){
            T d = data[i];
            static if(refin) d = reflect!8(d);

            reg ^= (cast(T)d << dataShift);

            foreach(_; 0..8){ // деление на Polynom (mod 2)
                if (reg & MSB)
                    reg = cast(T)(reg << 1) ^ Polynom;
                else
                    reg <<= 1;
            }
        }
    }

    /// получить как число. (после refout и xorout, если заданы)
    T get() const @safe pure nothrow @nogc {
        static if(refout)
            return reflect!(width)(reg) ^ xorout;
        else
            return reg ^ xorout;
    }

    /// получить байты и сбросить. Релизует `isDigest`
    ubyte[T.sizeof] finish() @safe pure nothrow @nogc {
        scope(exit) this.start();
        return this.peek();
    }

    /// получить байты (без сброса). Релизует `isDigest`
    ubyte[T.sizeof] peek() const @safe pure nothrow @nogc {
        import std.bitmanip : nativeToLittleEndian;
        return nativeToLittleEndian(this.get());
    }

    
    // version(None)
    // {
    //     static immutable T[256] table = {
    //         // функция что существует только в компл-тайме
    //         // в objdump она не отображается
    //         // с таблицей даже размер text в бинарнике меньше
    //         T[256] t = void;
    //         foreach (ubyte i; 0..256) {
    //             T a = step(i, 0);
    //             static if (refin) a = reflect!width(a);
    //             t[i] = a;
    //         }
    //         return t;
    //     }();

    //     T crc(T init, scope const(ubyte)* p_data, size_t size)
    //     {
    //         T reg = init;
    //         while(size--){
    //             static if (isCRC_8){
    //                 reg = table[reg ^ *(p_data++)];
    //             }else{
    //                 static if(refin)
    //                     reg = (reg >> 8) ^ table[cast(ubyte)(reg & 0xFF) ^ *(p_data++)];
    //                 else
    //                     reg = cast(T)(reg << 8) ^ table[cast(ubyte)((reg >> (width - 8)) & 0xFF ) ^ *(p_data++)];
    //             }
    //         }
    //         return reg;
    //     }
    // }
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


version(unittest){
    struct CRC_unit {
        string name;
    }

    version(D_BetterC)
    extern(C)
    void main (){
        import std.traits : hasUDA, getUDAs, select;
        import core.stdc.stdio;

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
    alias crc8 = CRC!(ubyte, 0, 0, Yes.refin);
    alias crc16 = CRC!(ushort, 0, 0, Yes.refin);
    alias crc32 = CRC!(uint, 0, 0, Yes.refin);
    alias crc64 = CRC!(ulong, 0, 0, Yes.refin);

    static assert(isDigest!crc8);
    static assert(isDigest!crc16);
    static assert(isDigest!crc32);
    static assert(isDigest!crc64);

    static assert(__traits(isPOD, crc8));
    static assert(__traits(isPOD, crc16));
    static assert(__traits(isPOD, crc32));
    static assert(__traits(isPOD, crc64));
}

unittest // from SHT20
{
    CRC!(ubyte, 0x31u, 0) crc;
    
    static assert(isDigest!(typeof(crc)));

    ubyte[3][4] datas = [// CRC
        [0x6E, 0xA8,       0x7D],
        [0x6A, 0x86,       0x67],
        [0x6E, 0xA0,       0xC4],
        [0x6B, 0xC2,       0x6A]
    ];

    for(size_t i = 0; i < datas.length; i++)
    {
        crc.put(datas[i]);
        assert(crc.finish == [0]);
    }
}


version(D_BetterC){}else
unittest
{
    // POSIX zlib CRC32. Poly = 0x04C11DB7, init = uint.max, Reflect, xorout = uint.max
    // Ref: TODO:
    import std.zlib : crc32;

    CRC!(uint, 0x04C11DB7, uint.max, Yes.refin, Yes.refout, uint.max) my_crc;
    
    static assert(isDigest!(typeof(my_crc)));

    immutable ubyte[8] data = [1, 2, 3, 4, 5, 6, 7, 8];

    my_crc.put(data);

    assert(my_crc.get() == crc32(0, data));
}

version(D_BetterC){}else
unittest
{
    import std.digest.crc : CRC32;
    CRC32 std_crc;
    CRC!(uint, 0x04C11DB7, uint.max, Yes.refin, Yes.refout, uint.max) my_crc;

    static assert(isDigest!(typeof(my_crc)));

    immutable ubyte[6] data = [1, 2, 3, 4, 5, 6];

    std_crc.put(data);
    my_crc.put(data);

    assert(std_crc.peek() == my_crc.peek());
    assert(std_crc.finish() == my_crc.finish());
    assert(my_crc.reg == uint.max);
    assert(std_crc.peek() == my_crc.peek());
    assert(my_crc.reg == uint.max);
}

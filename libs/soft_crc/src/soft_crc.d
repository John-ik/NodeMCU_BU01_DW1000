version(D_BetterC){

CRC_T crc_slow (CRC_T, CRC_T polinom) (CRC_T init, CRC_T* p_data, size_t size){
    CRC_T crc = init;
    while(size--){
        crc ^= *p_data++;
        for(size_t i = 0; i < CRC_T.sizeof*8; i++)
            crc = cast(CRC_T) (crc & (1 << CRC_T.sizeof*8 - 1) ? (crc << 1) ^ polinom : (crc << 1));
    }
    return crc;
}

extern(C) __gshared{

ubyte crc8_slow (ubyte init, ubyte* p_data, const size_t size){
    return crc_slow!(ubyte, 0x31)(init, p_data, size);
}

}

import core.stdc.stdio;

extern(C)
void main (){
    static foreach(u; __traits(getUnitTests, __traits(parent, main)))
        u();

    const ubyte[2][4] datas = [
        [0x6E, 0xA8],
        [0x6A, 0x86],
        [0x6E, 0xA0],
        [0x6B, 0xC2]
    ];
    for(size_t i = 0; i < datas.length; i++)
    {
        printf("0x%X\n", crc8_slow(0, cast(ubyte*) datas[i].ptr, 2));
    }
}


unittest
{
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
        assert(crc8_slow(0, datas[i].data.ptr, 2) == datas[i].check);
    }
}
}
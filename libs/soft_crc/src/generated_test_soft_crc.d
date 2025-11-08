import soft_crc;
import std.typecons : Flag;
immutable str = cast(ubyte[9]) "123456789";

@CRC_unit("CRC-8/AUTOSAR")
unittest
{
    auto t = crc!(ubyte, 0x2f, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0xff, str.ptr, str.length);
    t = t ^ 0xff;
    assert(t == 0xdf);
}
@CRC_unit("CRC-8/BLUETOOTH")
unittest
{
    auto t = crc!(ubyte, 0xa7, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0x00, str.ptr, str.length);
    t = t ^ 0x00;
    assert(t == 0x26);
}
@CRC_unit("CRC-8/CDMA2000")
unittest
{
    auto t = crc!(ubyte, 0x9b, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0xff, str.ptr, str.length);
    t = t ^ 0x00;
    assert(t == 0xda);
}
@CRC_unit("CRC-8/DARC")
unittest
{
    auto t = crc!(ubyte, 0x39, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0x00, str.ptr, str.length);
    t = t ^ 0x00;
    assert(t == 0x15);
}
@CRC_unit("CRC-8/DVB-S2")
unittest
{
    auto t = crc!(ubyte, 0xd5, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0x00, str.ptr, str.length);
    t = t ^ 0x00;
    assert(t == 0xbc);
}
@CRC_unit("CRC-8/GSM-A")
unittest
{
    auto t = crc!(ubyte, 0x1d, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0x00, str.ptr, str.length);
    t = t ^ 0x00;
    assert(t == 0x37);
}
@CRC_unit("CRC-8/GSM-B")
unittest
{
    auto t = crc!(ubyte, 0x49, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0x00, str.ptr, str.length);
    t = t ^ 0xff;
    assert(t == 0x94);
}
@CRC_unit("CRC-8/HITAG")
unittest
{
    auto t = crc!(ubyte, 0x1d, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0xff, str.ptr, str.length);
    t = t ^ 0x00;
    assert(t == 0xb4);
}
@CRC_unit("CRC-8/I-432-1")
unittest
{
    auto t = crc!(ubyte, 0x07, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0x00, str.ptr, str.length);
    t = t ^ 0x55;
    assert(t == 0xa1);
}
@CRC_unit("CRC-8/I-CODE")
unittest
{
    auto t = crc!(ubyte, 0x1d, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0xfd, str.ptr, str.length);
    t = t ^ 0x00;
    assert(t == 0x7e);
}
@CRC_unit("CRC-8/LTE")
unittest
{
    auto t = crc!(ubyte, 0x9b, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0x00, str.ptr, str.length);
    t = t ^ 0x00;
    assert(t == 0xea);
}
@CRC_unit("CRC-8/MAXIM-DOW")
unittest
{
    auto t = crc!(ubyte, 0x31, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0x00, str.ptr, str.length);
    t = t ^ 0x00;
    assert(t == 0xa1);
}
@CRC_unit("CRC-8/MIFARE-MAD")
unittest
{
    auto t = crc!(ubyte, 0x1d, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0xc7, str.ptr, str.length);
    t = t ^ 0x00;
    assert(t == 0x99);
}
@CRC_unit("CRC-8/NRSC-5")
unittest
{
    auto t = crc!(ubyte, 0x31, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0xff, str.ptr, str.length);
    t = t ^ 0x00;
    assert(t == 0xf7);
}
@CRC_unit("CRC-8/OPENSAFETY")
unittest
{
    auto t = crc!(ubyte, 0x2f, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0x00, str.ptr, str.length);
    t = t ^ 0x00;
    assert(t == 0x3e);
}
@CRC_unit("CRC-8/ROHC")
unittest
{
    auto t = crc!(ubyte, 0x07, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0xff, str.ptr, str.length);
    t = t ^ 0x00;
    assert(t == 0xd0);
}
@CRC_unit("CRC-8/SAE-J1850")
unittest
{
    auto t = crc!(ubyte, 0x1d, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0xff, str.ptr, str.length);
    t = t ^ 0xff;
    assert(t == 0x4b);
}
@CRC_unit("CRC-8/SMBUS")
unittest
{
    auto t = crc!(ubyte, 0x07, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0x00, str.ptr, str.length);
    t = t ^ 0x00;
    assert(t == 0xf4);
}
@CRC_unit("CRC-8/TECH-3250")
unittest
{
    auto t = crc!(ubyte, 0x1d, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0xff, str.ptr, str.length);
    t = t ^ 0x00;
    assert(t == 0x97);
}
@CRC_unit("CRC-8/WCDMA")
unittest
{
    auto t = crc!(ubyte, 0x9b, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0x00, str.ptr, str.length);
    t = t ^ 0x00;
    assert(t == 0x25);
}
@CRC_unit("CRC-16/ARC")
unittest
{
    auto t = crc!(ushort, 0x8005, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0x0000, str.ptr, str.length);
    t = t ^ 0x0000;
    assert(t == 0xbb3d);
}
@CRC_unit("CRC-16/CDMA2000")
unittest
{
    auto t = crc!(ushort, 0xc867, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0xffff, str.ptr, str.length);
    t = t ^ 0x0000;
    assert(t == 0x4c06);
}
@CRC_unit("CRC-16/CMS")
unittest
{
    auto t = crc!(ushort, 0x8005, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0xffff, str.ptr, str.length);
    t = t ^ 0x0000;
    assert(t == 0xaee7);
}
@CRC_unit("CRC-16/DDS-110")
unittest
{
    auto t = crc!(ushort, 0x8005, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0x800d, str.ptr, str.length);
    t = t ^ 0x0000;
    assert(t == 0x9ecf);
}
@CRC_unit("CRC-16/DECT-R")
unittest
{
    auto t = crc!(ushort, 0x0589, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0x0000, str.ptr, str.length);
    t = t ^ 0x0001;
    assert(t == 0x007e);
}
@CRC_unit("CRC-16/DECT-X")
unittest
{
    auto t = crc!(ushort, 0x0589, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0x0000, str.ptr, str.length);
    t = t ^ 0x0000;
    assert(t == 0x007f);
}
@CRC_unit("CRC-16/DNP")
unittest
{
    auto t = crc!(ushort, 0x3d65, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0x0000, str.ptr, str.length);
    t = t ^ 0xffff;
    assert(t == 0xea82);
}
@CRC_unit("CRC-16/EN-13757")
unittest
{
    auto t = crc!(ushort, 0x3d65, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0x0000, str.ptr, str.length);
    t = t ^ 0xffff;
    assert(t == 0xc2b7);
}
@CRC_unit("CRC-16/GENIBUS")
unittest
{
    auto t = crc!(ushort, 0x1021, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0xffff, str.ptr, str.length);
    t = t ^ 0xffff;
    assert(t == 0xd64e);
}
@CRC_unit("CRC-16/GSM")
unittest
{
    auto t = crc!(ushort, 0x1021, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0x0000, str.ptr, str.length);
    t = t ^ 0xffff;
    assert(t == 0xce3c);
}
@CRC_unit("CRC-16/IBM-3740")
unittest
{
    auto t = crc!(ushort, 0x1021, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0xffff, str.ptr, str.length);
    t = t ^ 0x0000;
    assert(t == 0x29b1);
}
@CRC_unit("CRC-16/IBM-SDLC")
unittest
{
    auto t = crc!(ushort, 0x1021, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0xffff, str.ptr, str.length);
    t = t ^ 0xffff;
    assert(t == 0x906e);
}
@CRC_unit("CRC-16/ISO-IEC-14443-3-A")
unittest
{
    auto t = crc!(ushort, 0x1021, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0xc6c6, str.ptr, str.length);
    t = t ^ 0x0000;
    assert(t == 0xbf05);
}
@CRC_unit("CRC-16/KERMIT")
unittest
{
    auto t = crc!(ushort, 0x1021, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0x0000, str.ptr, str.length);
    t = t ^ 0x0000;
    assert(t == 0x2189);
}
@CRC_unit("CRC-16/LJ1200")
unittest
{
    auto t = crc!(ushort, 0x6f63, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0x0000, str.ptr, str.length);
    t = t ^ 0x0000;
    assert(t == 0xbdf4);
}
@CRC_unit("CRC-16/M17")
unittest
{
    auto t = crc!(ushort, 0x5935, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0xffff, str.ptr, str.length);
    t = t ^ 0x0000;
    assert(t == 0x772b);
}
@CRC_unit("CRC-16/MAXIM-DOW")
unittest
{
    auto t = crc!(ushort, 0x8005, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0x0000, str.ptr, str.length);
    t = t ^ 0xffff;
    assert(t == 0x44c2);
}
@CRC_unit("CRC-16/MCRF4XX")
unittest
{
    auto t = crc!(ushort, 0x1021, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0xffff, str.ptr, str.length);
    t = t ^ 0x0000;
    assert(t == 0x6f91);
}
@CRC_unit("CRC-16/MODBUS")
unittest
{
    auto t = crc!(ushort, 0x8005, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0xffff, str.ptr, str.length);
    t = t ^ 0x0000;
    assert(t == 0x4b37);
}
@CRC_unit("CRC-16/NRSC-5")
unittest
{
    auto t = crc!(ushort, 0x080b, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0xffff, str.ptr, str.length);
    t = t ^ 0x0000;
    assert(t == 0xa066);
}
@CRC_unit("CRC-16/OPENSAFETY-A")
unittest
{
    auto t = crc!(ushort, 0x5935, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0x0000, str.ptr, str.length);
    t = t ^ 0x0000;
    assert(t == 0x5d38);
}
@CRC_unit("CRC-16/OPENSAFETY-B")
unittest
{
    auto t = crc!(ushort, 0x755b, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0x0000, str.ptr, str.length);
    t = t ^ 0x0000;
    assert(t == 0x20fe);
}
@CRC_unit("CRC-16/PROFIBUS")
unittest
{
    auto t = crc!(ushort, 0x1dcf, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0xffff, str.ptr, str.length);
    t = t ^ 0xffff;
    assert(t == 0xa819);
}
@CRC_unit("CRC-16/RIELLO")
unittest
{
    auto t = crc!(ushort, 0x1021, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0xb2aa, str.ptr, str.length);
    t = t ^ 0x0000;
    assert(t == 0x63d0);
}
@CRC_unit("CRC-16/SPI-FUJITSU")
unittest
{
    auto t = crc!(ushort, 0x1021, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0x1d0f, str.ptr, str.length);
    t = t ^ 0x0000;
    assert(t == 0xe5cc);
}
@CRC_unit("CRC-16/T10-DIF")
unittest
{
    auto t = crc!(ushort, 0x8bb7, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0x0000, str.ptr, str.length);
    t = t ^ 0x0000;
    assert(t == 0xd0db);
}
@CRC_unit("CRC-16/TELEDISK")
unittest
{
    auto t = crc!(ushort, 0xa097, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0x0000, str.ptr, str.length);
    t = t ^ 0x0000;
    assert(t == 0x0fb3);
}
@CRC_unit("CRC-16/TMS37157")
unittest
{
    auto t = crc!(ushort, 0x1021, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0x89ec, str.ptr, str.length);
    t = t ^ 0x0000;
    assert(t == 0x26b1);
}
@CRC_unit("CRC-16/UMTS")
unittest
{
    auto t = crc!(ushort, 0x8005, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0x0000, str.ptr, str.length);
    t = t ^ 0x0000;
    assert(t == 0xfee8);
}
@CRC_unit("CRC-16/USB")
unittest
{
    auto t = crc!(ushort, 0x8005, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0xffff, str.ptr, str.length);
    t = t ^ 0xffff;
    assert(t == 0xb4c8);
}
@CRC_unit("CRC-16/XMODEM")
unittest
{
    auto t = crc!(ushort, 0x1021, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0x0000, str.ptr, str.length);
    t = t ^ 0x0000;
    assert(t == 0x31c3);
}
@CRC_unit("CRC-32/AIXM")
unittest
{
    auto t = crc!(uint, 0x814141ab, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0x00000000, str.ptr, str.length);
    t = t ^ 0x00000000;
    assert(t == 0x3010bf7f);
}
@CRC_unit("CRC-32/AUTOSAR")
unittest
{
    auto t = crc!(uint, 0xf4acfb13, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0xffffffff, str.ptr, str.length);
    t = t ^ 0xffffffff;
    assert(t == 0x1697d06a);
}
@CRC_unit("CRC-32/BASE91-D")
unittest
{
    auto t = crc!(uint, 0xa833982b, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0xffffffff, str.ptr, str.length);
    t = t ^ 0xffffffff;
    assert(t == 0x87315576);
}
@CRC_unit("CRC-32/BZIP2")
unittest
{
    auto t = crc!(uint, 0x04c11db7, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0xffffffff, str.ptr, str.length);
    t = t ^ 0xffffffff;
    assert(t == 0xfc891918);
}
@CRC_unit("CRC-32/CD-ROM-EDC")
unittest
{
    auto t = crc!(uint, 0x8001801b, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0x00000000, str.ptr, str.length);
    t = t ^ 0x00000000;
    assert(t == 0x6ec2edc4);
}
@CRC_unit("CRC-32/CKSUM")
unittest
{
    auto t = crc!(uint, 0x04c11db7, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0x00000000, str.ptr, str.length);
    t = t ^ 0xffffffff;
    assert(t == 0x765e7680);
}
@CRC_unit("CRC-32/ISCSI")
unittest
{
    auto t = crc!(uint, 0x1edc6f41, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0xffffffff, str.ptr, str.length);
    t = t ^ 0xffffffff;
    assert(t == 0xe3069283);
}
@CRC_unit("CRC-32/ISO-HDLC")
unittest
{
    auto t = crc!(uint, 0x04c11db7, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0xffffffff, str.ptr, str.length);
    t = t ^ 0xffffffff;
    assert(t == 0xcbf43926);
}
@CRC_unit("CRC-32/JAMCRC")
unittest
{
    auto t = crc!(uint, 0x04c11db7, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0xffffffff, str.ptr, str.length);
    t = t ^ 0x00000000;
    assert(t == 0x340bc6d9);
}
@CRC_unit("CRC-32/MEF")
unittest
{
    auto t = crc!(uint, 0x741b8cd7, cast(Flag!"Refin") true, cast(Flag!"Refout") true)(0xffffffff, str.ptr, str.length);
    t = t ^ 0x00000000;
    assert(t == 0xd2c22f51);
}
@CRC_unit("CRC-32/MPEG-2")
unittest
{
    auto t = crc!(uint, 0x04c11db7, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0xffffffff, str.ptr, str.length);
    t = t ^ 0x00000000;
    assert(t == 0x0376e6e7);
}
@CRC_unit("CRC-32/XFER")
unittest
{
    auto t = crc!(uint, 0x000000af, cast(Flag!"Refin") false, cast(Flag!"Refout") false)(0x00000000, str.ptr, str.length);
    t = t ^ 0x00000000;
    assert(t == 0xbd0be338);
}

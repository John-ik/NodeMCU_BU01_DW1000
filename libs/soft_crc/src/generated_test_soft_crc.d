// auto-generated from https://reveng.sourceforge.io/crc-catalogue/all.htm
import soft_crc;
import std.typecons : Flag, Yes, No;
immutable str = cast(ubyte[9]) "123456789"; /// check byte string

@CRC_unit("CRC-8/AUTOSAR")
unittest
{
    CRC!(ubyte, 0x2f, 0xff, cast(Flag!"refin") false, cast(Flag!"refout") false, 0xff) crc;
    crc.put(str);
    assert(crc.get() == 0xdf);
}
@CRC_unit("CRC-8/BLUETOOTH")
unittest
{
    CRC!(ubyte, 0xa7, 0x00, cast(Flag!"refin") true, cast(Flag!"refout") true, 0x00) crc;
    crc.put(str);
    assert(crc.get() == 0x26);
}
@CRC_unit("CRC-8/CDMA2000")
unittest
{
    CRC!(ubyte, 0x9b, 0xff, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x00) crc;
    crc.put(str);
    assert(crc.get() == 0xda);
}
@CRC_unit("CRC-8/DARC")
unittest
{
    CRC!(ubyte, 0x39, 0x00, cast(Flag!"refin") true, cast(Flag!"refout") true, 0x00) crc;
    crc.put(str);
    assert(crc.get() == 0x15);
}
@CRC_unit("CRC-8/DVB-S2")
unittest
{
    CRC!(ubyte, 0xd5, 0x00, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x00) crc;
    crc.put(str);
    assert(crc.get() == 0xbc);
}
@CRC_unit("CRC-8/GSM-A")
unittest
{
    CRC!(ubyte, 0x1d, 0x00, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x00) crc;
    crc.put(str);
    assert(crc.get() == 0x37);
}
@CRC_unit("CRC-8/GSM-B")
unittest
{
    CRC!(ubyte, 0x49, 0x00, cast(Flag!"refin") false, cast(Flag!"refout") false, 0xff) crc;
    crc.put(str);
    assert(crc.get() == 0x94);
}
@CRC_unit("CRC-8/HITAG")
unittest
{
    CRC!(ubyte, 0x1d, 0xff, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x00) crc;
    crc.put(str);
    assert(crc.get() == 0xb4);
}
@CRC_unit("CRC-8/I-432-1")
unittest
{
    CRC!(ubyte, 0x07, 0x00, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x55) crc;
    crc.put(str);
    assert(crc.get() == 0xa1);
}
@CRC_unit("CRC-8/I-CODE")
unittest
{
    CRC!(ubyte, 0x1d, 0xfd, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x00) crc;
    crc.put(str);
    assert(crc.get() == 0x7e);
}
@CRC_unit("CRC-8/LTE")
unittest
{
    CRC!(ubyte, 0x9b, 0x00, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x00) crc;
    crc.put(str);
    assert(crc.get() == 0xea);
}
@CRC_unit("CRC-8/MAXIM-DOW")
unittest
{
    CRC!(ubyte, 0x31, 0x00, cast(Flag!"refin") true, cast(Flag!"refout") true, 0x00) crc;
    crc.put(str);
    assert(crc.get() == 0xa1);
}
@CRC_unit("CRC-8/MIFARE-MAD")
unittest
{
    CRC!(ubyte, 0x1d, 0xc7, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x00) crc;
    crc.put(str);
    assert(crc.get() == 0x99);
}
@CRC_unit("CRC-8/NRSC-5")
unittest
{
    CRC!(ubyte, 0x31, 0xff, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x00) crc;
    crc.put(str);
    assert(crc.get() == 0xf7);
}
@CRC_unit("CRC-8/OPENSAFETY")
unittest
{
    CRC!(ubyte, 0x2f, 0x00, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x00) crc;
    crc.put(str);
    assert(crc.get() == 0x3e);
}
@CRC_unit("CRC-8/ROHC")
unittest
{
    CRC!(ubyte, 0x07, 0xff, cast(Flag!"refin") true, cast(Flag!"refout") true, 0x00) crc;
    crc.put(str);
    assert(crc.get() == 0xd0);
}
@CRC_unit("CRC-8/SAE-J1850")
unittest
{
    CRC!(ubyte, 0x1d, 0xff, cast(Flag!"refin") false, cast(Flag!"refout") false, 0xff) crc;
    crc.put(str);
    assert(crc.get() == 0x4b);
}
@CRC_unit("CRC-8/SMBUS")
unittest
{
    CRC!(ubyte, 0x07, 0x00, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x00) crc;
    crc.put(str);
    assert(crc.get() == 0xf4);
}
@CRC_unit("CRC-8/TECH-3250")
unittest
{
    CRC!(ubyte, 0x1d, 0xff, cast(Flag!"refin") true, cast(Flag!"refout") true, 0x00) crc;
    crc.put(str);
    assert(crc.get() == 0x97);
}
@CRC_unit("CRC-8/WCDMA")
unittest
{
    CRC!(ubyte, 0x9b, 0x00, cast(Flag!"refin") true, cast(Flag!"refout") true, 0x00) crc;
    crc.put(str);
    assert(crc.get() == 0x25);
}
@CRC_unit("CRC-16/ARC")
unittest
{
    CRC!(ushort, 0x8005, 0x0000, cast(Flag!"refin") true, cast(Flag!"refout") true, 0x0000) crc;
    crc.put(str);
    assert(crc.get() == 0xbb3d);
}
@CRC_unit("CRC-16/CDMA2000")
unittest
{
    CRC!(ushort, 0xc867, 0xffff, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x0000) crc;
    crc.put(str);
    assert(crc.get() == 0x4c06);
}
@CRC_unit("CRC-16/CMS")
unittest
{
    CRC!(ushort, 0x8005, 0xffff, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x0000) crc;
    crc.put(str);
    assert(crc.get() == 0xaee7);
}
@CRC_unit("CRC-16/DDS-110")
unittest
{
    CRC!(ushort, 0x8005, 0x800d, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x0000) crc;
    crc.put(str);
    assert(crc.get() == 0x9ecf);
}
@CRC_unit("CRC-16/DECT-R")
unittest
{
    CRC!(ushort, 0x0589, 0x0000, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x0001) crc;
    crc.put(str);
    assert(crc.get() == 0x007e);
}
@CRC_unit("CRC-16/DECT-X")
unittest
{
    CRC!(ushort, 0x0589, 0x0000, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x0000) crc;
    crc.put(str);
    assert(crc.get() == 0x007f);
}
@CRC_unit("CRC-16/DNP")
unittest
{
    CRC!(ushort, 0x3d65, 0x0000, cast(Flag!"refin") true, cast(Flag!"refout") true, 0xffff) crc;
    crc.put(str);
    assert(crc.get() == 0xea82);
}
@CRC_unit("CRC-16/EN-13757")
unittest
{
    CRC!(ushort, 0x3d65, 0x0000, cast(Flag!"refin") false, cast(Flag!"refout") false, 0xffff) crc;
    crc.put(str);
    assert(crc.get() == 0xc2b7);
}
@CRC_unit("CRC-16/GENIBUS")
unittest
{
    CRC!(ushort, 0x1021, 0xffff, cast(Flag!"refin") false, cast(Flag!"refout") false, 0xffff) crc;
    crc.put(str);
    assert(crc.get() == 0xd64e);
}
@CRC_unit("CRC-16/GSM")
unittest
{
    CRC!(ushort, 0x1021, 0x0000, cast(Flag!"refin") false, cast(Flag!"refout") false, 0xffff) crc;
    crc.put(str);
    assert(crc.get() == 0xce3c);
}
@CRC_unit("CRC-16/IBM-3740")
unittest
{
    CRC!(ushort, 0x1021, 0xffff, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x0000) crc;
    crc.put(str);
    assert(crc.get() == 0x29b1);
}
@CRC_unit("CRC-16/IBM-SDLC")
unittest
{
    CRC!(ushort, 0x1021, 0xffff, cast(Flag!"refin") true, cast(Flag!"refout") true, 0xffff) crc;
    crc.put(str);
    assert(crc.get() == 0x906e);
}
@CRC_unit("CRC-16/ISO-IEC-14443-3-A")
unittest
{
    CRC!(ushort, 0x1021, 0xc6c6, cast(Flag!"refin") true, cast(Flag!"refout") true, 0x0000) crc;
    crc.put(str);
    assert(crc.get() == 0xbf05);
}
@CRC_unit("CRC-16/KERMIT")
unittest
{
    CRC!(ushort, 0x1021, 0x0000, cast(Flag!"refin") true, cast(Flag!"refout") true, 0x0000) crc;
    crc.put(str);
    assert(crc.get() == 0x2189);
}
@CRC_unit("CRC-16/LJ1200")
unittest
{
    CRC!(ushort, 0x6f63, 0x0000, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x0000) crc;
    crc.put(str);
    assert(crc.get() == 0xbdf4);
}
@CRC_unit("CRC-16/M17")
unittest
{
    CRC!(ushort, 0x5935, 0xffff, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x0000) crc;
    crc.put(str);
    assert(crc.get() == 0x772b);
}
@CRC_unit("CRC-16/MAXIM-DOW")
unittest
{
    CRC!(ushort, 0x8005, 0x0000, cast(Flag!"refin") true, cast(Flag!"refout") true, 0xffff) crc;
    crc.put(str);
    assert(crc.get() == 0x44c2);
}
@CRC_unit("CRC-16/MCRF4XX")
unittest
{
    CRC!(ushort, 0x1021, 0xffff, cast(Flag!"refin") true, cast(Flag!"refout") true, 0x0000) crc;
    crc.put(str);
    assert(crc.get() == 0x6f91);
}
@CRC_unit("CRC-16/MODBUS")
unittest
{
    CRC!(ushort, 0x8005, 0xffff, cast(Flag!"refin") true, cast(Flag!"refout") true, 0x0000) crc;
    crc.put(str);
    assert(crc.get() == 0x4b37);
}
@CRC_unit("CRC-16/NRSC-5")
unittest
{
    CRC!(ushort, 0x080b, 0xffff, cast(Flag!"refin") true, cast(Flag!"refout") true, 0x0000) crc;
    crc.put(str);
    assert(crc.get() == 0xa066);
}
@CRC_unit("CRC-16/OPENSAFETY-A")
unittest
{
    CRC!(ushort, 0x5935, 0x0000, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x0000) crc;
    crc.put(str);
    assert(crc.get() == 0x5d38);
}
@CRC_unit("CRC-16/OPENSAFETY-B")
unittest
{
    CRC!(ushort, 0x755b, 0x0000, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x0000) crc;
    crc.put(str);
    assert(crc.get() == 0x20fe);
}
@CRC_unit("CRC-16/PROFIBUS")
unittest
{
    CRC!(ushort, 0x1dcf, 0xffff, cast(Flag!"refin") false, cast(Flag!"refout") false, 0xffff) crc;
    crc.put(str);
    assert(crc.get() == 0xa819);
}
@CRC_unit("CRC-16/RIELLO")
unittest
{
    CRC!(ushort, 0x1021, 0xb2aa, cast(Flag!"refin") true, cast(Flag!"refout") true, 0x0000) crc;
    crc.put(str);
    assert(crc.get() == 0x63d0);
}
@CRC_unit("CRC-16/SPI-FUJITSU")
unittest
{
    CRC!(ushort, 0x1021, 0x1d0f, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x0000) crc;
    crc.put(str);
    assert(crc.get() == 0xe5cc);
}
@CRC_unit("CRC-16/T10-DIF")
unittest
{
    CRC!(ushort, 0x8bb7, 0x0000, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x0000) crc;
    crc.put(str);
    assert(crc.get() == 0xd0db);
}
@CRC_unit("CRC-16/TELEDISK")
unittest
{
    CRC!(ushort, 0xa097, 0x0000, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x0000) crc;
    crc.put(str);
    assert(crc.get() == 0x0fb3);
}
@CRC_unit("CRC-16/TMS37157")
unittest
{
    CRC!(ushort, 0x1021, 0x89ec, cast(Flag!"refin") true, cast(Flag!"refout") true, 0x0000) crc;
    crc.put(str);
    assert(crc.get() == 0x26b1);
}
@CRC_unit("CRC-16/UMTS")
unittest
{
    CRC!(ushort, 0x8005, 0x0000, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x0000) crc;
    crc.put(str);
    assert(crc.get() == 0xfee8);
}
@CRC_unit("CRC-16/USB")
unittest
{
    CRC!(ushort, 0x8005, 0xffff, cast(Flag!"refin") true, cast(Flag!"refout") true, 0xffff) crc;
    crc.put(str);
    assert(crc.get() == 0xb4c8);
}
@CRC_unit("CRC-16/XMODEM")
unittest
{
    CRC!(ushort, 0x1021, 0x0000, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x0000) crc;
    crc.put(str);
    assert(crc.get() == 0x31c3);
}
@CRC_unit("CRC-32/AIXM")
unittest
{
    CRC!(uint, 0x814141ab, 0x00000000, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x00000000) crc;
    crc.put(str);
    assert(crc.get() == 0x3010bf7f);
}
@CRC_unit("CRC-32/AUTOSAR")
unittest
{
    CRC!(uint, 0xf4acfb13, 0xffffffff, cast(Flag!"refin") true, cast(Flag!"refout") true, 0xffffffff) crc;
    crc.put(str);
    assert(crc.get() == 0x1697d06a);
}
@CRC_unit("CRC-32/BASE91-D")
unittest
{
    CRC!(uint, 0xa833982b, 0xffffffff, cast(Flag!"refin") true, cast(Flag!"refout") true, 0xffffffff) crc;
    crc.put(str);
    assert(crc.get() == 0x87315576);
}
@CRC_unit("CRC-32/BZIP2")
unittest
{
    CRC!(uint, 0x04c11db7, 0xffffffff, cast(Flag!"refin") false, cast(Flag!"refout") false, 0xffffffff) crc;
    crc.put(str);
    assert(crc.get() == 0xfc891918);
}
@CRC_unit("CRC-32/CD-ROM-EDC")
unittest
{
    CRC!(uint, 0x8001801b, 0x00000000, cast(Flag!"refin") true, cast(Flag!"refout") true, 0x00000000) crc;
    crc.put(str);
    assert(crc.get() == 0x6ec2edc4);
}
@CRC_unit("CRC-32/CKSUM")
unittest
{
    CRC!(uint, 0x04c11db7, 0x00000000, cast(Flag!"refin") false, cast(Flag!"refout") false, 0xffffffff) crc;
    crc.put(str);
    assert(crc.get() == 0x765e7680);
}
@CRC_unit("CRC-32/ISCSI")
unittest
{
    CRC!(uint, 0x1edc6f41, 0xffffffff, cast(Flag!"refin") true, cast(Flag!"refout") true, 0xffffffff) crc;
    crc.put(str);
    assert(crc.get() == 0xe3069283);
}
@CRC_unit("CRC-32/ISO-HDLC")
unittest
{
    CRC!(uint, 0x04c11db7, 0xffffffff, cast(Flag!"refin") true, cast(Flag!"refout") true, 0xffffffff) crc;
    crc.put(str);
    assert(crc.get() == 0xcbf43926);
}
@CRC_unit("CRC-32/JAMCRC")
unittest
{
    CRC!(uint, 0x04c11db7, 0xffffffff, cast(Flag!"refin") true, cast(Flag!"refout") true, 0x00000000) crc;
    crc.put(str);
    assert(crc.get() == 0x340bc6d9);
}
@CRC_unit("CRC-32/MEF")
unittest
{
    CRC!(uint, 0x741b8cd7, 0xffffffff, cast(Flag!"refin") true, cast(Flag!"refout") true, 0x00000000) crc;
    crc.put(str);
    assert(crc.get() == 0xd2c22f51);
}
@CRC_unit("CRC-32/MPEG-2")
unittest
{
    CRC!(uint, 0x04c11db7, 0xffffffff, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x00000000) crc;
    crc.put(str);
    assert(crc.get() == 0x0376e6e7);
}
@CRC_unit("CRC-32/XFER")
unittest
{
    CRC!(uint, 0x000000af, 0x00000000, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x00000000) crc;
    crc.put(str);
    assert(crc.get() == 0xbd0be338);
}
@CRC_unit("CRC-64/ECMA-182")
unittest
{
    CRC!(ulong, 0x42f0e1eba9ea3693, 0x0000000000000000, cast(Flag!"refin") false, cast(Flag!"refout") false, 0x0000000000000000) crc;
    crc.put(str);
    assert(crc.get() == 0x6c40df5f0b497347);
}
@CRC_unit("CRC-64/GO-ISO")
unittest
{
    CRC!(ulong, 0x000000000000001b, 0xffffffffffffffff, cast(Flag!"refin") true, cast(Flag!"refout") true, 0xffffffffffffffff) crc;
    crc.put(str);
    assert(crc.get() == 0xb90956c775a41001);
}
@CRC_unit("CRC-64/MS")
unittest
{
    CRC!(ulong, 0x259c84cba6426349, 0xffffffffffffffff, cast(Flag!"refin") true, cast(Flag!"refout") true, 0x0000000000000000) crc;
    crc.put(str);
    assert(crc.get() == 0x75d4b74f024eceea);
}
@CRC_unit("CRC-64/NVME")
unittest
{
    CRC!(ulong, 0xad93d23594c93659, 0xffffffffffffffff, cast(Flag!"refin") true, cast(Flag!"refout") true, 0xffffffffffffffff) crc;
    crc.put(str);
    assert(crc.get() == 0xae8b14860a799888);
}
@CRC_unit("CRC-64/REDIS")
unittest
{
    CRC!(ulong, 0xad93d23594c935a9, 0x0000000000000000, cast(Flag!"refin") true, cast(Flag!"refout") true, 0x0000000000000000) crc;
    crc.put(str);
    assert(crc.get() == 0xe9c6d914c4b8d9ca);
}
@CRC_unit("CRC-64/WE")
unittest
{
    CRC!(ulong, 0x42f0e1eba9ea3693, 0xffffffffffffffff, cast(Flag!"refin") false, cast(Flag!"refout") false, 0xffffffffffffffff) crc;
    crc.put(str);
    assert(crc.get() == 0x62ec59e3f1a4f00a);
}
@CRC_unit("CRC-64/XZ")
unittest
{
    CRC!(ulong, 0x42f0e1eba9ea3693, 0xffffffffffffffff, cast(Flag!"refin") true, cast(Flag!"refout") true, 0xffffffffffffffff) crc;
    crc.put(str);
    assert(crc.get() == 0x995dc9bbdf1939fa);
}

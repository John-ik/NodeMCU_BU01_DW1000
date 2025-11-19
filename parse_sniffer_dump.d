module parse_trace_msg;

version(linux)
shared static this(){
}

import messages;
import io = std.stdio;
import std.digest : fromHexString;
import core.thread : Thread;
import std.datetime;
import std.conv : bitCast;
import std.string : fromStringz;
import std.mmfile;

real fromUwbTime(ulong time) => time * 15.65 * 10.0^^-12;

ptrdiff_t parseRecord(ubyte[] data){
    import std.algorithm : countUntil;
    ptrdiff_t found = data.countUntil([0x41, 0x88]);

    real time;
    if (found >= 5){
        ulong t = *cast(ulong*) &data[found-5];
        t &= 0x00_00_00_ff_ff_ff_ff_ff; // только 5 байт значат. Little-Endian
        time = t.fromUwbTime();
    }

    char[512] buf;
    showMsg(buf.ptr, buf.length, data[found..$].ptr);
    auto s = buf.fromStringz();
    io.writefln("%f: %s", time, s);

    return found + msgGetLen(cast(MSG_Types) data[found+9]);
}

int main(string[] args){
    if (args.length != 2){
        return 1;
    }

    scope file = new MmFile(args[1]);
    auto data = cast(ubyte[]) file[];

    while ( true )
    {
        ptrdiff_t i = parseRecord(data);
        if (i == -1 || i >= data.length)
            return 0;
        data = data[i..$];
    }

    return 0;
}

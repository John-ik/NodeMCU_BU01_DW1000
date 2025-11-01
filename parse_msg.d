module parse_trace_msg;

import messages;
import std;

alias io = std.stdio;

real fromUwbTime(ulong time) => time * 15.65 * 10.0^^-12;

pragma(msg, messages.uint32_t.sizeof);

void main(){
    foreach(string line; lines(io.stdin)){
        auto words = line.split;
        char[512] msg_str;
        ubyte[22] msg;
        auto header = words[0].split(":");
        auto file = header[0];
        auto num_line = header[1];
        real sys_ts= formattedRead!ulong(header[2], "0x%x")[0].fromUwbTime();
        real tx_ts = formattedRead!ulong(words[2], "0x%x")[0].fromUwbTime();
        real rx_ts = formattedRead!ulong(words[3], "0x%x")[0].fromUwbTime();
        for(size_t i = 0; i + 2 < words[4].length; i += 2){
            auto arr = words[4][i..i+2];
            msg[i/2] = parse!(ubyte)(arr, 16);
        }

        showMsg(msg_str.ptr, msg_str.length, msg.ptr);
        writeln("--------------------");
        writefln("Sys = %2.10f s\nTx = %2.10f s\nRx = %2.10f s\n%s",
            sys_ts, tx_ts, rx_ts, msg_str.fromStringz
        );
        io.stdout.flush();
    }
}

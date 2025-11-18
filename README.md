# Misc

Втроенная фукнция `__builtin_prefetch(some_addr);` генерирует `PLD` инструкцию,
которой нету в Cortex-M3, одако она все же исполняется без ошибки, как `NOP`,
однако на M ядрах в целом нет слысла от prefetch т.к. у них нету кэша.

> PLD and PLI are both hints and so act as a NOP [1].

[1]: https://developer.arm.com/documentation/ddi0337/e/Programmer-s-Model/Instruction-set

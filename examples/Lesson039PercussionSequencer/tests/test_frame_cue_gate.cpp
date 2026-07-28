#include "../FrameCueGate.h"

#include <assert.h>

int main ()
{
    FrameCueGate gate;

    assert (!gate.advance (false, 0, 0, 1600));
    assert (gate.advance (true, 0, 10, 1600));
    assert (!gate.advance (true, 0, 11, 1600));
    assert (!gate.advance (true, 0, 70, 1600));

    assert (gate.advance (true, 1, 110, 1600));
    assert (!gate.advance (true, 1, 111, 1600));
    assert (gate.advance (true, 15, 1510, 1600));
    assert (gate.advance (true, 0, 1610, 1600));

    assert (!gate.advance (false, 0, 1700, 1600));
    assert (gate.advance (true, 0, 1710, 1600));
    assert (!gate.advance (true, 0, 1711, 1600));

    FrameCueGate sparse;
    assert (sparse.advance (true, 4, 100, 1600));
    assert (!sparse.advance (true, 4, 1699, 1600));
    assert (sparse.advance (true, 4, 1700, 1600));

    FrameCueGate rollover;
    assert (rollover.advance (true, 2, 0xfffffff0UL, 1600));
    assert (!rollover.advance (true, 2, 20, 1600));
    assert (rollover.advance (true, 2, 1584, 1600));
}

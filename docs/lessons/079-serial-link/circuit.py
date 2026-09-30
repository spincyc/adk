# Serial1 sends from TX1 through 1 kΩ to its own RX1. The pull-up keeps
# RX1 high when the link is removed, so an open path cannot look like data.
bench = Bench ("Serial1 sends a byte from pin 18 through 1 kΩ to pin 19",
               columns=(1, 16))

bench.stage ("the 1 kΩ link between TX1 and RX1")
bench.wire ("18", "j6")
bench.resistor ("1 kΩ", "g6", "e6")
bench.wire ("19", "c6")

bench.stage ("the 10 kΩ pull-up on RX1")
bench.resistor ("10 kΩ", "T+7", "j7")
bench.wire ("h7", "d6")

bench.closeup (4, 8)

# Radiation Hardened Software (RHS)
Proof-of-concept RHS library inspired by [*Expecting the Unexpected - Radiation
Hardened
Software*](https://ti.arc.nasa.gov/m/pub-archive/1075h/1075%20(Mehlitz).pdf) by
Mehlitz and Penix.

## Features
### RHS Boolean
Traditional Booleans use a single 1 bit for `true` and 0 for `false`.
Additionally, conditionals recognize any value other than 0 as `true`.  This
means than any single bit error in a `false` variable will change the outcome of
a conditional using that variable.  The RHS Boolean in rhsbool.h solves this by
using a full 32 bit value, 0 for `false` and 0xFFFFFFFF for `true`.  It then
provides two functions: `rhs_is_true()` and `rhs_is_false()` which count the
number of 1 bits in the given value.  Any value with more than half of the bits
set to 1 is considered `true`, less than half is `false`, and exactly half is an
error case that should be handled by the client.

### EDAC Memory
The EDAC memory is edacmemory.h works similarly to smart pointers.  It
automatically adds Reed-Solomon error correction to any object, and verifies and
corrects the data when `operator*` or `operator->` are called.

### TMR Memory
The TMR memory in edacmemory.h is designed to provided redundancy for simple
types.  By default it keeps three copies of the type, but additional copies can
be requested using the template parameter.  It compares the three copies on any
access and restores the majority.

## License
The RHS library is released in the [BDS 3-clause license](LICENSE).  However,
it also used fec-3.0.1 which is [released by Phil
Karn](http://www.ka9q.net/code/fec/) under the [LGPL](fec-3.0.1/lesser.txt).

/* Minimal POKEY-@450 crash repro (see 7800port.md #16/#17). Deliberately
 * contains NO POKEY-related code at all -- not even an #include -- to prove
 * the crash this cart type causes in this SDK's test emulator has nothing
 * to do with what a program actually does with POKEY. The .a78 header
 * declaring the cart type (48K + POKEY@450, via CMakeLists.txt's
 * --defsym overrides) is the only thing that differs from a normal,
 * working build; this is otherwise the smallest possible llvm-mos-atari7800
 * program.
 */
int main(void) {
  for (;;) {
  }
}

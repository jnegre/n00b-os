/* Prints a message and stops the computer.
 */
void panic(const char*) __attribute__((noreturn));

/* Stops the computer.
 */
 void halt(void) __attribute__((noreturn));
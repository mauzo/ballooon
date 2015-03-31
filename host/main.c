/* main.c
 * Substitute main for building on amd64.
 */

void setup  (void);
void loop   (void);

int
main (void)
{
    setup();
    while (1) loop();
}

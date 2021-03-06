/*
 *   These are the low-level serial IO routines for the GDB stub
 *   This is platform-specific, since it deals with the serial registers directly
 */

#define RS232IN_DATA (*(unsigned *) 0xFF000004)
#define RS232IN_TAG  (*(unsigned *) 0xFF000008)
#define TSC          (*(unsigned *) 0xFF00000C)

#define BUFFERSIZE 1024
static char input_buffer[BUFFERSIZE];
static unsigned input_pending = 0, input_read = 0;
static int overrun = 0;
static int initialized = 0;

void DbgInitSerial(void)
{
	input_pending = 0;
	input_read = 0;
	overrun = 0;
	initialized = 1;
}




static void
service_rs232in(void)
{
	static unsigned char last_rs232_tag;
	unsigned char new_tag = RS232IN_TAG;
	unsigned ch;
	unsigned new_pending;

	if (new_tag == last_rs232_tag)
		return;

	if (new_tag != ((last_rs232_tag + 1) & 255))
		overrun = 1000;

	last_rs232_tag = new_tag;
	ch =  RS232IN_DATA;

	new_pending = input_pending + 1;
	new_pending &= BUFFERSIZE - 1;
	if (new_pending == input_read)
		return;
	input_buffer[input_pending] = ch;
	input_pending = new_pending;
}

int
putDebugChar(unsigned char ch)
{
	if (!initialized) { 		/* need to init device first */
		DbgInitSerial();
	}

        if (overrun) {
                overrun--;
                ch = 'O';
        }

	if (ch == '\n')
		putDebugChar('\r');

#define SER_OUTBUSY() (*(volatile unsigned *)0xFF000000 != 0)
#define SER_OUT(data) (*(volatile unsigned *)0xFF000000 = (data))

	while (SER_OUTBUSY()) {
		service_rs232in();
	}

	SER_OUT(ch);

	return 1;
}

char
getDebugChar(void)
{
	if (!initialized) { 		/* need to init device first */
		DbgInitSerial();
	}

	unsigned char ch;
	while (input_pending == input_read) {
		service_rs232in();
	}

	ch = input_buffer[input_read++];
	input_read &= BUFFERSIZE - 1;

	return ch;
}


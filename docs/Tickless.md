# Tickless Kernel Support

This chapter describes how to configure a kernel for a system which has no
available timer tick. Such systems can be made to function but will not be
able to provide particularly fair time sharing or deal with a process spinning
in user space very well.

A clock reference is required and interrupt driven I/O is recommended.


## Configuration

To configure such a system

    #define CONFIG_NO_CLOCK

Provide a function

    void sync_clock(void)
    {
	static uint8_t depth;
	irqflags_t irq = di();
	if (!depth++) {
		Read Timer
		Call timer_interrupt the correct number of times to
			advance
		Poll any non interrupt devices like serial
		depth--;
	}
	irqrestore(irq);
    }

Also provide a function

    void update_sync_clock(void)
    {
    }

This is called with interrupts off when the kernel changes its time base. If
you are using the kernel ticks as a timebase for sync_clock you can adjust
your own timing here. If you are doing your timing entirely via internal
variables this function can probably be empty.

Finally ensure your platform_idle routine calls sync_clock(). This ensures
that when idle the time progresses as expected, events happen, sleeps wake and
alarms go off.

## Functional Limitations

The timing values for process time used will be rather more inaccurate.

Uptime will be pretty meaningless for CPU load.

Task switching will only occur when a process makes a system call after
exceeding its time quantum, when it sleeps, on process exit, or when a
signal is delivered. The last one is done to ensure that actions such as
hitting ^C have a reasonable chance of working correctly even when a process
is stuck in userspace.

If you have an RTC and your RTC is fast to access you can also consider calling
sync_clock() on keyboard interrupts. That will normally allow an interactive
user to regain control of the system in all cases. If the RTC is slow (for
example a bitbanged SPI) then it is likely to be too expensive to do so.

On some systems it's relatively easy to retrofit a periodic interrupt handler.
A spare serial port can be used as a timing generator, or an interrupt driven
serial port with control lines and interrupts for them (eg a 16x50) can have
one of the control signals wired to something like a PIC generating a 10Hz
square wave, without interfering with the other use of the port.

Such an approach is preferable.

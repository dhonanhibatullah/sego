# **sego**

**sego** (pronounced /sə:ɡô/) is a Go-inspired framework for the C programming language, designed to bring lightweight concurrency and simplicity to your C projects. It’s a header-only library — just include and start coding!

Built on native thread handling using pthread (currently Linux-only), **sego** offers seamless routine management by integrating the [uthash](https://github.com/troydhanson/uthash) library. Windows support is on the roadmap.

🚧 **sego** is still under active development — stay tuned for updates!

## **A. Why You Should Try sego?**

1. ⚡ **Blazing Fast & Feather-Light**

   No garbage collector. No goroutine scheduler. Just pure speed—leaner and meaner than Go!

2. 🛠️ **C/C++ Fans, Rejoice!**

   If you love the raw power and control of C/C++, sego gives you that same freedom—with a Go twist.

## **B. Examples**

### **1. Compiling**

Since sego relies on `pthread`, remember to compile with the `-pthread` flag.

```bash
gcc -pthread SOURCE_NAME.c -o EXECUTABLE_NAME
```

### **2. Sego Routine**

Here's how to run two simple sego routines. Note that each routine function must accept a `void *` argument and return a `void *`.

```c
#include <stdio.h>
#include "sego.h"

// first sego routine function
void *routine1(void *arg)
{
    for (uint8_t i = 0; i < 20; ++i)
    {
        printf("[ R1 ] Counter: %u\n", i);
        sgMomentSleep(170L * SG_TIME_MS);
    }
}

// second sego routine function
void *routine2(void *arg)
{
    for (uint8_t i = 0; i < 10; ++i)
    {
        printf("[ R2 ] Counter: %u\n", i);
        sgMomentSleep(300L * SG_TIME_MS);
    }
}

int main()
{
    // sego handler init
    sgInit();

    // starts sego routines
    sego(routine1, NULL);
    sego(routine2, NULL);

    // waits until sego routines are completed
    sgMomentSleep(3500L * SG_TIME_MS);

    // sego handler close
    sgClose();
    return 0;
}
```

### **3. Sego Channel**

You can use **sego** channels to safely communicate between routines — just like in Go. A channel is initialized with two parameters: `itemSize` (the size of each item to send through the channel) and `bufferSize` (the capacity of the channel buffer; use 1 for an unbuffered channel).

```c
#include <stdio.h>
#include "sego.h"

// sender sego routine function
void *sender(void *arg)
{
    // casts the argument into channel
    sgChan *ch = (sgChan *)arg;

    // string to send
    char msg[32] = "Hello from sender!";

    for (uint8_t i = 0; i < 5; ++i)
    {
        // sends the message
        sgChanIn(ch, msg);

        // gives the CPU some time to sleep
        sgMomentSleep(500L * SG_TIME_MS);
    }

    // closes the channel
    sgChanDestroy(ch);
}

// receiver sego routine function
void *receiver(void *arg)
{
    // casts the argument into channel
    sgChan *ch = (sgChan *)arg;

    // buffer to receive
    char recv[32];

    for (uint8_t i = 0; i < 5; ++i)
    {
        // receives the message
        sgChanOut(ch, recv);
        printf("Received: %s\n", recv);
    }
}

int main()
{
    // sego handler init
    sgInit();

    // creates a channel to communicate between routines
    // assume we want to send a string
    sgChan *ch = sgChanMake(sizeof(char[32]), 8);

    // starts sego routines and pass the channel
    sego(sender, ch);
    sego(receiver, ch);

    // waits until sego routines are completed
    sgMomentSleep(2500L * SG_TIME_MS);

    // sego handler close
    sgClose();
    return 0;
}
```

### **4. Sego Context**

Contexts in **sego** work a bit differently from those in Go. In **sego**, a context acts more like a flag and can be used for various signaling purposes.

```c
#include <stdio.h>
#include "sego.h"

int main()
{
    // sego handler init
    sgInit();

    // creates a context
    sgContext *ctx = sgContextCreate();

    // raises after 2s
    sgContextRaiseAfter(ctx, 2L * SG_TIME_S);

    // checks after 1s
    sgMomentSleep(1L * SG_TIME_S);
    if (sgContextGetFlag(ctx) != SG_CTX_RAISED)
        printf("Still lowered after 1s\n");
    else
        printf("NANI?! how is that even possible?\n");

    // checks after 3s
    sgMomentSleep(2L * SG_TIME_S);
    if (sgContextGetFlag(ctx) != SG_CTX_RAISED)
        printf("There is no way...\n");
    else
        printf("Flag is raised\n");

    // destroys the context
    sgContextDestroy(ctx);

    // sego handler close
    sgClose();
    return 0;
}
```

### **5. Sego Select**

Just like in Go, **sego** select can be used to wait for incoming data from a channel or a signal from a context.

```c
#include <stdio.h>
#include "sego.h"

// define the argument for the routine
typedef struct
{
    sgChan *ch;
    sgContext *ctx;
} senderArgs;

// sender sego routine function
void *sender(void *arg)
{
    // casts the argument into channel and context
    sgChan *ch = ((senderArgs *)arg)->ch;
    sgContext *ctx = ((senderArgs *)arg)->ctx;

    // send messages
    char msg[32];
    for (uint8_t i = 0; i < 20; ++i)
    {
        // creates and sends the message
        snprintf(msg, 32, "Counter : %02u", i);
        sgChanIn(ch, msg);

        // gives the CPU some time to sleep
        sgMomentSleep(30L * SG_TIME_MS);
    }

    // raises the context to stop the select in main()
    sgContextRaise(ctx);

    // closes the context and channels
    sgChanDestroy(ch);
    sgContextDestroy(ctx);
}

int main()
{
    // sego handler init
    sgInit();

    // creates a channel and a context
    sgChan *ch = sgChanMake(sizeof(char[32]), 1);
    sgContext *ctx = sgContextCreate();

    // passes the channel and the context to the routine
    senderArgs args = {
        .ch = ch,
        .ctx = ctx};

    // runs the routine
    sego(sender, &args);

    // receives the messages via select
    char buf[32];
    while (1)
    {
        // selects from 1 context and 1 channel
        sgSel sel = sgSelectWithContext(1, 1, ctx, ch);

        if (sel == (sgSel)ctx)
        {
            printf("Context raised, terminating...\n");
            break;
        }
        else if (sel == (sgSel)ch)
        {
            sgChanOut(ch, buf);
            printf("Channel -> %s\n", buf);
        }
    }

    // sego handler close
    sgClose();
    return 0;
}
```

### **6. Sego Timer**

Need a time-triggered event? Let the **sego** timer handles it!

```c
#include <stdio.h>
#include "sego.h"

// defines the timer callback
void timerCallback(void *args)
{
    printf("Hello from timer!\n");
}

int main()
{
    // sego handler init
    sgInit();

    // creates the timer
    sgMomentTimer *t = sgMomentTimerCreate(
        1L * SG_TIME_MS,
        300L * SG_TIME_MS,
        0,
        timerCallback,
        NULL);

    // waits and destroys
    sgMomentSleep(2L * SG_TIME_S);
    sgMomentTimerDestroy(t);

    // sego handler close
    sgClose();
    return 0;
}
```

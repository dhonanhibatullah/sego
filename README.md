# **sego**

**sego** is a Go-inspired framework for the C programming language, designed to bring lightweight concurrency and simplicity to your C projects. It’s a header-only library — just include and start coding!

Built on native thread handling using pthread (currently Linux-only), **sego** offers seamless routine management by integrating the excellent [uthash](https://github.com/troydhanson/uthash) library under the hood. Windows support is on the roadmap.

🚧 **sego** is still under active development — stay tuned for updates!

## **A. Examples**

### **0. Compiling**

Since sego relies on `pthread`, remember to compile with the `-pthread` flag.

```bash
gcc -pthread SOURCE_NAME.c -o EXECUTABLE_NAME
```

### **1. Sego Routine**

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

### **2. Sego Channel**

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

### **3. Sego Context**

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

    // sego handler close
    sgClose();
    return 0;
}
```

## **B. To be Developed**

1. windows support
2. `select {}` mechanism
3. many other Go features
4. libraries based on **sego** for building apps

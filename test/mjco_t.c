#include <stdio.h>
#include <string.h>
#include "co.h"

#define BUF_SIZE    1024

struct dinput {
    int state;
    char data[BUF_SIZE];
};

struct doutput {
    int state;
    char data[BUF_SIZE];
};

void rfun(int *state, struct dinput *in)
{
    CoBegin(state);
    while (1) {
        strcpy(in->data, "data");
        in->data[4] = 0;
        CoReturnVoid(state);
    }
    CoEnd;
    CoFinishVoid(state);
}

void wfun(int *state, struct doutput *out, struct dinput *in)
{
    CoBegin(state);
    while (1) {
        strcpy(out->data, in->data);
        CoReturnVoid(state);
    }
    CoEnd;
    CoFinishVoid(state);
}

int main()
{
    struct dinput in;
    struct doutput out;

    in.state = 0;
    out.state = 0;

    for (int i = 0; i < 100000; i++) {
        rfun(&in.state, &in);
        wfun(&out.state, &out, &in);
        printf("%s\n", out.data);
    }

    return 0;
}



/*
 * Authors: Ossi Saresoja, Finland 2009-2012
 *          Tomi Jylhä-Ollila, Finland 2010-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include <debug/assert.h>
#include <Filter.h>
#include <mathnum/common.h>


void simple_lowpass_fir_create(int n, double f, double* coeffs)
{
    assert(coeffs != NULL);

    for (int i = 0; i <= n; ++i)
        coeffs[i] = 2 * f * sinc(PI * f * (2 * i - n));

    return;
}


void one_pole_filter_create(
        double f,
        int bandform,
        double coeffs[1],
        double* mul)
{
    assert(0 < f);
    assert(f < 0.5);
    assert(coeffs != NULL);
    assert(mul != NULL);

    //    static int created = 0;
    //    fprintf(stderr, "  %d \n", ++created);

    const double s  = sin(PI * f);
    const double c  = cos(PI * f);
    const double a0 =  s + c     ;
    coeffs[0] = (s - c) / a0;

    const double t = (bandform == 0) ? s : c;
    *mul = t / a0;

    return;
}


void two_pole_bandpass_filter_create(
        double f1,
        double f2,
        double coeffs[2],
        double* mul)
{
    assert(0 < f1);
    assert(f1 < f2);
    assert(f2 < 0.5);
    assert(coeffs != NULL);
    assert(mul != NULL);

    //    static int created = 0;
    //    fprintf(stderr, "  %d \n", ++created);

    const double sm = sin(PI * (f2 - f1));
    const double cm = cos(PI * (f2 - f1));
    const double cp = cos(PI * (f2 + f1));
    const double a0 =  cm + sm;

    coeffs[0] = (cm - sm) / a0;
    coeffs[1] = - 2 * cp  / a0;
    *mul = sm / a0;

    return;
}


void two_pole_filter_create(
        double f,
        double q,
        int bandform,
        double coeffs[2],
        double* mul)
{
    assert(0 < f);
    assert(f < 0.5);
    assert(q >= 0.5);
    assert(q <= 1000);
    assert(coeffs != NULL);
    assert(mul != NULL);

    //    static int created = 0;
    //    fprintf(stderr, "  %d \n", ++created);

    const double s2 = sin(2 * PI * f);
    const double c2 = cos(2 * PI * f);
    const double a0 =   1 + s2 / (2 * q);
    coeffs[0] =  (1 - s2 / (2 * q)) / a0;
    coeffs[1] = - 2 * c2            / a0;

    double t = (bandform == 0) ? sin(PI * f) : cos(PI * f);
    *mul = t * t / a0;

    return;
}

#define safe_sqrt(x) (sqrt(fmax(0.0, (x))))
void four_pole_bandpass_filter_create(
        double f1,
        double f2,
        double q,
        double coeffs[4],
        double* mul)
{
    assert(0  < f1);
    assert(f1 < f2);
    assert(f2 < 0.5);
    assert(q >= 0.5);
    assert(q <= 1000);
    assert(coeffs != NULL);
    assert(mul != NULL);

    //    static int created = 0;
    //    fprintf(stderr, "  %d \n", ++created);

    const double ss2 = sin(2 * PI * f1) * sin(2 * PI * f2);
    const double sp = sin(PI * (f2 + f1));
    const double sm = sin(PI * (f2 - f1));
    const double cp = cos(PI * (f2 + f1));
    const double cm = cos(PI * (f2 - f1));
    const double x = safe_sqrt((sp * sp) * (sp * sp) - (sm * sm) * ss2 / (q * q));
    const double y = (sm * sm + x) / ss2;
    const double z = sm * safe_sqrt(ss2 / (sp * sp + x) / 2) / q;
    const double ap = safe_sqrt((y + 1) / 2);
    const double am = safe_sqrt((y - 1) / 2);

    const double a0_1 =  cm * ap - cp * am + z        ;
    coeffs[0]   =       (cm * ap - cp * am - z) / a0_1;
    coeffs[1]   = - 2 * (cp * ap - cm * am)     / a0_1;

    const double a0_2 =  cm * ap + cp * am + z        ;
    coeffs[2]   =       (cm * ap + cp * am - z) / a0_2;
    coeffs[3]   = - 2 * (cp * ap + cm * am)     / a0_2;

    *mul =  sm * sm / (a0_1 * a0_2);

    return;
}


void butterworth_filter_create(
        int n,
        double f,
        int bandform,
        double coeffs[n],
        double* mul)

{
    assert(0 < f);
    assert(n > 0);
    assert(f < 0.5);
    assert(coeffs != NULL);
    assert(mul != NULL);

    //    static int created = 0;
    //    fprintf(stderr, "  %d \n", ++created);

    const double s  = sin(PI * f);
    const double c  = cos(PI * f);
    const double s2 = sin(2 * PI * f);
    const double c2 = cos(2 * PI * f);
    double a0 = 1.0;

    for (int i = 0; i < n / 2; i++)
    {
        const double si = sin(PI / 2 / n * (2 * i + 1));
        const double a0_temp =   1 + s2 * si           ;
        coeffs[2*i  ]        =  (1 - s2 * si) / a0_temp;
        coeffs[2*i+1]        = - 2 * c2       / a0_temp;
        a0 *= a0_temp;
    }

    if ((n & 1) != 0)
    {
        const double a0_temp =  s + c           ;
        coeffs[n-1]          = (s - c) / a0_temp;
        a0 *= a0_temp;
    }

    const double t = (bandform == 0) ? s : c;
    *mul = powi(t, n) /a0;

    return;
}


void butterworth_bandpass_filter_create(
        int n,
        double f1,
        double f2,
        double coeffs[2*n],
        double* mul)

{
    assert(0  < f1);
    assert(f1 < f2);
    assert(f2 < 0.5);
    assert(n > 0);
    assert(coeffs != NULL);
    assert(mul != NULL);

    //    static int created = 0;
    //    fprintf(stderr, "  %d \n", ++created);

    const double ss2 = sin(2 * PI * f1) * sin(2 * PI * f2);
    const double sp = sin(PI * (f2 + f1));
    const double sm = sin(PI * (f2 - f1));
    const double cp = cos(PI * (f2 + f1));
    const double cm = cos(PI * (f2 - f1));
    double a0 = 1.0;

    for (int i = 0; i < n / 2; i++)
    {
        const double si = sin(PI / 2 / n * (2 * i + 1));
        const double x = safe_sqrt((sp * sp) * (sp * sp) - 4 * (sm * sm) * ss2 * (si * si));
        const double y = (sm * sm + x) / ss2;
        const double z = sm * safe_sqrt(ss2 / (sp * sp + x) * 2) * si;
        const double ap = safe_sqrt((y + 1) / 2);
        const double am = safe_sqrt((y - 1) / 2);

        const double a0_1_temp =  cm * ap - cp * am + z             ;
        coeffs[0]        =       (cm * ap - cp * am - z) / a0_1_temp;
        coeffs[1]        = - 2 * (cp * ap - cm * am)     / a0_1_temp;

        const double a0_2_temp =  cm * ap + cp * am + z             ;
        coeffs[2]        =       (cm * ap + cp * am - z) / a0_2_temp;
        coeffs[3]        = - 2 * (cp * ap + cm * am)     / a0_2_temp;

        a0 *= a0_1_temp * a0_2_temp;
    }

    if ((n & 1) != 0)
    {
        const double a0_temp =  cm + sm           ;
        coeffs[2*n-2]        = (cm - sm) / a0_temp;
        coeffs[2*n-1]        = - 2 * cp  / a0_temp;
        a0 *= a0_temp;
    }

    *mul = powi(sm, n) /a0;

    return;
}


void buffer(
        kqt_frame* histbuf,
        const kqt_frame* restrict sourcebuf,
        int n,
        int nframes)
{
    assert(histbuf != NULL);
    assert(sourcebuf != NULL);

    if (nframes < n)
    {
        memmove(histbuf, histbuf + nframes, (n - nframes) * sizeof(kqt_frame));
        memcpy(histbuf + n - nframes, sourcebuf, nframes * sizeof(kqt_frame));
    }
    else
    {
        memcpy(histbuf, sourcebuf + nframes - n, n * sizeof(kqt_frame));
    }

    return;
}


double iir_filter_strict_cascade(
        int n,
        const double coeffs[n],
        double buf[n],
        double var)
{
    assert(coeffs != NULL);
    assert(buf != NULL);

    for (int i = 0; i < (n & ~((int)1)); i += 2)
    {
        var -= coeffs[i  ] * buf[i  ] + 
               coeffs[i+1] * buf[i+1];
        buf[i  ] = buf[i+1];
        buf[i+1] = var;
    }

    if ((n & 1) != 0)
    {
        var -= coeffs[n-1] * buf[n-1];
        buf[n-1] = var;
    }

    return var;
}


double iir_filter_strict_transposed_cascade(
        int n,
        const double coeffs[n],
        double buf[n],
        double var)
{
    assert(coeffs != NULL);
    assert(buf != NULL);

    for (int i = 0; i < (n & ~((int)1)); i += 2)
    {
        var += buf[i+1];
        buf[i+1] = buf[i] - coeffs[i+1] * var;
        buf[i  ] =        - coeffs[i  ] * var;
    }

    if ((n & 1) != 0)
    {
        var += buf[n-1];
        buf[n-1] =        - coeffs[n-1] * var;
    }

    return var;
}


double dc_zero_filter(
        int n,
        double buf[n],
        double var)
{
    assert(buf != NULL);

    for (int i = 0; i < n; ++i)
    {
        const double temp = buf[i];
        buf[i] = var;
        var -= temp;
    }

    return var;
}


double nq_zero_filter(
        int n,
        double buf[n],
        double var)
{
    assert(buf != NULL);

    for (int i = 0; i < n; ++i)
    {
        const double temp = buf[i];
        buf[i] = var;
        var += temp;
    }

    return var;
}


double dc_nq_zero_filter(
        int n,
        double buf[2*n],
        double var,
        int* s)
{
    assert(buf != NULL);
    assert(s != NULL);

    var = dc_zero_filter(n, buf + (*s & n), var);
    *s = ~*s;

    return var;
}


double dc_pole_filter(
        int n,
        double buf[n],
        double var)
{
    assert(buf != NULL);

    for (int i = 0; i < n; ++i)
    {
        var += buf[i];
        buf[i] = var;
    }

    return var;
}


double nq_pole_filter(
        int n,
        double buf[n],
        double var)
{
    assert(buf != NULL);

    for (int i = 0; i < n; ++i)
    {
        var -= buf[i];
        buf[i] = var;
    }

    return var;
}


#if 0
void fir_filter(int n,
                double* coeffs,
                kqt_frame* histbuf,
                int nframes,
                kqt_frame* inbuf,
                kqt_frame* outbuf)
{
    double temp;

    for (int i = 0; i < nframes; ++i)
    {
        temp = inbuf[i] * coeffs[n];
        dprod2(histbuf, inbuf, coeffs, n, i, temp, +=);
        outbuf[i] = temp;
    }

    buffer(histbuf, inbuf, n, nframes);

    return;
}
#endif



// Compare float / double operation speed.
// Run gled, then do:
//  .L testfltdbl.C+  // loads this in compiled mode
//  testfltdbl()

#include <../GledCore/Gled/GTime.h>
#include <cmath>
#include <cstdio>

typedef ULong64_t Count_t;

volatile Count_t NN = 100000000;
volatile float   FX = 0.000001f;
volatile double  DX = 0.000001;

/**************************************************************************/

double emptyloop()
{
  GTime t(GTime::I_Now);
  volatile Count_t j = 0;
  for (volatile Count_t i=0; i<NN; ++i)
    ++j;
  return t.TimeUntilNow().ToDouble() + (j - j);
}

/**************************************************************************/

double dbl_to_flt()
{
  GTime t(GTime::I_Now);
  volatile float f = 0.0f;
  volatile Count_t j = 0;
  for (volatile Count_t i=0; i<NN; ++i)
    f = DX, ++j;
  return t.TimeUntilNow().ToDouble() + (f - f) + (j - j);
}

double flt_to_dbl()
{
  GTime t(GTime::I_Now);
  volatile double f = 0.0;
  volatile Count_t j = 0;
  for (volatile Count_t i=0; i<NN; ++i)
    f = FX, ++j;
  return t.TimeUntilNow().ToDouble() + (f - f);
}

/**************************************************************************/

double dbl_plus_dbl()
{
  GTime t(GTime::I_Now);
  volatile double f = 0.0;
  volatile Count_t j = 0;
  for (volatile Count_t i=0; i<NN; ++i)
    f += DX, ++j;
  return t.TimeUntilNow().ToDouble() + (f - f);
}

double dbl_plus_flt()
{
  GTime t(GTime::I_Now);
  volatile double f = 0.0;
  volatile Count_t j = 0;
  for (volatile Count_t i=0; i<NN; ++i)
    f += FX, ++j;
  return t.TimeUntilNow().ToDouble() + (f - f);
}

double flt_plus_flt()
{
  GTime t(GTime::I_Now);
  volatile float f = 0.0f;
  volatile Count_t j = 0;
  for (volatile Count_t i=0; i<NN; ++i)
    f += FX, ++j;
  return t.TimeUntilNow().ToDouble() + (f - f);
}

double flt_plus_dbl()
{
  GTime t(GTime::I_Now);
  volatile float f = 0.0f;
  volatile Count_t j = 0;
  for (volatile Count_t i=0; i<NN; ++i)
    f += DX, ++j;
  return t.TimeUntilNow().ToDouble() + (f - f);
}

double flt_plus_dbl_2()
{
  GTime t(GTime::I_Now);
  volatile float f = 0.0f;
  volatile Count_t j = 0;
  for (volatile Count_t i=0; i<NN; ++i)
    f += (float) DX, ++j;
  return t.TimeUntilNow().ToDouble() + (f - f);
}

/**************************************************************************/

double dbl_times_dbl()
{
  GTime t(GTime::I_Now);
  volatile double f = 0.0;
  volatile Count_t j = 0;
  for (volatile Count_t i=0; i<NN; ++i)
    f *= DX, ++j;
  return t.TimeUntilNow().ToDouble() + (f - f);
}

double dbl_times_flt()
{
  GTime t(GTime::I_Now);
  volatile double f = 0.0;
  volatile Count_t j = 0;
  for (volatile Count_t i=0; i<NN; ++i)
    f *= FX, ++j;
  return t.TimeUntilNow().ToDouble() + (f - f);
}

double flt_times_flt()
{
  GTime t(GTime::I_Now);
  volatile float f = 0.0f;
  volatile Count_t j = 0;
  for (volatile Count_t i=0; i<NN; ++i)
    f *= FX, ++j;
  return t.TimeUntilNow().ToDouble() + (f - f);
}

double flt_times_dbl()
{
  GTime t(GTime::I_Now);
  volatile float f = 0.0f;
  volatile Count_t j = 0;
  for (volatile Count_t i=0; i<NN; ++i)
    f *= DX, ++j;
  return t.TimeUntilNow().ToDouble() + (f - f);
}

double flt_times_dbl_2()
{
  GTime t(GTime::I_Now);
  volatile float f = 0.0f;
  volatile Count_t j = 0;
  for (volatile Count_t i=0; i<NN; ++i)
    f *= (float) DX, ++j;
  return t.TimeUntilNow().ToDouble() + (f - f);
}

/**************************************************************************/

double dbl_divide_dbl()
{
  GTime t(GTime::I_Now);
  volatile double f = 0.0;
  volatile Count_t j = 0;
  for (volatile Count_t i=0; i<NN; ++i)
    f /= DX, ++j;
  return t.TimeUntilNow().ToDouble() + (f - f);
}

double dbl_divide_flt()
{
  GTime t(GTime::I_Now);
  volatile double f = 0.0;
  volatile Count_t j = 0;
  for (volatile Count_t i=0; i<NN; ++i)
    f /= FX, ++j;
  return t.TimeUntilNow().ToDouble() + (f - f);
}

double flt_divide_flt()
{
  GTime t(GTime::I_Now);
  volatile float f = 0.0f;
  volatile Count_t j = 0;
  for (volatile Count_t i=0; i<NN; ++i)
    f /= FX, ++j;
  return t.TimeUntilNow().ToDouble() + (f - f);
}

double flt_divide_dbl()
{
  GTime t(GTime::I_Now);
  volatile float f = 0.0f;
  volatile Count_t j = 0;
  for (volatile Count_t i=0; i<NN; ++i)
    f /= DX, ++j;
  return t.TimeUntilNow().ToDouble() + (f - f);
}

double flt_divide_dbl_2()
{
  GTime t(GTime::I_Now);
  volatile float f = 0.0f;
  volatile Count_t j = 0;
  for (volatile Count_t i=0; i<NN; ++i)
    f /= (float) DX, ++j;
  return t.TimeUntilNow().ToDouble() + (f - f);
}

/**************************************************************************/

double sin_flt()
{
  GTime t(GTime::I_Now);
  volatile float f = 0.14f;
  volatile Count_t j = 0;
  for (volatile Count_t i=0; i<NN; ++i)
    f = sin(f), ++j;
  return t.TimeUntilNow().ToDouble() + (f - f);
}

double sin_dbl()
{
  GTime t(GTime::I_Now);
  volatile double f = 0.14;
  volatile Count_t j = 0;
  for (volatile Count_t i=0; i<NN; ++i)
    f = sin(f), ++j;
  return t.TimeUntilNow().ToDouble() + (f - f);
}

double sinf_flt()
{
  GTime t(GTime::I_Now);
  volatile float f = 0.14f;
  volatile Count_t j = 0;
  for (volatile Count_t i=0; i<NN; ++i)
    f = sinf(f), ++j;
  return t.TimeUntilNow().ToDouble() + (f - f);
}

double sinf_dbl()
{
  GTime t(GTime::I_Now);
  volatile double f = 0.14;
  volatile Count_t j = 0;
  for (volatile Count_t i=0; i<NN; ++i)
    f = sinf(f), ++j;
  return t.TimeUntilNow().ToDouble() + (f - f);
}

/**************************************************************************/

typedef double (*foo)();

struct bar {
  const char*  name;
  foo          func;
};

bar tests[] =
{
  { "emptyloop",	emptyloop	},
  { "dbl_to_flt",	dbl_to_flt,	},
  { "flt_to_dbl",	flt_to_dbl	},

  { "dbl_plus_dbl",	dbl_plus_dbl	},
  { "dbl_plus_flt",	dbl_plus_flt	},
  { "flt_plus_flt",	flt_plus_flt	},
  { "flt_plus_dbl",	flt_plus_dbl	},
  { "flt_plus_dbl_2",	flt_plus_dbl_2	},

  { "dbl_times_dbl",	dbl_times_dbl	},
  { "dbl_times_flt",	dbl_times_flt	},
  { "flt_times_flt",	flt_times_flt	},
  { "flt_times_dbl",	flt_times_dbl	},
  { "flt_times_dbl_2",	flt_times_dbl_2	},

  { "dbl_divide_dbl",	dbl_divide_dbl	},
  { "dbl_divide_flt",	dbl_divide_flt	},
  { "flt_divide_flt",	flt_divide_flt	},
  { "flt_divide_dbl",	flt_divide_dbl	},
  { "flt_divide_dbl_2",	flt_divide_dbl_2},

  { "sin_flt",		sin_flt         },
  { "sin_dbl",	        sin_dbl 	},
  { "sinf_flt",		sinf_flt        },
  { "sinf_dbl",	        sinf_dbl 	},

  // { "",		},
  { 0, 0 }
};

void testfltdbl()
{
  bar* tst = tests;
  while (tst->name)
  {
    double time = tst->func();
    printf("%-16s %-10f\n", tst->name, time);
    ++tst;
  }
}

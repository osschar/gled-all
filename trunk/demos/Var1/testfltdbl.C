#include <Gled/GTime.h>
#include <math.h>

int    NN = 1000000000;
float  FX = 0.000001f;
double DX = 0.000001;

/**************************************************************************/

double emptyloop()
{
  GTime t(GTime::I_Now);
  for (int i=0; i<NN; ++i);
  return t.TimeUntilNow().ToDouble();
}

double emptyloop2()
{
  GTime t(GTime::I_Now);
  int n = NN;
  while(n--);
  return t.TimeUntilNow().ToDouble();
}

/**************************************************************************/

double dbl_to_flt()
{
  GTime t(GTime::I_Now);
  float f = 0.0f;
  for (int i=0; i<NN; ++i)
    f = DX;
  return t.TimeUntilNow().ToDouble();
}

double flt_to_dbl()
{
  GTime t(GTime::I_Now);
  double f = 0.0;
  for (int i=0; i<NN; ++i)
    f = FX;
  return t.TimeUntilNow().ToDouble();
}

/**************************************************************************/

double dbl_plus_dbl()
{
  GTime t(GTime::I_Now);
  double f = 0.0;
  for (int i=0; i<NN; ++i)
    f += DX;
  return t.TimeUntilNow().ToDouble();
}

double dbl_plus_flt()
{
  GTime t(GTime::I_Now);
  double f = 0.0;
  for (int i=0; i<NN; ++i)
    f += FX;
  return t.TimeUntilNow().ToDouble();
}

double flt_plus_flt()
{
  GTime t(GTime::I_Now);
  float f = 0.0f;
  for (int i=0; i<NN; ++i)
    f += FX;
  return t.TimeUntilNow().ToDouble();
}

double flt_plus_dbl()
{
  GTime t(GTime::I_Now);
  float f = 0.0f;
  for (int i=0; i<NN; ++i)
    f += DX;
  return t.TimeUntilNow().ToDouble();
}

/**************************************************************************/

double dbl_times_dbl()
{
  GTime t(GTime::I_Now);
  double f = 0.0;
  for (int i=0; i<NN; ++i)
    f *= DX;
  return t.TimeUntilNow().ToDouble();
}

double dbl_times_flt()
{
  GTime t(GTime::I_Now);
  double f = 0.0;
  for (int i=0; i<NN; ++i)
    f *= FX;
  return t.TimeUntilNow().ToDouble();
}

double flt_times_flt()
{
  GTime t(GTime::I_Now);
  float f = 0.0f;
  for (int i=0; i<NN; ++i)
    f *= FX;
  return t.TimeUntilNow().ToDouble();
}

double flt_times_dbl()
{
  GTime t(GTime::I_Now);
  float f = 0.0f;
  for (int i=0; i<NN; ++i)
    f *= DX;
  return t.TimeUntilNow().ToDouble();
}

/**************************************************************************/

double dbl_divide_dbl()
{
  GTime t(GTime::I_Now);
  double f = 0.0;
  for (int i=0; i<NN; ++i)
    f /= DX;
  return t.TimeUntilNow().ToDouble();
}

double dbl_divide_flt()
{
  GTime t(GTime::I_Now);
  double f = 0.0;
  for (int i=0; i<NN; ++i)
    f /= FX;
  return t.TimeUntilNow().ToDouble();
}

double flt_divide_flt()
{
  GTime t(GTime::I_Now);
  float f = 0.0f;
  for (int i=0; i<NN; ++i)
    f /= FX;
  return t.TimeUntilNow().ToDouble();
}

double flt_divide_dbl()
{
  GTime t(GTime::I_Now);
  float f = 0.0f;
  for (int i=0; i<NN; ++i)
    f /= DX;
  return t.TimeUntilNow().ToDouble();
}

/**************************************************************************/

double sin_flt()
{
  GTime t(GTime::I_Now);
  float f = 0.14f;
  for (int i=0; i<NN; ++i)
    f = sin(f);
  return t.TimeUntilNow().ToDouble();
}

double sin_dbl()
{
  GTime t(GTime::I_Now);
  double f = 0.14;
  for (int i=0; i<NN; ++i)
    f = sin(f);
  return t.TimeUntilNow().ToDouble();
}


/**************************************************************************/

typedef double (*foo)();

struct bar {
  const char*  name;
  foo          func;
};

bar tests[] = {
  { "emptyloop",	emptyloop	},
  /*
  { "emptyloop2",	emptyloop2	},
  { "dbl_to_flt",	dbl_to_flt,	},
  { "flt_to_dbl",	flt_to_dbl	},
  { "dbl_plus_dbl",	dbl_plus_dbl	},
  { "dbl_plus_flt",	dbl_plus_flt	},
  { "flt_plus_flt",	flt_plus_flt	},
  { "flt_plus_dbl",	flt_plus_dbl	},
  { "dbl_times_dbl",	dbl_times_dbl	},
  { "dbl_times_flt",	dbl_times_flt	},
  { "flt_times_flt",	flt_times_flt	},
  { "flt_times_dbl",	flt_times_dbl	},
  { "dbl_divide_dbl",	dbl_divide_dbl	},
  { "dbl_divide_flt",	dbl_divide_flt	},
  { "flt_divide_flt",	flt_divide_flt	},
  { "flt_divide_dbl",	flt_divide_dbl	},
  */
  { "sin_flt",		sin_flt         },
  { "sin_dbl",	        sin_dbl 	},

  // { "",		},
  { 0, 0 }
};
    
int main()
{
  double empty = emptyloop();

  bar* tst = tests;
  while (tst->name)
  {
    double time = tst->func();
    printf("%-16s %-10f %-10f\n", tst->name, time, time - empty);
    ++tst;
  }
}

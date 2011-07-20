#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "deltas.h"
#include "deltas_api.h"

#ifdef SWIG
% include deltas_api.h
#endif
/** \file
\brief The Caperiffic API
*/
  Deltas_state * deltas_new (void)
{
  State *s = malloc (sizeof (State));

  deltas_init_state (s);

  return (Deltas_state *) s;
}

Deltas_state *
deltas_destroy (Deltas_state * s)
{
  if (s)
  {

    deltas_free_state ((State *) s);
    free (s);
  }
  return NULL;
}

Deltas_state *
deltas_init (Deltas_state * s)
{
  if (!s)
  {
    s = deltas_new ();
  }

  _cem_initialize ((State *) s);

  return s;
}

int
deltas_run_until (Deltas_state * s, double time_in_days)
{
  State *p = (State *) s;

  int until_time_step = time_in_days / TimeStep;

  return _cem_run_until (p, until_time_step);
}

Deltas_state *
deltas_finalize (Deltas_state * s, int free)
{
  State *p = (State *) s;

  _cem_finalize (p);

  if (free)
    s = deltas_destroy (s);

  return s;
}

Deltas_state *
deltas_set_save_file (Deltas_state * s, char *name)
{
  State *p = (State *) s;

  p->savefilename = strdup (name);
  return s;
}

Deltas_state *
deltas_set_read_file (Deltas_state * s, char *name)
{
  State *p = (State *) s;

  p->readfilename = strdup (name);
  return s;
}

Deltas_state *
deltas_init_grid_shape (Deltas_state * s, int dimen[2])
{
  State *p = (State *) s;

  fprintf (stderr, "*** Grid size is (%d,%d)\n", p->nx, p->ny);
  fprintf (stderr, "*** Requested size is (%d,%d)\n", dimen[0], dimen[1]);
  if (s && (p->nx == 0 && p->ny == 0))
  {
    int i;

    //const int len = dimen[0] * (dimen[1] * 2);
    const int len = dimen[0] * dimen[1];

    //const int stride = dimen[1] * 2;
    const int stride = dimen[1];

    p->nx = dimen[0];
    p->ny = dimen[1]/2;
    p->max_beach_len = len;

    p->AllBeach = (char **)malloc (sizeof (char *) * p->nx);
    p->PercentFull = (double **)malloc (sizeof (double *) * p->nx);
    p->Age = (int **)malloc (sizeof (int *) * p->nx);
    p->CellDepth = (double **)malloc (sizeof (double *) * p->nx);
    p->InitDepth = (double **)malloc (sizeof (double *) * p->nx);

    p->AllBeach[0] = (char *)malloc (sizeof (char) * len);
    p->PercentFull[0] = (double *)malloc (sizeof (double) * len);
    p->Age[0] = (int *)malloc (sizeof (int) * len);
    p->CellDepth[0] = (double *)malloc (sizeof (double) * len);
    p->InitDepth[0] = (double *)malloc (sizeof (double) * len);

    for (i = 1; i < p->nx; i++)
    {
      p->AllBeach[i] = p->AllBeach[i - 1] + stride;
      p->PercentFull[i] = p->PercentFull[i - 1] + stride;
      p->Age[i] = p->Age[i - 1] + stride;
      p->CellDepth[i] = p->CellDepth[i - 1] + stride;
      p->InitDepth[i] = p->InitDepth[i - 1] + stride;
    }

    p->river_flux = (double *)malloc (sizeof (double) * len);
    p->river_x = (int *)malloc (sizeof (int) * len);
    p->river_y = (int *)malloc (sizeof (int) * len);
    p->n_rivers = 1;

    p->X = (int *)malloc (sizeof (int) * len);
    p->Y = (int *)malloc (sizeof (int) * len);
    p->InShadow = (char *)malloc (sizeof (char) * len);
    p->ShorelineAngle = (double *)malloc (sizeof (double) * len);
    p->SurroundingAngle = (double *)malloc (sizeof (double) * len);
    p->UpWind = (char *)malloc (sizeof (char) * len);
    p->VolumeIn = (double *)malloc (sizeof (double) * len);
    p->VolumeOut = (double *)malloc (sizeof (double) * len);
  }
  fprintf (stderr, "*** New grid size is (%d,%d)\n",
           deltas_get_nx (s), deltas_get_ny (s));

  return s;
}

Deltas_state *
deltas_init_cell_width (Deltas_state * s, double dx)
{
  State *p = (State *) s;

  if (s)
  {
    p->cell_width = dx;
  }
  return s;
}

Deltas_state *
deltas_init_grid (Deltas_state * s, double *z)
{
  State *p = (State *) s;

  if (s && (p->nx > 0 && p->ny > 0))
  {
    int i;

    const int len = p->nx * p->ny * 2;

    for (i = 0; i < len; i++)
      p->InitDepth[0][i] = z[i];
  }

  return s;
}

Deltas_state *
deltas_destroy_grid (Deltas_state * s)
{
  State *p = (State *) s;

  if (p)
  {
    p->nx = 0;
    p->ny = 0;

    free (p->AllBeach[0]);
    free (p->AllBeach);

    free (p->Age[0]);
    free (p->Age);

    free (p->PercentFull[0]);
    free (p->PercentFull);

    free (p->CellDepth[0]);
    free (p->CellDepth);

    free (p->InitDepth[0]);
    free (p->InitDepth);
  }

  return s;
}

double
deltas_get_sed_rate (Deltas_state * s)
{
  State *p = (State *) s;

  return p->SedRate;
}

Deltas_state *
deltas_find_river_mouth (Deltas_state * s, int n)
{
  State *p = (State *) s;

  int x,
    y;

  x = 0;
  y = p->stream_spot;

  while (p->AllBeach[x][y] == 'y')
  {
    x += 1;
  }

  p->river_x[n] = x;
  p->river_y[n] = y;

//fprintf (stderr, "x = %d\n", x);
//fprintf (stderr, "y = %d\n", y);

  return s;
}

void
deltas_avulsion (Deltas_state * s, double *qs, double river_flux)
{
  State *p = (State *) s;

  int x,
    y;

  int i;

  int qs_x,
    qs_y,
    qs_i;

  int len;

  deltas_find_river_mouth (s, 0);

  x = p->river_x[0];
  y = p->river_y[0];

  len = deltas_get_nx (s) * deltas_get_ny (s) / 2;
  for (i = 0; i < len; i++)
    qs[i] = 0;

  qs_x = x;
  qs_y = y % deltas_get_ny (s) - deltas_get_ny (s) / 4;
  qs_i = qs_x * deltas_get_ny (s) / 2 + qs_y;

//fprintf (stderr, "qs_x = %d\n", qs_x);
//fprintf (stderr, "qs_y = %d\n", qs_y);

  qs[qs_i] = river_flux;

  return;
}

Deltas_state *
deltas_set_sed_rate (Deltas_state * s, double rate)
{
  State *p = (State *) s;

  p->SedRate = rate;
  return s;
}

/** Set sediment flux in kg/s
*/
Deltas_state *
deltas_set_sed_flux (Deltas_state * s, double flux)
{
  State *p = (State *) s;

  p->SedFlux = flux;
  return s;
}

/** Set sediment flux in kg/s
*/
Deltas_state *
deltas_set_river_sed_flux (Deltas_state * s, double flux, int n)
{
  State *p = (State *) s;

  p->river_flux[n] = flux;
  return s;
}

/** Set sediment flux in kg/s
*/
Deltas_state *
deltas_set_river_position (Deltas_state * s, int x, int y, int n)
{
  State *p = (State *) s;

  p->river_x[n] = x;
  p->river_y[n] = y;
  return s;
}

Deltas_state *
deltas_set_sediment_flux_grid_old (Deltas_state * s, double *qs)
{
  State *p = (State *) s;

  int i;

  int n;

  int len;
  const int lower[2] = { deltas_get_ny (s) / 4, 0 };
  const int stride[2] = { 1, deltas_get_ny (s) / 2 };
  int dimen[3];

  fprintf (stderr, "Set flux grid\n");
  fprintf (stderr, "Start.\n");
  fflush (stderr);
  deltas_get_value_dimen (s, NULL, dimen);

  len = dimen[0] * dimen[1] * dimen[2];

  for (i = 0, n = 0; i < len; i++)
  {
    if (qs[i] > 0)
    {
      p->river_flux[n] = qs[i];
      p->river_x[n] = i / stride[1];
      p->river_y[n] = i % stride[1] + lower[0];
      fprintf (stderr, "  river position = %d, %d\n", p->river_x[n],
               p->river_y[n]);

      fprintf (stderr, "n = %d\n", n);
      fprintf (stderr, "i = %d\n", i);
      fprintf (stderr, "river_x = %d\n", p->river_x[n]);
      fprintf (stderr, "river_y = %d\n", p->river_y[n]);
      fprintf (stderr, "qs[%d] = %f\n", i, qs[i]);
      n++;
    }
  }
  p->n_rivers = n;
  fprintf (stderr, "Number of rivers = %d\n", n);
  fprintf (stderr, "Done.\n");
  fflush (stderr);

  return s;
}

Deltas_state *
deltas_set_sediment_flux_grid (Deltas_state * s, double *qs)
{
  State *p = (State *) s;

  int i;

  int n;

  int len;
  const int lower[2] = { 0, 0 };
  const int stride[2] = { 1, deltas_get_ny (s) };
  int dimen[3];

  fprintf (stderr, "Set flux grid\n");
  fprintf (stderr, "Start.\n");
  fflush (stderr);
  deltas_get_value_dimen (s, NULL, dimen);

  len = dimen[0] * dimen[1] * dimen[2];

  for (i = 0, n = 0; i < len; i++)
  {
    if (qs[i] > 0)
    {
      p->river_flux[n] = qs[i];
      p->river_x[n] = i / stride[1];
      p->river_y[n] = i % stride[1] + lower[0];
      fprintf (stderr, "  river position = %d, %d\n", p->river_x[n],
               p->river_y[n]);

      fprintf (stderr, "n = %d\n", n);
      fprintf (stderr, "i = %d\n", i);
      fprintf (stderr, "river_x = %d\n", p->river_x[n]);
      fprintf (stderr, "river_y = %d\n", p->river_y[n]);
      fprintf (stderr, "qs[%d] = %f\n", i, qs[i]);
      n++;
    }
  }
  p->n_rivers = n;
  fprintf (stderr, "Number of rivers = %d\n", n);
  fprintf (stderr, "Done.\n");
  fflush (stderr);

  return s;
}

Deltas_state *
deltas_set_angle_asymmetry (Deltas_state * s, double angle_asymmetry)
{
  State *p = (State *) s;

  p->angle_asymmetry = angle_asymmetry;
  return s;
}

Deltas_state *
deltas_set_angle_highness (Deltas_state * s, double angle_highness)
{
  State *p = (State *) s;

  p->angle_highness = angle_highness;
  return s;
}

Deltas_state *
deltas_set_wave_angle (Deltas_state * s, double wave_angle)
{
  State *p = (State *) s;

  p->WaveAngle = wave_angle;
  return s;
}

Deltas_state *
deltas_set_depth (Deltas_state * s, double *depth)
{
  State *p = (State *) s;

  memcpy (p->CellDepth[0], depth, sizeof (p->CellDepth));
  return s;
}

Deltas_state *
deltas_set_wave_height (Deltas_state * s, double height_in_m)
{
  State *p = (State *) s;

  p->wave_height = height_in_m;
  return s;
}

Deltas_state *
deltas_set_wave_period (Deltas_state * s, double period_in_s)
{
  State *p = (State *) s;

  p->wave_period = period_in_s;
  return s;
}

Deltas_state *
deltas_set_shoreface_slope (Deltas_state * s, double shoreface_slope)
{
  State *p = (State *) s;

  p->shoreface_slope = shoreface_slope;
  return s;
}

Deltas_state *
deltas_set_shelf_slope (Deltas_state * s, double shelf_slope)
{
  State *p = (State *) s;

  p->shelf_slope = shelf_slope;
  return s;
}

Deltas_state *
deltas_set_shoreface_depth (Deltas_state * s, double shoreface_depth)
{
  State *p = (State *) s;

  p->shoreface_depth = shoreface_depth;
  return s;
}

Deltas_state *
deltas_set_cell_width (Deltas_state * s, double cell_width)
{
  State *p = (State *) s;

  p->cell_width = cell_width;
  return s;
}

const char *_deltas_exchange_items[] = {
  "DEPTH",
  "PERCENT_FILLED",
  NULL
};

const char **
deltas_get_exchange_items (void)
{
  return _deltas_exchange_items;
}

const double *
deltas_get_value_grid (Deltas_state * s, const char *value)
{
  if (strcasecmp (value, "DEPTH") == 0)
    return deltas_get_depth (s);
  else if (strcasecmp (value, "PERCENT_FILLED") == 0)
    return deltas_get_percent (s);
//  else if (strcasecmp (value, "Elevation") == 0)
//    return deltas_get_elevation_dup (s);
  else
    fprintf (stderr, "ERROR: %s: Bad value string.", value);

  return NULL;
}

double *
deltas_get_value_grid_dup (Deltas_state * s, const char *value)
{
  if (strcasecmp (value, "DEPTH") == 0)
    return deltas_get_depth_dup (s);
  else if (strcasecmp (value, "PERCENT_FILLED") == 0)
    return deltas_get_percent_dup (s);
  else if (strcasecmp (value, "Elevation") == 0)
    return deltas_get_elevation_dup (s);
  else
    fprintf (stderr, "ERROR: %s: Bad value string.", value);

  return NULL;
}

const double *
deltas_get_value_data (Deltas_state * s, const char *value, int lower[2],
                       int upper[2], int stride[2])
{
  double *data = NULL;

  lower[0] = 0;
  lower[1] = 0;
  upper[0] = deltas_get_ny (s) - 1;
  upper[1] = deltas_get_nx (s) - 1;
  stride[0] = 1;
  stride[1] = deltas_get_ny (s);

  if (strcasecmp (value, "DEPTH") == 0)
    data = (double*)deltas_get_depth (s);
  else if (strcasecmp (value, "PERCENT_FILLED") == 0)
    data = (double*)deltas_get_percent (s);
//  else if (strcasecmp (value, "ELEVATION") == 0)
//    data = (double*)deltas_get_elevation (s);
  else
    fprintf (stderr, "ERROR: %s: Bad value string.", value);

  return (const double*)data;
}

double *
deltas_get_value_data_dup (Deltas_state * s, const char *value, int lower[2],
                       int upper[2], int stride[2])
{
  double *data = NULL;

/*
  lower[0] = deltas_get_ny (s)/4;
  lower[1] = 0;
  upper[0] = 3*deltas_get_ny (s)/4-1;
  upper[1] = deltas_get_nx (s)-1;
  stride[0] = 1;
  stride[1] = deltas_get_ny (s);
*/
  lower[0] = 0;
  lower[1] = 0;
  upper[0] = deltas_get_ny (s) / 2 - 1;
  upper[1] = deltas_get_nx (s) - 1;
  stride[0] = 1;
  stride[1] = deltas_get_ny (s) / 2;

  if (strcasecmp (value, "DEPTH") == 0)
    data = deltas_get_depth_dup (s);
  else if (strcasecmp (value, "PERCENT_FILLED") == 0)
    data = deltas_get_percent_dup (s);
  else if (strcasecmp (value, "Elevation") == 0)
    data = deltas_get_elevation_dup (s);
  else
    fprintf (stderr, "ERROR: %s: Bad value string.", value);

  return data;
}

int *
deltas_get_value_dimen_old (Deltas_state * s, const char *value, int shape[3])
{
  shape[0] = deltas_get_ny (s) / 2;
  //shape[0] = deltas_get_ny (s);
  shape[1] = deltas_get_nx (s);
  shape[2] = 1;

  return shape;
}

int *
deltas_get_value_dimen (Deltas_state * s, const char *value, int shape[3])
{
  shape[0] = deltas_get_ny (s);
  //shape[0] = deltas_get_ny (s);
  shape[1] = deltas_get_nx (s);
  shape[2] = 1;

  return shape;
}

double *
deltas_get_value_res (Deltas_state * s, const char *value, double res[3])
{
  res[0] = deltas_get_dy (s);
  res[1] = deltas_get_dx (s);
  res[2] = 1;

  return res;
}

const double *
deltas_get_depth (Deltas_state * s)
{
  State *p = (State *) s;

  return p->CellDepth[0];
}

double *
dup_subgrid (Deltas_state * s, double **src)
{
  double *dest = NULL;

  {
    int lower[2] = { deltas_get_ny (s) / 4, 0 };
    int upper[2] = { 3 * deltas_get_ny (s) / 4 - 1, deltas_get_nx (s) - 1 };
    int stride[2] = { 1, deltas_get_ny (s) };
    const int len = (upper[0] - lower[0] + 1) * (upper[1] - lower[1] + 1);

    dest = (double *)malloc (sizeof (double) * len);

    if (dest)
    {
      //int src_id;
      //int dst_id;
      int id;

      int i,
        j;

/*
      for (src_id=lower[0], dst_id=0, j=lower[1]; j<=upper[1];
           src_id+=(stride[1]-(upper[0]-lower[0])), j++)
        for (i=lower[0]; i<=upper[0]; src_id+=stride[0], dst_id++, i++)
          dest[dst_id] = src[src_id];
*/
      for (i = lower[1], id = 0; i <= upper[1]; i++)
        for (j = lower[0]; j <= upper[0]; j++, id++)
          dest[id] = src[i][j];
    }
  }

  return dest;
}

double *
deltas_get_depth_dup (Deltas_state * s)
{
  State *p = (State *) s;

  double *val = NULL;

  {
/*
    const int len = deltas_get_nx (s)*deltas_get_ny (s);
    const double* f_val = deltas_get_depth (s);

    val = (double*) malloc (sizeof(double)*len);
    if (val)
    {
      int i;
      for (i=0; i<len; i++)
        val[i] = f_val[i];
    }
*/
    val = dup_subgrid (s, p->CellDepth);
  }

  return val;
}

double *
deltas_get_elevation_dup (Deltas_state * s)
{
  State *p = (State *) s;

  double *val = NULL;

  {
    const int len = deltas_get_nx (s) * deltas_get_ny (s) / 2;

    int i;

    //val = deltas_get_depth_dup (s);
    val = dup_subgrid (s, p->CellDepth);
    for (i = 0; i < len; i++)
      val[i] *= -1;
  }

  return val;
}

const double *
deltas_get_percent (Deltas_state * s)
{
  State *p = (State *) s;

  return p->PercentFull[0];
}

double *
deltas_get_percent_dup (Deltas_state * s)
{
  State *p = (State *) s;

  double *val = NULL;

  {
/*
    const int len = deltas_get_nx (s)*deltas_get_ny (s);
    const double* f_val = deltas_get_percent (s);

    val = (double*) malloc (sizeof(double)*len);

    if (val)
    {
      int i;
      for (i=0; i<len; i++)
        val[i] = f_val[i];
    }
*/
    val = dup_subgrid (s, p->PercentFull);
  }

  return val;
}

double
deltas_get_angle_asymmetry (Deltas_state * s)
{
  State *p = (State *) s;

  return p->angle_asymmetry;
}

double
deltas_get_angle_highness (Deltas_state * s)
{
  State *p = (State *) s;

  return p->angle_highness;
}

double
deltas_get_wave_angle (Deltas_state * s)
{
  State *p = (State *) s;

  return p->WaveAngle;
}

double
deltas_get_start_time (Deltas_state * s)
{
  return 0.;
}

#include <float.h>
double
deltas_get_end_time (Deltas_state * s)
{
  return DBL_MAX;
}

double
deltas_get_current_time (Deltas_state * s)
{
  State *p = (State *) s;

  return p->CurrentTimeStep * TimeStep;
}

int
deltas_get_nx (Deltas_state * s)
{
  State *p = (State *) s;

  return p->nx;
}

int
deltas_get_ny (Deltas_state * s)
{
  State *p = (State *) s;

  return p->ny * 2;
  //return 2*Ymax;
}

int
deltas_get_stride_old (Deltas_state * s, int dimen)
{
  if (dimen == 0)
    return 1;
  else if (dimen == 1)
    return deltas_get_ny (s) / 2;
  else
    return 0;
}

int
deltas_get_stride (Deltas_state * s, int dimen)
{
  if (dimen == 0)
    return 1;
  else if (dimen == 1)
    return deltas_get_ny (s);
  else
    return 0;
}

int
deltas_get_len (Deltas_state * s, int dimen)
{
  if (dimen == 0)
    return deltas_get_nx (s);
  else if (dimen == 1)
    return deltas_get_ny (s);
  else
    return 0;
}

double
deltas_get_dx (Deltas_state * s)
{
  State *p = (State *) s;

  return p->cell_width;
  //return CellWidth;
}

double
deltas_get_dy (Deltas_state * s)
{
  State *p = (State *) s;

  return p->cell_width;
  //return CellWidth;
}

void
deltas_use_external_waves (Deltas_state * s)
{
  State *p = (State *) s;

  p->external_waves = TRUE;
  //fprintf (stderr, "*** Ignoring request for external waves\n");
  //p->external_waves = FALSE;
}

void
deltas_use_sed_flux (Deltas_state * s)
{
  State *p = (State *) s;

  p->use_sed_flux = TRUE;
  //fprintf (stderr, "*** Ignoring request for sediment flux\n");
  //p->use_sed_flux = FALSE;
}

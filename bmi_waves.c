#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include "bmi.h"

/* Implement this: Add model-specific includes */
#include "waves_model.h"


static int
get_component_name (void *self, char * name)
{
    strncpy (name, "waves", BMI_MAX_COMPONENT_NAME);
    return BMI_SUCCESS;
}


#define INPUT_VAR_NAME_COUNT (4)
static const char *input_var_names[INPUT_VAR_NAME_COUNT] = {
    "sea_surface_water_wave__height",
    "sea_surface_water_wave__period",
    "sea_shoreline_wave~incoming~deepwater__ashton_et_al_approach_angle_highness_parameter",
    "sea_shoreline_wave~incoming~deepwater__ashton_et_al_approach_angle_asymmetry_parameter"
};


static int
get_input_var_name_count(void *self, int *count)
{
    *count = INPUT_VAR_NAME_COUNT;
    return BMI_SUCCESS;
}


static int
get_input_var_names(void *self, char **names)
{
    int i;
    for (i=0; i<INPUT_VAR_NAME_COUNT; i++) {
        strncpy(names[i], input_var_names[i], BMI_MAX_VAR_NAME);
    }
    return BMI_SUCCESS;
}


#define OUTPUT_VAR_NAME_COUNT (6)
static const char *output_var_names[OUTPUT_VAR_NAME_COUNT] = {
    "sea_surface_water_wave__min_of_increment_of_azimuth_angle_of_opposite_of_phase_velocity",
    "sea_surface_water_wave__azimuth_angle_of_opposite_of_phase_velocity",
    "sea_surface_water_wave__mean_of_increment_of_azimuth_angle_of_opposite_of_phase_velocity",
    "sea_surface_water_wave__max_of_increment_of_azimuth_angle_of_opposite_of_phase_velocity",
    "sea_surface_water_wave__height",
    "sea_surface_water_wave__period"
};


static int
get_output_var_name_count(void *self, int *count)
{
    *count = OUTPUT_VAR_NAME_COUNT;
    return BMI_SUCCESS;
}


static int
get_output_var_names(void *self, char **names)
{
    int i;
    for (i=0; i<OUTPUT_VAR_NAME_COUNT; i++) {
        strncpy(names[i], output_var_names[i], BMI_MAX_VAR_NAME);
    }
    return BMI_SUCCESS;
}


static int
get_start_time(void * self, double *time)
{
    *time = 0.0;
    return BMI_SUCCESS;
}


static int
get_end_time(void * self, double *time)
{ /* Implement this: Set end time */
    WavesModel *model = (WavesModel*)self;
    *time = model->end * model->time_step;
    return BMI_SUCCESS;
}


static int
get_current_time(void * self, double *time)
{ /* Implement this: Set current time */
    WavesModel *model = (WavesModel*)self;
    *time = model->now * model->time_step;
    return BMI_SUCCESS;
}


static int
get_time_step(void * self, double *dt)
{ /* Implement this: Set time step */
    WavesModel *model = (WavesModel*)self;
    *dt = model->time_step;
    return BMI_SUCCESS;
}


static int
get_time_units(void * self, char *units)
{
    strncpy(units, "d", BMI_MAX_UNITS_NAME);
    return BMI_SUCCESS;
}


static int
initialize(const char * file, void **handle)
{ /* Implement this: Create and initialize a model handle */
  {
    WavesModel * self = waves_new();
    double end_time = 20.;
    double wave_height = 2.;
    double wave_period = 7.;
    double angle_highness = 0.2;
    double angle_asymmetry = 0.5;

    //_waves_initialize((State *) self);

    if (file) {
      FILE *fp = fopen(file, "r");
      if (fp) {
        int n_assigned;
        n_assigned = fscanf(fp, "%lf, %lf, %lf, %lf, %lf", &end_time,
            &wave_height, &wave_period, &angle_highness, &angle_asymmetry);
        if (n_assigned != 5)
          return BMI_FAILURE;
      }
      else
        return BMI_FAILURE;
    }

    waves_set_height(self, wave_height);
    waves_set_period(self, wave_period);
    waves_set_angle_highness(self, angle_highness);
    waves_set_angle_asymmetry(self, angle_asymmetry);

    {
      WavesModel *p = (WavesModel *) self;
      p->end = end_time / p->time_step;
      fprintf(stderr, "Setting end time to %d\n", p->end);
      fflush(stderr);
    }

    *handle = self;
  }

  return BMI_SUCCESS;
}


static int
update_frac(void * self, double f)
{ /* Implement this: Update for a fraction of a time step */
    WavesModel *p = (WavesModel *) self;
    double now;
    //int until_time_step = p->time_step * f;

    get_current_time(self, &now);
    //until_time_step = now + p->time_step * f;

    //waves_run_until (p, until_time_step);
    waves_run_until (p, now + p->time_step * f);

    return BMI_SUCCESS;
}


static int
update(void * self)
{
    return update_frac(self, 1.);
}


static int
update_until(void * self, double then)
{
    double dt;
    double now;

    if (get_time_step(self, &dt) == BMI_FAILURE)
        return BMI_FAILURE;

    if (get_current_time(self, &now) == BMI_FAILURE)
        return BMI_FAILURE;

    {
        int n;
        const double n_steps = (then - now) / dt;
        for (n=0; n<(int)n_steps; n++) {
            if (update(self) == BMI_FAILURE)
                return BMI_FAILURE;
        }

        if (update_frac(self, n_steps - (int)n_steps) == BMI_FAILURE)
            return BMI_FAILURE;
    }

    return BMI_SUCCESS;
}


static int
finalize(void * self)
{ /* Implement this: Clean up */
    waves_destroy ((WavesModel*) self);

    return BMI_SUCCESS;
}


static int
get_grid_type(void *self, int id, char *type)
{
    if (id == 0) {
        strncpy(type, "scalar", 2048);
    } else {
        type[0] = '\0'; return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_grid_rank(void *self, int id, int *rank)
{
    if (id == 0) {
        *rank = 0;
    } else {
        *rank = -1; return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_grid_size(void *self, int id, int *size)
{
    int rank;
    if (get_grid_rank(self, id, &rank) == BMI_FAILURE)
        return BMI_FAILURE;

    *size = 1;

    return BMI_SUCCESS;
}


static int
get_var_grid(void *self, const char *name, int *grid)
{
    if (strcmp(name, "sea_surface_water_wave__min_of_increment_of_azimuth_angle_of_opposite_of_phase_velocity") == 0) {
        *grid = 0;
    } else if (strcmp(name, "sea_surface_water_wave__azimuth_angle_of_opposite_of_phase_velocity") == 0) {
        *grid = 0;
    } else if (strcmp(name, "sea_surface_water_wave__mean_of_increment_of_azimuth_angle_of_opposite_of_phase_velocity") == 0) {
        *grid = 0;
    } else if (strcmp(name, "sea_surface_water_wave__max_of_increment_of_azimuth_angle_of_opposite_of_phase_velocity") == 0) {
        *grid = 0;
    } else if (strcmp(name, "sea_surface_water_wave__height") == 0) {
        *grid = 0;
    } else if (strcmp(name, "sea_surface_water_wave__period") == 0) {
        *grid = 0;
    } else if (strcmp(name, "sea_shoreline_wave~incoming~deepwater__ashton_et_al_approach_angle_highness_parameter") == 0) {
        *grid = 0;
    } else if (strcmp(name, "sea_shoreline_wave~incoming~deepwater__ashton_et_al_approach_angle_asymmetry_parameter") == 0) {
        *grid = 0;
    } else {
        *grid = -1; return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_var_type(void *self, const char *name, char *type)
{
    if (strcmp(name, "sea_surface_water_wave__min_of_increment_of_azimuth_angle_of_opposite_of_phase_velocity") == 0) {
        strncpy(type, "float", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_surface_water_wave__azimuth_angle_of_opposite_of_phase_velocity") == 0) {
        strncpy(type, "float", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_surface_water_wave__mean_of_increment_of_azimuth_angle_of_opposite_of_phase_velocity") == 0) {
        strncpy(type, "float", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_surface_water_wave__max_of_increment_of_azimuth_angle_of_opposite_of_phase_velocity") == 0) {
        strncpy(type, "float", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_surface_water_wave__height") == 0) {
        strncpy(type, "float", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_surface_water_wave__period") == 0) {
        strncpy(type, "float", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_shoreline_wave~incoming~deepwater__ashton_et_al_approach_angle_highness_parameter") == 0) {
        strncpy(type, "float", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_shoreline_wave~incoming~deepwater__ashton_et_al_approach_angle_asymmetry_parameter") == 0) {
        strncpy(type, "float", BMI_MAX_UNITS_NAME);
    } else {
        type[0] = '\0'; return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_var_units(void *self, const char *name, char *units)
{
    if (strcmp(name, "sea_surface_water_wave__min_of_increment_of_azimuth_angle_of_opposite_of_phase_velocity") == 0) {
        strncpy(units, "radians", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_surface_water_wave__azimuth_angle_of_opposite_of_phase_velocity") == 0) {
        strncpy(units, "radians", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_surface_water_wave__mean_of_increment_of_azimuth_angle_of_opposite_of_phase_velocity") == 0) {
        strncpy(units, "radians", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_surface_water_wave__max_of_increment_of_azimuth_angle_of_opposite_of_phase_velocity") == 0) {
        strncpy(units, "radians", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_surface_water_wave__height") == 0) {
        strncpy(units, "meters", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_surface_water_wave__period") == 0) {
        strncpy(units, "seconds", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_shoreline_wave~incoming~deepwater__ashton_et_al_approach_angle_highness_parameter") == 0) {
        strncpy(units, "-", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_shoreline_wave~incoming~deepwater__ashton_et_al_approach_angle_asymmetry_parameter") == 0) {
        strncpy(units, "-", BMI_MAX_UNITS_NAME);
    } else {
        units[0] = '\0'; return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_var_itemsize(void *self, const char *name, int *itemsize)
{
    if (strcmp(name, "sea_surface_water_wave__min_of_increment_of_azimuth_angle_of_opposite_of_phase_velocity") == 0) {
        *itemsize = sizeof(float);
    } else if (strcmp(name, "sea_surface_water_wave__azimuth_angle_of_opposite_of_phase_velocity") == 0) {
        *itemsize = sizeof(float);
    } else if (strcmp(name, "sea_surface_water_wave__mean_of_increment_of_azimuth_angle_of_opposite_of_phase_velocity") == 0) {
        *itemsize = sizeof(float);
    } else if (strcmp(name, "sea_surface_water_wave__max_of_increment_of_azimuth_angle_of_opposite_of_phase_velocity") == 0) {
        *itemsize = sizeof(float);
    } else if (strcmp(name, "sea_surface_water_wave__height") == 0) {
        *itemsize = sizeof(float);
    } else if (strcmp(name, "sea_surface_water_wave__period") == 0) {
        *itemsize = sizeof(float);
    } else if (strcmp(name, "sea_shoreline_wave~incoming~deepwater__ashton_et_al_approach_angle_highness_parameter") == 0) {
        *itemsize = sizeof(float);
    } else if (strcmp(name, "sea_shoreline_wave~incoming~deepwater__ashton_et_al_approach_angle_asymmetry_parameter") == 0) {
        *itemsize = sizeof(float);
    } else {
        *itemsize = 0; return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_var_nbytes(void *self, const char *name, int *nbytes)
{
    int id, size, itemsize;

    if (get_var_grid(self, name, &id) == BMI_FAILURE)
        return BMI_FAILURE;

    if (get_grid_size(self, id, &size) == BMI_FAILURE)
        return BMI_FAILURE;

    if (get_var_itemsize(self, name, &itemsize) == BMI_FAILURE)
        return BMI_FAILURE;

    *nbytes = itemsize * size;

    return BMI_SUCCESS;
}


static int
get_value(void *self, const char *name, void *dest)
{
    double *dptr = (double*)dest;

    if (strcmp(name, "sea_surface_water_wave__min_of_increment_of_azimuth_angle_of_opposite_of_phase_velocity") == 0) {
        *dptr = waves_get_wave_angle_min ((WavesModel*)self);
    } else if (strcmp(name, "sea_surface_water_wave__azimuth_angle_of_opposite_of_phase_velocity") == 0) {
        *dptr = waves_get_wave_angle ((WavesModel*)self);
    } else if (strcmp(name, "sea_surface_water_wave__mean_of_increment_of_azimuth_angle_of_opposite_of_phase_velocity") == 0) {
        *dptr =  waves_get_wave_angle_mean ((WavesModel*)self);
    } else if (strcmp(name, "sea_surface_water_wave__max_of_increment_of_azimuth_angle_of_opposite_of_phase_velocity") == 0) {
        *dptr = waves_get_wave_angle_max ((WavesModel*)self);
    } else if (strcmp(name, "sea_surface_water_wave__height") == 0) {
        *dptr = waves_get_height ((WavesModel*)self);
    } else if (strcmp(name, "sea_surface_water_wave__period") == 0) {
        *dptr = waves_get_period ((WavesModel*)self);
    } else {
        return BMI_FAILURE;
    }

    return BMI_SUCCESS;
}


static int
set_value (void *self, const char *name, void *array)
{
    if (strcmp(name, "sea_shoreline_wave~incoming~deepwater__ashton_et_al_approach_angle_asymmetry_parameter") == 0)
      waves_set_angle_asymmetry((WavesModel*)self, *(double*)array);
    else if (strcmp(name, "sea_shoreline_wave~incoming~deepwater__ashton_et_al_approach_angle_highness_parameter") == 0)
      waves_set_angle_highness((WavesModel*)self, *(double*)array);
    else if (strcmp(name, "sea_surface_water_wave__height") == 0)
      waves_set_height((WavesModel*)self, *(double*)array);
    else if (strcmp(name, "sea_surface_water_wave__period") == 0)
      waves_set_period((WavesModel*)self, *(double*)array);

    return BMI_SUCCESS;
}


BMI_Model*
register_bmi_waves(BMI_Model *model)
{
    model->self = NULL;

    model->initialize = initialize;
    model->update = update;
    model->update_until = update_until;
    model->update_frac = update_frac;
    model->finalize = finalize;
    model->run_model = NULL;

    model->get_component_name = get_component_name;
    model->get_input_var_name_count = get_input_var_name_count;
    model->get_output_var_name_count = get_output_var_name_count;
    model->get_input_var_names = get_input_var_names;
    model->get_output_var_names = get_output_var_names;

    model->get_var_grid = get_var_grid;
    model->get_var_type = get_var_type;
    model->get_var_units = get_var_units;
    model->get_var_nbytes = get_var_nbytes;
    model->get_current_time = get_current_time;
    model->get_start_time = get_start_time;
    model->get_end_time = get_end_time;
    model->get_time_units = get_time_units;
    model->get_time_step = get_time_step;

    model->get_value = get_value;
    model->get_value_ptr = NULL;
    model->get_value_at_indices = NULL;

    model->set_value = set_value;
    model->set_value_ptr = NULL;
    model->set_value_at_indices = NULL;

    model->get_grid_rank = get_grid_rank;
    model->get_grid_size = get_grid_size;
    model->get_grid_type = get_grid_type;
    model->get_grid_shape = NULL;
    model->get_grid_spacing = NULL;
    model->get_grid_origin = NULL;

    model->get_grid_x = NULL;
    model->get_grid_y = NULL;
    model->get_grid_z = NULL;

    return model;
}

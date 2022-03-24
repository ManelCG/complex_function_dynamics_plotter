#include <ComplexPlane.h>
#include <image_manipulation.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include <draw_julia.h>

//---Setters and getters
ComplexPlane *complex_plane_new(ComplexPlane **cp){
  ComplexPlane *new = malloc(sizeof(ComplexPlane));
  new->plot = NULL;
  new->drawn_plot = NULL;
  new->pixel_stride = 3;

  new->polynomial = NULL;
  complex_plane_set_function_type(new, 0);

  new->is_drawing_active = true;
  new->is_lines_active = true;

  new->zoom_point1[0] = NULL;
  new->zoom_point1[1] = NULL;
  new->zoom_point2[0] = NULL;
  new->zoom_point2[1] = NULL;

  new->cl = malloc(sizeof(struct OpencL_Program *));
  new->cl->init = false;

  if (cp != NULL){
    *cp = new;
  }
  return new;
}

ComplexPlane *complex_plane_copy(ComplexPlane **dest, ComplexPlane *src){
  ComplexPlane *new = complex_plane_new(NULL);

  complex_plane_set_stride(new, complex_plane_get_stride(src));
  complex_plane_set_dimensions(new,
                               complex_plane_get_width(src),
                               complex_plane_get_height(src));

  complex_plane_set_iterations(new, complex_plane_get_iterations(src));
  complex_plane_set_line_iterations(new, complex_plane_get_line_iterations(src));

  complex_plane_set_center(new, complex_plane_get_center(src));
  complex_plane_set_spanx(new, complex_plane_get_spanx(src));
  complex_plane_set_spany(new, complex_plane_get_spany(src));

  complex_plane_set_plot_type(new, complex_plane_get_plot_type(src, NULL));
  complex_plane_set_quadratic_parameter(new, complex_plane_get_quadratic_parameter(src));

  complex_plane_set_function_type(new, complex_plane_get_function_type(src));

  complex_plane_set_drawing_active(new, complex_plane_is_drawing_active(src));
  complex_plane_set_drawing_lines_active(new, complex_plane_is_drawing_lines_active(src));

  complex_plane_set_polynomial_order(new, complex_plane_get_polynomial_order(src));
  complex_plane_set_polynomial_parameter(new, complex_plane_get_polynomial_parameter(src));
  complex_plane_copy_polynomial(new, src);

  //TODO: Copy zoom_point1 & zoom_point2

  if (dest != NULL){
    *dest = new;
  }
  return new;
}

void complex_plane_set_id(ComplexPlane *cp, int id){
  cp->ID = id;
}
int complex_plane_get_id(ComplexPlane *cp){
  return cp->ID;
}

//--- Dimensions
int complex_plane_set_dimensions(ComplexPlane *cp, int w, int h){
  cp->w = w; cp->h = h; cp->a = w*h;
  return cp->a;
}
int complex_plane_get_width(ComplexPlane *cp){
  return cp->w;
}
int complex_plane_get_height(ComplexPlane *cp){
  return cp->h;
}
int complex_plane_get_area(ComplexPlane *cp){
  return cp->a;
}
void complex_plane_set_stride(ComplexPlane *cp, int s){
  cp->pixel_stride = s;
}
int complex_plane_get_stride(ComplexPlane *cp){
  return cp->pixel_stride;
}
int complex_plane_get_size(ComplexPlane *cp){
  return (complex_plane_get_area(cp) * complex_plane_get_stride(cp));
}

void complex_plane_set_plot_type(ComplexPlane *cp, char *plot_type){
  cp->plot_type = plot_type;
}
char *complex_plane_get_plot_type(ComplexPlane *cp, char **r){
  char *ret = malloc(strlen(cp->plot_type) * sizeof(*cp->plot_type));
  if (r != NULL){
    *r = malloc(strlen(cp->plot_type) * sizeof(*cp->plot_type));
    strcpy(*r, cp->plot_type);
  }
  strcpy(ret, cp->plot_type);
  return ret;
}
void complex_plane_set_function_type(ComplexPlane *cp, int type){
  cp->function_type = type;
  if (type == 0){
    complex_plane_set_polynomial_order(cp, -1);
  }
}
int complex_plane_get_function_type(ComplexPlane *cp){
  return cp->function_type;
}

//---stop_drawing
void complex_plane_set_drawing_active(ComplexPlane *cp, _Bool b){
  cp->is_drawing_active = b;
}
_Bool complex_plane_is_drawing_active(ComplexPlane *cp){
  return cp->is_drawing_active;
}
void complex_plane_set_drawing_lines_active(ComplexPlane *cp, _Bool b){
  cp->is_lines_active = b;
}
_Bool complex_plane_is_drawing_lines_active(ComplexPlane *cp){
  return cp->is_lines_active;
}

//---polynomial
void complex_plane_set_polynomial_order(ComplexPlane *cp, int o){
  complex_plane_free_polynomial(cp);

  cp->polynomial_order = o;
  cp->polynomial_parameter = -1;

  if (o > 0){
    cp->polynomial = malloc(sizeof(complex double) * (o+2));
    for (int i = 0; i <= o+1; i++){
      complex_plane_set_polynomial_member(cp, 0, i);
    }
  }

}
int complex_plane_get_polynomial_order(ComplexPlane *cp){
  return cp->polynomial_order;
}
void complex_plane_set_polynomial_member(ComplexPlane *cp, complex v, int index){
  if (index > cp->polynomial_order + 1){
    return;
  }
  cp->polynomial[index] = v;
}
complex complex_plane_get_polynomial_member(ComplexPlane *cp, int index){
  return cp->polynomial[index];
}
_Bool complex_plane_polynomial_is_null(ComplexPlane *cp){
  return (cp->polynomial == NULL);
}
void complex_plane_free_polynomial(ComplexPlane *cp){
  if (cp->polynomial != NULL){
    free(cp->polynomial);
    cp->polynomial = NULL;
  }
}
int complex_plane_set_polynomial_parameter(ComplexPlane *cp, int p){
  if (p > cp->polynomial_order+1){
    return -1;
  }
  cp->polynomial_parameter = p;
  return 0;
}
int complex_plane_get_polynomial_parameter(ComplexPlane *cp){
  return cp->polynomial_parameter;
}
int complex_plane_copy_polynomial(ComplexPlane *d, ComplexPlane *s){
  int order = complex_plane_get_polynomial_order(s);
  int orderd = complex_plane_get_polynomial_order(d);
  if (order != orderd ||
      complex_plane_polynomial_is_null(d) ||
      complex_plane_polynomial_is_null(s)){
    return -1;
  }
  for (int i = 0; i <= order + 1; i++){
    complex v = complex_plane_get_polynomial_member(s, i);
    complex_plane_set_polynomial_member(d, v, i);
  }
  return 0;
}
const complex double *complex_plane_get_polynomial(ComplexPlane *cp){
  return cp->polynomial;
}


//---iterations
void complex_plane_set_iterations(ComplexPlane *cp, int N){
  cp->N = N;
}
void complex_plane_set_line_iterations(ComplexPlane *cp, int N){
  cp->N_line = N;
}
int complex_plane_get_line_iterations(ComplexPlane *cp){
  return cp->N_line;
}
int complex_plane_get_iterations(ComplexPlane *cp){
  return cp->N;
}

//---Parameters
void complex_plane_set_quadratic_parameter(ComplexPlane *cp, complex double p){
  cp->param = p;
}
void complex_plane_set_quadratic_parameter_real(ComplexPlane *cp, double x){
  cp->param = x + cimag(cp->param)*I;
}
void complex_plane_set_quadratic_parameter_imag(ComplexPlane *cp, double x){
  cp->param = creal(cp->param) + x*I;
}
complex double complex_plane_get_quadratic_parameter(ComplexPlane *cp){
  return(cp->param);
}
double complex_plane_get_quadratic_parameter_real(ComplexPlane *cp){
  return(creal(complex_plane_get_quadratic_parameter(cp)));
}
double complex_plane_get_quadratic_parameter_imag(ComplexPlane *cp){
  return(cimag(complex_plane_get_quadratic_parameter(cp)));
}

//---center
void complex_plane_set_center(ComplexPlane *cp, complex double p){
  cp->center = p;
  complex_plane_set_spanx(cp, complex_plane_get_spanx(cp));
  complex_plane_set_spany(cp, complex_plane_get_spany(cp));
}
void complex_plane_set_center_real(ComplexPlane *cp, double x){
  cp->center = x + cimag(cp->center)*I;
  complex_plane_set_spanx(cp, complex_plane_get_spanx(cp));
  complex_plane_set_spany(cp, complex_plane_get_spany(cp));
}
void complex_plane_set_center_imag(ComplexPlane *cp, double x){
  cp->center = creal(cp->center) + x*I;
  complex_plane_set_spanx(cp, complex_plane_get_spanx(cp));
  complex_plane_set_spany(cp, complex_plane_get_spany(cp));
}
complex double complex_plane_get_center(ComplexPlane *cp){
  return(cp->center);
}
double complex_plane_get_center_real(ComplexPlane *cp){
  return(creal(complex_plane_get_center(cp)));
}
double complex_plane_get_center_imag(ComplexPlane *cp){
  return(cimag(complex_plane_get_center(cp)));
}

//---spans
void complex_plane_set_spanx(ComplexPlane *cp, double s){
  cp->Sx[0] = (-s/2) + complex_plane_get_center_real(cp);
  cp->Sx[1] = (s/2)  + complex_plane_get_center_real(cp);
  cp->SpanX = s;
}
void complex_plane_set_spany(ComplexPlane *cp, double s){
  cp->Sy[0] = (-s/2) + complex_plane_get_center_imag(cp);
  cp->Sy[1] = (s/2)  + complex_plane_get_center_imag(cp);
  cp->SpanY = s;
}
void complex_plane_adjust_span_ratio(ComplexPlane *cp){
  int w = complex_plane_get_width(cp);
  int h = complex_plane_get_height(cp);
  if (w >= h){
    double ratio = (double) w / (double) h;
    complex_plane_set_spanx(cp, complex_plane_get_spany(cp)*ratio);
  } else {
    double ratio = (double) h / (double) w;
    complex_plane_set_spany(cp, complex_plane_get_spanx(cp)*ratio);
  }
}
void complex_plane_set_default_spans(ComplexPlane *cp){
  complex_plane_set_spanx(cp, 3);
  complex_plane_set_spany(cp, 3);
  complex_plane_adjust_span_ratio(cp);
}
double complex_plane_get_spanx(ComplexPlane *cp){
  return cp->SpanX;
}
double complex_plane_get_spany(ComplexPlane *cp){
  return cp->SpanY;
}
double complex_plane_get_spanx0(ComplexPlane *cp){
  return cp->Sx[0];
}
double complex_plane_get_spanx1(ComplexPlane *cp){
  return cp->Sx[1];
}
double complex_plane_get_spany0(ComplexPlane *cp){
  return cp->Sy[0];
}
double complex_plane_get_spany1(ComplexPlane *cp){
  return cp->Sy[1];
}
double *complex_plane_get_spanx_array(ComplexPlane *cp){
  double *Sx;
  Sx = malloc(sizeof *Sx *2);
  Sx[0] = cp->Sx[0]; Sx[1] = cp->Sx[1];
  return Sx;
} //TODO: Possible memory leak.
double *complex_plane_get_spany_array(ComplexPlane *cp){
  double *Sy;
  Sy = malloc(sizeof *Sy *2);
  Sy[0] = cp->Sy[0]; Sy[1] = cp->Sy[1];
  return Sy;
} //TODO: Possible memory leak.

//---drawing box
void complex_plane_set_zoom_point1(ComplexPlane *cp, double x, double y){
  complex_plane_free_zoom_point1(cp);
  cp->zoom_point1[0] = malloc(sizeof(double));
  cp->zoom_point1[1] = malloc(sizeof(double));
  *cp->zoom_point1[0] = x;
  *cp->zoom_point1[1] = y;
}
void complex_plane_set_zoom_point2(ComplexPlane *cp, double x, double y){
  complex_plane_free_zoom_point2(cp);
  cp->zoom_point2[0] = malloc(sizeof(double));
  cp->zoom_point2[1] = malloc(sizeof(double));
  *cp->zoom_point2[0] = x;
  *cp->zoom_point2[1] = y;
}
void complex_plane_free_zoom_point1(ComplexPlane *cp){
  if (cp->zoom_point1[0] != NULL){
  if (cp->zoom_point1[1] != NULL){
    free(cp->zoom_point1[0]);
    free(cp->zoom_point1[1]);
    cp->zoom_point1[0] = NULL;
    cp->zoom_point1[1] = NULL;
  }
  }
}
void complex_plane_free_zoom_point2(ComplexPlane *cp){
  if (cp->zoom_point2[0] != NULL){
  if (cp->zoom_point2[1] != NULL){
    free(cp->zoom_point2[0]);
    free(cp->zoom_point2[1]);
    cp->zoom_point2[0] = NULL;
    cp->zoom_point2[1] = NULL;
  }
  }
}
_Bool complex_plane_zoom_point1_is_null(ComplexPlane *cp){
  if (cp->zoom_point1[0] == NULL && cp->zoom_point1[1] == NULL){
    return true;
  }
  return false;
}
_Bool complex_plane_zoom_point2_is_null(ComplexPlane *cp){
  if (cp->zoom_point2[0] == NULL && cp->zoom_point2[1] == NULL){
    return true;
  }
  return false;
}
void complex_plane_zoom_points_normalize(ComplexPlane *cp){
  double aux;
  if (*cp->zoom_point1[0] > *cp->zoom_point2[0]){
      aux = *cp->zoom_point1[0];
      *cp->zoom_point1[0] = *cp->zoom_point2[0];
      *cp->zoom_point2[0] = aux;
  }
  if (*cp->zoom_point1[1] > *cp->zoom_point2[1]){
    aux = *cp->zoom_point1[1];
    *cp->zoom_point1[1] = *cp->zoom_point2[1];
    *cp->zoom_point2[1] = aux;
  }
}
double complex_plane_zoom_point_get_spanx(ComplexPlane *cp){
  return *cp->zoom_point2[0] - *cp->zoom_point1[0];
}
double complex_plane_zoom_point_get_spany(ComplexPlane *cp){
  return *cp->zoom_point2[1] - *cp->zoom_point1[1];
}
void complex_plane_zoom_points_set_center(ComplexPlane *cp){
    complex_plane_set_center(cp, ((*cp->zoom_point2[0] + *cp->zoom_point1[0])/2) + ((*cp->zoom_point2[1] + *cp->zoom_point1[1])/2)*I);
}
int complex_plane_zoom_point1_get_pixel_value_x(ComplexPlane *cp){
    int w = complex_plane_get_width(cp);
    double spanx = complex_plane_get_spanx(cp);

    return  (int) floor((*cp->zoom_point1[0] - complex_plane_get_spanx0(cp)) / spanx * w);
}
int complex_plane_zoom_point1_get_pixel_value_y(ComplexPlane *cp){
    int h = complex_plane_get_height(cp);
    double spany = complex_plane_get_spany(cp);
    return (int) floor((-(*cp->zoom_point1[1] - 2*complex_plane_get_center_imag(cp))
                          - complex_plane_get_spany0(cp)) / spany * h);
}
int complex_plane_zoom_point2_get_pixel_value_x(ComplexPlane *cp){
    int w = complex_plane_get_width(cp);
    double spanx = complex_plane_get_spanx(cp);

    return  (int) floor((*cp->zoom_point2[0] - complex_plane_get_spanx0(cp)) / spanx * w);
}
int complex_plane_zoom_point2_get_pixel_value_y(ComplexPlane *cp){
    int h = complex_plane_get_height(cp);
    double spany = complex_plane_get_spany(cp);
    return (int) floor((-(*cp->zoom_point2[1] - 2*complex_plane_get_center_imag(cp))
                          - complex_plane_get_spany0(cp)) / spany * h);
}





void complex_plane_set_mandelbrot_parameters(ComplexPlane *cp){
  complex_plane_set_iterations(cp, 500);
  complex_plane_set_line_iterations(cp, 25);
  complex_plane_set_plot_type(cp, "parameter_space");
  complex_plane_set_quadratic_parameter(cp, 0);
  complex_plane_set_center(cp, 0);
  complex_plane_set_default_spans(cp);
  complex_plane_set_stride(cp, 3);
}

//---Plots
void complex_plane_gen_plot(ComplexPlane *cp){
  if (cp == NULL){
     complex_plane_new(&cp);
  }

  double SpanXmin;
  double SpanYmin;
  double ratio;

  complex_plane_free_plot(cp);
  complex_plane_free_drawn_plot(cp);

  #ifdef DEBUG_GUI
  printf("Spans: %f %f | %f %f\n", complex_plane_get_spanx0(cp),
                                   complex_plane_get_spanx1(cp),
                                   complex_plane_get_spany0(cp),
                                   complex_plane_get_spany1(cp));

  #endif

  switch(cp->function_type){
    case 0:   //Simple quadratic family
      cp->plot = draw_julia(complex_plane_get_iterations(cp),
                                                complex_plane_get_height(cp),
                                                complex_plane_get_width(cp),
                                                complex_plane_get_quadratic_parameter(cp),
                                                complex_plane_get_spanx_array(cp),
                                                complex_plane_get_spany_array(cp),
                                                cp->plot_type,
                                                &(cp->cl), !(cp->cl->init));
      break;
    case 1:   //Polynomial function
      if (complex_plane_get_polynomial_order(cp) != -1){
        cp->plot = draw_julia_polynomial(complex_plane_get_iterations(cp),
                                                             complex_plane_get_height(cp),
                                                             complex_plane_get_width(cp),
                                                             complex_plane_get_polynomial_order(cp),
                                                             cp->polynomial,
                                                             complex_plane_get_spanx_array(cp),
                                                             complex_plane_get_spany_array(cp),
                                                             complex_plane_get_polynomial_parameter(cp),
                                                             &(cp->cl), !cp->cl->init);
      }
      break;
    case 2:   //Newton's method
      if (complex_plane_get_polynomial_order(cp) != -1){
        // printf("WIP Newton's method\n");
      }
      break;
  }
}

void complex_plane_gen_thumb(ComplexPlane *cp){
  switch(complex_plane_get_function_type(cp)){
    case 0:
      cp->plot = draw_thumbnail(cp->N,
                                      complex_plane_get_height(cp),
                                      complex_plane_get_width(cp),
                                      complex_plane_get_quadratic_parameter(cp),
                                      complex_plane_get_plot_type(cp, NULL),
                                      &(cp->cl), !(cp->cl->init));
      break;
    case 1:
      if (complex_plane_get_polynomial_order(cp) != -1 && !complex_plane_polynomial_is_null(cp)){

        cp->plot = draw_thumbnail_polynomial(cp->N,
                                                   cp->h,
                                                   cp->w,
                                                   cp->polynomial_order,
                                                   cp->polynomial,
                                                   cp->polynomial_parameter,
                                                   &(cp->cl),
                                                   !cp->cl->init);
      } else {
        complex_plane_alloc_empty_plot(cp);
      }
      break;
  }
}




void complex_plane_alloc_empty_plot(ComplexPlane *cp){
  if (cp->plot != NULL){
    free(cp->plot);
  }
  cp->plot = calloc(complex_plane_get_size(cp) * sizeof(unsigned char), 1);
}
void complex_plane_alloc_drawn_plot(ComplexPlane *cp){
  if (cp->drawn_plot != NULL){
    free(cp->drawn_plot);
  }
  cp->drawn_plot = calloc(complex_plane_get_size(cp) * sizeof(unsigned char), 1);
}
int complex_plane_free_plot(ComplexPlane *cp){
  if (cp->plot != NULL){
    free(cp->plot);
    cp->plot = NULL;
    return 1;
  }
  return 0;
}
int complex_plane_free_drawn_plot(ComplexPlane *cp){
  if (cp->drawn_plot != NULL){
    free(cp->drawn_plot);
    cp->drawn_plot = NULL;
    return 1;
  }
  return 0;
}
const unsigned char *complex_plane_get_plot(ComplexPlane *cp){
  return cp->plot;
}
unsigned char *complex_plane_get_drawn_plot(ComplexPlane *cp){
  return cp->drawn_plot;
}
int complex_plane_copy_plot(ComplexPlane *cp){
  if (cp->drawn_plot == NULL || cp->plot == NULL){
    return -1;
  }
  for (int i = 0; i < complex_plane_get_size(cp); i++){
    cp->drawn_plot[i] = cp->plot[i];
  }
  return 0;
}

double complex_plane_get_norm(complex double z){
   return (pow(pow(creal(z), 2) + pow(cimag(z), 2), 0.5));
}

void draw_sequence_lines(struct ComplexPlane *C, double point[2], int w, int h){
  double c[2], p[2], old_p[2];
  int x, y, oldx, oldy;

  if (strcmp(C->plot_type, "parameter_space") == 0){
    p[0] = creal(C->param);
    p[1] = cimag(C->param);
    c[0] = point[0];
    c[1] = point[1];
    x = (int) floor((p[0]                              - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
    y = (int) floor((-(p[1]-complex_plane_get_center_imag(C) - ((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
  } else if (strcmp(C->plot_type, "rec_f") == 0){
    p[0] = point[0];
    p[1] = point[1];
    c[0] = creal(C->param);
    c[1] = cimag(C->param);
    x = (int) floor((p[0]                              - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
    y = (int) floor((-(p[1]-complex_plane_get_center_imag(C) - ((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
  }

  for (int i = 0; i < C->N_line; i++){
    old_p[0] = p[0]; old_p[1] = p[1];
    oldx = x;        oldy = y;

    double aux = (pow(p[0], 2) - pow(p[1], 2)) + c[0];
    p[1] = 2*p[0]*p[1] + c[1];
    p[0] = aux;

    x = (int) floor((p[0] - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
    y = (int) floor((-(p[1]-complex_plane_get_center_imag(C)-((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);

    draw_line(C->drawn_plot, x, y, oldx, oldy, w, h);
    if (x >= C->w*2 || y >= C->h*2 ||
        x < -C->w   || y < -C->h   ){
      break;
    }
  }
}

complex complex_mul(complex a, complex b){
  return (((creal(a) * creal(b)) - (cimag(a) * cimag(b))) +
          ((creal(a) * cimag(b)) + (cimag(a) * creal(b)))*I);
}

void draw_sequence_lines_polynomial(struct ComplexPlane *C, const complex double *polynomial, int order, double point[2], int w, int h){
  complex param, old_p, c, z;
  int x, y, oldx, oldy;

  int parameter = C->polynomial_parameter;

  param = point[0] + point[1]*I;

  z = polynomial[order + 1];
  c = polynomial[order];

  if (parameter == order + 1){
    x = (int) floor((creal(param)                              - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
    y = (int) floor((-(cimag(param)-complex_plane_get_center_imag(C) - ((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
  } else {
    x = (int) floor((creal(z)                              - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
    y = (int) floor((-(cimag(z)-complex_plane_get_center_imag(C) - ((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
  }

  for (int i = 0; i < C->N_line; i++){
    old_p = z;
    oldx = x; oldy = y;
    complex auxz = 0, aux = 0;

    for (int j = order; j >= 0; j--){
      if (j == order){            //Add c
        if (parameter == order){  //c is my param
          auxz = param;
        } else {                  //param is somewere else
          auxz = polynomial[j];
        }
      } else if (j == order-1){
        if (parameter == order + 1){  //z is param
          if (polynomial[j] != 0){
            auxz += complex_mul(param, polynomial[j]);
          }
        } else if (parameter == j){   //param is a
          auxz += complex_mul(param, z);
        } else {    //param is somewhere else
          if (polynomial[j] != 0){
            auxz += complex_mul(polynomial[j], z);
          }
        }
      } else {
        if (parameter == order + 1) { //param is z
          if (polynomial[j] != 0){
            aux = param;
            for (int k = 0; k < order-j-1; k++){
              aux = complex_mul(aux, param);
            }
            auxz += complex_mul(aux, polynomial[j]);
          }
        } else if (parameter == j){ //parameter is a
          aux = z;
          for (int k = 0; k < order-j-1; k++){
            aux = complex_mul(aux, z);
          }
          auxz += complex_mul(aux, param);
        } else {    //param is somewhere else
          if (polynomial[j] != 0){
            aux = z;
            for (int k = 0; k < order-j-1; k++){
              aux = complex_mul(aux, z);
            }
            auxz += complex_mul(aux, polynomial[j]);
          }
        }
      }
    }
    if (parameter == order + 1){    //z is param
      param = auxz;
    } else {  //z is z
      z = auxz;
    }

    if (parameter == order + 1){
      x = (int) floor((creal(param) - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
      y = (int) floor((-(cimag(param)-complex_plane_get_center_imag(C)-((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
    } else {
      x = (int) floor((creal(z) - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
      y = (int) floor((-(cimag(z)-complex_plane_get_center_imag(C)-((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
    }

    draw_line(C->drawn_plot, x, y, oldx, oldy, w, h);
    if (x >= C->w*2 || y >= C->h*2 ||
        x < -C->w   || y < -C->h   ){
      break;
    }
  }

}

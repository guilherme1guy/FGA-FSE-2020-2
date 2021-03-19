
#ifndef __BME280__
#define __BME280__

/*! CPP guard */
#ifdef __cplusplus
extern "C"
{
#endif

int bme280Init(int iChannel, int iAddr);
int bme280ReadValues(int *T, int *P, int *H);

#ifdef __cplusplus
}
#endif /* End of CPP guard */

#endif // __BME280__
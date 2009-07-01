#ifdef APPLICATION_GPIBBRIDGE

  #ifndef GPIB_H_
    #define GPIB_H_
 
    #ifndef REQUIRES_GBIP
      #error "gpib.h requires GPIB Kernel Module"
    #endif

  #endif /*GPIB_H_*/
#endif

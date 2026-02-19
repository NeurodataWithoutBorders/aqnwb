
#ifndef AQNWB_EXPORT_H
#define AQNWB_EXPORT_H

#ifdef AQNWB_STATIC_DEFINE
#  define AQNWB_EXPORT
#  define AQNWB_NO_EXPORT
#else
#  ifndef AQNWB_EXPORT
#    ifdef aqnwb_aqnwb_EXPORTS
        /* We are building this library */
#      define AQNWB_EXPORT 
#    else
        /* We are using this library */
#      define AQNWB_EXPORT 
#    endif
#  endif

#  ifndef AQNWB_NO_EXPORT
#    define AQNWB_NO_EXPORT 
#  endif
#endif

#ifndef AQNWB_DEPRECATED
#  define AQNWB_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef AQNWB_DEPRECATED_EXPORT
#  define AQNWB_DEPRECATED_EXPORT AQNWB_EXPORT AQNWB_DEPRECATED
#endif

#ifndef AQNWB_DEPRECATED_NO_EXPORT
#  define AQNWB_DEPRECATED_NO_EXPORT AQNWB_NO_EXPORT AQNWB_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef AQNWB_NO_DEPRECATED
#    define AQNWB_NO_DEPRECATED
#  endif
#endif

#endif /* AQNWB_EXPORT_H */

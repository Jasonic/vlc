/*****************************************************************************
 * modules.h : Module management functions.
 *****************************************************************************
 * Copyright (C) 2001 VideoLAN
 * $Id: modules.h,v 1.34 2001/12/07 16:47:47 jobi Exp $
 *
 * Authors: Samuel Hocevar <sam@zoy.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/

/*****************************************************************************
 * module_bank_t, p_module_bank (global variable)
 *****************************************************************************
 * This global variable is accessed by any function using modules.
 *****************************************************************************/
typedef struct
{
    struct module_s *   first; /* First module of the bank */

    vlc_mutex_t         lock;  /* Global lock -- you can't imagine how awful it
                                  is to design thread-safe linked lists. */
} module_bank_t;

extern module_bank_t *p_module_bank;

/*****************************************************************************
 * Module #defines.
 *****************************************************************************/

/* Number of tries before we unload an unused module */
#define MODULE_HIDE_DELAY 10000

/* The module handle type. */
#ifdef SYS_BEOS
typedef int     module_handle_t;
#else
typedef void *  module_handle_t;
#endif

/*****************************************************************************
 * Module capabilities.
 *****************************************************************************/

#define MODULE_CAPABILITY_NULL     0        /* The Module can't do anything */
#define MODULE_CAPABILITY_INTF     1 <<  0  /* Interface */
#define MODULE_CAPABILITY_ACCESS   1 <<  1  /* Input */
#define MODULE_CAPABILITY_INPUT    1 <<  2  /* Input */
#define MODULE_CAPABILITY_DECAPS   1 <<  3  /* Decaps */
#define MODULE_CAPABILITY_DEC      1 <<  4  /* Video decoder */
#define MODULE_CAPABILITY_MOTION   1 <<  5  /* Motion compensation */
#define MODULE_CAPABILITY_IDCT     1 <<  6  /* IDCT transformation */
#define MODULE_CAPABILITY_AOUT     1 <<  7  /* Audio output */
#define MODULE_CAPABILITY_VOUT     1 <<  8  /* Video output */
#define MODULE_CAPABILITY_YUV      1 <<  9  /* YUV colorspace conversion */
#define MODULE_CAPABILITY_IMDCT    1 << 10  /* IMDCT transformation */
#define MODULE_CAPABILITY_DOWNMIX  1 << 11  /* AC3 downmix */
#define MODULE_CAPABILITY_MEMCPY   1 << 12  /* memcpy */

/* FIXME: kludge */
struct input_area_s;
struct imdct_s;
struct complex_s;
struct dm_par_s;
struct bit_stream_s;
struct decoder_fifo_s;

struct decoder_config_s;

/* FIXME: not yet used */
typedef struct probedata_s
{
    u8  i_type;
    struct
    {
        char * psz_data;
    } aout;
} probedata_t;

/* FIXME: find a nicer way to do this. */
typedef struct function_list_s
{
    int ( * pf_probe ) ( probedata_t * p_data );

    union
    {
        /* Interface plugin */
        struct
        {
            int  ( * pf_open ) ( struct intf_thread_s * );
            void ( * pf_close )( struct intf_thread_s * );
            void ( * pf_run )  ( struct intf_thread_s * );
        } intf;

        /* Input plugin */
        struct
        {
            void ( * pf_init ) ( struct input_thread_s * );
            void ( * pf_open ) ( struct input_thread_s * );
            void ( * pf_close )( struct input_thread_s * );
            void ( * pf_end )  ( struct input_thread_s * );
            void ( * pf_init_bit_stream ) ( struct bit_stream_s *,
                                            struct decoder_fifo_s *,
                      void (* pf_bitstream_callback)( struct bit_stream_s *,
                                                      boolean_t ),
                                           void * );

            int  ( * pf_read ) ( struct input_thread_s *,
                                 struct data_packet_s *
                                        pp_packets[] );
            void ( * pf_demux )( struct input_thread_s *,
                                 struct data_packet_s * );

            struct data_packet_s * ( * pf_new_packet ) ( void *, size_t );
            struct pes_packet_s *  ( * pf_new_pes )    ( void * );
            void ( * pf_delete_packet )  ( void *, struct data_packet_s * );
            void ( * pf_delete_pes )     ( void *, struct pes_packet_s * );

            int  ( * pf_set_program ) ( struct input_thread_s *,
                                     struct pgrm_descriptor_s * );

            int  ( * pf_set_area ) ( struct input_thread_s *,
                                     struct input_area_s * );
            int  ( * pf_rewind )   ( struct input_thread_s * );
            void ( * pf_seek )     ( struct input_thread_s *, off_t );
        } input;

        /* Audio output plugin */
        struct
        {
            int  ( * pf_open )       ( struct aout_thread_s * );
            int  ( * pf_setformat )  ( struct aout_thread_s * );
            long ( * pf_getbufinfo ) ( struct aout_thread_s *, long );
            void ( * pf_play )       ( struct aout_thread_s *, byte_t *, int );
            void ( * pf_close )      ( struct aout_thread_s * );
        } aout;

        /* Video output plugin */
        struct
        {
            int  ( * pf_create )     ( struct vout_thread_s * );
            int  ( * pf_init )       ( struct vout_thread_s * );
            void ( * pf_end )        ( struct vout_thread_s * );
            void ( * pf_destroy )    ( struct vout_thread_s * );
            int  ( * pf_manage )     ( struct vout_thread_s * );
            void ( * pf_display )    ( struct vout_thread_s * );
            void ( * pf_setpalette ) ( struct vout_thread_s *, u16 *red,
                                       u16 *green, u16 *blue, u16 *transp );
        } vout;

        /* Motion compensation plugin */
        struct
        {
            void ( * ppppf_motion[2][2][4] ) ( yuv_data_t *, yuv_data_t *,
                                               int, int );
        } motion;

        /* IDCT plugin */
        struct
        {
            void ( * pf_idct_init )    ( void ** );
            void ( * pf_sparse_idct_add )( dctelem_t *, yuv_data_t *, int,
                                         void *, int );
            void ( * pf_idct_add )     ( dctelem_t *, yuv_data_t *, int,
                                         void *, int );
            void ( * pf_sparse_idct_copy )( dctelem_t *, yuv_data_t *, int,
                                         void *, int );
            void ( * pf_idct_copy )    ( dctelem_t *, yuv_data_t *, int,
                                         void *, int );
            void ( * pf_norm_scan )    ( u8 ppi_scan[2][64] );
        } idct;

        /* YUV transformation plugin */
        struct
        {
            int  ( * pf_init )         ( struct vout_thread_s * );
            int  ( * pf_reset )        ( struct vout_thread_s * );
            void ( * pf_end )          ( struct vout_thread_s * );
        } yuv;

        /* IMDCT plugin */
        struct
        {
            void ( * pf_imdct_init )   ( struct imdct_s * );
            void ( * pf_imdct_256 )    ( struct imdct_s *,
                                         float data[], float delay[] );
            void ( * pf_imdct_256_nol )( struct imdct_s *,
                                         float data[], float delay[] );
            void ( * pf_imdct_512 )    ( struct imdct_s *,
                                         float data[], float delay[] );
            void ( * pf_imdct_512_nol )( struct imdct_s *,
                                         float data[], float delay[] );
//            void ( * pf_fft_64p )      ( struct complex_s * );

        } imdct;

        /* AC3 downmix plugin */
        struct
        {
            void ( * pf_downmix_3f_2r_to_2ch ) ( float *, struct dm_par_s * );
            void ( * pf_downmix_3f_1r_to_2ch ) ( float *, struct dm_par_s * );
            void ( * pf_downmix_2f_2r_to_2ch ) ( float *, struct dm_par_s * );
            void ( * pf_downmix_2f_1r_to_2ch ) ( float *, struct dm_par_s * );
            void ( * pf_downmix_3f_0r_to_2ch ) ( float *, struct dm_par_s * );
            void ( * pf_stream_sample_2ch_to_s16 ) ( s16 *, float *, float * );
            void ( * pf_stream_sample_1ch_to_s16 ) ( s16 *, float * );

        } downmix;

        /* Decoder plugins */
        struct
        {
            int  ( * pf_run ) ( struct decoder_config_s * p_config );
        } dec;

        /* memcpy plugins */
        struct
        {
            void* ( * pf_fast_memcpy ) ( void *, const void *, size_t );
        } memcpy;

    } functions;

} function_list_t;

typedef struct module_functions_s
{
    /* XXX: The order here has to be the same as above for the #defines */
    function_list_t intf;
    function_list_t access;
    function_list_t input;
    function_list_t decaps;
    function_list_t dec;
    function_list_t motion;
    function_list_t idct;
    function_list_t aout;
    function_list_t vout;
    function_list_t yuv;
    function_list_t imdct;
    function_list_t downmix;
    function_list_t memcpy;

} module_functions_t;

typedef struct module_functions_s * p_module_functions_t;

/*****************************************************************************
 * Macros used to build the configuration structure.
 *****************************************************************************/

/* Mandatory first and last parts of the structure */
#define MODULE_CONFIG_ITEM_START       0xdead  /* The main window */
#define MODULE_CONFIG_ITEM_END         0xbeef  /* End of the window */

/* Configuration widgets */
#define MODULE_CONFIG_ITEM_WINDOW      0x0001  /* The main window */
#define MODULE_CONFIG_ITEM_PANE        0x0002  /* A notebook pane */
#define MODULE_CONFIG_ITEM_FRAME       0x0003  /* A frame */
#define MODULE_CONFIG_ITEM_COMMENT     0x0004  /* A comment text */
#define MODULE_CONFIG_ITEM_STRING      0x0005  /* A string */
#define MODULE_CONFIG_ITEM_FILE        0x0006  /* A file selector */
#define MODULE_CONFIG_ITEM_CHECK       0x0007  /* A checkbox */
#define MODULE_CONFIG_ITEM_CHOOSE      0x0008  /* A choose box */
#define MODULE_CONFIG_ITEM_RADIO       0x0009  /* A radio box */
#define MODULE_CONFIG_ITEM_SCALE       0x000a  /* A horizontal ruler */
#define MODULE_CONFIG_ITEM_SPIN        0x000b  /* A numerical selector */

typedef struct module_config_s
{
    int         i_type;                         /* Configuration widget type */
    char *      psz_text;        /* Text commenting or describing the widget */
    char *      psz_name;                                   /* Variable name */
    void *      p_getlist;          /* Function to call to get a choice list */
    void *      p_change;        /* Function to call when commiting a change */
} module_config_t;

/*****************************************************************************
 * Bank and module description structures
 *****************************************************************************/

/* The module description structure */
typedef struct module_s
{
    boolean_t           b_builtin;  /* Set to true if the module is built in */

    union
    {
        struct
        {
            module_handle_t     handle;                     /* Unique handle */
            char *              psz_filename;             /* Module filename */

        } plugin;

        struct
        {
            int ( *pf_deactivate ) ( struct module_s * );

        } builtin;

    } is;

    char *              psz_name;                    /* Module _unique_ name */
    char *              psz_longname;             /* Module descriptive name */
    char *              psz_version;                       /* Module version */

    int                 i_usage;                        /* Reference counter */
    int                 i_unused_delay;    /* Delay until module is unloaded */

    struct module_s *   next;                                 /* Next module */
    struct module_s *   prev;                             /* Previous module */

    module_config_t         *p_config;     /* Module configuration structure */
    struct module_symbols_s *p_symbols;

    u32                      i_capabilities;              /* Capability list */
    p_module_functions_t     p_functions;            /* Capability functions */

} module_t;

/*****************************************************************************
 * Exported functions.
 *****************************************************************************/
void            module_InitBank     ( void );
void            module_EndBank      ( void );
void            module_ResetBank    ( void );
void            module_ManageBank   ( void );
module_t *      module_Need         ( int i_capabilities, void *p_data );
void            module_Unneed       ( module_t * p_module );


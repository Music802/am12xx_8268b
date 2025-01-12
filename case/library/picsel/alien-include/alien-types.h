/**
 * Structures and types used by interfaces between the Picsel library and
 * the Alien application.
 *
 * @file
 * $Id: alien-types.h,v 1.36 2012/08/01 09:21:20 hugh Exp $
 */
/* Copyright (C) Picsel, 2007-2008. All Rights Reserved. */
/**
 * @defgroup TgvCoreTypes Types for TGV Core
 * @ingroup TgvCore
 *
 * Structure and type definitions shared by interfaces between the Picsel
 * library and the Alien application.
 *
 * @{
 */

#ifndef ALIEN_TYPES_H
#define ALIEN_TYPES_H


typedef struct Alien_Stream Alien_Stream;
typedef struct Picsel_Stream Picsel_Stream;



/**
 * An opaque pointer to global data that the Alien application
 * may wish to store. The Picsel library will pass this back in many Alien*
 * calls, for example AlienConfig_ready(). The Picsel library will
 * not access the memory it points to, so it is offered as a convenience
 * for the application integrator. There is only one value for this,
 * which is set by the Alien application before calling PicselApp_start().
 */
typedef struct Alien_Context Alien_Context;


/**
 * @c Picsel_Context contains data used by the Picsel library which is
 * effectively global, but is only accessed through this pointer, to avoid
 * using actual global variables, which are not allowed on some device
 * operating systems. It will be needed as a parameter to most Picsel*
 * functions. The Alien application should store the pointer, but never
 * read or write the data it points to. It is set
 * by AlienEvent_setPicselContext(), which will be called before the value
 * is needed.
 */
typedef struct Picsel_Context Picsel_Context;


/**
 * An opaque alien type to be implemented in the Alien application and
 * returned by the initialisation function passed to PicselAntiVirus_register().
 * This data type should store information that allows the alien application
 * to access AntiVirus contextual data when Picsel provides this type via
 * the AntiVirus alien requests. This structures should be defined by
 * the Alien application integrator.
 *
 * @see @ref PicselUserRequest_Type_AntiVirus_Content,
 *      @ref PicselUserRequest_Type_AntiVirus_URL.
 */
typedef struct Alien_AntiVirus_Context Alien_AntiVirus_Context;


/**
  * An opaque structure owned by the Picsel library which is passed to
  * the Alien application when the Picsel library requires that a certain
  * view can be later identified. The Alien application needs to store
  * the pointer to this structure and return it to Picsel in another
  * function call. The Alien application must not access the data within it.
  *
  * @ingroup TgvMultipleViews
  */
typedef struct Picsel_View Picsel_View;


/**
  * An opaque structure owned by the Picsel library which is passed to
  * the Alien application when the Picsel library requires that a certain
  * download can be later identified. The Alien application needs to store
  * the pointer to this structure and return it to Picsel in another
  * function call. The Alien application must not access the data within it.
  *
  * @ingroup TgvContentInformation
  */
typedef struct Picsel_Download Picsel_Download;


/**
  * Type to indicate how to handle a file save request. It is used
  * when the Alien application is in the image navigation mode.
  *
  * @see PicselFocus_setNavigationMode()
  */
typedef enum PicselSaveType
{
    /** Request to save a document */
    PicselSaveType_Document,

    /** Request to save an image embedded within a document.
     * The target save image is determined by the image highlighted
     * by the user while in the image navigation mode.  For this,
     * the Alien application must have already called
     * PicselFocus_setNavigationMode() that allows the user
     * to navigate to an image. */
    PicselSaveType_Image,

    /** Request to save the background image of the current page. */
    PicselSaveType_BackgroundImage
}
PicselSaveType;

/**
 * Indicates the type of the content associated with an HTTP status.
 * It is used when the Alien application is in the image navigation mode.
 *
 * @see PicselFocus_setNavigationMode().
 * @see @ref AlienInformation_HttpStatus
 *
 * @ingroup TgvContentInformation
 */
typedef enum PicselContentType
{
    /** The main document */
    PicselContentType_MainDocument = (1<<16),

    /** A frame - a subdocument of the main document */
    PicselContentType_FrameDocument,

    /** Embedded content - Image, JS, CSS, etc. */
    PicselContentType_EmbeddedContent
}
PicselContentType;

/**
 * A list of character encodings used by Picsel library for
 * displaying the document. The character encoding should be
 * set by the Alien application.
 *
 * @see @ref PicselConfig_defaultCharSet
 * @ingroup TgvLanguageInit
 */
typedef enum PicselConfig_CharSet
{
    /** Character set "ISO-8859-1" - Western */
    PicselConfig_CharSet_Iso88591 = (1<<16),

    /** Character set "Big5" - Traditional Chinese */
    PicselConfig_CharSet_Big5,

    /** Character set "GBK" - Simplified Chinese */
    PicselConfig_CharSet_Gbk,

    /** Character set "Shift_JIS" - Japanese */
    PicselConfig_CharSet_ShiftJis,

    /** Character set "ISO-2022-JP" - Japanese */
    PicselConfig_CharSet_Iso2022Jp,

    /** Character set "EUC-JP" - Japanese */
    PicselConfig_CharSet_EucJp,

    /** Character set "WINDOWS-949" - Korean */
    PicselConfig_CharSet_Windows949,

    /** Character set "UTF-8" - Universal alphabet */
    PicselConfig_CharSet_Utf8,

    /** Character set "WINDOWS-1251" - Cyrillic */
    PicselConfig_CharSet_Win1251,

    /** Character set "KOI8" - Cyrillic */
    PicselConfig_CharSet_Koi8,

    /** Character set "WINDOWS-866" - Cyrillic */
    PicselConfig_CharSet_Win866,

    /** Character set "ISO-8859-5" - Cyrillic */
    PicselConfig_CharSet_Iso88595,

    /** Character set "ISO-8859-7" - Greek */
    PicselConfig_CharSet_Iso88597,

    /**  Unsupported character set */
    PicselConfig_CharSet_Unsupported
}
PicselConfig_CharSet;

/**
 * An identifier of the embedded Picsel application which can run
 * within the same context as another. The handle is provided in
 * @ref AlienInformation_AppHandle.appHandle which is sent with
 * the @ref AlienInformation_InitComplete event.
 *
 */
typedef void Picsel_AppHandle;

/**
 * A timestamp used by Picsel library for tracking changes of the content
 * over time.
 *
 * @implement All timestamps must be relative to GMT (UTC+0) and
 * the 'year' field is absolute.
 */
typedef struct Picsel_Timestamp
{
    unsigned int  year;             /**< Absolute (ie '2008' not '8') */
    unsigned char month;            /**< 1-12 */
    unsigned char day;              /**< 1-31 */
    unsigned char hour;             /**< 0-23 */
    unsigned char minute;           /**< 0-59 */
    unsigned char second;           /**< 0-59 */
    unsigned char unused[3];        /**< Set to zeroes */
}
Picsel_Timestamp;

/**
 *  This structure contains a pointer to unsigned char string pointers
 *  and the number of pointers assigned. It is used for storing names,
 *  URLs and other string values.
 *
 *  @ingroup TgvContactsDatabase
 */
typedef struct PicselStringArray
{
    /** Number of pointers to strings */
    unsigned int    nStrings;

    /** Array of pointers to strings  */
    unsigned char **ptrStrings;
}
PicselStringArray;

/**
 * @}
 */

#endif /* !ALIEN_TYPES_H */

/*
   +----------------------------------------------------------------------+
   | PHP Version 7                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) The PHP Group                                          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Rasmus Lerdorf <rasmus@php.net>                             |
   |          Stig Bakken <ssb@php.net>                                   |
   |          Jim Winstead <jimw@php.net>                                 |
   +----------------------------------------------------------------------+
 */

/* gd 1.2 is copyright 1994, 1995, Quest Protein Database Center,
   Cold Spring Harbor Labs. */

/* Note that there is no code from the gd package in this file */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/head.h"
#include <math.h>
#include "SAPI.h"
#include "php_gd.h"
#include "ext/standard/info.h"
#include "php_open_temporary_file.h"


#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef PHP_WIN32
# include <io.h>
# include <fcntl.h>
# include <windows.h>
# include <Winuser.h>
# include <Wingdi.h>
#endif

#if defined(HAVE_GD_XPM) && defined(HAVE_GD_BUNDLED)
# include <X11/xpm.h>
#endif

# include "gd_compat.h"


static int le_gd, le_gd_font;

#include <gd.h>
#include <gd_errors.h>
#include <gdfontt.h>  /* 1 Tiny font */
#include <gdfonts.h>  /* 2 Small font */
#include <gdfontmb.h> /* 3 Medium bold font */
#include <gdfontl.h>  /* 4 Large font */
#include <gdfontg.h>  /* 5 Giant font */

#if defined(HAVE_GD_FREETYPE) && defined(HAVE_GD_BUNDLED)
# include <ft2build.h>
# include FT_FREETYPE_H
#endif

#if defined(HAVE_GD_XPM) && defined(HAVE_GD_BUNDLED)
# include "X11/xpm.h"
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef HAVE_GD_FREETYPE
static void php_imagettftext_common(INTERNAL_FUNCTION_PARAMETERS, int, int);
#endif

#include "gd_ctx.c"
#include "gd_arginfo.h"

/* as it is not really public, duplicate declaration here to avoid
   pointless warnings */
int overflow2(int a, int b);

/* Section Filters Declarations */
/* IMPORTANT NOTE FOR NEW FILTER
 * Do not forget to update:
 * IMAGE_FILTER_MAX: define the last filter index
 * IMAGE_FILTER_MAX_ARGS: define the biggest amount of arguments
 * image_filter array in PHP_FUNCTION(imagefilter)
 * */
#define IMAGE_FILTER_NEGATE         0
#define IMAGE_FILTER_GRAYSCALE      1
#define IMAGE_FILTER_BRIGHTNESS     2
#define IMAGE_FILTER_CONTRAST       3
#define IMAGE_FILTER_COLORIZE       4
#define IMAGE_FILTER_EDGEDETECT     5
#define IMAGE_FILTER_EMBOSS         6
#define IMAGE_FILTER_GAUSSIAN_BLUR  7
#define IMAGE_FILTER_SELECTIVE_BLUR 8
#define IMAGE_FILTER_MEAN_REMOVAL   9
#define IMAGE_FILTER_SMOOTH         10
#define IMAGE_FILTER_PIXELATE       11
#define IMAGE_FILTER_SCATTER		12
#define IMAGE_FILTER_MAX            12
#define IMAGE_FILTER_MAX_ARGS       6
static void php_image_filter_negate(INTERNAL_FUNCTION_PARAMETERS);
static void php_image_filter_grayscale(INTERNAL_FUNCTION_PARAMETERS);
static void php_image_filter_brightness(INTERNAL_FUNCTION_PARAMETERS);
static void php_image_filter_contrast(INTERNAL_FUNCTION_PARAMETERS);
static void php_image_filter_colorize(INTERNAL_FUNCTION_PARAMETERS);
static void php_image_filter_edgedetect(INTERNAL_FUNCTION_PARAMETERS);
static void php_image_filter_emboss(INTERNAL_FUNCTION_PARAMETERS);
static void php_image_filter_gaussian_blur(INTERNAL_FUNCTION_PARAMETERS);
static void php_image_filter_selective_blur(INTERNAL_FUNCTION_PARAMETERS);
static void php_image_filter_mean_removal(INTERNAL_FUNCTION_PARAMETERS);
static void php_image_filter_smooth(INTERNAL_FUNCTION_PARAMETERS);
static void php_image_filter_pixelate(INTERNAL_FUNCTION_PARAMETERS);
static void php_image_filter_scatter(INTERNAL_FUNCTION_PARAMETERS);

/* End Section filters declarations */
static gdImagePtr _php_image_create_from_string(zend_string *Data, char *tn, gdImagePtr (*ioctx_func_p)());
static void _php_image_create_from(INTERNAL_FUNCTION_PARAMETERS, int image_type, char *tn, gdImagePtr (*func_p)(), gdImagePtr (*ioctx_func_p)());
static void _php_image_output(INTERNAL_FUNCTION_PARAMETERS, int image_type, char *tn, void (*func_p)());
static int _php_image_type(char data[12]);

/* {{{ gd_functions[]
 */
static const zend_function_entry gd_functions[] = {
	PHP_FE(gd_info,                                 arginfo_gd_info)
	PHP_FE(imagearc,								arginfo_imagearc)
	PHP_FE(imageellipse,							arginfo_imageellipse)
	PHP_FE(imagechar,								arginfo_imagechar)
	PHP_FE(imagecharup,								arginfo_imagecharup)
	PHP_FE(imagecolorat,							arginfo_imagecolorat)
	PHP_FE(imagecolorallocate,						arginfo_imagecolorallocate)
	PHP_FE(imagepalettecopy,						arginfo_imagepalettecopy)
	PHP_FE(imagecreatefromstring,					arginfo_imagecreatefromstring)
	PHP_FE(imagecolorclosest,						arginfo_imagecolorclosest)
	PHP_FE(imagecolorclosesthwb,					arginfo_imagecolorclosesthwb)
	PHP_FE(imagecolordeallocate,					arginfo_imagecolordeallocate)
	PHP_FE(imagecolorresolve,						arginfo_imagecolorresolve)
	PHP_FE(imagecolorexact,							arginfo_imagecolorexact)
	PHP_FE(imagecolorset,							arginfo_imagecolorset)
	PHP_FE(imagecolortransparent,					arginfo_imagecolortransparent)
	PHP_FE(imagecolorstotal,						arginfo_imagecolorstotal)
	PHP_FE(imagecolorsforindex,						arginfo_imagecolorsforindex)
	PHP_FE(imagecopy,								arginfo_imagecopy)
	PHP_FE(imagecopymerge,							arginfo_imagecopymerge)
	PHP_FE(imagecopymergegray,						arginfo_imagecopymergegray)
	PHP_FE(imagecopyresized,						arginfo_imagecopyresized)
	PHP_FE(imagecreate,								arginfo_imagecreate)
	PHP_FE(imagecreatetruecolor,					arginfo_imagecreatetruecolor)
	PHP_FE(imageistruecolor,						arginfo_imageistruecolor)
	PHP_FE(imagetruecolortopalette,					arginfo_imagetruecolortopalette)
	PHP_FE(imagepalettetotruecolor,					arginfo_imagepalettetotruecolor)
	PHP_FE(imagesetthickness,						arginfo_imagesetthickness)
	PHP_FE(imagefilledarc,							arginfo_imagefilledarc)
	PHP_FE(imagefilledellipse,						arginfo_imagefilledellipse)
	PHP_FE(imagealphablending,						arginfo_imagealphablending)
	PHP_FE(imagesavealpha,							arginfo_imagesavealpha)
	PHP_FE(imagecolorallocatealpha,					arginfo_imagecolorallocatealpha)
	PHP_FE(imagecolorresolvealpha, 					arginfo_imagecolorresolvealpha)
	PHP_FE(imagecolorclosestalpha,					arginfo_imagecolorclosestalpha)
	PHP_FE(imagecolorexactalpha,					arginfo_imagecolorexactalpha)
	PHP_FE(imagecopyresampled,						arginfo_imagecopyresampled)

#ifdef PHP_WIN32
	PHP_FE(imagegrabwindow,							arginfo_imagegrabwindow)
	PHP_FE(imagegrabscreen,							arginfo_imagegrabscreen)
#endif

	PHP_FE(imagerotate,     						arginfo_imagerotate)
	PHP_FE(imageflip,								arginfo_imageflip)

	PHP_FE(imageantialias,							arginfo_imageantialias)
	PHP_FE(imagecrop,								arginfo_imagecrop)
	PHP_FE(imagecropauto,							arginfo_imagecropauto)
	PHP_FE(imagescale,								arginfo_imagescale)
	PHP_FE(imageaffine,								arginfo_imageaffine)
	PHP_FE(imageaffinematrixconcat,					arginfo_imageaffinematrixconcat)
	PHP_FE(imageaffinematrixget,					arginfo_imageaffinematrixget)
	PHP_FE(imagesetinterpolation,                   arginfo_imagesetinterpolation)
	PHP_FE(imagesettile,							arginfo_imagesettile)
	PHP_FE(imagesetbrush,							arginfo_imagesetbrush)
	PHP_FE(imagesetstyle,							arginfo_imagesetstyle)

#ifdef HAVE_GD_PNG
	PHP_FE(imagecreatefrompng,						arginfo_imagecreatefrompng)
#endif
#ifdef HAVE_GD_WEBP
	PHP_FE(imagecreatefromwebp,						arginfo_imagecreatefromwebp)
#endif
	PHP_FE(imagecreatefromgif,						arginfo_imagecreatefromgif)
#ifdef HAVE_GD_JPG
	PHP_FE(imagecreatefromjpeg,						arginfo_imagecreatefromjpeg)
#endif
	PHP_FE(imagecreatefromwbmp,						arginfo_imagecreatefromwbmp)
	PHP_FE(imagecreatefromxbm,						arginfo_imagecreatefromxbm)
#if defined(HAVE_GD_XPM)
	PHP_FE(imagecreatefromxpm,						arginfo_imagecreatefromxpm)
#endif
	PHP_FE(imagecreatefromgd,						arginfo_imagecreatefromgd)
	PHP_FE(imagecreatefromgd2,						arginfo_imagecreatefromgd2)
	PHP_FE(imagecreatefromgd2part,					arginfo_imagecreatefromgd2part)
#ifdef HAVE_GD_BMP
	PHP_FE(imagecreatefrombmp,						arginfo_imagecreatefrombmp)
#endif
#ifdef HAVE_GD_TGA
	PHP_FE(imagecreatefromtga,						arginfo_imagecreatefromtga)
#endif
#ifdef HAVE_GD_PNG
	PHP_FE(imagepng,								arginfo_imagepng)
#endif
#ifdef HAVE_GD_WEBP
	PHP_FE(imagewebp,								arginfo_imagewebp)
#endif
	PHP_FE(imagegif,								arginfo_imagegif)
#ifdef HAVE_GD_JPG
	PHP_FE(imagejpeg,								arginfo_imagejpeg)
#endif
	PHP_FE(imagewbmp,                               arginfo_imagewbmp)
	PHP_FE(imagegd,									arginfo_imagegd)
	PHP_FE(imagegd2,								arginfo_imagegd2)
#ifdef HAVE_GD_BMP
	PHP_FE(imagebmp,								arginfo_imagebmp)
#endif

	PHP_FE(imagedestroy,							arginfo_imagedestroy)
	PHP_FE(imagegammacorrect,						arginfo_imagegammacorrect)
	PHP_FE(imagefill,								arginfo_imagefill)
	PHP_FE(imagefilledpolygon,						arginfo_imagefilledpolygon)
	PHP_FE(imagefilledrectangle,					arginfo_imagefilledrectangle)
	PHP_FE(imagefilltoborder,						arginfo_imagefilltoborder)
	PHP_FE(imagefontwidth,							arginfo_imagefontwidth)
	PHP_FE(imagefontheight,							arginfo_imagefontheight)
	PHP_FE(imageinterlace,							arginfo_imageinterlace)
	PHP_FE(imageline,								arginfo_imageline)
	PHP_FE(imageloadfont,							arginfo_imageloadfont)
	PHP_FE(imagepolygon,							arginfo_imagepolygon)
	PHP_FE(imageopenpolygon,						arginfo_imageopenpolygon)
	PHP_FE(imagerectangle,							arginfo_imagerectangle)
	PHP_FE(imagesetpixel,							arginfo_imagesetpixel)
	PHP_FE(imagestring,								arginfo_imagestring)
	PHP_FE(imagestringup,							arginfo_imagestringup)
	PHP_FE(imagesx,									arginfo_imagesx)
	PHP_FE(imagesy,									arginfo_imagesy)
	PHP_FE(imagesetclip,							arginfo_imagesetclip)
	PHP_FE(imagegetclip,							arginfo_imagegetclip)
	PHP_FE(imagedashedline,							arginfo_imagedashedline)

#ifdef HAVE_GD_FREETYPE
	PHP_FE(imagettfbbox,							arginfo_imagettfbbox)
	PHP_FE(imagettftext,							arginfo_imagettftext)
	PHP_FE(imageftbbox,								arginfo_imageftbbox)
	PHP_FE(imagefttext,								arginfo_imagefttext)
#endif

	PHP_FE(imagetypes,								arginfo_imagetypes)

	PHP_FE(imagelayereffect,						arginfo_imagelayereffect)
	PHP_FE(imagexbm,                                arginfo_imagexbm)

	PHP_FE(imagecolormatch,							arginfo_imagecolormatch)

/* gd filters */
	PHP_FE(imagefilter,     						arginfo_imagefilter)
	PHP_FE(imageconvolution,						arginfo_imageconvolution)

	PHP_FE(imageresolution,							arginfo_imageresolution)

	PHP_FE_END
};
/* }}} */

zend_module_entry gd_module_entry = {
	STANDARD_MODULE_HEADER,
	"gd",
	gd_functions,
	PHP_MINIT(gd),
	PHP_MSHUTDOWN(gd),
	NULL,
	PHP_RSHUTDOWN(gd),
	PHP_MINFO(gd),
	PHP_GD_VERSION,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_GD
ZEND_GET_MODULE(gd)
#endif

/* {{{ PHP_INI_BEGIN */
PHP_INI_BEGIN()
	PHP_INI_ENTRY("gd.jpeg_ignore_warning", "1", PHP_INI_ALL, NULL)
PHP_INI_END()
/* }}} */

/* {{{ php_free_gd_image
 */
static void php_free_gd_image(zend_resource *rsrc)
{
	gdImageDestroy((gdImagePtr) rsrc->ptr);
}
/* }}} */

/* {{{ php_free_gd_font
 */
static void php_free_gd_font(zend_resource *rsrc)
{
	gdFontPtr fp = (gdFontPtr) rsrc->ptr;

	if (fp->data) {
		efree(fp->data);
	}

	efree(fp);
}
/* }}} */

/* {{{ php_gd_error_method
 */
void php_gd_error_method(int type, const char *format, va_list args)
{

	switch (type) {
#ifndef PHP_WIN32
		case GD_DEBUG:
		case GD_INFO:
#endif
		case GD_NOTICE:
			type = E_NOTICE;
			break;
		case GD_WARNING:
			type = E_WARNING;
			break;
		default:
			type = E_ERROR;
	}
	php_verror(NULL, "", type, format, args);
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(gd)
{
	le_gd = zend_register_list_destructors_ex(php_free_gd_image, NULL, "gd", module_number);
	le_gd_font = zend_register_list_destructors_ex(php_free_gd_font, NULL, "gd font", module_number);

#if defined(HAVE_GD_FREETYPE) && defined(HAVE_GD_BUNDLED)
	gdFontCacheMutexSetup();
#endif
	gdSetErrorMethod(php_gd_error_method);

	REGISTER_INI_ENTRIES();

	REGISTER_LONG_CONSTANT("IMG_GIF", PHP_IMG_GIF, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_JPG", PHP_IMG_JPG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_JPEG", PHP_IMG_JPEG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_PNG", PHP_IMG_PNG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_WBMP", PHP_IMG_WBMP, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_XPM", PHP_IMG_XPM, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_WEBP", PHP_IMG_WEBP, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_BMP", PHP_IMG_BMP, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_TGA", PHP_IMG_TGA, CONST_CS | CONST_PERSISTENT);

	/* special colours for gd */
	REGISTER_LONG_CONSTANT("IMG_COLOR_TILED", gdTiled, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_COLOR_STYLED", gdStyled, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_COLOR_BRUSHED", gdBrushed, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_COLOR_STYLEDBRUSHED", gdStyledBrushed, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_COLOR_TRANSPARENT", gdTransparent, CONST_CS | CONST_PERSISTENT);

	/* for imagefilledarc */
	REGISTER_LONG_CONSTANT("IMG_ARC_ROUNDED", gdArc, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_ARC_PIE", gdPie, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_ARC_CHORD", gdChord, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_ARC_NOFILL", gdNoFill, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_ARC_EDGED", gdEdged, CONST_CS | CONST_PERSISTENT);

    /* GD2 image format types */
	REGISTER_LONG_CONSTANT("IMG_GD2_RAW", GD2_FMT_RAW, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_GD2_COMPRESSED", GD2_FMT_COMPRESSED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FLIP_HORIZONTAL", GD_FLIP_HORINZONTAL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FLIP_VERTICAL", GD_FLIP_VERTICAL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FLIP_BOTH", GD_FLIP_BOTH, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_EFFECT_REPLACE", gdEffectReplace, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_EFFECT_ALPHABLEND", gdEffectAlphaBlend, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_EFFECT_NORMAL", gdEffectNormal, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_EFFECT_OVERLAY", gdEffectOverlay, CONST_CS | CONST_PERSISTENT);
#ifdef gdEffectMultiply
	REGISTER_LONG_CONSTANT("IMG_EFFECT_MULTIPLY", gdEffectMultiply, CONST_CS | CONST_PERSISTENT);
#endif

	REGISTER_LONG_CONSTANT("IMG_CROP_DEFAULT", GD_CROP_DEFAULT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_CROP_TRANSPARENT", GD_CROP_TRANSPARENT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_CROP_BLACK", GD_CROP_BLACK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_CROP_WHITE", GD_CROP_WHITE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_CROP_SIDES", GD_CROP_SIDES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_CROP_THRESHOLD", GD_CROP_THRESHOLD, CONST_CS | CONST_PERSISTENT);


	REGISTER_LONG_CONSTANT("IMG_BELL", GD_BELL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_BESSEL", GD_BESSEL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_BILINEAR_FIXED", GD_BILINEAR_FIXED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_BICUBIC", GD_BICUBIC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_BICUBIC_FIXED", GD_BICUBIC_FIXED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_BLACKMAN", GD_BLACKMAN, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_BOX", GD_BOX, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_BSPLINE", GD_BSPLINE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_CATMULLROM", GD_CATMULLROM, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_GAUSSIAN", GD_GAUSSIAN, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_GENERALIZED_CUBIC", GD_GENERALIZED_CUBIC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_HERMITE", GD_HERMITE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_HAMMING", GD_HAMMING, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_HANNING", GD_HANNING, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_MITCHELL", GD_MITCHELL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_POWER", GD_POWER, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_QUADRATIC", GD_QUADRATIC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_SINC", GD_SINC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_NEAREST_NEIGHBOUR", GD_NEAREST_NEIGHBOUR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_WEIGHTED4", GD_WEIGHTED4, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_TRIANGLE", GD_TRIANGLE, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("IMG_AFFINE_TRANSLATE", GD_AFFINE_TRANSLATE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_AFFINE_SCALE", GD_AFFINE_SCALE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_AFFINE_ROTATE", GD_AFFINE_ROTATE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_AFFINE_SHEAR_HORIZONTAL", GD_AFFINE_SHEAR_HORIZONTAL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_AFFINE_SHEAR_VERTICAL", GD_AFFINE_SHEAR_VERTICAL, CONST_CS | CONST_PERSISTENT);

#if defined(HAVE_GD_BUNDLED)
	REGISTER_LONG_CONSTANT("GD_BUNDLED", 1, CONST_CS | CONST_PERSISTENT);
#else
	REGISTER_LONG_CONSTANT("GD_BUNDLED", 0, CONST_CS | CONST_PERSISTENT);
#endif

	/* Section Filters */
	REGISTER_LONG_CONSTANT("IMG_FILTER_NEGATE", IMAGE_FILTER_NEGATE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_GRAYSCALE", IMAGE_FILTER_GRAYSCALE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_BRIGHTNESS", IMAGE_FILTER_BRIGHTNESS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_CONTRAST", IMAGE_FILTER_CONTRAST, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_COLORIZE", IMAGE_FILTER_COLORIZE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_EDGEDETECT", IMAGE_FILTER_EDGEDETECT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_GAUSSIAN_BLUR", IMAGE_FILTER_GAUSSIAN_BLUR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_SELECTIVE_BLUR", IMAGE_FILTER_SELECTIVE_BLUR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_EMBOSS", IMAGE_FILTER_EMBOSS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_MEAN_REMOVAL", IMAGE_FILTER_MEAN_REMOVAL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_SMOOTH", IMAGE_FILTER_SMOOTH, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_PIXELATE", IMAGE_FILTER_PIXELATE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_SCATTER", IMAGE_FILTER_SCATTER, CONST_CS | CONST_PERSISTENT);
	/* End Section Filters */

#ifdef GD_VERSION_STRING
	REGISTER_STRING_CONSTANT("GD_VERSION", GD_VERSION_STRING, CONST_CS | CONST_PERSISTENT);
#endif

#if defined(GD_MAJOR_VERSION) && defined(GD_MINOR_VERSION) && defined(GD_RELEASE_VERSION) && defined(GD_EXTRA_VERSION)
	REGISTER_LONG_CONSTANT("GD_MAJOR_VERSION", GD_MAJOR_VERSION, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GD_MINOR_VERSION", GD_MINOR_VERSION, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GD_RELEASE_VERSION", GD_RELEASE_VERSION, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("GD_EXTRA_VERSION", GD_EXTRA_VERSION, CONST_CS | CONST_PERSISTENT);
#endif


#ifdef HAVE_GD_PNG

	/*
	 * cannot include #include "png.h"
	 * /usr/include/pngconf.h:310:2: error: #error png.h already includes setjmp.h with some additional fixup.
	 * as error, use the values for now...
	 */
	REGISTER_LONG_CONSTANT("PNG_NO_FILTER",	    0x00, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PNG_FILTER_NONE",   0x08, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PNG_FILTER_SUB",    0x10, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PNG_FILTER_UP",     0x20, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PNG_FILTER_AVG",    0x40, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PNG_FILTER_PAETH",  0x80, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PNG_ALL_FILTERS",   0x08 | 0x10 | 0x20 | 0x40 | 0x80, CONST_CS | CONST_PERSISTENT);
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(gd)
{
#if defined(HAVE_GD_FREETYPE) && defined(HAVE_GD_BUNDLED)
	gdFontCacheMutexShutdown();
#endif
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(gd)
{
#ifdef HAVE_GD_FREETYPE
	gdFontCacheShutdown();
#endif
	return SUCCESS;
}
/* }}} */

#if defined(HAVE_GD_BUNDLED)
#define PHP_GD_VERSION_STRING "bundled (2.1.0 compatible)"
#else
# define PHP_GD_VERSION_STRING GD_VERSION_STRING
#endif

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(gd)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "GD Support", "enabled");

	/* need to use a PHPAPI function here because it is external module in windows */

#if defined(HAVE_GD_BUNDLED)
	php_info_print_table_row(2, "GD Version", PHP_GD_VERSION_STRING);
#else
	php_info_print_table_row(2, "GD headers Version", PHP_GD_VERSION_STRING);
#if defined(HAVE_GD_LIBVERSION)
	php_info_print_table_row(2, "GD library Version", gdVersionString());
#endif
#endif

#ifdef HAVE_GD_FREETYPE
	php_info_print_table_row(2, "FreeType Support", "enabled");
	php_info_print_table_row(2, "FreeType Linkage", "with freetype");
#ifdef HAVE_GD_BUNDLED
	{
		char tmp[256];

#ifdef FREETYPE_PATCH
		snprintf(tmp, sizeof(tmp), "%d.%d.%d", FREETYPE_MAJOR, FREETYPE_MINOR, FREETYPE_PATCH);
#elif defined(FREETYPE_MAJOR)
		snprintf(tmp, sizeof(tmp), "%d.%d", FREETYPE_MAJOR, FREETYPE_MINOR);
#else
		snprintf(tmp, sizeof(tmp), "1.x");
#endif
		php_info_print_table_row(2, "FreeType Version", tmp);
	}
#endif
#endif

	php_info_print_table_row(2, "GIF Read Support", "enabled");
	php_info_print_table_row(2, "GIF Create Support", "enabled");

#ifdef HAVE_GD_JPG
	{
		php_info_print_table_row(2, "JPEG Support", "enabled");
#if defined(HAVE_GD_BUNDLED)
		php_info_print_table_row(2, "libJPEG Version", gdJpegGetVersionString());
#endif
	}
#endif

#ifdef HAVE_GD_PNG
	php_info_print_table_row(2, "PNG Support", "enabled");
#if defined(HAVE_GD_BUNDLED)
	php_info_print_table_row(2, "libPNG Version", gdPngGetVersionString());
#endif
#endif
	php_info_print_table_row(2, "WBMP Support", "enabled");
#if defined(HAVE_GD_XPM)
	php_info_print_table_row(2, "XPM Support", "enabled");
#if defined(HAVE_GD_BUNDLED)
	{
		char tmp[12];
		snprintf(tmp, sizeof(tmp), "%d", XpmLibraryVersion());
		php_info_print_table_row(2, "libXpm Version", tmp);
	}
#endif
#endif
	php_info_print_table_row(2, "XBM Support", "enabled");
#if defined(USE_GD_JISX0208)
	php_info_print_table_row(2, "JIS-mapped Japanese Font Support", "enabled");
#endif
#ifdef HAVE_GD_WEBP
	php_info_print_table_row(2, "WebP Support", "enabled");
#endif
#ifdef HAVE_GD_BMP
	php_info_print_table_row(2, "BMP Support", "enabled");
#endif
#ifdef HAVE_GD_TGA
	php_info_print_table_row(2, "TGA Read Support", "enabled");
#endif
	php_info_print_table_end();
	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ proto array gd_info()
 */
PHP_FUNCTION(gd_info)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	array_init(return_value);

	add_assoc_string(return_value, "GD Version", PHP_GD_VERSION_STRING);

#ifdef HAVE_GD_FREETYPE
	add_assoc_bool(return_value, "FreeType Support", 1);
	add_assoc_string(return_value, "FreeType Linkage", "with freetype");
#else
	add_assoc_bool(return_value, "FreeType Support", 0);
#endif
	add_assoc_bool(return_value, "GIF Read Support", 1);
	add_assoc_bool(return_value, "GIF Create Support", 1);
#ifdef HAVE_GD_JPG
	add_assoc_bool(return_value, "JPEG Support", 1);
#else
	add_assoc_bool(return_value, "JPEG Support", 0);
#endif
#ifdef HAVE_GD_PNG
	add_assoc_bool(return_value, "PNG Support", 1);
#else
	add_assoc_bool(return_value, "PNG Support", 0);
#endif
	add_assoc_bool(return_value, "WBMP Support", 1);
#if defined(HAVE_GD_XPM)
	add_assoc_bool(return_value, "XPM Support", 1);
#else
	add_assoc_bool(return_value, "XPM Support", 0);
#endif
	add_assoc_bool(return_value, "XBM Support", 1);
#ifdef HAVE_GD_WEBP
	add_assoc_bool(return_value, "WebP Support", 1);
#else
	add_assoc_bool(return_value, "WebP Support", 0);
#endif
#ifdef HAVE_GD_BMP
	add_assoc_bool(return_value, "BMP Support", 1);
#else
	add_assoc_bool(return_value, "BMP Support", 0);
#endif
#ifdef HAVE_GD_TGA
	add_assoc_bool(return_value, "TGA Read Support", 1);
#else
	add_assoc_bool(return_value, "TGA Read Support", 0);
#endif
#if defined(USE_GD_JISX0208)
	add_assoc_bool(return_value, "JIS-mapped Japanese Font Support", 1);
#else
	add_assoc_bool(return_value, "JIS-mapped Japanese Font Support", 0);
#endif
}
/* }}} */

/* Need this for cpdf. See also comment in file.c php3i_get_le_fp() */
PHP_GD_API int phpi_get_le_gd(void)
{
	return le_gd;
}
/* }}} */

#define FLIPWORD(a) (((a & 0xff000000) >> 24) | ((a & 0x00ff0000) >> 8) | ((a & 0x0000ff00) << 8) | ((a & 0x000000ff) << 24))

/* {{{ proto int imageloadfont(string filename)
   Load a new font */
PHP_FUNCTION(imageloadfont)
{
	zval *ind;
	zend_string *file;
	int hdr_size = sizeof(gdFont) - sizeof(char *);
	int body_size, n = 0, b, i, body_size_check;
	gdFontPtr font;
	php_stream *stream;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "P", &file) == FAILURE) {
		return;
	}

	stream = php_stream_open_wrapper(ZSTR_VAL(file), "rb", IGNORE_PATH | IGNORE_URL_WIN | REPORT_ERRORS, NULL);
	if (stream == NULL) {
		RETURN_FALSE;
	}

	/* Only supports a architecture-dependent binary dump format
	 * at the moment.
	 * The file format is like this on machines with 32-byte integers:
	 *
	 * byte 0-3:   (int) number of characters in the font
	 * byte 4-7:   (int) value of first character in the font (often 32, space)
	 * byte 8-11:  (int) pixel width of each character
	 * byte 12-15: (int) pixel height of each character
	 * bytes 16-:  (char) array with character data, one byte per pixel
	 *                    in each character, for a total of
	 *                    (nchars*width*height) bytes.
	 */
	font = (gdFontPtr) emalloc(sizeof(gdFont));
	b = 0;
	while (b < hdr_size && (n = php_stream_read(stream, (char*)&font[b], hdr_size - b)) > 0) {
		b += n;
	}

	if (n <= 0) {
		efree(font);
		if (php_stream_eof(stream)) {
			php_error_docref(NULL, E_WARNING, "End of file while reading header");
		} else {
			php_error_docref(NULL, E_WARNING, "Error while reading header");
		}
		php_stream_close(stream);
		RETURN_FALSE;
	}
	i = php_stream_tell(stream);
	php_stream_seek(stream, 0, SEEK_END);
	body_size_check = php_stream_tell(stream) - hdr_size;
	php_stream_seek(stream, i, SEEK_SET);

	if (overflow2(font->nchars, font->h) || overflow2(font->nchars * font->h, font->w )) {
		php_error_docref(NULL, E_WARNING, "Error reading font, invalid font header");
		efree(font);
		php_stream_close(stream);
		RETURN_FALSE;
	}

	body_size = font->w * font->h * font->nchars;
	if (body_size != body_size_check) {
		font->w = FLIPWORD(font->w);
		font->h = FLIPWORD(font->h);
		font->nchars = FLIPWORD(font->nchars);
		body_size = font->w * font->h * font->nchars;
	}

	if (body_size != body_size_check) {
		php_error_docref(NULL, E_WARNING, "Error reading font");
		efree(font);
		php_stream_close(stream);
		RETURN_FALSE;
	}

	font->data = emalloc(body_size);
	b = 0;
	while (b < body_size && (n = php_stream_read(stream, &font->data[b], body_size - b)) > 0) {
		b += n;
	}

	if (n <= 0) {
		efree(font->data);
		efree(font);
		if (php_stream_eof(stream)) {
			php_error_docref(NULL, E_WARNING, "End of file while reading body");
		} else {
			php_error_docref(NULL, E_WARNING, "Error while reading body");
		}
		php_stream_close(stream);
		RETURN_FALSE;
	}
	php_stream_close(stream);

	ind = zend_list_insert(font, le_gd_font);

	/* Adding 5 to the font index so we will never have font indices
	 * that overlap with the old fonts (with indices 1-5).  The first
	 * list index given out is always 1.
	 */
	RETURN_LONG(Z_RES_HANDLE_P(ind) + 5);
}
/* }}} */

/* {{{ proto bool imagesetstyle(resource im, array styles)
   Set the line drawing styles for use with imageline and IMG_COLOR_STYLED. */
PHP_FUNCTION(imagesetstyle)
{
	zval *IM, *styles, *item;
	gdImagePtr im;
	int *stylearr;
	int index = 0;
    uint32_t num_styles;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ra", &IM, &styles) == FAILURE)  {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

    num_styles = zend_hash_num_elements(Z_ARRVAL_P(styles));
    if (num_styles == 0) {
        php_error_docref(NULL, E_WARNING, "styles array must not be empty");
        RETURN_FALSE;
    }

	/* copy the style values in the stylearr */
	stylearr = safe_emalloc(sizeof(int), num_styles, 0);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(styles), item) {
		stylearr[index++] = zval_get_long(item);
	} ZEND_HASH_FOREACH_END();

	gdImageSetStyle(im, stylearr, index);

	efree(stylearr);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto resource imagecreatetruecolor(int x_size, int y_size)
   Create a new true color image */
PHP_FUNCTION(imagecreatetruecolor)
{
	zend_long x_size, y_size;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &x_size, &y_size) == FAILURE) {
		return;
	}

	if (x_size <= 0 || y_size <= 0 || x_size >= INT_MAX || y_size >= INT_MAX) {
		php_error_docref(NULL, E_WARNING, "Invalid image dimensions");
		RETURN_FALSE;
	}

	im = gdImageCreateTrueColor(x_size, y_size);

	if (!im) {
		RETURN_FALSE;
	}

	RETURN_RES(zend_register_resource(im, le_gd));
}
/* }}} */

/* {{{ proto bool imageistruecolor(resource im)
   return true if the image uses truecolor */
PHP_FUNCTION(imageistruecolor)
{
	zval *IM;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &IM) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	RETURN_BOOL(im->trueColor);
}
/* }}} */

/* {{{ proto void imagetruecolortopalette(resource im, bool ditherFlag, int colorsWanted)
   Convert a true color image to a palette based image with a number of colors, optionally using dithering. */
PHP_FUNCTION(imagetruecolortopalette)
{
	zval *IM;
	zend_bool dither;
	zend_long ncolors;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rbl", &IM, &dither, &ncolors) == FAILURE)  {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	if (ncolors <= 0 || ZEND_LONG_INT_OVFL(ncolors)) {
		php_error_docref(NULL, E_WARNING, "Number of colors has to be greater than zero and no more than %d", INT_MAX);
		RETURN_FALSE;
	}
	if (gdImageTrueColorToPalette(im, dither, (int)ncolors)) {
		RETURN_TRUE;
	} else {
		php_error_docref(NULL, E_WARNING, "Couldn't convert to palette");
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto void imagepalettetotruecolor(resource im)
   Convert a palette based image to a true color image. */
PHP_FUNCTION(imagepalettetotruecolor)
{
	zval *IM;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &IM) == FAILURE)  {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	if (gdImagePaletteToTrueColor(im) == 0) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool imagecolormatch(resource im1, resource im2)
   Makes the colors of the palette version of an image more closely match the true color version */
PHP_FUNCTION(imagecolormatch)
{
	zval *IM1, *IM2;
	gdImagePtr im1, im2;
	int result;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rr", &IM1, &IM2) == FAILURE) {
		return;
	}

	if ((im1 = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM1), "Image", le_gd)) == NULL) {
		return;
	}
	if ((im2 = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM2), "Image", le_gd)) == NULL) {
		return;
	}

	result = gdImageColorMatch(im1, im2);
	switch (result) {
		case -1:
			php_error_docref(NULL, E_WARNING, "Image1 must be TrueColor" );
			RETURN_FALSE;
			break;
		case -2:
			php_error_docref(NULL, E_WARNING, "Image2 must be Palette" );
			RETURN_FALSE;
			break;
		case -3:
			php_error_docref(NULL, E_WARNING, "Image1 and Image2 must be the same size" );
			RETURN_FALSE;
			break;
		case -4:
			php_error_docref(NULL, E_WARNING, "Image2 must have at least one color" );
			RETURN_FALSE;
			break;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool imagesetthickness(resource im, int thickness)
   Set line thickness for drawing lines, ellipses, rectangles, polygons etc. */
PHP_FUNCTION(imagesetthickness)
{
	zval *IM;
	zend_long thick;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rl", &IM, &thick) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	gdImageSetThickness(im, thick);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool imagefilledellipse(resource im, int cx, int cy, int w, int h, int color)
   Draw an ellipse */
PHP_FUNCTION(imagefilledellipse)
{
	zval *IM;
	zend_long cx, cy, w, h, color;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rlllll", &IM, &cx, &cy, &w, &h, &color) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	gdImageFilledEllipse(im, cx, cy, w, h, color);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool imagefilledarc(resource im, int cx, int cy, int w, int h, int s, int e, int col, int style)
   Draw a filled partial ellipse */
PHP_FUNCTION(imagefilledarc)
{
	zval *IM;
	zend_long cx, cy, w, h, ST, E, col, style;
	gdImagePtr im;
	int e, st;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rllllllll", &IM, &cx, &cy, &w, &h, &ST, &E, &col, &style) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	e = E;
	if (e < 0) {
		e %= 360;
	}

	st = ST;
	if (st < 0) {
		st %= 360;
	}

	gdImageFilledArc(im, cx, cy, w, h, st, e, col, style);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool imagealphablending(resource im, bool on)
   Turn alpha blending mode on or off for the given image */
PHP_FUNCTION(imagealphablending)
{
	zval *IM;
	zend_bool blend;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rb", &IM, &blend) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	gdImageAlphaBlending(im, blend);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool imagesavealpha(resource im, bool on)
   Include alpha channel to a saved image */
PHP_FUNCTION(imagesavealpha)
{
	zval *IM;
	zend_bool save;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rb", &IM, &save) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	gdImageSaveAlpha(im, save);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool imagelayereffect(resource im, int effect)
   Set the alpha blending flag to use the bundled libgd layering effects */
PHP_FUNCTION(imagelayereffect)
{
	zval *IM;
	zend_long effect;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rl", &IM, &effect) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	gdImageAlphaBlending(im, effect);

	RETURN_TRUE;
}
/* }}} */

#define CHECK_RGBA_RANGE(component, name) \
	if (component < 0 || component > gd##name##Max) { \
		php_error_docref(NULL, E_WARNING, #name " component is out of range"); \
		RETURN_FALSE; \
	}

/* {{{ proto int imagecolorallocatealpha(resource im, int red, int green, int blue, int alpha)
   Allocate a color with an alpha level.  Works for true color and palette based images */
PHP_FUNCTION(imagecolorallocatealpha)
{
	zval *IM;
	zend_long red, green, blue, alpha;
	gdImagePtr im;
	int ct = (-1);

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rllll", &IM, &red, &green, &blue, &alpha) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	CHECK_RGBA_RANGE(red, Red);
	CHECK_RGBA_RANGE(green, Green);
	CHECK_RGBA_RANGE(blue, Blue);
	CHECK_RGBA_RANGE(alpha, Alpha);

	ct = gdImageColorAllocateAlpha(im, red, green, blue, alpha);
	if (ct < 0) {
		RETURN_FALSE;
	}
	RETURN_LONG((zend_long)ct);
}
/* }}} */

/* {{{ proto int imagecolorresolvealpha(resource im, int red, int green, int blue, int alpha)
   Resolve/Allocate a colour with an alpha level.  Works for true colour and palette based images */
PHP_FUNCTION(imagecolorresolvealpha)
{
	zval *IM;
	zend_long red, green, blue, alpha;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rllll", &IM, &red, &green, &blue, &alpha) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	CHECK_RGBA_RANGE(red, Red);
	CHECK_RGBA_RANGE(green, Green);
	CHECK_RGBA_RANGE(blue, Blue);
	CHECK_RGBA_RANGE(alpha, Alpha);

	RETURN_LONG(gdImageColorResolveAlpha(im, red, green, blue, alpha));
}
/* }}} */

/* {{{ proto int imagecolorclosestalpha(resource im, int red, int green, int blue, int alpha)
   Find the closest matching colour with alpha transparency */
PHP_FUNCTION(imagecolorclosestalpha)
{
	zval *IM;
	zend_long red, green, blue, alpha;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rllll", &IM, &red, &green, &blue, &alpha) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	CHECK_RGBA_RANGE(red, Red);
	CHECK_RGBA_RANGE(green, Green);
	CHECK_RGBA_RANGE(blue, Blue);
	CHECK_RGBA_RANGE(alpha, Alpha);

	RETURN_LONG(gdImageColorClosestAlpha(im, red, green, blue, alpha));
}
/* }}} */

/* {{{ proto int imagecolorexactalpha(resource im, int red, int green, int blue, int alpha)
   Find exact match for colour with transparency */
PHP_FUNCTION(imagecolorexactalpha)
{
	zval *IM;
	zend_long red, green, blue, alpha;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rllll", &IM, &red, &green, &blue, &alpha) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	CHECK_RGBA_RANGE(red, Red);
	CHECK_RGBA_RANGE(green, Green);
	CHECK_RGBA_RANGE(blue, Blue);
	CHECK_RGBA_RANGE(alpha, Alpha);

	RETURN_LONG(gdImageColorExactAlpha(im, red, green, blue, alpha));
}
/* }}} */

/* {{{ proto bool imagecopyresampled(resource dst_im, resource src_im, int dst_x, int dst_y, int src_x, int src_y, int dst_w, int dst_h, int src_w, int src_h)
   Copy and resize part of an image using resampling to help ensure clarity */
PHP_FUNCTION(imagecopyresampled)
{
	zval *SIM, *DIM;
	zend_long SX, SY, SW, SH, DX, DY, DW, DH;
	gdImagePtr im_dst, im_src;
	int srcH, srcW, dstH, dstW, srcY, srcX, dstY, dstX;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rrllllllll", &DIM, &SIM, &DX, &DY, &SX, &SY, &DW, &DH, &SW, &SH) == FAILURE) {
		return;
	}

	if ((im_dst = (gdImagePtr)zend_fetch_resource(Z_RES_P(DIM), "Image", le_gd)) == NULL) {
		return;
	}

	if ((im_src = (gdImagePtr)zend_fetch_resource(Z_RES_P(SIM), "Image", le_gd)) == NULL) {
		return;
	}

	srcX = SX;
	srcY = SY;
	srcH = SH;
	srcW = SW;
	dstX = DX;
	dstY = DY;
	dstH = DH;
	dstW = DW;

	gdImageCopyResampled(im_dst, im_src, dstX, dstY, srcX, srcY, dstW, dstH, srcW, srcH);

	RETURN_TRUE;
}
/* }}} */

#ifdef PHP_WIN32
/* {{{ proto resource imagegrabwindow(int window_handle [, int client_area])
   Grab a window or its client area using a windows handle (HWND property in COM instance) */
PHP_FUNCTION(imagegrabwindow)
{
	HWND window;
	zend_long client_area = 0;
	RECT rc = {0};
	int Width, Height;
	HDC		hdc;
	HDC memDC;
	HBITMAP memBM;
	HBITMAP hOld;
	zend_long lwindow_handle;
	gdImagePtr im = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|l", &lwindow_handle, &client_area) == FAILURE) {
		RETURN_FALSE;
	}

	window = (HWND) lwindow_handle;

	if (!IsWindow(window)) {
		php_error_docref(NULL, E_NOTICE, "Invalid window handle");
		RETURN_FALSE;
	}

	hdc		= GetDC(0);

	if (client_area) {
		GetClientRect(window, &rc);
		Width = rc.right;
		Height = rc.bottom;
	} else {
		GetWindowRect(window, &rc);
		Width	= rc.right - rc.left;
		Height	= rc.bottom - rc.top;
	}

	Width		= (Width/4)*4;

	memDC	= CreateCompatibleDC(hdc);
	memBM	= CreateCompatibleBitmap(hdc, Width, Height);
	hOld	= (HBITMAP) SelectObject (memDC, memBM);

	PrintWindow(window, memDC, (UINT) client_area);

	im = gdImageCreateTrueColor(Width, Height);
	if (im) {
		int x,y;
		for (y=0; y <= Height; y++) {
			for (x=0; x <= Width; x++) {
				int c = GetPixel(memDC, x,y);
				gdImageSetPixel(im, x, y, gdTrueColor(GetRValue(c), GetGValue(c), GetBValue(c)));
			}
		}
	}

	SelectObject(memDC,hOld);
	DeleteObject(memBM);
	DeleteDC(memDC);
	ReleaseDC( 0, hdc );

	if (!im) {
		RETURN_FALSE;
	} else {
		RETURN_RES(zend_register_resource(im, le_gd));
	}
}
/* }}} */

/* {{{ proto resource imagegrabscreen()
   Grab a screenshot */
PHP_FUNCTION(imagegrabscreen)
{
	HWND window = GetDesktopWindow();
	RECT rc = {0};
	int Width, Height;
	HDC		hdc;
	HDC memDC;
	HBITMAP memBM;
	HBITMAP hOld;
	gdImagePtr im;
	hdc		= GetDC(0);

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	if (!hdc) {
		RETURN_FALSE;
	}

	GetWindowRect(window, &rc);
	Width	= rc.right - rc.left;
	Height	= rc.bottom - rc.top;

	Width		= (Width/4)*4;

	memDC	= CreateCompatibleDC(hdc);
	memBM	= CreateCompatibleBitmap(hdc, Width, Height);
	hOld	= (HBITMAP) SelectObject (memDC, memBM);
	BitBlt( memDC, 0, 0, Width, Height , hdc, rc.left, rc.top , SRCCOPY );

	im = gdImageCreateTrueColor(Width, Height);
	if (im) {
		int x,y;
		for (y=0; y <= Height; y++) {
			for (x=0; x <= Width; x++) {
				int c = GetPixel(memDC, x,y);
				gdImageSetPixel(im, x, y, gdTrueColor(GetRValue(c), GetGValue(c), GetBValue(c)));
			}
		}
	}

	SelectObject(memDC,hOld);
	DeleteObject(memBM);
	DeleteDC(memDC);
	ReleaseDC( 0, hdc );

	if (!im) {
		RETURN_FALSE;
	} else {
		RETURN_RES(zend_register_resource(im, le_gd));
	}
}
/* }}} */
#endif /* PHP_WIN32 */

/* {{{ proto resource imagerotate(resource src_im, float angle, int bgdcolor [, int ignoretransparent])
   Rotate an image using a custom angle */
PHP_FUNCTION(imagerotate)
{
	zval *SIM;
	gdImagePtr im_dst, im_src;
	double degrees;
	zend_long color;
	zend_long ignoretransparent = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rdl|l", &SIM, &degrees, &color, &ignoretransparent) == FAILURE) {
		return;
	}

	if ((im_src = (gdImagePtr)zend_fetch_resource(Z_RES_P(SIM), "Image", le_gd)) == NULL) {
		return;
	}

	im_dst = gdImageRotateInterpolated(im_src, (const float)degrees, color);

	if (im_dst != NULL) {
		RETURN_RES(zend_register_resource(im_dst, le_gd));
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool imagesettile(resource image, resource tile)
   Set the tile image to $tile when filling $image with the "IMG_COLOR_TILED" color */
PHP_FUNCTION(imagesettile)
{
	zval *IM, *TILE;
	gdImagePtr im, tile;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rr", &IM, &TILE) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	if ((tile = (gdImagePtr)zend_fetch_resource(Z_RES_P(TILE), "Image", le_gd)) == NULL) {
		return;
	}

	gdImageSetTile(im, tile);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool imagesetbrush(resource image, resource brush)
   Set the brush image to $brush when filling $image with the "IMG_COLOR_BRUSHED" color */
PHP_FUNCTION(imagesetbrush)
{
	zval *IM, *TILE;
	gdImagePtr im, tile;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rr", &IM, &TILE) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	if ((tile = (gdImagePtr)zend_fetch_resource(Z_RES_P(TILE), "Image", le_gd)) == NULL) {
		return;
	}

	gdImageSetBrush(im, tile);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto resource imagecreate(int x_size, int y_size)
   Create a new image */
PHP_FUNCTION(imagecreate)
{
	zend_long x_size, y_size;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &x_size, &y_size) == FAILURE) {
		return;
	}

	if (x_size <= 0 || y_size <= 0 || x_size >= INT_MAX || y_size >= INT_MAX) {
		php_error_docref(NULL, E_WARNING, "Invalid image dimensions");
		RETURN_FALSE;
	}

	im = gdImageCreate(x_size, y_size);

	if (!im) {
		RETURN_FALSE;
	}

	RETURN_RES(zend_register_resource(im, le_gd));
}
/* }}} */

/* {{{ proto int imagetypes(void)
   Return the types of images supported in a bitfield - 1=GIF, 2=JPEG, 4=PNG, 8=WBMP, 16=XPM */
PHP_FUNCTION(imagetypes)
{
	int ret = 0;
	ret = PHP_IMG_GIF;
#ifdef HAVE_GD_JPG
	ret |= PHP_IMG_JPG;
#endif
#ifdef HAVE_GD_PNG
	ret |= PHP_IMG_PNG;
#endif
	ret |= PHP_IMG_WBMP;
#if defined(HAVE_GD_XPM)
	ret |= PHP_IMG_XPM;
#endif
#ifdef HAVE_GD_WEBP
	ret |= PHP_IMG_WEBP;
#endif
#ifdef HAVE_GD_BMP
	ret |= PHP_IMG_BMP;
#endif
#ifdef HAVE_GD_TGA
	ret |= PHP_IMG_TGA;
#endif

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_LONG(ret);
}
/* }}} */

/* {{{ _php_ctx_getmbi
 */

static int _php_ctx_getmbi(gdIOCtx *ctx)
{
	int i, mbi = 0;

	do {
		i = (ctx->getC)(ctx);
		if (i < 0) {
			return -1;
		}
		mbi = (mbi << 7) | (i & 0x7f);
	} while (i & 0x80);

	return mbi;
}
/* }}} */

/* {{{ _php_image_type
 */
static const char php_sig_gd2[3] = {'g', 'd', '2'};

static int _php_image_type (char data[12])
{
	/* Based on ext/standard/image.c */

	if (data == NULL) {
		return -1;
	}

	if (!memcmp(data, php_sig_gd2, sizeof(php_sig_gd2))) {
		return PHP_GDIMG_TYPE_GD2;
	} else if (!memcmp(data, php_sig_jpg, sizeof(php_sig_jpg))) {
		return PHP_GDIMG_TYPE_JPG;
	} else if (!memcmp(data, php_sig_png, sizeof(php_sig_png))) {
		return PHP_GDIMG_TYPE_PNG;
	} else if (!memcmp(data, php_sig_gif, sizeof(php_sig_gif))) {
		return PHP_GDIMG_TYPE_GIF;
	} else if (!memcmp(data, php_sig_bmp, sizeof(php_sig_bmp))) {
		return PHP_GDIMG_TYPE_BMP;
	} else if(!memcmp(data, php_sig_riff, sizeof(php_sig_riff)) && !memcmp(data + sizeof(php_sig_riff) + sizeof(uint32_t), php_sig_webp, sizeof(php_sig_webp))) {
		return PHP_GDIMG_TYPE_WEBP;
	}
	else {
		gdIOCtx *io_ctx;
		io_ctx = gdNewDynamicCtxEx(8, data, 0);
		if (io_ctx) {
			if (_php_ctx_getmbi(io_ctx) == 0 && _php_ctx_getmbi(io_ctx) >= 0) {
				io_ctx->gd_free(io_ctx);
				return PHP_GDIMG_TYPE_WBM;
			} else {
				io_ctx->gd_free(io_ctx);
			}
		}
	}
	return -1;
}
/* }}} */

/* {{{ _php_image_create_from_string
 */
gdImagePtr _php_image_create_from_string(zend_string *data, char *tn, gdImagePtr (*ioctx_func_p)())
{
	gdImagePtr im;
	gdIOCtx *io_ctx;

	io_ctx = gdNewDynamicCtxEx(ZSTR_LEN(data), ZSTR_VAL(data), 0);

	if (!io_ctx) {
		return NULL;
	}

	im = (*ioctx_func_p)(io_ctx);
	if (!im) {
		php_error_docref(NULL, E_WARNING, "Passed data is not in '%s' format", tn);
		io_ctx->gd_free(io_ctx);
		return NULL;
	}

	io_ctx->gd_free(io_ctx);

	return im;
}
/* }}} */

/* {{{ proto resource imagecreatefromstring(string image)
   Create a new image from the image stream in the string */
PHP_FUNCTION(imagecreatefromstring)
{
	zend_string *data;
	gdImagePtr im;
	int imtype;
	char sig[12];

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &data) == FAILURE) {
		return;
	}

	if (ZSTR_LEN(data) < sizeof(sig)) {
		php_error_docref(NULL, E_WARNING, "Empty string or invalid image");
		RETURN_FALSE;
	}

	memcpy(sig, ZSTR_VAL(data), sizeof(sig));

	imtype = _php_image_type(sig);

	switch (imtype) {
		case PHP_GDIMG_TYPE_JPG:
#ifdef HAVE_GD_JPG
			im = _php_image_create_from_string(data, "JPEG", gdImageCreateFromJpegCtx);
#else
			php_error_docref(NULL, E_WARNING, "No JPEG support in this PHP build");
			RETURN_FALSE;
#endif
			break;

		case PHP_GDIMG_TYPE_PNG:
#ifdef HAVE_GD_PNG
			im = _php_image_create_from_string(data, "PNG", gdImageCreateFromPngCtx);
#else
			php_error_docref(NULL, E_WARNING, "No PNG support in this PHP build");
			RETURN_FALSE;
#endif
			break;

		case PHP_GDIMG_TYPE_GIF:
			im = _php_image_create_from_string(data, "GIF", gdImageCreateFromGifCtx);
			break;

		case PHP_GDIMG_TYPE_WBM:
			im = _php_image_create_from_string(data, "WBMP", gdImageCreateFromWBMPCtx);
			break;

		case PHP_GDIMG_TYPE_GD2:
			im = _php_image_create_from_string(data, "GD2", gdImageCreateFromGd2Ctx);
			break;

		case PHP_GDIMG_TYPE_BMP:
			im = _php_image_create_from_string(data, "BMP", gdImageCreateFromBmpCtx);
			break;

		case PHP_GDIMG_TYPE_WEBP:
#ifdef HAVE_GD_WEBP
			im = _php_image_create_from_string(data, "WEBP", gdImageCreateFromWebpCtx);
			break;
#else
			php_error_docref(NULL, E_WARNING, "No WEBP support in this PHP build");
			RETURN_FALSE;
#endif

		default:
			php_error_docref(NULL, E_WARNING, "Data is not in a recognized format");
			RETURN_FALSE;
	}

	if (!im) {
		php_error_docref(NULL, E_WARNING, "Couldn't create GD Image Stream out of Data");
		RETURN_FALSE;
	}

	RETURN_RES(zend_register_resource(im, le_gd));
}
/* }}} */

/* {{{ _php_image_create_from
 */
static void _php_image_create_from(INTERNAL_FUNCTION_PARAMETERS, int image_type, char *tn, gdImagePtr (*func_p)(), gdImagePtr (*ioctx_func_p)())
{
	char *file;
	size_t file_len;
	zend_long srcx, srcy, width, height;
	gdImagePtr im = NULL;
	php_stream *stream;
	FILE * fp = NULL;
#ifdef HAVE_GD_JPG
	long ignore_warning;
#endif

	if (image_type == PHP_GDIMG_TYPE_GD2PART) {
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "pllll", &file, &file_len, &srcx, &srcy, &width, &height) == FAILURE) {
			return;
		}
		if (width < 1 || height < 1) {
			php_error_docref(NULL, E_WARNING, "Zero width or height not allowed");
			RETURN_FALSE;
		}
	} else {
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "p", &file, &file_len) == FAILURE) {
			return;
		}
	}


	stream = php_stream_open_wrapper(file, "rb", REPORT_ERRORS|IGNORE_PATH|IGNORE_URL_WIN, NULL);
	if (stream == NULL)	{
		RETURN_FALSE;
	}

	/* try and avoid allocating a FILE* if the stream is not naturally a FILE* */
	if (php_stream_is(stream, PHP_STREAM_IS_STDIO))	{
		if (FAILURE == php_stream_cast(stream, PHP_STREAM_AS_STDIO, (void**)&fp, REPORT_ERRORS)) {
			goto out_err;
		}
	} else if (ioctx_func_p) {
		/* we can create an io context */
		gdIOCtx* io_ctx;
		zend_string *buff;
		char *pstr;

		buff = php_stream_copy_to_mem(stream, PHP_STREAM_COPY_ALL, 0);

		if (!buff) {
			php_error_docref(NULL, E_WARNING,"Cannot read image data");
			goto out_err;
		}

		/* needs to be malloc (persistent) - GD will free() it later */
		pstr = pestrndup(ZSTR_VAL(buff), ZSTR_LEN(buff), 1);
		io_ctx = gdNewDynamicCtxEx(ZSTR_LEN(buff), pstr, 0);
		if (!io_ctx) {
			pefree(pstr, 1);
			zend_string_release_ex(buff, 0);
			php_error_docref(NULL, E_WARNING,"Cannot allocate GD IO context");
			goto out_err;
		}

		if (image_type == PHP_GDIMG_TYPE_GD2PART) {
			im = (*ioctx_func_p)(io_ctx, srcx, srcy, width, height);
		} else {
			im = (*ioctx_func_p)(io_ctx);
		}
		io_ctx->gd_free(io_ctx);
		pefree(pstr, 1);
		zend_string_release_ex(buff, 0);
	}
	else if (php_stream_can_cast(stream, PHP_STREAM_AS_STDIO)) {
		/* try and force the stream to be FILE* */
		if (FAILURE == php_stream_cast(stream, PHP_STREAM_AS_STDIO | PHP_STREAM_CAST_TRY_HARD, (void **) &fp, REPORT_ERRORS)) {
			goto out_err;
		}
	}

	if (!im && fp) {
		switch (image_type) {
			case PHP_GDIMG_TYPE_GD2PART:
				im = (*func_p)(fp, srcx, srcy, width, height);
				break;
#if defined(HAVE_GD_XPM)
			case PHP_GDIMG_TYPE_XPM:
				im = gdImageCreateFromXpm(file);
				break;
#endif

#ifdef HAVE_GD_JPG
			case PHP_GDIMG_TYPE_JPG:
				ignore_warning = INI_INT("gd.jpeg_ignore_warning");
				im = gdImageCreateFromJpegEx(fp, ignore_warning);
			break;
#endif

			default:
				im = (*func_p)(fp);
				break;
		}

		fflush(fp);
	}

/* register_im: */
	if (im) {
		RETVAL_RES(zend_register_resource(im, le_gd));
		php_stream_close(stream);
		return;
	}

	php_error_docref(NULL, E_WARNING, "'%s' is not a valid %s file", file, tn);
out_err:
	php_stream_close(stream);
	RETURN_FALSE;

}
/* }}} */

/* {{{ proto resource imagecreatefromgif(string filename)
   Create a new image from GIF file or URL */
PHP_FUNCTION(imagecreatefromgif)
{
	_php_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_GDIMG_TYPE_GIF, "GIF", gdImageCreateFromGif, gdImageCreateFromGifCtx);
}
/* }}} */

#ifdef HAVE_GD_JPG
/* {{{ proto resource imagecreatefromjpeg(string filename)
   Create a new image from JPEG file or URL */
PHP_FUNCTION(imagecreatefromjpeg)
{
	_php_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_GDIMG_TYPE_JPG, "JPEG", gdImageCreateFromJpeg, gdImageCreateFromJpegCtx);
}
/* }}} */
#endif /* HAVE_GD_JPG */

#ifdef HAVE_GD_PNG
/* {{{ proto resource imagecreatefrompng(string filename)
   Create a new image from PNG file or URL */
PHP_FUNCTION(imagecreatefrompng)
{
	_php_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_GDIMG_TYPE_PNG, "PNG", gdImageCreateFromPng, gdImageCreateFromPngCtx);
}
/* }}} */
#endif /* HAVE_GD_PNG */

#ifdef HAVE_GD_WEBP
/* {{{ proto resource imagecreatefromwebp(string filename)
   Create a new image from WEBP file or URL */
PHP_FUNCTION(imagecreatefromwebp)
{
	_php_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_GDIMG_TYPE_WEBP, "WEBP", gdImageCreateFromWebp, gdImageCreateFromWebpCtx);
}
/* }}} */
#endif /* HAVE_GD_WEBP */

/* {{{ proto resource imagecreatefromxbm(string filename)
   Create a new image from XBM file or URL */
PHP_FUNCTION(imagecreatefromxbm)
{
	_php_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_GDIMG_TYPE_XBM, "XBM", gdImageCreateFromXbm, NULL);
}
/* }}} */

#if defined(HAVE_GD_XPM)
/* {{{ proto resource imagecreatefromxpm(string filename)
   Create a new image from XPM file or URL */
PHP_FUNCTION(imagecreatefromxpm)
{
	_php_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_GDIMG_TYPE_XPM, "XPM", gdImageCreateFromXpm, NULL);
}
/* }}} */
#endif

/* {{{ proto resource imagecreatefromwbmp(string filename)
   Create a new image from WBMP file or URL */
PHP_FUNCTION(imagecreatefromwbmp)
{
	_php_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_GDIMG_TYPE_WBM, "WBMP", gdImageCreateFromWBMP, gdImageCreateFromWBMPCtx);
}
/* }}} */

/* {{{ proto resource imagecreatefromgd(string filename)
   Create a new image from GD file or URL */
PHP_FUNCTION(imagecreatefromgd)
{
	_php_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_GDIMG_TYPE_GD, "GD", gdImageCreateFromGd, gdImageCreateFromGdCtx);
}
/* }}} */

/* {{{ proto resource imagecreatefromgd2(string filename)
   Create a new image from GD2 file or URL */
PHP_FUNCTION(imagecreatefromgd2)
{
	_php_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_GDIMG_TYPE_GD2, "GD2", gdImageCreateFromGd2, gdImageCreateFromGd2Ctx);
}
/* }}} */

/* {{{ proto resource imagecreatefromgd2part(string filename, int srcX, int srcY, int width, int height)
   Create a new image from a given part of GD2 file or URL */
PHP_FUNCTION(imagecreatefromgd2part)
{
	_php_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_GDIMG_TYPE_GD2PART, "GD2", gdImageCreateFromGd2Part, gdImageCreateFromGd2PartCtx);
}
/* }}} */

#if defined(HAVE_GD_BMP)
/* {{{ proto resource imagecreatefrombmp(string filename)
   Create a new image from BMP file or URL */
PHP_FUNCTION(imagecreatefrombmp)
{
	_php_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_GDIMG_TYPE_BMP, "BMP", gdImageCreateFromBmp, gdImageCreateFromBmpCtx);
}
/* }}} */
#endif

#if defined(HAVE_GD_TGA)
/* {{{ proto resource imagecreatefromtga(string filename)
   Create a new image from TGA file or URL */
PHP_FUNCTION(imagecreatefromtga)
{
	_php_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_GDIMG_TYPE_TGA, "TGA", gdImageCreateFromTga, gdImageCreateFromTgaCtx);
}
/* }}} */
#endif

/* {{{ _php_image_output
 */
static void _php_image_output(INTERNAL_FUNCTION_PARAMETERS, int image_type, char *tn, void (*func_p)())
{
	zval *imgind;
	char *file = NULL;
	zend_long quality = 0, type = 0;
	gdImagePtr im;
	char *fn = NULL;
	FILE *fp;
	size_t file_len = 0;
	int argc = ZEND_NUM_ARGS();
	int q = -1, t = 1;

	/* The quality parameter for gd2 stands for chunk size */

	if (zend_parse_parameters(argc, "r|pll", &imgind, &file, &file_len, &quality, &type) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(imgind), "Image", le_gd)) == NULL) {
		return;
	}

	if (argc > 1) {
		fn = file;
		if (argc >= 3) {
			q = quality;
			if (argc == 4) {
				t = type;
			}
		}
	}

	if (argc >= 2 && file_len) {
		PHP_GD_CHECK_OPEN_BASEDIR(fn, "Invalid filename");

		fp = VCWD_FOPEN(fn, "wb");
		if (!fp) {
			php_error_docref(NULL, E_WARNING, "Unable to open '%s' for writing", fn);
			RETURN_FALSE;
		}

		switch (image_type) {
			case PHP_GDIMG_CONVERT_WBM:
				if (q == -1) {
					q = 0;
				} else if (q < 0 || q > 255) {
					php_error_docref(NULL, E_WARNING, "Invalid threshold value '%d'. It must be between 0 and 255", q);
					q = 0;
				}
				gdImageWBMP(im, q, fp);
				break;
			case PHP_GDIMG_TYPE_GD:
				(*func_p)(im, fp);
				break;
			case PHP_GDIMG_TYPE_GD2:
				if (q == -1) {
					q = 128;
				}
				(*func_p)(im, fp, q, t);
				break;
			default:
				ZEND_ASSERT(0);
		}
		fflush(fp);
		fclose(fp);
	} else {
		int   b;
		FILE *tmp;
		char  buf[4096];
		zend_string *path;

		tmp = php_open_temporary_file(NULL, NULL, &path);
		if (tmp == NULL) {
			php_error_docref(NULL, E_WARNING, "Unable to open temporary file");
			RETURN_FALSE;
		}

		switch (image_type) {
			case PHP_GDIMG_CONVERT_WBM:
 				if (q == -1) {
  					q = 0;
  				} else if (q < 0 || q > 255) {
  					php_error_docref(NULL, E_WARNING, "Invalid threshold value '%d'. It must be between 0 and 255", q);
 					q = 0;
  				}
				gdImageWBMP(im, q, tmp);
				break;
			case PHP_GDIMG_TYPE_GD:
				(*func_p)(im, tmp);
				break;
			case PHP_GDIMG_TYPE_GD2:
				if (q == -1) {
					q = 128;
				}
				(*func_p)(im, tmp, q, t);
				break;
			default:
				ZEND_ASSERT(0);
		}

		fseek(tmp, 0, SEEK_SET);

		while ((b = fread(buf, 1, sizeof(buf), tmp)) > 0) {
			php_write(buf, b);
		}

		fclose(tmp);
		VCWD_UNLINK((const char *)ZSTR_VAL(path)); /* make sure that the temporary file is removed */
		zend_string_release_ex(path, 0);
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int imagexbm(int im, string filename [, int foreground])
   Output XBM image to browser or file */
PHP_FUNCTION(imagexbm)
{
	_php_image_output_ctx(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_GDIMG_TYPE_XBM, "XBM", gdImageXbmCtx);
}
/* }}} */

/* {{{ proto bool imagegif(resource im [, mixed to])
   Output GIF image to browser or file */
PHP_FUNCTION(imagegif)
{
	_php_image_output_ctx(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_GDIMG_TYPE_GIF, "GIF", gdImageGifCtx);
}
/* }}} */

#ifdef HAVE_GD_PNG
/* {{{ proto bool imagepng(resource im [, mixed to])
   Output PNG image to browser or file */
PHP_FUNCTION(imagepng)
{
	_php_image_output_ctx(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_GDIMG_TYPE_PNG, "PNG", gdImagePngCtxEx);
}
/* }}} */
#endif /* HAVE_GD_PNG */


#ifdef HAVE_GD_WEBP
/* {{{ proto bool imagewebp(resource im [, mixed to[, int quality]] )
   Output WEBP image to browser or file */
PHP_FUNCTION(imagewebp)
{
	_php_image_output_ctx(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_GDIMG_TYPE_WEBP, "WEBP", gdImageWebpCtx);
}
/* }}} */
#endif /* HAVE_GD_WEBP */


#ifdef HAVE_GD_JPG
/* {{{ proto bool imagejpeg(resource im [, mixed to [, int quality]])
   Output JPEG image to browser or file */
PHP_FUNCTION(imagejpeg)
{
	_php_image_output_ctx(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_GDIMG_TYPE_JPG, "JPEG", gdImageJpegCtx);
}
/* }}} */
#endif /* HAVE_GD_JPG */

/* {{{ proto bool imagewbmp(resource im [, mixed to [, int foreground]])
   Output WBMP image to browser or file */
PHP_FUNCTION(imagewbmp)
{
	_php_image_output_ctx(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_GDIMG_TYPE_WBM, "WBMP", gdImageWBMPCtx);
}
/* }}} */

/* {{{ proto bool imagegd(resource im [, mixed to])
   Output GD image to browser or file */
PHP_FUNCTION(imagegd)
{
	_php_image_output(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_GDIMG_TYPE_GD, "GD", gdImageGd);
}
/* }}} */

/* {{{ proto bool imagegd2(resource im [, mixed to [, int chunk_size [, int type]]])
   Output GD2 image to browser or file */
PHP_FUNCTION(imagegd2)
{
	_php_image_output(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_GDIMG_TYPE_GD2, "GD2", gdImageGd2);
}
/* }}} */

#ifdef HAVE_GD_BMP
/* {{{ proto bool imagebmp(resource im [, mixed to [, bool compressed]])
   Output BMP image to browser or file */
PHP_FUNCTION(imagebmp)
{
	_php_image_output_ctx(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_GDIMG_TYPE_BMP, "BMP", gdImageBmpCtx);
}
/* }}} */
#endif

/* {{{ proto bool imagedestroy(resource im)
   Destroy an image */
PHP_FUNCTION(imagedestroy)
{
	zval *IM;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &IM) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	zend_list_close(Z_RES_P(IM));

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int imagecolorallocate(resource im, int red, int green, int blue)
   Allocate a color for an image */
PHP_FUNCTION(imagecolorallocate)
{
	zval *IM;
	zend_long red, green, blue;
	gdImagePtr im;
	int ct = (-1);

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rlll", &IM, &red, &green, &blue) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	CHECK_RGBA_RANGE(red, Red);
	CHECK_RGBA_RANGE(green, Green);
	CHECK_RGBA_RANGE(blue, Blue);

	ct = gdImageColorAllocate(im, red, green, blue);
	if (ct < 0) {
		RETURN_FALSE;
	}
	RETURN_LONG(ct);
}
/* }}} */

/* {{{ proto void imagepalettecopy(resource dst, resource src)
   Copy the palette from the src image onto the dst image */
PHP_FUNCTION(imagepalettecopy)
{
	zval *dstim, *srcim;
	gdImagePtr dst, src;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rr", &dstim, &srcim) == FAILURE) {
		return;
	}

	if ((dst = (gdImagePtr)zend_fetch_resource(Z_RES_P(dstim), "Image", le_gd)) == NULL) {
		return;
	}

	if ((src = (gdImagePtr)zend_fetch_resource(Z_RES_P(srcim), "Image", le_gd)) == NULL) {
		return;
	}

	gdImagePaletteCopy(dst, src);
}
/* }}} */

/* {{{ proto int imagecolorat(resource im, int x, int y)
   Get the index of the color of a pixel */
PHP_FUNCTION(imagecolorat)
{
	zval *IM;
	zend_long x, y;
	gdImagePtr im;

	ZEND_PARSE_PARAMETERS_START(3, 3)
		Z_PARAM_RESOURCE(IM)
		Z_PARAM_LONG(x)
		Z_PARAM_LONG(y)
	ZEND_PARSE_PARAMETERS_END();

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	if (gdImageTrueColor(im)) {
		if (im->tpixels && gdImageBoundsSafe(im, x, y)) {
			RETURN_LONG(gdImageTrueColorPixel(im, x, y));
		} else {
			php_error_docref(NULL, E_NOTICE, "" ZEND_LONG_FMT "," ZEND_LONG_FMT " is out of bounds", x, y);
			RETURN_FALSE;
		}
	} else {
		if (im->pixels && gdImageBoundsSafe(im, x, y)) {
			RETURN_LONG(im->pixels[y][x]);
		} else {
			php_error_docref(NULL, E_NOTICE, "" ZEND_LONG_FMT "," ZEND_LONG_FMT " is out of bounds", x, y);
			RETURN_FALSE;
		}
	}
}
/* }}} */

/* {{{ proto int imagecolorclosest(resource im, int red, int green, int blue)
   Get the index of the closest color to the specified color */
PHP_FUNCTION(imagecolorclosest)
{
	zval *IM;
	zend_long red, green, blue;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rlll", &IM, &red, &green, &blue) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	CHECK_RGBA_RANGE(red, Red);
	CHECK_RGBA_RANGE(green, Green);
	CHECK_RGBA_RANGE(blue, Blue);

	RETURN_LONG(gdImageColorClosest(im, red, green, blue));
}
/* }}} */

/* {{{ proto int imagecolorclosesthwb(resource im, int red, int green, int blue)
   Get the index of the color which has the hue, white and blackness nearest to the given color */
PHP_FUNCTION(imagecolorclosesthwb)
{
	zval *IM;
	zend_long red, green, blue;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rlll", &IM, &red, &green, &blue) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	CHECK_RGBA_RANGE(red, Red);
	CHECK_RGBA_RANGE(green, Green);
	CHECK_RGBA_RANGE(blue, Blue);

	RETURN_LONG(gdImageColorClosestHWB(im, red, green, blue));
}
/* }}} */

/* {{{ proto bool imagecolordeallocate(resource im, int index)
   De-allocate a color for an image */
PHP_FUNCTION(imagecolordeallocate)
{
	zval *IM;
	zend_long index;
	int col;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rl", &IM, &index) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	/* We can return right away for a truecolor image as deallocating colours is meaningless here */
	if (gdImageTrueColor(im)) {
		RETURN_TRUE;
	}

	col = index;

	if (col >= 0 && col < gdImageColorsTotal(im)) {
		gdImageColorDeallocate(im, col);
		RETURN_TRUE;
	} else {
		php_error_docref(NULL, E_WARNING, "Color index %d out of range",	col);
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto int imagecolorresolve(resource im, int red, int green, int blue)
   Get the index of the specified color or its closest possible alternative */
PHP_FUNCTION(imagecolorresolve)
{
	zval *IM;
	zend_long red, green, blue;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rlll", &IM, &red, &green, &blue) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	CHECK_RGBA_RANGE(red, Red);
	CHECK_RGBA_RANGE(green, Green);
	CHECK_RGBA_RANGE(blue, Blue);

	RETURN_LONG(gdImageColorResolve(im, red, green, blue));
}
/* }}} */

/* {{{ proto int imagecolorexact(resource im, int red, int green, int blue)
   Get the index of the specified color */
PHP_FUNCTION(imagecolorexact)
{
	zval *IM;
	zend_long red, green, blue;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rlll", &IM, &red, &green, &blue) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	CHECK_RGBA_RANGE(red, Red);
	CHECK_RGBA_RANGE(green, Green);
	CHECK_RGBA_RANGE(blue, Blue);

	RETURN_LONG(gdImageColorExact(im, red, green, blue));
}
/* }}} */

/* {{{ proto bool imagecolorset(resource im, int col, int red, int green, int blue)
   Set the color for the specified palette index */
PHP_FUNCTION(imagecolorset)
{
	zval *IM;
	zend_long color, red, green, blue, alpha = 0;
	int col;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rllll|l", &IM, &color, &red, &green, &blue, &alpha) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	CHECK_RGBA_RANGE(red, Red);
	CHECK_RGBA_RANGE(green, Green);
	CHECK_RGBA_RANGE(blue, Blue);
	CHECK_RGBA_RANGE(alpha, Alpha);

	col = color;

	if (col >= 0 && col < gdImageColorsTotal(im)) {
		im->red[col]   = red;
		im->green[col] = green;
		im->blue[col]  = blue;
		im->alpha[col]  = alpha;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto array imagecolorsforindex(resource im, int col)
   Get the colors for an index */
PHP_FUNCTION(imagecolorsforindex)
{
	zval *IM;
	zend_long index;
	int col;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rl", &IM, &index) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	col = index;

	if ((col >= 0 && gdImageTrueColor(im)) || (!gdImageTrueColor(im) && col >= 0 && col < gdImageColorsTotal(im))) {
		array_init(return_value);

		add_assoc_long(return_value,"red",  gdImageRed(im,col));
		add_assoc_long(return_value,"green", gdImageGreen(im,col));
		add_assoc_long(return_value,"blue", gdImageBlue(im,col));
		add_assoc_long(return_value,"alpha", gdImageAlpha(im,col));
	} else {
		php_error_docref(NULL, E_WARNING, "Color index %d out of range", col);
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool imagegammacorrect(resource im, float inputgamma, float outputgamma)
   Apply a gamma correction to a GD image */
PHP_FUNCTION(imagegammacorrect)
{
	zval *IM;
	gdImagePtr im;
	int i;
	double input, output, gamma;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rdd", &IM, &input, &output) == FAILURE) {
		return;
	}

	if ( input <= 0.0 || output <= 0.0 ) {
		php_error_docref(NULL, E_WARNING, "Gamma values should be positive");
		RETURN_FALSE;
	}

	gamma = input / output;

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	if (gdImageTrueColor(im))	{
		int x, y, c;

		for (y = 0; y < gdImageSY(im); y++)	{
			for (x = 0; x < gdImageSX(im); x++)	{
				c = gdImageGetPixel(im, x, y);
				gdImageSetPixel(im, x, y,
					gdTrueColorAlpha(
						(int) ((pow((gdTrueColorGetRed(c)   / 255.0), gamma) * 255) + .5),
						(int) ((pow((gdTrueColorGetGreen(c) / 255.0), gamma) * 255) + .5),
						(int) ((pow((gdTrueColorGetBlue(c)  / 255.0), gamma) * 255) + .5),
						gdTrueColorGetAlpha(c)
					)
				);
			}
		}
		RETURN_TRUE;
	}

	for (i = 0; i < gdImageColorsTotal(im); i++) {
		im->red[i]   = (int)((pow((im->red[i]   / 255.0), gamma) * 255) + .5);
		im->green[i] = (int)((pow((im->green[i] / 255.0), gamma) * 255) + .5);
		im->blue[i]  = (int)((pow((im->blue[i]  / 255.0), gamma) * 255) + .5);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool imagesetpixel(resource im, int x, int y, int col)
   Set a single pixel */
PHP_FUNCTION(imagesetpixel)
{
	zval *IM;
	zend_long x, y, col;
	gdImagePtr im;

	ZEND_PARSE_PARAMETERS_START(4, 4)
		Z_PARAM_RESOURCE(IM)
		Z_PARAM_LONG(x)
		Z_PARAM_LONG(y)
		Z_PARAM_LONG(col)
	ZEND_PARSE_PARAMETERS_END();

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	gdImageSetPixel(im, x, y, col);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool imageline(resource im, int x1, int y1, int x2, int y2, int col)
   Draw a line */
PHP_FUNCTION(imageline)
{
	zval *IM;
	zend_long x1, y1, x2, y2, col;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rlllll", &IM, &x1, &y1, &x2, &y2, &col) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	if (im->AA) {
		gdImageSetAntiAliased(im, col);
		col = gdAntiAliased;
	}
	gdImageLine(im, x1, y1, x2, y2, col);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool imagedashedline(resource im, int x1, int y1, int x2, int y2, int col)
   Draw a dashed line */
PHP_FUNCTION(imagedashedline)
{
	zval *IM;
	zend_long x1, y1, x2, y2, col;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rlllll", &IM, &x1, &y1, &x2, &y2, &col) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	gdImageDashedLine(im, x1, y1, x2, y2, col);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool imagerectangle(resource im, int x1, int y1, int x2, int y2, int col)
   Draw a rectangle */
PHP_FUNCTION(imagerectangle)
{
	zval *IM;
	zend_long x1, y1, x2, y2, col;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rlllll", &IM, &x1, &y1, &x2, &y2, &col) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	gdImageRectangle(im, x1, y1, x2, y2, col);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool imagefilledrectangle(resource im, int x1, int y1, int x2, int y2, int col)
   Draw a filled rectangle */
PHP_FUNCTION(imagefilledrectangle)
{
	zval *IM;
	zend_long x1, y1, x2, y2, col;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rlllll", &IM, &x1, &y1, &x2, &y2, &col) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}
	gdImageFilledRectangle(im, x1, y1, x2, y2, col);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool imagearc(resource im, int cx, int cy, int w, int h, int s, int e, int col)
   Draw a partial ellipse */
PHP_FUNCTION(imagearc)
{
	zval *IM;
	zend_long cx, cy, w, h, ST, E, col;
	gdImagePtr im;
	int e, st;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rlllllll", &IM, &cx, &cy, &w, &h, &ST, &E, &col) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	e = E;
	if (e < 0) {
		e %= 360;
	}

	st = ST;
	if (st < 0) {
		st %= 360;
	}

	gdImageArc(im, cx, cy, w, h, st, e, col);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool imageellipse(resource im, int cx, int cy, int w, int h, int color)
   Draw an ellipse */
PHP_FUNCTION(imageellipse)
{
	zval *IM;
	zend_long cx, cy, w, h, color;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rlllll", &IM, &cx, &cy, &w, &h, &color) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	gdImageEllipse(im, cx, cy, w, h, color);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool imagefilltoborder(resource im, int x, int y, int border, int col)
   Flood fill to specific color */
PHP_FUNCTION(imagefilltoborder)
{
	zval *IM;
	zend_long x, y, border, col;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rllll", &IM, &x, &y, &border, &col) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	gdImageFillToBorder(im, x, y, border, col);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool imagefill(resource im, int x, int y, int col)
   Flood fill */
PHP_FUNCTION(imagefill)
{
	zval *IM;
	zend_long x, y, col;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rlll", &IM, &x, &y, &col) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	gdImageFill(im, x, y, col);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int imagecolorstotal(resource im)
   Find out the number of colors in an image's palette */
PHP_FUNCTION(imagecolorstotal)
{
	zval *IM;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &IM) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	RETURN_LONG(gdImageColorsTotal(im));
}
/* }}} */

/* {{{ proto int imagecolortransparent(resource im [, int col])
   Define a color as transparent */
PHP_FUNCTION(imagecolortransparent)
{
	zval *IM;
	zend_long COL = 0;
	gdImagePtr im;
	int argc = ZEND_NUM_ARGS();

	if (zend_parse_parameters(argc, "r|l", &IM, &COL) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	if (argc > 1) {
		gdImageColorTransparent(im, COL);
	}

	RETURN_LONG(gdImageGetTransparent(im));
}
/* }}} */

/* {{{ proto int imageinterlace(resource im [, int interlace])
   Enable or disable interlace */
PHP_FUNCTION(imageinterlace)
{
	zval *IM;
	int argc = ZEND_NUM_ARGS();
	zend_long INT = 0;
	gdImagePtr im;

	if (zend_parse_parameters(argc, "r|l", &IM, &INT) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	if (argc > 1) {
		gdImageInterlace(im, INT);
	}

	RETURN_LONG(gdImageGetInterlaced(im));
}
/* }}} */

/* {{{ php_imagepolygon
   arg = -1 open polygon
   arg = 0  normal polygon
   arg = 1  filled polygon */
/* im, points, num_points, col */
static void php_imagepolygon(INTERNAL_FUNCTION_PARAMETERS, int filled)
{
	zval *IM, *POINTS;
	zend_long NPOINTS, COL;
	zval *var = NULL;
	gdImagePtr im;
	gdPointPtr points;
	int npoints, col, nelem, i;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rall", &IM, &POINTS, &NPOINTS, &COL) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	npoints = NPOINTS;
	col = COL;

	nelem = zend_hash_num_elements(Z_ARRVAL_P(POINTS));
	if (nelem < 6) {
		php_error_docref(NULL, E_WARNING, "You must have at least 3 points in your array");
		RETURN_FALSE;
	}
	if (npoints <= 0) {
		php_error_docref(NULL, E_WARNING, "You must give a positive number of points");
		RETURN_FALSE;
	}
	if (nelem < npoints * 2) {
		php_error_docref(NULL, E_WARNING, "Trying to use %d points in array with only %d points", npoints, nelem/2);
		RETURN_FALSE;
	}

	points = (gdPointPtr) safe_emalloc(npoints, sizeof(gdPoint), 0);

	for (i = 0; i < npoints; i++) {
		if ((var = zend_hash_index_find(Z_ARRVAL_P(POINTS), (i * 2))) != NULL) {
			points[i].x = zval_get_long(var);
		}
		if ((var = zend_hash_index_find(Z_ARRVAL_P(POINTS), (i * 2) + 1)) != NULL) {
			points[i].y = zval_get_long(var);
		}
	}

	if (im->AA) {
		gdImageSetAntiAliased(im, col);
		col = gdAntiAliased;
	}
	switch (filled) {
		case -1:
			gdImageOpenPolygon(im, points, npoints, col);
			break;
		case 0:
			gdImagePolygon(im, points, npoints, col);
			break;
		case 1:
			gdImageFilledPolygon(im, points, npoints, col);
			break;
	}

	efree(points);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool imagepolygon(resource im, array point, int num_points, int col)
   Draw a polygon */
PHP_FUNCTION(imagepolygon)
{
	php_imagepolygon(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto bool imageopenpolygon(resource im, array point, int num_points, int col)
   Draw a polygon */
PHP_FUNCTION(imageopenpolygon)
{
	php_imagepolygon(INTERNAL_FUNCTION_PARAM_PASSTHRU, -1);
}
/* }}} */

/* {{{ proto bool imagefilledpolygon(resource im, array point, int num_points, int col)
   Draw a filled polygon */
PHP_FUNCTION(imagefilledpolygon)
{
	php_imagepolygon(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ php_find_gd_font
 */
static gdFontPtr php_find_gd_font(int size)
{
	gdFontPtr font;

	switch (size) {
		case 1:
			font = gdFontTiny;
			break;
		case 2:
			font = gdFontSmall;
			break;
		case 3:
			font = gdFontMediumBold;
			break;
		case 4:
			font = gdFontLarge;
			break;
		case 5:
			font = gdFontGiant;
			break;
		default: {
			 zval *zv = zend_hash_index_find(&EG(regular_list), size - 5);
			 if (!zv || (Z_RES_P(zv))->type != le_gd_font) {
				 if (size < 1) {
					 font = gdFontTiny;
				 } else {
					 font = gdFontGiant;
				 }
			 } else {
				 font = (gdFontPtr)Z_RES_P(zv)->ptr;
			 }
		 }
		 break;
	}

	return font;
}
/* }}} */

/* {{{ php_imagefontsize
 * arg = 0  ImageFontWidth
 * arg = 1  ImageFontHeight
 */
static void php_imagefontsize(INTERNAL_FUNCTION_PARAMETERS, int arg)
{
	zend_long SIZE;
	gdFontPtr font;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &SIZE) == FAILURE) {
		return;
	}

	font = php_find_gd_font(SIZE);
	RETURN_LONG(arg ? font->h : font->w);
}
/* }}} */

/* {{{ proto int imagefontwidth(int font)
   Get font width */
PHP_FUNCTION(imagefontwidth)
{
	php_imagefontsize(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto int imagefontheight(int font)
   Get font height */
PHP_FUNCTION(imagefontheight)
{
	php_imagefontsize(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ php_gdimagecharup
 * workaround for a bug in gd 1.2 */
static void php_gdimagecharup(gdImagePtr im, gdFontPtr f, int x, int y, int c, int color)
{
	int cx, cy, px, py, fline;
	cx = 0;
	cy = 0;

	if ((c < f->offset) || (c >= (f->offset + f->nchars))) {
		return;
	}

	fline = (c - f->offset) * f->h * f->w;
	for (py = y; (py > (y - f->w)); py--) {
		for (px = x; (px < (x + f->h)); px++) {
			if (f->data[fline + cy * f->w + cx]) {
				gdImageSetPixel(im, px, py, color);
			}
			cy++;
		}
		cy = 0;
		cx++;
	}
}
/* }}} */

/* {{{ php_imagechar
 * arg = 0  ImageChar
 * arg = 1  ImageCharUp
 * arg = 2  ImageString
 * arg = 3  ImageStringUp
 */
static void php_imagechar(INTERNAL_FUNCTION_PARAMETERS, int mode)
{
	zval *IM;
	zend_long SIZE, X, Y, COL;
	char *C;
	size_t C_len;
	gdImagePtr im;
	int ch = 0, col, x, y, size, i, l = 0;
	unsigned char *str = NULL;
	gdFontPtr font;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rlllsl", &IM, &SIZE, &X, &Y, &C, &C_len, &COL) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	col = COL;

	if (mode < 2) {
		ch = (int)((unsigned char)*C);
	} else {
		str = (unsigned char *) estrndup(C, C_len);
		l = strlen((char *)str);
	}

	y = Y;
	x = X;
	size = SIZE;

	font = php_find_gd_font(size);

	switch (mode) {
		case 0:
			gdImageChar(im, font, x, y, ch, col);
			break;
		case 1:
			php_gdimagecharup(im, font, x, y, ch, col);
			break;
		case 2:
			for (i = 0; (i < l); i++) {
				gdImageChar(im, font, x, y, (int) ((unsigned char) str[i]), col);
				x += font->w;
			}
			break;
		case 3: {
			for (i = 0; (i < l); i++) {
				/* php_gdimagecharup(im, font, x, y, (int) str[i], col); */
				gdImageCharUp(im, font, x, y, (int) str[i], col);
				y -= font->w;
			}
			break;
		}
	}
	if (str) {
		efree(str);
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool imagechar(resource im, int font, int x, int y, string c, int col)
   Draw a character */
PHP_FUNCTION(imagechar)
{
	php_imagechar(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto bool imagecharup(resource im, int font, int x, int y, string c, int col)
   Draw a character rotated 90 degrees counter-clockwise */
PHP_FUNCTION(imagecharup)
{
	php_imagechar(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto bool imagestring(resource im, int font, int x, int y, string str, int col)
   Draw a string horizontally */
PHP_FUNCTION(imagestring)
{
	php_imagechar(INTERNAL_FUNCTION_PARAM_PASSTHRU, 2);
}
/* }}} */

/* {{{ proto bool imagestringup(resource im, int font, int x, int y, string str, int col)
   Draw a string vertically - rotated 90 degrees counter-clockwise */
PHP_FUNCTION(imagestringup)
{
	php_imagechar(INTERNAL_FUNCTION_PARAM_PASSTHRU, 3);
}
/* }}} */

/* {{{ proto bool imagecopy(resource dst_im, resource src_im, int dst_x, int dst_y, int src_x, int src_y, int src_w, int src_h)
   Copy part of an image */
PHP_FUNCTION(imagecopy)
{
	zval *SIM, *DIM;
	zend_long SX, SY, SW, SH, DX, DY;
	gdImagePtr im_dst, im_src;
	int srcH, srcW, srcY, srcX, dstY, dstX;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rrllllll", &DIM, &SIM, &DX, &DY, &SX, &SY, &SW, &SH) == FAILURE) {
		return;
	}

	if ((im_dst = (gdImagePtr)zend_fetch_resource(Z_RES_P(DIM), "Image", le_gd)) == NULL) {
		return;
	}

	if ((im_src = (gdImagePtr)zend_fetch_resource(Z_RES_P(SIM), "Image", le_gd)) == NULL) {
		return;
	}

	srcX = SX;
	srcY = SY;
	srcH = SH;
	srcW = SW;
	dstX = DX;
	dstY = DY;

	gdImageCopy(im_dst, im_src, dstX, dstY, srcX, srcY, srcW, srcH);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool imagecopymerge(resource dst_im, resource src_im, int dst_x, int dst_y, int src_x, int src_y, int src_w, int src_h, int pct)
   Merge one part of an image with another */
PHP_FUNCTION(imagecopymerge)
{
	zval *SIM, *DIM;
	zend_long SX, SY, SW, SH, DX, DY, PCT;
	gdImagePtr im_dst, im_src;
	int srcH, srcW, srcY, srcX, dstY, dstX, pct;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rrlllllll", &DIM, &SIM, &DX, &DY, &SX, &SY, &SW, &SH, &PCT) == FAILURE) {
		return;
	}

	if ((im_dst = (gdImagePtr)zend_fetch_resource(Z_RES_P(DIM), "Image", le_gd)) == NULL) {
		return;
	}

	if ((im_src = (gdImagePtr)zend_fetch_resource(Z_RES_P(SIM), "Image", le_gd)) == NULL) {
		return;
	}

	srcX = SX;
	srcY = SY;
	srcH = SH;
	srcW = SW;
	dstX = DX;
	dstY = DY;
	pct  = PCT;

	gdImageCopyMerge(im_dst, im_src, dstX, dstY, srcX, srcY, srcW, srcH, pct);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool imagecopymergegray(resource dst_im, resource src_im, int dst_x, int dst_y, int src_x, int src_y, int src_w, int src_h, int pct)
   Merge one part of an image with another */
PHP_FUNCTION(imagecopymergegray)
{
	zval *SIM, *DIM;
	zend_long SX, SY, SW, SH, DX, DY, PCT;
	gdImagePtr im_dst, im_src;
	int srcH, srcW, srcY, srcX, dstY, dstX, pct;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rrlllllll", &DIM, &SIM, &DX, &DY, &SX, &SY, &SW, &SH, &PCT) == FAILURE) {
		return;
	}

	if ((im_dst = (gdImagePtr)zend_fetch_resource(Z_RES_P(DIM), "Image", le_gd)) == NULL) {
		return;
	}

	if ((im_src = (gdImagePtr)zend_fetch_resource(Z_RES_P(SIM), "Image", le_gd)) == NULL) {
		return;
	}

	srcX = SX;
	srcY = SY;
	srcH = SH;
	srcW = SW;
	dstX = DX;
	dstY = DY;
	pct  = PCT;

	gdImageCopyMergeGray(im_dst, im_src, dstX, dstY, srcX, srcY, srcW, srcH, pct);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool imagecopyresized(resource dst_im, resource src_im, int dst_x, int dst_y, int src_x, int src_y, int dst_w, int dst_h, int src_w, int src_h)
   Copy and resize part of an image */
PHP_FUNCTION(imagecopyresized)
{
	zval *SIM, *DIM;
	zend_long SX, SY, SW, SH, DX, DY, DW, DH;
	gdImagePtr im_dst, im_src;
	int srcH, srcW, dstH, dstW, srcY, srcX, dstY, dstX;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rrllllllll", &DIM, &SIM, &DX, &DY, &SX, &SY, &DW, &DH, &SW, &SH) == FAILURE) {
		return;
	}

	if ((im_dst = (gdImagePtr)zend_fetch_resource(Z_RES_P(DIM), "Image", le_gd)) == NULL) {
		return;
	}

	if ((im_src = (gdImagePtr)zend_fetch_resource(Z_RES_P(SIM), "Image", le_gd)) == NULL) {
		return;
	}

	srcX = SX;
	srcY = SY;
	srcH = SH;
	srcW = SW;
	dstX = DX;
	dstY = DY;
	dstH = DH;
	dstW = DW;

	if (dstW <= 0 || dstH <= 0 || srcW <= 0 || srcH <= 0) {
		php_error_docref(NULL, E_WARNING, "Invalid image dimensions");
		RETURN_FALSE;
	}

	gdImageCopyResized(im_dst, im_src, dstX, dstY, srcX, srcY, dstW, dstH, srcW, srcH);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int imagesx(resource im)
   Get image width */
PHP_FUNCTION(imagesx)
{
	zval *IM;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &IM) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	RETURN_LONG(gdImageSX(im));
}
/* }}} */

/* {{{ proto int imagesy(resource im)
   Get image height */
PHP_FUNCTION(imagesy)
{
	zval *IM;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &IM) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	RETURN_LONG(gdImageSY(im));
}
/* }}} */

/* {{{ proto bool imagesetclip(resource im, int x1, int y1, int x2, int y2)
   Set the clipping rectangle. */
PHP_FUNCTION(imagesetclip)
{
	zval *im_zval;
	gdImagePtr im;
	zend_long x1, y1, x2, y2;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rllll", &im_zval, &x1, &y1, &x2, &y2) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(im_zval), "Image", le_gd)) == NULL) {
		return;
	}

	gdImageSetClip(im, x1, y1, x2, y2);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto array imagegetclip(resource im)
   Get the clipping rectangle. */
PHP_FUNCTION(imagegetclip)
{
	zval *im_zval;
	gdImagePtr im;
	int x1, y1, x2, y2;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &im_zval) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(im_zval), "Image", le_gd)) == NULL) {
		return;
	}

	gdImageGetClip(im, &x1, &y1, &x2, &y2);

	array_init(return_value);
	add_next_index_long(return_value, x1);
	add_next_index_long(return_value, y1);
	add_next_index_long(return_value, x2);
	add_next_index_long(return_value, y2);
}
/* }}} */

#define TTFTEXT_DRAW 0
#define TTFTEXT_BBOX 1

#ifdef HAVE_GD_FREETYPE
/* {{{ proto array imageftbbox(float size, float angle, string font_file, string text [, array extrainfo])
   Give the bounding box of a text using fonts via freetype2 */
PHP_FUNCTION(imageftbbox)
{
	php_imagettftext_common(INTERNAL_FUNCTION_PARAM_PASSTHRU, TTFTEXT_BBOX, 1);
}
/* }}} */

/* {{{ proto array imagefttext(resource im, float size, float angle, int x, int y, int col, string font_file, string text [, array extrainfo])
   Write text to the image using fonts via freetype2 */
PHP_FUNCTION(imagefttext)
{
	php_imagettftext_common(INTERNAL_FUNCTION_PARAM_PASSTHRU, TTFTEXT_DRAW, 1);
}
/* }}} */

/* {{{ proto array imagettfbbox(float size, float angle, string font_file, string text)
   Give the bounding box of a text using TrueType fonts */
PHP_FUNCTION(imagettfbbox)
{
	php_imagettftext_common(INTERNAL_FUNCTION_PARAM_PASSTHRU, TTFTEXT_BBOX, 0);
}
/* }}} */

/* {{{ proto array imagettftext(resource im, float size, float angle, int x, int y, int col, string font_file, string text)
   Write text to the image using a TrueType font */
PHP_FUNCTION(imagettftext)
{
	php_imagettftext_common(INTERNAL_FUNCTION_PARAM_PASSTHRU, TTFTEXT_DRAW, 0);
}
/* }}} */

/* {{{ php_imagettftext_common
 */
static void php_imagettftext_common(INTERNAL_FUNCTION_PARAMETERS, int mode, int extended)
{
	zval *IM, *EXT = NULL;
	gdImagePtr im=NULL;
	zend_long col = -1, x = 0, y = 0;
	size_t str_len, fontname_len;
	int i, brect[8];
	double ptsize, angle;
	char *str = NULL, *fontname = NULL;
	char *error = NULL;
	int argc = ZEND_NUM_ARGS();
	gdFTStringExtra strex = {0};

	if (mode == TTFTEXT_BBOX) {
		if (argc < 4 || argc > ((extended) ? 5 : 4)) {
			ZEND_WRONG_PARAM_COUNT();
		} else if (zend_parse_parameters(argc, "ddss|a", &ptsize, &angle, &fontname, &fontname_len, &str, &str_len, &EXT) == FAILURE) {
			return;
		}
	} else {
		if (argc < 8 || argc > ((extended) ? 9 : 8)) {
			ZEND_WRONG_PARAM_COUNT();
		} else if (zend_parse_parameters(argc, "rddlllss|a", &IM, &ptsize, &angle, &x, &y, &col, &fontname, &fontname_len, &str, &str_len, &EXT) == FAILURE) {
			return;
		}
		if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
			return;
		}
	}

	/* convert angle to radians */
	angle = angle * (M_PI/180);

	if (extended && EXT) {	/* parse extended info */
		zval *item;
		zend_string *key;

		/* walk the assoc array */
		ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(EXT), key, item) {
			if (key == NULL) {
				continue;
			}
			if (strcmp("linespacing", ZSTR_VAL(key)) == 0) {
				strex.flags |= gdFTEX_LINESPACE;
				strex.linespacing = zval_get_double(item);
			}
		} ZEND_HASH_FOREACH_END();
	}

#ifdef VIRTUAL_DIR
	{
		char tmp_font_path[MAXPATHLEN];

		if (!VCWD_REALPATH(fontname, tmp_font_path)) {
			fontname = NULL;
		}
	}
#endif /* VIRTUAL_DIR */

	PHP_GD_CHECK_OPEN_BASEDIR(fontname, "Invalid font filename");

	if (extended) {
		error = gdImageStringFTEx(im, brect, col, fontname, ptsize, angle, x, y, str, &strex);
	} else {
		error = gdImageStringFT(im, brect, col, fontname, ptsize, angle, x, y, str);
	}

	if (error) {
		php_error_docref(NULL, E_WARNING, "%s", error);
		RETURN_FALSE;
	}

	array_init(return_value);

	/* return array with the text's bounding box */
	for (i = 0; i < 8; i++) {
		add_next_index_long(return_value, brect[i]);
	}
}
/* }}} */
#endif /* HAVE_GD_FREETYPE */

/* Section Filters */
#define PHP_GD_SINGLE_RES	\
	zval *SIM;	\
	gdImagePtr im_src;	\
	if (zend_parse_parameters(1, "r", &SIM) == FAILURE) {	\
		return;	\
	}	\
	if ((im_src = (gdImagePtr)zend_fetch_resource(Z_RES_P(SIM), "Image", le_gd)) == NULL) {	\
		return;	\
	}

static void php_image_filter_negate(INTERNAL_FUNCTION_PARAMETERS)
{
	PHP_GD_SINGLE_RES

	if (gdImageNegate(im_src) == 1) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void php_image_filter_grayscale(INTERNAL_FUNCTION_PARAMETERS)
{
	PHP_GD_SINGLE_RES

	if (gdImageGrayScale(im_src) == 1) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void php_image_filter_brightness(INTERNAL_FUNCTION_PARAMETERS)
{
	zval *SIM;
	gdImagePtr im_src;
	zend_long brightness, tmp;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "zll", &SIM, &tmp, &brightness) == FAILURE) {
		return;
	}

	if ((im_src = (gdImagePtr)zend_fetch_resource(Z_RES_P(SIM), "Image", le_gd)) == NULL) {
		return;
	}

	if (gdImageBrightness(im_src, (int)brightness) == 1) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void php_image_filter_contrast(INTERNAL_FUNCTION_PARAMETERS)
{
	zval *SIM;
	gdImagePtr im_src;
	zend_long contrast, tmp;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rll", &SIM, &tmp, &contrast) == FAILURE) {
		return;
	}

	if ((im_src = (gdImagePtr)zend_fetch_resource(Z_RES_P(SIM), "Image", le_gd)) == NULL) {
		return;
	}

	if (gdImageContrast(im_src, (int)contrast) == 1) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void php_image_filter_colorize(INTERNAL_FUNCTION_PARAMETERS)
{
	zval *SIM;
	gdImagePtr im_src;
	zend_long r,g,b,tmp;
	zend_long a = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rllll|l", &SIM, &tmp, &r, &g, &b, &a) == FAILURE) {
		return;
	}

	if ((im_src = (gdImagePtr)zend_fetch_resource(Z_RES_P(SIM), "Image", le_gd)) == NULL) {
		return;
	}

	if (gdImageColor(im_src, (int) r, (int) g, (int) b, (int) a) == 1) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void php_image_filter_edgedetect(INTERNAL_FUNCTION_PARAMETERS)
{
	PHP_GD_SINGLE_RES

	if (gdImageEdgeDetectQuick(im_src) == 1) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void php_image_filter_emboss(INTERNAL_FUNCTION_PARAMETERS)
{
	PHP_GD_SINGLE_RES

	if (gdImageEmboss(im_src) == 1) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void php_image_filter_gaussian_blur(INTERNAL_FUNCTION_PARAMETERS)
{
	PHP_GD_SINGLE_RES

	if (gdImageGaussianBlur(im_src) == 1) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void php_image_filter_selective_blur(INTERNAL_FUNCTION_PARAMETERS)
{
	PHP_GD_SINGLE_RES

	if (gdImageSelectiveBlur(im_src) == 1) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void php_image_filter_mean_removal(INTERNAL_FUNCTION_PARAMETERS)
{
	PHP_GD_SINGLE_RES

	if (gdImageMeanRemoval(im_src) == 1) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void php_image_filter_smooth(INTERNAL_FUNCTION_PARAMETERS)
{
	zval *SIM;
	zend_long tmp;
	gdImagePtr im_src;
	double weight;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rld", &SIM, &tmp, &weight) == FAILURE) {
		return;
	}

	if ((im_src = (gdImagePtr)zend_fetch_resource(Z_RES_P(SIM), "Image", le_gd)) == NULL) {
		return;
	}

	if (gdImageSmooth(im_src, (float)weight)==1) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void php_image_filter_pixelate(INTERNAL_FUNCTION_PARAMETERS)
{
	zval *IM;
	gdImagePtr im;
	zend_long tmp, blocksize;
	zend_bool mode = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rll|b", &IM, &tmp, &blocksize, &mode) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	if (gdImagePixelate(im, (int) blocksize, (const unsigned int) mode)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void php_image_filter_scatter(INTERNAL_FUNCTION_PARAMETERS)
{
	zval *IM;
	zval *hash_colors = NULL;
	gdImagePtr im;
	zend_long tmp;
	zend_long scatter_sub, scatter_plus;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rlll|a", &IM, &tmp, &scatter_sub, &scatter_plus, &hash_colors) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	if (hash_colors) {
		uint32_t i = 0;
		uint32_t num_colors = zend_hash_num_elements(Z_ARRVAL_P(hash_colors));
		zval *color;
		int *colors;

		if (num_colors == 0) {
			RETURN_BOOL(gdImageScatter(im, (int)scatter_sub, (int)scatter_plus));
		}

		colors = emalloc(num_colors * sizeof(int));

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(hash_colors), color) {
			*(colors + i++) = (int) zval_get_long(color);
		} ZEND_HASH_FOREACH_END();

		RETVAL_BOOL(gdImageScatterColor(im, (int)scatter_sub, (int)scatter_plus, colors, num_colors));

		efree(colors);
	} else {
		RETURN_BOOL(gdImageScatter(im, (int) scatter_sub, (int) scatter_plus));
	}
}

/* {{{ proto bool imagefilter(resource src_im, int filtertype[, int arg1 [, int arg2 [, int arg3 [, int arg4 ]]]] )
   Applies Filter an image using a custom angle */
PHP_FUNCTION(imagefilter)
{
	zval *tmp;

	typedef void (*image_filter)(INTERNAL_FUNCTION_PARAMETERS);
	zend_long filtertype;
	image_filter filters[] =
	{
		php_image_filter_negate ,
		php_image_filter_grayscale,
		php_image_filter_brightness,
		php_image_filter_contrast,
		php_image_filter_colorize,
		php_image_filter_edgedetect,
		php_image_filter_emboss,
		php_image_filter_gaussian_blur,
		php_image_filter_selective_blur,
		php_image_filter_mean_removal,
		php_image_filter_smooth,
		php_image_filter_pixelate,
		php_image_filter_scatter
	};

	if (ZEND_NUM_ARGS() < 2 || ZEND_NUM_ARGS() > IMAGE_FILTER_MAX_ARGS) {
		WRONG_PARAM_COUNT;
	} else if (zend_parse_parameters(2, "rl", &tmp, &filtertype) == FAILURE) {
		return;
	}

	if (filtertype >= 0 && filtertype <= IMAGE_FILTER_MAX) {
		filters[filtertype](INTERNAL_FUNCTION_PARAM_PASSTHRU);
	}
}
/* }}} */

/* {{{ proto resource imageconvolution(resource src_im, array matrix3x3, double div, double offset)
   Apply a 3x3 convolution matrix, using coefficient div and offset */
PHP_FUNCTION(imageconvolution)
{
	zval *SIM, *hash_matrix;
	zval *var = NULL, *var2 = NULL;
	gdImagePtr im_src = NULL;
	double div, offset;
	int nelem, i, j, res;
	float matrix[3][3] = {{0,0,0}, {0,0,0}, {0,0,0}};

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "radd", &SIM, &hash_matrix, &div, &offset) == FAILURE) {
		return;
	}

	if ((im_src = (gdImagePtr)zend_fetch_resource(Z_RES_P(SIM), "Image", le_gd)) == NULL) {
		return;
	}

	nelem = zend_hash_num_elements(Z_ARRVAL_P(hash_matrix));
	if (nelem != 3) {
		php_error_docref(NULL, E_WARNING, "You must have 3x3 array");
		RETURN_FALSE;
	}

	for (i=0; i<3; i++) {
		if ((var = zend_hash_index_find(Z_ARRVAL_P(hash_matrix), (i))) != NULL && Z_TYPE_P(var) == IS_ARRAY) {
			if (zend_hash_num_elements(Z_ARRVAL_P(var)) != 3 ) {
				php_error_docref(NULL, E_WARNING, "You must have 3x3 array");
				RETURN_FALSE;
			}

			for (j=0; j<3; j++) {
				if ((var2 = zend_hash_index_find(Z_ARRVAL_P(var), j)) != NULL) {
					matrix[i][j] = (float) zval_get_double(var2);
				} else {
					php_error_docref(NULL, E_WARNING, "You must have a 3x3 matrix");
					RETURN_FALSE;
				}
			}
		}
	}
	res = gdImageConvolution(im_src, matrix, (float)div, (float)offset);

	if (res) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */
/* End section: Filters */

/* {{{ proto bool imageflip(resource im, int mode)
   Flip an image (in place) horizontally, vertically or both directions. */
PHP_FUNCTION(imageflip)
{
	zval *IM;
	zend_long mode;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rl", &IM, &mode) == FAILURE)  {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	switch (mode) {
		case GD_FLIP_VERTICAL:
			gdImageFlipVertical(im);
			break;

		case GD_FLIP_HORINZONTAL:
			gdImageFlipHorizontal(im);
			break;

		case GD_FLIP_BOTH:
			gdImageFlipBoth(im);
			break;

		default:
			php_error_docref(NULL, E_WARNING, "Unknown flip mode");
			RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool imageantialias(resource im, bool on)
   Should antialiased functions used or not*/
PHP_FUNCTION(imageantialias)
{
	zval *IM;
	zend_bool alias;
	gdImagePtr im;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rb", &IM, &alias) == FAILURE) {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	if (im->trueColor) {
		im->AA = alias;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto resource imagecrop(resource im, array rect)
   Crop an image using the given coordinates and size, x, y, width and height. */
PHP_FUNCTION(imagecrop)
{
	zval *IM;
	gdImagePtr im;
	gdImagePtr im_crop;
	gdRect rect;
	zval *z_rect;
	zval *tmp;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ra", &IM, &z_rect) == FAILURE)  {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	if ((tmp = zend_hash_str_find(Z_ARRVAL_P(z_rect), "x", sizeof("x") -1)) != NULL) {
		rect.x = zval_get_long(tmp);
	} else {
		php_error_docref(NULL, E_WARNING, "Missing x position");
		RETURN_FALSE;
	}

	if ((tmp = zend_hash_str_find(Z_ARRVAL_P(z_rect), "y", sizeof("y") - 1)) != NULL) {
		rect.y = zval_get_long(tmp);
	} else {
		php_error_docref(NULL, E_WARNING, "Missing y position");
		RETURN_FALSE;
	}

	if ((tmp = zend_hash_str_find(Z_ARRVAL_P(z_rect), "width", sizeof("width") - 1)) != NULL) {
		rect.width = zval_get_long(tmp);
	} else {
		php_error_docref(NULL, E_WARNING, "Missing width");
		RETURN_FALSE;
	}

	if ((tmp = zend_hash_str_find(Z_ARRVAL_P(z_rect), "height", sizeof("height") - 1)) != NULL) {
		rect.height = zval_get_long(tmp);
	} else {
		php_error_docref(NULL, E_WARNING, "Missing height");
		RETURN_FALSE;
	}

	im_crop = gdImageCrop(im, &rect);

	if (im_crop == NULL) {
		RETURN_FALSE;
	} else {
		RETURN_RES(zend_register_resource(im_crop, le_gd));
	}
}
/* }}} */

/* {{{ proto resource imagecropauto(resource im [, int mode = GD_CROP_DEFAULT [, float threshold [, int color]]])
   Crop an image automatically using one of the available modes. */
PHP_FUNCTION(imagecropauto)
{
	zval *IM;
	zend_long mode = GD_CROP_DEFAULT;
	zend_long color = -1;
	double threshold = 0.5f;
	gdImagePtr im;
	gdImagePtr im_crop;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r|ldl", &IM, &mode, &threshold, &color) == FAILURE)  {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	switch (mode) {
		case GD_CROP_DEFAULT:
		case GD_CROP_TRANSPARENT:
		case GD_CROP_BLACK:
		case GD_CROP_WHITE:
		case GD_CROP_SIDES:
			im_crop = gdImageCropAuto(im, mode);
			break;

		case GD_CROP_THRESHOLD:
			if (color < 0 || (!gdImageTrueColor(im) && color >= gdImageColorsTotal(im))) {
				php_error_docref(NULL, E_WARNING, "Color argument missing with threshold mode");
				RETURN_FALSE;
			}
			im_crop = gdImageCropThreshold(im, color, (float) threshold);
			break;

		default:
			php_error_docref(NULL, E_WARNING, "Unknown crop mode");
			RETURN_FALSE;
	}
	if (im_crop == NULL) {
		RETURN_FALSE;
	} else {
		RETURN_RES(zend_register_resource(im_crop, le_gd));
	}
}
/* }}} */

/* {{{ proto resource imagescale(resource im, int new_width[, int new_height[, int method]])
   Scale an image using the given new width and height. */
PHP_FUNCTION(imagescale)
{
	zval *IM;
	gdImagePtr im;
	gdImagePtr im_scaled = NULL;
	int new_width, new_height;
	zend_long tmp_w, tmp_h=-1, tmp_m = GD_BILINEAR_FIXED;
	gdInterpolationMethod method, old_method;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rl|ll", &IM, &tmp_w, &tmp_h, &tmp_m) == FAILURE)  {
		return;
	}
	method = tmp_m;

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	if (tmp_h < 0 || tmp_w < 0) {
		/* preserve ratio */
		long src_x, src_y;

		src_x = gdImageSX(im);
		src_y = gdImageSY(im);

		if (src_x && tmp_h < 0) {
			tmp_h = tmp_w * src_y / src_x;
		}
		if (src_y && tmp_w < 0) {
			tmp_w = tmp_h * src_x / src_y;
		}
	}

	if (tmp_h <= 0 || tmp_h > INT_MAX || tmp_w <= 0 || tmp_w > INT_MAX) {
		RETURN_FALSE;
	}

	new_width = tmp_w;
	new_height = tmp_h;

	/* gdImageGetInterpolationMethod() is only available as of GD 2.1.1 */
	old_method = im->interpolation_id;
	if (gdImageSetInterpolationMethod(im, method)) {
		im_scaled = gdImageScale(im, new_width, new_height);
	}
	gdImageSetInterpolationMethod(im, old_method);

	if (im_scaled == NULL) {
		RETURN_FALSE;
	} else {
		RETURN_RES(zend_register_resource(im_scaled, le_gd));
	}
}
/* }}} */

/* {{{ proto resource imageaffine(resource src, array affine[, array clip])
   Return an image containing the affine tramsformed src image, using an optional clipping area */
PHP_FUNCTION(imageaffine)
{
	zval *IM;
	gdImagePtr src;
	gdImagePtr dst;
	gdRect rect;
	gdRectPtr pRect = NULL;
	zval *z_rect = NULL;
	zval *z_affine;
	zval *tmp;
	double affine[6];
	int i, nelems;
	zval *zval_affine_elem = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ra|a", &IM, &z_affine, &z_rect) == FAILURE)  {
		return;
	}

	if ((src = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	if ((nelems = zend_hash_num_elements(Z_ARRVAL_P(z_affine))) != 6) {
		php_error_docref(NULL, E_WARNING, "Affine array must have six elements");
		RETURN_FALSE;
	}

	for (i = 0; i < nelems; i++) {
		if ((zval_affine_elem = zend_hash_index_find(Z_ARRVAL_P(z_affine), i)) != NULL) {
			switch (Z_TYPE_P(zval_affine_elem)) {
				case IS_LONG:
					affine[i]  = Z_LVAL_P(zval_affine_elem);
					break;
				case IS_DOUBLE:
					affine[i] = Z_DVAL_P(zval_affine_elem);
					break;
				case IS_STRING:
					affine[i] = zval_get_double(zval_affine_elem);
					break;
				default:
					php_error_docref(NULL, E_WARNING, "Invalid type for element %i", i);
					RETURN_FALSE;
			}
		}
	}

	if (z_rect != NULL) {
		if ((tmp = zend_hash_str_find(Z_ARRVAL_P(z_rect), "x", sizeof("x") - 1)) != NULL) {
			rect.x = zval_get_long(tmp);
		} else {
			php_error_docref(NULL, E_WARNING, "Missing x position");
			RETURN_FALSE;
		}

		if ((tmp = zend_hash_str_find(Z_ARRVAL_P(z_rect), "y", sizeof("y") - 1)) != NULL) {
			rect.y = zval_get_long(tmp);
		} else {
			php_error_docref(NULL, E_WARNING, "Missing y position");
			RETURN_FALSE;
		}

		if ((tmp = zend_hash_str_find(Z_ARRVAL_P(z_rect), "width", sizeof("width") - 1)) != NULL) {
			rect.width = zval_get_long(tmp);
		} else {
			php_error_docref(NULL, E_WARNING, "Missing width");
			RETURN_FALSE;
		}

		if ((tmp = zend_hash_str_find(Z_ARRVAL_P(z_rect), "height", sizeof("height") - 1)) != NULL) {
			rect.height = zval_get_long(tmp);
		} else {
			php_error_docref(NULL, E_WARNING, "Missing height");
			RETURN_FALSE;
		}
		pRect = &rect;
	} else {
		rect.x = -1;
		rect.y = -1;
		rect.width = gdImageSX(src);
		rect.height = gdImageSY(src);
		pRect = NULL;
	}

	if (gdTransformAffineGetImage(&dst, src, pRect, affine) != GD_TRUE) {
		RETURN_FALSE;
	}

	if (dst == NULL) {
		RETURN_FALSE;
	} else {
		RETURN_RES(zend_register_resource(dst, le_gd));
	}
}
/* }}} */

/* {{{ proto array imageaffinematrixget(int type[, array options])
   Return an image containing the affine tramsformed src image, using an optional clipping area */
PHP_FUNCTION(imageaffinematrixget)
{
	double affine[6];
	zend_long type;
	zval *options = NULL;
	zval *tmp;
	int res = GD_FALSE, i;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|z", &type, &options) == FAILURE)  {
		return;
	}

	switch((gdAffineStandardMatrix)type) {
		case GD_AFFINE_TRANSLATE:
		case GD_AFFINE_SCALE: {
			double x, y;
			if (!options || Z_TYPE_P(options) != IS_ARRAY) {
				php_error_docref(NULL, E_WARNING, "Array expected as options");
				RETURN_FALSE;
			}
			if ((tmp = zend_hash_str_find(Z_ARRVAL_P(options), "x", sizeof("x") - 1)) != NULL) {
				x = zval_get_double(tmp);
			} else {
				php_error_docref(NULL, E_WARNING, "Missing x position");
				RETURN_FALSE;
			}

			if ((tmp = zend_hash_str_find(Z_ARRVAL_P(options), "y", sizeof("y") - 1)) != NULL) {
				y = zval_get_double(tmp);
			} else {
				php_error_docref(NULL, E_WARNING, "Missing y position");
				RETURN_FALSE;
			}

			if (type == GD_AFFINE_TRANSLATE) {
				res = gdAffineTranslate(affine, x, y);
			} else {
				res = gdAffineScale(affine, x, y);
			}
			break;
		}

		case GD_AFFINE_ROTATE:
		case GD_AFFINE_SHEAR_HORIZONTAL:
		case GD_AFFINE_SHEAR_VERTICAL: {
			double angle;

			if (!options) {
				php_error_docref(NULL, E_WARNING, "Number is expected as option");
				RETURN_FALSE;
			}

			angle = zval_get_double(options);

			if (type == GD_AFFINE_SHEAR_HORIZONTAL) {
				res = gdAffineShearHorizontal(affine, angle);
			} else if (type == GD_AFFINE_SHEAR_VERTICAL) {
				res = gdAffineShearVertical(affine, angle);
			} else {
				res = gdAffineRotate(affine, angle);
			}
			break;
		}

		default:
			php_error_docref(NULL, E_WARNING, "Invalid type for element " ZEND_LONG_FMT, type);
			RETURN_FALSE;
	}

	if (res == GD_FALSE) {
		RETURN_FALSE;
	} else {
		array_init(return_value);
		for (i = 0; i < 6; i++) {
			add_index_double(return_value, i, affine[i]);
		}
	}
} /* }}} */

/* {{{ proto array imageaffineconcat(array m1, array m2)
   Concat two matrices (as in doing many ops in one go) */
PHP_FUNCTION(imageaffinematrixconcat)
{
	double m1[6];
	double m2[6];
	double mr[6];

	zval *tmp;
	zval *z_m1;
	zval *z_m2;
	int i, nelems;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "aa", &z_m1, &z_m2) == FAILURE)  {
		return;
	}

	if (((nelems = zend_hash_num_elements(Z_ARRVAL_P(z_m1))) != 6) || (nelems = zend_hash_num_elements(Z_ARRVAL_P(z_m2))) != 6) {
		php_error_docref(NULL, E_WARNING, "Affine arrays must have six elements");
		RETURN_FALSE;
	}

	for (i = 0; i < 6; i++) {
		if ((tmp = zend_hash_index_find(Z_ARRVAL_P(z_m1), i)) != NULL) {
			switch (Z_TYPE_P(tmp)) {
				case IS_LONG:
					m1[i]  = Z_LVAL_P(tmp);
					break;
				case IS_DOUBLE:
					m1[i] = Z_DVAL_P(tmp);
					break;
				case IS_STRING:
					m1[i] = zval_get_double(tmp);
					break;
				default:
					php_error_docref(NULL, E_WARNING, "Invalid type for element %i", i);
					RETURN_FALSE;
			}
		}
		if ((tmp = zend_hash_index_find(Z_ARRVAL_P(z_m2), i)) != NULL) {
			switch (Z_TYPE_P(tmp)) {
				case IS_LONG:
					m2[i]  = Z_LVAL_P(tmp);
					break;
				case IS_DOUBLE:
					m2[i] = Z_DVAL_P(tmp);
					break;
				case IS_STRING:
					m2[i] = zval_get_double(tmp);
					break;
				default:
					php_error_docref(NULL, E_WARNING, "Invalid type for element %i", i);
					RETURN_FALSE;
			}
		}
	}

	if (gdAffineConcat (mr, m1, m2) != GD_TRUE) {
		RETURN_FALSE;
	}

	array_init(return_value);
	for (i = 0; i < 6; i++) {
		add_index_double(return_value, i, mr[i]);
	}
} /* }}} */

/* {{{ proto resource imagesetinterpolation(resource im [, int method]])
   Set the default interpolation method, passing -1 or 0 sets it to the libgd default (bilinear). */
PHP_FUNCTION(imagesetinterpolation)
{
	zval *IM;
	gdImagePtr im;
	zend_long method = GD_BILINEAR_FIXED;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r|l", &IM, &method) == FAILURE)  {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	if (method == -1) {
		 method = GD_BILINEAR_FIXED;
	}
	RETURN_BOOL(gdImageSetInterpolationMethod(im, (gdInterpolationMethod) method));
}
/* }}} */

/* {{{ proto array imageresolution(resource im [, res_x, [res_y]])
   Get or set the resolution of the image in DPI. */
PHP_FUNCTION(imageresolution)
{
	zval *IM;
	gdImagePtr im;
	zend_long res_x = GD_RESOLUTION, res_y = GD_RESOLUTION;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r|ll", &IM, &res_x, &res_y) == FAILURE)  {
		return;
	}

	if ((im = (gdImagePtr)zend_fetch_resource(Z_RES_P(IM), "Image", le_gd)) == NULL) {
		return;
	}

	switch (ZEND_NUM_ARGS()) {
		case 3:
			gdImageSetResolution(im, res_x, res_y);
			RETURN_TRUE;
		case 2:
			gdImageSetResolution(im, res_x, res_x);
			RETURN_TRUE;
		default:
			array_init(return_value);
			add_next_index_long(return_value, gdImageResolutionX(im));
			add_next_index_long(return_value, gdImageResolutionY(im));
	}
}
/* }}} */

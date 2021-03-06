/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * Video Redirection Virtual Channel - Codec
 *
 * Copyright 2010-2011 Vic Lee
 * Copyright 2012 Hewlett-Packard Development Company, L.P.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <winpr/stream.h>
#include <winpr/print.h>

#include "tsmf_constants.h"
#include "tsmf_types.h"

#include "tsmf_codec.h"

typedef struct _TSMFMediaTypeMap
{
	BYTE guid[16];
	const char* name;
	int type;
} TSMFMediaTypeMap;

static const TSMFMediaTypeMap tsmf_major_type_map[] =
{
	/* 73646976-0000-0010-8000-00AA00389B71 */
	{
		{ 0x76, 0x69, 0x64, 0x73, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 },
		"MEDIATYPE_Video",
		TSMF_MAJOR_TYPE_VIDEO
	},

	/* 73647561-0000-0010-8000-00AA00389B71 */
	{
		{ 0x61, 0x75, 0x64, 0x73, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 },
		"MEDIATYPE_Audio",
		TSMF_MAJOR_TYPE_AUDIO
	},

	{
		{ 0 },
		"Unknown",
		TSMF_MAJOR_TYPE_UNKNOWN
	}
};

static const TSMFMediaTypeMap tsmf_sub_type_map[] =
{
	/* 31435657-0000-0010-8000-00AA00389B71 */
	{
		{ 0x57, 0x56, 0x43, 0x31, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 },
		"MEDIASUBTYPE_WVC1",
		TSMF_SUB_TYPE_WVC1
	},

	/* 00000161-0000-0010-8000-00AA00389B71 */
	{
		{ 0x61, 0x01, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 },
		"MEDIASUBTYPE_WMAudioV2", /* V7, V8 has the same GUID */
		TSMF_SUB_TYPE_WMA2
	},

	/* 00000162-0000-0010-8000-00AA00389B71 */
	{
		{ 0x62, 0x01, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 },
		"MEDIASUBTYPE_WMAudioV9",
		TSMF_SUB_TYPE_WMA9
	},

	/* 00000055-0000-0010-8000-00AA00389B71 */
	{
		{ 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 },
		"MEDIASUBTYPE_MP3",
		TSMF_SUB_TYPE_MP3
	},

	/* E06D802B-DB46-11CF-B4D1-00805F6CBBEA */
	{
		{ 0x2B, 0x80, 0x6D, 0xE0, 0x46, 0xDB, 0xCF, 0x11, 0xB4, 0xD1, 0x00, 0x80, 0x5F, 0x6C, 0xBB, 0xEA },
		"MEDIASUBTYPE_MPEG2_AUDIO",
		TSMF_SUB_TYPE_MP2A
	},

	/* E06D8026-DB46-11CF-B4D1-00805F6CBBEA */
	{
		{ 0x26, 0x80, 0x6D, 0xE0, 0x46, 0xDB, 0xCF, 0x11, 0xB4, 0xD1, 0x00, 0x80, 0x5F, 0x6C, 0xBB, 0xEA },
		"MEDIASUBTYPE_MPEG2_VIDEO",
		TSMF_SUB_TYPE_MP2V
	},

	/* 31564D57-0000-0010-8000-00AA00389B71 */
	{
		{ 0x57, 0x4D, 0x56, 0x31, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 },
		"MEDIASUBTYPE_WMV1",
		TSMF_SUB_TYPE_WMV1
	},

	/* 32564D57-0000-0010-8000-00AA00389B71 */
	{
		{ 0x57, 0x4D, 0x56, 0x32, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 },
		"MEDIASUBTYPE_WMV2",
		TSMF_SUB_TYPE_WMV2
	},

	/* 33564D57-0000-0010-8000-00AA00389B71 */
	{
		{ 0x57, 0x4D, 0x56, 0x33, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 },
		"MEDIASUBTYPE_WMV3",
		TSMF_SUB_TYPE_WMV3
	},

	/* 00001610-0000-0010-8000-00AA00389B71 */
	{
		{ 0x10, 0x16, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 },
		"MEDIASUBTYPE_MPEG_HEAAC",
		TSMF_SUB_TYPE_AAC
	},

	/* 34363248-0000-0010-8000-00AA00389B71 */
	{
		{ 0x48, 0x32, 0x36, 0x34, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 },
		"MEDIASUBTYPE_H264",
		TSMF_SUB_TYPE_H264
	},

	/* 31435641-0000-0010-8000-00AA00389B71 */
	{
		{ 0x41, 0x56, 0x43, 0x31, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 },
		"MEDIASUBTYPE_AVC1",
		TSMF_SUB_TYPE_AVC1
	},

	/* 3334504D-0000-0010-8000-00AA00389B71 */
	{
		{ 0x4D, 0x50, 0x34, 0x33, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 },
		"MEDIASUBTYPE_MP43",
		TSMF_SUB_TYPE_MP43
	},

	/* 5634504D-0000-0010-8000-00AA00389B71 */
	{
		{ 0x4D, 0x50, 0x34, 0x56, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 },
		"MEDIASUBTYPE_MP4S",
		TSMF_SUB_TYPE_MP4S
	},

	/* 3234504D-0000-0010-8000-00AA00389B71 */
	{
		{ 0x4D, 0x50, 0x34, 0x32, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 },
		"MEDIASUBTYPE_MP42",
		TSMF_SUB_TYPE_MP42
	},

	/* E436EB81-524F-11CE-9F53-0020AF0BA770 */
	/*
	{
		{ 0x81, 0xEB, 0x36, 0xE4, 0x4F, 0x52, 0xCE, 0x11, 0x9F, 0x53, 0x00, 0x20, 0xAF, 0x0B, 0xA7, 0x70 },
		"MEDIASUBTYPE_MP1V",
		TSMF_SUB_TYPE_MP1V
	},
	*/

	/* 00000050-0000-0010-8000-00AA00389B71 */
	/*
	{
		{ 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 },
		"MEDIASUBTYPE_MP1A",
		TSMF_SUB_TYPE_MP1A
	},
	*/

	/* E06D802C-DB46-11CF-B4D1-00805F6CBBEA */
	/*
	{
		{ 0x2C, 0x80, 0x6D, 0xE0, 0x46, 0xDB, 0xCF, 0x11, 0xB4, 0xD1, 0x00, 0x80, 0x5F, 0x6C, 0xBB, 0xEA },
		"MEDIASUBTYPE_DOLBY_AC3",
		TSMF_SUB_TYPE_AC3
	},
	*/

	/* 32595559-0000-0010-8000-00AA00389B71 */
	/*
	{
		{ 0x59, 0x55, 0x59, 0x32, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 },
		"MEDIASUBTYPE_YUY2",
		TSMF_SUB_TYPE_YUY2
	},
	*/

	{
		{ 0 },
		"Unknown",
		TSMF_SUB_TYPE_UNKNOWN
	}

};

static const TSMFMediaTypeMap tsmf_format_type_map[] =
{
	/* AED4AB2D-7326-43CB-9464-C879CAB9C43D */
	{
		{ 0x2D, 0xAB, 0xD4, 0xAE, 0x26, 0x73, 0xCB, 0x43, 0x94, 0x64, 0xC8, 0x79, 0xCA, 0xB9, 0xC4, 0x3D },
		"FORMAT_MFVideoFormat",
		TSMF_FORMAT_TYPE_MFVIDEOFORMAT
	},

	/* 05589F81-C356-11CE-BF01-00AA0055595A */
	{
		{ 0x81, 0x9F, 0x58, 0x05, 0x56, 0xC3, 0xCE, 0x11, 0xBF, 0x01, 0x00, 0xAA, 0x00, 0x55, 0x59, 0x5A },
		"FORMAT_WaveFormatEx",
		TSMF_FORMAT_TYPE_WAVEFORMATEX
	},

	/* E06D80E3-DB46-11CF-B4D1-00805F6CBBEA */
	{
		{ 0xE3, 0x80, 0x6D, 0xE0, 0x46, 0xDB, 0xCF, 0x11, 0xB4, 0xD1, 0x00, 0x80, 0x5F, 0x6C, 0xBB, 0xEA },
		"FORMAT_MPEG2_VIDEO",
		TSMF_FORMAT_TYPE_MPEG2VIDEOINFO
	},

	/* F72A76A0-EB0A-11D0-ACE4-0000C0CC16BA */
	{
		{ 0xA0, 0x76, 0x2A, 0xF7, 0x0A, 0xEB, 0xD0, 0x11, 0xAC, 0xE4, 0x00, 0x00, 0xC0, 0xCC, 0x16, 0xBA },
		"FORMAT_VideoInfo2",
		TSMF_FORMAT_TYPE_VIDEOINFO2
	},

	/* 05589F82-C356-11CE-BF01-00AA0055595A */
	{
		{ 0x82, 0x9F, 0x58, 0x05, 0x56, 0xC3, 0xCE, 0x11, 0xBF, 0x01, 0x00, 0xAA, 0x00, 0x55, 0x59, 0x5A },
		"FORMAT_MPEG1_VIDEO",
		TSMF_FORMAT_TYPE_MPEG1VIDEOINFO
	},

	{
		{ 0 },
		"Unknown",
		TSMF_FORMAT_TYPE_UNKNOWN
	}
};

static void tsmf_print_guid(const BYTE* guid)
{
#ifdef WITH_DEBUG_DVC
	int i;

	for (i = 3; i >= 0; i--)
		fprintf(stderr, "%02X", guid[i]);
	fprintf(stderr, "-");
	for (i = 5; i >= 4; i--)
		fprintf(stderr, "%02X", guid[i]);
	fprintf(stderr, "-");
	for (i = 7; i >= 6; i--)
		fprintf(stderr, "%02X", guid[i]);
	fprintf(stderr, "-");
	for (i = 8; i < 16; i++)
	{
		fprintf(stderr, "%02X", guid[i]);
		if (i == 9)
			fprintf(stderr, "-");
	}
	fprintf(stderr, "\n");
#endif
}

/* http://msdn.microsoft.com/en-us/library/dd318229.aspx */
static UINT32 tsmf_codec_parse_BITMAPINFOHEADER(TS_AM_MEDIA_TYPE* mediatype, wStream* s, BOOL bypass)
{
	UINT32 biSize;
	UINT32 biWidth;
	UINT32 biHeight;

	stream_read_UINT32(s, biSize);
	stream_read_UINT32(s, biWidth);
	stream_read_UINT32(s, biHeight);
	stream_seek(s, 28);

	if (mediatype->Width == 0)
		mediatype->Width = biWidth;
	if (mediatype->Height == 0)
		mediatype->Height = biHeight;
	/* Assume there will be no color table for video? */

	if (bypass && biSize > 40)
		stream_seek(s, biSize - 40);

	return (bypass ? biSize : 40);
}

/* http://msdn.microsoft.com/en-us/library/dd407326.aspx */
static UINT32 tsmf_codec_parse_VIDEOINFOHEADER2(TS_AM_MEDIA_TYPE* mediatype, wStream* s)
{
	UINT64 AvgTimePerFrame;

	/* VIDEOINFOHEADER2.rcSource, RECT(LONG left, LONG top, LONG right, LONG bottom) */
	stream_seek_UINT32(s);
	stream_seek_UINT32(s);
	stream_read_UINT32(s, mediatype->Width);
	stream_read_UINT32(s, mediatype->Height);
	/* VIDEOINFOHEADER2.rcTarget */
	stream_seek(s, 16);
	/* VIDEOINFOHEADER2.dwBitRate */
	stream_read_UINT32(s, mediatype->BitRate);
	/* VIDEOINFOHEADER2.dwBitErrorRate */
	stream_seek_UINT32(s);
	/* VIDEOINFOHEADER2.AvgTimePerFrame */
	stream_read_UINT64(s, AvgTimePerFrame);
	mediatype->SamplesPerSecond.Numerator = 1000000;
	mediatype->SamplesPerSecond.Denominator = (int)(AvgTimePerFrame / 10LL);
	/* Remaining fields before bmiHeader */
	stream_seek(s, 24);

	return 72;
}

/* http://msdn.microsoft.com/en-us/library/dd390700.aspx */
static UINT32 tsmf_codec_parse_VIDEOINFOHEADER(TS_AM_MEDIA_TYPE* mediatype, wStream* s)
{
/*
typedef struct tagVIDEOINFOHEADER {
  RECT             rcSource;			//16
  RECT             rcTarget;			//16	32
  DWORD            dwBitRate;			//4	36
  DWORD            dwBitErrorRate;		//4	40
  REFERENCE_TIME   AvgTimePerFrame;		//8	48
  BITMAPINFOHEADER bmiHeader;
} VIDEOINFOHEADER;
*/
	UINT64 AvgTimePerFrame;

	/* VIDEOINFOHEADER.rcSource, RECT(LONG left, LONG top, LONG right, LONG bottom) */
	stream_seek_UINT32(s);
	stream_seek_UINT32(s);
	stream_read_UINT32(s, mediatype->Width);
	stream_read_UINT32(s, mediatype->Height);
	/* VIDEOINFOHEADER.rcTarget */
	stream_seek(s, 16);
	/* VIDEOINFOHEADER.dwBitRate */
	stream_read_UINT32(s, mediatype->BitRate);
	/* VIDEOINFOHEADER.dwBitErrorRate */
	stream_seek_UINT32(s);
	/* VIDEOINFOHEADER.AvgTimePerFrame */
	stream_read_UINT64(s, AvgTimePerFrame);
	mediatype->SamplesPerSecond.Numerator = 1000000;
	mediatype->SamplesPerSecond.Denominator = (int)(AvgTimePerFrame / 10LL);

	return 48;
}

BOOL tsmf_codec_parse_media_type(TS_AM_MEDIA_TYPE* mediatype, wStream* s)
{
	int i;
	UINT32 cbFormat;
	BOOL ret = TRUE;

	memset(mediatype, 0, sizeof(TS_AM_MEDIA_TYPE));

	/* MajorType */
	DEBUG_DVC("MajorType:");
	tsmf_print_guid(stream_get_tail(s));
	for (i = 0; tsmf_major_type_map[i].type != TSMF_MAJOR_TYPE_UNKNOWN; i++)
	{
		if (memcmp(tsmf_major_type_map[i].guid, stream_get_tail(s), 16) == 0)
			break;
	}
	mediatype->MajorType = tsmf_major_type_map[i].type;
	if (mediatype->MajorType == TSMF_MAJOR_TYPE_UNKNOWN)
		ret = FALSE;
	DEBUG_DVC("MajorType %s", tsmf_major_type_map[i].name);
	stream_seek(s, 16);

	/* SubType */
	DEBUG_DVC("SubType:");
	tsmf_print_guid(stream_get_tail(s));
	for (i = 0; tsmf_sub_type_map[i].type != TSMF_SUB_TYPE_UNKNOWN; i++)
	{
		if (memcmp(tsmf_sub_type_map[i].guid, stream_get_tail(s), 16) == 0)
			break;
	}
	mediatype->SubType = tsmf_sub_type_map[i].type;
	if (mediatype->SubType == TSMF_SUB_TYPE_UNKNOWN)
		ret = FALSE;
	DEBUG_DVC("SubType %s", tsmf_sub_type_map[i].name);
	stream_seek(s, 16);

	/* bFixedSizeSamples, bTemporalCompression, SampleSize */
	stream_seek(s, 12);

	/* FormatType */
	DEBUG_DVC("FormatType:");
	tsmf_print_guid(stream_get_tail(s));
	for (i = 0; tsmf_format_type_map[i].type != TSMF_FORMAT_TYPE_UNKNOWN; i++)
	{
		if (memcmp(tsmf_format_type_map[i].guid, stream_get_tail(s), 16) == 0)
			break;
	}
	mediatype->FormatType = tsmf_format_type_map[i].type;
	if (mediatype->FormatType == TSMF_FORMAT_TYPE_UNKNOWN)
		ret = FALSE;
	DEBUG_DVC("FormatType %s", tsmf_format_type_map[i].name);
	stream_seek(s, 16);

	/* cbFormat */
	stream_read_UINT32(s, cbFormat);
	DEBUG_DVC("cbFormat %d", cbFormat);

#ifdef WITH_DEBUG_DVC
	winpr_HexDump(stream_get_tail(s), cbFormat);
#endif

	switch (mediatype->FormatType)
	{
		case TSMF_FORMAT_TYPE_MFVIDEOFORMAT:
			/* http://msdn.microsoft.com/en-us/library/aa473808.aspx */

			stream_seek(s, 8); /* dwSize and ? */
			stream_read_UINT32(s, mediatype->Width); /* videoInfo.dwWidth */
			stream_read_UINT32(s, mediatype->Height); /* videoInfo.dwHeight */
			stream_seek(s, 32);
			/* videoInfo.FramesPerSecond */
			stream_read_UINT32(s, mediatype->SamplesPerSecond.Numerator);
			stream_read_UINT32(s, mediatype->SamplesPerSecond.Denominator);
			stream_seek(s, 80);
			stream_read_UINT32(s, mediatype->BitRate); /* compressedInfo.AvgBitrate */
			stream_seek(s, 36);

			if (cbFormat > 176)
			{
				mediatype->ExtraDataSize = cbFormat - 176;
				mediatype->ExtraData = stream_get_tail(s);
			}
			break;

		case TSMF_FORMAT_TYPE_WAVEFORMATEX:
			/* http://msdn.microsoft.com/en-us/library/dd757720.aspx */

			stream_seek_UINT16(s);
			stream_read_UINT16(s, mediatype->Channels);
			stream_read_UINT32(s, mediatype->SamplesPerSecond.Numerator);
			mediatype->SamplesPerSecond.Denominator = 1;
			stream_read_UINT32(s, mediatype->BitRate);
			mediatype->BitRate *= 8;
			stream_read_UINT16(s, mediatype->BlockAlign);
			stream_read_UINT16(s, mediatype->BitsPerSample);
			stream_read_UINT16(s, mediatype->ExtraDataSize);
			if (mediatype->ExtraDataSize > 0)
				mediatype->ExtraData = stream_get_tail(s);
			
			break;

		case TSMF_FORMAT_TYPE_MPEG1VIDEOINFO:
			/* http://msdn.microsoft.com/en-us/library/dd390700.aspx */

			i = tsmf_codec_parse_VIDEOINFOHEADER(mediatype, s);
			i += tsmf_codec_parse_BITMAPINFOHEADER(mediatype, s, TRUE);
			if (cbFormat > i)
			{
				mediatype->ExtraDataSize = cbFormat - i;
				mediatype->ExtraData = stream_get_tail(s);
			}
			break;

		case TSMF_FORMAT_TYPE_MPEG2VIDEOINFO:
			/* http://msdn.microsoft.com/en-us/library/dd390707.aspx */

			i = tsmf_codec_parse_VIDEOINFOHEADER2(mediatype, s);
			i += tsmf_codec_parse_BITMAPINFOHEADER(mediatype, s, TRUE);
			if (cbFormat > i)
			{
				mediatype->ExtraDataSize = cbFormat - i;
				mediatype->ExtraData = stream_get_tail(s);
			}
			break;

		case TSMF_FORMAT_TYPE_VIDEOINFO2:
			i = tsmf_codec_parse_VIDEOINFOHEADER2(mediatype, s);
			i += tsmf_codec_parse_BITMAPINFOHEADER(mediatype, s, FALSE);
			if (cbFormat > i)
			{
				mediatype->ExtraDataSize = cbFormat - i;
				mediatype->ExtraData = stream_get_tail(s);
			}
			break;

		default:
			break;
	}

	if (mediatype->SamplesPerSecond.Numerator == 0)
		mediatype->SamplesPerSecond.Numerator = 1;
	if (mediatype->SamplesPerSecond.Denominator == 0)
		mediatype->SamplesPerSecond.Denominator = 1;

	return ret;
}

BOOL tsmf_codec_check_media_type(wStream* s)
{
	BYTE* m;
	BOOL ret;
	TS_AM_MEDIA_TYPE mediatype;

	stream_get_mark(s, m);
	ret = tsmf_codec_parse_media_type(&mediatype, s);
	stream_set_mark(s, m);

	return ret;
}

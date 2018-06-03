#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

typedef GQuark GstTranscodingFormat;

/* Passthrough type, for containers this means no muxing */
#define GST_TRANSCODING_FORMAT_NONE (gst_transcoding_format_none_quark())
GstTranscodingFormat gst_transcoding_format_none_quark (void);

/* Audio + Video Matroska */
#define GST_TRANSCODING_FORMAT_MATROSKA (gst_transcoding_format_matroska_quark())
GstTranscodingFormat gst_transcoding_format_matroska_quark (void);

#define GST_TRANSCODING_FORMAT_H264 (gst_transcoding_format_h264_quark())
GstTranscodingFormat gst_transcoding_format_h264_quark (void);

#define GST_TRANSCODING_FORMAT_AAC (gst_transcoding_format_aac_quark())
GstTranscodingFormat gst_transcoding_format_aac_quark (void);

#define GST_TRANSCODING_TYPE_JOB gst_transcoding_job_get_type ()
G_DECLARE_FINAL_TYPE(GstTranscodingJob, gst_transcoding_job, GST_TRANSCODING, JOB, GObject)

#define GST_TRANSCODING_TYPE_STREAM_PROFILE gst_transcoding_stream_profile_get_type ()
G_DECLARE_DERIVABLE_TYPE(GstTranscodingStreamProfile, gst_transcoding_stream_profile, GST_TRANSCODING, STREAM_PROFILE, GObject)

struct _GstTranscodingStreamProfileClass
{
  GObjectClass parent_class;
};

#define GST_TRANSCODING_TYPE_VIDEO_PROFILE gst_transcoding_video_profile_get_type ()
G_DECLARE_FINAL_TYPE(GstTranscodingVideoProfile, gst_transcoding_video_profile, GST_TRANSCODING, VIDEO_PROFILE, GstTranscodingStreamProfile)

#define GST_TRANSCODING_TYPE_AUDIO_PROFILE gst_transcoding_audio_profile_get_type ()
G_DECLARE_FINAL_TYPE(GstTranscodingAudioProfile, gst_transcoding_audio_profile, GST_TRANSCODING, AUDIO_PROFILE, GstTranscodingStreamProfile)

#define GST_TRANSCODING_TYPE_CONTAINER_PROFILE gst_transcoding_container_profile_get_type ()
G_DECLARE_FINAL_TYPE(GstTranscodingContainerProfile, gst_transcoding_container_profile, GST_TRANSCODING, CONTAINER_PROFILE, GObject)

#define GST_TRANSCODING_TYPE_INPUT gst_transcoding_input_get_type ()
G_DECLARE_FINAL_TYPE(GstTranscodingInput, gst_transcoding_input, GST_TRANSCODING, INPUT, GObject)

#define GST_TRANSCODING_TYPE_OUTPUT gst_transcoding_output_get_type ()
G_DECLARE_FINAL_TYPE(GstTranscodingOutput, gst_transcoding_output, GST_TRANSCODING, OUTPUT, GObject)

GstTranscodingJob *gst_transcoding_job_new();

GstTranscodingVideoProfile * gst_transcoding_job_map_video_stream (GstTranscodingJob *self,
                                                                   const gchar *in_uri,
                                                                   const gchar *stream_id,
                                                                   const gchar *out_uri);

GstTranscodingAudioProfile * gst_transcoding_job_map_audio_stream (GstTranscodingJob *self,
                                                                   const gchar *in_uri,
                                                                   const gchar *stream_id,
                                                                   const gchar *out_uri);

GstTranscodingInput * gst_transcoding_job_add_input (GstTranscodingJob *self,
                                                     const gchar *uri);

GstTranscodingOutput * gst_transcoding_job_add_output (GstTranscodingJob *self,
                                                       const gchar *uri,
                                                       GstTranscodingContainerProfile *profile);

GstTranscodingInput * gst_transcoding_stream_profile_get_input (GstTranscodingStreamProfile *self);

GstTranscodingOutput * gst_transcoding_stream_profile_get_output (GstTranscodingStreamProfile *self);

GstTranscodingFormat gst_transcoding_stream_profile_get_format (GstTranscodingStreamProfile *self);

void gst_transcoding_stream_profile_set_format (GstTranscodingStreamProfile *self, GstTranscodingFormat format);

GstTranscodingFormat gst_transcoding_container_profile_get_format (GstTranscodingContainerProfile *self);

void gst_transcoding_container_profile_set_format (GstTranscodingContainerProfile *self, GstTranscodingFormat format);

GstTranscodingVideoProfile * gst_transcoding_container_profile_get_meta_video_profile (GstTranscodingContainerProfile * self);

GstTranscodingAudioProfile * gst_transcoding_container_profile_get_meta_audio_profile (GstTranscodingContainerProfile * self);

GstTranscodingContainerProfile * gst_transcoding_container_profile_new (GstTranscodingAudioProfile *meta_audio_profile,
    GstTranscodingVideoProfile *meta_video_profile);

gchar * gst_transcoding_input_get_uri (GstTranscodingInput *self);

gchar * gst_transcoding_output_get_uri (GstTranscodingOutput *self);

GstTranscodingContainerProfile * gst_transcoding_output_get_profile (GstTranscodingOutput *self);

GstTranscodingVideoProfile * gst_transcoding_video_profile_new (void);

GstTranscodingAudioProfile * gst_transcoding_audio_profile_new (void);

gchar *gst_transcoding_job_to_json (GstTranscodingJob *self, gboolean pretty);

G_END_DECLS
